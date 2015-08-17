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


#include <thread>
#include <chrono>
using namespace std::chrono;




#include <limits>

#define PRINT 0

int MinValue (TronState&, int alpha, int beta, int maxPlies);

Coord MiniMaxDecision(TronState& state, int maxPlies = numeric_limits<int>::max()) {
	auto actions = state.NextActions();
	if (actions.empty()) {
		// game over. no valid move to return.
		return Coord::Invalid;
	}

	int alpha = numeric_limits<int>::min();

	int bestValue = numeric_limits<int>::min();
	Coord bestAction = Coord::Invalid;
	for (auto action = actions.begin(); action != actions.end(); ++action) {
		Coord oldCoord = state.DoAction(*action);

		int value = MinValue(state, alpha, numeric_limits<int>::max(), maxPlies-1);
		if (value > bestValue) {
			bestValue = value;
			bestAction = *action;
		}

		alpha = max(alpha, value);

		state.UndoAction(*action, oldCoord);
	}

	return bestAction;
}

int MaxValue (TronState& state, int alpha, int beta, int maxPlies) {
	auto actions = state.NextActions();
	if (actions.empty()) {
		// game over. did we loose, or tie?
		return state.Result();
	}

	if (maxPlies == 0) {
		// TODO: make us, them, and empty private again
		auto score = DualFloodFill(state.us, state.them, state.empty);
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
		Coord oldCoord = state.DoAction(*action);

#if PRINT
		cout << CursorUp15;
		state.PrintMap();
#endif

		int value = MinValue(state, alpha, beta, maxPlies-1);

		if (value >= beta) {
			state.UndoAction(*action, oldCoord);
			return value;
		}

		if (value > bestValue) {
			bestValue = value;
		}

		alpha = max(alpha, value);

		state.UndoAction(*action, oldCoord);
	}

	return bestValue;
}

int MinValue (TronState& state, int alpha, int beta, int maxPlies) {
	auto actions = state.NextActions();
	if (actions.empty()) {
		// game over. since we go at the same time, and we evaluate our moves first
		// if they don't have any moves, we've won
		return Win;
	}

	if (maxPlies == 0) {
		auto score = DualFloodFill(state.us, state.them, state.empty);
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
		Coord oldCoord = state.DoAction(*action);

#if PRINT
		cout << CursorUp15;
		state.PrintMap();
#endif

		int value = MaxValue(state, alpha, beta, maxPlies-1);

		if (value <= alpha) {
			state.UndoAction(*action, oldCoord);
			return value;
		}

		if (value < bestValue) {
			bestValue = value;
		}

		beta = min(beta, value);

		state.UndoAction(*action, oldCoord);
	}

	return bestValue;
}


int main () {
	auto t0 = high_resolution_clock::now();

	Coord::MaxX = 15;

	TronState gameState;
	cin >> gameState;

	gameState.PrintMap();

	Coord bestMove = Coord::Invalid;
	int depth = 2;

	auto t1 = high_resolution_clock::now();
	auto elapsed = duration_cast<milliseconds>(t1-t0);

	auto limit = milliseconds(800);
	while (elapsed < limit) {
		bestMove = MiniMaxDecision(gameState, depth);
		
		t1 = high_resolution_clock::now();
		elapsed = duration_cast<milliseconds>(t1-t0);

		depth += 2;
	}

	cout << "our best move: " << bestMove << endl;
	cout << "finished after " << elapsed.count() << " seconds and " << depth << " plies" << endl;

	return 0;
}
