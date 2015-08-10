#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

struct Coord {
	int X, Y;

	Coord (int x, int y) : X{x}, Y{y} {}

	Coord& operator+= (const Coord& a) {
		X += a.X;
		Y += a.Y;
		return *this;
	}
};

bool operator== (const Coord& a, const Coord& b) {
	return a.X == b.X && a.Y == b.Y;
}

Coord operator+ (Coord a, const Coord &b) {
	return a += b;
}

struct CoordHash {
	size_t operator() (const Coord &coord) const {
		return hash<int>()(coord.X) ^ hash<int>()(coord.Y);
	}
};

Coord Directions [4] { {-1,0}, {0,1}, {1,0}, {0,-1} };

vector<Coord> Neighbours(Coord c, unordered_set<Coord,CoordHash>& walls) {
	vector<Coord> neighbours;
	for (int i = 0; i < 4; ++i) {
		Coord neighbour = c + Directions[i];
		if (walls.find(neighbour) != walls.end()) {
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

const string CursorUp15 = "\x1b[15A\r";
const string CursorDown15 = "\x1b[15B\r";

int main () {
	char player;
	int x, y, ox, oy;

	cin >> player;
	cin >> x >> y >> ox >> oy;

	unordered_set<Coord, CoordHash> walls;

	for (int r = 0; r < 15; ++r) {
		for (int c = 0; c < 15; ++c) {
			char space; cin >> space;
			if (space != '-') {
				walls.insert(Coord(r,c));
			}
		}
	}

	auto start = Coord(x,y);
	auto neighbours = Neighbours(start, walls);

	for (int i = 0; i < neighbours.size(); ++i) {
		cout << neighbours[i].X << "," << neighbours[i].Y << "\n";
	}

	return 0;
}
