void setupSD() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  while (!SD.begin(SD_CS)) {
    Serial.println("Error talking to SD card!");
    delay(5000);
  }

  uint8_t cardType = SD.cardType();
  while (cardType == CARD_NONE) {
    Serial.println("No SD card found!");
    delay(5000);
  }

  if (DEBUG) displayCardInfos(cardType);
}

void displayCardInfos(uint8_t cardType) {
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if(cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void getAudioFiles(fs::FS &fs, const char *directory, String *files,int *filesNb, boolean mp3Only) {
  File root = fs.open(directory);
  if (!root) {
    Serial.println("Can't open directory");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("This is not a directory");
    return;
  }

  File file = root.openNextFile();
  int foundFiles = 0;
  while (file && foundFiles < 10) {
    if (!file.isDirectory()) {
      if (mp3Only && strstr(file.name(), "mp3") || !mp3Only) {
        files[foundFiles] = file.name();
        foundFiles++;
      }
    }
    file = root.openNextFile();
  }
  *filesNb = foundFiles;
}

void loadDataChunk(String lineBuffer, DanceData *data, int *currentLine) {
  int index = lineBuffer.indexOf(" ");
  uint32_t timestamp = (lineBuffer.substring(0, index)).toInt();
  lineBuffer = lineBuffer.substring(index + 1);
  char type = lineBuffer.charAt(0);
  if (type == 'S') { // Stop motors instruction
    DanceData newData;
    newData.timestamp = timestamp;
    newData.stopCMD = true;
    newData.moveHead = false;
    newData.duty = 0;
    data[*currentLine] = newData;
    (*currentLine)++;
  } else {
    index = lineBuffer.indexOf(" ");
    uint8_t duty = (uint8_t)((lineBuffer.substring(index)).toInt());
    DanceData newData;
    newData.timestamp = timestamp;
    newData.stopCMD = false;
    newData.moveHead = type == 'H';
    newData.duty = duty;
    data[*currentLine] = newData;
    (*currentLine)++;
  }
}

DanceData *loadDanceData(fs::FS &fs) {
  int fileNameLength = files[currentMusic].length();
  String baseName = files[currentMusic].substring(0, fileNameLength - 4); // Strip the ".mp3"
  baseName.concat(".txt");

  DanceData *data = NULL;
  int currentLine = 0;
  File dataFile = fs.open(baseName);
  if (dataFile) {
    size_t fileSize = dataFile.size();
    if (fileSize == 0) return data;

    String lineBuffer = "";
    int lines = 0;
    boolean readLinesNb = false;
    while (dataFile.available()) {
      for (int lastPos = 0; lastPos < fileSize; lastPos++) {
        char character = dataFile.read();
        if (character == 10) {
          if (!readLinesNb) {
            lines = lineBuffer.toInt();
            data = (DanceData*)malloc(lines * sizeof(DanceData));
            danceDataLength = lines;
            readLinesNb = true;
          } else {
            loadDataChunk(lineBuffer, data, &currentLine);
          }
          lineBuffer = "";
        } else {
          lineBuffer = lineBuffer + character;
        }
      }
      loadDataChunk(lineBuffer, data, &currentLine);
    }
  }

  return data;
}

void cleanDanceData(DanceData *data) {
  free(data);
  danceDataLength = 0;
  danceStep = 0;
}
