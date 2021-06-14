/*
~ ~ ~ ~ ~ ~ Collab-Hub Workshop: NIME 2021 ~ ~ ~ ~ ~ ~

Collab-Hub Client for ESP32 microcontrollers

Example: Basic Connection, Room Joining, and Receiving from other CH clients/the server
version 0.1.0 - 6/12/2021

Firmware by Anthony T. Marasco, portions based on Markus Sattler's arduinoWebSockets examples

Collab-Hub is designed and maintained by Nick Hwang, Eric Sheffield, and Anthony T. Marasco

This code is only for NIME 2021 workshop participants. Please do not distribute elsewhere.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <ArduinoJson.h>
#include <CollabHub_Client.h>

//Starting on-board LED state
int ledState = LOW;

//Enter wifi credentials and client username here
const char* ssid = "your_network_name";
const char* password = "your_network_password";
const String myUsername = "espClient_NIME";

String nSpace = "/hub"; //Do not change this. Cannot change with current lib version

//Connecting to CH online server
const char host[] = "ch-server.herokuapp.com";
const int port = 80;


//Init instance of CollabHub_Client library

CollabHub_Client ch;


//Processing function for all Socket-related and Collab-Hub specific events

void parseEvents(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case chServerMessagetype_DISCONNECT:
    Serial.printf("[CH CLIENT] Disconnected!\n");
    break;
  case chServerMessagetype_CONNECT:
    Serial.printf("[CH CLIENT] Connected to url: %s\n", payload);

    // Joins the specified namespace on connection
    ch.connect(nSpace);

    //Send username to server after connection
    ch.sendUsername(myUsername);

    //Join the specified room
    ch.joinRoom("arduinoRoom");
    break;
  case chServerMessagetype_EVENT:
    //Parses the incoming payload from the server, seperates the event name from the header/object pair
    ch.parsePayload(payload, length);

    //print the payload of data coming from the server to the console
    Serial.println(ch.eventPayloadData);
    //Do something based on an incoming event or control
    if(ch.eventName == "event"){
      //react to a specific event header
      if(strcmp(ch.eventHeader,"bam")==0){
        Serial.println("bam received!");
        
        ledState = !ledState;

        digitalWrite(2, ledState);
      }
    }
    break;
  }
}


void setup() {
  //Built-in LED on most 30-pin ESP boards is tied to pin 2. Your mileage may vary.
  pinMode(2, OUTPUT);

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  // disable ESP Access Point Mode (helpful, but not neccessary)
  if (WiFi.getMode() & WIFI_AP)
  {
    WiFi.softAPdisconnect(true);
  }
  //Begin WiFi, connect to your network
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(50);
    Serial.println("Connecting to WiFi...");

  }

  Serial.println("WiFi Connection established to ");
  Serial.println(WiFi.SSID());
  
  Serial.println("Connecting to namespace:");
  Serial.println(nSpace);

  
  //Begin communication with CH server
  ch.begin(host, port);
  //Parse server events
  ch.onEvent(parseEvents);
}

void loop() {
  //Watches for client/server communication
  ch.loop();
}

