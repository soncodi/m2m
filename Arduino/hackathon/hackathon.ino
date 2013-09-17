#include <ATTCloudClient.h>
#include "Timer.h"

// Longest variable is 32 chars
PROGMEM const prog_uchar  M2MIO_USERNAME[]    = "asoncodi@gmail.com";
PROGMEM const prog_uchar  M2MIO_PASSWORD[]    = "3dbcf1dd4d06937acec56583b6f8f1b7";    // MD5 key of password
PROGMEM const prog_uchar  M2MIO_DOMAIN[]      =  "1ce215c3f9414890642cbc67595780a7";
PROGMEM const prog_uchar  M2MIO_DEVICE_TYPE[] =	 "arduino";
PROGMEM const prog_uchar  M2MIO_DEVICE_ID[]   =  "device02";

PROGMEM const prog_uchar M2MIO_CLIENT_ID[]    =  "arduino/device02";

#define REPORTING_INTERVAL_MS  3000

ATTCloudClient acc;
ATT3GModemClient c;

Timer t;

int tempSensorPin = 0;
int lightSensorPin = 1;
int moistureSensorPin = 4;

float Res0=20.0;              // Resistance in the circuit of sensor 0 (KOhms)
// depending of the Resistance used, you could measure better at dark or at bright conditions.
// you could use a double circuit (using other LDR connected to analog pin 1) to have fun testing the sensors.
// Change the value of Res0 depending of what you use in the circuit


void cmdCallBack(char *topic, uint8_t* payload, unsigned int len);

void setup()
{  
  while(!Serial1);

  Serial.begin(9600); // port to communicate to PC
  Serial1.begin(115200); // port that talks to 3G modem

  c = ATT3GModemClient();
  acc = ATTCloudClient(cmdCallBack,c);
  
  Serial.print("Connected: ");
  Serial.println(acc.connected());
  
  if (!acc.connect(M2MIO_DEVICE_ID,M2MIO_USERNAME,M2MIO_PASSWORD))
  {
   Serial.println(F("Unable to connect to network (12)"));
  }

  acc.setDomainStuffThing(M2MIO_DOMAIN,M2MIO_DEVICE_TYPE,M2MIO_DEVICE_ID);
  acc.registerForCommands();
  //acc.subscribe("1ce215c3f9414890642cbc67595780a7/arduino/device02");
  
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  int sensorEvent = t.every(REPORTING_INTERVAL_MS, getSensorValue);
}

void loop()
{
  acc.loop();
  t.update();
}

void cmdCallBack(char *topic, uint8_t* payload, unsigned int len)
{
  Serial.println(F("Command Received"));

  if (acc.commandCompare("{\"light\":\"on\"}", payload, len)) {
    digitalWrite(13, HIGH);
  } else if (acc.commandCompare("{\"light\":\"off\"}", payload, len)) {
    digitalWrite(13, LOW);
  } else {
    Serial.println(F("No match"));
  }
}

void getSensorValue()
{  
  Serial.println(F("getSensorValue"));
  // temp
  int B=3975;
  int val = analogRead(tempSensorPin);
  
  float resistance=(float)(1023-val)*10000/val; 
  float dat=1/(log(resistance/10000)/B+1/298.15)-273.15;
  dat = dat * 1.8 + 32;
  
  int fanOn = 0;
  if ((int)dat > 75)
  {
    digitalWrite(12, HIGH);
    fanOn = 1;
    Serial.println(F("Fan on"));
  }
  else
  {
    digitalWrite(12, LOW);
    fanOn = 0;
    Serial.println(F("Fan off"));
  }
    
  unsigned int dig1 = int(dat);
  unsigned int frac;
  if(val >= 0)
    frac = (dat - int(dat)) * 100;
  else
    frac = (int(dat)- dat ) * 100;
    
  char temp[10];
  sprintf(temp, "%d.%d", dig1, frac);
  Serial.print(F("Temp: "));
  Serial.println(temp);
  
  // light sensor
  val = analogRead(lightSensorPin); 
  
  dat=val*0.0048828125;                        // calculate the voltage
  int lux0=500/(Res0*((5-dat)/dat));           // calculate the Lux
  
  Serial.print(F("Luminosity: "));
  Serial.print(lux0);
  Serial.println(F(" Lux"));
    
  int lightsOn = 0;
  if (lux0 < 50)
  {
    digitalWrite(13, HIGH);
    lightsOn = 1;
    Serial.println(F("Lights on"));
  }
  else
  {
    digitalWrite(13, LOW);
    lightsOn = 0;
    Serial.println(F("Lights off"));
  }
  
  // moisture sensor
  val = analogRead(moistureSensorPin);
  
  char moisture[10];
  sprintf(moisture, "%d", val/10);
  Serial.print(F("Moisture: "));
  Serial.println(moisture);
  
  acc.startMessage();
  acc.addKVToMessage("t", temp);
  acc.addKVToMessage("l", lux0);
  acc.addKVToMessage("m", moisture);
  acc.addKVToMessage("fs", fanOn);
  acc.addKVToMessage("ls", lightsOn);
  acc.endMessage();
  
  boolean q = acc.sendMessage();
  Serial.println(q);
  
  Serial.println(acc.getMessage());
}
