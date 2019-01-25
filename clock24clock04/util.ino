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
    delay(1);
  }
}

void sendShiftOut() {
  digitalWrite(SHIFTOUT_LATCH_PIN, LOW);
  for (int i = 0; i < 8; i++) shiftOut(SHIFTOUT_DATA_PIN, SHIFTOUT_CLOCK_PIN, MSBFIRST, stepLittle[little[i] % 4] | stepBig[big[i] % 4]);
  for (int i = 15; i > 7; i--) shiftOut(SHIFTOUT_DATA_PIN, SHIFTOUT_CLOCK_PIN, MSBFIRST, stepLittle[little[i] % 4] | stepBig[big[i] % 4]);
  for (int i = 16; i < 24; i++) shiftOut(SHIFTOUT_DATA_PIN, SHIFTOUT_CLOCK_PIN, MSBFIRST, stepLittle[little[i] % 4] | stepBig[big[i] % 4]);
  digitalWrite(SHIFTOUT_LATCH_PIN, HIGH);
}
