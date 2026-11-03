#include "web_interface.h"
#include "motion_controller.h"
#include "servo_manager.h"
#include "tmc_driver.h"
#include "c3_receiver.h"
#include <LittleFS.h>

// Fallback HTML page embedded in Flash
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Arm V2 Control</title>
    <style>
        :root {
            --bg-primary: #0d1117;
            --bg-secondary: #161b22;
            --bg-card: #21262d;
            --accent-blue: #58a6ff;
            --accent-green: #2ea043;
            --accent-red: #da3633;
            --accent-yellow: #d29922;
            --text-main: #f0f6fc;
            --text-dim: #8b949e;
            --border: #30363d;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background: var(--bg-primary); color: var(--text-main); padding: 16px; }
        .header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border); padding-bottom: 12px; margin-bottom: 20px; }
        .status-badge { padding: 4px 10px; border-radius: 12px; font-size: 13px; font-weight: bold; }
        .connected { background: #238636; color: white; }
        .disconnected { background: var(--accent-red); color: white; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 16px; }
        .card { background: var(--bg-card); border: 1px solid var(--border); border-radius: 8px; padding: 16px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); }
        h2 { font-size: 16px; margin-bottom: 12px; color: var(--accent-blue); display: flex; justify-content: space-between; }
        .axis-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px; padding: 8px; background: var(--bg-secondary); border-radius: 6px; }
        .axis-name { font-weight: bold; width: 60px; font-size: 15px; }
        .axis-values { flex: 1; text-align: center; }
        .val-stepper { color: var(--accent-blue); font-size: 14px; font-weight: bold; }
        .val-sensor { color: var(--accent-green); font-size: 13px; }
        .jog-btns { display: flex; gap: 6px; }
        button { cursor: pointer; border: none; outline: none; border-radius: 4px; padding: 8px 12px; font-size: 13px; font-weight: bold; transition: 0.2s; }
        .btn-jog { background: var(--border); color: var(--text-main); min-width: 40px; }
        .btn-jog:hover { background: #484f58; }
        .btn-primary { background: #1f6feb; color: white; }
        .btn-primary:hover { background: #388bfd; }
        .btn-danger { background: var(--accent-red); color: white; width: 100%; padding: 12px; font-size: 16px; margin-top: 10px; }
        .btn-danger:hover { background: #f85149; }
        .btn-action { background: #238636; color: white; }
        .btn-action:hover { background: #2ea043; }
        .slider-row { margin-bottom: 14px; }
        .slider-label { display: flex; justify-content: space-between; margin-bottom: 4px; font-size: 13px; color: var(--text-dim); }
        input[type=range] { width: 100%; height: 6px; border-radius: 3px; background: var(--border); outline: none; }
        .sensor-status { font-size: 12px; color: var(--text-dim); }
        .sensor-ok { color: var(--accent-green); }
        .sensor-fail { color: var(--accent-red); }
        .quick-actions { display: flex; gap: 8px; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🦾 V2 Robot Arm Controller</h1>
        <div>
            <span id="c3-badge" class="status-badge disconnected">C3: OFF</span>
            <span id="ws-badge" class="status-badge disconnected">WS: Disconnected</span>
        </div>
    </div>

    <div class="grid">
        <!-- JOINT STEPPERS & SENSORS -->
        <div class="card">
            <h2>4-Axis Steppers & AS5600 Feedback <span>TMC2209</span></h2>
            
            <div class="axis-row">
                <span class="axis-name">X (Base)</span>
                <div class="axis-values">
                    <div class="val-stepper">Cmd: <span id="pos-0">0.0</span></div>
                    <div class="val-sensor">AS5600: <span id="sensor-0">0.0</span>°</div>
                </div>
                <div class="jog-btns">
                    <button class="btn-jog" onclick="jog(0, -5)">-5</button>
                    <button class="btn-jog" onclick="jog(0, 5)">+5</button>
                </div>
            </div>

            <div class="axis-row">
                <span class="axis-name">Y (Shoulder)</span>
                <div class="axis-values">
                    <div class="val-stepper">Cmd: <span id="pos-1">0.0</span></div>
                    <div class="val-sensor">AS5600: <span id="sensor-1">0.0</span>°</div>
                </div>
                <div class="jog-btns">
                    <button class="btn-jog" onclick="jog(1, -5)">-5</button>
                    <button class="btn-jog" onclick="jog(1, 5)">+5</button>
                </div>
            </div>

            <div class="axis-row">
                <span class="axis-name">Z (Elbow)</span>
                <div class="axis-values">
                    <div class="val-stepper">Cmd: <span id="pos-2">0.0</span></div>
                    <div class="val-sensor">AS5600: <span id="sensor-2">0.0</span>°</div>
                </div>
                <div class="jog-btns">
                    <button class="btn-jog" onclick="jog(2, -5)">-5</button>
                    <button class="btn-jog" onclick="jog(2, 5)">+5</button>
                </div>
            </div>

            <div class="axis-row">
                <span class="axis-name">A (Wrist)</span>
                <div class="axis-values">
                    <div class="val-stepper">Cmd: <span id="pos-3">0.0</span></div>
                    <div class="val-sensor">AS5600: <span id="sensor-3">0.0</span>°</div>
                </div>
                <div class="jog-btns">
                    <button class="btn-jog" onclick="jog(3, -5)">-5</button>
                    <button class="btn-jog" onclick="jog(3, 5)">+5</button>
                </div>
            </div>

            <div class="quick-actions">
                <button class="btn-action" style="flex:1" onclick="sendCmd({cmd:'zero_all'})">Set Zero</button>
                <button class="btn-primary" style="flex:1" onclick="toggleMotors()">Toggle Power</button>
            </div>
        </div>

        <!-- END EFFECTOR SERVOS -->
        <div class="card">
            <h2>End Effector (2x Servo) <span>GPIO 32 / 33</span></h2>
            
            <div class="slider-row">
                <div class="slider-label">
                    <span>Servo B (Rotate)</span>
                    <span id="lbl-servo-b">90°</span>
                </div>
                <input id="slider-rotate" type="range" min="0" max="180" value="90" oninput="updateServoRotate(this.value)">
            </div>

            <div class="slider-row">
                <div class="slider-label">
                    <span>Servo C (Gripper)</span>
                    <span id="lbl-servo-c">90°</span>
                </div>
                <input id="slider-gripper" type="range" min="0" max="180" value="90" oninput="updateServoGripper(this.value)">
            </div>

            <div class="quick-actions">
                <button class="btn-primary" style="flex:1" onclick="quickGrip('open')">👐 Full Open</button>
                <button class="btn-action" style="flex:1" onclick="quickGrip('close')">✊ Full Close</button>
            </div>

            <button class="btn-danger" onclick="sendCmd({cmd:'estop'})">🚨 EMERGENCY STOP</button>
        </div>
    </div>

    <script>
        let ws;
        let motorsEnabled = true;

        function connectWs() {
            const proto = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const host = window.location.hostname || '192.168.4.1';
            ws = new WebSocket(`${proto}//${host}:81/`);

            ws.onopen = () => {
                const b = document.getElementById('ws-badge');
                b.textContent = 'WS: Connected';
                b.className = 'status-badge connected';
            };

            ws.onclose = () => {
                const b = document.getElementById('ws-badge');
                b.textContent = 'WS: Disconnected';
                b.className = 'status-badge disconnected';
                setTimeout(connectWs, 2000);
            };

            ws.onmessage = (evt) => {
                try {
                    const data = JSON.parse(evt.data);
                    if (data.type === 'telemetry') {
                        // Steppers
                        data.steppers.forEach((s, idx) => {
                            const el = document.getElementById(`pos-${idx}`);
                            if (el) el.textContent = s.pos.toFixed(1);
                        });
                        // C3 Sensors
                        const c3Badge = document.getElementById('c3-badge');
                        if (data.sensors && data.sensors.c3_connected) {
                            c3Badge.textContent = 'C3: Online';
                            c3Badge.className = 'status-badge connected';
                            data.sensors.actual.forEach((val, idx) => {
                                const sEl = document.getElementById(`sensor-${idx}`);
                                if (sEl) sEl.textContent = val.toFixed(1);
                            });
                        } else {
                            c3Badge.textContent = 'C3: Offline';
                            c3Badge.className = 'status-badge disconnected';
                        }
                    }
                } catch(e) {}
            };
        }

        function sendCmd(obj) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(obj));
            }
        }

        function jog(axis, delta) {
            sendCmd({cmd: 'jog', axis: axis, delta: delta});
        }

        function updateServoRotate(val) {
            document.getElementById('lbl-servo-b').textContent = val + '°';
            sendCmd({cmd: 'servo_rotate', val: parseInt(val)});
        }

        function updateServoGripper(val) {
            document.getElementById('lbl-servo-c').textContent = val + '°';
            sendCmd({cmd: 'servo_grip', val: parseInt(val)});
        }

        function quickGrip(action) {
            sendCmd({cmd: 'gripper_quick', action: action});
        }

        function toggleMotors() {
            motorsEnabled = !motorsEnabled;
            sendCmd({cmd: 'enable_motors', state: motorsEnabled});
        }

        window.addEventListener('load', connectWs);
    </script>
</body>
</html>
)rawliteral";

WebInterface::WebInterface(MotionController& motion, ServoManager& servos, TmcManager& tmc, C3Receiver& c3)
    : server(WEB_SERVER_PORT), webSocket(WS_SERVER_PORT),
      motionCtrl(motion), servoMgr(servos), tmcMgr(tmc), c3Recv(c3) {}

void WebInterface::begin() {
    // Start WiFi in AP mode (Access Point)
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("[WIFI] Access Point Started! SSID: ");
    Serial.println(WIFI_AP_SSID);
    Serial.print("[WIFI] IP Address: ");
    Serial.println(IP);

    setupRoutes();
    server.begin();

    // Start WebSocket
    webSocket.begin();
    webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        this->handleWebSocketEvent(num, type, payload, length);
    });

    Serial.println("[WEB] HTTP & WebSocket Servers started.");
}

void WebInterface::setupRoutes() {
    server.on("/", HTTP_GET, [this]() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    server.on("/api/status", HTTP_GET, [this]() {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });
}

void WebInterface::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        String msg = String((char*)payload).substring(0, length);
        handleCommand(msg);
    }
}

void WebInterface::handleCommand(const String& jsonStr) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) return;

    const char* cmd = doc["cmd"];
    if (!cmd) return;

    if (strcmp(cmd, "jog") == 0) {
        uint8_t axis = doc["axis"] | 0;
        float delta = doc["delta"] | 0.0f;
        motionCtrl.jogAxis(axis, delta);
    } else if (strcmp(cmd, "move_to") == 0) {
        float x = doc["x"] | 0.0f;
        float y = doc["y"] | 0.0f;
        float z = doc["z"] | 0.0f;
        float a = doc["a"] | 0.0f;
        motionCtrl.moveTo(x, y, z, a);
    } else if (strcmp(cmd, "servo_rotate") == 0) {
        int val = doc["val"] | 90;
        servoMgr.setRotateAngle(val);
    } else if (strcmp(cmd, "servo_grip") == 0) {
        int val = doc["val"] | 90;
        servoMgr.setGripperAngle(val);
    } else if (strcmp(cmd, "gripper_quick") == 0) {
        const char* action = doc["action"];
        if (action && strcmp(action, "open") == 0) servoMgr.openGripper();
        else servoMgr.closeGripper();
    } else if (strcmp(cmd, "enable_motors") == 0) {
        bool state = doc["state"] | true;
        tmcMgr.enableSteppers(state);
    } else if (strcmp(cmd, "zero_all") == 0) {
        motionCtrl.setCurrentPositionAsZero();
    } else if (strcmp(cmd, "estop") == 0) {
        motionCtrl.emergencyStop();
    } else if (strcmp(cmd, "calib_c3") == 0) {
        uint8_t axis = doc["axis"] | 0;
        c3Recv.sendCalibrateCommand(axis);
    }
}

void WebInterface::broadcastTelemetry() {
    StaticJsonDocument<512> doc;
    doc["type"] = "telemetry";

    JsonArray stArr = doc.createNestedArray("steppers");
    for (int i = 0; i < 4; i++) {
        auto st = motionCtrl.getAxisState(i);
        JsonObject obj = stArr.createNestedObject();
        obj["pos"] = st.currentPos;
        obj["target"] = st.targetPos;
        obj["moving"] = st.isMoving;
    }

    JsonObject sObj = doc.createNestedObject("sensors");
    const auto& c3State = c3Recv.getState();
    sObj["c3_connected"] = c3State.isConnected;
    JsonArray actArr = sObj.createNestedArray("actual");
    JsonArray hArr = sObj.createNestedArray("healthy");
    for (int i = 0; i < 4; i++) {
        actArr.add(c3State.actualAngle[i]);
        hArr.add(c3State.sensorHealthy[i]);
    }

    JsonObject srvObj = doc.createNestedObject("servos");
    srvObj["rotate"] = servoMgr.getRotateAngle();
    srvObj["gripper"] = servoMgr.getGripperAngle();

    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
}

void WebInterface::update() {
    server.handleClient();
    webSocket.loop();

    unsigned long now = millis();
    if (now - lastBroadcastTime >= 50) { // 20Hz update
        lastBroadcastTime = now;
        broadcastTelemetry();
    }
}

