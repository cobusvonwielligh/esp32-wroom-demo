#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "weather_icons.h"

/* --- Configuration --- */
#define WIFI_SSID "CobusIphone"
#define WIFI_PASS "cobus123"

#define PLACE_NAME "Rondebosch"
#define PLACE_LAT "-33.3689"
#define PLACE_LNG "19.3117"

const String WEATHER_URL =
  String("https://api.open-meteo.com/v1/forecast?latitude=") + PLACE_LAT + "&longitude=" + PLACE_LNG + "&current_weather=true&daily=temperature_2m_min,temperature_2m_max&timezone=auto";


#define REFRESH_MS 30000UL  // Refresh every ms

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int fetchWeatcherFailCount = 0;

/* --- Prototypes --- */
bool fetchWeather(float &tempC, uint8_t &code);
void drawWeatherScreen(float tempC, bool isRain);

void setup() {
  Serial.begin(115200);
  delay(100);

  Wire.begin(5, 4);  // LOLIN32: SDA=5, SCL=4

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false)) {
    Serial.println(F("SSD1306 init failed"));
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.setSleep(false);  // prevent ESP32 from sleeping during connect
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int retries = 0;
  const int maxRetries = 25;

  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("WiFi connect FAILED");
    display.println("SSID:");
    display.print(WIFI_SSID);
    display.display();

    while (true) delay(1000);  // halt
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("WiFi connected.");
  display.println("Getting weather...");
  display.display();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static float tempC = 0, tempMin = 0, tempMax = 0;
  static uint8_t code = 0;
  static bool isRain = false;

  unsigned long now = millis();
  float progress = float(now - lastUpdate) / REFRESH_MS;
  if (progress > 1.0) progress = 1.0;

  if (now - lastUpdate > REFRESH_MS || lastUpdate == 0) {
    if (fetchWeather(tempC, code, tempMin, tempMax)) {
      isRain = (code >= 51 && code <= 67) || (code >= 80 && code <= 82);
      lastUpdate = now;
      fetchWeatcherFailCount = 0;
    } else {
      fetchWeatcherFailCount++;
      if (fetchWeatcherFailCount > 3) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("Failed to fetch weather data.");
        display.println("fetchWeather");
        display.display();

        delay(100000); // halt
      }
    }
  }

  drawWeatherScreen(tempC, tempMin, tempMax, isRain, progress);
  delay(50);  // smoother animation
}


/* --- Fetch current weather from Open-Meteo API --- */
bool fetchWeather(float &tempC, uint8_t &code, float &tempMin, float &tempMax) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return false;
  }

  HTTPClient http;
  http.begin(WEATHER_URL);
  http.useHTTP10();  // force HTTP/1.0 to avoid chunked encoding issues

  int status = http.GET();
  Serial.printf("HTTP status: %d\n", status);

  if (status != HTTP_CODE_OK) {
    Serial.println("HTTP GET failed");
    http.end();
    return false;
  }

  DynamicJsonDocument doc(1024);  // bump for safety
  DeserializationError err = deserializeJson(doc, http.getStream());

  if (err) {
    Serial.print("JSON parse error: ");
    Serial.println(err.c_str());
    http.end();
    return false;
  }

  tempC = doc["current_weather"]["temperature"] | NAN;
  code = doc["current_weather"]["weathercode"] | 0;
  tempMin = doc["daily"]["temperature_2m_min"][0] | NAN;
  tempMax = doc["daily"]["temperature_2m_max"][0] | NAN;

  http.end();
  Serial.printf("Parsed: %.1f C, code %d\n", tempC, code);
  return !isnan(tempC);
}


void drawWeatherScreen(float tempC, float tempMin, float tempMax, bool isRain, float progress) {
  display.clearDisplay();

  const int margin = 4;

  display.setTextSize(1);
  display.setCursor(margin, margin);
  display.setTextColor(SSD1306_WHITE);
  display.print("Weather");

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(PLACE_NAME, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(SCREEN_WIDTH - w - margin, margin);
  display.print(PLACE_NAME);

  display.setTextSize(2);
  display.setCursor(margin, 15 + margin);
  display.printf("%.1f", tempC);
  display.setTextSize(1);
  display.print((char)247);
  display.setTextSize(2);
  display.print("C");

  display.setTextSize(1);
  display.setCursor(margin, 38 + margin);
  display.printf("Min: %.0f%cC", tempMin, (char)247);
  display.setCursor(margin, 50 + margin);
  display.printf("Max: %.0f%cC", tempMax, (char)247);

  display.drawBitmap(
    SCREEN_WIDTH - 50 - margin, 8 + margin,
    isRain ? iconRainBitmap : iconSunBitmap,
    50, 50, SSD1306_WHITE, SSD1306_BLACK);

  drawProgressBorder(progress);
  display.display();
}

void drawProgressBorder(float progress) {
  const int border = 2;
  const int x0 = border - 2;
  const int y0 = border - 2;
  const int w = SCREEN_WIDTH - 2 * x0;
  const int h = SCREEN_HEIGHT - 2 * y0;
  const int perimeter = 2 * (w + h - 2 * border);

  int progressLen = int(progress * perimeter);

  // Always draw static white border once
  for (int t = 0; t < border; t++) {
    display.drawRect(x0 - t, y0 - t, w + 2 * t, h + 2 * t, SSD1306_WHITE);
  }

  // Draw the black segment
  int x = x0, y = y0;
  for (int i = 0; i < progressLen; i++) {
    if (x < x0 + w - 1 && y == y0) x++;
    else if (x == x0 + w - 1 && y < y0 + h - 1) y++;
    else if (y == y0 + h - 1 && x > x0) x--;
    else if (x == x0 && y > y0) y--;
    display.drawPixel(x, y, SSD1306_BLACK);
  }
}
