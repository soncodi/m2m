/*
 *  ATTCloudClient.h
 *
 *
 */

#ifndef ATTCloudClient_h
#define ATTCloudClient_h

#include <Arduino.h>
#include "Client.h"

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE 150

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 30

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

class ATTCloudClient {
private:
   Client* _client;
   uint8_t buffer[MQTT_MAX_PACKET_SIZE];
   uint16_t nextMsgId;
   unsigned long lastOutActivity;
   unsigned long lastInActivity;
   bool pingOutstanding;
   void (*callback)(char*,uint8_t*,unsigned int);
   int readPacket(); //uint8_t*);
   //uint8_t readByte();
   boolean write(uint8_t header, uint8_t* buf, uint16_t length);
   uint16_t writeString(char* string, uint8_t* buf, uint16_t pos);
   boolean connect(char *, char *, char *, char *, uint8_t, uint8_t, char*);
   uint8_t *ip;
   char* domain;
   uint16_t port;
   //char pubTopic[50];
   char pubTopic[60];
   String mqttMessage;

public:
   void test();
   ATTCloudClient();
   ATTCloudClient(uint8_t *, uint16_t, void(*)(char*,uint8_t*,unsigned int),Client& client);
   ATTCloudClient(char*, uint16_t, void(*)(char*,uint8_t*,unsigned int),Client& client);

   //ATT
   ATTCloudClient(void(*)(char*,uint8_t*,unsigned int),Client& client);

   //boolean connect(char *);

   //ATT
   //boolean connect(char *, char *, char *);
   boolean connect(const prog_uchar *, const prog_uchar *, const prog_uchar *);
   
   //boolean connect(char *, char *, uint8_t, uint8_t, char *);
   
   //ATT
   void disconnect();

   boolean publish(char *, char *);

   //ATT
   //void setDomainStuffThing(char *, char *, char *);
   void setDomainStuffThing(const prog_uchar *, const prog_uchar *, const prog_uchar *);
   boolean publish(char *, char *, char *, char *);
   boolean sendKV(char *, char *);
   boolean sendKV(char *, int);
   boolean sendKV(char *, boolean);

   void startMessage(void);
   void addKVToMessage(String, String);
   void addKVToMessage(String, int);
   void addKVToMessage(String, boolean);

   void endMessage(void);
   boolean sendMessage(void);

   String getMessage(void);

   boolean registerForCommands(void);
   boolean commandCompare(char *, byte *, unsigned int);

   boolean publish(char *, uint8_t *, unsigned int);
   boolean publish(char *, uint8_t *, unsigned int, boolean);
   //boolean publish_P(char *, uint8_t PROGMEM *, unsigned int, boolean);
   boolean subscribe(char *);
   boolean unsubscribe(char *);
   boolean loop();
   boolean connected();
};


class ATT3GModemClient : public Client {
public:
  ATT3GModemClient();
  int connect(IPAddress ip, uint16_t port) {return 0;}
  int connect(const char *host, uint16_t port); // {return 0;}
  size_t write(uint8_t) {size_t s; return s;}   // Not implemented
  size_t write(const uint8_t *buf, size_t size); 
  int available() {return 1;} // Not implemented
  int read() {return 0;}  // Not implemented
  int read(uint8_t *buf, size_t size);
  int peek() {return 0;}  // Not implmented
  void flush() {return;} //Not implmented 
  void stop() {return;} //Not implmented
  uint8_t connected();
  operator bool() {return 1;}  //Not implmented
 private:
  uint8_t isConnected;

};

void power_on();
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout);
void progmem2ram(char *buf, const prog_uchar *pmem);

#endif