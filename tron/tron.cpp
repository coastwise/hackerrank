#include "tron.hpp"

int Coord::MaxX = 0;

const Coord TronState::NullAction {-1,-1};
const Coord Coord::Invalid {-1,-1};
const Coord Coord::Directions [4] { {-1,0}, {0,1}, {1,0}, {0,-1} };

bool Coord::operator== (const Coord& that) {
	return this->X == that.X && this->Y == that.Y;
}

Coord Coord::operator+ (Coord that) const {
	return that += *this;
}

std::ostream& operator<< (std::ostream& out, const Coord& coord) {
	return out << "{" << coord.X << "," << coord.Y << "}";
}

std::vector<Coord> Neighbours(const Coord& c, const std::bitset<15*15>& empty) {
	std::vector<Coord> neighbours;
	for (int i = 0; i < 4; ++i) {
		Coord neighbour = c + Coord::Directions[i];
		if (empty[neighbour.Index()]) {
			neighbours.push_back(neighbour);
		}
	}
	return neighbours;
}

std::istream& operator>> (std::istream& cin, Coord& coord) {
	return cin >> coord.Y >> coord.X;
}

std::istream& operator>> (std::istream& cin, TronState& state) {
	char player;
	cin >> player;

	if (player == 'r') {
		cin >> state.us >> state.them;
	} else {
		cin >> state.them >> state.us;
	}

	for (int index = 0; index < 15*15; ++index) {
		char space; cin >> space;
		state.empty[index] = (space == '-');
	}

	state.ourTurn = true;

	return cin;
}
