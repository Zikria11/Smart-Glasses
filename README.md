# 🧠 Smart Glasses with ESP32

Smart Glasses is a wearable, IoT-powered embedded system built using the ESP32 microcontroller. Designed for real-time interaction, this project enables hands-free access to essential information like time, weather, and phone notifications through a compact OLED display. It supports WiFi and Bluetooth-based notification delivery, touch-based mode switching, and includes battery monitoring for portable usage.

Developed as an innovative IoT-semester project, these glasses demonstrate practical implementation of embedded software, wearable computing, and smart device integration.

---

## 🧰 Key Features

| Feature                     | Description                                                                 |
|----------------------------|-----------------------------------------------------------------------------|
| 📶 **WiFi Auto-Setup**      | ESP32 launches a configuration portal using WiFiManager on first boot       |
| 🕒 **Time Mode**            | Displays live time from an NTP server                                       |
| 🌦 **Weather Mode**         | Fetches weather data via [TomTom Weather API](https://developer.tomtom.com/) |
| 🔔 **Notification Mode**    | Receives notifications via **BLE** using the **Chrono Android App**        |
| 🔋 **Battery Monitoring**   | Shows real-time battery voltage and charge status                          |
| 👆 **Touch-based Control**  | Capacitive touch sensor cycles through display modes                       |
| ⚡ **Low Power Optimization**| Power-efficient display and connectivity design                            |
| 📷 **ESP32-CAM Support (Future)** | Vision-based features like facial detection                      |

---

## 🧑‍💻 Tech Stack

- **Microcontroller**: ESP32 Dev Board (WiFi + BLE support)
- **Display**: OLED 0.96" SSD1306 (I2C)
- **Language**: C++ (Arduino)
- **IDE**: Arduino IDE / PlatformIO
- **APIs**: TomTom Weather API, NTP server
- **Android App**: [Chrono Bluetooth App](https://play.google.com/store/apps/details?id=com.fbiego.ble)

---

## 📦 Hardware Components

| Component                 | Purpose                                 |
|--------------------------|-----------------------------------------|
| ESP32 Dev Board          | Main microcontroller                    |
| OLED Display (SSD1306)   | Main user interface                     |
| TP4056 Charging Module   | Safe LiPo charging                      |
| LiPo Battery (3.7V)      | Portable power                          |
| Capacitive Touch Sensor  | User input for switching modes          |
| ESP32-CAM (Optional)     | For future image-based features         |
| DHT11 / Sound Sensor     | Experimental extensions                 |
| Bone Conduction Speaker  | (Planned) Audio feedback                |
| Glasses Frame (DIY/3D print) | Wearable integration               |

---

## 📁 Project Directory

Smart-Glasses/
├── firmware/
│ ├── main.ino # Core firmware logic
│ ├── config.h # API keys and constants
├── android-app/ # Optional custom app for sending data
├── docs/
│ ├── schematics.pdf # Circuit diagrams
│ └── block-diagram.png # System overview
├── assets/ # Images and videos
└── README.md


---

## 📡 Connectivity Options

### 🔷 BLE Notifications via Chrono App

This project supports Bluetooth notifications via the **Chrono app**, available on Android:

- App: [Chrono - BLE Notifications](https://play.google.com/store/apps/details?id=com.fbiego.ble)
- Connect the app to the ESP32 BLE service
- Notifications from your phone will be forwarded to the OLED

**Note**: BLE connectivity and data parsing were heavily inspired by the [fbiego/ESP32_OLED_BLE](https://github.com/fbiego/ESP32_OLED_BLE) open-source project. Special thanks for the BLE UART structure and OLED integration.

---

### 📶 WiFi Configuration

- On first boot, ESP32 opens a WiFi AP (e.g., `SmartGlass-Setup`)
- Connect via phone and enter WiFi credentials
- ESP32 stores these for future boots and auto-connects

---

## 🖥 Display Modes

| Mode             | Info Displayed                            |
|------------------|--------------------------------------------|
| Time             | Current time (via NTP)                     |
| Weather          | Temperature, humidity, and condition       |
| Notifications    | Latest phone notification (via BLE)        |
| Battery Status   | Current battery voltage and health         |

Tap the **Touch Sensor** to switch modes.

---

## 📜 API Key Configuration

Create a file named `config.h` in the `firmware/` directory:

```cpp
#define TOMTOM_API_KEY "m2wArn6paTSv7prXcXK2cUhclrrFG9aC"
#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET +5  // Adjust to your region
