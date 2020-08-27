/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
#include <WiFi.h>
#include <WiFiUdp.h>

// includes wifi password, etc.; that file shouldn't go into the repository
#include "secrets.h"

// WiFi network name and password:
const char * networkName = MY_SSID;
const char * networkPswd = MY_WIFI_PWD;

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * telloAddress = "192.168.10.1";
const int telloPort = 8889;
const int ownUdpPort = 9000;

// Are we currently connected to WiFi?
boolean wifiConnected = false;

// Do we have a Tello connection, yet? 
boolean telloConnected = false;

//The udp library class
WiFiUDP udp;

// a buffer for incoming packets
char packetBuffer[255];

void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);
  
  // Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);

  // Open local UDP port
  udp.begin(ownUdpPort);
}

void loop(){
  //only send data when connected
  if(wifiConnected){
    // Tello not yet connected? Try to connect. 
    if (!telloConnected) {
      Serial.println("Trying to connect to Tello");
      udp.beginPacket(telloAddress, telloPort);
      udp.printf("conn_req:lh00");
      udp.printf("\r\n");
      udp.endPacket();

      delay(1000);

      // receive response
      int packetSize = udp.parsePacket();
      Serial.println(packetSize);
      if (packetSize) {
        int len = udp.read(packetBuffer, 255);
        if (len > 0) packetBuffer[len-1] = 0;
        Serial.println(packetBuffer);
      }
      telloConnected = true;
    }
  }
  //Wait for 1 second
  delay(1000);
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          wifiConnected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          wifiConnected = false;
          break;
      default: break;
    }
}
