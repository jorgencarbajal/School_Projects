/*
    Dr. Mark E. Lehr
    CSC 7 Template for Mastermind AI
    May 11th, 2021
 */

//System Libraries
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iomanip>
using namespace std;

//Function Prototypes
string AI(char,char);
bool eval(string,string,char &,char &);
string set();

int main(int argc, char** argv) {
    //Set the random number seed
    srand(static_cast<unsigned int>(time(0)));
    
    //Declare variables
    string code,guess;  //code to break, and current guess
    char rr,rw;         //right digit in right place vs. wrong place
    int nGuess;         //number of guesses
    
    //Initialize Values
    nGuess=0;
    code=set();
    //code="3815";
    rr=rw=0;
    cout<<"The code is: "<<code<<endl;
    
    do{
        nGuess++;
        guess=AI(rr,rw);
        cout<<"Guess "<<nGuess<<" guessing: "<<guess<<endl;
        
    }while(eval(code,guess,rr,rw));
    cout<<"Number of guesses using your super cool AI search = "<<nGuess<<endl;
    cout<<code<<"="<<guess<<endl;
    cout<<endl;

    //Exit the program
    
    return 0;
}


string AI(char rr,char rw){

    // A mess of variables...
        static string nextGuess = "0000",
                      finalGuess = "0000",
                      marked = "    ";

        static int guess = 0, found = 0, testValue = -1, nextDigit = 0,
                   first = -1, second = -1, third = -1, fourth = -1,
                   testPhase1 = 0, testPhase2 = 0, testPhase3 = 0, testPhase4 = 0,
                   attemptsPhase2 = 0, attemptsPhase3 = 0, attemptsPhase4 = 0;

        static bool confirmed[4] = {false, false, false, false},
                    second2 = false;

        int correctDigit;
        guess++;

    // There is alot of unnecessary code because I was A) short on time with my other classes and B) scared to
    // ruin something and not be able to trace my way back to success

    // This area of the function takes in the results of your last guess, if rr >=1 and all the spots are not filled
    // we enter and mark the spots were we have guessed values.
    // Here we also start setting up our finalGuess
    if (found == 4 && static_cast<int>(rr) >= 1 && !(confirmed[0] && confirmed[1] && confirmed[2] && confirmed[3])){
        if (static_cast<int>(rr) == 1){
            
            for (int i = 0; i < 4; i++) {
                if (nextGuess[i] != testValue + '0'){
                    confirmed[i] = true;
                    marked[i] = 'X';
                    finalGuess[i] = nextGuess[i];
                }
            }
            nextDigit++;
            if (confirmed[0] && confirmed[1] && confirmed[2] && confirmed[3]){
                nextGuess = finalGuess;
            }
        }

        if (static_cast<int>(rr) == 2){

            for (int i = 0; i < 4; i++) {
                if (nextGuess[i] != testValue + '0'){
                    confirmed[i] = true;
                    marked[i] = 'X';
                    finalGuess[i] = nextGuess[i];
                }
                
            }
            nextDigit += 2;
            if (confirmed[0] && confirmed[1] && confirmed[2] && confirmed[3]){
                nextGuess = finalGuess;
            }
        }

        if (static_cast<int>(rr) == 3){

            for (int i = 0; i < 4; i++) {
                if (nextGuess[i] != testValue + '0'){
                    confirmed[i] = true;
                    marked[i] = 'X';
                    finalGuess[i] = nextGuess[i];
                }
            }
            nextDigit += 3;
            if (confirmed[0] && confirmed[1] && confirmed[2] && confirmed[3]){
                nextGuess = finalGuess;
            }
        }
    }
    
    // Functions starts here, it initially starts at '0000' loops through to 9999 until all the digits are found
    // When rr > 1 we found a digit and save it and place it in variables. 
    // Found gets incremente when we find values so once all values are found this if staments ends
    if (found < 4){

        if (static_cast<int>(rr) >= 1){
            found++;
            if (found == 1){
                correctDigit = (((stoi(nextGuess)%1000)%100)%10);
                first = correctDigit;

                if (static_cast<int>(rr) == 2){
                    first = correctDigit;
                    second = correctDigit;
                    found++;
                }
                if (static_cast<int>(rr) == 3){
                    first = correctDigit;
                    second = correctDigit;
                    third = correctDigit;
                    found += 2;
                }
            }
            else if (found == 2){
                correctDigit = (((stoi(nextGuess)%1000)%100)%10);
                second = correctDigit;

                if (static_cast<int>(rr) == 2){
                    second = correctDigit;
                    third = correctDigit;
                    found++;
                }
                if (static_cast<int>(rr) == 3){
                    second = correctDigit;
                    third = correctDigit;
                    fourth = correctDigit;
                    found += 2;
                }
            }
            else if (found == 3){
                correctDigit = (((stoi(nextGuess)%1000)%100)%10);
                third = correctDigit;

                if (static_cast<int>(rr) == 2){
                    third = correctDigit;
                    fourth = correctDigit;
                    found++;
                }
                if (static_cast<int>(rr) == 3){
                    found += 2;
                }
            }
            else if (found == 4 && fourth == -1){
                correctDigit = (((stoi(nextGuess)%1000)%100)%10);
                fourth = correctDigit;

                if (static_cast<int>(rr) == 2){
                    found++;
                }
                if (static_cast<int>(rr) == 3){
                    found += 2;
                }
            }
        }
        
        if (guess < 10){
            if (guess==1)
                nextGuess = "0000";
            else if (guess >=2 && guess<=9)
                for (int i=0; i<nextGuess.length(); i++){
                    nextGuess[i] +=1;
                }
        }

        else if (guess == 10 && found < 4){ 
            if (found == 3){
                fourth = 9;
                found = 4;
            }
            if (found == 2){
                third = 9;
                fourth = 9;
                found = 4;
            }
            if (found == 1){
                second = 9;
                third = 9;
                fourth = 9;
                found = 4;
            }
        }
    }

    // Now that all four values are found and all positions havent been filled we start by checking each digit
    // in each spot. A tester variable '/' helps so that we can start at 0/// then /0// and so forth.
    // Four If statments, one for each digit. Once a position has been found, Positions get marked when found
    // and here we test to see if the next position we will be testing is marked, if it is, it moves to the next
    // postion. It also keeps track of attempts per each spot to ensure the minimum number of attempts.
    // If on the last digit, the digit is set to the empty spot and the nextGuess is set to finalGuess.
    if (found == 4 && !(confirmed[0] && confirmed[1] && confirmed[2] && confirmed[3])){

        if (nextDigit == 0){
            int positionIndex = testPhase1 % 4;
            for (int i = 0; i < 4; i++) {
                nextGuess[i] = (i == positionIndex) ? (first + '0') : (testValue + '0');
            }
            if (positionIndex == 3){
                finalGuess[testPhase1] = first + '0';
                nextDigit++;
            }
            cout<<nextGuess<<endl;
            testPhase1++;
        }
        if (nextDigit == 1){
            int positionIndex = testPhase2 % 4;
            if (marked[positionIndex]=='X'){
                positionIndex++;
                testPhase2++;
                if (marked[positionIndex]=='X'){
                    positionIndex++;
                    testPhase2++;
                    if (marked[positionIndex]=='X'){
                        positionIndex++;
                        testPhase2++;
                    }
                }
            }
            for (int i = 0; i < 4; i++) {
                nextGuess[i] = (i == positionIndex) ? (second + '0') : (testValue + '0');
            }
            if (attemptsPhase2 == 2){
                finalGuess[positionIndex] = second + '0';
                nextDigit++;
            }
            cout<<nextGuess<<endl;
            attemptsPhase2++;
            testPhase2++;
        }
        if (nextDigit == 2){
            int positionIndex = testPhase3 % 4;
            cout<<attemptsPhase3<<endl;
            if (marked[positionIndex]=='X'){
                positionIndex++;
                testPhase3++;
                if (marked[positionIndex]=='X'){
                    positionIndex++;
                    testPhase3++;
                    if (marked[positionIndex]=='X'){
                        positionIndex++;
                        testPhase3++;
                    }
                }
            }
            for (int i = 0; i < 4; i++) {
                nextGuess[i] = (i == positionIndex) ? (third + '0') : (testValue + '0');
            }
            
            if (attemptsPhase3 == 1){
                finalGuess[positionIndex] = third + '0';
                nextDigit++;
            }
            cout<<finalGuess<<endl;
            cout<<nextGuess<<endl;
            attemptsPhase3++;
            testPhase3++;
        }
        if (nextDigit == 3){
            int positionIndex = testPhase4 % 4;
            if (marked[positionIndex]=='X'){
                positionIndex++;
                testPhase4++;
                if (marked[positionIndex]=='X'){
                    positionIndex++;
                    testPhase4++;
                    if (marked[positionIndex]=='X'){
                        positionIndex++;
                        testPhase4++;
                    }
                }
            }
            for (int i = 0; i < 4; i++) {
                nextGuess[i] = (i == positionIndex) ? (fourth + '0') : (testValue + '0');
            }
            if (attemptsPhase4 == 0){
                finalGuess[positionIndex] = fourth + '0';
                nextDigit++;
            }
            nextGuess = finalGuess;
        }
    }
    
    return nextGuess;
}


bool eval(string code,string guess,char &rr,char &rw){
    string check="    ";
    rr=0,rw=0;
    //Check how many are right place
    for(int i=0;i<code.length();i++){
        if(code[i]==guess[i]){
            rr++;
            check[i]='x';
            guess[i]='x';
        }
    }
    //Check how many are wrong place
    for(int j=0;j<code.length();j++){
        for(int i=0;i<code.length();i++){
            if((i!=j)&&(code[i]==guess[j])&&(check[i]==' ')){
                rw++;
                check[i]='x';
                break;
            }
        }
    }
    
    //Found or not
    if(rr==4)return false;
    return true;
}

string set(){
    string code="0000";
    for(int i=0;i<code.length();i++){
        code[i]=rand()%10+'0';
    }
    return code;
}
