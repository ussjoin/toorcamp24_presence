#include <WS2801FX.h>

#define LED_COUNT 20
#define LED_DATA_PIN 2
#define LED_CLOCK_PIN 3

WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

void setup() {
  ws2801fx.init();
  ws2801fx.setBrightness(100);
  ws2801fx.setSpeed(50);
  ws2801fx.setMode(FX_MODE_RAINBOW);
  ws2801fx.turn_off_pixel(10);
  ws2801fx.turn_off_pixel(11);
  ws2801fx.turn_off_pixel(12);
  ws2801fx.turn_on_all_pixels();
  ws2801fx.turn_off_pixel(2);
  ws2801fx.turn_off_pixel(1);
  ws2801fx.start();
}

void loop() {
  ws2801fx.service();
}