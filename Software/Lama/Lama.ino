#include <Arduino.h>
#include <Update.h>
#include <Audio.h>
#include <SD.h>
#include <FS.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
 
// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      21 // SD Card
#define SPI_MISO      19
#define SPI_SCK       18
 
#define I2S_DOUT      25
#define I2S_BCLK      27 // I2S
#define I2S_LRC       26

#define HEAD_MOTOR    22
#define TAIL_MOTOR    23

#define MUSIC_BTN     15 // Next music button

#define FILE_SERVICE_UUID   "85dff000-c85b-44d9-9417-3d8917a8f0f0"
#define FILE_ACTION_UUID    "85dff001-c85b-44d9-9417-3d8917a8f0f0"
#define FILE_INFO_UUID      "85dff002-c85b-44d9-9417-3d8917a8f0f0"
#define FILE_CONTENT_UUID   "85dff003-c85b-44d9-9417-3d8917a8f0f0"

#define REMOTE_SERVICE_UUID "92844c00-d860-46d8-8c82-ac450962df16"
#define REMOTE_ACTION_UUID  "92844c01-d860-46d8-8c82-ac450962df16"
#define REMOTE_RESULT_UUID  "92844c02-d860-46d8-8c82-ac450962df16"

#define UPDATE_FILE_PATH "/system/update.bin"

#define DEBUG true

typedef struct {
  uint32_t timestamp;
  boolean stopCMD; // true to stop motor, false to move head or tail
  boolean moveHead; // true for head, false for tail
  uint8_t duty; // the motor duty cycle
} DanceData;
 
Audio audio;
String files[100];

const int freq = 30000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;

int currentMusic = 0;
int filesNb;
DanceData *danceData;
int danceDataLength = 0;
int danceStep = 0;
boolean playing = false;
boolean playNextMusic = false;
uint32_t lastAudioTime = -1;

volatile unsigned long milliseconds = 0;
volatile unsigned long lastTime = 0;
hw_timer_t *timeTracker = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

boolean uploading = false;

BLECharacteristic *pFileActionCharacteristic;
BLECharacteristic *pFileInfoCharacteristic;
BLECharacteristic *pFileContentCharacteristic;

BLECharacteristic *pRemoteActionCharacteristic;
BLECharacteristic *pRemoteResultCharacteristic;

File uploadedFile;

void playNext() {
  playing = true;
  lastTime = milliseconds;
  danceData = loadDanceData(SD);
  if (DEBUG) {
    Serial.print("Playing ");
    Serial.print(files[currentMusic].c_str());
    Serial.println("...");
  }
  audio.connecttoFS(SD, files[currentMusic].c_str());
}

void play(String music) {
  playing = true;
  lastTime = milliseconds;
  danceData = loadDanceData(SD);
  if (DEBUG) {
    Serial.print("Playing ");
    Serial.print(music.c_str());
    Serial.println("...");
  }
  audio.connecttoFS(SD, music.c_str());
}

void IRAM_ATTR onTime() {
  portENTER_CRITICAL_ISR(&timerMux);
  milliseconds++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  if (DEBUG) Serial.begin(115200);

  WiFi.mode(WIFI_MODE_NULL);

  setupSD();
  performUpdate(SD);
  setupAudio();

  setupMotors();
  pinMode(MUSIC_BTN, INPUT_PULLUP);
  setupBluetooth();

  timeTracker = timerBegin(0, 80, true);
  timerAttachInterrupt(timeTracker, &onTime, true);
  timerAlarmWrite(timeTracker, 1000, true);
  timerAlarmEnable(timeTracker);

  filesNb = 0;
  getAudioFiles(SD, "/", files, &filesNb, true);
}
 
void loop() {
  if (playing) {
    audio.loop();

    uint32_t currentAudioTime = milliseconds - lastTime;
    if (lastAudioTime != currentAudioTime) {
      if (danceData[danceStep].timestamp <= currentAudioTime) {
        if (playing == false) return;
        if (danceData[danceStep].stopCMD) {
          stopMoving();
        } else if (danceData[danceStep].moveHead) {
          moveHead(danceData[danceStep].duty);
        } else {
          moveTail(danceData[danceStep].duty);
        }
        danceStep++;
        lastAudioTime = currentAudioTime;
      }
    }
  } else if (digitalRead(MUSIC_BTN) == LOW || playNextMusic) {
    playNextMusic = false;
    playNext();
  }
}
