#include <SPI.h>
#include <SD.h>

File root;

struct file_entry{
  String name;
  bool folder;
};

file_entry start[100]; 

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(23)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  root = SD.open("/binaries");

  printDirectory(root, 0);
  for(int j = 0; j<100; j++){
    if(start[j].name != ""){
      Serial.println(start[j].name);
    }
  }
  Serial.println("done!");
}


void printDirectory(File dir, int numTabs) {
  int i = 0;
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    start[i].name = entry.name();
    for (uint8_t i = 0; i < numTabs; i++) {
    }
    if (entry.isDirectory()) {
      start[i].folder = true;
      printDirectory(entry, numTabs + 1);
    } else {
      start[i].folder = false;
      // files have sizes, directories do not
    }
    i++;
    entry.close();
  }
}