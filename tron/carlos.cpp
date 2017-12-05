#include <vector>
#include <algorithm> // random_shuffle
#include <cmath>     // sqrt & log
#include <memory>    // unique_ptr

namespace MCTS {

template <typename GameState> class Tree;

template <typename GameState>
class Node {
	friend class Tree<GameState>;

	typedef typename GameState::score_type score_t;
	typedef typename GameState::player_type player_t;
	typedef typename GameState::action_type action_t;

	typedef Node<GameState>* NodePtr;
	typedef std::unique_ptr<Node> UniqueNodePtr;

private:
	NodePtr parent;

	player_t player;
	action_t action;
	int visitCount;
	float sumValue; // TODO: refactor to score_t

	std::vector<UniqueNodePtr> children;

	std::vector<action_t> untriedActions;

public:
	Node (NodePtr parent, action_t action, std::vector<action_t> untriedActions) :
		parent {parent},
		player {},
		action {action},
		visitCount {0},
		sumValue {0},
		children {},
		untriedActions {untriedActions}
	{
		std::random_shuffle(untriedActions.begin(), untriedActions.end());
	}

	action_t BestMove () {
		action_t bestMove = GameState::NullAction;
		int mostVisits = 0;
		for (UniqueNodePtr& unique_node_ptr : children) {
			Node<GameState>& child = *unique_node_ptr;
			if (child.visitCount > mostVisits) {
				mostVisits = child.visitCount;
				bestMove = child.action;
			}
		}
		return bestMove;
	}

	void Update (float value) {
		visitCount++;
		sumValue += value;
	}

	float EstimatedValue () {
		return sumValue / visitCount;
	}

	NodePtr SelectChild (float C = 1) {
		NodePtr bestChild = nullptr;
		float bestValue = 0;
		for (UniqueNodePtr& unique_node_ptr : children) {
			Node<GameState>& child = *unique_node_ptr;

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
		UniqueNodePtr child {new Node(this, action, move(nextActions))};
		NodePtr childRef = child.get();
		children.push_back(move(child));
		return childRef;
	}

};

template <typename GameState>
class Tree {
	typedef typename GameState::action_type action_t;
	typedef Node<GameState>* NodePtr;
	typedef action_t (*PolicyFn) (const GameState&);

private:
	GameState gameState;
	Node<GameState> root;
	PolicyFn simulationPolicy;

public:
	explicit Tree (GameState game, PolicyFn simulationPolicy) :
		gameState {game},
		root {nullptr, GameState::NullAction, game.NextActions()},
		simulationPolicy {simulationPolicy}
	{}

	void Update () {

		NodePtr currentNode = &root;
		GameState currentState = gameState; // copy

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

		// cache results
		int ourResult = currentState.Result(true);
		int theirResult = Result::Win - ourResult; // zero sum

		// backpropagation
		while (currentNode != nullptr) {
			if (currentNode->player) {
				currentNode->Update(ourResult);
			} else {
				currentNode->Update(theirResult);
			}
			currentNode = currentNode->parent;
		}

	}

	action_t BestMove () {
		return root.BestMove();
	}

	void CommitMove (action_t action) {
		gameState.DoAction(action);
		NodePtr newRoot = root.FindChild(action); // move-assignment? (&&)
		delete root;    // free memory
		root = newRoot; // keep subtree
	}
};


} // namespace MCTS

#include "tron.hpp"

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

	bool gameover = false;
	while(!gameover)
	{
		TronState game;
		std::cin >> game;
		std::cerr << game;
		gameover = game.GameOver();

		if (gameover)
		{
			std::cerr << "gameover" << std::endl;
			break;
		}

		auto searchTree = MCTS::Tree<TronState>(game, BasicPolicy);

		auto t1 = high_resolution_clock::now();
		milliseconds elapsed = duration_cast<milliseconds>(t1-t0);
		milliseconds timeLimit = milliseconds(800);
		while (elapsed < timeLimit) {
			searchTree.Update();
			t1 = high_resolution_clock::now();
			elapsed = duration_cast<milliseconds>(t1-t0);
		}

		Coord curr = game.us;
		Coord dest = searchTree.BestMove();

		if (dest.X > curr.X)      std::cout << "2";
		else if (dest.X < curr.X) std::cout << "4";
		else if (dest.Y > curr.Y) std::cout << "3";
		else if (dest.Y < curr.Y) std::cout << "1";

		std::cout << std::endl;
	}

	std::cerr << "terminating" << std::endl;

	return 0;
}

/*
bool GameOver () {
	return true;
}

typedef int Action;

void PlayGame () {
	TronState game = TronState();
	auto searchTree = MCTS::Tree<TronState,Action>(game);
	while (!GameOver()) {
		Action ourMove = searchTree.FindMoveInTime(1.0f);
		searchTree.CommitMove(ourMove);
		Action theirMove; cin >> theirMove; // opponent's move
		searchTree.CommitMove(move);
	}
}
*/
