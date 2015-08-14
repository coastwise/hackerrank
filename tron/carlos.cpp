#include <vector>

namespace MCTS {

template <typename GameState> class Tree;

template <typename GameState>
class Node {
	friend class Tree<GameState>;

	typedef typename GameState::score_type score_t;
	typedef typename GameState::player_type player_t;
	typedef typename GameState::action_type action_t;

private:
	Node* parent;

	player_t player;
	action_t action;
	int visitCount;
	float sumValue; // TODO: refactor to score_t

	std::vector<Node> children;

	std::vector<action_t> untriedActions;

public:
	Node (Node* parent, action_t action, std::vector<action_t> untriedActions) : 
		parent{parent},
		action{action},
		untriedActions{untriedActions}
	{}
	
	action_t BestMove () {
		action_t bestMove;
		int mostVisits = 0;
		for (auto node = children.begin(); node != children.end(); ++node) {
			if (node->visitCount > mostVisits) {
				mostVisits = node->visitCount;
				bestMove = node->action;
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

	Node<GameState>* SelectChild () {
		// TODO: actually use UCB
		return &*children.begin();
	}

	bool IsFullyExpanded () {
		return false;
	}

	bool IsTerminal () {
		return true;
	}

	action_t RandomUntriedAction () {
		return action;
	}

	Node<GameState>* AddChild(action_t action, std::vector<action_t> nextActions) {
		children.emplace_back(this, action, move(nextActions));
		return &children.back();
	}

};

template <typename GameState>
class Tree {
	typedef typename GameState::action_type action_t;
	typedef Node<GameState>* NodePtr;

private:
	GameState gameState;
	Node<GameState> root;

public:
	explicit Tree (GameState game) : gameState{game}, root{nullptr,0,game.NextActions()} {}

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
			currentState.PlayBasicPolicy();
		}

		// backpropagation
		while (currentNode != nullptr) {
			currentNode->Update(currentState.Result(currentNode->player));
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

#include <bitset>
class TronState {
	std::bitset<15*15> bits;

public:
	using score_type = int;
	using player_type = int;
	using action_type = int;

	std::vector<action_type> NextActions () {
		std::vector<action_type> actions;
		// TODO: actually generate possible actions
		return actions;
	}

	void DoAction(int a) {

	}

	bool GameOver () {
		return true;
	}

	void PlayBasicPolicy () {

	}

	score_type Result (player_type player) {
		return 0;
	}
};

#include <iostream>
#include <chrono>
using namespace std::chrono;

void FindMoveInTime (float timeLimit) {
	auto t0 = high_resolution_clock::now();

	TronState game = TronState();

	auto searchTree = MCTS::Tree<TronState>(game);

	auto t1 = high_resolution_clock::now();
	auto elapsed = duration_cast<milliseconds>(t1-t0).count();
	while (elapsed < timeLimit) {
		searchTree.Update();
		t1 = high_resolution_clock::now();
		elapsed = duration_cast<milliseconds>(t1-t0).count();
	}

	std::cout << searchTree.BestMove();
}

int main () {
	FindMoveInTime(1000);
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
