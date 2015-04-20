
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 1024

#define SSID  "****"      // change this to match your WiFi SSID
#define PASS  "****"  // change this to match your WiFi password
#define PORT  "9001"      // using port 8080 by default
#define PIN_LIGHT 0
#define PIN_REED 2
#define PIN_LED 3

char *ip = "****";
//char *ip = "192.168.1.8";
char *port = PORT;

char buffer[BUFFER_SIZE];

void setup() {
  
  pinMode(PIN_REED, INPUT);
  pinMode(PIN_LIGHT, OUTPUT);
  
  Serial.begin(115200);
  //Serial.begin(9600);
  
  delay(2000);

  Serial.println(F("AT"));
  //readAll();
  delay(1000);

  Serial.println(F("AT+RST\r"));
  //readAll();
  delay(3000);
  
  Serial.println(F("AT+CWMODE=1"));
  //readAll();
  delay(2000);
  
  Serial.println(F("AT+CWJAP=\"" SSID "\", \"" PASS "\""));
  //readAll();
  delay(3000);
  
  Serial.println(F("AT+CIPMUX=0"));
  delay(1000);
  
}

char data[1024];

void loop() {
  
  int valueLight = analogRead(PIN_LIGHT);
  int valueReed = digitalRead(PIN_REED);
  
  if (valueReed == 1) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
  }
  
  sprintf(data, "{\"type\":\"event\", \"place\": \"home\", \"light\":\"%d\", \"reed\":\"%d\"}", 
   valueLight, valueReed);
  
  Serial.print(F("AT+CIPSTART="));
  Serial.print(F("\"UDP\",\""));
  Serial.print(ip);
  Serial.print("\",");
  Serial.println(PORT);
  delay(2000);  
 
  Serial.print(F("AT+CIPSEND="));
  Serial.println(strlen(data)); 
  delay(1000);
  
  Serial.println(data);
  delay(1000);
  
  Serial.println("AT+CIPCLOSE");
  
  delay(56 * 1000);  
  
}
