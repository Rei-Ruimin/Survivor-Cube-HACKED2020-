#define SD_CS 10
#define JOY_VERT  A9  // should connect A9 to pin VRx
#define JOY_HORIZ A8  // should connect A8 to pin VRy
#define JOY_SEL   53

#include <Arduino.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include <SD.h>
//#include <cstdlib.h>
//#include <algorithm>
using namespace std;

MCUFRIEND_kbv tft;

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320

#define JOY_CENTER   512
#define JOY_DEADZONE 200

#define CURSOR_SIZE 9
char Gametitle[] = {
  'H','A','C','K','E','D','2','0','2','0',
};

int cursorX, cursorY, old_ver_pos = 1, old_hor_pos = 0;


void setup() {
  init();

  Serial.begin(9600);

	pinMode(JOY_SEL, INPUT_PULLUP);

	//    tft.reset();             // hardware reset
  uint16_t ID = tft.readID();    // read ID from display
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield

  // must come before SD.begin() ...
  tft.begin(ID);

	tft.setRotation(1);

  cursorX = (DISPLAY_WIDTH - 60)/2;
  cursorY = DISPLAY_HEIGHT/2;
}

int processJoystick() {
  int xVal = analogRead(JOY_HORIZ);
  int yVal = analogRead(JOY_VERT);
  int buttonVal = digitalRead(JOY_SEL);

    if (yVal < JOY_CENTER - JOY_DEADZONE) {
      // up
      return 2;
    }
    else if (yVal > JOY_CENTER + JOY_DEADZONE) {
      // down
      return 1;
    }
    // remember the x-reading increases as we push left
    if (xVal > JOY_CENTER + JOY_DEADZONE) {
      //left
      return 3;
    }
    else if (xVal < JOY_CENTER - JOY_DEADZONE) {
      //right
      return 4;
    }
    return 0;
}


void up_down_cursor() {
  if(processJoystick() == 1 && old_ver_pos == 2) {
    tft.fillRect(31, 110, 70, 70, TFT_BLACK);
    tft.fillRect(31, 220, 70, 70,TFT_RED);
    old_ver_pos = 1;
  }
  else if(processJoystick() == 2 && old_ver_pos == 1) {
    tft.fillRect(31, 220, 70, 70, TFT_BLACK);
    tft.fillRect(31, 110, 70, 70,TFT_RED);
    old_ver_pos = 2;
  }
}

void SetInitGameEnter(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(31,30);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(7);
  tft.print("HACKED2020");
  //
  tft.setCursor(10+42*6,30);
  //tft.setTextColor(TFT_WHITE,TFT_RED);
  //tft.print("A");
  tft.drawFastHLine(0,DISPLAY_HEIGHT-30,DISPLAY_WIDTH,TFT_RED);
  tft.fillRect(31,220,70,70,TFT_RED);
  //tft.fillRect(31,110,70,70,TFT_RED);

  tft.setCursor(30,120);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(5);
  tft.println("PUSH TO START!");
  while(processJoystick() == 0){
    tft.fillRect(30, 120, 14*6*5, 8*5, TFT_BLACK);
    delay(10);
    tft.setCursor(30,120);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(5);
    tft.println("PUSH TO START!");
    delay(150);
  }

  tft.fillRect(30, 120, 14*6*5, 8*5, TFT_BLACK);
  int consts[3] = {'3', '2', '1'};
  for(int i = 0; i < 3; i++) {
  tft.drawChar(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 40, consts[i], TFT_RED, TFT_BLACK, 7);
  delay(600);
  }
  tft.fillRect(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 40, 42, 56, TFT_BLACK);
}


void lower_obs(int cursorX) {
    int obstacel_size = 70;
    int cursorY = 220;
    tft.fillRect(cursorX+15, cursorY, obstacel_size+15, obstacel_size, TFT_BLACK);
    tft.fillTriangle(cursorX, cursorY+68, cursorX+34, cursorY, cursorX+68, cursorY+68, TFT_WHITE);
  }

void upper_obs(int cursorX) {
    int obstacel_size = 70;
    int cursorY = 110;
    tft.fillRect(cursorX+15, cursorY, obstacel_size+15, obstacel_size+2, TFT_BLACK);
    tft.fillTriangle(cursorX, cursorY+2, cursorX+34, cursorY+70, cursorX+68, cursorY+2, TFT_WHITE);
}

void character(char Z, int cursorX) {
  int cursorY = 290-48;
  tft.fillRect(cursorX+36, cursorY, 15,48 , TFT_BLACK);
  tft.drawChar(cursorX, cursorY, Z, TFT_WHITE, TFT_BLACK, 6);
}

void MainGame(bool &alive) {
  SetInitGameEnter();
  int remain_char = 10;
  int subarray[10] = {0};
  int cursorX;

  while(alive && remain_char!=0) {
    Serial.println(remain_char);
      int rand_obs = rand()%3 + 1;
      if(rand_obs == 3) {
        // left
        bool get_char = false;
        int rand_char_i = rand()%10;
        while(subarray[rand_char_i] == 1) {
          rand_char_i = rand()%10;
        }
        char rand_char = Gametitle[rand_char_i];
        int cursorX = DISPLAY_WIDTH;
        while(cursorX > -15-36) {
          old_hor_pos = processJoystick();
          up_down_cursor();
          
          character(rand_char, cursorX);
          if (31 < cursorX && cursorX < 31 + 70 && old_hor_pos == 3) {
            tft.drawChar(31 + 42*rand_char_i, 30, rand_char, TFT_WHITE, TFT_RED, 7);
            get_char = true;
          }
          cursorX -= 10;
          delay(10);
        }
        if ( get_char ){
          subarray[rand_char_i] = 1;
          remain_char--;
        }
      }
      else if(rand_obs == 1) {
        // down
        cursorX = DISPLAY_WIDTH;
        while(cursorX > -100) {
          up_down_cursor();
          upper_obs(cursorX);
          if(31 < cursorX && cursorX < 31+70 && old_ver_pos == 2) {
            alive = false;
            break;
          } 

          cursorX -= (rand()%3 + 4)*5;
          delay(10);
        }
      }
      else if(rand_obs == 2) {
        // up
        cursorX = DISPLAY_WIDTH;
        while(cursorX > -100) {
          up_down_cursor();
          lower_obs(cursorX);
          if(31 < cursorX && cursorX < 31+70 && old_ver_pos == 1) {
            alive = false;
            break;
          } 

          cursorX -= (rand()%3 + 4)*5;
          delay(10);
        }
      }
      if(old_ver_pos == 1) {
        tft.fillRect(31, 110, 70, 70, TFT_BLACK);
        tft.fillRect(31, 220, 70, 70,TFT_RED);
      }
      else if(old_ver_pos == 2) {
        tft.fillRect(31, 220, 70, 70, TFT_BLACK);
        tft.fillRect(31, 110, 70, 70,TFT_RED);
      }
  }
}

void FinishCeremony(bool alive){
  if(alive){
    tft.fillScreen(TFT_RED);
    delay(20);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(31,30);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);
    tft.setTextSize(7);
    tft.print("HACKED2020");
    tft.setCursor(60,110);
    tft.setTextSize(4);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);
    tft.println("Congratulation!");
    tft.setCursor(30,160);
    tft.println("You Just Finished");
    tft.setCursor(70,200);
    tft.print("OUR TRASH GAME");
    tft.print((char)3);
    while(processJoystick() == 0){
      tft.setCursor(60,110);
      tft.setTextColor(TFT_BLACK,TFT_BLACK);
      tft.print("Congratulation!");
      delay(30);
      tft.setCursor(60,110);
      tft.setTextColor(TFT_GREEN,TFT_BLACK);
      tft.print("Congratulation!");
      delay(70);
    }
  }
  else if(alive == false){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60,30);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);
    tft.setTextSize(7);
    tft.println("Oooops!!!");
    tft.setCursor(235,100);
    tft.println((char)1);
    tft.setCursor(120,160);
    tft.setTextSize(4);
    tft.println("Try-Again?");
    tft.setCursor(50,200);
    tft.println("Push Joystick to");
    tft.setCursor(140,250);
    tft.print("Re-Play");
    tft.print((char)3);
    while(processJoystick()==0){}
}}

// provided in eclass
int main() {
	setup();
  /*
  while(true){
    Serial.print(processJoystick());
  }*/
  randomSeed(rand()%100);
  while(true){
    old_ver_pos = 1;
    processJoystick();
    bool alive = true;
    MainGame(alive);
    FinishCeremony(alive);
    
    while(processJoystick() != 0){ break; }
  }

  Serial.end();
  return 0;
}
