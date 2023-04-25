// Module.cpp
// implements Module class

#include "Module.h"
#include "ST7735.h"
#include "Sprites.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "random.h"

const int BOMB_WIDTH = 120;
const int BOMB_HEIGHT = 80;
const int BOMB_X = 3;
const int BOMB_Y = 35;
const int SELECTOR_SIZE = 40;
const int X_LEFT = 3;
const int Y_TOP = 35;
const int X_MID = 43;
const int X_RIGHT = 83;
const int Y_BOTTOM = 75;
extern Bomb b;
extern Timer t;
extern int screen;
extern void Bomb_Init(int[12]);
extern int num_wires, num_math, num_button, num_MorseCode;
const int xCords[13] = {0,3,43,83,3,43,83,3,43,83,3,43,83}; //xcord for top left of each box; xCords[1] is top left x cord of box 1, etc
const int yCords[13] = {0,35,35,35,75,75,75,35,35,35,75,75,75};
const unsigned short *math_images[29] = {0, image1, image2, image3, image4, image5, image6, image7, 
		image8, image9, image10, image11, image12, image13, image14,
		image15, image16, image17, image18, image19, image20, image21,
		image22, image23, image24, image25, image26, image27, image28};
const char* morse[10] = {
	"000 0000 0 0100 0100a",
	"0000 01 0100 0100 000a",
	"000 0100 00 1010 101a",
	"0001 0 1010 1 111 010a",
	"000 1 0 01 101a",
	"1000 010 0 01 101a",
	"0010 0100 00 1010 101a",
	"1000 111 1001 0 000a",
	"1000 111 11 1000 000a",
	"1000 0 01 1 000a"
}; 

const char* words[10] = {"shell","halls","slick","vector","steak", "break", "flick", "boxes", "bombs", "beats"};

const int frequencies[10] = {3505, 3515, 3525, 3535, 3555, 3565, 3570, 3580, 3595, 3600};

// draw an unfilled rectangle (x and y are the top left of the rectangle)
void drawRect(int x, int y, int w, int h, uint16_t color) {
	ST7735_DrawFastVLine(x,y,h,color);
	ST7735_DrawFastVLine(x+w,y,h,color);
	ST7735_DrawFastHLine(x,y,w,color);
	ST7735_DrawFastHLine(x,y+h,w+1,color);
}

// Bomb Constructor
// initializes everything to 0
Bomb::Bomb() {
	strikes = 0;
	total_time = 0;
	explode = false;
	side = 1;
	moduleSetUp = 0;
	total_modules = 0;
	total_solved = 0;
	score = 0;
	selector = 1;
	serial = 0;
	num_solved = 0;
}

// Bomb constructor with module setup and time parameters
// arr determines what modules are in what location, and t determines how much time the player is given to defuse the bomb
Bomb::Bomb(int* arr, int t) {
	strikes = 0;
	this->total_time = t;      
	explode = false;	
	side = 1;
	moduleSetUp = arr;
	for (int i = 0; i < 12; i++) {
		if (arr[i] > 0)
			total_modules++;
	}
	total_solved = 0;
	score = 0;
	selector = 1;
	serial = 0;
	num_solved = 0;
}

int Bomb::returnSide() {
	return this->side;
}

bool Bomb::isSolved() {
	int total_modules = num_button + num_math + num_wires + num_MorseCode;
	if (total_modules == num_solved)
		return true;
	return false;
}

void Bomb::setSide(int s) {
	side = s;
}

// Draw the left side of the bomb
void Bomb::drawBomb2() {
	ST7735_FillRect(53,35,20,80,ST7735_GRAY);
	ST7735_DrawFastVLine(53,35,80,ST7735_WHITE);
	ST7735_DrawFastVLine(73,35,80,ST7735_WHITE);
	ST7735_DrawFastHLine(53,35,20,ST7735_WHITE);
	ST7735_DrawFastHLine(53,115,21,ST7735_WHITE);
}

// Draw the right side of the bomb
void Bomb::drawBomb4() {
	ST7735_FillRect(53,35,20,80,ST7735_GRAY);
	ST7735_DrawFastVLine(53,35,80,ST7735_WHITE);
	ST7735_DrawFastVLine(73,35,80,ST7735_WHITE);
	ST7735_DrawFastHLine(53,35,20,ST7735_WHITE);
	ST7735_DrawFastHLine(53,115,21,ST7735_WHITE);
}

// returns if the bomb has exploded or not
bool Bomb::hasExploded() {
	return explode;
}

// determines when the bomb explodes
void Bomb::failure() {
	if (strikes == 3 || total_time == 0) 	
		explode = true;
}

// method to make the bomb explode instantly (not useful in real game)
void Bomb::instant() {
	explode = true;
}

// the graphics for the selector (a yellow square)
void Bomb::drawSelectorRect(int x, int y) {
	ST7735_DrawFastVLine(x, y, 40, ST7735_YELLOW);
	ST7735_DrawFastVLine(x+40, y, 40, ST7735_YELLOW);
	ST7735_DrawFastHLine(x, y, 40, ST7735_YELLOW);
	ST7735_DrawFastHLine(x, y+40, 41, ST7735_YELLOW);
}

// draws the selector based on where the selector is
void Bomb::drawSelector() {
		switch (selector) {
			case 1:
			case 7:
				drawSelectorRect(3, 35);
				break;
			case 2:
			case 8:
				drawSelectorRect(43, 35);
				break;
			case 3:
			case 9:
				drawSelectorRect(83, 35);
				break;
			case 4:
			case 10:
				drawSelectorRect(3, 75);
				break;
			case 5:
			case 11:
				drawSelectorRect(43, 75);
				break;
			case 6:
			case 12:
				drawSelectorRect(83, 75);
				break;
		}
}

// move the selector in a certain direction
void Bomb::moveSelector(int dir) {
	switch (dir) {
		case 0:  // up
			if(selector != 1 && selector!= 2 && selector != 3 && selector != 7 && selector != 8 && selector != 9)
				selector-=3;
			break;
		case 1:  // down
			if(selector == 1 || selector == 2 || selector == 3 || selector == 7 || selector == 8 || selector == 9)
				selector+=3;
			break;
		case 2:  // left
			if(selector != 1 && selector != 4 && selector !=7 && selector != 10)
				selector--;
			break;
		case 3:   // right
			if(selector != 3 && selector != 6 && selector != 9 && selector != 12)
				selector++;
			break;
	}
}

void Bomb::setSelector(int s) {
	selector = s;
}

void Bomb::addStrikes() {
	strikes++;
	if (strikes == 3) {
		failure();
	}
}

int Bomb::getStrikes() {
	return strikes;
}

int Bomb::getSelector() {
	return selector;
}

void Bomb::drawStrike(int location) {
	if (location == 1) {
		if (strikes == 1) 
			ST7735_DrawBitmap(X_LEFT+5, Y_TOP+35, strike, 10, 7);
		if (strikes == 2) {
			ST7735_DrawBitmap(X_LEFT+5, Y_TOP+35, strike, 10, 7);
			ST7735_DrawBitmap(X_LEFT+15, Y_TOP+35, strike, 10, 7);
		}
	} else if (location == 4) {
		if (strikes == 1) 
			ST7735_DrawBitmap(X_LEFT+5, Y_BOTTOM+35, strike, 10, 7);
		if (strikes == 2) {
			ST7735_DrawBitmap(X_LEFT+5, Y_TOP+35, strike, 10, 7);
			ST7735_DrawBitmap(X_LEFT+15, Y_BOTTOM+35, strike, 10, 7);
		}
	}
}




Module::Module() {
	solved = false;
	selected = false;
	empty = false;
}

Module::Module(bool isEmpty) {
	solved = false;
	selected = false;
	empty = isEmpty;
}

void Module::Draw(int location) {
	drawModule(location);
}

void Module::selectModule(void) {
	if (solved == false)
		selected = true;
	else
		selected = false;
}

void Module::deselectModule() {
	if (selected)
		selected = false;
}

/*
How the modules are numbered:

Side 1:
	____________________________________
 |           |           |            |
 |           |           |            |
 |     1     |     2     |     3      |
 |           |           |            |
  ------------------------------------
 |           |           |            |
 |           |           |            |
 |    4      |     5     |     6      |
 |           |           |            |
	------------------------------------

Side 3:

	____________________________________
 |           |           |            |
 |           |           |            |
 |     7     |     8     |     9      |
 |           |           |            |
  ------------------------------------
 |           |           |            |
 |           |           |            |
 |    10     |     11    |     12     |
 |           |           |            |
	------------------------------------

*/

// draws the gray background, white outline, and light for the module
void Module::drawModule(int location) {
	int x = xCords[location];
	int y = yCords[location];
	if (empty) {
		drawEmptyModule(location);
	} else {
		ST7735_FillRect(x,y,40,40,ST7735_GRAY);
		drawRect(x,y,40,40,ST7735_WHITE);
		if (!solved) {
			switch (location) {
				case 1:
				case 7:
					ST7735_DrawBitmap(X_LEFT+34, Y_TOP+7, unsolved_light, 5, 5);
					break;
				case 2:
				case 8:
					ST7735_DrawBitmap(X_MID+34, Y_TOP+7, unsolved_light, 5, 5);
					break;
				case 3:
				case 9:
					ST7735_DrawBitmap(X_RIGHT+34, Y_TOP+7, unsolved_light, 5, 5);
					break;
				case 4:
				case 10:
					ST7735_DrawBitmap(X_LEFT+34, Y_BOTTOM+7, unsolved_light, 5, 5);
					break;
				case 5:
				case 11:
					ST7735_DrawBitmap(X_MID+34, Y_BOTTOM+7, unsolved_light, 5, 5);
					break;
				case 6:
				case 12:
					ST7735_DrawBitmap(X_RIGHT+34, Y_BOTTOM+7, unsolved_light, 5, 5);
					break;
			};
		} 
	}
}

void Module::drawSolved(int location) {
	if (solved) {
		switch (location) {
			case 1:
			case 7:
				ST7735_DrawBitmap(X_LEFT+34, Y_TOP+7, solved_light, 5, 5);
				break;
			case 2:
			case 8:
				ST7735_DrawBitmap(X_MID+34, Y_TOP+7, solved_light, 5, 5);
				break;
			case 3:
			case 9:
				ST7735_DrawBitmap(X_RIGHT+34, Y_TOP+7, solved_light, 5, 5);
				break;
			case 4:
			case 10:
				ST7735_DrawBitmap(X_LEFT+34, Y_BOTTOM+7, solved_light, 5, 5);
				break;
			case 5:
			case 11:
				ST7735_DrawBitmap(X_MID+34, Y_BOTTOM+7, solved_light, 5, 5);
				break;
			case 6:
			case 12:
				ST7735_DrawBitmap(X_RIGHT+34, Y_BOTTOM+7, solved_light, 5, 5);
				break;
		}
	}
}

void Module::drawEmptyModule(int location) {
	int x = xCords[location];
	int y = yCords[location];
	ST7735_FillRect(x,y,40,40,ST7735_GRAY);
	drawRect(x,y,40,40,ST7735_WHITE);
	ST7735_DrawBitmap(x+5,y+35,emptymodule,30,30);
}

void Module::outTime(char*) {
}



Timer::Timer(int location) {
	selected = false;
	loc = location;
}

void Timer::DrawTimerRect(int x, int y) {
	ST7735_FillRect(x,y,30,16,ST7735_BLACK);
	drawRect(x,y,30,16,ST7735_WHITE);
}

// timer is only on either location 1 or 4 (the other 4 do not line up well with OutString)
void Timer::Draw(int location) {
	switch (loc) {
		case 1: {
			DrawTimerRect(X_LEFT+5, Y_TOP+10);
			ST7735_FillRect(X_LEFT+5, Y_TOP+28, 30, 8, ST7735_BLACK);
			drawRect(X_LEFT+5, Y_TOP+28, 30, 8, ST7735_WHITE);
			break;
		}
		case 4: {
			DrawTimerRect(X_LEFT+5, Y_BOTTOM+10);
			ST7735_FillRect(X_LEFT+5, Y_BOTTOM+28, 30, 8, ST7735_BLACK);
			drawRect(X_LEFT+5, Y_BOTTOM+28, 30, 8, ST7735_WHITE);
			break;
		}
		default:
			break;
	}
	if(solved)
		drawSolved(loc);
}

void Timer::outTime(char* t) {
	switch (loc) {
		case 1:
			ST7735_SetCursor(2, 5);
			//ST7735_OutString(t);
			ST7735_DrawString(2,5,t,ST7735_RED);
			break;
		case 4:
			ST7735_SetCursor(2, 9);
			//ST7735_OutString(t);
		  ST7735_DrawString(2,9,t,ST7735_RED);
			break;
	}
}

int Timer::getloc() {
	return loc;
}





Wires::Wires(int location) {
	Random_Init(NVIC_ST_CURRENT_R);
	amt = 3 + Random()%4; //generates a number between 3-6
	for(int x = 0; x < amt; x++) { //generates array of numbers between 0-4
		arr[x] = ((Random()) % 5); //0 = RED, 1 = WHITE, 2 = BLUE, 3 = BLACK, 4 = YELLOW, COLOR+5 = CUT
	}
	selector = 0;
	loc = location;
	answer = determineAnswer();
}

void Wires::Draw(int location) {
	drawModule(loc);
	if(solved)
		drawSolved(loc);
	int x = xCords[loc];
	int y = yCords[loc];
	ST7735_FillRect(x + 5, y + 10, 5, 28, ST7735_BLACK);
	ST7735_FillRect(x + 30, y + 10, 5, 28, ST7735_BLACK);
	for(int a = 0; a < amt; a++) {
		switch(arr[a]) {
			case 0:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_RED);
				break;
			case 1:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_WHITE);
				break;
			case 2:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_BLUE);
				break;
			case 3:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_BLACK);
				break;
			case 4:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_YELLOW);
				break;
			case 5:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_RED);
				drawCutWire();
				break;
			case 6:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_WHITE);
				drawCutWire();
				break;
			case 7:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_BLUE);
				drawCutWire();
				break;
			case 8:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_BLACK);
				drawCutWire();
				break;
			case 9:
				ST7735_FillRect(x + 10, y + 10 + (a * 5), 20, 2, ST7735_YELLOW);
				drawCutWire();
				break;
		};	
	}
	drawWSelector();
}

void Wires::drawCutWire() {
	int x = xCords[loc];
	int y = yCords[loc];
	for (int i = 0; i < amt; i++) {
		if (arr[i] >= 5) {
			ST7735_FillRect(x+17, y+10+(i*5),5,2,ST7735_GRAY);
		}
	}
}

int Wires::determineAnswer() { //returns index of wire that should be cut
	if(amt == 3) {
		if(indexOf(0) == -1) {
			return 1;
		} else if(arr[amt - 1] == 1) {
			return amt - 1;
		} else if(indexOf(2) != lastIndexOf(2)) {
			return lastIndexOf(2);
		} else {
			return amt - 1;
			}
	} else if(amt == 4) {
		if(indexOf(0) != lastIndexOf(0)) {
			return lastIndexOf(0);
		} else if(indexOf(0) == -1 && arr[amt - 1] == 4) {
			return 0;
		} else if(indexOf(2) != -1 && indexOf(2) == lastIndexOf(2)) {
			return 0;
		}	else if(indexOf(4) != lastIndexOf(4)) {
			return amt - 1;
		} else {
			return 1;
		}
	} else if(amt == 5) {
		if(arr[amt - 1] == 3) {
			return 3;
		} else if(indexOf(0) != -1 && indexOf(0) == lastIndexOf(0) && indexOf(4)!= lastIndexOf(4)) {
			return 0;
		} else if(indexOf(3) == -1) {
			return 1;
		} else {
			return 0;
		}
	}
	else if(amt == 6) {
		if(indexOf(4) == -1) {
			return 2;
		} else if(indexOf(4) == lastIndexOf(4) && indexOf(4) != -1 && indexOf(1)!= lastIndexOf(1) )  {
			return 3;
		} else if(indexOf(0) == -1) {
			return amt - 1;
		} else {
			return 3;
		}
	}
	return -1;
}

int Wires::indexOf(int color) {
	for(int x = 0; x < amt; x++) {
		if(arr[x] == color) {
			return x;
		}
	}
	return -1;
}

int Wires::lastIndexOf(int color) {
	for(int x = amt - 1; x >= 0; x--) {
		if(arr[x] == color)
			return x;
	}
	return -1;
}

void Wires::moveSelector(int dir) {
	// 0 is up, 1 is down
	if (dir == 0) {
		if (selector > 0){
			int i = selector-1;
			while (i >= 0 && (arr[i] >= 5))
				i--;
			if (i >= 0) 
				selector = i;
            
		}
	} else if (dir == 1) {
			if (selector < amt-1) {
				int i = selector+1;
				while (i < amt && (arr[i] >= 5)) 
					i++;
				if (i < amt)
					selector = i;
			}
		}
	drawModule(loc);
	Draw(0);
}

void Wires::drawWSelector() {
	if (!selected)
		return;
	int x = xCords[loc];
	int y = yCords[loc];
	switch (selector) {
			case 0: {
				drawRect(x+9, y+9, 21, 3, ST7735_CYAN);
				break;
			}
			case 1: {
				drawRect(x+9, y+14, 21, 3, ST7735_CYAN);
				break;
			}
			case 2: {
				drawRect(x+9, y+19, 21, 3, ST7735_CYAN);
				break;
			}
			case 3: {
				drawRect(x+9, y+24, 21, 3, ST7735_CYAN);
				break;
			}
			case 4: {
				drawRect(x+9, y+29, 21, 3, ST7735_CYAN);
				break;
			}
			case 5: {
				drawRect(x+9, y+34, 21, 3, ST7735_CYAN);
				break;
			}
	}
}

int Wires::getSelector() {
	return this->selector;
}

void Wires::setSelector(int s) {
	this->selector = s;
}

void Wires::check(int s) {
	if(s == answer) {
		solved = true;
	}
	else {
		b.addStrikes();
	}
}

void Wires::cut() {
	check(selector);
	arr[selector] += 5;      // +5 indicates the wire has been cut
	if (solved == true) {
		selected = false;
		b.num_solved++;
		drawModule(loc);
		Draw(0);
		drawSolved(loc);
		screen = 1;
	} else {
		if (selector != amt-1)
			moveSelector(1);
		else 
			moveSelector(0);
	}
}



const unsigned short* buttons[20] = {
	blue_abort,
	blue_hold,
	blue_no,
	blue_stop,
	blue_press,
	red_abort,
	red_hold,
	red_no, 
	red_stop, 
	red_press,
	white_abort,
	white_hold, 
	white_no,
	white_press, 
	white_stop, 
	yellow_bumper, 
	yellow_hold,
	yellow_no,
	yellow_press,
	yellow_stop};

	//blue = 0, red = 1, white = 2, yellow = 3
const int colors[20] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3};
	//abort = 0, hold = 1, no = 2, stop = 3, press = 4}
const int phrases[20] = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 4, 3, 0, 1, 2, 4, 3};

Button::Button(int location) {
	loc = location;
	Random_Init(NVIC_ST_CURRENT_R);
	ind = Random() % 20;
	color = colors[ind];
	phrase = phrases[ind];
}

void Button::Draw(int location) {
	ST7735_DrawBitmap(xCords[loc], yCords[loc]+40, buttons[ind], 40, 40);
	drawRect(xCords[loc],yCords[loc],40,40,ST7735_WHITE);
	ST7735_DrawBitmap(xCords[loc]+34, yCords[loc]+7, unsolved_light, 5, 5);
	if(solved)
		drawSolved(loc);
}

int Button::getLocation() {
	return loc;
}
int Button::determineAnswer() {
	if(color == 1 && (phrase == 3 || phrase == 0)) {
		return 2;
	}
	if(phrase == 1 && color != 1 && color != 2) {
			return 0; //press once and immediately release
	}
	else if(phrase == 0 && (color == 0 || color == 1)) {
		return 1; //hold for three seconds
	}
	else if(color == 3 && (phrase == 3 || phrase == 4)) {
		return 1; //hold for three seconds
	}
	else if(color == 2 && (phrase == 0 || phrase == 4)) {
		return 0; //press button once
	}
	else if(phrase == 2) {
		return 2; //wait 7s, then press once
	}
	else {
		return 1; //hold for three sec
	}
}




MorseCode::MorseCode(int location) {
	loc = location;
	Random_Init(NVIC_ST_CURRENT_R);
	index  = Random() % 10;
	word = (char*) words[index];
	morseWord = (char*) morse[index];
}

void MorseCode::Draw(int location) {
	// only locations 3,6,9,12
	drawModule(loc);
	int x = xCords[loc];
	int y = yCords[loc];
	ST7735_FillRect(x+2,y+10,36,25,ST7735_BLACK);
	drawRect(x+2,y+10,36,25,ST7735_WHITE);
	if(solved)
		drawSolved(loc);
}

void MorseCode::outFreq() {
	switch (loc) {
		case 3: {
			ST7735_SetCursor(15,5);
			ST7735_OutFix(freq);
			ST7735_SetCursor(16,6);
			ST7735_OutString((char*)"MHz");
			break;
		}
		case 6: {
			ST7735_SetCursor(15,9);
			ST7735_OutFix(freq);
			ST7735_SetCursor(16,10);
			ST7735_OutString((char*)"MHz");
			break;
		}
	}
}

void MorseCode:: flashLED() {
	int i = 0;
	while(morseWord[i] != 'a'){
	if(morseWord[i] == '0')
			dot();
		else if(morseWord[i] == '1')
			dash();
		else if(morseWord[i] == ' ')
			space();
		i++;
	}
}

char* MorseCode:: getMW() {
	return morseWord;
}

void MorseCode::dot() {
	GPIO_PORTD_DATA_R |= 0x01;
	Delay1ms(1000);
	GPIO_PORTD_DATA_R &= ~0x01;
	Delay1ms(1000);
}

void MorseCode::dash() {
	GPIO_PORTD_DATA_R |= 0x01;
	Delay1ms(3000);
	GPIO_PORTD_DATA_R &= ~0x01;
	Delay1ms(1000);
}

void MorseCode::space() {
	Delay1ms(3000);
}

void MorseCode::setFreq(int fr) {
	freq = fr;
}

void MorseCode::check() {
	if(freq == frequencies[index]) {
		selected = false;
		solved = true;
		b.num_solved++;
		drawModule(loc);
		Draw(0);
		drawSolved(loc);
		screen = 1;
	}
	else {
		b.addStrikes();
	}
	
}
img img1 = {math_images[1], 1, 0};         //1
img img2 = {math_images[2], 10000, 1};     //+
img img3 = {math_images[3], 5, 0};         //5
img img4 = {math_images[4], 10, 0};        //10
img img5 = {math_images[5], 15, 0};        //15
img img6 = {math_images[6], 9, 0};         //9
img img7 = {math_images[7], 2, 0};         //2
img img8 = {math_images[8], 14, 0};        //14
img img9 = {math_images[9], 4, 0};         //4
img img10 = {math_images[10], 10000, 1};   //+
img img11 = {math_images[11], 3, 0};       //3
img img12 = {math_images[12], -1, 0};      //-1
img img13 = {math_images[13], 13, 0};      //13
img img14 = {math_images[14], 7, 0};       //7
img img15 = {math_images[15], -2, 0};      //-2
img img16 = {math_images[16], -3, 0};      //-3
img img17 = {math_images[17], -4, 0};      //-4
img img18 = {math_images[18], 17, 0};      //17
img img19 = {math_images[19], 6, 0};       //6
img img20 = {math_images[20], 16, 0};      //16
img img21 = {math_images[21], -5, 0};      //-5
img img22 = {math_images[22], 10000, 2};   //*
img img23 = {math_images[23], 8, 0};       //8
img img24 = {math_images[24], 11, 0};      //11
img img25 = {math_images[25], 9, 0};       //9
img img26 = {math_images[26], 0, 0};       //0
img img27 = {math_images[27], 12, 0};      //12
img img28 = {math_images[28], 100000, -1}; //-
img imgarray[29] = {img1, img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12, img13, img14, img15, img16,
	img17, img18, img19, img20, img21, img22, img23, img24, img25, img26, img27, img28};

const unsigned short *operations[4] = {image2, image10, image22, image28}; 
/*
		0 +
		1 +
		2 *
		3 -
*/

img imgdigits[11] = {img1, img3, img6, img7, img9, img11, img14, img19, img23, img25, img26};
img imgops[4] = {img2, img10, img22, img28};
img imgposs[24] = {img1, img3, img4, img5, img6, img7, img8, img9, img11, img12, img13, img14, img15, img16, img17, img18, img19,
	img20, img21, img23, img24, img25, img26, img27};

Math::Math(int location) {
	Random_Init(NVIC_ST_CURRENT_R);
  num1 = Random()%10;
  num2 = Random()%10;
  op = Random()%4;
	loc = location;
	answer_location = Random()%3+1;
	selector = 1;
	answer = determineAnswer();
  rand1 = Random()%24;
  rand2 = Random()%24;
}

void Math::Draw(int location) {	
	drawModule(loc);
	if(solved)
		drawSolved(loc);
	int x = xCords[loc];
	int y = yCords[loc];
	ST7735_DrawBitmap(x+1,y+22,imgdigits[num1].picture,13,13);
	ST7735_DrawBitmap(x+14,y+22,imgops[op].picture,13,13);
	ST7735_DrawBitmap(x+27,y+22,imgdigits[num2].picture,13,13);
	switch (answer_location) {
		case 1: {
			ST7735_DrawBitmap(x+1,y+39,determineAnswer().picture,13,13);
			ST7735_DrawBitmap(x+14,y+39,imgposs[rand1].picture,13,13);
			ST7735_DrawBitmap(x+27,y+39,imgposs[rand2].picture,13,13);
			break;
		}
		case 2: {
			ST7735_DrawBitmap(x+1,y+39,imgposs[rand2].picture,13,13);
			ST7735_DrawBitmap(x+14,y+39,determineAnswer().picture,13,13);
			ST7735_DrawBitmap(x+27,y+39,imgposs[rand1].picture,13,13);
			break;
		}
		case 3: {
			ST7735_DrawBitmap(x+1,y+39,imgposs[rand1].picture,13,13);
			ST7735_DrawBitmap(x+14,y+39,imgposs[rand2].picture,13,13);
			ST7735_DrawBitmap(x+27,y+39,determineAnswer().picture,13,13);
			break;
		}
	}
	drawMSelector();
}

img Math::determineAnswer() {
	int S, A, M;
	img imgS, imgA, imgM;
	switch (imgops[op].op) {
		case -1: {
			S = imgdigits[num1].number - imgdigits[num2].number;
			while (S < -5 || S > 17) {
				num1 = Random()%10;
				num2 = Random()%10;
			}
			for (int i = 0; i < 24; i++) {
				if (S == imgposs[i].number)
					imgS = imgposs[i];
			}
			return imgS;
		}
		case 1: {
			A = imgdigits[num1].number + imgdigits[num2].number;
			while (A < -5 || A > 17) {
				num1 = Random()%10;
				num2 = Random()%10;
			}
			for (int i = 0; i < 24; i++) {
				if (A == imgposs[i].number)
					imgA = imgposs[i];
			}
			return imgA;
		}
		case 2: {
			M = imgdigits[num1].number * imgdigits[num2].number;
			while (M < -5 || M > 17) {
				num1 = Random()%10;
				num2 = Random()%10;
			}
			for (int i = 0; i < 24; i++) {
				if (M == imgposs[i].number)
					imgM = imgposs[i];
			}
			return imgM;
		}
	}
	return img26;
}

void Math::drawMSelector() {
	if (!selected)
		return;
	int x = xCords[loc];
	int y = yCords[loc];
	switch (selector) {
		case 1: 
			drawRect(x,y+26,13,14,ST7735_CYAN);
			break;
		case 2: 
			drawRect(x+13,y+26,13,14,ST7735_CYAN);
			break;
		case 3: 
			drawRect(x+26,y+26,13,14,ST7735_CYAN);
			break;
	}
}

void Math::moveSelector(int dir) {
	// 0 is left, 1 is right
	if (dir == 0) {
		if (selector != 1)
			selector--;
	} else if (dir == 1) {
		if (selector != 3)
			selector++;
	}
	drawModule(loc);
	Draw(0);
}

void Math::check(int s) {
	if (s == answer_location)
		solved = true;
	else
		b.addStrikes();
}

void Math::select() {
	check(selector);
	if (solved) {
		b.num_solved++;
		selected = false;
		drawModule(loc);
		Draw(0);
		drawSolved(loc);
		screen = 1;
	}
}

// **************Key_Init*********************
// Initialize switch inputs on PE3-0
// Input: none 
// Output: none
void Key_Init(void){ 
	SYSCTL_RCGCGPIO_R |= 0x38;
	volatile int i = 0;
	i++;
	i++;
	GPIO_PORTD_DIR_R |= 0x01;
	GPIO_PORTD_DEN_R |= 0x01;
	GPIO_PORTE_DIR_R &= ~(0x0F);
	GPIO_PORTE_DEN_R |= 0x0F;
	GPIO_PORTF_DIR_R &= ~(0x11);
	GPIO_PORTF_DEN_R |= 0x11;
}

// **************Key_In*********************
// Input from switch inputs PE3-0
// Input: none 
// Output: 0 to 15 depending on keys
//   0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2, 0x08 is just Key3
//  PE0 is up, PE1 is down, PE2 is left, PE3 is right
uint32_t Key_In(void){ 
	int i = 0;
	i |= (GPIO_PORTE_DATA_R&0x0F);
	return i;
}

uint32_t Key_InF(void) {
	int i = 0;
	i |= (GPIO_PORTF_DATA_R&0x11);
	return i;
}