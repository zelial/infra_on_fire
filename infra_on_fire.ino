#include <utils.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>
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

void setup() {
  // LED controlling pins
  // red
  pinMode(RED_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);
  // green
  pinMode(GREEN_PIN, OUTPUT);
  digitalWrite(GREEN_PIN, LOW);
  
  init_serial();
  logln("ima alive");

  wifi_reconnect(ssid, wifi_password, ip_last_byte);

  server.on("/text", handle_text);
  server.on("/red", handle_red);
  server.on("/green", handle_green);

  /*server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.onNotFound(handle_not_found);
  */
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
  display.setCursor(0,0);
  display.println("InfraStatus v" + String(version));
  display.println(WiFi.localIP());
  display.println(millis() / 1000);
  display.display();
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
}
