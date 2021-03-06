//to do :
// - plutot faire des design generatifs, ce sera plus beau...
// - faire un truc avec les secondes et la division du temps !

#include <Superbe_rtc_DS1302.h>
#include <SPI.h>

#define BUTTON_DISABLE_PIN 3
#define BUTTON_UPDATE_PIN 4
#define RTC_CLK_PIN 5
#define RTC_DATA_PIN 6
#define RTC_CE_PIN 7
#define SLAVE_SELECT_PIN 10 //data sur D11 et clk sur D13
#define MOTOR_ENABLE_PIN 9

#define N_CLOCK 24
#define STEP_N 8
#define STEP 180*STEP_N

#define STATE_OFF 0
#define STATE_CLOCK 1
#define STATE_CLOCK2 2
#define STATE_FRAME 3
#define STATE_UPDATE 4

const PROGMEM uint16_t frame[] = {
  STEP/4, STEP/2, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP*3/4, STEP/2, 0, STEP/2, STEP/4, STEP/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP*3/4, STEP*3/4, 0, STEP/2, 0, STEP/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, STEP/4, STEP*3/4, 0, STEP*3/4
};

const PROGMEM uint16_t chiffres[] = {
  STEP/4, STEP/2, STEP*3/4, STEP/2,   0, STEP/2,   0, STEP/2,   0, STEP/4,   0, STEP*3/4,
  STEP*5/8, STEP*5/8, STEP*5/8, STEP/2, STEP*5/8, STEP*5/8,   0, STEP/2, STEP*5/8, STEP*5/8,   0,   0,
  STEP/4, STEP/4, STEP*3/4, STEP/2, STEP/4, STEP/2,   0, STEP*3/4,   0, STEP/4, STEP*3/4, STEP*3/4,
  STEP/4, STEP/4, STEP*3/4, STEP/2, STEP/4, STEP/4, STEP*3/4, STEP/2, STEP/4, STEP/4,   0, STEP*3/4,
  STEP/2, STEP/2, STEP/2, STEP/2,   0, STEP/4, STEP*3/4, STEP/2, STEP*5/8, STEP*5/8,   0,   0,
  STEP/4, STEP/2, STEP*3/4, STEP*3/4,   0, STEP/4, STEP*3/4, STEP/2, STEP/4, STEP/4,   0, STEP*3/4,
  STEP/4, STEP/2, STEP*3/4, STEP*3/4,   0, STEP/2, STEP*3/4, STEP/2,   0, STEP/4,   0, STEP*3/4,
  STEP/4, STEP/4, STEP*3/4, STEP/2, STEP*5/8, STEP*5/8,   0, STEP/2, STEP*5/8, STEP*5/8,   0,   0,
  STEP/4, STEP/2, STEP*3/4, STEP/2, STEP/4, STEP/2, STEP*3/4, STEP/2,   0, STEP/4,   0, STEP*3/4,
  STEP/4, STEP/2, STEP*3/4, STEP/2,   0, STEP/4,   0, STEP/2, STEP/4, STEP/4,   0, STEP*3/4
};

Rtc_DS1302 rtc;
byte stepBig[8] =    {B10000000, B10100000, B11100000, B01100000, B01110000, B01010000, B00010000, B10010000};
byte stepLittle[8] = {B00001000, B00001001, B00000001, B00000101, B00000111, B00000110, B00001110, B00001010};
//byte stepBig[4] =    {B10100000, B01100000, B01010000, B10010000};
//byte stepLittle[4] = {B00001010, B00001001, B00000101, B00000110};
unsigned int little[N_CLOCK];
unsigned int big[N_CLOCK];
unsigned int littleNext[N_CLOCK];
unsigned int bigNext[N_CLOCK];
int state;
unsigned long chrono = 0;
boolean settingsEnable = false;

void setup() {
  pinMode(BUTTON_DISABLE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UPDATE_PIN, INPUT_PULLUP);
  pinMode(SLAVE_SELECT_PIN, OUTPUT);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_ENABLE_PIN, LOW);
  rtc.begin(RTC_CLK_PIN, RTC_DATA_PIN, RTC_CE_PIN);
  Serial.begin(9600);
  Serial.println("hello clock24");
  for (int i = 0; i < N_CLOCK; i++) {
    little[i] = STEP / 2;
    big[i] = STEP / 2;
  }
  delay(2000);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
  state = STATE_CLOCK;
}

void loop() {
  if (!digitalRead(BUTTON_DISABLE_PIN)) {
    settingsEnable = true;
    Serial.println("disable");
    for (int i = 0; i < N_CLOCK; i++) {
      littleNext[i] = STEP / 2;
      bigNext[i] = STEP / 2;
    }
    moveLittleBig();
    digitalWrite(MOTOR_ENABLE_PIN, LOW);
    while (!digitalRead(BUTTON_DISABLE_PIN)) delay(10);
    delay(10);
    while (digitalRead(BUTTON_DISABLE_PIN)) delay(10);
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
    Serial.println("enable");
    settingsEnable = false;
    delay(10);
  }
  if (!digitalRead(BUTTON_UPDATE_PIN)) {
    Serial.println("update");
    settingsEnable = true;
    rtc.readTime();
    int hour = rtc.hour;
    int minute = rtc.minute;
    draw24clock(hour, minute);
    while (!digitalRead(BUTTON_UPDATE_PIN)) delay(10);
    delay(10);
    chrono = millis();
    while (millis() - chrono < 10000) {
      if (!digitalRead(BUTTON_UPDATE_PIN)) {
        chrono = millis();
        minute++;
        if (minute >= 60) {
          enleverUnTourMinute();
          minute = 0;
          hour++;
          if (hour >= 24) {
            hour = 0;
            enleverDeuxToursHeure();
          }
        }
      }
      else if (!digitalRead(BUTTON_DISABLE_PIN)) {
        chrono = millis();
        minute--;
        if (minute < 0) {
          ajouterUnTourMinute();
          minute = 59;
          hour--;
          if (hour < 0) {
            hour = 23;
            ajouterDeuxToursHeure();
          }
        }
      }
      draw24clock(hour, minute);
      delay(40);
    }
    rtc.writeTime(hour, minute, 0);
    Serial.print("updated ");
    Serial.print(hour);
    Serial.print(" : ");
    Serial.println(minute);
    settingsEnable = false;
    state = STATE_CLOCK;
    chrono = millis();
  }
  switch (state) {
    case STATE_CLOCK:
      rtc.readTime();
      int myChiffres[4];
      myChiffres[0] = rtc.hour / 10;
      myChiffres[1] = rtc.hour % 10;
      myChiffres[2] = rtc.minute / 10;
      myChiffres[3] = rtc.minute % 10;
      for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 3; i++) {
          littleNext[i*8+0+j*2] = pgm_read_word_near(chiffres + 12*myChiffres[j] + 0 + i*4);
          littleNext[i*8+1+j*2] = pgm_read_word_near(chiffres + 12*myChiffres[j] + 2 + i*4);
          bigNext[i*8+0+j*2] = pgm_read_word_near(chiffres + 12*myChiffres[j] + 1 + i*4);
          bigNext[i*8+1+j*2] = pgm_read_word_near(chiffres + 12*myChiffres[j] + 3 + i*4);
        }
      }
      moveLittleBig();
      if (millis() - chrono > 15000) {
        state = STATE_CLOCK2;
        chrono = millis();
      }
    break;
    case STATE_CLOCK2:
      rtc.readTime();
      draw24clockPlus(rtc.hour, rtc.minute);
      if (millis() - chrono > 15000) {
        state = STATE_FRAME;
        chrono = millis();
      }
    break;
    case STATE_FRAME:
      for (int i = 0; i < N_CLOCK; i++) {
        littleNext[i] = pgm_read_word_near(frame + i * 2);
        bigNext[i] = pgm_read_word_near(frame + i * 2 + 1);
      }
      moveLittleBig();
      if (millis() - chrono > 15000) {
        state = STATE_CLOCK;
        chrono = millis();
      }
    break;
    default:
      Serial.println("!!! problème de STATE");
    break;
  }
  delay(100);
}
