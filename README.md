# 🦾 Dự Án Robot Arm V2 - Điều Khiển Thông Minh Qua Web

Dự án điều khiển cánh tay robot thông minh đa trục sử dụng **ESP32 Dev Module (MCU Chính)**, **ESP32-C3 Super Mini (Sub-MCU đọc cảm biến)**, 4 động cơ bước điều khiển bởi **TMC2209**, 4 cảm biến góc từ tính **AS5600** qua multiplexer **TCA9548A**, và 2 **RC Servo** cho bộ kẹp (Gripper).

Giao diện Web điều khiển thời gian thực qua **WebSocket** với phản hồi góc thực tế trực quan.

---

## 📐 1. Sơ Đồ Khối & Nguyên Lý Hoạt Động

```
  [ Web Dashboard (Browser / Điện thoại / Máy tính) ]
                          │
                   (WiFi WebSocket)
                          ▼
             [ ESP32 Dev Module (MCU Chính) ]
              │                   │
         (Bus UART1)          (PWM 50Hz)
              │                   │
        [ 4x TMC2209 ]       [ 2x RC Servo ]
      (Trục X, Y, Z, A)     (B: Xoay, C: Kẹp)
              │
          (UART2: TX=17, RX=16)
              ▲
              │ Phản hồi góc $ARM,degX,degY,degZ,degA,status
              │
        [ ESP32-C3 Super Mini (Sensor Hub) ]
              │
             (I2C)
              ▼
        [ TCA9548A I2C MUX ]
    ├── Kênh 0 ──> AS5600 Trục X (Base)
    ├── Kênh 1 ──> AS5600 Trục Y (Shoulder)
    ├── Kênh 2 ──> AS5600 Trục Z (Elbow)
    └── Kênh 3 ──> AS5600 Trục A (Wrist)
```

---

## 🔌 2. Bảng Đấu Nối Phần Cứng (Pinout)

### 2.1 MCU Chính (ESP32 Dev Module)
| Chân ESP32 | Chức năng | Ghi chú |
| :--- | :--- | :--- |
| **GPIO 13** | Stepper Enable/Disable chung | Mức LOW = Bật, HIGH = Tắt động cơ |
| **GPIO 23** | TMC2209 UART TX | Nối với chân RX của cả 4 TMC2209 |
| **GPIO 22** | TMC2209 UART RX | Nối với chân TX của cả 4 TMC2209 qua trở 1kΩ |
| **GPIO 14** | Trục X (Base) STEP | TMC2209 Addr 0 |
| **GPIO 27** | Trục X (Base) DIR | |
| **GPIO 35** | Trục X Limit / StallGuard | Công tắc hành trình hoặc DIAG |
| **GPIO 26** | Trục Y (Shoulder) STEP | TMC2209 Addr 1 |
| **GPIO 25** | Trục Y (Shoulder) DIR | |
| **GPIO 34** | Trục Y Limit / StallGuard | |
| **GPIO 21** | Trục Z (Elbow) STEP | TMC2209 Addr 2 |
| **GPIO 19** | Trục Z (Elbow) DIR | |
| **GPIO 36** | Trục Z Limit / StallGuard | |
| **GPIO 18** | Trục A (Wrist) STEP | TMC2209 Addr 3 |
| **GPIO 4**  | Trục A (Wrist) DIR | |
| **GPIO 39** | Trục A Limit / StallGuard | |
| **GPIO 32** | Servo B (Xoay kẹp) | PWM 50Hz, 1000 - 2000µs |
| **GPIO 33** | Servo C (Kẹp / Nhả) | PWM 50Hz, 1000 - 2000µs |
| **GPIO 17** | UART2 TX ──> ESP32-C3 RX | Baud 115200 |
| **GPIO 16** | UART2 RX <── ESP32-C3 TX | Baud 115200 |

### 2.2 MCU Phụ (ESP32-C3 Super Mini)
| Chân ESP32-C3 | Chức năng | Ghi chú |
| :--- | :--- | :--- |
| **GPIO 8** | I2C SDA | Nối chân SDA của TCA9548A |
| **GPIO 9** | I2C SCL | Nối chân SCL của TCA9548A |
| **GPIO 21** | UART TX | Nối sang GPIO 16 (RX) của MCU chính |
| **GPIO 20** | UART RX | Nối sang GPIO 17 (TX) của MCU chính |
| **3V3 & GND** | Nguồn cảm biến | |

### 2.3 Phân Kênh TCA9548A cho 4 AS5600
- **Kênh SD0 / SC0**: AS5600 đuôi động cơ X (Trục xoay đế)
- **Kênh SD1 / SC1**: AS5600 đuôi động cơ Y (Khớp vai)
- **Kênh SD2 / SC2**: AS5600 đuôi động cơ Z (Khớp khuỷu)
- **Kênh SD3 / SC3**: AS5600 đuôi động cơ A (Khớp cổ tay)

---

## 🚀 3. Hướng Dẫn Biên Dịch & Nạp Code (PlatformIO)

Dự án đã được cấu hình sẵn 2 môi trường (Environment) trong `platformio.ini`:

### 3.1 Nạp cho ESP32-C3 Super Mini
1. Cắm cáp USB vào ESP32-C3.
2. Mở tab PlatformIO, chọn:
   ```
   Default / env:esp32_c3_sensor -> Upload
   ```
   Hoặc chạy lệnh:
   ```bash
   pio run -e esp32_c3_sensor -t upload
   ```

### 3.2 Nạp cho MCU Chính (ESP32 Dev Module)
1. Cắm cáp USB vào ESP32 Dev Module.
2. Mở tab PlatformIO, chọn:
   ```
   Default / env:esp32_main -> Upload
   ```
   Hoặc chạy lệnh:
   ```bash
   pio run -e esp32_main -t upload
   ```

---

## 🌐 4. Kết Nối & Điều Khiển Web

1. Khi MCU chính khởi động, nó tự phát mạng Wi-Fi:
   - **Tên Wi-Fi (SSID):** `RobotArm_V2_AP`
   - **Mật khẩu:** `12345678`
2. Kết nối máy tính hoặc điện thoại vào mạng Wi-Fi trên.
3. Mở trình duyệt web bất kỳ (Chrome, Edge, Safari) và truy cập:
   ```
   http://192.168.4.1
   ```
4. Giao diện điều khiển thời gian thực xuất hiện với:
   - Thanh trượt và nút Jog điều khiển từng độ cho 4 trục.
   - Mô phỏng trực quan 2D hiển thị góc đặt (màu xanh cyan) và góc thực tế AS5600 (màu xanh lá cây).
   - Điều khiển góc xoay và độ mở kẹp của 2 Servo.
   - Tính năng **Teach & Play**: Lưu các điểm vị trí và cho cánh tay chạy tự động theo chu trình.
   - Nút **E-STOP (Dừng khẩn cấp)** lập tức dừng toàn bộ chuyển động.

---

## 📁 5. Cấu Trúc Thư Mục Dự Án

```
V2-Robot-Arm/
├── platformio.ini              # Cấu hình 2 môi trường nạp cho cả 2 ESP32
├── src/
│   ├── main_mcu/               # Firmware MCU Chính ESP32 Dev
│   │   ├── config.h            # Định nghĩa toàn bộ chân & thông số động cơ
│   │   ├── tmc_driver.*        # Giao tiếp UART điều khiển 4x TMC2209
│   │   ├── motion_controller.* # Tạo xung mượt FastAccelStepper qua RMT
│   │   ├── servo_manager.*     # Điều khiển 2 Servo kẹp & xoay
│   │   ├── c3_receiver.*       # Nhận và giải mã gói tin góc từ C3
│   │   ├── web_interface.*     # Máy chủ Web & WebSocket nhúng
│   │   └── main.cpp            # Vòng lặp điều phối chính
│   │
│   └── c3_sensor/              # Firmware ESP32-C3 Super Mini
│       ├── c3_config.h         # Cấu hình chân I2C & UART
│       ├── tca9548a.h          # Driver chuyển kênh I2C TCA9548A
│       ├── as5600_manager.*    # Đọc góc 12-bit & khử trôi góc 4x AS5600
│       └── main.cpp            # Đọc chu kỳ 50Hz & phát UART sang MCU chính
│
└── web/                        # Web Dashboard độc lập
    ├── index.html              # Giao diện điều khiển Cyber Dashboard
    ├── css/style.css           # Giao diện Dark mode hiện đại
    └── js/app.js               # Quản lý WebSocket, đồ họa cánh tay & Teach-Play
```

