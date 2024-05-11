#include <LittleFS.h>

const uint16_t STARTING_OFFSET = 44;
const uint16_t SAMPLE_LENGTH = 1200;
static int samples[600][10];

void readWT(String filename, int where)
{
  byte buffer[2];
  uint16_t position;

  if (!LittleFS.begin())
  {
    DEBUG_PRINTLN("An Error has occurred while mounting LittleFS");
    return;
  }
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DEBUG_PRINTLN("Failed to open file for reading");
    return;
  }

  file.seek(STARTING_OFFSET);
  position = 0;
  while (file.available() and file.position() < STARTING_OFFSET + SAMPLE_LENGTH)
  {
    file.read(buffer, 2);
    uint16_t n = (buffer[0] | buffer[1] << 8) + (1 << 15);
    samples[position][where] = n >> 4;
    position++;
  }
  file.close();
}

// link to all waveforms (its visual representation) -
// https://www.adventurekid.se/AKRTfiles/AKWF/view/waveforms_index.html
void initWaveTables()
{
  readWT("/AKWF_0001/AKWF_0001.wav", 0);
  readWT("/AKWF_0001/AKWF_0002.wav", 1);
  readWT("/AKWF_0001/AKWF_0003.wav", 2);
  readWT("/AKWF_0001/AKWF_0049.wav", 3);
  readWT("/AKWF_0001/AKWF_0025.wav", 4);
  readWT("/AKWF_0001/AKWF_0084.wav", 5);
  readWT("/AKWF_0001/AKWF_0073.wav", 6);
  readWT("/AKWF_0001/AKWF_0026.wav", 7);
  readWT("/AKWF_0001/AKWF_0026.wav", 8);
  readWT("/AKWF_0001/AKWF_0010.wav", 9);
}
