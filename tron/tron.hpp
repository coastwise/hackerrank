struct Coord {
	int X, Y;

	static int MaxX;

	Coord (int x, int y) : X{x}, Y{y} {}

	int Index () const {
		int result = MaxX*Y + X;
		return result;
	}

	Coord& operator+= (const Coord& a) {
		X += a.X;
		Y += a.Y;
		return *this;
	}

	bool operator== (const Coord&);

	Coord operator+ (Coord) const;

	static const Coord Invalid;
	static const Coord Directions [4];

};

#include <iostream>
std::ostream& operator<< (std::ostream&, const Coord&);


#include <vector>
#include <bitset>

std::vector<Coord> Neighbours(const Coord& c, const std::bitset<15*15>& empty);

class TronState {
	
	std::bitset<15*15> empty;
	Coord us;
	Coord them;

	bool ourTurn;

public:
	using score_type = int;
	using player_type = bool;
	using action_type = Coord;

	static const Coord NullAction;

	TronState (std::bitset<15*15> empty, Coord us, Coord them) :
		empty {empty},
		us {us},
		them {them},
		ourTurn {true}
	{}

	std::vector<action_type> NextActions () const {
		std::vector<action_type> actions;
		if (ourTurn) {
			actions = Neighbours(us, empty);
		} else {
			actions = Neighbours(them, empty);
		}
		return actions;
	}

	void DoAction(action_type a) {
		empty[a.Index()] = false;
		ourTurn = ~ourTurn;
	}

	bool GameOver () const {
		if (empty.none()) {
			return true;
		}

		if ( Neighbours(us, empty).empty() || Neighbours(them, empty).empty() ) {
			return true;
		}

		return false;
	}

	score_type Result (player_type player) const {
		return 0;
	}

};
