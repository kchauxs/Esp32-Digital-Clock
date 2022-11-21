#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "time.h"
#include <WiFiManager.h>
#include "OneButton.h"
#include <Preferences.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW // Uncomment according to your hardware type
#define MAX_DEVICES 4                     // Defining size, and output pins
#define CS_PIN 5
#define BUTTON_PIN A17
#define INTENSITY 10
#define INTERVAL 5000
#define TIMEOUT_AP 120

const char *ntpServer = "pool.ntp.org";
const char *TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";

String apName = "That Watch 😱🔥";
String apPassword = "what_watch!";

bool isNTPConnected;
unsigned int intensity;

WiFiManager wm;
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
OneButton button(BUTTON_PIN, false);
Preferences preferences;

void showTime()
{
  static unsigned long currentMillis;
  if (millis() - currentMillis > INTERVAL)
  {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      String hour;
      hour += String(timeinfo.tm_hour);
      hour += ":";
      hour += String(timeinfo.tm_min);

      Display.setTextAlignment(PA_CENTER);
      Display.displayClear();
      Display.print(hour.c_str());

      if (!isNTPConnected)
      {
        isNTPConnected = true;
        wm.disconnect();
        wm.~WiFiManager();
        WiFi.mode(WIFI_OFF);
      }
    }
    currentMillis = millis();
  }
}

void initBlockingPortal()
{
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("[Portal] Init Captive Portal...");
  Display.displayClear();
  Display.setTextAlignment(PA_CENTER);
  Display.print("AP");
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFiManager _wm;
  delay(1000);

  _wm.setConfigPortalTimeout(TIMEOUT_AP);

  if (_wm.startConfigPortal(apName.c_str(), apPassword.c_str()))
    Display.print("DONE!");
  else
    Display.print("FAILD");

  delay(1000);
  Serial.println("[Portal] Restarting...");
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
  ESP.restart();
  delay(5000);
}

void setIntensity()
{
  intensity = (intensity + 1) % 16;
  Display.setIntensity(intensity);
  Display.print(intensity);
  preferences.putInt("intensity", intensity);
  delay(50);
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  button.attachClick(setIntensity);
  button.attachLongPressStart(initBlockingPortal);

  preferences.begin("watch", false);
  intensity = preferences.getInt("intensity", INTENSITY);
  delay(100);

  Display.begin();
  Display.setIntensity(intensity);
  Display.displayClear();
  Display.setTextAlignment(PA_CENTER);

  Display.print("THAT");
  delay(1000);
  Display.print("WATCH!");
  delay(2000);
  Display.print(":v");
  delay(500);

  static String dots;
  for (size_t i = 0; i < 10; i++)
  {
    dots += ".";
    Display.print(dots);
    delay(random(100, 500));
  }
  Display.print("AP");
  digitalWrite(LED_BUILTIN, HIGH);
  if (!wm.autoConnect(apName.c_str(), apPassword.c_str()))
  {
    Display.print("FAILD");
    delay(5000);
    ESP.restart();
  }
  digitalWrite(LED_BUILTIN, LOW);

  Display.print("DONE!");
  delay(1000);
  Serial.println(WiFi.localIP());

  isNTPConnected = false;
  configTzTime(TZ_INFO,
               "time.nist.gov",
               "0.pool.ntp.org",
               "1.pool.ntp.org");
  delay(1000);
}

void loop()
{
  showTime();
  button.tick();
}
