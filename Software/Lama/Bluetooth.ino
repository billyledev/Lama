class FileActionCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String((pCharacteristic->getValue()).c_str());

    if (value.length() > 0) {
      int index = 0;
      if ((index = value.indexOf("delete")) != -1) {
        deleteFile(value.substring(index + 7));
      } else if ((index = value.indexOf("create")) != -1) {
        createFile(value.substring(index + 7));
      } else if (value.indexOf("end") != -1) {
        endUpload();
      }
    } else {
      
    }
  }
};

class RemoteActionCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = String((pCharacteristic->getValue()).c_str());

    if (value.length() > 0) {
      int index = 0;
      if (value.indexOf("next") != -1) {
        if (!playing) playNextMusic = true;
      } else if ((index = value.indexOf("play")) != -1) {
        if (!playing) play(value.substring(index + 5));
      } else if (value.indexOf("list") != -1) {
        listFiles();
      }
    } else {
      
    }
  }
};

void setupBluetooth() {
  BLEDevice::init("Lama");
  BLEServer *pServer = BLEDevice::createServer();

//  BLEService *pFileService = pServer->createService(FILE_SERVICE_UUID);
//  pFileActionCharacteristic = pFileService->createCharacteristic(
//                                              FILE_ACTION_UUID,
//                                              BLECharacteristic::PROPERTY_WRITE
//                                            );
//  pFileActionCharacteristic->setCallbacks(new FileActionCallback());
//  pFileInfoCharacteristic = pFileService->createCharacteristic(
//                                            FILE_INFO_UUID,
//                                            BLECharacteristic::PROPERTY_READ
//                                          );
//  pFileContentCharacteristic = pFileService->createCharacteristic(
//                                               FILE_CONTENT_UUID,
//                                               BLECharacteristic::PROPERTY_WRITE
//                                             );
//  pFileInfoCharacteristic->setValue("Hello world from file info characteristic!");
//  pFileService->start();

  BLEService *pRemoteService = pServer->createService(REMOTE_SERVICE_UUID);
  pRemoteActionCharacteristic = pRemoteService->createCharacteristic(
                                                  REMOTE_ACTION_UUID,
                                                  BLECharacteristic::PROPERTY_WRITE
                                                );
  pRemoteActionCharacteristic->setCallbacks(new RemoteActionCallback());
  pRemoteResultCharacteristic = pRemoteService->createCharacteristic(
                                                  REMOTE_RESULT_UUID,
                                                  BLECharacteristic::PROPERTY_READ
                                                );
  pRemoteService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(FILE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void listFiles() {
  String filesList = "";
  for (int i = 0; i < filesNb; i++) {
    filesList.concat(files[i]);
    filesList.concat(" ");
  }
  pRemoteResultCharacteristic->setValue(filesList.c_str());
}

void deleteFile(String file) {
  if (SD.exists(file)) {
    SD.remove(file);
    pFileInfoCharacteristic->setValue("File deleted!");
  } else {
    pFileInfoCharacteristic->setValue("File doesn't exists!");
  }
}

void createFile(String file) {
  if (uploading == true) {
    return;
  }
  Serial.print("Creating file : ");
  Serial.println(file);
  uploading = true;
  if (!SD.exists(file)) {
    uploadedFile = SD.open(file, FILE_WRITE);
    uploadedFile.println("Hello World!");
    pFileInfoCharacteristic->setValue("File created!");
  } else {
    pFileInfoCharacteristic->setValue("File already exists!");
  }
}

void endUpload() {
  if (uploading == false) {
    return;
  }
  uploading = false;
  pFileInfoCharacteristic->setValue("Finished uploading!");
  uploadedFile.close();
}
