/*
CollabHub_Client.h

Written by Anthony T. Marasco, with input from Nick Hwang. Collab-Hub is co-developed and maintained by Nick Hwang, Eric Sheffield, and Anthony T. Marasco.

This is part of Collab-Hub's library for wifi-enabled microcontrollers that use the Arduino core
(tested on ESP32 chipsets as of 6/13/21). This library allows microcontrollers to connect to local or remote Collab-Hub servers as a client device and send/receive data to connected Max, Pure Data, Unity, web browser, and hardware clients.
IT IS CURRENTLY UNDER ACTIVE DEVELOPMENT AND NOT YET READY FOR USE IN MAJOR PROJECTS!
DO NOT DISTRIBUTE THIS LIBRARY OR THE ACCOMPANYING EXAMPLE SKETCH.

This library wraps the arduinoWebsockets and ArduinoJSON libraries. Both of those libraries must be installed in your IDE and included in your main sketch in order
for this library to function properly.
*/

#ifndef COLLABHUB_CLIENT_H
#define COLLABHUB_CLIENT_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "SocketIOclient.h"
#include "WebSocketsClient.h"

//Wrap SIO library event types to match CH nomenclature
typedef enum {
    chServerMessagetype_CONNECT = sIOtype_CONNECT,
    chServerMessagetype_DISCONNECT = sIOtype_DISCONNECT,
    chServerMessagetype_EVENT = sIOtype_EVENT,
    chServerMessagetype_ACK = sIOtype_ACK,
    chServerMessagetype_ERROR = sIOtype_ERROR,
    chServerMessagetype_BINARY_EVENT = sIOtype_BINARY_EVENT,
    chServerMessagetype_BINARY_ACK = sIOtype_BINARY_ACK,
} chServerMessageType_t;


typedef std::function<void(socketIOmessageType_t type, uint8_t * payload, size_t length)> SocketIOclientEvent;


class CollabHub_Client
{
    public:

        //Public Variables
        //The CH event name sent from the server on a server EVENT message type
        String eventName;
        String eventPayloadData;
        String chEventPayloadData;
        const char* eventHeader;
        const char* eventSender;

        //Constructor - Declare class and call initHexdump
        CollabHub_Client();
        void parseEvent();
        void parsePayload(uint8_t *payload, size_t length);
        bool getCHEvent(String chEvent);
        void begin(String host, uint16_t port, String url = "/socket.io/?EIO=4", String protocol = "arduino");
        void begin(const char *host, uint16_t port, const char *url = "/socket.io/?EIO=4", const char *protocol = "arduino");

        void loop(void);
        void onEvent(SocketIOclientEvent cbEvent);
        void sendUsername(String name);
        void joinRoom(String roomName);
        void connect(String server_namespace);
        //send a push event header and data object to the server
        void sendPushEvent(String header, String from, String target);
        //send a push control header and data object to the server
        void sendPushControl(String header, String values , String target);
        //send your username to the server
        
        //declare the hexdump serial printing function required by SIO library for ESP32 boards
        void initHexdump();
        //void hexdump(const void *mem, uint32_t len, uint8_t cols = 16);
        void testInit(void);
        //turn on print debuggin print outs to the serial port
        void enablePrint(bool state = false, int baudrate = 115200);
    private:
        SocketIOclient socketIO;
        void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length);
    
    
};


#endif