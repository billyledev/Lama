// Duty cycle ranges from 150 to 255

void setupMotors() {
  pinMode(HEAD_MOTOR, OUTPUT);
  pinMode(TAIL_MOTOR, OUTPUT);

  ledcSetup(pwmChannel1, freq, resolution);
  ledcAttachPin(HEAD_MOTOR, pwmChannel1);
  ledcSetup(pwmChannel2, freq, resolution);
  ledcAttachPin(TAIL_MOTOR, pwmChannel2);
}

void moveHead(uint8_t duty) {
  ledcWrite(pwmChannel1, duty);
  ledcWrite(pwmChannel2, 0);
}

void moveTail(uint8_t duty) {
  ledcWrite(pwmChannel1, 0);
  ledcWrite(pwmChannel2, duty);
}

void stopMoving() {
  ledcWrite(pwmChannel1, 0);
  ledcWrite(pwmChannel2, 0);
}
