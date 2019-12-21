#include "Particle.h"
#include "Dimmer.h"
#include "E131.h"



struct eeprom_data
{
	uint8_t numChannels;	// max 10
	uint8_t channels[10];	// which dmx channel
	uint8_t pins[10];		// which pin to output on
};

// MUST USE D5, D6, D7, A2, WKP, TX, RX as ZC_PIN
const int PIN_ZC_IN   = D5;


const int numOutputs = 6;
int outputPins[] = {D6, D4, D3, D2, D1, D0};

E131 e131;

void setup()
{
  Particle.publish("lightshow/ip", WiFi.localIP().toString(), PRIVATE);
  ZCDimmer::getInstance()->begin(PIN_ZC_IN, outputPins, numOutputs);
  e131.begin();
}

int count = 0;

void loop()
{
  e131.parsePacket();
  ZCDimmer::getInstance()->setBrightness(0, e131.data[1]);
  ZCDimmer::getInstance()->setBrightness(1, e131.data[2]);

  if (count %100 == 0)
  {
  	// Serial.printlnf("loop %d", e131.data[3]);
  }
}
