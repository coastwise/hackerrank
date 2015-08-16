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

	bool operator== (const Coord&);

	Coord operator+ (Coord) const;

	static const Coord Invalid;
	static const Coord Directions [4];

};

#include <iostream>
std::ostream& operator<< (std::ostream&, const Coord&);


#include <vector>
#include <bitset>
#include <random>

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

	std::vector<action_type> NextActions () {
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

	bool GameOver () {
		if (empty.none()) {
			return true;
		}

		if ( Neighbours(us, empty).empty() || Neighbours(them, empty).empty() ) {
			return true;
		}

		return false;
	}

	void PlayBasicPolicy () {
		auto actions = NextActions();

		if (actions.empty()) return;

		int count = actions.size();
		int choice = 0;
		if (count > 1) {
			auto dist = std::uniform_int_distribution<> {0, count-1};
			// TODO: don't create a new engine for every call
			auto engine = std::default_random_engine {};
			choice = dist(engine);
		}

		DoAction(actions[choice]);
	}

	score_type Result (player_type player) {
		return 0;
	}
};
