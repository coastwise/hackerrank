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
std::istream& operator>> (std::istream&, Coord&);


#include <vector>
#include <bitset>

std::vector<Coord> Neighbours(const Coord& c, const std::bitset<15*15>& empty);

class TronState {
public: // TODO: make us, them, and empty private again
	std::bitset<15*15> empty;
	Coord us;
	Coord them;

	bool ourTurn;

public:
	using score_type = int;
	using player_type = bool;
	using action_type = Coord;

	static const Coord NullAction;

	TronState () :
		us {Coord::Invalid},
		them {Coord::Invalid}
	{}

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

	action_type DoAction(action_type newCoord) {
		action_type oldCoord = Coord::Invalid;
		if (ourTurn) {
			oldCoord = us;
			us = newCoord;
		} else {
			oldCoord = them;
			them = newCoord;
		}

		empty[newCoord.Index()] = false;
		ourTurn = ~ourTurn;

		return oldCoord;
	}

	void UndoAction(action_type newCoord, action_type oldCoord) {
		ourTurn = ~ourTurn;
		empty[newCoord.Index()] = true;
		if (ourTurn) us = oldCoord;
		else them = oldCoord;
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

	score_type Result () const {
		return Result(ourTurn);
	}

	friend std::istream& operator>> (std::istream&, TronState&);	

};

std::istream& operator>> (std::istream&, TronState&);
