#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class MotionController;
class ServoManager;
class TmcManager;
class C3Receiver;

class WebInterface {
public:
    WebInterface(MotionController& motion, ServoManager& servos, TmcManager& tmc, C3Receiver& c3);
    void begin();
    void update();
    void broadcastTelemetry();

private:
    WebServer server;
    WebSocketsServer webSocket;

    MotionController& motionCtrl;
    ServoManager& servoMgr;
    TmcManager& tmcMgr;
    C3Receiver& c3Recv;

    unsigned long lastBroadcastTime = 0;

    void setupRoutes();
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    void handleCommand(const String& jsonStr);
};

