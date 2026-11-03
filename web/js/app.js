// Robot Arm V2 - Frontend Application Controller
let ws = null;
let isConnected = false;
let currentTelemetry = null;
let waypoints = [];
let isPlayingSequence = false;

// Default Joint targets
const state = {
    joints: [0, 0, 0, 0],       // X, Y, Z, A targets
    actualJoints: [0, 0, 0, 0], // From AS5600 feedback
    servoRotate: 90,
    servoGripper: 90,
    motorsEnabled: true
};

// Canvas Kinematics Visualization
const canvas = document.getElementById('armCanvas');
const ctx = canvas.getContext('2d');

function resizeCanvas() {
    canvas.width = canvas.parentElement.clientWidth;
    canvas.height = canvas.parentElement.clientHeight;
    drawArm();
}
window.addEventListener('resize', resizeCanvas);

// ----------------- WEBSOCKET -----------------
function connectWebSocket() {
    const ip = document.getElementById('esp-ip').value.trim() || '192.168.4.1';
    const wsUrl = `ws://${ip}:81/`;

    updateStatusBadge('ws-status', 'Connecting...', 'status-offline');

    if (ws) {
        ws.close();
    }

    ws = new WebSocket(wsUrl);

    ws.onopen = () => {
        isConnected = true;
        updateStatusBadge('ws-status', 'Connected', 'status-online');
        console.log('[WS] Connected to ' + wsUrl);
    };

    ws.onclose = () => {
        isConnected = false;
        updateStatusBadge('ws-status', 'Disconnected', 'status-offline');
        updateStatusBadge('c3-status', 'C3: Offline', 'status-offline');
    };

    ws.onerror = (err) => {
        console.error('[WS] Error:', err);
    };

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            handleTelemetry(data);
        } catch (e) {}
    };
}

function sendCommand(obj) {
    if (ws && isConnected && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(obj));
    }
}

// ----------------- TELEMETRY PARSER -----------------
function handleTelemetry(data) {
    if (data.type !== 'telemetry') return;
    currentTelemetry = data;

    // 1. Update Stepper States
    if (data.steppers) {
        data.steppers.forEach((s, i) => {
            document.getElementById(`cmd-val-${i}`).textContent = s.pos.toFixed(1);
        });
    }

    // 2. Update C3 AS5600 Feedback
    if (data.sensors) {
        if (data.sensors.c3_connected) {
            updateStatusBadge('c3-status', 'C3: Online', 'status-online');
            data.sensors.actual.forEach((val, i) => {
                state.actualJoints[i] = val;
                document.getElementById(`actual-val-${i}`).textContent = val.toFixed(1) + '°';
            });
        } else {
            updateStatusBadge('c3-status', 'C3: Offline', 'status-offline');
        }
    }

    // 3. Update Servos
    if (data.servos) {
        document.getElementById('lbl-rotate-val').textContent = data.servos.rotate + '°';
        document.getElementById('lbl-gripper-val').textContent = data.servos.gripper + '°';
    }

    drawArm();
}

function updateStatusBadge(id, text, className) {
    const el = document.getElementById(id);
    if (!el) return;
    el.className = `badge ${className}`;
    el.innerHTML = `<span class="badge-dot"></span> ${text}`;
}

// ----------------- JOINT CONTROLS -----------------
function onSliderChange(axisIndex, value) {
    const val = parseFloat(value);
    state.joints[axisIndex] = val;
    document.getElementById(`cmd-val-${axisIndex}`).textContent = val.toFixed(1);

    sendCommand({
        cmd: 'move_to',
        x: state.joints[0],
        y: state.joints[1],
        z: state.joints[2],
        a: state.joints[3]
    });
    drawArm();
}

function jog(axisIndex, delta) {
    sendCommand({ cmd: 'jog', axis: axisIndex, delta: delta });
}

// ----------------- SERVOS -----------------
function onRotateSlider(val) {
    state.servoRotate = parseInt(val);
    document.getElementById('lbl-rotate-val').textContent = val + '°';
    sendCommand({ cmd: 'servo_rotate', val: state.servoRotate });
}

function onGripperSlider(val) {
    state.servoGripper = parseInt(val);
    document.getElementById('lbl-gripper-val').textContent = val + '°';
    sendCommand({ cmd: 'servo_grip', val: state.servoGripper });
}

function quickGrip(action) {
    sendCommand({ cmd: 'gripper_quick', action: action });
}

// ----------------- ACTIONS -----------------
function setZero() {
    if (confirm('Calibrate current position as Zero (0) for all axes?')) {
        sendCommand({ cmd: 'zero_all' });
        state.joints = [0, 0, 0, 0];
        for (let i = 0; i < 4; i++) {
            const slider = document.getElementById(`slider-${i}`);
            if (slider) slider.value = 0;
            document.getElementById(`cmd-val-${i}`).textContent = '0.0';
        }
        drawArm();
    }
}

function togglePower() {
    state.motorsEnabled = !state.motorsEnabled;
    sendCommand({ cmd: 'enable_motors', state: state.motorsEnabled });
    const btn = document.getElementById('btn-power');
    btn.textContent = state.motorsEnabled ? '⚡ Motors: ON' : '🔌 Motors: OFF';
}

function emergencyStop() {
    sendCommand({ cmd: 'estop' });
    alert('🚨 EMERGENCY STOP ACTIVATED!');
}

// ----------------- TEACH & PLAYBACK (WAYPOINTS) -----------------
function recordWaypoint() {
    const pt = {
        id: waypoints.length + 1,
        x: currentTelemetry?.steppers?.[0]?.pos ?? state.joints[0],
        y: currentTelemetry?.steppers?.[1]?.pos ?? state.joints[1],
        z: currentTelemetry?.steppers?.[2]?.pos ?? state.joints[2],
        a: currentTelemetry?.steppers?.[3]?.pos ?? state.joints[3],
        rot: state.servoRotate,
        grip: state.servoGripper,
        delay: 1000 // default 1 second wait
    };
    waypoints.push(pt);
    renderWaypoints();
}

function renderWaypoints() {
    const tbody = document.getElementById('wp-table-body');
    tbody.innerHTML = '';
    waypoints.forEach((wp, idx) => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>#${wp.id}</td>
            <td>${wp.x.toFixed(1)}</td>
            <td>${wp.y.toFixed(1)}</td>
            <td>${wp.z.toFixed(1)}</td>
            <td>${wp.a.toFixed(1)}</td>
            <td>${wp.grip}°</td>
            <td><button class="btn btn-gray" style="padding:2px 6px; font-size:10px;" onclick="removeWaypoint(${idx})">✕</button></td>
        `;
        tbody.appendChild(tr);
    });
}

function removeWaypoint(idx) {
    waypoints.splice(idx, 1);
    renderWaypoints();
}

function clearWaypoints() {
    waypoints = [];
    renderWaypoints();
}

async function playSequence() {
    if (waypoints.length === 0 || isPlayingSequence) return;
    isPlayingSequence = true;
    const btn = document.getElementById('btn-play-seq');
    btn.textContent = '⏳ Running...';

    for (let i = 0; i < waypoints.length; i++) {
        if (!isPlayingSequence) break;
        const pt = waypoints[i];
        
        // Move arm
        sendCommand({ cmd: 'move_to', x: pt.x, y: pt.y, z: pt.z, a: pt.a });
        sendCommand({ cmd: 'servo_rotate', val: pt.rot });
        sendCommand({ cmd: 'servo_grip', val: pt.grip });

        // Wait delay
        await new Promise(r => setTimeout(r, pt.delay));
    }

    isPlayingSequence = false;
    btn.textContent = '▶ Run Sequence';
}

// ----------------- 2D VISUALIZER -----------------
function drawArm() {
    if (!canvas.width) return;
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const originX = canvas.width / 2;
    const originY = canvas.height - 40;

    // Draw Ground Base
    ctx.strokeStyle = '#2a384e';
    ctx.lineWidth = 4;
    ctx.beginPath();
    ctx.moveTo(originX - 60, originY + 10);
    ctx.lineTo(originX + 60, originY + 10);
    ctx.stroke();

    // Link lengths in pixels
    const L1 = 70; // Shoulder link
    const L2 = 65; // Elbow link
    const L3 = 45; // Wrist link

    // Calculate Target Link Angles (radians)
    const radY = (-state.joints[1] + 90) * (Math.PI / 180);
    const radZ = (-state.joints[2]) * (Math.PI / 180);
    const radA = (-state.joints[3]) * (Math.PI / 180);

    const x1 = originX + L1 * Math.cos(radY);
    const y1 = originY - L1 * Math.sin(radY);

    const x2 = x1 + L2 * Math.cos(radY + radZ);
    const y2 = y1 - L2 * Math.sin(radY + radZ);

    const x3 = x2 + L3 * Math.cos(radY + radZ + radA);
    const y3 = y2 - L3 * Math.sin(radY + radZ + radA);

    // 1. Draw Target Silhouette (Cyan Glow)
    ctx.strokeStyle = '#00d2ff';
    ctx.lineWidth = 6;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
    ctx.beginPath();
    ctx.moveTo(originX, originY);
    ctx.lineTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.lineTo(x3, y3);
    ctx.stroke();

    // 2. Draw Joints
    ctx.fillStyle = '#ffffff';
    [ [originX, originY], [x1, y1], [x2, y2], [x3, y3] ].forEach(([jx, jy]) => {
        ctx.beginPath();
        ctx.arc(jx, jy, 5, 0, Math.PI * 2);
        ctx.fill();
    });

    // 3. Draw Actual from AS5600 (Green dashed line if available)
    if (currentTelemetry?.sensors?.c3_connected) {
        const actY = (-state.actualJoints[1] + 90) * (Math.PI / 180);
        const actZ = (-state.actualJoints[2]) * (Math.PI / 180);
        const actA = (-state.actualJoints[3]) * (Math.PI / 180);

        const ax1 = originX + L1 * Math.cos(actY);
        const ay1 = originY - L1 * Math.sin(actY);
        const ax2 = ax1 + L2 * Math.cos(actY + actZ);
        const ay2 = ay1 - L2 * Math.sin(actY + actZ);
        const ax3 = ax2 + L3 * Math.cos(actY + actZ + actA);
        const ay3 = ay2 - L3 * Math.sin(actY + actZ + actA);

        ctx.strokeStyle = '#00e676';
        ctx.lineWidth = 3;
        ctx.setLineDash([4, 4]);
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(ax1, ay1);
        ctx.lineTo(ax2, ay2);
        ctx.lineTo(ax3, ay3);
        ctx.stroke();
        ctx.setLineDash([]); // Reset dash
    }
}

// Auto init on load
window.addEventListener('load', () => {
    resizeCanvas();
    // Default ESP IP is current host if opened from ESP32 directly, otherwise 192.168.4.1
    if (window.location.hostname && window.location.hostname !== '') {
        document.getElementById('esp-ip').value = window.location.hostname;
    }
    connectWebSocket();
});

