void performUpdate(fs::FS &fs) {
  File updateBin = fs.open(UPDATE_FILE_PATH);
  if (DEBUG) Serial.println("Opening update file...");
  if (updateBin) {
    if (!updateBin.isDirectory()) {
      if (DEBUG) Serial.println("Update file found!");
      size_t updateSize = updateBin.size();

      if (updateSize > 0) {
        if (DEBUG) Serial.println("Starting update...");
        launchUpdate(updateBin, updateSize);
      }
    }

    updateBin.close();
    fs.remove(UPDATE_FILE_PATH);
  }
}

void launchUpdate(Stream &updateSource, size_t updateSize) {
  if (Update.begin(updateSize)) {
    Update.writeStream(updateSource);
    Update.end();
    if (DEBUG) Serial.println("Update finished!");
  }
}
