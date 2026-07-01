#define fetchDelay 1 //In Minutes
#define WIFI "ssid"
#define PASS "12345678"
#include <Arduino.h>

const char *coordinate[1][3]={{"Jakarta", "-6.20", "106.82"}};

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "ui/screens.h"
#include "ui/ui.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ui/images.h"
#include "ui/vars.h"
#include <string.h>

TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t WifiTaskHandle = NULL;
void TaskDisplay(void *pvParameters);
void TaskWifi(void *pvParameters);

#define XPT2046_CS  21
#define XPT2046_IRQ -1

#define TOUCH_X_MIN  200
#define TOUCH_X_MAX  3800
#define TOUCH_Y_MIN  200
#define TOUCH_Y_MAX  3800

char city[100] = { 0 };
const char *get_var_city() {
    return city;
}
void set_var_city(const char *value) {
    strncpy(city, value, sizeof(city) / sizeof(char));
    city[sizeof(city) / sizeof(char) - 1] = 0;
}


float wind_speed;
float get_var_wind_speed() {
    return wind_speed;
}
void set_var_wind_speed(float value) {
    wind_speed = value;
}

int32_t humidity;
int32_t get_var_humidity() {
    return humidity;
}
void set_var_humidity(int32_t value) {
    humidity = value;
}

float temp;
float get_var_temp() {
    return temp;
}
void set_var_temp(float value) {
    temp = value;
}

#include <string.h>

char condition[100] = { 0 };
const char *get_var_condition() {
    return condition;
}
void set_var_condition(const char *value) {
    strncpy(condition, value, sizeof(condition) / sizeof(char));
    condition[sizeof(condition) / sizeof(char) - 1] = 0;
}

bool network=true;
bool get_var_network() {
    return network;
}
void set_var_network(bool value) {
    network = value;
}

static const uint16_t SCREEN_W = 320;
static const uint16_t SCREEN_H = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t        *buf1 = NULL;
static lv_color_t        *buf2 = NULL;

static TFT_eSPI           tft;
static XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (touch.tirqTouched() && touch.touched()) {
        TS_Point p = touch.getPoint();

        int16_t x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_W - 1);
        int16_t y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_H - 1);

        data->point.x = constrain(x, 0, SCREEN_W - 1);
        data->point.y = constrain(y, 0, SCREEN_H - 1);
        data->state   = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}


void decodeWeather(int code) {
  switch (code) {
    case 0: case 1: case 2: case 3:
        set_var_condition("Clear");
        lv_img_set_src(objects.obj15, &img_clear);
        break; // Stops execution here
    case 45: case 48:
        set_var_condition("Cloudy");
        lv_img_set_src(objects.obj15, &img_cloudy);
        break;
    case 51: case 53: case 55:
        set_var_condition("Overcast");
        lv_img_set_src(objects.obj15, &img_clouded);
        break;
    case 61: case 63: case 65:  
        set_var_condition("Rain");
        lv_img_set_src(objects.obj15, &img_rain);
        break;
    default:
        set_var_condition("Unknown"); // It's better to show something
        break;
  }
}

void getWeatherData(String city, String lat, String lon) {
    set_var_city(city.c_str());
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.open-meteo.com/v1/forecast?latitude="+lat+"&longitude="+lon+"&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m");
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      
      // Parse JSON
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);
      
      float temp = doc["current"]["temperature_2m"];
      int humidity = doc["current"]["relative_humidity_2m"];
      int wmoCode = doc["current"]["weather_code"];
      float windSpeed = doc["current"]["wind_speed_10m"];
      Serial.println(wmoCode);

      set_var_temp(temp);
      set_var_humidity(humidity);
      set_var_wind_speed(windSpeed);
      decodeWeather(wmoCode);
    }else{
        set_var_network(true);
        Serial.printf("Fetch Weather Data Failed...");
    }
    http.end();
  }
}

void setup()
{
    Serial.begin(9600);
    tft.init();
    tft.setRotation(0);
    touch.begin();
    touch.setRotation(3);

    lv_init();

    uint32_t buffer_pixels = SCREEN_W * (SCREEN_H / 10);
    buf1 = (lv_color_t *)malloc(buffer_pixels * sizeof(lv_color_t));
    buf2 = (lv_color_t *)malloc(buffer_pixels * sizeof(lv_color_t));

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_pixels);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = SCREEN_W;
    disp_drv.ver_res  = SCREEN_H;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

  xTaskCreate(TaskWifi, "Wifi", 6144, NULL, 2, &WifiTaskHandle);
  xTaskCreate(TaskDisplay, "Display", 4096, NULL, 1, &DisplayTaskHandle);
}

void loop(){
    // Print the remaining memory every 5 seconds
  Serial.println("--- Stack High Water Marks ---");
  
  // Display Task remaining bytes
  UBaseType_t displayMem = uxTaskGetStackHighWaterMark(DisplayTaskHandle);
  Serial.printf("Display Task Free Memory: %d bytes\n", displayMem);

  // WiFi/JSON Task remaining bytes
  UBaseType_t wifiMem = uxTaskGetStackHighWaterMark(WifiTaskHandle);
  Serial.printf("WiFi Task Free Memory: %d bytes\n", wifiMem);
  
  Serial.println("------------------------------");
  
  delay(5000);
}

void TaskDisplay(void *pvParameters) {
ui_init();
  for (;;) { 
    static uint32_t last_tick = 0;
    uint32_t current_tick = millis();
    uint32_t delta = current_tick - last_tick;
    last_tick = current_tick;

    lv_tick_inc(delta); 
    
    lv_task_handler();  
    ui_tick();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void TaskWifi(void *pvParameters) {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI, PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  for (;;) { 
    if (WiFi.status() == WL_CONNECTED) {
        getWeatherData(coordinate[0][0],coordinate[0][1],coordinate[0][2]);
        set_var_network(false);
    } else {
        Serial.println("[fetch] WiFi disconnected — reconnecting...");
        set_var_network(true);
        WiFi.reconnect();
    }
    vTaskDelay(fetchDelay*60 / portTICK_PERIOD_MS);
  }
}