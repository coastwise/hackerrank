#include <iostream>
#include <vector>
#include <queue>
using namespace std;

struct Coord {
	int X, Y;

	static int MaxX;

	Coord (int x, int y) : X{x}, Y{y} {}

	int Index () {
		int result = MaxX*Y + X;
		return result;
	}

	Coord& operator+= (const Coord& a) {
		X += a.X;
		Y += a.Y;
		return *this;
	}
};

int Coord::MaxX = 0;
const Coord NullCoord = Coord(-1,-1);

bool operator== (const Coord& a, const Coord& b) {
	return a.X == b.X && a.Y == b.Y;
}

Coord operator+ (Coord a, const Coord &b) {
	return a += b;
}

ostream& operator<< (ostream& out, const Coord& c) {
	return out << "{" << c.X << "," << c.Y << "}";
}

namespace std {
	template<>
	struct hash<Coord> {
		size_t operator() (const Coord &coord) const {
			return hash<int>()(coord.X) ^ hash<int>()(coord.Y);
		}
	};

	// print's vector<bool> like a bitset
	ostream& operator<< (ostream& out, const vector<bool>& bits) {
		for (auto i = bits.crbegin(); i != bits.crend(); ++i) {
			out << *i;
		}
		return out;
	}

	ostream& operator<< (ostream& out, const pair<int,int>& p) {
		return out << "<" << p.first << "," << p.second << ">";
	}
}

Coord Directions [4] { {-1,0}, {0,1}, {1,0}, {0,-1} };

typedef bitset<15*15> MapBits;
//typedef vector<bool> MapBits;

vector<Coord> Neighbours(Coord c, MapBits& empty) {
	vector<Coord> neighbours;
	for (int i = 0; i < 4; ++i) {
		Coord neighbour = c + Directions[i];
		if (empty[neighbour.Index()]) {
			neighbours.push_back(neighbour);
		}
	}
	return neighbours;
}


pair<int,int> DualFloodFill (const Coord& A, const Coord& B, MapBits& empty) {

	MapBits a_fill;
	MapBits b_fill;

	auto frontier = queue<pair<Coord,bool>>();
	{
		auto as = Neighbours(A,empty);
		for (auto i = as.begin(); i != as.end(); ++i) {
			frontier.push(make_pair(move(*i),false));
		}
		auto bs = Neighbours(B,empty);
		for (auto i = bs.begin(); i != bs.end(); ++i) {
			frontier.push(make_pair(move(*i),true));
		}
	}

	while (!frontier.empty()) {
		auto current = frontier.front();
		frontier.pop();
		Coord coord = current.first;
		if (!current.second) {
			a_fill[coord.Index()] = true;
			empty[coord.Index()] = false;
		} else {
			b_fill[coord.Index()] = true;
			empty[coord.Index()] = false;
		}

		auto next = Neighbours(coord, empty);
		for (auto i = next.begin(); i != next.end(); ++i) {
			frontier.push(make_pair(move(*i),current.second));
		}
	}

	empty = a_fill | b_fill; // unecessary if we pass in empty by value

	return make_pair(a_fill.count(), b_fill.count());
}

#include <sstream>
// VT100 terminal control escape sequences
string CursorUp (int numLines) {
	stringstream out;
	out << "\x1b[" << numLines << "A";
	out << '\r'; // carriage return
	return out.str();
}

string CursorDown (int numLines) {
	stringstream out;
	out << "\x1b[" << numLines << "B";
	out << '\r'; // carriage return
	return out.str();
}

const string CursorUp1 = "\x1b[A\r";
const string CursorDown1 = "\x1b[B\r";
const string CursorUp15 = "\x1b[15A\r";
const string CursorDown15 = "\x1b[15B\r";

void print_map (MapBits& empty) {
	for (int index = 0; index < empty.size(); ++index) {
		cout << (empty[index] ? ' ' : '#');
		if ((index+1) % Coord::MaxX == 0) {
			cout << endl;
		}
	}
}

#include <thread>
#include <chrono>
using namespace std::chrono;


const int Win = 2;
const int Loss = 0;
const int Tie = 1;

#include <limits>

int MinValue (MapBits& empty, Coord us, Coord them, int alpha, int beta);

Coord MiniMaxDecision(MapBits& empty, Coord us, Coord them) {
	auto actions = Neighbours(us, empty);
	if (actions.empty()) {
		// game over. did we loose, or tie?
		actions = Neighbours(them, empty);
		if (actions.empty()) {
			return NullCoord;
		} else {
			return NullCoord;
		}
	}

	int alpha = numeric_limits<int>::min();

	int bestValue = numeric_limits<int>::min();
	Coord bestAction = NullCoord;
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

		int value = MinValue(empty, *action, them, alpha, numeric_limits<int>::max());
		if (value > bestValue) {
			bestValue = value;
			bestAction = *action;
		}

		alpha = max(alpha, value);

		empty[action->Index()] = true; // undo action
	}

	return bestAction;
}

int MaxValue (MapBits& empty, Coord us, Coord them, int alpha, int beta) {
	auto actions = Neighbours(us, empty);
	if (actions.empty()) {
		// game over. did we loose, or tie?
		actions = Neighbours(them, empty);
		if (actions.empty()) {
			return Tie;
		} else {
			return Loss;
		}
	}

	int bestValue = numeric_limits<int>::min();
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

		cout << CursorUp15;
		print_map(empty);

		int value = MinValue(empty, *action, them, alpha, beta);

		if (value >= beta) {
			empty[action->Index()] = true; // undo action
			return value;
		}

		if (value > bestValue) {
			bestValue = value;
		}

		alpha = max(alpha, value);

		empty[action->Index()] = true; // undo action
	}

	return bestValue;
}

int MinValue (MapBits& empty, Coord us, Coord them, int alpha, int beta) {
	auto actions = Neighbours(them, empty);
	if (actions.empty()) {
		// game over. since we go at the same time, and we evaluate our moves first
		// if they don't have any moves, we've won
		return Win;
	}

	int bestValue = numeric_limits<int>::max();
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

		cout << CursorUp15;
		print_map(empty);

		int value = MaxValue(empty, us, *action, alpha, beta);

		if (value <= alpha) {
			empty[action->Index()] = true; // undo action
			return value;
		}

		if (value < bestValue) {
			bestValue = value;
		}

		beta = min(beta, value);

		empty[action->Index()] = true; // undo action
	}

	return bestValue;
}


int main () {
	char player;
	int x, y, ox, oy;

	cin >> player;
	cin >> y >> x >> oy >> ox;

	Coord::MaxX = 15;

	MapBits empty;

	for (int index = 0; index < 15*15; ++index) {
		char space; cin >> space;
		empty[index] = (space == '-');
	}

	print_map(empty);

	Coord us = Coord(x,y);
	Coord them = Coord(ox,oy);

	Coord bestMove = MiniMaxDecision(empty, us, them);

	cout << "our best move: " << bestMove << endl;

	return 0;
}
