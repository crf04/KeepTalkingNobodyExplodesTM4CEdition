// Modules.h
// all the extra code
// switch stuff is in here

#ifndef __Module_h
#define __Module_h

#include <stdint.h>

class Bomb {
	private: 
		int strikes;             // how many strikes the bomb has
		bool explode;            // if the bomb has exploded or not
		int* moduleSetUp;        // which modules are where (-1 - Nothing, 0 - Timer, 1 - Alphabet, 2 - Wires, 3 - Button, 4 - Math, 5 - Morse Code)
		int total_modules;       // how many total modules there are
		int total_solved;        // how many of the modules are solved
		int* serial;
		int side;                // what side of the bomb the player can see/interact with (1-4)
		int selector;            // where the selector is on the bomb
	public:
		Bomb();
		Bomb(int*, int);   
		int total_time;                // total time in seconds
		int returnSide();        // which side of the bomb can the player see
		void setSide(int);       // set the side of the bomb (input integer is 1-4)
		void drawBomb2();        // draw the left side of the bomb
		void drawBomb4();        // draw the right side of the bomb
		bool hasExploded();      // checks if the bomb has exploded
		void failure();          // once three strikes have occurred or the timer has run out
		void instant();          // explode no matter what
		void drawSelectorRect(int, int);
		void drawSelector();     // draw the selector so the player can see where it is
		void moveSelector(int);   // 0 - up, 1 - down, 2 - left, 3 - right
		void setSelector(int);
		int getSelector();
		int score;
		void addStrikes();
		int getStrikes();
		void drawStrike(int);
		int num_solved;
		bool isSolved();         // checks if the bomb is solved
};


class Module: public Bomb {
public:
	Module();
	Module(bool);
	virtual void Draw(int);
	virtual void outTime(char*);
	void selectModule();         // selects the module (only if it is not solved)
	void deselectModule();
	void drawModule(int);        // draw the module join the bomb as is; each module class has its own draw method
	void drawEmptyModule(int l); // draw an empty module (no lights, cannot be interacted with)
	void drawSolved(int l);
	bool selected;               // is module selected
	bool solved;                 // true if module is solved, false if not
	bool empty;                  // true if empty, false if actual module
};

class Timer: public Module {
private:
	int loc;
public:
	int c;
	int flag;
	Timer(int location);
	void DrawTimerRect(int, int);
	virtual void Draw(int) override;
	virtual void outTime(char*) override;
	int getloc();
};

class Button: public Module {
private:
	int color;         // options: red, white, yellow
	int phrase;
	int loc;
	int ind;
	const unsigned short* button;
public:
	Button(int);
	virtual void Draw(int) override;
	int determineAnswer();
	int getLocation();
	void Draw();
	void init();
};

class MorseCode: public Module {
private:
	int loc;
	int index;
	int freq;
public:
	char* word;
	char* morseWord;
	MorseCode(int);
	void init();
	virtual void Draw(int) override;
	void dot();
	void dash();
	void space();
	void flashLED();
	char* getMW();
	void setFreq(int);
	void outFreq();
	void check();
	int determineAnswer();
	void select();
};

class Wires: public Module {
private:
	uint8_t arr[6];
	uint8_t amt;        // between 3-6
	int color;   // options: red, white, blue, black, yellow
	int selector;  // selects which wire
	int loc;       // location of the wire module
	int answer;    // the correct wire to cut (doesnt change even after cutting the wrong wire(s))
public:  
	Wires(int location);
	virtual void Draw(int) override;  //draws the entire set of wires
	void drawCutWire();       //draws a cut wire
	int determineAnswer();    //returns index of wire that should be cut
	int indexOf(int color);
	int lastIndexOf(int color);
	void moveSelector(int dir);
	void drawWSelector();
	int getSelector();
	void setSelector(int);
	void check(int);       
	void cut();
};


typedef struct img {
	const unsigned short* picture;
	int number;    // really large number indicates no number, and an operator instead
	int op;        // if op == 1, indicates +(addition); op == -1, indicates -(subtraction); op == 2, indicates x(multiplication)
	// 0 indicates nothing ^^
}img;

class Math: public Module {
private:
	int num1;             // num 1 index
	int num2;             // num 2 index
	int op;               // operator (+,-,*)
	img answer;           // image that corresponds to the correct anser
	int loc;              // location of the module
	int answer_location;  // pick the location of the answer so its not in the same place every time
	int selector;         // location of selector
	int rand1, rand2;     // the other two images that are not the answer that the player can still select
public:
	Math(int location);
	virtual void Draw(int) override;
	img determineAnswer();  // returns the correct(s) answer
	void drawMSelector();
	void moveSelector(int dir);  // 0 is left, 1 is right
	void check(int selected);    // checks if the current picture is the right answer
	void select();
};

void Key_Init(void);
uint32_t Key_In(void);   //  PE0 is up, PE1 is down, PE2 is left, PE3 is right
uint32_t Key_InF(void);
void Delay1ms(uint32_t);

#endif