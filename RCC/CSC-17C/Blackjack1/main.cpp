/*
** A one v one game of Black Jack
**
** No splitting for doubles just a simple add to 21
**
** Dealer will always add when under 21 and below the player
** in value.
**
**/

#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <list>
#include <map>
#include <iterator>
#include <algorithm>
#include <random>
#include <deque>
#include <set>
#include <stack>
#include <numeric>
#include <queue>

using namespace std;

// Each of the 52 card objects is initialized using the default constructor, which
// initializes every card value to "0" and " " character
class Cards {
    private:
        char suit;
        int value;
        
        // A static map to share card values across all card objects
        static const map<int, string> valueNames;
        static const map<char, string> suitNames;

    public:
        Cards() : suit(' '), value(0){}
        Cards(int v, char s) : value(v), suit(s) {}

    void display() const {

        // The find function is a member function of the <map> class
        // It searches the map for a key and returns an iterator to the key value pair
        // valueNames is the map that it searches which only contains the 1, 11, 12, 13
        // keys. If the value is in there the first statment iterator comparison would be
        // false printing the following line
        if (valueNames.find(value) != valueNames.end()) {
            std::cout << valueNames.at(value);
        } else {
            std::cout << value; // Print numeric value for 2-10
        }

        cout << " of ";

        auto it = suitNames.find(suit);
        // same as: map<char, string>::const_iterator it = suitNames.find(suit);
        if(it != suitNames.end())
            cout << it->second;
        else
            cout << "Unknown";
    }

    // Each card object has corresponding private member "value"
    int getValue() const {
        if (value > 10) return 10;
        return value;
    }

    // Required for std::set to compare Cards objects
    // Takes in the Cards object as a constant reference
    // This will compar the object with the sets root node and organize it into
    // the set accordingly
    // In simple terms, takes in the card object and compares it to the set
    // if it is not a duplicate store it accordingly
    bool operator<(const Cards& other) const {
        // Ensuring there are no duplicates
        return (suit < other.suit) || (suit == other.suit && value < other.value);
    }

};

// Defines the static map, 
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

// Before the constructor of the Deck class is called the private member
// "cards" is initialzed, it is an object array of Cards
class Deck{
    private:
        // cards is a container of the deque type
        deque<Cards> cards;
        set<Cards> dealtCards;
        // Alternative deck
        stack<Cards> cardsStack;
        // Backup deck
        list<Cards> backupDeck;
        int top;

    public:
        // After cards has been initialize we mover over to the Deck constructor which sets top
        // to zero and fills the private member "cards", which is an object array of the type Class,
        // with actual card values
        Deck() : top(0) {

            char suit[4] = {'H', 'D', 'C', 'S'};

            // Implementing the list, we go through all the card options and pushes
            // the Cards object to the back of the list
            for(int i = 0; i < 4; i++){
                for(int j = 1; j <= 13; j++){
                    cards.push_back(Cards(j, suit[i]));
                }
            }
            // Copy to backup deck before shuffling
            std::copy(cards.begin(), cards.end(), std::back_inserter(backupDeck));
            // everytime the deck is initialized, in this case the deque deck,
            // the shuffle function is called
            shuffle();
        }

        // "rng" is a variable holding a mt19937 object
        // "mt19937" is a reliable algoritm for producing random sequences
        // "random_device" gives a random starting seed to makes rng's output unique
        void shuffle() {
            // creates an object of the type "random_device" called rd
            // the random number from rd are used as a seed to initialize the random
            // number generator
            // "rd()" returns an unsigned int
            random_device rd;
            // rng is an object of the type mt19937 and it is initialize with the seed
            // mt19937 is a random number generator based on the Mersenne Twister algo
            // It produces a sequence of random numbers based on the initial seed
            mt19937 rng(rd());
            // std:: is needed to distinguish from the method and the shuffle method from
            // the <algo... header
            std::shuffle(cards.begin(), cards.end(), rng);
            // clears the contents of the set after shuffling
            top = 0;

            // Clear the cardsStack deck
            while(!cardsStack.empty()){
                cardsStack.pop();
            }
            // copy all the cards from the deque list to this stack
            for(const auto& card : cards){
                cardsStack.push(card);
            }
            // Clear all the cards in the set
            dealtCards.clear();
            // At this point you have two deck with a similar shuffle pattern
        }

        Cards deal(){
            if (top >= cards.size() || cardsStack.empty()){
                // Restore from backup deck
                cards.clear();
                std::copy(backupDeck.begin(), backupDeck.end(), std::back_inserter(cards));
                shuffle();
            }

            // The stack deck
            Cards card = cardsStack.top();
            cardsStack.pop();
            dealtCards.insert(card);
            top++;
            return card;
        }

};

// Each Hand object is a players (or dealers) hand and they can hold up to 10 cards
class Hand {
    private:
        list<Cards> cards;
        //Cards cards[10];

    public:

        // After the "cards" private member has been initialized, the default constructor sets
        // count to zero
        Hand() {}

        // Puts the card into the cards array
        void addCard(Cards card){
            if(cards.size() < 10){
                cards.push_back(card);
            }
        }

        // Here we are adding the value of the cards, and returning that value
        int getValue() const {

            // Accumulator algorithm from the <numeric> header
            // Returns an integer, takes start and end iterators to define the card range
            // Third argument is the initial sum value (0 in this case)
            // The last argument is a custom binary operation (lambda) replacing default addition
            // The lambda defines custom logic to sum card values, treating Aces as 11
            // [](int sum, const Cards& card) is a lambda taking two parameters:
            // - int sum: the current accumulated sum, starting at 0
            // - const Cards& card: a const reference to the current Cards object
            // Returns the sum plus the card's value (11 for Aces, otherwise getValue())
            int value = accumulate(cards.begin(), cards.end(), 0, [](int sum, const Cards& card) {
                return sum + (card.getValue() == 1 ? 11 : card.getValue());});

            //int value = 0;
            // Keep track of aces incase the conversion from 1 and 11 are necessary

            // Implement an algorithm find_if
            int ace = 0;
            auto it = cards.begin();
            while (it != cards.end())
            {
                // The lambda function takes a const Cards& and returns true if the card is an Ace
                // (getValue() == 1), false otherwise, for use in find_if to locate Aces
                it = find_if(it, cards.end(), [](const Cards& card){return (card.getValue() == 1);});
                if(it != cards.end()){
                    ace++;
                    ++it;
                }
            }
            
            // Old way of finding aces
            //for(const auto& card : cards){
            //    if(card.getValue() == 1){
            //        ace++;
            //    }
            //}

            // Old accumulation method with an iterator
            //list<Cards>::const_iterator pos = cards.begin();
            //
            //for(pos; pos != cards.end(); pos++){
            //    // cards[].getValue() returns the value of the indexed card.
            //    int cardValue = pos->getValue();
            //
            //    if (cardValue == 1){
            //        ace++;
            //        value += 11;
            //    }
            //    else {
            //        value += cardValue;
            //    }
            //
            //}

            // Correct the ace conversion if necessary
            while(value > 21 && ace > 0){
                value -= 10;
                ace--;
            }

            return value;
        }

        void display(bool val){  
            // Use std::for_each with a lambda to display cards, handling the hidden card flag
            std::for_each(cards.begin(), cards.end(), [&val](const Cards& card) {
                if(val == true){
                    cout << "[Hidden]";
                    val = false;
                }
                else {
                    card.display();
                }
                cout << "... ";
            });
        }

        bool isBust() const {
            // If your hand value is greater than 21 return true
            return getValue() > 21;
        }

        bool isBlackjack() const {
            // Maybe can remove the first argument?
            return (getValue() == 21);
        }

        void clear(){
            cards.clear();
        }

};

// The Blackjack class is called and all the private members are initialized
class Blackjack {
    private:
        Deck deck;
        Hand player;
        Hand dealer;
        map<string, int> stats; // Track game statistics

    public:
        // The constructor to the Blackjack class seeds the time and shuffle the deck
        Blackjack() {
            srand(static_cast<unsigned>(time(0)));
            deck.shuffle();
            // Initialize stats
            stats["Player"] = 0;
            stats["Dealer"] = 0;
            stats["Tie"] = 0;
        }

        void play(){
            char playAgain;
            queue<Hand*> turnOrder; // Queue to manage turn order
            
            do
            {
                cout << endl;
                // Clears hands for new games, really just resets count in the Hand class so when cards are
                // drawn in order from the private cards array
                player.clear();
                dealer.clear();

                // First start by drawing cards, player, dealer, player, dealer
                // Use std::for_each to deal initial cards
                list<Hand*> hands = {&player, &dealer, &player, &dealer};
                std::for_each(hands.begin(), hands.end(), [this](Hand* hand) {
                    hand->addCard(deck.deal());
                });

                // Initialize turn order
                turnOrder.push(&player);
                turnOrder.push(&dealer);

                bool val = true;
                cout << "The dealers hand: ";
                dealer.display(val);
                val = false;
                cout << endl;

                cout << "Your hand: ";
                player.display(val);

                // Process player's turn
                if(!player.isBlackjack()){
                    char choice;
                    do
                    {
                        cout << endl;
                        cout << "Hit or Stand? (h/s) ";
                        cin >> choice;

                        if (choice == 's' || choice == 'S'){
                            cout << "Awesome your total card value is: " << player.getValue();
                            cout << endl;
                        }

                        if (choice  == 'h' || choice == 'H'){
                            player.addCard(deck.deal());
                            cout << "You add: ";
                            player.display(val);
                            if (player.isBust()){
                                cout << endl;
                                cout << "Bust, You lost!! ";
                                break;
                            }
                            else if (player.isBlackjack()){
                                break;
                            }
                        }

                    } while ((choice == 'h' || choice == 'H') && !player.isBust());
                }

                // Process dealer's turn if player hasn't busted
                if (!player.isBust()) {
                    cout << endl;
                    cout << "Dealers hand: ";
                    dealer.display(val);
                    while(dealer.getValue() < player.getValue() || dealer.getValue() < 17){
                        dealer.addCard(deck.deal());
                        cout << endl;
                        cout << "Dealer adds: ";
                        dealer.display(val);
                    }

                    // Determine the winner
                    if (dealer.isBust()) {
                        cout << endl;
                        cout << "Dealer busts! You win!\n";
                        stats["Player"]++;
                    } else if (player.isBlackjack() && !dealer.isBlackjack()) {
                        cout << endl;
                        cout << "Blackjack! You win!\n";
                        stats["Player"]++;
                    } else if (dealer.isBlackjack() && !player.isBlackjack()) {
                        cout << endl;
                        cout << "Dealer has Blackjack! You lose!\n";
                        stats["Dealer"]++;
                    } else if (player.getValue() > dealer.getValue()) {
                        cout << endl;
                        cout << "You win!\n";
                        stats["Player"]++;
                    } else if (player.getValue() < dealer.getValue()) {
                        cout << endl;
                        cout << "You lose!\n";
                        stats["Dealer"]++;
                    } else {
                        cout << endl;
                        cout << "Push! Its a tie\n";
                        stats["Tie"]++;
                    }
                } else {
                    stats["Dealer"]++;
                }

                // Display game statistics
                cout << endl << "Game Statistics:" << endl;
                for (auto it = stats.begin(); it != stats.end(); ++it) {
                    cout << it->first << ": " << it->second << " wins" << endl;
                }
                // Find the leading player
                auto maxWins = std::max_element(stats.begin(), stats.end(), 
                    [](const auto& a, const auto& b) { return a.second < b.second; });
                cout << "Leading: " << maxWins->first << " with " << maxWins->second << " wins" << endl;

                cout << endl;
                cout << "Play again? (y/n): ";
                cin >> playAgain;

            } while (playAgain == 'Y' || playAgain == 'y');
        }

};

int main(int argc, char** argv) {
    // Creating the game of black jack, create all the objects and shuffle the deck
    Blackjack game;

    // Start dealing cards
    game.play();

    return 0;
}
