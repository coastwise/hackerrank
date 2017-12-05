#include "tron.hpp"

int Coord::MaxX = 0;

const Coord TronState::NullAction {-1,-1};
const Coord Coord::Invalid {-1,-1};
const Coord Coord::Directions [4] { {-1,0}, {0,1}, {1,0}, {0,-1} };

bool Coord::operator== (const Coord& that) const {
	return this->X == that.X && this->Y == that.Y;
}

Coord Coord::operator+ (Coord that) const {
	return that += *this;
}

int ManhattanDistance(const Coord& a, const Coord& b)
{
	int result = abs(a.X - b.X) + abs(a.Y - b.Y);
	return result;
}

std::ostream& operator<< (std::ostream& out, const Coord& coord) {
	return out << "{" << coord.X << "," << coord.Y << "}";
}

std::vector<Coord> Neighbours(const Coord& c, const std::vector<bool>& empty) {
	std::vector<Coord> neighbours;
	neighbours.reserve(3);
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
	int width, height;
	cin >> width >> height;
	cin.ignore(256,'\n');

	if (cin.fail())
	{
		std::cerr << "input error." << std::endl;
		return cin;
	}

	Coord::MaxX = width;

	int size = width * height;
	state.empty.resize( size, false );

	for (int row = 0; row < height; ++row)
	{
		char line[1024];
		cin.getline(line, 1024);

		for (int col = 0; col < width; ++col)
		{
			int index = row * width + col;

			switch(line[col])
			{
				case '1':
					state.us = Coord( col, row );
					break;
				case '2':
					state.them = Coord( col, row );
					break;
				//case '#':
				case ' ':
					state.empty[index] = true;
					break;
				case '\0':
					std::cerr << "warning: early ending to row " << row << std::endl;

					break;
			}
		}

	}

	state.ourTurn = true;

	return cin;
}

std::ostream& operator<< (std::ostream& cout, const TronState& state) {
	int ourIndex = state.us.Index();
	int theirIndex = state.them.Index();
	for (int index = 0; index < state.empty.size(); ++index) {
		if (index == ourIndex) cout << 'U';
		else if (index == theirIndex) cout << 'T';
		else if (state.empty[index]) cout << ' ';
		else cout << '#';
		
		if ((index+1) % Coord::MaxX == 0) {
			cout << std::endl;
		}
	}
	cout << "us: " << state.us << std::endl;
	cout << "them: " << state.them << std::endl;
	return cout;
}
