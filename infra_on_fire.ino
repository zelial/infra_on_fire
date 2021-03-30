#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>
// https://github.com/zelial/arduino-library-utils
#include <utils.h>

#include "config.h"

// Setup basic 128x32 OLED
// SCK -> D1
// SDA -> D2
Adafruit_SSD1306 display(128, 32, &Wire, 2);

// Red LED PIN
#define RED_PIN D5
//Green LED PIN
#define GREEN_PIN D6

ESP8266WebServer server(80);

// for how long will text be displayed (in millis)
long TEXT_TIMEOUT = 10000;
// time in millis when to clear display
// after displaying text
long DISPLAY_TIMEOUT = 0;

// whether we should display welcome header
// will be shown until 1st client connects
boolean DISPLAY_HEADER = true;


// for how long will LEDs blink (in millis)
long BLINK_TIMEOUT = 10000;
// time in millis when to stop blinking
long RED_BLINK_TIMEOUT = 0;
long GREEN_BLINK_TIMEOUT = 0;
// save state of LED so we can return to it after blinking
boolean RED_ON = false;
boolean GREEN_ON = false;


// timestamp of last client request to indicate we have fresh data
long LAST_REQUEST = 0;
// how long since last request we consider current state valid
// after that we show header again
// 5 minutes in millis
long DATA_FRESHNESS = 5*60*1000;

void setup() {
  // LED controlling pins
  // red
  pinMode(RED_PIN, OUTPUT);
  // green
  pinMode(GREEN_PIN, OUTPUT);
  
  init_serial();
  logln("ima alive");

  wifi_reconnect(ssid, wifi_password, ip_last_byte);

  server.on("/text", handle_text);
  server.on("/red", handle_red);
  server.on("/green", handle_green);

  server.begin();

 // init display
 if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    logln("SSD1306 allocation failed");
    for(;;);
  }else {
    display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
  }
  // upside down (pins on right)
  display.setRotation(2);
}

void loop() {

  // no client has connected for DATA_FRESHNESS millis
  if (millis() > (LAST_REQUEST + DATA_FRESHNESS)){
      DISPLAY_HEADER = true;
  }
  if (DISPLAY_HEADER){
      display_header();
  }

  server.handleClient();

  // stop showing text after timeout
  if (DISPLAY_TIMEOUT > 0 && millis() > DISPLAY_TIMEOUT){
      DISPLAY_TIMEOUT = 0;
      display.clearDisplay();
      display.display();
  }

  // handle red led
  if (RED_BLINK_TIMEOUT > 0 && millis() < RED_BLINK_TIMEOUT){
     if ((millis() / 1000) % 2 == 0){
          digitalWrite(RED_PIN, HIGH);
      }else{
          digitalWrite(RED_PIN, LOW);
      }
  }else{
    // set state of LED
    if (RED_ON){
        digitalWrite(RED_PIN, HIGH);
    }else{
        digitalWrite(RED_PIN, LOW);
    }
  }
  
  // handle green led
  if (GREEN_BLINK_TIMEOUT > 0 && millis() < GREEN_BLINK_TIMEOUT){
     if ((millis() / 1000) % 2 == 0){
          digitalWrite(GREEN_PIN, HIGH);
      }else{
          digitalWrite(GREEN_PIN, LOW);
      }
  }else{
    // set state of LED
    if (GREEN_ON){
        digitalWrite(GREEN_PIN, HIGH);
    }else{
        digitalWrite(GREEN_PIN, LOW);
    }
  }

  delay(5);
}

void display_header(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("InfraStatus v" + String(version));
  display.println(WiFi.localIP());
  display.println(millis() / 1000);
  display.display();
  RED_ON=true;
  GREEN_ON=true;
}

void handle_not_found(){
  server.send(404, "text/plain", "404: Not found");
}

// display text on display
void handle_text() {
  server.send(200, "text/plain", "text accepted\n");
  display.setTextSize(2);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(server.arg(0));
  display.display();
  DISPLAY_TIMEOUT = millis() + TEXT_TIMEOUT;
  DISPLAY_HEADER = false;
  LAST_REQUEST = millis();
}


// turn red LED on/off
void handle_red() {
  server.send(200, "text/plain", "red LED state accepted\n");
  if (server.arg(0) == "on"){
      RED_ON = true;
      logln("RED LED on");
  }
  if (server.arg(0) == "off"){
      RED_ON = false;
      logln("RED LED off");
  }
  if (server.arg(0) == "blink"){
      logln("RED LED blinking");
      RED_BLINK_TIMEOUT = millis() + BLINK_TIMEOUT;
  }
  DISPLAY_HEADER = false;
  // clear the welcome message on first client request
  if (LAST_REQUEST == 0){
      display.clearDisplay();
      display.display();
  }
  LAST_REQUEST = millis();
}


// turn red LED on/off
void handle_green() {
  server.send(200, "text/plain", "green LED state accepted\n");
  if (server.arg(0) == "on"){
      GREEN_ON = true;
      logln("GREEN LED on");
  }
  if (server.arg(0) == "off"){
      GREEN_ON = false;
      logln("GREEN LED on");
  }
  if (server.arg(0) == "blink"){
      logln("GREEN LED blinking");
      GREEN_BLINK_TIMEOUT = millis() + BLINK_TIMEOUT;
  }
  DISPLAY_HEADER = false;
  // clear the welcome message on first client request
  if (LAST_REQUEST == 0){
      display.clearDisplay();
      display.display();
  }
  LAST_REQUEST = millis();
}
