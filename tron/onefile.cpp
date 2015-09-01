// tron.h

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
		ourTurn = !ourTurn;

		return oldCoord;
	}

	void UndoAction(action_type newCoord, action_type oldCoord) {
		ourTurn = !ourTurn;
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
	friend std::ostream& operator<< (std::ostream&, TronState&);

};

std::istream& operator>> (std::istream&, TronState&);
std::ostream& operator<< (std::ostream&, const TronState&);

// tron.cpp

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
	return cout;
}



// carlos.cpp

#include <vector>
#include <algorithm> // random_shuffle
#include <cmath>     // sqrt & log
#include <memory>
#include <cassert>
#include <limits>

namespace MCTS {


class Node {
	friend class Tree;

	typedef TronState::score_type score_t;
	typedef TronState::player_type player_t;
	typedef TronState::action_type action_t;

	typedef Node* NodePtr;
	typedef std::unique_ptr<Node> UniqueNodePtr;

private:
	NodePtr parent;

	player_t player; // who took the action to get to this state
	action_t action; // what action was taken to get to this state
	int visitCount;
	float sumValue; // TODO: refactor to score_t

	std::vector<UniqueNodePtr> children;

	std::vector<action_t> untriedActions;

public:
	Node (NodePtr parent, player_t player, action_t action, std::vector<action_t> untriedActions) :
		parent {parent},
		player {player},
		action {action},
		visitCount {0},
		sumValue {0},
		children {},
		untriedActions {untriedActions}
	{
		std::random_shuffle(untriedActions.begin(), untriedActions.end());
	}

	void Update (float value) {
		visitCount++;
		sumValue += value;
	}

	float EstimatedValue () {
		return sumValue / visitCount;
	}

	NodePtr SelectChild (float C = 1) {
		assert(children.size() > 0);
		
		NodePtr bestChild = nullptr;
		float bestValue = -std::numeric_limits<float>::infinity();
		for (UniqueNodePtr& unique_node_ptr : children) {
			Node& child = *unique_node_ptr;

			// NOTE: UCB1
			float value = child.EstimatedValue() + C * sqrt(2 * log(visitCount) / child.visitCount);
			if (value > bestValue) {
				bestValue = value;
				bestChild = unique_node_ptr.get();
			}
		}
		return bestChild;
	}

	bool IsFullyExpanded () {
		int numActions = untriedActions.size();
		return numActions == 0;
	}

	bool IsTerminal () {
		int numChildren = children.size();
		return numChildren == 0;
	}

	action_t RandomUntriedAction () {
		// NOTE: untriedActions is shuffled in constructor
		auto action = untriedActions.back();
		untriedActions.pop_back();
		return action;
	}

	NodePtr AddChild(action_t action, std::vector<action_t> nextActions) {
		UniqueNodePtr child {new Node(this, !player, action, move(nextActions))};
		NodePtr childRef = child.get();
		children.push_back(move(child));
		return childRef;
	}

};

class Tree {
	typedef TronState::action_type action_t;
	typedef Node* NodePtr;
	typedef action_t (*PolicyFn) (const TronState&);

private:
	TronState tronState;
	Node root;
	PolicyFn simulationPolicy;

public:
	explicit Tree (TronState game, PolicyFn simulationPolicy) :
		tronState {game},
		root {nullptr, false, TronState::NullAction, game.NextActions()},
		simulationPolicy {simulationPolicy}
	{}

	void Update () {

		NodePtr currentNode = &root;
		TronState currentState = tronState; // copy

		// selection
		while (currentNode->IsFullyExpanded() && !currentNode->IsTerminal()) {
			currentNode = currentNode->SelectChild();
			currentState.DoAction(currentNode->action);
		}

		// expansion
		if (!currentNode->IsFullyExpanded()) {
			action_t action = currentNode->RandomUntriedAction();
			currentState.DoAction(action);
			currentNode = currentNode->AddChild(action, move(currentState.NextActions()));
		}

		// simulation
		while (!currentState.GameOver()) {
			action_t action = simulationPolicy(currentState);
			currentState.DoAction(action);
		}

		// backpropagation
		while (currentNode != nullptr) {
			// TODO: cache result somehow?
			currentNode->Update(currentState.Result(currentNode->player));
			currentNode = currentNode->parent;
		}

	}

	action_t BestMove () {
	
		action_t bestMove = TronState::NullAction;
		int mostVisits = 0;

		std::cerr << "Move\tVisits\tEstimated" << std::endl;

		for (std::unique_ptr<Node>& unique_node_ptr : root.children) {
			Node& child = *unique_node_ptr;

			TronState currentState = tronState; // copy
			currentState.DoAction(child.action);
			int result = currentState.Result(root.player);

			std::cerr << child.action << "\t" << child.visitCount << "\t" << child.EstimatedValue() << std::endl;

			if (child.visitCount > mostVisits) {
				mostVisits = child.visitCount;
				bestMove = child.action;
			}
		}
		return bestMove;
	
	}

};


} // namespace MCTS


#include <random>

auto randomEngine = std::default_random_engine {};

TronState::action_type BasicPolicy (const TronState& currentState) {
	auto actions = currentState.NextActions();

	if (actions.empty()) return Coord::Invalid;

	int count = actions.size();
	int choice = 0;
	if (count > 1) {
		auto dist = std::uniform_int_distribution<> {0, count-1};
		choice = dist(randomEngine);
	}

	return actions[choice];
}

#include <iostream>
#include <chrono>
using namespace std::chrono;

int main () {
	auto t0 = high_resolution_clock::now();

	Coord::MaxX = 15;

	TronState game;
	std::cin >> game;

	auto searchTree = MCTS::Tree(game, BasicPolicy);

	auto t1 = high_resolution_clock::now();
	milliseconds elapsed = duration_cast<milliseconds>(t1-t0);
	milliseconds timeLimit = milliseconds(1500);
	while (elapsed < timeLimit) {
		searchTree.Update();
		t1 = high_resolution_clock::now();
		elapsed = duration_cast<milliseconds>(t1-t0);
	}

	Coord curr = game.us;
	Coord dest = searchTree.BestMove();

	if (dest.X > curr.X)      std::cout << "RIGHT";
	else if (dest.X < curr.X) std::cout << "LEFT";
	else if (dest.Y > curr.Y) std::cout << "DOWN";
	else if (dest.Y < curr.Y) std::cout << "UP";

	std::cout << std::endl;

	return 0;
}
