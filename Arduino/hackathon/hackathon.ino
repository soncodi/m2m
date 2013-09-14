#include <ATTCloudClient.h>
#include "Timer.h"

// Longest variable is 32 chars
PROGMEM const prog_uchar  M2MIO_USERNAME[]    = "team7@att.com";
PROGMEM const prog_uchar  M2MIO_PASSWORD[]    = "a04f81faf54465382e71526874920e26";    // MD5 key of password
PROGMEM const prog_uchar  M2MIO_DOMAIN[]      =  "69bd3ed2f51a8d7bd68180eda5d0c2c8";
PROGMEM const prog_uchar  M2MIO_DEVICE_TYPE[] =	 "arduino";
PROGMEM const prog_uchar  M2MIO_DEVICE_ID[]   =  "device01";

PROGMEM const prog_uchar M2MIO_CLIENT_ID[]    =  "arduino/device01";

#define REPORTING_INTERVAL_MS  3000

ATTCloudClient acc;
ATT3GModemClient c;

Timer t;

int sensorPin = 0;
int sensorValue = 0;  // sensor reading

void cmdCallBack(char *topic, uint8_t* payload, unsigned int len);

void setup()
{
  while(!Serial);
  delay(1500);

  Serial.begin(9600); // port to communicate to PC
  Serial1.begin(115200); // port that talks to 3G modem

  Serial.println(F("Sensor Loop"));

  c = ATT3GModemClient();
  acc = ATTCloudClient(cmdCallBack,c);

  if (!acc.connect(M2MIO_DEVICE_ID,M2MIO_USERNAME,M2MIO_PASSWORD))
  {
   Serial.println(F("Unable to connect to network (12)"));
  }

  acc.setDomainStuffThing(M2MIO_DOMAIN,M2MIO_DEVICE_TYPE,M2MIO_DEVICE_ID);

  //acc.registerForCommands();
  
  int sensorEvent = t.every(REPORTING_INTERVAL_MS, getSensorValue);
}

void loop()
{
  delay(200);
  acc.loop();
  t.update();
}

void cmdCallBack(char *topic, uint8_t* payload, unsigned int len)
{
  Serial.println(F("In the cmdCallBack()"));
  Serial.println((char*)payload);
}

void getSensorValue()
{
  int val;
  float dat;
  val = analogRead(sensorPin);
  dat = (125*val)>>8 ; // Temperature calculation formula
  dat = dat * 1.8 + 32;
  
  unsigned int dig1 = int(dat);
  unsigned int frac;
  if(val >= 0)
    frac = (dat - int(dat)) * 100;
  else
    frac = (int(dat)- dat ) * 100;
    
  char result[20];
  sprintf(result, "%d.%d", dig1, frac);
  Serial.print("Temp: ");
  Serial.println(result);
  acc.sendKV("Temperature", result);
}

