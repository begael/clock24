//to do :
// - faut-il inverser le sens de communication ?
// - plutot faire des design generatifs, ce sera plus beau...
// - peut-on éteindre les moteurs entre les changements ou pas ?
// - essayer 8 pas à la place de 4 (pour le bruit ? et ou la vitesse ?)
// - génération des frames
// - bouton remettre à l'heure

#include <Superbe_rtc_DS1302.h>

#define BUTTON_DISABLE_PIN 3
#define BUTTON_UPDATE_PIN 4
#define RTC_CLK_PIN 5
#define RTC_DATA_PIN 6
#define RTC_CE_PIN 7
#define SHIFTOUT_DATA_PIN 8
#define SHIFTOUT_CLOCK_PIN 9
#define SHIFTOUT_LATCH_PIN 10
#define MOTOR_ENABLE_PIN 11

#define LED_PIN 13
#define N_CLOCK 24
#define STEP 720

#define STATE_OFF 0
#define STATE_CLOCK 1
#define STATE_RANDOM 2
#define STATE_FRAME 3

const PROGMEM uint16_t frame[] = {
  0, 360, 20, 380, 40, 400, 60, 420, 80, 440, 100, 460, 120, 480, 140, 500, 0, 360, 20, 380, 40, 400, 60, 420, 80, 440, 100, 460, 120, 480, 140, 500,0, 360, 20, 380, 40, 400, 60, 420, 80, 440, 100, 460, 120, 480, 140, 500, 
  180, 540, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  540, 540, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const PROGMEM uint16_t chiffres[] = {
  180, 360, 540, 360,   0, 360,   0, 360,   0, 180,   0, 540,
  450, 450, 450, 360, 450, 450,   0, 360, 450, 450,   0,   0,
  180, 180, 540, 360, 180, 360,   0, 540,   0, 180, 540, 540,
  180, 180, 540, 360, 180, 180, 540, 360, 180, 180,   0, 540,
  360, 360, 360, 360,   0, 180, 540, 360, 450, 450,   0,   0,
  180, 360, 540, 540,   0, 180, 540, 360, 180, 180,   0, 540,
  180, 360, 540, 540,   0, 360, 540, 360,   0, 180,   0, 540,
  180, 180, 540, 360, 450, 450,   0, 360, 450, 450,   0,   0,
  180, 360, 540, 360, 180, 360, 540, 360,   0, 180,   0, 540,
  180, 360, 540, 360,   0, 180,   0, 360, 180, 180,   0, 540
};

Rtc_DS1302 rtc;
byte stepBig[4] = {B10100000, B01100000, B01010000, B10010000};
byte stepLittle[4] = {B00001010, B00001001, B00000101, B00000110};
unsigned int little[N_CLOCK];
unsigned int big[N_CLOCK];
unsigned int littleNext[N_CLOCK];
unsigned int bigNext[N_CLOCK];
int state = STATE_OFF;
unsigned long chrono = 0;

void setup() {
  pinMode(BUTTON_DISABLE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UPDATE_PIN, INPUT_PULLUP);
  pinMode(SHIFTOUT_LATCH_PIN, OUTPUT);
  digitalWrite(SHIFTOUT_LATCH_PIN, HIGH);
  pinMode(SHIFTOUT_CLOCK_PIN, OUTPUT);
  digitalWrite(SHIFTOUT_CLOCK_PIN, HIGH);
  pinMode(SHIFTOUT_DATA_PIN, OUTPUT);
  digitalWrite(SHIFTOUT_DATA_PIN, HIGH);
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_ENABLE_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  rtc.begin(RTC_CLK_PIN, RTC_DATA_PIN, RTC_CE_PIN);
  Serial.begin(9600);
  Serial.println("hello clock24");
  for (int i = 0; i < N_CLOCK; i++) {
    little[i] = STEP / 2;
    big[i] = STEP / 2;
  }
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  if (!digitalRead(BUTTON_DISABLE_PIN)) {
    Serial.println("disable");
    for (int i = 0; i < N_CLOCK; i++) {
      littleNext[i] = STEP / 2;
      bigNext[i] = STEP / 2;
    }
    moveLittleBig();
    digitalWrite(MOTOR_ENABLE_PIN, LOW);
    state = STATE_OFF;
    chrono = millis();
  }
  if (!digitalRead(BUTTON_UPDATE_PIN)) {
    Serial.println("update");
    rtc.readTime();
    for (int i = 0; i < N_CLOCK; i++) {
      littleNext[i] = (rtc.hour % 12) * STEP / 12;
      bigNext[i] = rtc.minute * STEP / 60;
    }
    moveLittleBig();
    state = STATE_OFF;
    chrono = millis();
  }
  switch (state) {
    case STATE_OFF:
      if (millis() - chrono > 10000) {
        digitalWrite(MOTOR_ENABLE_PIN, HIGH);
        state = STATE_CLOCK;
        chrono = millis();
      }
    break;
    case STATE_CLOCK:
      rtc.readTime();
      Serial.print(rtc.hour);
      Serial.println(rtc.minute);
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
        state = STATE_FRAME;
        chrono = millis();
      }
    break;
    case STATE_RANDOM:
      for (int i = 0; i < N_CLOCK; i++) {
        littleNext[i] = random(STEP);
        bigNext[i] = random(STEP);
      }
      moveLittleBig();
      if (millis() - chrono > 5000) {
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
  }
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
