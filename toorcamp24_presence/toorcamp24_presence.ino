#include <WS2801FX.h>

#define LED_COUNT 20
#define LED_DATA_PIN 2
#define LED_CLOCK_PIN 3

WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

void setup() {
  ws2801fx.init();
  ws2801fx.setBrightness(100);
  ws2801fx.setSpeed(200);
  ws2801fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2801fx.start();
}

void loop() {
  ws2801fx.service();
}