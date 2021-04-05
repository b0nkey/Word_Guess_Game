#include <iostream>
#include <fstream>
#include <vector>
#include <map>
// For some reason, these two are needed on Windows machines but not on Mac
// #include <time.h>
// #include <math.h>
using namespace std;

// Keeping the "available words" list the same size, and adding an "available" flag to each word was
// significantly faster than removing words from a vector. This will use more memory as values that arent in 
// use will be still reserved, but the trade off for speed is worth it in tis situation.


class WordGuess{
private:
    map<string, bool> wordList; // remaining words
    map <char, bool> usedChars; 
    pair <char, bool>* word = nullptr;
    int wordLen; //user specified word length
    int guesses; // user specified guesses

public:
    WordGuess(string, int, int); // Default constructor
    ~WordGuess();

    // Methods
    bool again();
    bool already_used(char);
    bool successful();
    string current_word();
    void add_letter(char c);
    map<string,bool> updateCategory(map<string,bool>, string, char);
    void set_word(string, string);
    string toBinary(int);
    int countWords(map<string, bool>);

    // Getters
    int words_left();
    int guesses_left();
    string get_answer();

    // Setters
    void set_word_len(int);
    void set_guesses(int);
};


WordGuess::WordGuess(string fileName, int l, int g){
    // Initialize setup data
    set_guesses(g);
    set_word_len(l);
    word = new pair<char, bool>[wordLen];
    for(int i=0; i<wordLen; i++)  word[i].second = false;

    // Populate wordList with words of length wordLen
    ifstream f(fileName);
    string s;
    if(!f){
        cout<<"Error opening output file"<<endl;
        exit(0);
    }
    while(!f.eof()){
        f >> s;
        if(s.length() == wordLen)  wordList.insert(make_pair(s, true));
    }

    // Initialize usedChar list
    for(char ch='a'; ch<='z'; ++ch)
        usedChars.insert(make_pair(ch, false));
}

WordGuess::~WordGuess(){
    delete word;
}

int WordGuess::countWords(map<string, bool>wordMap){
    // Counts words remaining in the map it's passed
    int r=0;
    for(auto &e : wordMap)
        if(e.second == true)  r++;
    return r;
}

string WordGuess::toBinary(int n){
    // converts an int to a binary string
    string r;
    for(int i=0; i<wordLen; i++){
        r=(n%2==0 ?"0":"1")+r; // if n is even, add 0 to string, else add 1
        n/=2; // half n
    }
    return r;
}

bool WordGuess::again(){
    if(guesses<=0)  return false;
    else return !successful();
}

bool WordGuess::already_used(char c){ return usedChars[c]; }

bool WordGuess::successful(){
    for(int i=0; i<wordLen; i++)
        if(word[i].second == false)  return false;
    return true;
}

string WordGuess::current_word(){
    string s;
    for(int i=0; i<wordLen; i++){
        if(word[i].second)  s.push_back(word[i].first);
        else s.push_back('_');   
    }
    return s;
}

void WordGuess::set_word(string w, string bin){
    int i=0;
    for(i=0; i<wordLen; i++){
        word[i].first = w[i];
        if(bin[i] == '1') word[i].second=true;
    }
}

void WordGuess::add_letter(char c){
    // using a binary system to create an instance of every possible word category.
    int i,j,biggestCat=0;

    vector<map<string,bool> > catList; // holds a version of wordList for every possible category
    map<string,bool> thisCat = wordList; // create copy of wordList
    for(i=0; i<pow(2, wordLen); i++){ // 2^wordLen = every combination
        catList.push_back(updateCategory(thisCat, toBinary(i), c)); // add updated category to parent list
        if (countWords(catList[i]) >= countWords(catList[biggestCat])) // update index of largest cat
            biggestCat = i;
    }

    wordList = catList[biggestCat]; // update main wordList

    string newWord;
    for(auto &e : wordList) // get new word
        if(e.second == true){  newWord = e.first;  break;  } 
    set_word(newWord, toBinary(biggestCat));

    usedChars[c] = true;
    set_guesses(guesses_left()-1);
}

map<string,bool> WordGuess::updateCategory(map<string,bool> v, string bin, char c){
    int i,j;
    bool inclusiveFlag=false;
    for(i=0; i<wordLen; i++)
        if(bin[i] == '1'){ inclusiveFlag=true; break; }

    if(inclusiveFlag == false){ // "no-match" category, needed a seperate check as its inclusive instead of exclusive
        bool deleteFlag=false;
        for(auto &e : v){
            for(i=0; i<wordLen; i++){
                if(e.first[i] == c){  deleteFlag=true;  break;  }
            }
            if(deleteFlag == true){
                e.second = false;
                deleteFlag = false;
            }
        }
    }else{
        int i;
        for(auto &e : v){ // for each word
            if(e.second == false)  continue;  // if word is available
            for(i=0; i<wordLen; i++){   // check each char in the word
                if((e.first[i] == c) ^ (bin[i] == '1')){ // (if char in word is c) XOR (category position is true)
                    e.second = false; // remove from available list
                    break; // no need to check the rest of the letters
                }
            }
        }
    }
    
    return v;
}

int WordGuess::words_left(){
    int count=0;
    for(auto &e : wordList)
        if(e.second==true) count++;
    return count;
}

int WordGuess::guesses_left(){ return guesses; } 

string WordGuess::get_answer(){
    string s;
    for(int i=0; i<wordLen; i++)
        s.push_back(word[i].first);
    return s;
}

void WordGuess::set_word_len(int len){ this->wordLen = len; }

void WordGuess::set_guesses(int g){ this->guesses = g; }









int main(int argc, char * argv[]) {
    char letter;

    if (argc < 3) {
        cout << "Must enter word length and number of guesses on command line" << endl;
        exit(0);
    }
    WordGuess wg("dictionary.txt", atoi(argv[1]), atoi(argv[2]));
    while (wg.again()) {
        cout << "Enter a letter: ";
        cin >> letter;
        if (!islower(letter))
            exit(0);
        if (wg.already_used(letter)) {
            cout << "You have already tried " << letter << endl;
            continue;
        }
        wg.add_letter(letter);
        cout << "Full word: " << wg.get_answer() << endl;
        cout << wg.current_word() << ", Guesses left = " << wg.guesses_left() <<
            ", Words left = " << wg.words_left() << endl;
    }
    if (wg.successful())
        cout << "Good job!" << endl;
    else
        cout << "Sorry, the word was " << wg.get_answer() << endl;
    return 0;
}