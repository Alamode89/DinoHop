#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

const unsigned short tasksNum = 2;
task tasks[tasksNum];

byte dino_sprite[8] 
{ B00110,
  B00101,
  B10111,
  B10110,
  B11110,
  B11111,
  B01010,
  B01010
};

byte bird_sprite[8] = {
  B00000,
  B00000,
  B00010,
  B01110,
  B11111,
  B00110,
  B00010,
  B00000
};

enum GAME{MENU, PLAY, COLLISION, GAME_OVER};
enum OBSTACLES{WAIT, UPDATE, STOP};

const int SW_pin = 9; //pin connected to button
const int MENU_SIZE = 2;
const int buzzer = 8;
const int dino = 7;
const int bird = 6;

int joyX;
int joyY;
int menuIndex = 0;
unsigned char isPlaying = 0;
unsigned char hit = 0;
unsigned char playerPos = 1;
unsigned char i;
int score = 0;
int jump = 0;
int gameSpeed = 6;
int gameTime = 0;
int cnt = 0;
int hitTime = 0;
int scoreCnt = 0;
unsigned char maxObjects = 5;
int numObstacles[5][2] = {{-1, -1},{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
int maxObsScreen = 0;
int randNum;

void setup() {
  lcd.begin(16,2);
  pinMode(A0, INPUT); //x-axis for joystick
  pinMode(A1, INPUT); //y-axis for joystick
  pinMode(A2, INPUT_PULLUP);
  pinMode(SW_pin, INPUT_PULLUP); //joystick press
  pinMode(buzzer, OUTPUT);
  lcd.createChar(dino, dino_sprite);
  lcd.createChar(bird, bird_sprite);
  Serial.begin(9600);
  
  unsigned char i = 0;
  tasks[i].state = MENU;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_GAME;
  i++;
  tasks[i].state = WAIT;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_OBSTACLES;
}

int TickFct_GAME(int state) {
  switch(state) {
    case MENU:
      if(menuIndex == 0 && digitalRead(A2) == HIGH) {
        isPlaying = 1;
        hit = 0;
        maxObsScreen = 0;
        score = 0;
        gameSpeed = 6;
        scoreCnt = 0;
        playerPos = 1;
        state = PLAY;
        tone(buzzer, 500); // Send 1KHz sound signal...
        delay(100);        // ...for 1 sec
        noTone(buzzer);     // Stop sound...
        delay(1000);        // ...for 1sec
      }
      else {
        state = MENU;
      }
      
      break;
      
    case PLAY: 
      lcd.clear();
      definePosition();
      if(digitalRead(A2) == HIGH) {
        isPlaying = 0;
        state = GAME_OVER;
      } 

      if(scoreCnt >= 50) {
        scoreCnt = 0;
        tone(buzzer, 750);
        delay(100);
        noTone(buzzer);
      }
      
      for(i = 0; i < maxObjects; i++) {
        if(numObstacles[i][1] == playerPos && numObstacles[i][0] == 1){
          lcd.setCursor(1, numObstacles[i][1]);
          lcd.write("X");
          hit = 1;
          state = COLLISION;
        }
      }
      
      break;

    case COLLISION:
      if(hitTime >= 30) {
        hitTime = 0;
        state = GAME_OVER;
      }
      hitTime++;
      break;
      
    case GAME_OVER:
      if(digitalRead(A2) == HIGH) {
        state = MENU;
      }
      lcd.clear();
      isPlaying = 0;
      lcd.setCursor(0, 1);
      lcd.print("SCORE: ");
      lcd.print(score);
      break;
      
    default:
      break;
  }

  switch(state) {
    case MENU:
      joyPos();
      lcd.clear();
      String menu[MENU_SIZE] = { "PLAY", "OPTIONS" };

      for (int i = 0; i < MENU_SIZE; i++) {
        if (i == menuIndex) {
          lcd.setCursor(0, i);
          lcd.write("-> ");
          if(i == 0) {
            lcd.setCursor(0,1);
            lcd.write("  ");
          }
          if(i == 1) {
            lcd.setCursor(0,0);
            lcd.write("  ");                      
          }
        }

      lcd.setCursor(3, i);
      lcd.print(menu[i]);
      }

      if(joyY >= 6) {
        menuIndex = 0;
          tone(buzzer, 500); // Send 1KHz sound signal...
          delay(100);        // ...for 1 sec
          noTone(buzzer);     // Stop sound...
          //delay(100);        // ...for 1sec
      }
      else if(joyY <= 2){
          menuIndex = 1;
          tone(buzzer, 500); // Send 1KHz sound signal...
          delay(100);        // ...for 1 sec
          noTone(buzzer);     // Stop sound...
          //delay(100);        // ...for 1sec
      }
      
      break;
      
    case PLAY:
      break;

    case COLLISION:
        hitTime++;
        lcd.setCursor(1, playerPos);
        lcd.write("X");
        break;
        
    case GAME_OVER:
        break;
           
    default:
      break;
  }
  return state;
}

int TickFct_OBSTACLES(int state) {
  switch(state) {
    case WAIT:
      if(isPlaying == 1) {
        state = UPDATE;
      }
      else {
        state = WAIT;
      }

      break;
      
    case UPDATE:
      if(hit == 1 || isPlaying == 0) {
        state = STOP;
      }
      else {
        lcd.setCursor(12, 0);
        lcd.print(score);
        for(i = 0; i < maxObjects; i++) {
            if(numObstacles[i][0] > -1) {
              lcd.setCursor(numObstacles[i][0], numObstacles[i][1]);
              lcd.write(bird);
            }
        }
        state = UPDATE;
      }

      break;
   
    case STOP:
      if(isPlaying == 1) {
         state = UPDATE;
      }
      else {
        state = STOP;
      }
      
    default:
      break;
      
  }

  switch(state) {
    case WAIT:
      break;
      
    case UPDATE:
      if(cnt >= gameSpeed) {
        cnt = 0;
        maxObsScreen++;
        score++;
        scoreCnt++;
        for(i = 0; i < maxObjects; i++){
           if(numObstacles[i][0] <= 0) {
              numObstacles[i][0] = -1;
              numObstacles[i][1] = -1;
           }
           else if(numObstacles[i][0] > 0){
              numObstacles[i][0] = numObstacles[i][0] - 1;
           }
        }
        if(gameSpeed > 3 && maxObsScreen % 8 == 0) { gameSpeed--; }
        if(maxObsScreen % 5 == 0){
           for(i = 0; i < maxObjects; i++) {
            if(numObstacles[i][0] <= 0){
              lcd.setCursor(0, numObstacles[i][1]);
              lcd.write(" ");
              numObstacles[i][0] = 16;
              randNum = random(0, 2);
              numObstacles[i][1] = randNum;
              break;
            }
          }
        }
      }
      cnt++;
      break;

    case STOP:
      for(i = 0; i < maxObjects; i++){
        numObstacles[i][0] = -1;
        numObstacles[i][1] = -1;
      }
      break;
      
    default:
      break;
  }

  return state;
}

void joyPos() {
  int x = analogRead(A0);
  int y = analogRead(A1);

  joyX = map(x, 0, 1023, 0, 7); //0 left, 7 right
  joyY = map(y, 0, 1023, 7, 0); //0 is down 7 up
}

void definePosition() {
  joyPos();
  if(joyY >= 6) { // top row
      playerPos = 0;
        tone(buzzer, 500); // Send 1KHz sound signal...
        delay(50);        // ...for 1 sec
        noTone(buzzer);     // Stop sound...
        //delay(1000);        // ...for 1sec
  }

  else if(joyY <= 2) { //bottom row
      playerPos = 1;
      tone(buzzer, 500); // Send 1KHz sound signal...
      delay(50);        // ...for 1 sec
      noTone(buzzer);     // Stop sound...
       // delay(1000);        // ...for 1sec
  }

  if(playerPos == 0 && jump <= 8) {
    jump++;
  }
  if(playerPos == 0 && jump > 8) {
    playerPos = 1;
    jump = 0;
  }
  
  lcd.setCursor(1, playerPos);
  lcd.write(dino);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) {
    if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = millis(); // Last time this task was ran
    }
  }
  delay(100);
}
