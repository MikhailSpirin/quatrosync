// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#include "readWavetable.h"
#include "dac.h"
#include <Ticker.h>
#include <LittleFS.h>
#include <FastLED.h>
#include <RunningMedian.h>

#define POT_PIN A0
#define TRIG_IN_PIN D7
#define BLUE_LED_PIN D6
const byte PATTERNS[8][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 1, 0, 0, 0, 1},
    {0, 0, 1, 1, 1, 0, 0, 1},
    {0, 1, 2, 0, 1, 2, 0, 1},
    {0, 1, 2, 0, 2, 1, 2, 1},
    {0, 2, 1, 0, 0, 2, 1, 2},
    {0, 2, 1, 1, 2, 2, 1, 0},
    {2, 1, 0, 1, 2, 0, 1, 2}};

byte current_cycle_number = 0;

uint16_t led_on;
Ticker update_bpm_ticker;
Ticker main_loop_ticker;
Ticker pot_poll_ticker;
RunningMedian trig_ins = RunningMedian(3);
long current_time = 0, previous_time = 0;
uint16_t current_cycle_time = 1200;
uint16_t samples_position = 0;
uint32_t perlin_position_x = 0, perlin_position_y = 0;
uint16_t out_a, out_b, out_c, out_d;
uint16_t pot_pos = 0;
uint16_t random_out;

void playStartAnimation()
{
  for (byte i = 0; i < 255; i++)
  {
    analogWrite(BLUE_LED_PIN, i);
    delay(2);
  }
  digitalWrite(BLUE_LED_PIN, 0);
}

void updatepot_pos()
{
  pot_pos = analogRead(POT_PIN);
}

uint16_t getout_a(uint16_t first, uint16_t second, uint32_t random_seed)
{
  if (pot_pos >= 0 and pot_pos < 512)
  {
    return (first * (512 - pot_pos) + second * pot_pos) >> 9;
  }
  else
  {
    // for some reason, FastLED inoise function doesn't give me whole scale, from 0 to 1<<16, so there some coeffs
    random_out = scale16(inoise16(perlin_position_x, (ESP.getCycleCount() >> 10), random_seed) - 10000, 44000) >> 3;
    return (second * (1024 - pot_pos) + (random_out) * (pot_pos - 512)) >> 9;
  }
}

uint16_t getout_b(uint16_t first, uint16_t second, uint32_t random_seed)
{
  if (pot_pos >= 0 and pot_pos < 512)
  {
    return (first * (512 - pot_pos) + second * pot_pos) >> 9;
  }
  else
  {
    // for some reason, FastLED inoise function doesn't give me whole scale, from 0 to 1<<16, so there some coeffs
    random_out = scale16(inoise16(perlin_position_x, perlin_position_y, random_seed) - 10000, 44000) >> 3;
    return (second * (1024 - pot_pos) + (random_out) * (pot_pos - 512)) >> 9;
  }
}
uint16_t getout_c(uint16_t first, uint16_t second, uint16_t third)
{
  byte pattern = (pot_pos - 1) >> 7;
  // DEBUG_PRINT(current_cycle_number % 8);
  // DEBUG_PRINT(" ");
  // DEBUG_PRINTLN(patterns[pattern][current_cycle_number % 8]);
  switch (PATTERNS[pattern][current_cycle_number % 8])
  {
  case 0:
    return first;
  case 1:
    return second;
  case 2:
    return third;
  }
  return 0;
}

uint16_t getout_d(uint16_t first, uint16_t second, uint16_t third)
{
    if (pot_pos >= 0 and pot_pos < 512)
  {
    return (first * (512 - pot_pos) + second * pot_pos) >> 9;
  }
  else
  {
    return (second * (1024 - pot_pos) + third * (pot_pos - 512)) >> 9;
  }
}

void updateDac()
{
  if (led_on)
  {
    led_on = !led_on;
    digitalWrite(BLUE_LED_PIN, led_on);
  }
  out_a = getout_a(samples[samples_position % 600][0],
                 samples[samples_position % 600][1],
                 1);
  // DEBUG_PRINTLN(out_a);
  out_b = getout_b(samples[samples_position % 600][2],
                 samples[samples_position % 600][3],
                 1 << 8);
  // DEBUG_PRINT(out_b);
  
  out_c = getout_c(samples[samples_position % 600][4],
                 samples[samples_position % 600][5],
                 samples[samples_position % 600][6]);

  // DEBUG_PRINTLN(out_c);
  out_d = getout_d(samples[(samples_position % 1200) >> 1][7],
                 samples[samples_position % 600][8],
                 samples[samples_position >> 2][9]);
  // DEBUG_PRINTLN(out_d);


  if (samples_position == (600 * 4 - 1))
  {
    led_on = !led_on;
    digitalWrite(BLUE_LED_PIN, led_on);
    samples_position = 0;
    current_cycle_number++;
  }
  else
  {
    samples_position++;
  }
  perlin_position_x += (1 << 4);

  dac.analogWrite(out_a, out_b, out_c, out_d);
}

void updateBPM()
{
  long median = trig_ins.getMedian();
  // 2147483647 is returned from RunningMedian lib if there is not enough inputs.
  // So until we have enough - don't change bpm
  if (abs(median - current_cycle_time) > 20 and median != 2147483647)
  {
    current_cycle_time = median;
    main_loop_ticker.detach();
    long ms = current_cycle_time * 4 / 600;
    if (ms < 1)
      ms = 1;
    main_loop_ticker.attach_ms(ms, updateDac);
  }
}

// Just add new timedifference to RunnnigMedian input data, prtocessing happens later on Ticker tick
IRAM_ATTR void trigInHandler()
{
  current_time = millis();
  trig_ins.add(current_time - previous_time);
  previous_time = current_time;
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // GPIO setup
  pinMode(POT_PIN, INPUT);
  pinMode(TRIG_IN_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  setupDac();

  initWaveTables();

  playStartAnimation();

  attachInterrupt(digitalPinToInterrupt(TRIG_IN_PIN), trigInHandler, RISING);

  main_loop_ticker.attach_ms(4, updateDac);
  pot_poll_ticker.attach_ms(50, updatepot_pos);
  update_bpm_ticker.attach_ms(500, updateBPM);
}

void loop()
{
  perlin_position_y += 1;
}
