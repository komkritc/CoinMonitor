#include <Arduino.h>

// shorturl.at/ewCV4
// https://github.com/GRHMLGGT/ESP32-Crypto-Currency-Ticker/blob/master/Coin_Gecko.ino

#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "Button2.h"
#include "esp_adc_cal.h"
#include <IotWebConf.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>

const char thingName[] = "coinThing";
const char wifiInitialApPassword[] = "12345678";

void handleRoot();

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

#define ADC_EN 14 //ADC_EN is the ADC detection enable port
#define ADC_PIN 34
#define BUTTON_1 35
#define BUTTON_2 0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

char buff[512];
int vref = 1100;
int btnCick = false;

BlynkTimer timer;

void myTimerEvent()
{

  if ((WiFi.status() == WL_CONNECTED))

  {
    HTTPClient http;

    //https://api.coingecko.com/api/v3/simple/price?ids=bitcoin%2Cethereum&vs_currencies=usd&include_24hr_change=true
    http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin%2Cethereum&vs_currencies=usd&include_24hr_change=true");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
    }

    Serial.println("\nStatuscode: " + String(httpCode));
    delay(100);

    const size_t capacity = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(6) + 150;
    DynamicJsonDocument doc(capacity);
    String payload = http.getString();
    const char *json = payload.c_str();

    deserializeJson(doc, json);

    JsonObject bitcoin = doc["bitcoin"];
    //    float bitcoin_eur = bitcoin["eur"]; // 9473.3
    //    float bitcoin_eur_24h_change = bitcoin["eur_24h_change"]; // 11.379516678954898
    //    float bitcoin_gbp = bitcoin["gbp"]; // 8642.89
    //    float bitcoin_gbp_24h_change = bitcoin["gbp_24h_change"]; // 11.58637677393075
    float bitcoin_usd = bitcoin["usd"];                       // 11140.6
    float bitcoin_usd_24h_change = bitcoin["usd_24h_change"]; // 12.464050392648252

    JsonObject ethereum = doc["ethereum"];
    //    float ethereum_eur = ethereum["eur"]; // 276.02
    //    float ethereum_eur_24h_change = ethereum["eur_24h_change"]; // 3.5689620753981264
    //    float ethereum_gbp = ethereum["gbp"]; // 251.82
    //    float ethereum_gbp_24h_change = ethereum["gbp_24h_change"]; // 3.7613159836416026
    float ethereum_usd = ethereum["usd"];                       // 324.6
    float ethereum_usd_24h_change = ethereum["usd_24h_change"]; // 4.577442219792744

    Serial.println("-------------------------------");

    //    Serial.print("EUR: ");
    //    Serial.println(bitcoin_eur);
    //    Serial.print("EUR 24hr %: ");
    //    Serial.println(bitcoin_eur_24h_change);
    //
    //    Serial.print("GBP: ");
    //    Serial.println(bitcoin_gbp);
    //    Serial.print("GBP 24hr %: ");
    //    Serial.println(bitcoin_gbp_24h_change);

    Serial.print("USD: ");
    Serial.println(bitcoin_usd);
    Serial.print("USD 24hr %: ");
    Serial.println(ethereum_usd_24h_change);

    //    Serial.print("EUR: ");
    //    Serial.println(ethereum_eur);
    //    Serial.print("EUR 24hr %: ");
    //    Serial.println(ethereum_eur_24h_change);
    //
    //    Serial.print("GBP: ");
    //    Serial.println(ethereum_gbp);
    //    Serial.print("GBP 24hr %: ");
    //    Serial.println(ethereum_gbp_24h_change);

    Serial.print("USD: ");
    Serial.println(ethereum_usd);
    Serial.print("USD 24hr %: ");
    Serial.println(ethereum_usd_24h_change);

    Serial.println("-------------------------------");

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(5);

    tft.drawString(String(ethereum_usd, 0), tft.width() / 2, tft.height() / 2);
    tft.setTextDatum(TL_DATUM);

    tft.setTextSize(3);
    tft.drawString(String(ethereum_usd_24h_change), 80, tft.height() / 2 + 38);
    tft.setTextDatum(TL_DATUM);
  }
}

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void button_init()
{
  btn1.setLongClickHandler([](Button2 &b)
                           {
                             btnCick = false;
                             int r = digitalRead(TFT_BL);
                             tft.fillScreen(TFT_BLACK);
                             tft.setTextColor(TFT_GREEN, TFT_BLACK);
                             tft.setTextDatum(MC_DATUM);
                             tft.drawString("Press again to wake up", tft.width() / 2, tft.height() / 2);
                             espDelay(6000);
                             digitalWrite(TFT_BL, !r);

                             tft.writecommand(TFT_DISPOFF);
                             tft.writecommand(TFT_SLPIN);
                             //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
                             esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
                             // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
                             esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
                             delay(200);
                             esp_deep_sleep_start();
                           });
  btn1.setPressedHandler([](Button2 &b)
                         {
                           Serial.println("Detect Voltage..");
                           btnCick = true;
                         });

  btn2.setPressedHandler([](Button2 &b)
                         {
                           btnCick = false;
                           Serial.println("btn press wifi scan");
                         });
}

void button_loop()
{
  btn1.loop();
  btn2.loop();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.init();
  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []
            { iotWebConf.handleConfig(); });
  server.onNotFound([]()
                    { iotWebConf.handleNotFound(); });

  Serial.println("Ready.");

  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  tft.setRotation(1);
  tft.setTextSize(2);
  tft.drawString("Waiting Internet...", tft.width() / 2, tft.height() / 2);
  tft.setTextDatum(TL_DATUM);

  //tft.fillScreen(TFT_GREEN);
  espDelay(1000);

  button_init();

  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  }
  else
  {
    Serial.println("Default Vref: 1100mV");
  }

  timer.setInterval(30000L, myTimerEvent);
}

void loop()
{
  iotWebConf.doLoop();
  timer.run(); // Initiates BlynkTimer
}

/**
   Handle web requests to "/" path.
*/
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}
