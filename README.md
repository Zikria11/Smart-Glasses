# 🧠 Smart Glasses with ESP32

Smart Glasses is a wearable, IoT-powered embedded system built using the ESP32 microcontroller. Designed for real-time interaction, this project enables hands-free access to essential information like time, weather, and phone notifications through a minimal OLED display. It features a capacitive touch interface for mode switching, integrated battery monitoring, and planned AI-based features in future upgrades.

Developed as an innovative DIY and IoT-semester project, these glasses demonstrate practical implementation of embedded software, low-power wearable electronics, and smart device integration.

---

## 🧰 Key Features

| Feature                     | Description                                                                 |
|----------------------------|-----------------------------------------------------------------------------|
| 📶 **WiFi Auto-Setup**      | ESP32 launches a configuration portal using WiFiManager on first boot       |
| 🕒 **Time Mode**            | Displays live time from an NTP server                                       |
| 🌦 **Weather Mode**         | Fetches real-time weather data using [TomTom Weather API](https://developer.tomtom.com/) |
| 🔔 **Notification Mode**    | Receives and displays push notifications from a paired Android device      |
| 🔋 **Battery Monitoring**   | Real-time display of battery voltage and charge status                      |
| 👆 **Touch-based Control**  | Change display modes with a simple tap on capacitive touch sensor           |
| ⚡ **Low Power Optimization**| Efficient display updates and sleep functions to extend battery life       |
| 📷 **ESP32-CAM Support (Future)** | For computer vision integration like face detection or photo capture   |

---

## 🧑‍💻 Tech Stack

- **Microcontroller**: ESP32 Dev Board (with WiFi + Bluetooth)
- **Display**: OLED 0.96" SSD1306 (I2C)
- **Programming Language**: C++ (Arduino)
- **IDE**: Arduino IDE / PlatformIO
- **APIs**: TomTom Weather, NTP
- **Android App**: Custom APK to send notifications via local WiFi

---

## 🧱 Hardware Components

| Component                 | Purpose                                 |
|--------------------------|-----------------------------------------|
| ESP32 Dev Board          | Main microcontroller with WiFi support  |
| OLED Display (SSD1306)   | UI interface for user                   |
| TP4056 Charging Module   | Battery management and USB charging     |
| LiPo Battery (3.7V)      | Portable power source                   |
| Capacitive Touch Sensor  | Physical user input (mode switching)    |
| ESP32-CAM (Optional)     | Vision-based upgrades                   |
| DHT11 / Sound Sensor     | Experimental extensions                 |
| Bone Conduction Speaker  | Future sound integration                |
| Glasses Frame (3D printed / DIY) | Wearable housing                 |

---

## 📁 Project Directory
