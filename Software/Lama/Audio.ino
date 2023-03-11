void setupAudio() {
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // 0...21
  audio.forceMono(true);
}

void audio_eof_mp3(const char *info) {
  playing = false;
  stopMoving();
  lastAudioTime = -1;
  cleanDanceData(danceData);
  currentMusic++;
  if (currentMusic >= filesNb) {
    currentMusic = 0;
  }
}
