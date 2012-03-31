//ARDUINO 1.0+ ONLY
//ARDUINO 1.0+ ONLY
#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include "BlinkM_funcs.h"
#define blinkm_addr 0x00

////////////////////////////////////////////////////////////////////////
//CONFIGURE
////////////////////////////////////////////////////////////////////////
char serverName[] = "busstop.ch.azdev.nine.ch";

//The location to go to on the server
//make sure to keep HTTP/1.0 at the end, this is telling it what type of file it is
String location = "/gottago_status/dgQX/csv HTTP/1.1";

// if need to change the MAC address (Very Rare)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
////////////////////////////////////////////////////////////////////////
EthernetClient client;




void setup(){
  BlinkM_beginWithPower(); 
  BlinkM_stopScript( blinkm_addr );  // turn off startup script
    BlinkM_fadeToRGB(blinkm_addr,0x00,0x00,0x99);
  Serial.println("Booting...");
  Serial.begin(9600);
  BlinkM_fadeToRGB(blinkm_addr,0x00,0x00,0x44);
  Ethernet.begin(mac);
  BlinkM_fadeToRGB(blinkm_addr,0x00,0x00,0x00);
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
  
  BlinkM_setFadeSpeed(blinkm_addr,20);
}

void loop(){

  char* pageValue = connectAndRead(); //connect to the server and read the output
  if (pageValue != false) {
    Serial.println("Got a response: ");
    Serial.println(pageValue); //print out the findings.

    char *str;
    String current_status;
    int next_go, next_no_go, next_off, next_refresh;
    int i = 0;
    while ((str = strtok_r(pageValue, ";", &pageValue)) != NULL) {
       i++;
       Serial.println(str);
         switch (i) {
           case 1:
             current_status = str;
           break;
           case 2:
             next_go = (int)str;
           break;
           case 3:
             next_no_go = (int)str;
           break;
           case 4:
             next_off = (int)str;
           break;
           case 5:
             next_refresh = (int)str;
           break;       
         }

    }
    Serial.println("current_status: ");
    Serial.println(current_status);
    

    if (current_status == "go") {
      BlinkM_fadeToRGB(blinkm_addr,0x00,0x99,0x00);    
    } else if (current_status == "no_go") {
      BlinkM_fadeToRGB(blinkm_addr,0x99,0x00,0x00);
    } else {
      BlinkM_fadeToRGB(blinkm_addr,0x99,0x99,0x99);
    }
    delay(2000);
  }

}

char* connectAndRead(){
  //connect to the server
  Serial.println("connecting...");

  //port 80 is typical of a www page
  if (client.connect(serverName, 80)) {
    Serial.println("connected");
    client.print("GET ");
    client.println(location);
    client.println("HOST: busstop.ch.azdev.nine.ch");    
    client.println();

    //Connected - Read the page
    return readPage(); //go and read the output

  }else{
    Serial.println("connection failed");
    return false;
  }

}

char* readPage(){
  //read the page, and capture & return everything between '<' and '>'
  char inString[200]; // string for incoming serial data
  int stringPos = 0; // string index counter
  boolean startRead = false; // is reading?
  Serial.println("reading Page ...");
  char oldChar;
  while(true){

    if (client.available()) {
      char c = client.read();
      if (c == '\r'){
        continue;
      }
      if(startRead){
        if(c != '\n'){
          inString[stringPos] = c;
          stringPos ++;
          if (sizeof(inString) == stringPos){
            Serial.println("TO BIG RESPONSE");
            return false;
          }
        }
      } else if (c == '\n' && oldChar == '\n' ) {
        startRead = true; //Ready to start reading the part
      }
      oldChar = c;
    } else if (startRead) {
          startRead = false;
          client.stop();
          client.flush();
          inString[stringPos] = 0;
          char returnString[stringPos];
          Serial.print("stringPos: ");
          Serial.println(stringPos);
          int i = 0;
          for (i = 0; i < stringPos; ++i)
          {
            Serial.print(inString[i]);
            returnString[i] = inString[i];
          }
          returnString[i] = '\0';
          Serial.println("finishing reading, returning:");
          Serial.println(returnString);
          return returnString;      
    }
    
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      client.flush();
      return false;
    }    
    

  }

}
