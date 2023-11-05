#include "audiodata.h"  // wav data with header 
// Audio dac 
#include <driver/dac.h>
#define LED_PIN 33

//wav parameter
uint32_t chunkSize;
uint16_t numChannels;
uint32_t sampleRate;
uint16_t bitsPerSample;

uint8_t  data8;
uint16_t data16;

uint8_t  left;
uint16_t delayus;

float volumeScale = 1.0;
int soundeffect = 1;

void setup()
{
  Serial.begin(9600);
  Serial.println("start");
  dac_output_enable(DAC_CHANNEL_1);
  pinMode(LED_PIN, OUTPUT);
}

int play(const unsigned char *audio, uint32_t length ) {
  uint32_t count = 0;

  if(strncmp((char*)audio, "RIFF", 4)){
    Serial.println( "Error: It is not RIFF.");
    return -1; 
  }

  if(strncmp((char*)audio + 8, "WAVE", 4)){
    Serial.println( "Error: It is not WAVE.");
    return -1; 
  }

  if(strncmp((char*)audio + 12, "fmt ", 4)){
    Serial.println( "Error: fmt not found.");
    return -1; 
  }

  memcpy(&chunkSize, (char*)audio + 16, sizeof(chunkSize));
  memcpy(&numChannels, (char*)audio + 22, sizeof(numChannels));
  memcpy(&sampleRate, (char*)audio + 24, sizeof(sampleRate));
  memcpy(&bitsPerSample, (char*)audio + 34, sizeof(bitsPerSample));
  
  count = 16 + sizeof(chunkSize) + chunkSize;

  //skip to the data chunk
  while (strncmp((char*)audio + count , "data", 4)) {
    count += 4;
    memcpy(&chunkSize, (char*)audio + count, sizeof(chunkSize));   
    count += sizeof(chunkSize) + chunkSize;
    if ( count > length ) return -1;
  }
  count += 4 + sizeof(chunkSize);

  delayus = 1000000/sampleRate;

  while (count < length) {
    if (bitsPerSample == 16) {
      memcpy(&data16, (char*)audio + count, sizeof(data16));
      left = ((uint16_t) data16 + 32767) >> 8;
      left = left * volumeScale;  // 音量を調整
      count += sizeof(data16);
      if (numChannels == 2) count += sizeof(data16);
    } else {
      memcpy(&data8, (char*)audio + count, sizeof(data8));
      left = data8;
      left = left * volumeScale;  // 音量を調整
      count += sizeof(data8);
      if (numChannels == 2) count += sizeof(data8);
    }
    dac_output_voltage(DAC_CHANNEL_1, left);
    
    ets_delay_us(delayus);
  }
  dac_output_voltage(DAC_CHANNEL_1, 127);

}

void loop()
{
  float sensorValue = analogRead(A17);
  sensorValue = sensorValue/4096*200;
  Serial.print("weight = ");
  Serial.print(sensorValue);
  Serial.println(" N");
  delay(100);

  if (sensorValue < 1) {
    digitalWrite(LED_PIN, HIGH);
    if (soundeffect == 0) {
      play(Mono22Khz, sizeof(Mono22Khz));
      soundeffect = 1;
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    soundeffect = 0;
  }
}