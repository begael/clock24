void ajouterDeuxToursHeure() {
  for (int i = 0; i < N_CLOCK; i++) {
    little[i] += 2 * STEP;
  }
}

void enleverDeuxToursHeure() {
  for (int i = 0; i < N_CLOCK; i++) {
    little[i] -= 2 * STEP;
  }
}

void ajouterUnTourMinute() {
  for (int i = 0; i < N_CLOCK; i++) {
    big[i] += STEP;
  }
}

void enleverUnTourMinute() {
  for (int i = 0; i < N_CLOCK; i++) {
    big[i] -= STEP;
  }
}

void draw24clock(int h, int m) {
  for (int i = 0; i < N_CLOCK; i++) {
    littleNext[i] = h * STEP / 12 + m * STEP / 12 / 60;
    bigNext[i] = m * STEP / 60;
  }
  moveLittleBig();
}

void draw24clockPlus(int h, int m, int s) {
  for (int i = 0; i < N_CLOCK; i++) {
    int temp = map(i, 0, N_CLOCK-1, h * 5 + m / 12, m);
    littleNext[i] = temp * (STEP / 60);
    temp = map(i, 0, N_CLOCK-1, m, s);
    bigNext[i] = temp * (STEP / 60);
  }
  moveLittleBig();
}

void moveLittleBig() {
  boolean go = true;
  while (go) {
    go = false;
    for (int i = 0; i < N_CLOCK; i++) {
      if (littleNext[i] > little[i]) little[i]++;
      if (littleNext[i] < little[i]) little[i]--;
      if (bigNext[i] > big[i]) big[i]++;
      if (bigNext[i] < big[i]) big[i]--;
      if ((littleNext[i] != little[i]) || (bigNext[i] != big[i])) go = true;
    }
    sendShiftOut();
    if (!settingsEnable) {
      if (!digitalRead(BUTTON_DISABLE_PIN) || !digitalRead(BUTTON_UPDATE_PIN)) go = false;
    }
  }
}

void sendShiftOut() {
  digitalWrite(SLAVE_SELECT_PIN, LOW);
  for (int i = 0; i < 8; i++) SPI.transfer(stepLittle[little[23-i] % STEP_N] | stepBig[big[23-i] % STEP_N]);
  for (int i = 15; i > 7; i--) SPI.transfer(stepLittle[little[23-i] % STEP_N] | stepBig[big[23-i] % STEP_N]);
  for (int i = 16; i < 24; i++) SPI.transfer(stepLittle[little[23-i] % STEP_N] | stepBig[big[23-i] % STEP_N]);
  delayMicroseconds(CHRONO2);
  digitalWrite(SLAVE_SELECT_PIN, HIGH);
  delayMicroseconds(CHRONO2);
}
