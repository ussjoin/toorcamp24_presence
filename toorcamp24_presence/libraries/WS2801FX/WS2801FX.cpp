/* 2024
Edited by github.com/ussjoin to add the ability to "notch out" individual
LEDs on a string.
License remains the same.
*/

/*
  WS2801FX.cpp - Library for WS2801 LED effects.

  Harm Aldick - 2016
  www.aldick.org

  WS2801 fork:
  Lennart Buhl - 2017

  FEATURES
    * A lot of blinken modes and counting
    * WS2801FX can be used as drop-in replacement for Adafruit WS2801 Library

  NOTES
    * Uses the Adafruit WS2801 Library. Get it here:
      https://github.com/adafruit/Adafruit-WS2801-Library



  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG

  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-05   port to WS2801 Library

*/

#include "Arduino.h"
#include "WS2801FX.h"

#define CALL_MODE(n) (this->*_mode[n])();

static uint8_t blocked_array [] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

void WS2801FX::init() {
  Adafruit_WS2801::begin();
  setBrightness(_brightness);
  Adafruit_WS2801::show();
}

void WS2801FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis();

    if(now - _mode_last_call_time > _mode_delay || _triggered) {
      CALL_MODE(_mode_index);
      _counter_mode_call++;
      _mode_last_call_time = now;
      _triggered = false;
    }
  }
}

void WS2801FX::start() {
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _running = true;
}

void WS2801FX::stop() {
  _running = false;
  strip_off();
}

void WS2801FX::trigger() {
  _triggered = true;
}

void WS2801FX::setMode(uint8_t m) {
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _mode_index = constrain(m, 0, MODE_COUNT-1);
  _mode_color = _color;
  setBrightness(_brightness);
  //strip_off();
}

void WS2801FX::setSpeed(uint8_t s) {
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _speed = constrain(s, SPEED_MIN, SPEED_MAX);
  //strip_off();
}

void WS2801FX::increaseSpeed(uint8_t s) {
  s = constrain(_speed + s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2801FX::decreaseSpeed(uint8_t s) {
  s = constrain(_speed - s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2801FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2801FX::setColor(uint32_t c) {
  _color = c;
  _counter_mode_call = 0;
  _counter_mode_step = 0;
  _mode_last_call_time = 0;
  _mode_color = _color;
  setBrightness(_brightness);
  //strip_off();
}

void WS2801FX::setBrightness(uint8_t b) {
  _brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  wrapNeoPixelSetBrightness(_brightness);
  Adafruit_WS2801::show();
}

void WS2801FX::increaseBrightness(uint8_t s) {
  s = constrain(_brightness + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2801FX::decreaseBrightness(uint8_t s) {
  s = constrain(_brightness - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

boolean WS2801FX::isRunning() {
  return _running;
}

uint8_t WS2801FX::getMode(void) {
  return _mode_index;
}

uint8_t WS2801FX::getSpeed(void) {
  return _speed;
}

uint8_t WS2801FX::getBrightness(void) {
  return _brightness;
}

uint8_t WS2801FX::getModeCount(void) {
  return MODE_COUNT;
}

uint32_t WS2801FX::getColor(void) {
  return _color;
}

const char* WS2801FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _name[m];
  } else {
    return "";
  }
}

/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */

/*
 * Turns everything off. Doh.
 */
void WS2801FX::strip_off() {
  WS2801FX::clear();
  Adafruit_WS2801::show();
}


/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2801FX::color_wheel(uint8_t pos) {
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}


/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2801FX::get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t d = 0;

  while(d < 42) {
    r = random(256);
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }

  return r;
}


/*
 * No blinking. Just plain old static light.
 */
void WS2801FX::mode_static(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }
  Adafruit_WS2801::show();

  _mode_delay = 50;
}


/*
 * Normal blinking. 50% on/off time.
 */
void WS2801FX::mode_blink(void) {
  if(_counter_mode_call % 2 == 1) {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, _color);
    }
    Adafruit_WS2801::show();
  } else {
    strip_off();
  }

  _mode_delay = 100 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights all LEDs after each other up. Then turns them in
 * that order off. Repeat.
 */
void WS2801FX::mode_color_wipe(void) {
  if(_counter_mode_step < _led_count) {
    WS2801FX::setPixelColor(_counter_mode_step, _color);
  } else {
    WS2801FX::setPixelColor(_counter_mode_step - _led_count, 0);
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % (_led_count * 2);

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
void WS2801FX::mode_color_wipe_random(void) {
  if(_counter_mode_step == 0) {
    _mode_color = get_random_wheel_index(_mode_color);
  }

  WS2801FX::setPixelColor(_counter_mode_step, color_wheel(_mode_color));
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;

  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
void WS2801FX::mode_random_color(void) {
  _mode_color = get_random_wheel_index(_mode_color);

  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color_wheel(_mode_color));
  }

  Adafruit_WS2801::show();
  _mode_delay = 100 + ((5000 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 */
void WS2801FX::mode_single_dynamic(void) {
  if(_counter_mode_call == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, color_wheel(random(256)));
    }
  }

  WS2801FX::setPixelColor(random(_led_count), color_wheel(random(256)));
  Adafruit_WS2801::show();
  _mode_delay = 10 + ((5000 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
void WS2801FX::mode_multi_dynamic(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color_wheel(random(256)));
  }
  Adafruit_WS2801::show();
  _mode_delay = 100 + ((5000 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
void WS2801FX::mode_breath(void) {
  //                                      0    1    2   3   4   5   6    7   8   9  10  11   12   13   14   15   16    // step
  uint16_t breath_delay_steps[] =     {   7,   9,  13, 15, 16, 17, 18, 930, 19, 18, 15, 13,   9,   7,   4,   5,  10 }; // magic numbers for breathing LED
  uint8_t breath_brightness_steps[] = { 150, 125, 100, 75, 50, 25, 16,  15, 16, 25, 50, 75, 100, 125, 150, 220, 255 }; // even more magic numbers!

  if(_counter_mode_call == 0) {
    _mode_color = breath_brightness_steps[0] + 1;
  }

  uint8_t breath_brightness = _mode_color; // we use _mode_color to store the brightness

  if(_counter_mode_step < 8) {
    breath_brightness--;
  } else {
    breath_brightness++;
  }

  // update index of current delay when target brightness is reached, start over after the last step
  if(breath_brightness == breath_brightness_steps[_counter_mode_step]) {
    _counter_mode_step = (_counter_mode_step + 1) % (sizeof(breath_brightness_steps)/sizeof(uint8_t));
  }

  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);           // set all LEDs to selected color
  }
  WS2801FX::setBrightness(breath_brightness);                     // set new brightness to leds
  Adafruit_WS2801::show();

  _mode_color = breath_brightness;                         // we use _mode_color to store the brightness
  _mode_delay = breath_delay_steps[_counter_mode_step];
}


/*
 * Fades the LEDs on and (almost) off again.
 */
void WS2801FX::mode_fade(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  int b = _counter_mode_step - 127;
  b = 255 - (abs(b) * 2);
  b = map(b, 0, 255, min((uint8_t) 25, _brightness), _brightness);
  WS2801FX::setBrightness(b);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;
  _mode_delay = 5 + ((15 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Runs a single pixel back and forth.
 */
void WS2801FX::mode_scan(void) {
  if(_counter_mode_step > (_led_count*2) - 2) {
    _counter_mode_step = 0;
  }
  _counter_mode_step++;

  int i = _counter_mode_step - (_led_count - 1);
  i = abs(i);

  WS2801FX::clear();
  WS2801FX::setPixelColor(abs(i), _color);
  Adafruit_WS2801::show();

  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
void WS2801FX::mode_dual_scan(void) {
  if(_counter_mode_step > (_led_count*2) - 2) {
    _counter_mode_step = 0;
  }
  _counter_mode_step++;

  int i = _counter_mode_step - (_led_count - 1);
  i = abs(i);

  WS2801FX::clear();
  WS2801FX::setPixelColor(i, _color);
  WS2801FX::setPixelColor(_led_count - (i+1), _color);
  Adafruit_WS2801::show();

  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
void WS2801FX::mode_rainbow(void) {
  uint32_t color = color_wheel(_counter_mode_step);
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color);
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;

  _mode_delay = 1 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
void WS2801FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color_wheel(((i * 256 / _led_count) + _counter_mode_step) % 256));
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 256;

  _mode_delay = 1 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
void WS2801FX::mode_theater_chase(void) {
  uint8_t j = _counter_mode_call % 6;
  if(j % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      WS2801FX::setPixelColor(i+(j/2), _color);
    }
    Adafruit_WS2801::show();
    _mode_delay = 50 + ((500 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  } else {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      WS2801FX::setPixelColor(i+(j/2), 0);
    }
    _mode_delay = 1;
  }
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
void WS2801FX::mode_theater_chase_rainbow(void) {
  uint8_t j = _counter_mode_call % 6;
  if(j % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      WS2801FX::setPixelColor(i+(j/2), color_wheel((i+_counter_mode_step) % 256));
    }
    Adafruit_WS2801::show();
    _mode_delay = 50 + ((500 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  } else {
    for(uint16_t i=0; i < _led_count; i=i+3) {
      WS2801FX::setPixelColor(i+(j/2), 0);
    }
    _mode_delay = 1;
  }
  _counter_mode_step = (_counter_mode_step + 1) % 256;
}


/*
 * Running lights effect with smooth sine transition.
 */
void WS2801FX::mode_running_lights(void) {
  uint8_t r = ((_color >> 16) & 0xFF);
  uint8_t g = ((_color >> 8) & 0xFF);
  uint8_t b = (_color & 0xFF);

  for(uint16_t i=0; i < _led_count; i++) {
    int s = (sin(i+_counter_mode_call) * 127) + 128;
     WS2801FX::setPixelColor(i, (((uint32_t)(r * s)) / 255), (((uint32_t)(g * s)) / 255), (((uint32_t)(b * s)) / 255));
  }

  Adafruit_WS2801::show();

  _mode_delay = 35 + ((350 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2801FX::mode_twinkle(void) {
  if(_counter_mode_step == 0) {
    strip_off();
    uint16_t min_leds = max(1, _led_count/5); // make sure, at least one LED is on
    uint16_t max_leds = max(1, _led_count/2); // make sure, at least one LED is on
    _counter_mode_step = random(min_leds, max_leds);
  }

  WS2801FX::setPixelColor(random(_led_count), _mode_color);
  Adafruit_WS2801::show();

  _counter_mode_step--;
  _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2801FX::mode_twinkle_random(void) {
  _mode_color = color_wheel(random(256));
  mode_twinkle();
}


/*
 * Blink several LEDs on, fading out.
 */
void WS2801FX::mode_twinkle_fade(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = Adafruit_WS2801::getPixelColor(i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    WS2801FX::setPixelColor(i, px_r, px_g, px_b);
  }

  if(random(3) == 0) {
    WS2801FX::setPixelColor(random(_led_count), _mode_color);
  }

  Adafruit_WS2801::show();

  _mode_delay = 100 + ((100 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
void WS2801FX::mode_twinkle_fade_random(void) {
  _mode_color = color_wheel(random(256));
  mode_twinkle_fade();
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2801FX::mode_sparkle(void) {
  WS2801FX::clear();
  WS2801FX::setPixelColor(random(_led_count),_color);
  Adafruit_WS2801::show();
  _mode_delay = 10 + ((200 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * Lights all LEDs in the _color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2801FX::mode_flash_sparkle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  if(random(10) == 7) {
    WS2801FX::setPixelColor(random(_led_count), 255, 255, 255);
    _mode_delay = 20;
  } else {
    _mode_delay = 20 + ((200 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }

  Adafruit_WS2801::show();
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
void WS2801FX::mode_hyper_sparkle(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  if(random(10) < 4) {
    for(uint16_t i=0; i < max(1, _led_count/3); i++) {
      WS2801FX::setPixelColor(random(_led_count), 255, 255, 255);
    }
    _mode_delay = 20;
  } else {
    _mode_delay = 15 + ((120 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }

  Adafruit_WS2801::show();
}


/*
 * Classic Strobe effect.
 */
void WS2801FX::mode_strobe(void) {
  if(_counter_mode_call % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, _color);
    }
    _mode_delay = 20;
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, 0);
    }
    _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
  Adafruit_WS2801::show();
}


/*
 * Strobe effect with different strobe count and pause, controled by _speed.
 */
void WS2801FX::mode_multi_strobe(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, 0);
  }

  if(_counter_mode_step < (2 * ((_speed / 10) + 1))) {
    if(_counter_mode_step % 2 == 0) {
      for(uint16_t i=0; i < _led_count; i++) {
        WS2801FX::setPixelColor(i, _color);
      }
      _mode_delay = 20;
    } else {
      _mode_delay = 50;
    }

  } else {
    _mode_delay = 100 + ((9 - (_speed % 10)) * 125);
  }

  Adafruit_WS2801::show();
  _counter_mode_step = (_counter_mode_step + 1) % ((2 * ((_speed / 10) + 1)) + 1);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
void WS2801FX::mode_strobe_rainbow(void) {
  if(_counter_mode_call % 2 == 0) {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, color_wheel(_counter_mode_call % 256));
    }
    _mode_delay = 20;
  } else {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, 0);
    }
    _mode_delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
  }
  Adafruit_WS2801::show();
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
void WS2801FX::mode_blink_rainbow(void) {
  if(_counter_mode_call % 2 == 1) {
    for(uint16_t i=0; i < _led_count; i++) {
      WS2801FX::setPixelColor(i, color_wheel(_counter_mode_call % 256));
    }
    Adafruit_WS2801::show();
  } else {
    strip_off();
  }

  _mode_delay = 100 + ((1986 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


/*
 * _color running on white.
 */
void WS2801FX::mode_chase_white(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, 255, 255, 255);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, _color);
  WS2801FX::setPixelColor(m, _color);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * White running on _color.
 */
void WS2801FX::mode_chase_color(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, 255, 255, 255);
  WS2801FX::setPixelColor(m, 255, 255, 255);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * White running followed by random color.
 */
void WS2801FX::mode_chase_random(void) {
  if(_counter_mode_step == 0) {
    WS2801FX::setPixelColor(_led_count-1, color_wheel(_mode_color));
    _mode_color = get_random_wheel_index(_mode_color);
  }

  for(uint16_t i=0; i < _counter_mode_step; i++) {
    WS2801FX::setPixelColor(i, color_wheel(_mode_color));
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, 255, 255, 255);
  WS2801FX::setPixelColor(m, 255, 255, 255);

  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * White running on rainbow.
 */
void WS2801FX::mode_chase_rainbow(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color_wheel(((i * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, 255, 255, 255);
  WS2801FX::setPixelColor(m, 255, 255, 255);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * White flashes running on _color.
 */
void WS2801FX::mode_chase_flash(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = _counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = _counter_mode_step;
      uint16_t m = (_counter_mode_step + 1) % _led_count;
      WS2801FX::setPixelColor(n, 255, 255, 255);
      WS2801FX::setPixelColor(m, 255, 255, 255);
      _mode_delay = 20;
    } else {
      _mode_delay = 30;
    }
  } else {
    _counter_mode_step = (_counter_mode_step + 1) % _led_count;
    _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
  }

  Adafruit_WS2801::show();
}


/*
 * White flashes running, followed by random color.
 */
void WS2801FX::mode_chase_flash_random(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = _counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < _counter_mode_step; i++) {
    WS2801FX::setPixelColor(i, color_wheel(_mode_color));
  }

  if(flash_step < (flash_count * 2)) {
    uint16_t n = _counter_mode_step;
    uint16_t m = (_counter_mode_step + 1) % _led_count;
    if(flash_step % 2 == 0) {
      WS2801FX::setPixelColor(n, 255, 255, 255);
      WS2801FX::setPixelColor(m, 255, 255, 255);
      _mode_delay = 20;
    } else {
      WS2801FX::setPixelColor(n, color_wheel(_mode_color));
      WS2801FX::setPixelColor(m, 0, 0, 0);
      _mode_delay = 30;
    }
  } else {
    _counter_mode_step = (_counter_mode_step + 1) % _led_count;
    _mode_delay = 1 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);

    if(_counter_mode_step == 0) {
      _mode_color = get_random_wheel_index(_mode_color);
    }
  }

  Adafruit_WS2801::show();
}


/*
 * Rainbow running on white.
 */
void WS2801FX::mode_chase_rainbow_white(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, 255, 255, 255);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, color_wheel(((n * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  WS2801FX::setPixelColor(m, color_wheel(((m * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Black running on _color.
 */
void WS2801FX::mode_chase_blackout(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, _color);
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, 0, 0, 0);
  WS2801FX::setPixelColor(m, 0, 0, 0);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Black running on rainbow.
 */
void WS2801FX::mode_chase_blackout_rainbow(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    WS2801FX::setPixelColor(i, color_wheel(((i * 256 / _led_count) + (_counter_mode_call % 256)) % 256));
  }

  uint16_t n = _counter_mode_step;
  uint16_t m = (_counter_mode_step + 1) % _led_count;
  WS2801FX::setPixelColor(n, 0, 0, 0);
  WS2801FX::setPixelColor(m, 0, 0, 0);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random color intruduced alternating from start and end of strip.
 */
void WS2801FX::mode_color_sweep_random(void) {
  if(_counter_mode_step == 0 || _counter_mode_step == _led_count) {
    _mode_color = get_random_wheel_index(_mode_color);
  }

  if(_counter_mode_step < _led_count) {
    WS2801FX::setPixelColor(_counter_mode_step, color_wheel(_mode_color));
  } else {
    WS2801FX::setPixelColor((_led_count * 2) - _counter_mode_step - 1, color_wheel(_mode_color));
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % (_led_count * 2);
  _mode_delay = 5 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Alternating color/white pixels running.
 */
void WS2801FX::mode_running_color(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      WS2801FX::setPixelColor(i, _mode_color);
    } else {
      WS2801FX::setPixelColor(i, 255, 255, 255);
    }
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 10 + ((30 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Alternating red/blue pixels running.
 */
void WS2801FX::mode_running_red_blue(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      WS2801FX::setPixelColor(i, 255, 0, 0);
    } else {
      WS2801FX::setPixelColor(i, 0, 0, 255);
    }
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 100 + ((100 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random colored pixels running.
 */
void WS2801FX::mode_running_random(void) {
  for(uint16_t i=_led_count-1; i > 0; i--) {
    WS2801FX::setPixelColor(i, Adafruit_WS2801::getPixelColor(i-1));
  }

  if(_counter_mode_step == 0) {
    _mode_color = get_random_wheel_index(_mode_color);
    WS2801FX::setPixelColor(0, color_wheel(_mode_color));
  }

  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 2;

  _mode_delay = 50 + ((50 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * K.I.T.T.
 */
void WS2801FX::mode_larson_scanner(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = Adafruit_WS2801::getPixelColor(i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    WS2801FX::setPixelColor(i, px_r, px_g, px_b);
  }

  uint16_t pos = 0;

  if(_counter_mode_step < _led_count) {
    pos = _counter_mode_step;
  } else {
    pos = (_led_count * 2) - _counter_mode_step - 2;
  }

  WS2801FX::setPixelColor(pos, _color);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % ((_led_count * 2) - 2);
  _mode_delay = 10 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Fireing comets from one end.
 */
void WS2801FX::mode_comet(void) {

  for(uint16_t i=0; i < _led_count; i++) {
    uint32_t px_rgb = Adafruit_WS2801::getPixelColor(i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    WS2801FX::setPixelColor(i, px_r, px_g, px_b);
  }

  WS2801FX::setPixelColor(_counter_mode_step, _color);
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % _led_count;
  _mode_delay = 10 + ((10 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Firework sparks.
 */
void WS2801FX::mode_fireworks(void) {
  uint32_t px_rgb = 0;
  byte px_r = 0;
  byte px_g = 0;
  byte px_b = 0;

  for(uint16_t i=0; i < _led_count; i++) {
    px_rgb = Adafruit_WS2801::getPixelColor(i);

    px_r = (px_rgb & 0x00FF0000) >> 16;
    px_g = (px_rgb & 0x0000FF00) >>  8;
    px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    WS2801FX::setPixelColor(i, px_r, px_g, px_b);
  }

  // first LED has only one neighbour
  px_r = (((Adafruit_WS2801::getPixelColor(1) & 0x00FF0000) >> 16) >> 1) + ((Adafruit_WS2801::getPixelColor(0) & 0x00FF0000) >> 16);
  px_g = (((Adafruit_WS2801::getPixelColor(1) & 0x0000FF00) >>  8) >> 1) + ((Adafruit_WS2801::getPixelColor(0) & 0x0000FF00) >>  8);
  px_b = (((Adafruit_WS2801::getPixelColor(1) & 0x000000FF) >>  0) >> 1) + ((Adafruit_WS2801::getPixelColor(0) & 0x000000FF) >>  0);
  WS2801FX::setPixelColor(0, px_r, px_g, px_b);

  // set brightness(i) = ((brightness(i-1)/2 + brightness(i+1)) / 2) + brightness(i)
  for(uint16_t i=1; i < _led_count-1; i++) {
    px_r = ((
            (((Adafruit_WS2801::getPixelColor(i-1) & 0x00FF0000) >> 16) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i+1) & 0x00FF0000) >> 16) >> 0) ) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i  ) & 0x00FF0000) >> 16) >> 0);

    px_g = ((
            (((Adafruit_WS2801::getPixelColor(i-1) & 0x0000FF00) >> 8) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i+1) & 0x0000FF00) >> 8) >> 0) ) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i  ) & 0x0000FF00) >> 8) >> 0);

    px_b = ((
            (((Adafruit_WS2801::getPixelColor(i-1) & 0x000000FF) >> 0) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i+1) & 0x000000FF) >> 0) >> 0) ) >> 1) +
            (((Adafruit_WS2801::getPixelColor(i  ) & 0x000000FF) >> 0) >> 0);

    WS2801FX::setPixelColor(i, px_r, px_g, px_b);
  }

  // last LED has only one neighbour
  px_r = (((Adafruit_WS2801::getPixelColor(_led_count-2) & 0x00FF0000) >> 16) >> 2) + ((Adafruit_WS2801::getPixelColor(_led_count-1) & 0x00FF0000) >> 16);
  px_g = (((Adafruit_WS2801::getPixelColor(_led_count-2) & 0x0000FF00) >>  8) >> 2) + ((Adafruit_WS2801::getPixelColor(_led_count-1) & 0x0000FF00) >>  8);
  px_b = (((Adafruit_WS2801::getPixelColor(_led_count-2) & 0x000000FF) >>  0) >> 2) + ((Adafruit_WS2801::getPixelColor(_led_count-1) & 0x000000FF) >>  0);
  WS2801FX::setPixelColor(_led_count-1, px_r, px_g, px_b);

  if(!_triggered) {
    for(uint16_t i=0; i<max(1,_led_count/20); i++) {
      if(random(10) == 0) {
        WS2801FX::setPixelColor(random(_led_count), _mode_color);
      }
    }
  } else {
    for(uint16_t i=0; i<max(1,_led_count/10); i++) {
      WS2801FX::setPixelColor(random(_led_count), _mode_color);
    }
  }

  Adafruit_WS2801::show();

  _mode_delay = 20 + ((20 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}


/*
 * Random colored firework sparks.
 */
void WS2801FX::mode_fireworks_random(void) {
  _mode_color = color_wheel(random(256));
  mode_fireworks();
}


/*
 * Alternating red/green pixels running.
 */
void WS2801FX::mode_merry_christmas(void) {
  for(uint16_t i=0; i < _led_count; i++) {
    if((i + _counter_mode_step) % 4 < 2) {
      WS2801FX::setPixelColor(i, 255, 0, 0);
    } else {
      WS2801FX::setPixelColor(i, 0, 255, 0);
    }
  }
  Adafruit_WS2801::show();

  _counter_mode_step = (_counter_mode_step + 1) % 4;
  _mode_delay = 100 + ((100 * (uint32_t)(SPEED_MAX - _speed)) / _led_count);
}

/*
 * Random flickering.
 */
void WS2801FX::mode_fire_flicker(void) {
   mode_fire_flicker_int(3);
}

/*
 * Random flickering, less intesity.
 */
void WS2801FX::mode_fire_flicker_soft(void) {
   mode_fire_flicker_int(6);
}

void WS2801FX::mode_fire_flicker_int(int rev_intensity)
{
    byte p_r = (_color & 0x00FF0000) >> 16;
    byte p_g = (_color & 0x0000FF00) >>  8;
    byte p_b = (_color & 0x000000FF) >>  0;
    byte flicker_val = max(p_r,max(p_g, p_b))/rev_intensity;
    for(uint16_t i=0; i < _led_count; i++)
    {
      int flicker = random(0,flicker_val);
      int r1 = p_r-flicker;
      int g1 = p_g-flicker;
      int b1 = p_b-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      WS2801FX::setPixelColor(i, r1, g1, b1);
    }
    Adafruit_WS2801::show();
    _mode_delay = 10 + ((500 * (uint32_t)(SPEED_MAX - _speed)) / SPEED_MAX);
}


//////////////////////////////////////////////////////////////////////////////////
// Functionality that is contained in Adafruit_NeoPixel but not in Adafruit_WS2801
//////////////////////////////////////////////////////////////////////////////////

void WS2801FX::clear() {
  for(uint16_t i=0; i < _led_count; i++) {
    setPixelColor(i, 0, 0, 0);
  }
  Adafruit_WS2801::show();
}

void WS2801FX::wrapNeoPixelSetBrightness(uint8_t newBrightness) {
  if (newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint16_t scale;
    if (brightness == 0)
      scale = 0; // Avoid /0
    else if (newBrightness == 255)
      scale = 65535 / brightness;
    else
      scale = ((uint16_t) newBrightness << 8) / brightness;

    uint32_t color;
    uint8_t a, b, c;
    // for (uint16_t i = 0; i < Adafruit_WS2801::numPixels(); i++) {
    //   color = Adafruit_WS2801::getPixelColor(i);
    //   a = (((color & 0x00FF0000) >> 16) * scale) >> 8;
    //   b = (((color & 0x0000FF00) >> 8) * scale) >> 8;
    //   c = ((color & 0x000000FF) * scale) >> 8;
    //   WS2801FX::setPixelColor(i, a, b, c);
    // }
    brightness = newBrightness;
  }
}

void WS2801FX::setPixelColor(uint16_t n, uint32_t c) {
  setPixelColor(n, (c & 0x00FF0000) >> 16, (c & 0x0000FF00) >> 8, c & 0x000000FF);
}

void WS2801FX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  // BFO
  r = (pgm_read_byte(&gamma8[r]) * _brightness * blocked_array[n]) >> 8;
  g = (pgm_read_byte(&gamma8[g]) * _brightness * blocked_array[n]) >> 8;
  b = (pgm_read_byte(&gamma8[b]) * _brightness * blocked_array[n]) >> 8;
  
  if (_order == WS2801_RBG) {
    Adafruit_WS2801::setPixelColor(n, r, b, g);
  } else if (_order == WS2801_GBR) {
    Adafruit_WS2801::setPixelColor(n, g, b, r);
  } else {
    Adafruit_WS2801::setPixelColor(n, r, g, b);
  }
}

///BFO
void WS2801FX::turn_off_pixel(uint8_t s) {
  blocked_array[s] = 0;
}

void WS2801FX::turn_on_pixel(uint8_t s) {
  blocked_array[s] = 1;
}

void WS2801FX::turn_on_all_pixels() {
  for (uint8_t i = 0; i < 20; i++)
  {
    blocked_array[i] = 1;
  }
}
