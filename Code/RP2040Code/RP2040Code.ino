#include <Arduino.h>
#include "hardware/spi.h"

#define PIN_SCK 29
#define PIN_MOSI 27
#define PIN_MISO 28
#define PIN_CS 26

const int MOTOR_IN1 = 24;
const int MOTOR_IN2 = 25;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  spi_init(spi0, 1000000);
  spi_set_slave(spi0, true);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
  Serial.println("rp2040 working! :)");

}

void loop() {r
  // put your main code here, to run repeatedly:
  if(spi_is_readable(spi0)) {
    uint8_t receivedCommand;
    spi_read_blocking(spi0, 0, &receivedCommand, 1);
    if(receivedCommand == 0x01) {
      Serial.println("received motor go!");
      analogWrite(MOTOR_IN1, 200);
      digitalWrite(MOTOR_IN2, LOW);
    }
    else if (receivedCommand == 0x00) {
      Serial.println("received stop!");
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
    }
  }
}
