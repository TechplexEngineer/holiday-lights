#include "Particle.h"
#include "Dimmer.h"
#include "E131.h"
#include <algorithm>

PRODUCT_ID(10541);
PRODUCT_VERSION(5);




struct eeprom_data
{
	uint8_t numChannels;	// max 10
	uint8_t channels[10];	// which dmx channel
	uint8_t pins[10];		// which pin to output on
};

// MUST USE D5, D6, D7, A2, WKP, TX, RX as ZC_PIN
const int PIN_ZC_IN   = D5;


uint16_t numOutputs = 1;
int dmxStart = 1;
int outputPins[] = {D6, D4, D3, D2, D1, D0};

E131 e131;

const String lights_bed     = "2c0044000447343337373738";
const String lights_sw      = "18002e000a47353235303037";
const String lights_office  = "410026000d51353432383931";
const String lights_outside = "390043000b47353235303037";
const String lights_great   = "35002a000f51353532343635";

bool needReset = false;

int cloudDeviceReset(String none)
{
  Serial.printf("\n\n------------ Cloud Function: devReset: ------------\n");
  needReset = true;
  return 0;
}

void setup()
{
  Serial.begin();
  Particle.publish("lightshow/ip", WiFi.localIP().toString(), PRIVATE);

  Particle.function("devReset", cloudDeviceReset);

  const String deviceID = System.deviceID();
  if (deviceID == lights_bed) {
    WiFi.setHostname("lights_bed");
    // 192.168.1.201
    numOutputs = 1;
  }
  else if (lights_sw == deviceID) {
    WiFi.setHostname("lights_sw");
    // 192.168.1.202
    numOutputs = 2;
  }
  else if (lights_office == deviceID) {
    WiFi.setHostname("lights_office");
    // 192.168.1.203
    numOutputs = 3;
  }
  else if (lights_outside == deviceID) {
    WiFi.setHostname("lights_outside");
    // 192.168.1.204
    numOutputs = 4;
  }
  else if (lights_great == deviceID) {
    WiFi.setHostname("lights_great");
    // 192.168.1.205
    numOutputs = 2;
  }

  ZCDimmer::getInstance()->begin(PIN_ZC_IN, outputPins, numOutputs);
  e131.beginUnicast();
}

int count = 0;

void loop()
{
  if (needReset)
  {
    // Wait 3 secondss to allow for the cloud function to complete before resetting
    Serial.println ("Reset requested, delaying 3sec\n");
    Particle.publish("spark/status", "resetting", PRIVATE);
    delay (3000);
    System.reset ();
  }

  int channels = e131.parsePacket();
  if (count ++ % 100 == 0)
  {
    Serial.printlnf("%d\tLoop %d ted %d", count, e131.data[1], channels);
  }
  for (int i = 0; i < numOutputs; ++i)
  {
    ZCDimmer::getInstance()->setBrightness(i, e131.data[i+dmxStart]);
  }
}
