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

	auto scores = DualFloodFill(Coord(x,y), Coord(ox,oy), empty);

	print_map(empty);

	cout << scores << endl;

	return 0;
}
