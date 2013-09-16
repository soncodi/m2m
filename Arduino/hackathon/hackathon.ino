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

int tempSensorPin = 0;
int lightSensorPin = 1;
int moistureSensorPin = 4;


void cmdCallBack(char *topic, uint8_t* payload, unsigned int len);

void setup()
{
  pinMode(13, OUTPUT);
  
  while(!Serial);
  delay(1500);

  Serial.begin(9600); // port to communicate to PC
  Serial1.begin(115200); // port that talks to 3G modem

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
  Serial.println("getSensorValue");
  // temp
  int B=3975;
  int val = analogRead(tempSensorPin);
  
  float resistance=(float)(1023-val)*10000/val; 
  float dat=1/(log(resistance/10000)/B+1/298.15)-273.15;
  dat = dat * 1.8 + 32;
  
  if ((int)dat > 75)
  {
    digitalWrite(13, HIGH);
    delay(500);
    acc.sendKV("Fan", "ON");
    Serial.println("Fan on");
  }
  else
  {
    digitalWrite(13, LOW);
    delay(500);
    acc.sendKV("Fan", "OFF");
    Serial.println("Fan off");
  }
    
  unsigned int dig1 = int(dat);
  unsigned int frac;
  if(val >= 0)
    frac = (dat - int(dat)) * 100;
  else
    frac = (int(dat)- dat ) * 100;
    
  char temp[10];
  sprintf(temp, "%d.%d", dig1, frac);
  Serial.print("Temp: ");
  Serial.println(temp);
  
  // light sensor
  val = analogRead(lightSensorPin); 
  
  dat = (float)(1023-val)*10/val;
  dig1 = int(dat);
  if(val >= 0)
    frac = (dat - int(dat)) * 100;
  else
    frac = (int(dat)- dat ) * 100;
    
  char light[10];
  sprintf(light, "%d.%d", dig1, frac);
  Serial.print("Light sensor resistance: ");
  Serial.println(light);
  
  // moisture sensor
  val = analogRead(moistureSensorPin);
  
  char moisture[10];
  sprintf(moisture, "%d", val);
  Serial.print("Moisture: ");
  Serial.println(moisture);
  
  acc.sendKV("Temperature", temp);
  acc.sendKV("Light", light);
  acc.sendKV("Moisture", moisture);
  
  delay(500);
}

