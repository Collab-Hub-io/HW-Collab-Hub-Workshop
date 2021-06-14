/*
CollabHub_Client.cpp

This is part of Collab-Hub's library for wifi-enabled microcontrollers that use the Arduino core
(tested on ESP32 chipsets as of 6/13/21). This library allows microcontrollers to connect to local or remote Collab-Hub servers as a client device and send/receive data to connected Max, Pure Data, Unity, web browser, and hardware clients.
IT IS CURRENTLY UNDER ACTIVE DEVELOPMENT AND NOT YET READY FOR USE IN MAJOR PROJECTS!
DO NOT DISTRIBUTE THIS LIBRARY OR THE ACCOMPANYING EXAMPLE SKETCH.

Written by Anthony T. Marasco, with input from Nick Hwang. Collab-Hub is co-developed and maintained by Nick Hwang, Eric Sheffield, and Anthony T. Marasco.

This library wraps the arduinoWebsockets and ArduinoJSON libraries. Both of those libraries must be installed in your IDE and included in your main sketch in order
for this library to function properly.
*/
#include "Arduino.h"
#include "ArduinoJson.h"
#include "WebSocketsClient.h"
#include "SocketIOclient.h"
#include "CollabHub_Client.h"

extern String nSpace;

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16)
{
    const uint8_t *src = (const uint8_t *)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++)
    {
        if (i % cols == 0)
        {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}

/*************************************
Begin Class Creation
*************************************/

CollabHub_Client::CollabHub_Client()
{
    testInit();
    
}


void CollabHub_Client::joinRoom(String roomName)
{
    DynamicJsonDocument outPayloadDoc(1024);
    //Structure our outgoing message as an array: first index is event name
    //second is the object payload as JSON

    String emitType = "joinRoom";

    JsonArray payloadArray = outPayloadDoc.to<JsonArray>();

    //Add event name to the beginning of the outgoing message
    payloadArray.add(emitType);

    //Then add the object key and member to the array
    JsonObject obj_1 = outPayloadDoc.createNestedObject();

    obj_1["room"] = roomName;

    String payload;

    serializeJson(outPayloadDoc, payload);
    //MUST add namespace tag and space to the front of any payload in order to emit
    String nSpacePayloadEmit = "/hub, " + payload;
    delay(1000);
    //The outgoing message format: /namespacetag,  ["event_name",{JSON}]
    socketIO.sendEVENT(nSpacePayloadEmit);
    //Serial.println(nSpacePayloadEmit);
}

void CollabHub_Client::parsePayload(uint8_t *payload, size_t length)
{
    //parsing incoming events, data payloads requires the JSON library
    //make pointer to string as a char first
    char *stringptr = NULL;
    int id = strtol((char *)payload, &stringptr, 10);

    //Since we receive data from a namespace, the first character of the payload array is "/". We need to check that and move the pointer to the first "[" instead

    //TO FIX: This solution is hardcoded. Need to make a char array searcher that moves pointer X number of indexes to dynamically adjust for any namespace size

    if (payload[0] == 47)
    {
        // /hub, == 5 indexes
        payload += 5;
    }

    //checks to see if there is an ID for an acknowledgment, and if so, it "removes" that ID by moving the pointer to the first character of the incoming JSON message (payload)
    if (id)
    {
        payload = (uint8_t *)stringptr;
    }

    //Build memory location for incoming message JSON doc
    DynamicJsonDocument incomingDoc(1024);

    //Deserialize the incoming message
    DeserializationError error = deserializeJson(incomingDoc, payload, length);

    //error handling for the JSON deserialization process
    if (error)
    {
        Serial.print(F("Deserialization of Event JSON failed: "));
        Serial.println(error.c_str());
        return;
    }

    //Pull event name to from JSON document that will just hold incoming event name. We don't know the name of the event/member yet, so we use an array index
    //remove ambiguity of data type for eventName by adding.as<String>() for JSON lib
    eventName = incomingDoc[0].as<String>();

    //Log the event name
    Serial.printf("[CH CLIENT] Received data on event name: %s\n", eventName.c_str());

    //Grab header/data object pair from payload
    //Build memory location for incoming data JSON doc
    DynamicJsonDocument eventJSON(1024);
    eventPayloadData = incomingDoc[1].as<String>();
    //Deserialize the second element of the incoming message doc, grabbing the event's data object
    deserializeJson(eventJSON, eventPayloadData);

    //For Debugging
    //Serial.println("here is the pretty JSON:");
    //serializeJsonPretty(incomingDoc[1], Serial);
    //Serial.println();

    if (eventName == "event")
    {
        //event from server arrives in this format - {header: <string>, from: <string>}
        eventHeader = eventJSON["header"];
        eventSender = eventJSON["from"];

        Serial.println("This is the latest event header: ");
        Serial.println(eventHeader);
        Serial.println("This is who sent the event: ");
        Serial.println(eventSender);
    }
}

void CollabHub_Client::begin(const char *host, uint16_t port, const char *url, const char *protocol)
{
    socketIO.begin(host, port, url, protocol);
}

// void begin(String host, uint16_t port, String url = "/socket.io/?EIO=3", String protocol = "arduino");
void CollabHub_Client::begin(String host, uint16_t port, String url, String protocol)
{
    socketIO.begin(host, port, url, protocol);
} 

void CollabHub_Client::loop(void)
{
    socketIO.loop();
}

void CollabHub_Client::sendPushEvent(String header, String from, String target)
{
    DynamicJsonDocument outPayloadDoc(1024);
    String emitType = "event";
    JsonArray payloadArray = outPayloadDoc.to<JsonArray>();
    payloadArray.add(emitType);
    JsonObject obj_1 = outPayloadDoc.createNestedObject();
    obj_1["header"] = header;
    obj_1["from"] = from;
    obj_1["mode"] = "push";
    obj_1["target"] = target;
    String payload;

    serializeJson(outPayloadDoc, payload);
    //MUST add namespace tag and space to the front of any payload in order to emit
    String nSpacePayloadEmit = "/hub, " + payload;

    //The outgoing message format: /namespacetag,  ["event_name",{object}]
    socketIO.sendEVENT(nSpacePayloadEmit);
    Serial.println(nSpacePayloadEmit);
}

void CollabHub_Client::testInit()
{
    
}
/*
void CollabHub_Client::hexdump(const void *mem, uint32_t len, uint8_t cols = 16)
{
    const uint8_t *src = (const uint8_t *)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++)
    {
        if (i % cols == 0)
        {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}
*/
void CollabHub_Client::onEvent(SocketIOclientEvent cbEvent)
{
    socketIO.onEvent(cbEvent);
}

void CollabHub_Client::sendUsername(String name)
{
    DynamicJsonDocument outPayloadDoc(1024);
    //Structure our outgoing message as an array: first index is event name
    //second is the object payload as JSON

    String emitType = "addUsername";

    JsonArray payloadArray = outPayloadDoc.to<JsonArray>();

    //Add event name to the beginning of the outgoing message
    payloadArray.add(emitType);

    //Then add the object key and member to the array
    JsonObject obj_1 = outPayloadDoc.createNestedObject();

    obj_1["username"] = name;

    String payload;

    serializeJson(outPayloadDoc, payload);
    //MUST add namespace tag and space to the front of any payload in order to emit
    String nSpacePayloadEmit = "/hub, " + payload;
    delay(1000);
    //The outgoing message format: /namespacetag,  ["event_name",{JSON}]
    socketIO.sendEVENT(nSpacePayloadEmit);
    //Serial.println(nSpacePayloadEmit);
}

void CollabHub_Client::connect(String server_namespace)
{
    socketIO.send(sIOtype_CONNECT, server_namespace);
}

/*
void CollabHub_Client::socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        Serial.printf("[IOc] Disconnected!\n");
        break;
    case sIOtype_CONNECT:
        Serial.printf("[IOc] Connected to url: %s\n", payload);

        // joins the specified namespace on connection
        socketIO.send(sIOtype_CONNECT, nSpace);
        //send username to server
        //sendUsername(myUsername);
        break;
    case sIOtype_EVENT:
    { //this extra set of brackets is used to contain all locally-defined variables inside of this case

        //parsing incoming events, data payloads requires the JSON library
        //make pointer to string as a char first
        char *stringptr = NULL;

        int id = strtol((char *)payload, &stringptr, 10);

        //Since we receive data from a namespace, the first character of the payload array is "/". We need to check that and move the pointer to the first "[" instead

        //TO FIX: This solution is hardcoded. Need to make a char array searcher that moves pointer X number of indexes to dynamically adjust for any namespace size

        if (payload[0] == 47)
        {
            // /hub, == 5 indexes
            payload += 5;
        }

        //checks to see if there is an ID for an acknowledgment, and if so, it "removes" that ID by moving the pointer to the first character of the incoming JSON message (payload)
        if (id)
        {
            payload = (uint8_t *)stringptr;
        }

        //****************Lets grab the incoming  event name here****************
        //Build memory location for incoming message JSON doc
        DynamicJsonDocument incomingDoc(1024);

        //Deserialize the incoming message
        DeserializationError error = deserializeJson(incomingDoc, payload, length);

        //error handling for the JSON deserialization process
        if (error)
        {
            Serial.print(F("Deserialization of Event JSON failed: "));
            Serial.println(error.c_str());
            return;
        }

        //Pull event name to from JSON document that will just hold incoming event name. We don't know the name of the event/member yet, so we use an array index

        String eventName = incomingDoc[0];
        //Log the event name
        Serial.printf("[IOc] event name: %s\n", eventName.c_str());

        //****************Lets grab the incoming data object here****************
        //Build memory location for incoming data JSON doc
        DynamicJsonDocument eventJSON(1024);
        String eventPayload = incomingDoc[1];
        //Deserialize the second element of the incoming message doc, grabbing the event's data object
        deserializeJson(eventJSON, eventPayload);
        //For Debugging
        //Serial.println("here is the pretty JSON:");
        //serializeJsonPretty(incomingDoc[1], Serial);
        //Serial.println();
        //Serial.println("Here it is a string:");
        //Serial.println(eventPayload);

        //***********************Reacting to events and payload data************

        if (eventName == "connection")
        {
            //send username to server
           // sendUsername(myUsername);
        }
        else if (eventName == "myUsername")
        {
            //extract members from the payload object based on their key
            const char *myUsername = eventJSON["username"];

            //Serial.println("Here is my username: ");
            //Serial.print(myUsername);
            Serial.println("This is my username: ");
            Serial.println(myUsername);
        }
        else if (eventName == "allRooms")
        {
            Serial.println("Here are the number of available rooms: ");
            Serial.println(eventJSON["rooms"].size());
            Serial.println("Here are the room names: ");
            for (int i = 0; i <= eventJSON["rooms"].size(); i++)
            {
                Serial.println(eventJSON["rooms"][i].as<char *>());
            }
        }
        else if (eventName == "otherUsers")
        {
            //allUsers returns an array. Handle it like this:
            Serial.println("Here are the number of the other connected users: ");
            Serial.println(eventJSON["users"].size());
            Serial.println("Here are the usernames of the other connected users: ");
            for (int i = 0; i <= eventJSON["users"].size(); i++)
            {
                Serial.println(eventJSON["users"][i].as<char *>());
            }
        }
        else if (eventName == "event")
        {
            //event from server arrives in this format - {header: <string>, from: <string>}
            const char *eventHeader = eventJSON["header"];
            const char *eventSender = eventJSON["from"];

            Serial.println("This is the latest event header: ");
            Serial.println(eventHeader);
            Serial.println("This is who sent the event: ");
            Serial.println(eventSender);
            Serial.println("Here is the pretty JSON:");
            serializeJsonPretty(eventJSON, Serial);
            Serial.println();
            if (strcmp(eventHeader, "webEvent3") == 0)
            {
                
            }
        }
        else if (eventName == "control")
        {
            //control from server arrives in this format - {header: <string>, values: <number> | <number> [] | <string> | <string>[]}
            const char *controlHeader = eventJSON["header"];
            int controlValue;
            //instead of pulling the "values" param as an int, try using JsonVariant
            //This might let you check to see what the data type "values" is
            //Max sends an array for "values", other clients might send a single int
            //Pull and store the param to the correct type based on this!
            JsonVariant valuesParam = eventJSON["values"];
            bool valuesArray = valuesParam.is<JsonArray>();
            bool valuesInt = valuesParam.is<int>();
            bool valuesFloat = valuesParam.is<float>();
            if (valuesArray)
            {
                Serial.println("Control Values are in an array");
                if (valuesParam[0].is<int>())
                {
                    Serial.println("The array is filled with the following ints:");
                    for (int i = 0; i < valuesParam.size(); i++)
                    {
                        Serial.println(valuesParam[i].as<int>());
                    }
                }
            }
            else if (valuesInt)
            {
                Serial.println("Control Value is a single int");
                controlValue = eventJSON["values"];
            }
            else if (valuesFloat)
            {
                Serial.println("Control Value is a single float");
            }

            //int controlValue = eventJSON["values"];
            const char *controlSender = eventJSON["from"];

            Serial.println("This is the latest control header: ");
            Serial.println(controlHeader);
            Serial.println("This is who sent the control:");
            Serial.println(controlSender);
            Serial.println("Here is the pretty JSON:");
            serializeJsonPretty(eventJSON, Serial);
            Serial.println();
            if (strcmp(controlHeader, "webSlider3") == 0)
            {
                String newVal = String(controlValue);
                //sendPushControl("espControl", newVal, "all");
            }
        }
    }
    break;
    case sIOtype_ACK:
        Serial.printf("[IOc] get ack: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_ERROR:
        Serial.printf("[IOc] get error: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        Serial.printf("[IOc] get binary: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_ACK:
        Serial.printf("[IOc] get binary ack: %u\n", length);
        hexdump(payload, length);
        break;
    }
}
*/


