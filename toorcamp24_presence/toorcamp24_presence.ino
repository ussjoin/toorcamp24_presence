#include <WS2801FX.h>
#include <SoftwareSerial.h>

#define LED_COUNT 20
#define LED_DATA_PIN 2
#define LED_CLOCK_PIN 3
#define LENGTH_OF_WELCOME 20000

const byte rxPin = 8; 
const byte txPin = 9; 
 
SoftwareSerial mySerial (rxPin, txPin); // Begin a software serial on pins D3 and D2 to talk to the Pi

WS2801FX ws2801fx = WS2801FX(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN, WS2801_RGB);

uint32_t animations [25] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

String cmd = "";               // String to store incoming serial commands
boolean cmd_complete = false;  // whether the command string is complete

unsigned long animation_start_time = 0;

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  ws2801fx.init();
  //ws2801fx.setBrightness(20);
  ws2801fx.setSpeed(100);
  ws2801fx.setMode(FX_MODE_RAINBOW_CYCLE);
  // ws2801fx.turn_off_pixel(10);
  // ws2801fx.turn_off_pixel(11);
  // ws2801fx.turn_off_pixel(12);
  // ws2801fx.turn_on_all_pixels();
  // ws2801fx.turn_off_pixel(2);
  // ws2801fx.turn_off_pixel(1);
  ws2801fx.start();
  Serial.println(F("Startup complete, beginning animation."));
  mySerial.println(F("Startup complete, beginning animation."));
}

void loop() {
  // The states are
  // animation_start_time > 0: in an animation
  //   check if it's time to end the animation
  //   if not, just run the service.
  // animation_start_time == 0: 
  //   check if there's an animation waiting
  //   if so, start the animation and set the timer, then run the service.
  //   if not, just run the service.

  if (animation_start_time > 0)
  {
    unsigned long current_time = millis();
    if (current_time > animation_start_time + LENGTH_OF_WELCOME)
    {
      updateAnimation();
    }
    // otherwise, we're supposed to keep running the welcome. Fall through.
  }
  else if (animations[0] > 0) //Something's waiting to go
  {
    updateAnimation();
  }

  ws2801fx.service();

  // On Atmega32U4 based boards (leonardo, micro) serialEvent is not called
  // automatically when data arrive on the serial RX. We need to do it ourself
  #if defined(__AVR_ATmega32U4__)
  serialEvent();
  #endif



  if(cmd_complete) {
    process_command();
  }
}

void updateAnimation()
{
  uint32_t new_animation = popAnimation();
  if (new_animation > 0)
  {
    setWelcomeAnimation(new_animation);
    animation_start_time = millis();
  }
  else
  {
    setDefaultAnimation();
    animation_start_time = 0;
  }
}

void setWelcomeAnimation(uint32_t color)
{
  ws2801fx.setColor(color); 
  ws2801fx.setMode(FX_MODE_BREATH);
}

void setDefaultAnimation()
{
  ws2801fx.setMode(FX_MODE_RAINBOW_CYCLE);
}

void queueWelcome(uint32_t color)
{
  // Larger than #FFFFFF? Bye.
  if (color > 16777215)
    return;
  for (uint8_t i=0; i < 25; i++)
  {
    // Check for dupes, and otherwise for first open space.
    if (animations[i] == 0)
    {
      // free space
      animations[i] = color;
      Serial.print(F("Queued welcome animation in slot: #"));
      Serial.println(i);
      mySerial.print(F("Queued welcome animation in slot: #"));
      mySerial.println(i);
      return;
    }
    else if (animations[i] == color)
    {
      // dupe
      // in case of dupe, just don't add it. We can return.
      return;
    }
  }
  // We couldn't find a space for it. Return anyway.
  return;
}

uint32_t popAnimation()
{
  // If animations[0] == 0, the queue is empty now and we don't need to do anything.
  // If it's > 0, then take it out and bring the queue items up one.

  if (animations[0] == 0)
  {
    Serial.println(F("Returning to default animation."));
    mySerial.println(F("Returning to default animation."));
    return 0; // 0 means "just go back to normal"
  }
  
  uint32_t ret = animations[0];

  for (uint8_t i = 1; i < 25; i++)
  {
    animations[i-1] = animations[i];
  }
  animations[24] = 0; //Adding an empty to the end.

  Serial.print(F("Starting welcome animation in color: "));
  Serial.println(ret);
  mySerial.print(F("Starting welcome animation in color: "));
  mySerial.println(ret);
  return ret;
}

/*
 * Checks received command and calls corresponding functions.
   Commands are:
    ON <N> - turn light N on
    OFF <N> - turn light N off
    WEL <ABCDEF> - runs the welcome person animation in hex color ABCDEF, then returns to normal

 */
void process_command() {
  if(cmd.startsWith(F("ON "))) {
    uint8_t ind = (uint8_t) cmd.substring(3, cmd.length()).toInt();
    ws2801fx.turn_on_pixel(ind);
    Serial.print(F("Turned on light: "));
    Serial.println(ind);
    mySerial.print(F("Turned on light: "));
    mySerial.println(ind);
  }
  else if (cmd.startsWith(F("OFF "))) {
    uint8_t ind = (uint8_t) cmd.substring(4, cmd.length()).toInt();
    ws2801fx.turn_off_pixel(ind);
    Serial.print(F("Turned off light: "));
    Serial.println(ind);
    mySerial.print(F("Turned off light: "));
    mySerial.println(ind);
  }
  else if (cmd.startsWith(F("WEL "))) {
    char buf [7];
    cmd.substring(4, cmd.length()).toCharArray(buf, 7);
    //Serial.println(buf);
    uint32_t color = strtoul(buf, NULL, 16);
    //Serial.println(color);
    queueWelcome(color);
    Serial.print(F("Queued welcome animation in color: "));
    Serial.println(color);
    mySerial.print(F("Queued welcome animation in color: "));
    mySerial.println(color);
  }
  else
  {
    Serial.print("I didn't understand <");
    Serial.print(cmd);
    Serial.println(">");
    mySerial.print("I didn't understand <");
    mySerial.print(cmd);
    mySerial.println(">");
  }

  cmd = "";              // reset the commandstring
  cmd_complete = false;  // reset command complete
}

/*
 * Reads new input from serial to cmd string. Command is completed on \n
 */
void serialEvent() {
  // while(Serial.available()) {
  //   char inChar = (char) Serial.read();
  //   if(inChar == '\n') {
  //     cmd_complete = true;
  //   } else {
  //     cmd += inChar;
  //   }
  // }
  while(mySerial.available()) {
    char inChar = (char) mySerial.read();
    if(inChar == '|') {
      cmd_complete = true;
    } else {
      cmd += inChar;
    }
  }
}
