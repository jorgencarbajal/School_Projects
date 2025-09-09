/*
** A multi-player game of Blackjack with advanced data structures
**
** No splitting for doubles, just a simple add to 21
** Dealer hits when under 17 and below highest non-busted player
** Player 1 is human, others use a decision tree for hit/stand
** Incorporates sorting (hand display), hashing (card tracking),
** trees (AI decisions), and graphs (game state analysis)
**
*/

#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <list>
#include <map>
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <random>
#include <deque>
#include <set>
#include <stack>
#include <numeric>
#include <queue>
#include <vector>
#include <functional>

using namespace std;

// Forward declaration for DecisionTree and GameGraph
class DecisionTree;
class GameGraph;

// Cards class represents a single playing card
class Cards {
private:
    char suit; // Suit: H, D, C, S
    int value; // Value: 1-13 (Ace=1, Jack=11, Queen=12, King=13)
    static const map<int, string> valueNames; // Maps values to names (e.g., 1 -> "Ace")
    static const map<char, string> suitNames; // Maps suits to names (e.g., 'H' -> "Hearts")

public:
    Cards() : suit(' '), value(0) {}
    Cards(int v, char s) : value(v), suit(s) {}

    // Display the card in a human-readable format
    void display() const {
        if (valueNames.find(value) != valueNames.end()) {
            cout << valueNames.at(value);
        } else {
            cout << value;
        }
        cout << " of ";
        auto it = suitNames.find(suit);
        if (it != suitNames.end())
            cout << it->second;
        else
            cout << "Unknown";
    }

    // Get the Blackjack value of the card
    int getValue() const {
        if (value > 10) return 10; // Face cards worth 10
        return value;
    }

    // Comparison for sorting or set operations
    bool operator<(const Cards& other) const {
        return (suit < other.suit) || (suit == other.suit && value < other.value);
    }

    // Equality for hashing
    bool operator==(const Cards& other) const {
        return suit == other.suit && value == other.value;
    }

    // Getters for suit and value
    char getSuit() const { return suit; }
    int getRawValue() const { return value; }
};

// Static member definitions
const map<int, string> Cards::valueNames = {
    {1, "Ace"},
    {11, "Jack"},
    {12, "Queen"},
    {13, "King"}
};

const map<char, string> Cards::suitNames = {
    {'H', "Hearts"},
    {'D', "Diamonds"},
    {'C', "Clubs"},
    {'S', "Spades"}
};

// Custom hash function for Cards
namespace std {
    template <>
    struct hash<Cards> {
        size_t operator()(const Cards& card) const {
            return hash<char>()(card.getSuit()) ^ (hash<int>()(card.getRawValue()) << 1);
        }
    };
}

// Deck class manages the card deck
class Deck {
private:
    deque<Cards> cards; // Main deck
    unordered_map<size_t, Cards> dealtCards; // Hashed dealt cards
    stack<Cards> cardsStack; // Alternative deck representation
    list<Cards> backupDeck; // Backup for reshuffling
    int top; // Tracks dealt cards

public:
    Deck() : top(0) {
        char suit[4] = {'H', 'D', 'C', 'S'};
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= 13; j++) {
                cards.push_back(Cards(j, suit[i]));
            }
        }
        copy(cards.begin(), cards.end(), back_inserter(backupDeck));
        shuffle();
    }

    // Shuffle the deck using Mersenne Twister
    void shuffle() {
        random_device rd;
        mt19937 rng(rd());
        std::shuffle(cards.begin(), cards.end(), rng);
        top = 0;
        while (!cardsStack.empty()) {
            cardsStack.pop();
        }
        for (const auto& card : cards) {
            cardsStack.push(card);
        }
        dealtCards.clear();
    }

    // Deal a card and track it
    Cards deal() {
        if (top >= cards.size() || cardsStack.empty()) {
            cards.clear();
            copy(backupDeck.begin(), backupDeck.end(), back_inserter(cards));
            shuffle();
        }
        Cards card = cardsStack.top();
        cardsStack.pop();
        dealtCards[hash<Cards>()(card)] = card;
        top++;
        return card;
    }

    // Check if a card has been dealt
    bool isCardDealt(const Cards& card) const {
        return dealtCards.find(hash<Cards>()(card)) != dealtCards.end();
    }
};

// Hand class represents a player's or dealer's hand
class Hand {
private:
    list<Cards> cards; // List of cards in hand

public:
    Hand() {}

    // Add a card to the hand
    void addCard(Cards card) {
        if (cards.size() < 10) {
            cards.push_back(card);
        }
    }

    // Calculate the hand's Blackjack value
    int getValue() const {
        int value = accumulate(cards.begin(), cards.end(), 0, [](int sum, const Cards& card) {
            return sum + (card.getValue() == 1 ? 11 : card.getValue());
        });
        int ace = 0;
        auto it = cards.begin();
        while (it != cards.end()) {
            it = find_if(it, cards.end(), [](const Cards& card) { return (card.getValue() == 1); });
            if (it != cards.end()) {
                ace++;
                ++it;
            }
        }
        while (value > 21 && ace > 0) {
            value -= 10;
            ace--;
        }
        return value;
    }

    // Display hand, sorting cards by value then suit
    void display(bool hideFirst = false) {
        vector<Cards> sortedCards(cards.begin(), cards.end());
        sort(sortedCards.begin(), sortedCards.end(), [](const Cards& a, const Cards& b) {
            if (a.getRawValue() != b.getRawValue())
                return a.getRawValue() < b.getRawValue();
            return a.getSuit() < b.getSuit();
        });

        bool val = hideFirst;
        for (const auto& card : sortedCards) {
            if (val) {
                cout << "[Hidden]";
                val = false;
            } else {
                card.display();
            }
            cout << "... ";
        }
    }

    // Check if hand is bust
    bool isBust() const {
        return getValue() > 21;
    }

    // Check if hand is Blackjack
    bool isBlackjack() const {
        return getValue() == 21;
    }

    // Clear the hand
    void clear() {
        cards.clear();
    }

    // Get copy of cards for decision tree
    list<Cards> getCards() const {
        return cards;
    }
};

// DecisionTree class for computer player decisions
class DecisionTree {
private:
    struct Node {
        bool isLeaf;
        bool hit; // Decision if leaf
        int minValue; // Hand value range
        int maxValue;
        int dealerUpCard; // Dealer's visible card value
        Node* left; // Lower value branch
        Node* right; // Higher value branch

        Node(int minV, int maxV, int dealerUp = 0)
            : isLeaf(false), hit(false), minValue(minV), maxValue(maxV),
              dealerUpCard(dealerUp), left(nullptr), right(nullptr) {}
        ~Node() {
            delete left;
            delete right;
        }
    };

    Node* root;

    // Build decision tree
    void buildTree() {
        root = new Node(4, 21);
        root->left = new Node(4, 11); // Soft hands or low value
        root->right = new Node(12, 21); // Hard hands

        // Low value hands
        root->left->isLeaf = true;
        root->left->hit = true; // Always hit below 12

        // Hard hands
        root->right->left = new Node(12, 16);
        root->right->right = new Node(17, 21);

        // 12-16: Consider dealer's up card
        root->right->left->left = new Node(12, 16, 2); // Dealer 2-6
        root->right->left->right = new Node(12, 16, 7); // Dealer 7-A
        root->right->left->left->isLeaf = true;
        root->right->left->left->hit = false; // Stand vs. weak dealer
        root->right->left->right->isLeaf = true;
        root->right->left->right->hit = true; // Hit vs. strong dealer

        // 17-21: Always stand
        root->right->right->isLeaf = true;
        root->right->right->hit = false;
    }

    // Traverse tree to make decision
    bool decideHit(Node* node, int handValue, int dealerUpCard) const {
        if (node->isLeaf) {
            return node->hit;
        }
        if (handValue <= node->left->maxValue) {
            return decideHit(node->left, handValue, dealerUpCard);
        }
        if (node->left->dealerUpCard == 0 || (dealerUpCard >= 2 && dealerUpCard <= 6)) {
            return decideHit(node->left, handValue, dealerUpCard);
        }
        return decideHit(node->right, handValue, dealerUpCard);
    }

public:
    DecisionTree() : root(nullptr) {
        buildTree();
    }

    ~DecisionTree() {
        delete root;
    }

    // Decide whether to hit based on hand and dealer's up card
    bool shouldHit(int handValue, int dealerUpCard) const {
        if (handValue > 21) return false;
        return decideHit(root, handValue, dealerUpCard);
    }
};

// GameGraph class to track game states
class GameGraph {
private:
    struct Node {
        int roundNumber;
        map<string, int> playerValues; // Hand values
        int dealerValue;
        map<string, bool> outcomes; // Win/loss per player
        vector<Node*> neighbors; // Next states

        Node(int round, const map<string, int>& pv, int dv)
            : roundNumber(round), playerValues(pv), dealerValue(dv) {}
    };

    vector<Node*> nodes;
    int currentRound;

public:
    GameGraph() : currentRound(0) {}

    ~GameGraph() {
        for (auto node : nodes) {
            delete node;
        }
    }

    // Add a game state
    void addState(const vector<Hand>& players, const Hand& dealer, const map<string, int>& stats) {
        map<string, int> playerValues;
        for (size_t i = 0; i < players.size(); i++) {
            playerValues["Player" + to_string(i + 1)] = players[i].getValue();
        }
        Node* newNode = new Node(currentRound, playerValues, dealer.getValue());

        // Record outcomes based on stats changes
        if (!nodes.empty()) {
            Node* prev = nodes.back();
            for (const auto& stat : stats) {
                if (stat.first != "Tie" && stat.first != "Dealer") {
                    newNode->outcomes[stat.first] = stat.second > (prev->outcomes.count(stat.first) ? prev->outcomes[stat.first] : 0);
                }
            }
            prev->neighbors.push_back(newNode);
        }

        nodes.push_back(newNode);
        currentRound++;
    }

    // Print graph summary
    void printSummary() const {
        cout << "Game Graph Summary:" << endl;
        for (const auto& node : nodes) {
            cout << "Round " << node->roundNumber << ": ";
            for (const auto& pv : node->playerValues) {
                cout << pv.first << "=" << pv.second << " ";
            }
            cout << "Dealer=" << node->dealerValue << endl;
            cout << "Outcomes: ";
            for (const auto& outcome : node->outcomes) {
                cout << outcome.first << (outcome.second ? " won" : " lost") << " ";
            }
            cout << endl;
        }
    }
};

// Blackjack class manages the game
class Blackjack {
private:
    Deck deck;
    vector<Hand> players;
    Hand dealer;
    map<string, int> stats;
    int numPlayers;
    DecisionTree ai;
    GameGraph gameGraph;

public:
    Blackjack(int num) : numPlayers(num) {
        srand(static_cast<unsigned>(time(0)));
        deck.shuffle();
        players.resize(numPlayers);
        stats["Dealer"] = 0;
        stats["Tie"] = 0;
        for (int i = 0; i < numPlayers; i++) {
            stats["Player" + to_string(i + 1)] = 0;
        }
    }

    void play() {
        char playAgain;
        do {
            cout << endl;
            for (auto& player : players) {
                player.clear();
            }
            dealer.clear();

            // Deal initial cards
            for (int round = 0; round < 2; round++) {
                for (int i = 0; i < numPlayers; i++) {
                    players[i].addCard(deck.deal());
                }
                dealer.addCard(deck.deal());
            }

            // Display all hands
            bool val = true;
            cout << "Dealer's hand: ";
            dealer.display(val);
            cout << endl;
            for (int i = 0; i < numPlayers; i++) {
                cout << "Player " << (i + 1) << "'s hand: ";
                players[i].display(false);
                cout << endl;
            }

            // Process Player 1 (human) turn
            if (!players[0].isBlackjack()) {
                char choice;
                do {
                    cout << "Player 1, Hit or Stand? (h/s) ";
                    cin >> choice;
                    if (choice == 's' || choice == 'S') {
                        cout << "Player 1's total card value is: " << players[0].getValue() << endl;
                        break;
                    }
                    if (choice == 'h' || choice == 'H') {
                        players[0].addCard(deck.deal());
                        cout << "Player 1 adds: ";
                        players[0].display(false);
                        cout << endl;
                        if (players[0].isBust()) {
                            cout << "Player 1 busts!" << endl;
                            break;
                        } else if (players[0].isBlackjack()) {
                            cout << "Player 1 has Blackjack!" << endl;
                            break;
                        }
                    }
                } while ((choice == 'h' || choice == 'H') && !players[0].isBust());
            } else {
                cout << "Player 1 has Blackjack!" << endl;
            }

            // Process computer players' turns
            int dealerUpCard = dealer.getCards().front().getValue();
            for (int i = 1; i < numPlayers; i++) {
                if (!players[i].isBlackjack()) {
                    while (ai.shouldHit(players[i].getValue(), dealerUpCard) && !players[i].isBust()) {
                        players[i].addCard(deck.deal());
                        cout << "Player " << (i + 1) << " hits: ";
                        players[i].display(false);
                        cout << endl;
                        if (players[i].isBust()) {
                            cout << "Player " << (i + 1) << " busts!" << endl;
                            break;
                        } else if (players[i].isBlackjack()) {
                            cout << "Player " << (i + 1) << " has Blackjack!" << endl;
                            break;
                        }
                    }
                    if (!players[i].isBust() && !players[i].isBlackjack()) {
                        cout << "Player " << (i + 1) << " stands with: " << players[i].getValue() << endl;
                    }
                } else {
                    cout << "Player " << (i + 1) << " has Blackjack!" << endl;
                }
            }

            // Process dealer's turn
            cout << endl << "Dealer's hand: ";
            dealer.display(false);
            cout << endl;
            bool anyPlayerActive = false;
            int maxPlayerValue = 0;
            for (const auto& player : players) {
                if (!player.isBust()) {
                    anyPlayerActive = true;
                    maxPlayerValue = max(maxPlayerValue, player.getValue());
                }
            }
            if (anyPlayerActive) {
                while (dealer.getValue() < 17 && dealer.getValue() <= maxPlayerValue) {
                    dealer.addCard(deck.deal());
                    cout << "Dealer hits: ";
                    dealer.display(false);
                    cout << endl;
                }
                if (dealer.isBust()) {
                    cout << "Dealer busts!" << endl;
                } else if (dealer.isBlackjack()) {
                    cout << "Dealer has Blackjack!" << endl;
                }
            }

            // Determine outcomes
            for (int i = 0; i < numPlayers; i++) {
                if (players[i].isBust()) {
                    cout << "Player " << (i + 1) << " busted. Dealer wins!" << endl;
                    stats["Dealer"]++;
                } else if (dealer.isBust()) {
                    cout << "Dealer busts! Player " << (i + 1) << " wins!" << endl;
                    stats["Player" + to_string(i + 1)]++;
                } else if (players[i].isBlackjack() && !dealer.isBlackjack()) {
                    cout << "Player " << (i + 1) << " has Blackjack! You win!" << endl;
                    stats["Player" + to_string(i + 1)]++;
                } else if (dealer.isBlackjack() && !players[i].isBlackjack()) {
                    cout << "Dealer has Blackjack! Player " << (i + 1) << " loses!" << endl;
                    stats["Dealer"]++;
                } else if (players[i].getValue() > dealer.getValue()) {
                    cout << "Player " << (i + 1) << " wins!" << endl;
                    stats["Player" + to_string(i + 1)]++;
                } else if (players[i].getValue() < dealer.getValue()) {
                    cout << "Player " << (i + 1) << " loses!" << endl;
                    stats["Dealer"]++;
                } else {
                    cout << "Player " << (i + 1) << " pushes! It's a tie." << endl;
                    stats["Tie"]++;
                }
            }

            // Record game state in graph
            gameGraph.addState(players, dealer, stats);

            // Display game statistics
            cout << endl << "Game Statistics:" << endl;
            for (auto it = stats.begin(); it != stats.end(); ++it) {
                cout << it->first << ": " << it->second << " wins" << endl;
            }
            auto maxWins = max_element(stats.begin(), stats.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });
            cout << "Leading: " << maxWins->first << " with " << maxWins->second << " wins" << endl;

            // Print game graph summary
            gameGraph.printSummary();

            cout << endl;
            cout << "Play again? (y/n): ";
            cin >> playAgain;
        } while (playAgain == 'Y' || playAgain == 'y');
    }
};

int main(int argc, char** argv) {
    cout << "Welcome to the Blackjack Casino!" << endl;
    cout << "There are 4 available tables:" << endl;
    cout << "Table 1 (3 players), Table 2 (1 player), Table 3 (5 players), Table 4 (2 players)" << endl;
    cout << "Enter a game for:" << endl;
    int tableChoice;
    cin >> tableChoice;
    int numPlayers;

    switch (tableChoice) {
        case 1:
            numPlayers = 3;
            break;
        case 2:
            numPlayers = 1;
            break;
        case 3:
            numPlayers = 5;
            break;
        case 4:
            numPlayers = 2;
            break;
        default:
            cout << "Invalid choice. Defaulting to Table 2 (1 player)." << endl;
            numPlayers = 1;
            break;
    }

    Blackjack game(numPlayers);
    game.play();

    return 0;
}
