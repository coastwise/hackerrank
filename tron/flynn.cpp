#include <iostream>
#include <vector>
#include <queue>
using namespace std;

#include "tron.hpp"

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

typedef bitset<15*15> MapBits;

pair<int,int> DualFloodFill (const Coord& A, const Coord& B, MapBits empty) {

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

#define PRINT 0

int MinValue (MapBits& empty, Coord us, Coord them, int alpha, int beta, int maxPlies);

Coord MiniMaxDecision(MapBits& empty, Coord us, Coord them, int maxPlies = numeric_limits<int>::max()) {
	auto actions = Neighbours(us, empty);
	if (actions.empty()) {
		// game over. did we loose, or tie?
		actions = Neighbours(them, empty);
		if (actions.empty()) {
			return Coord::Invalid;
		} else {
			return Coord::Invalid;
		}
	}

	int alpha = numeric_limits<int>::min();

	int bestValue = numeric_limits<int>::min();
	Coord bestAction = Coord::Invalid;
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

		int value = MinValue(empty, *action, them, alpha, numeric_limits<int>::max(), maxPlies-1);
		if (value > bestValue) {
			bestValue = value;
			bestAction = *action;
		}

		alpha = max(alpha, value);

		empty[action->Index()] = true; // undo action
	}

	return bestAction;
}

int MaxValue (MapBits& empty, Coord us, Coord them, int alpha, int beta, int maxPlies) {
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

	if (maxPlies == 0) {
		auto score = DualFloodFill(us, them, empty);
		if (score.first == score.second) {
			return Tie;
		} else if (score.first > score.second) {
			return Win;
		} else {
			return Loss;
		}
	}

	int bestValue = numeric_limits<int>::min();
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

#if PRINT
		cout << CursorUp15;
		print_map(empty);
#endif

		int value = MinValue(empty, *action, them, alpha, beta, maxPlies-1);

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

int MinValue (MapBits& empty, Coord us, Coord them, int alpha, int beta, int maxPlies) {
	auto actions = Neighbours(them, empty);
	if (actions.empty()) {
		// game over. since we go at the same time, and we evaluate our moves first
		// if they don't have any moves, we've won
		return Win;
	}

	if (maxPlies == 0) {
		auto score = DualFloodFill(us, them, empty);
		if (score.first == score.second) {
			return Tie;
		} else if (score.first > score.second) {
			return Win;
		} else {
			return Loss;
		}
	}

	int bestValue = numeric_limits<int>::max();
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		empty[action->Index()] = false; // apply action

#if PRINT
		cout << CursorUp15;
		print_map(empty);
#endif

		int value = MaxValue(empty, us, *action, alpha, beta, maxPlies-1);

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
	auto t0 = high_resolution_clock::now();

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

	Coord bestMove = Coord::Invalid;
	int depth = 2;

	auto t1 = high_resolution_clock::now();
	auto elapsed = duration_cast<milliseconds>(t1-t0).count();
	while (elapsed < 800) {
		t0 = t1;
		bestMove = MiniMaxDecision(empty, us, them, depth);
		depth += 2;

		t1 = high_resolution_clock::now();
		auto plyDuration = duration_cast<milliseconds>(t1-t0).count();
		elapsed += plyDuration;
	}

	cout << "our best move: " << bestMove << endl;
	cout << "finished after " << elapsed << " seconds and " << depth << " plies" << endl;

	return 0;
}
