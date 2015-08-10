#include <iostream>
#include <vector>
#include <unordered_set>
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

	auto start = Coord(x,y);
	auto neighbours = Neighbours(start, empty);

	for (int i = 0; i < neighbours.size(); ++i) {
		cout << neighbours[i] << flush;
		this_thread::sleep_for(milliseconds(500));
		cout << '\r';
	}

	return 0;
}
