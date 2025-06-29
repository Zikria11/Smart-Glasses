#include <WiFi.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// OLED Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Hardware Pins
#define TOUCH_PIN 4
#define BATTERY_PIN 35
#define LED_PIN 5
#define VIBRATION_PIN 18

// Animation Settings
#define MAX_PARTICLES 10
#define FRAME_DELAY 16
#define WAVE_AMPLITUDE 2
#define BOUNCE_AMPLITUDE 3
float waveOffset = 0;
float breathingScale = 1.0;
int bounceOffset = 0;
struct Particle {
  float x, y, vx, vy;
  int life, maxLife;
  bool active;
};
Particle particles[MAX_PARTICLES];

// Virtual Pet Settings
int petHunger = 0; // 0 to 100
int petHappiness = 100; // 0 to 100
unsigned long lastPetUpdate = 0;
const unsigned long petHungerInterval = 600000; // 10 minutes
unsigned long petAlertTime = 0;
bool petNeedsAttention = false;

// Pixelated Cat Sprite (16x16)
const uint16_t catSprite[16] = {
  0b0000001111000000,
  0b0000011111100000,
  0b0000111111110000,
  0b0001111111111000,
  0b0011111111111100,
  0b0111111111111110,
  0b1111100110011111,
  0b1111000110011111,
  0b1111000000001111,
  0b1111111001111111,
  0b1111111001111111,
  0b1111100000001111,
  0b0111110110111110,
  0b0011110110111100,
  0b0001110000011100,
  0b0000110000011000
};

// Traffic Settings
String trafficCondition = "Loading...";
String trafficColor = "GREEN"; // GREEN, YELLOW, RED
unsigned long lastTrafficUpdate = 0;
const unsigned long trafficUpdateInterval = 900000; // 15 minutes
bool useDemoTraffic = false; // Use real TomTom API

// Global States
volatile bool touchFlag = false;
unsigned long touchStartTime = 0;
const unsigned long longPressDuration = 1000;
const unsigned long debounceDelay = 150; // Reduced for better touch response
unsigned long lastTouchTime = 0;
int clickCount = 0;
unsigned long lastClickTime = 0;
unsigned long clickTimeout = 250; // Reduced for faster click detection
bool waitingForClick = false;
int mode = 0;
bool isTransitioning = false;
float transitionAlpha = 0.0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 300000;

// Weather API
String weatherApiKey = "7c1499c5ca1d24dc6f90e222c445ebb8";
String city = "Islamabad";
String weatherURL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + weatherApiKey + "&units=metric";
float currentTemp = 0;
String weatherDesc = "Loading...";
int humidity = 0;

// Traffic API
String trafficApiKey = "m2wArn6paTSv7prXcXK2cUhclrrFG9aC"; // Updated TomTom API key
String trafficURL = "https://api.tomtom.com/traffic/services/4/flowSegmentData/absolute/10/json?key=" + trafficApiKey + "&point=33.6844,73.0479"; // Islamabad coords

// Time Setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 18000; // PKT offset
const int daylightOffset_sec = 0;

// Mode Definitions
const int TOTAL_MODES = 4;
String modeNames[TOTAL_MODES] = {"Time", "Weather", "Virtual Pet", "Traffic"};
String modeIcons[TOTAL_MODES] = {"⏰", "🌤", "🐾", "🚗"};

// Function Declarations
void IRAM_ATTR onTouch();
void showTime();
void showWeather();
void showVirtualPet();
void showTraffic();
void drawTitleBar(const String& title);
void drawBattery();
void drawWiFiSignal();
void vibrate(int duration);
void updateWeather();
void updateTraffic();
void initParticles();
void updateParticles();
void createExplosion(int x, int y);
void drawWaveBackground();
void drawClock();

void IRAM_ATTR onTouch() {
  if (millis() - lastTouchTime > debounceDelay) {
    touchStartTime = millis();
    touchFlag = true;
    Serial.println("Touch detected"); // Debug touch interrupt
  }
}

void vibrate(int duration) {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(duration);
  digitalWrite(VIBRATION_PIN, LOW);
}

void createExplosion(int x, int y) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) {
      particles[i].x = x;
      particles[i].y = y;
      particles[i].vx = random(-20, 21) / 10.0;
      particles[i].vy = random(-20, 21) / 10.0;
      particles[i].life = random(15, 30);
      particles[i].maxLife = particles[i].life;
      particles[i].active = true;
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].life--;
      if (particles[i].life <= 0 || particles[i].x < 0 || particles[i].x > SCREEN_WIDTH || particles[i].x > SCREEN_WIDTH || particles[i].y < 0 || particles[i].y > SCREEN_HEIGHT) {
        particles[i].active = false;
      }
    }
  }
}

void initParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Ziki Smart Glasses...");

  // Initialize Pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(VIBRATION_PIN, LOW);

  // Initialize OLED
  Wire.begin(21, 22); // SDA=21, SCL=22
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Ziki Smart Glasses");
  display.println("Booting...");
  display.display();

  // Boot Animation
  for (int i = 0; i <= 100; i += 10) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Ziki Smart Glasses");
    int barWidth = map(i, 0, 100, 0, SCREEN_WIDTH - 20);
    display.fillRect(10, 40, barWidth, 8, SSD1306_WHITE);
    createExplosion(barWidth + 10, 44);
    updateParticles();
    display.display();
    delay(50);
  }

  // Touch Interrupt Setup
  touchAttachInterrupt(TOUCH_PIN, onTouch, 20); // Lowered threshold for better sensitivity

  // WiFi Setup
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect("SmartGlasses-Setup", "smart1234")) {
    Serial.println("WiFi failed, using AP mode");
    WiFi.softAP("SmartGlasses-Setup", "smart1234");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("WiFi Failed");
    display.println("AP: SmartGlasses-Setup");
    display.println("Pass: smart1234");
    display.display();
    // Stay in AP mode until configured
    while (true) {
      delay(1000);
    }
  } else {
    Serial.println("WiFi Connected: " + WiFi.localIP().toString());
    updateWeather();
    updateTraffic();
  }

  // Time Setup
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  initParticles();
}

void drawWaveBackground() {
  for (int x = 0; x < SCREEN_WIDTH; x += 4) {
    int y = SCREEN_HEIGHT - 10 + (int)(WAVE_AMPLITUDE * sin((x * 0.1) + waveOffset));
    display.drawPixel(x, y, SSD1306_WHITE);
  }
}

void loop() {
  Serial.println("Current mode: " + String(mode)); // Debug current mode

  // Update Animations
  waveOffset += 0.2;
  if (waveOffset > 2 * PI) waveOffset = 0;
  breathingScale = 1.0 + 0.2 * sin(millis() * 0.003);
  bounceOffset = (int)(BOUNCE_AMPLITUDE * sin(millis() * 0.005));
  updateParticles();

  // Update Virtual Pet
  if (millis() - lastPetUpdate > petHungerInterval) {
    petHunger = min(petHunger + 1, 100);
    if (petHunger > 80 && !petNeedsAttention) {
      petNeedsAttention = true;
      petAlertTime = millis();
      vibrate(150);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
    }
    lastPetUpdate = millis();
  }
  if (petNeedsAttention && millis() - petAlertTime > 5000) {
    petNeedsAttention = false;
  }

  // Handle Touch
  if (touchFlag) {
    Serial.println("Processing touch event"); // Debug touch processing
    touchFlag = false;
    unsigned long touchDuration = millis() - touchStartTime;

    if (touchDuration >= longPressDuration) {
      // Long Press Logic
      Serial.print("Long press in mode: ");
      Serial.println(mode);
      if (mode == 2) { // Virtual Pet: Play
        petHappiness = min(petHappiness + 10, 100);
        petHunger = max(petHunger - 5, 0);
        vibrate(100);
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
        createExplosion(SCREEN_WIDTH - 5, 20);
      } else if (mode == 3) { // Traffic: Refresh
        updateTraffic();
        vibrate(100);
      }
      waitingForClick = false;
      clickCount = 0;
    } else {
      // Short Tap: Count Clicks
      clickCount++;
      lastClickTime = millis();
      waitingForClick = true;
      Serial.println("Short tap, click count: " + String(clickCount));
    }

    lastTouchTime = millis();
  }

  // Evaluate Click Timing
  if (waitingForClick && (millis() - lastClickTime > clickTimeout)) {
    if (clickCount == 1) {
      // Single tap
      Serial.print("Single tap, switching to mode: ");
      if (mode == 2) { // Pet: Feed
        petHunger = max(petHunger - 10, 0);
        petHappiness = min(petHappiness + 5, 100);
        vibrate(50);
        createExplosion(SCREEN_WIDTH - 5, 20);
        Serial.println("Fed pet, staying in Virtual Pet");
      } else {
        mode = (mode + 1) % TOTAL_MODES;
        isTransitioning = true;
        transitionAlpha = 0.0;
        vibrate(50);
        createExplosion(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        Serial.println(mode);
      }
    } else if (clickCount == 2) {
      // Double tap
      Serial.print("Double tap in mode: ");
      Serial.println(mode);
      if (mode == 2 || mode == 3) {
        mode = 0; // Exit to Time
        vibrate(100);
        createExplosion(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        Serial.println("Exiting to Time Mode");
      }
    }

    clickCount = 0;
    waitingForClick = false;
  }

  // Update Weather and Traffic
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastWeatherUpdate > weatherUpdateInterval) {
      updateWeather();
      lastWeatherUpdate = millis();
    }
    if (millis() - lastTrafficUpdate > trafficUpdateInterval) {
      updateTraffic();
      lastTrafficUpdate = millis();
    }
  }

  // Render Display
  display.clearDisplay();
  drawWaveBackground();

  if (isTransitioning) {
    Serial.println("Transitioning, alpha: " + String(transitionAlpha)); // Debug transition
    transitionAlpha += 0.1;
    if (transitionAlpha >= 1.0) {
      isTransitioning = false;
    }
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setTextSize(1);
    int titleWidth = modeNames[mode].length() * 6;
    display.setCursor((SCREEN_WIDTH - titleWidth) / 2, SCREEN_HEIGHT / 2 - 8);
    display.print(modeNames[mode]);
    for (int i = 0; i < 5; i++) {
      if (random(100) < 30) {
        display.drawPixel(random(SCREEN_WIDTH), random(SCREEN_HEIGHT), SSD1306_WHITE);
      }
    }
  } else {
    Serial.print("Displaying mode: ");
    Serial.println(mode);
    switch (mode) {
      case 0: showTime(); break;
      case 1: showWeather(); break;
      case 2: showVirtualPet(); break;
      case 3: showTraffic(); break;
    }
    drawBattery();
    drawWiFiSignal();
  }

  updateParticles();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      display.drawPixel((int)particles[i].x, (int)particles[i].y, SSD1306_WHITE);
    }
  }

  display.display();
  delay(FRAME_DELAY);
}

// UI Functions
void drawTitleBar(const String& title) {
  int titleWidth = title.length() * 6;
  int centerX = (SCREEN_WIDTH - titleWidth) / 2;
  display.setTextSize(1 + (int)(breathingScale * 0.2));
  display.setCursor(centerX, 2 + bounceOffset);
  display.print(title);
  display.setTextSize(1);
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    int waveY = 12 + (int)(2 * sin((i * 0.1) + (millis() * 0.01)));
    display.drawPixel(i, waveY, SSD1306_WHITE);
  }
}

void drawBattery() {
  int batteryX = SCREEN_WIDTH - 20;
  int batteryY = 0;
  display.drawRect(batteryX, batteryY, 16, 8, SSD1306_WHITE);
  display.drawRect(batteryX + 16, batteryY + 2, 2, 4, SSD1306_WHITE);
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = adcValue * (3.3 / 4095) * 2.0;
  int percent = constrain(map(voltage * 100, 300, 420, 0, 100), 0, 100);
  int fillWidth = map(percent, 0, 100, 0, 14);
  for (int i = 0; i < fillWidth; i++) {
    int waveHeight = (int)(2 * sin((i * 0.5) + (millis() * 0.01)));
    display.drawFastVLine(batteryX + 1 + i, batteryY + 1, 6 + waveHeight, SSD1306_WHITE);
  }
}

void drawWiFiSignal() {
  int signalX = 2;
  int signalY = 0;
  int bars = map(abs(WiFi.RSSI()), 30, 90, 4, 0);
  bars = constrain(bars, 0, 4);
  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2;
    if (i < bars) {
      display.fillRect(signalX + i * 3, signalY + (8 - barHeight), 2, barHeight, SSD1306_WHITE);
    } else {
      display.drawRect(signalX + i * 3, signalY + (8 - barHeight), 2, barHeight, SSD1306_WHITE);
    }
  }
}

void drawClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  int centerX = SCREEN_WIDTH - 20;
  int centerY = 30;
  int radius = 10;
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  float hourAngle = (timeinfo.tm_hour % 12 + timeinfo.tm_min / 60.0) * PI / 6 - PI / 2;
  int hourX = centerX + (int)(6 * cos(hourAngle));
  int hourY = centerY + (int)(6 * sin(hourAngle));
  display.drawLine(centerX, centerY, hourX, hourY, SSD1306_WHITE);
  float minAngle = timeinfo.tm_min * PI / 30 - PI / 2;
  int minX = centerX + (int)(8 * cos(minAngle));
  int minY = centerY + (int)(8 * sin(minAngle));
  display.drawLine(centerX, centerY, minX, minY, SSD1306_WHITE);
}

void showTime() {
  drawTitleBar("⏰ Time");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    display.setCursor(10, 20);
    display.print("Time sync error!");
    return;
  }
  display.setTextSize(2);
  String timeStr = (timeinfo.tm_hour < 10 ? "0" : "") + String(timeinfo.tm_hour) +
                   (millis() % 1000 < 500 ? ":" : " ") +
                   (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min);
  int timeWidth = timeStr.length() * 12;
  display.setCursor((SCREEN_WIDTH - timeWidth) / 2, 20 + bounceOffset);
  display.print(timeStr);
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.printf("%s %d, %d",
                 (const char*[]){"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}[timeinfo.tm_mon],
                 timeinfo.tm_mday, timeinfo.tm_year + 1900);
  drawClock();
}

void showWeather() {
  drawTitleBar("🌤 Weather");
  if (WiFi.status() != WL_CONNECTED) {
    display.setCursor(10, 20);
    display.print("WiFi Disconnected");
    display.setCursor(10, 30);
    display.print("Last: " + weatherDesc); // Show cached data
    return;
  }
  display.setTextSize(2);
  int tempSize = (int)(2 + breathingScale * 0.3);
  display.setTextSize(tempSize);
  display.setCursor(10, 20);
  display.print((int)currentTemp);
  display.print("C");
  display.setTextSize(1);
  int y = 40 + (int)(2 * sin(millis() * 0.01));
  display.setCursor(10, y);
  display.print(weatherDesc);
  if (weatherDesc.indexOf("rain") >= 0 || weatherDesc.indexOf("Rain") >= 0) {
    for (int i = 0; i < 5; i++) {
      int x = random(SCREEN_WIDTH);
      int y = random(20, SCREEN_HEIGHT);
      display.drawLine(x, y, x, y + 3, SSD1306_WHITE);
    }
  } else if (weatherDesc.indexOf("cloud") >= 0 || weatherDesc.indexOf("Cloud") >= 0) {
    for (int i = 0; i < 2; i++) {
      int cloudX = (int)(i * 40 + 10 * sin(millis() * 0.001 + i));
      display.fillCircle(cloudX, 15 + i * 2, 4, SSD1306_WHITE);
      display.fillCircle(cloudX + 6, 15 + i * 2, 5, SSD1306_WHITE);
    }
  } else if (weatherDesc.indexOf("sun") >= 0 || weatherDesc.indexOf("Sun") >= 0 ||
             weatherDesc.indexOf("clear") >= 0 || weatherDesc.indexOf("Clear") >= 0) {
    int sunX = SCREEN_WIDTH - 15;
    int sunY = 15;
    display.fillCircle(sunX, sunY, 6, SSD1306_WHITE);
    for (int i = 0; i < 6; i++) {
      float angle = i * PI / 3 + millis() * 0.001;
      int rayX = sunX + (int)(10 * cos(angle));
      int rayY = sunY + (int)(10 * sin(angle));
      display.drawLine(sunX, sunY, rayX, rayY, SSD1306_WHITE);
    }
  }
}

void showVirtualPet() {
  drawTitleBar("🐾 Cat Pet");
  // Draw 16x16 pixelated cat sprite
  int petX = SCREEN_WIDTH - 21; // Adjusted for 16x16 sprite
  int petY = 18 + (int)(3 * sin(millis() * 0.005 * (petHappiness / 100.0)));
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (catSprite[y] & (1 << (15 - x))) {
        display.drawPixel(petX + x, petY + y, SSD1306_WHITE);
      }
    }
  }

  // Display metrics
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print("Hunger: ");
  display.print(petHunger);
  display.setCursor(10, 20);
  display.print("Happy: ");
  display.print(petHappiness);
  display.setCursor(10, 50);
  display.print("Tap: Feed, Long: Play");

  // Alert if needs attention
  if (petNeedsAttention) {
    display.fillRect(0, 48, SCREEN_WIDTH, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(10, 50);
    display.print("Meow! Needs TLC!");
    display.setTextColor(SSD1306_WHITE);
  }
}

void showTraffic() {
  Serial.println("ENTERING showTraffic() - Displaying Traffic Mode"); // Debug
  drawTitleBar("🚗 Traffic");
  if (WiFi.status() != WL_CONNECTED) {
    display.setCursor(10, 20);
    display.print("No WiFi");
    display.setCursor(10, 30);
    display.print("Last: " + trafficCondition); // Show cached data
  } else {
    // Display traffic condition with scrolling
    static int scrollX = SCREEN_WIDTH;
    scrollX--;
    if (scrollX < -(trafficCondition.length() * 6)) scrollX = SCREEN_WIDTH;
    display.setTextSize(1); // Ensure text size is set
    display.setCursor(scrollX, 20);
    display.print(trafficCondition);

    // Draw congestion meter
    int meterX = 10;
    int meterY = 40;
    display.drawRect(meterX, meterY, 20, 6, SSD1306_WHITE);
    int fillWidth;
    if (trafficColor == "GREEN") fillWidth = 7;
    else if (trafficColor == "YELLOW") fillWidth = 13;
    else fillWidth = 20;
    display.fillRect(meterX, meterY, fillWidth, 6, SSD1306_WHITE);
    display.setCursor(40, meterY);
    display.print(trafficColor);

    display.setCursor(10, 50);
    display.print("Long: Refresh");
  }
  Serial.println("Traffic Mode: " + trafficCondition);
}

void updateWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Weather update skipped: No WiFi");
    return;
  }
  HTTPClient http;
  http.begin(weatherURL);
  int httpCode = http.GET();
  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());
    currentTemp = doc["main"]["temp"];
    weatherDesc = String((const char*)doc["weather"][0]["main"]);
    humidity = doc["main"]["humidity"];
    Serial.println("Weather updated: " + weatherDesc);
  } else {
    Serial.println("Weather update failed: HTTP " + String(httpCode));
  }
  http.end();
}

void updateTraffic() {
  if (WiFi.status() != WL_CONNECTED) {
    trafficCondition = "No WiFi";
    trafficColor = "RED";
    Serial.println("Traffic update skipped: No WiFi");
    return;
  }
  HTTPClient http;
  http.begin(trafficURL);
  int httpCode = http.GET();
  if (httpCode == 200) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, http.getString());
    float currentSpeed = doc["flowSegmentData"]["currentSpeed"];
    float freeFlowSpeed = doc["flowSegmentData"]["freeFlowSpeed"];
    float ratio = currentSpeed / freeFlowSpeed;
    if (ratio > 0.8) {
      trafficCondition = city + ": Clear";
      trafficColor = "GREEN";
    } else if (ratio > 0.4) {
      trafficCondition = city + ": Moderate";
      trafficColor = "YELLOW";
    } else {
      trafficCondition = city + ": Heavy";
      trafficColor = "RED";
    }
    Serial.println("Updated real traffic: " + trafficCondition);
  } else {
    trafficCondition = "API Error";
    trafficColor = "RED";
    Serial.println("Traffic update failed: HTTP " + String(httpCode));
  }
  http.end();
}