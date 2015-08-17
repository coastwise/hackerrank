enum Result { Loss, Tie, Win }; // 0, 1, 2 = zero-sum

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
		empty {},
		us {Coord::Invalid},
		them {Coord::Invalid},
		ourTurn {false}
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

	score_type Result (bool ourPerspective) const {
		auto ourActions = Neighbours(us, empty);
		auto theirActions = Neighbours(them, empty);

		enum Result result = Tie;

		if (ourActions.empty() && theirActions.empty()) {
			result = Tie;
		} else if (ourActions.empty()) {
			result = ourPerspective ? Loss : Win;
		} else if (theirActions.empty()) {
			result = ourPerspective ? Win : Loss;
		} else {
			// Not a game over...
			result = Tie;
		}

		return result;
	}

	score_type Result () const {
		return Result(ourTurn);
	}

	friend std::istream& operator>> (std::istream&, TronState&);

	void PrintMap () {
		for (int index = 0; index < empty.size(); ++index) {
			std::cout << (empty[index] ? ' ' : '#');
			if ((index+1) % Coord::MaxX == 0) {
				std::cout << std::endl;
			}
		}
	}

};

std::istream& operator>> (std::istream&, TronState&);
