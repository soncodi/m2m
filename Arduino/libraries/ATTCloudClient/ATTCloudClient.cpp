/*
 *
 *  ATTCloudClient.cpp
 *
 *
*/

#include "ATTCloudClient.h"
#include <string.h>

#define AUX_STR_LEN 160
char aux_str[AUX_STR_LEN];

int onModulePin= 2;

void ATTCloudClient::test() {
  callback("topic",(uint8_t*)"payload",10);
}


ATTCloudClient::ATTCloudClient() {
   this->_client = NULL;
}

ATTCloudClient::ATTCloudClient(uint8_t *ip, uint16_t port, void (*callback)(char*,uint8_t*,unsigned int), Client& client) {
   this->_client = &client;
   this->callback = callback;
   this->ip = ip;
   this->port = port;
   this->domain = NULL;
}

ATTCloudClient::ATTCloudClient(char* domain, uint16_t port, void (*callback)(char*,uint8_t*,unsigned int), Client& client) {
   this->_client = &client;
   this->callback = callback;
   this->domain = domain;
   this->port = port;
}


ATTCloudClient::ATTCloudClient(void (*callback)(char*,uint8_t*,unsigned int), Client& client) {
   this->_client = &client;
   this->callback = callback;
   this->domain = "att-q.m2m.io";
   this->port = 1883;
}



/*
boolean ATTCloudClient::connect(char *id) {
   return connect(id,NULL,NULL,0,0,0,0);
}
*/

void progmem2ram(char *buf, const prog_uchar *pmem) {

  //int i = 0;
  char c;
  while ( c = pgm_read_byte(pmem++) ) {
      *buf = c;
      buf++;
    }
  *buf = '\0';
}
//boolean ATTCloudClient::connect(char *id, char *user, char *pass) {
boolean ATTCloudClient::connect(const prog_uchar *id, const prog_uchar *user, const prog_uchar *pass) {
  char idBuf[30];
  char userBuf[30];
  char passBuf[33];

  progmem2ram(idBuf,id);
  progmem2ram(userBuf,user);
  progmem2ram(passBuf,pass);
   return connect(idBuf,userBuf,passBuf,0,0,0,0);
}

/*
boolean ATTCloudClient::connect(char *id, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage)
{
   return connect(id,NULL,NULL,willTopic,willQos,willRetain,willMessage);
}
*/

// private connection method
boolean ATTCloudClient::connect(char *id, char *user, char *pass, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage) {
  if (!connected() ) { // connected() isn't implmented in Client -- always returns 1
      int result = 0;
      
      if (domain != NULL) {
        result = _client->connect(this->domain, this->port);
      } else {
        result = _client->connect(this->ip, this->port);
      }
      
      if (result) {
         nextMsgId = 1;
         uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p',MQTTPROTOCOLVERSION};
         // Leave room in the buffer for header and variable length field
         uint16_t length = 5;
         unsigned int j;
         for (j = 0;j<9;j++) {
            buffer[length++] = d[j];
         }

         uint8_t v;
         if (willTopic) {
            v = 0x06|(willQos<<3)|(willRetain<<5);
         } else {
            v = 0x02;
         }

         if(user != NULL) {
            v = v|0x80;

            if(pass != NULL) {
               v = v|(0x80>>1);
            }
         }

         buffer[length++] = v;

         buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
         buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
         length = writeString(id,buffer,length);
         if (willTopic) {
            length = writeString(willTopic,buffer,length);
            length = writeString(willMessage,buffer,length);
         }

         if(user != NULL) {
            length = writeString(user,buffer,length);
            if(pass != NULL) {
               length = writeString(pass,buffer,length);
            }
         }

         write(MQTTCONNECT,buffer,length-5);
         
         lastInActivity = lastOutActivity = millis();
         
         while (!_client->available()) {
            unsigned long t = millis();
            if (t-lastInActivity > MQTT_KEEPALIVE*1000UL) {
               _client->stop();
               return false;
            }
         }

         int len = readPacket(); //&llen);
         
         if (len == 4 && buffer[3] == 0) {
            lastInActivity = millis();
            pingOutstanding = false;
            return true;
         } else {
	   char tmp[20];
	   sprintf(tmp,"len: %d",len);
	   Serial.println(tmp);

	   sprintf(tmp,"buffer[0]: %x",buffer[0]);
	   Serial.println(tmp);

	   sprintf(tmp,"buffer[1]: %x",buffer[1]);
	   Serial.println(tmp);
	   sprintf(tmp,"buffer[2]: %x",buffer[2]);
	   Serial.println(tmp);

	   sprintf(tmp,"buffer[3]: %x",buffer[3]);
	   Serial.println(tmp);




	 }
      }
      _client->stop();
   }
   return false;
}

// uint8_t ATTCloudClient::readByte() {
//    while(!_client->available()) {}
//    return _client->read();
// }

int ATTCloudClient::readPacket() /*uint8_t* lengthLength)*/ {
  //int len;
  return _client->read((uint8_t*)&buffer, (size_t)MQTT_MAX_PACKET_SIZE);


   // uint16_t len = 0;
   // buffer[len++] = readByte();
   // uint8_t multiplier = 1;
   // uint16_t length = 0;
   // uint8_t digit = 0;
   // do {
   //    digit = readByte();
   //    buffer[len++] = digit;
   //    length += (digit & 127) * multiplier;
   //    multiplier *= 128;
   // } while ((digit & 128) != 0);

   // *lengthLength = len-1;
   // for (uint16_t i = 0;i<length;i++)
   // {
   //    if (len < MQTT_MAX_PACKET_SIZE) {
   //       buffer[len++] = readByte();
   //    } else {
   //       readByte();
   //       len = 0; // This will cause the packet to be ignored.
   //    }
   // }

   // return len;
}

boolean ATTCloudClient::loop() {
   if (connected()) {
      unsigned long t = millis();
      if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL/2) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL/2)) {
         if (pingOutstanding) {
            _client->stop();
            return false;
         } else {
            buffer[0] = MQTTPINGREQ;
            buffer[1] = 0;
            _client->write(buffer,2); 
            lastOutActivity = t;
            lastInActivity = t;
            pingOutstanding = true;
         }
      }
      if (_client->available()) {
	uint8_t llen;
         int len = readPacket(); 
	 if (len < 126)   // TODO is this logic correcT?  Test it.
	   llen = 1;
	 else
	   llen = 2;
         if (len > 0) {
            lastInActivity = t;
            uint8_t type = buffer[0]&0xF0;
            if (type == MQTTPUBLISH) {
               if (callback) {
		 uint16_t tl = (buffer[llen+1]<<8)+buffer[llen+2];
                  // char topic[tl+1];  // This is a long string; conserve memory and not return it???
                  // for (uint16_t i=0;i<tl;i++) {
                  //    topic[i] = buffer[llen+3+i];
                  // }
                  // topic[tl] = 0;
                  // // ignore msgID - only support QoS 0 subs
                  // uint8_t *payload = buffer+llen+3+tl;
                  // callback(topic,payload,len-llen-3-tl);
		  buffer[len] = 0;
		  callback(NULL,buffer+llen+3+tl,len-llen-3-tl);
               }
            } else if (type == MQTTPINGREQ) {
               buffer[0] = MQTTPINGRESP;
               buffer[1] = 0;
               _client->write(buffer,2);
            } else if (type == MQTTPINGRESP) {
               pingOutstanding = false;
            }
         }
      }
      return true;
   }
   return false;
}

boolean ATTCloudClient::publish(char* topic, char* payload) {
   return publish(topic,(uint8_t*)payload,strlen(payload),false);
}



//void ATTCloudClient::setDomainStuffThing(char *domain, char *stuff, char *thing) {
void ATTCloudClient::setDomainStuffThing(const prog_uchar *domain, const prog_uchar *stuff, const prog_uchar *thing) {
  char buffer[40];
   String topicStr = "";
   progmem2ram(buffer,domain);
   topicStr.concat(buffer);//topicStr.concat(domain);
   topicStr.concat("/");
   progmem2ram(buffer,stuff);
   topicStr.concat(buffer);//topicStr.concat(stuff);
   topicStr.concat("/");
   progmem2ram(buffer,thing);
   topicStr.concat(buffer);//   topicStr.concat(thing);
   topicStr.toCharArray(pubTopic, topicStr.length()+1);
}

boolean ATTCloudClient::publish(char* domain, char* stuff, char* thing, char* payload) {
   char topic[60];
   String topicStr = "";
   topicStr.concat(domain);
   topicStr.concat("/");
   topicStr.concat(stuff);
   topicStr.concat("/");
   topicStr.concat(thing);
   topicStr.toCharArray(topic, topicStr.length()+1);

   return publish(topic,(uint8_t*)payload,strlen(payload),false);
}

boolean ATTCloudClient::sendKV(char *key, char *value) {
   String msg;
   char payload[100];
   msg.concat("{\"");
   msg.concat(key);
   msg.concat("\":\"");
   msg.concat(value);
   msg.concat("\"}");
   msg.toCharArray(payload, msg.length()+1);

   return publish(pubTopic,(uint8_t*)payload,strlen(payload),false);
}

boolean ATTCloudClient::sendKV(char *key, int value) {
   String msg;
   char payload[100];
   msg.concat("{\"");
   msg.concat(key);
   msg.concat("\":");
   msg.concat(value);
   msg.concat("}");
   msg.toCharArray(payload, msg.length()+1);

   return publish(pubTopic,(uint8_t*)payload,strlen(payload),false);
}

boolean ATTCloudClient::sendKV(char *key, boolean value) {
   String msg;
   char payload[100];
   msg.concat("{\"");
   msg.concat(key);
   msg.concat("\":");
   if (value) {
      msg.concat("true");   
   } else {
      msg.concat("false");
   }
   msg.concat("}");
   msg.toCharArray(payload, msg.length()+1);

   return publish(pubTopic,(uint8_t*)payload,strlen(payload),false);
}

void ATTCloudClient::startMessage(void) {
   mqttMessage = "{";
}

void ATTCloudClient::addKVToMessage(String k, String v) {
   if (mqttMessage.length() > 1) {
      mqttMessage.concat(",");
   }
   mqttMessage.concat("\"");
   mqttMessage.concat(k);
   mqttMessage.concat("\":\"");
   mqttMessage.concat(v);
   mqttMessage.concat("\"");
}

void ATTCloudClient::addKVToMessage(String k, int v) {
   if (mqttMessage.length() > 1) {
      mqttMessage.concat(",");
   }
   mqttMessage.concat("\"");
   mqttMessage.concat(k);
   mqttMessage.concat("\":");
   mqttMessage.concat(v);
}

void ATTCloudClient::addKVToMessage(String k, boolean v) {
   if (mqttMessage.length() > 1) {
      mqttMessage.concat(",");
   }
   mqttMessage.concat("\"");
   mqttMessage.concat(k);
   mqttMessage.concat("\":");
   if (v) {
      mqttMessage.concat("true");   
   } else {
      mqttMessage.concat("false");
   }
}

void ATTCloudClient::endMessage(void) {
   mqttMessage.concat("}");
}

boolean ATTCloudClient::sendMessage() {
   char payload[200];
   mqttMessage.toCharArray(payload, mqttMessage.length()+1);
   return publish(pubTopic,(uint8_t*)payload,strlen(payload),false);
}

String ATTCloudClient::getMessage(void) {
   return mqttMessage;
}

boolean ATTCloudClient::registerForCommands(void) {
   char cmdTopic[100];
   String cmdTopicStr = "";
   cmdTopicStr.concat(pubTopic);
   cmdTopicStr.concat("/cmd");
   cmdTopicStr.toCharArray(cmdTopic, cmdTopicStr.length()+1);
   return subscribe(cmdTopic);
}

boolean ATTCloudClient::commandCompare(char *cmd, byte *rcvdBuff, unsigned int len) {
   if (strlen(cmd) != len) {
      return false;
   }
   for (unsigned int i=0; i<len; i++) {
      if (cmd[i] != (char)(rcvdBuff[i])) {
         return false;
      }
   }
   return true;
}

boolean ATTCloudClient::publish(char* topic, uint8_t* payload, unsigned int plength) {
   return publish(topic, payload, plength, false);
}

boolean ATTCloudClient::publish(char* topic, uint8_t* payload, unsigned int plength, boolean retained) {
  if (connected() ) {
      // Leave room in the buffer for header and variable length field
      uint16_t length = 5;
      length = writeString(topic,buffer,length);
      uint16_t i;
      for (i=0;i<plength;i++) {
	if (length >= MQTT_MAX_PACKET_SIZE) 
	  Serial.println(F("Overran ATTCloudClient buffer size."));
         buffer[length++] = payload[i];
      }
      uint8_t header = MQTTPUBLISH;
      if (retained) {
         header |= 1;
      }
      return write(header,buffer,length-5);
   }
   return false;
}

// boolean ATTCloudClient::publish_P(char* topic, uint8_t* PROGMEM payload, unsigned int plength, boolean retained) {
//    uint8_t llen = 0;
//    uint8_t digit;
//    int rc=0;
//    uint16_t tlen;
//    int pos = 0;
//    int i;
//    uint8_t header;
//    unsigned int len;
   
//    if (!connected()) {
//       return false;
//    }
   
//    tlen = strlen(topic);
   
//    header = MQTTPUBLISH;
//    if (retained) {
//       header |= 1;
//    }
//    buffer[pos++] = header;
//    len = plength + 2 + tlen;
//    do {
//       digit = len % 128;
//       len = len / 128;
//       if (len > 0) {
//          digit |= 0x80;
//       }
//       buffer[pos++] = digit;
//       llen++;
//    } while(len>0);
   
//    pos = writeString(topic,buffer,pos);
   
//    rc += _client->write(buffer,pos);
   
//    for (i=0;i<plength;i++) {
//       rc += _client->write((char)pgm_read_byte_near(payload + i));
//    }
   
//    lastOutActivity = millis();
//    return rc == len + 1 + plength;
// }

boolean ATTCloudClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
   uint8_t lenBuf[4];
   uint8_t llen = 0;
   uint8_t digit;
   uint8_t pos = 0;
   uint8_t rc;
   uint8_t len = length;
   do {
      digit = len % 128;
      len = len / 128;
      if (len > 0) {
         digit |= 0x80;
      }
      lenBuf[pos++] = digit;
      llen++;
   } while(len>0);

   buf[4-llen] = header;
   for (int i=0;i<llen;i++) {
      buf[5-llen+i] = lenBuf[i];
   }
   rc = _client->write(buf+(4-llen),length+1+llen);
   lastOutActivity = millis();
   return (rc == 1+llen+length);
}

boolean ATTCloudClient::subscribe(char* topic) {
   if (connected()) {
      // Leave room in the buffer for header and variable length field
      uint16_t length = 5;
      nextMsgId++;
      if (nextMsgId == 0) {
         nextMsgId = 1;
      }
      buffer[length++] = (nextMsgId >> 8);
      buffer[length++] = (nextMsgId & 0xFF);
      length = writeString(topic, buffer,length);
      // 0 is reserved as an invalid Message Id.
      if (nextMsgId == 0)
         nextMsgId = 1;
      buffer[length++] = 0; // Only do QoS 0 subs
      //return write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-5);
      write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-5);
      //readPacket();  // Read Response (and do nothing with it??)  TODO // loop should read
      return true;
   }
   return false;
}

// boolean ATTCloudClient::unsubscribe(char* topic) {
//    if (connected()) {
//       uint16_t length = 5;
//       nextMsgId++;
//       if (nextMsgId == 0) {
//          nextMsgId = 1;
//       }
//       buffer[length++] = (nextMsgId >> 8);
//       buffer[length++] = (nextMsgId & 0xFF);
//       length = writeString(topic, buffer,length);
//       return write(MQTTUNSUBSCRIBE|MQTTQOS1,buffer,length-5);
//    }
//    return false;
// }

void ATTCloudClient::disconnect() {
   buffer[0] = MQTTDISCONNECT;
   buffer[1] = 0;
   _client->write(buffer,2);
   _client->stop();
   lastInActivity = lastOutActivity = millis();
}

uint16_t ATTCloudClient::writeString(char* string, uint8_t* buf, uint16_t pos) {
   char* idp = string;
   uint16_t i = 0;
   pos += 2;
   while (*idp) {
      buf[pos++] = *idp++;
      i++;
   }
   buf[pos-i-2] = (i >> 8);
   buf[pos-i-1] = (i & 0xFF);
   return pos;
}


boolean ATTCloudClient::connected() {
   boolean rc;
   if (_client == NULL ) {
      rc = false;
   } else {
      rc = (int)_client->connected();
      if (!rc) _client->stop();
   }
   return rc;
}



////////////////////////
// Client
/////////////////


// char* host is the IP for now
int ATT3GModemClient::connect(const char *host, uint16_t port) {
  //char aux_str[50];
  uint8_t answer;
  power_on();

  delay(2000);
  Serial.println(F("Connected to network"));
  answer = sendATcommand("AT+CGSOCKCONT=1,\"IP\",\"m2m.com.attz\"", "OK", 20000);

  delay(2000);

  if (!answer)
    return 0;

  Serial.println(F("Connecting to the network..."));
  answer = sendATcommand("AT+CGATT=1", "OK", 20000);
  
  delay(2000);

  /*
  if(!answer) {
    Serial.println(F("Error attaching.\n"));
    while(true);
  }
  */

  Serial.println(F("PDP Context..."));
  answer = sendATcommand("AT+CGACT=1,1", "OK", 20000);
  
  delay(1000);

  if(!answer) {
    Serial.println(F("Error in PDP context.\n"));
    while(true);
  }
    
  //sendATcommand("AT+NETCLOSE","OK",2000);
  sprintf(aux_str, "AT+NETOPEN=\"TCP\""); 
  answer = sendATcommand(aux_str, "Network opened", 20000);
    
  if (answer == 1) {
    Serial.println(F("Network opened"));
    sprintf(aux_str, "AT+TCPCONNECT=\"%s\",%d", host, port);
    answer = sendATcommand(aux_str, "Connect ok", 20000);
  } else {
    Serial.println(F("Unable to open network."));
  }

  isConnected = answer;
  return answer;
}

size_t ATT3GModemClient::write(const uint8_t *buf, size_t size) {

  uint8_t answer;
  int x;
  char *p;
  char *p2;
  unsigned long previous;

  
  sprintf(aux_str, "AT+TCPWRITE=%d", size);
  answer = sendATcommand(aux_str, ">", 20000);
  delay(100);
  if (answer == 1) {
    //answer = sendATcommand(buf, "Send OK", 20000);  
    memset(aux_str,'\0', AUX_STR_LEN);
    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer
    Serial1.write(buf,size);

    x = 0;
    answer = 0;
    // this loop waits for the answer
    previous = millis();
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0){   
	  if (x < AUX_STR_LEN) {
            aux_str[x] = Serial1.read();
	  } else {
	    Serial.println(F("Overran aux_str_len in write while reading response."));
	    return 0;
	  }
	  x++;
	  // check if the desired answer is in the response of the module
	  if ( ((p = (char*)memmem(aux_str,x-1, "+TCPWRITE:",10)) != NULL) &&
	       ((p2 = strstr(p,"\r\n")) != NULL) )
            {
	      answer = 1;
            }
        }

	if ( (millis() - previous) > 5000 ) { 
	  Serial.println(F("Timeout waiting for +TCPWRITE:"));
	    char tmp[10];
	    sprintf(tmp,"len: %d",x);
	    Serial.println(tmp);
	    aux_str[x] = '\0';
	    Serial.println(aux_str);
	    return -1;

	}
    // Waits for the asnwer
    } while((answer == 0));

    
  } else {
    Serial.println(F("Didn't receive '>' from AT+TCPWRITE"));
    Serial.println(aux_str);
    
    return -1;
  }

  char buf2[20];

  strncpy(buf2,p+10, p2-(p+10));

  sscanf(buf2,"%*d, %d",&answer);

  if (size != answer) {
    Serial.println(F("Error writing"));
    return -1;
    //Serial.println(aux_str);
  }
  
  return answer; 
}

// Loop for +IPD<len> in serial stream
// Here we assume size is the maximum number of bytes we can read
int ATT3GModemClient::read(uint8_t *buf, size_t size) {
  int i = 0;
  int answer = 0;
  //#define TMP_BUF_LEN 300
  //char tmpBuf[TMP_BUF_LEN];
  char buf2[10];
  int len;
  char *p;
  char *p2;
  unsigned long previous;

  memset(aux_str,'\0', AUX_STR_LEN);
  // Lookfor +IPD<len>\r\n
  previous = millis();
  do {
    if ( ((p = (char*)memmem(aux_str,i,"+IPD",4)) != NULL) &&
	 ((p2 = strstr(p,"\r\n")) != NULL) &&
	 ( p2-(p+4) < 10 ) ) {
      // aux_str now contains +IPD<len>\r\n
      strncpy(buf2,p+4,p2-(p+4));
      if (sscanf(buf2,"%d",&len))  {
	answer = 1;
	break;
      } else
	return -1;
    }
    if(Serial1.available() != 0){
      if ( i < AUX_STR_LEN) {
	aux_str[i] = Serial1.read();
	i++;
      } else {
	Serial.println(F("Overran aux_str in ATT3GModemClient::read"));
	//aux_str[AUX_STR_LEN-1] = '\0';
	Serial.println(aux_str);
	return -1;
      }
    }

    if ( (millis() - previous) > 5000) {
      // timeout
      Serial.println(F("::read timeout")); //Timeout in ::read looking for +IPD"));
      // flush the return?
      //while (Serial.available() > 0) Serial.read();
      //aux_str[AUX_STR_LEN-1] = '\0';
      //Serial.println(aux_str);
      return -1;
    }

  }while (answer == 0);

  i = 0;
  previous = millis();
  do {
    if ( Serial1.available() > 0 ) {
      if ( i < size ) {
	buf[i] = Serial1.read();
	i++;
      } else {
	Serial.println(F("Attempted to read more than buffer size in ATT3GModemClient::read"));
	return -1;
      }
    }

    if ( (millis() - previous) > 5000) {
      // timeout
      Serial.println(F("Timeout in ::read looking for remaining bytes"));
      // char tmp[10];
      // sprintf(tmp,"len: %d",len);
      // Serial.println(tmp);
      // sprintf(tmp,"i: %d",i);
      // Serial.println(tmp);
      return -1;
    }
  } while ( i < len );

  if (i != len) {
    Serial.println(F("\nDidn't read the expected number of bytes."));
    return -1;
  }

  return len;
  
}

void power_on(){

    uint8_t answer=0;

    Serial.println(F("Powering on modem."));
    
    // checks if the module is started
    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        // power on pulse
        digitalWrite(onModulePin,HIGH);
        delay(3000);
        digitalWrite(onModulePin,LOW);
    
        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }
    
}



// Returns 1 if expected response is observed within timeout
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
#define AT_RESPONSE_LEN 100
    char response[AT_RESPONSE_LEN];
    unsigned long previous;

    //memset(aux_str, '\0', AUX_STR_LEN);    // Initialice the string
    memset(response, '\0', AT_RESPONSE_LEN);    // Initialice the string
    
    delay(100);
    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer
    
    Serial1.println(ATcommand);    // Send the AT command 
    Serial.println(ATcommand);  // debug: echo to PC

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial1.available() != 0){ 
	  if (x < AT_RESPONSE_LEN) {
            response[x] = Serial1.read();
	  } else {
	    Serial.println(F("Overan response buf in sendATcommand"));
	    char tmp[10];
	    sprintf(tmp,"len: %d",x);
	    Serial.println(tmp);
	    //aux_str[AUX_STR_LEN-1] = '\0';
	    //Serial.println(aux_str);
	    response[AT_RESPONSE_LEN-1] = '\0';
	    Serial.println(response);
	    return -1;
	  }
	  x++;
	  // check if the desired answer is in the response of the module
	  if (strstr(response, expected_answer) != NULL)    
            {
	      answer = 1;
            }
        } // serial.available()

    }while((answer == 0) && ((millis() - previous) < timeout));    

    Serial.println(response);

    return answer;
}

uint8_t ATT3GModemClient::connected() {
  return isConnected;
}

ATT3GModemClient::ATT3GModemClient() {
  isConnected = 0;
}