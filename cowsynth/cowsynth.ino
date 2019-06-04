#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include <FastLED.h>
#include <SD.h>
#include <Bounce.h>
#include <Audio.h>

#include "MOO_samples.h"

// GUItool: begin automatically generated code
AudioPlaySdRaw           playSdRaw1;     //xy=268,39
AudioPlaySdRaw           playSdRaw3;     //xy=268,123
AudioPlaySdRaw           playSdRaw2;     //xy=269,80
AudioSynthWavetable      wavetable4;     //xy=310,512
AudioSynthWavetable      wavetable5;     //xy=311,550
AudioSynthWavetable      wavetable6;     //xy=311,588
AudioSynthWavetable      wavetable1;     //xy=313,345
AudioSynthWavetable      wavetable3;     //xy=313,414
AudioSynthWavetable      wavetable2;     //xy=315,380
AudioPlaySdRaw           playSdRaw4;     //xy=368,214
AudioMixer4              wavetablemixer2; //xy=518,469
AudioMixer4              backgroundmixer;         //xy=524,86
AudioMixer4              wavetablemixer; //xy=676,338
AudioMixer4              mainmixer;      //xy=715,164
AudioOutputUSB           usb1;           //xy=921,134
AudioOutputI2S           i2s1;           //xy=930,210
AudioConnection          patchCord1(playSdRaw1, 0, backgroundmixer, 0);
AudioConnection          patchCord2(playSdRaw3, 0, backgroundmixer, 2);
AudioConnection          patchCord3(playSdRaw2, 0, backgroundmixer, 1);
AudioConnection          patchCord4(wavetable4, 0, wavetablemixer2, 0);
AudioConnection          patchCord5(wavetable5, 0, wavetablemixer2, 2);
AudioConnection          patchCord6(wavetable6, 0, wavetablemixer2, 3);
AudioConnection          patchCord7(wavetable1, 0, wavetablemixer, 0);
AudioConnection          patchCord8(wavetable3, 0, wavetablemixer, 2);
AudioConnection          patchCord9(wavetable2, 0, wavetablemixer, 1);
AudioConnection          patchCord10(playSdRaw4, 0, mainmixer, 1);
AudioConnection          patchCord11(wavetablemixer2, 0, wavetablemixer, 3);
AudioConnection          patchCord12(backgroundmixer, 0, mainmixer, 0);
AudioConnection          patchCord13(wavetablemixer, 0, mainmixer, 2);
AudioConnection          patchCord14(mainmixer, 0, usb1, 0);
AudioConnection          patchCord15(mainmixer, 0, i2s1, 0);
AudioConnection          patchCord16(mainmixer, 0, usb1, 1);
AudioConnection          patchCord17(mainmixer, 0, i2s1, 1);
AudioControlSGTL5000     headphones;     //xy=1021,321
// GUItool: end automatically generated code

enum {
  MIXER_BG = 0,
  MIXER_SFX = 1,
  MIXER_SYNTH = 2,
};

#define NUM_LEDS_PER_STRIP 180
#define NUM_STRIPS         8

#define NUM_TEATS          6
#define NUM_LEDS_PER_TEAT  16

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];
CRGBSet strips[] = {
  CRGBSet(&leds[1 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[3 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[2 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[5 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[4 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[6 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[7 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
  CRGBSet(&leds[0 * NUM_LEDS_PER_STRIP], NUM_LEDS_PER_STRIP),
};

enum {
  L1 = 0,
  L2,
  L3,
  L4,
  L5,
  L6,
  L7,
  L8,
};

#define EXT_STRIP  L1

float currentMixer[4] = {0};
float targetMixer[4] = {0};


void addGlitter( CRGBSet leds, fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(leds.size()) ] += CRGB::White;
  }
}

void FillLEDsFromPaletteColors(CRGBSet leds, CRGBPalette16 &palette, uint8_t colorIndex, uint8_t step) {    
    for( int i = 0; i < leds.size(); i++, colorIndex+=step) {
        leds[i] = ColorFromPalette( palette, colorIndex, 255, LINEARBLEND);
    }
}

struct Teat 
{
  int m_pin;
  float m_current;
  uint32_t m_pull_start;
  uint32_t m_fully_pulled_start;
  bool m_pulling;
  bool m_pull_started;
  bool m_pull_ended;
  bool m_fully_pulled;
  bool m_fully_pulled_started;
  bool m_fully_pulled_ended;
  int m_freq;
  AudioSynthWavetable * m_wavetable;
  
  Teat(int pin, AudioSynthWavetable * wavetable, int freq) {
     m_pin = pin;
     m_wavetable = wavetable;
     m_freq  = freq;

    if (m_wavetable) {
     m_wavetable->setInstrument(MOO);
     m_wavetable->amplitude(1);
    }
  }

  void update() {
    // read sensor
    m_current = (analogRead(m_pin) / 1024.0);

    m_pull_started = false;
    m_pull_ended = false;
    m_fully_pulled_started = false;
    m_fully_pulled_ended = false;

    if (!m_pulling) {
       if (m_current > 0.2) {
          m_pulling = true;
          m_pull_started = true; 
          m_pull_start = millis();
       }
    } else {
      if (m_current < 0.1) {
          m_pulling = false;
          m_pull_ended = true;
      }
    }

    if (!m_fully_pulled) {
       if (m_current > 0.9) {
          m_fully_pulled = true;
          m_fully_pulled_started = true; 
          m_fully_pulled_start = millis();
       }
    } else {
      if (m_current < 0.8) {
          m_fully_pulled = false;
          m_fully_pulled_ended = true;
      }
    }

  }

  void synth() {
    if (NULL == m_wavetable) return;
    
    m_wavetable->amplitude(m_current);
    if (m_pull_started) {
      m_wavetable->playFrequency(m_freq); 
    }

    if (m_pull_ended) {
      m_wavetable->stop();
    }
  }
  
  void render(CRGBSet leds) {

    CRGB c1(0xb0, 0xb0, 0);
    CRGB c2 = blend(CRGB::Green, CRGB::Red, constrain(map(pullDuration(), 2000, 7000, 0, 255), 0, 255));

    leds.fill_solid(blend(c1, c2, m_current * 255));
    
    uint8_t overlay = constrain(map((float)fullyPullDuration(), 2000, 5000, 0, 255), 0, 255);
    overlay = scale8(overlay, 255-beat8(150));
    leds.nblend(CRGB::White, overlay);
  }

  uint32_t pullDuration() {
    if (!m_pulling) {
      return 0;
    }

    return millis() - m_pull_start;
  }

  uint32_t fullyPullDuration() {
    if (!m_fully_pulled) {
      return 0;
    }

    return millis() - m_fully_pulled_start;
  }

};


// Pentatonic major scale - C, D, E, G, A, C
Teat teats[] = {
//  Teat(A1, &wavetable1, 262),
//  Teat(A20, &wavetable2, 294),
//  Teat(A19, &wavetable3, 330),
//  Teat(A18, &wavetable4, 392),
//  Teat(A17, &wavetable5, 440),
//  Teat(A16, &wavetable6, 523),
  Teat(A1, &wavetable1, 131),
  Teat(A20, &wavetable2, 147),
  Teat(A19, &wavetable3, 165),
  Teat(A18, &wavetable4, 196),
  Teat(A17, &wavetable5, 220),
  Teat(A16, &wavetable6, 262),


};

void renderTest() {
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    }
  }

  // Set the first n leds on each strip to show which strip it is
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j <= i; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
    }
  }

  hue++;
}

void setup() {
  Serial.begin(9600);
  //while (!Serial);
  
  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP);
  //LEDS.setBrightness(192);

  AudioMemory(100);

  Serial.println("Opening SD...");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Error opening SD");
  } else {
    Serial.println("Done");
  }

  headphones.enable();
  headphones.volume(1.0);

  mainmixer.gain(MIXER_BG, 0.4);
  mainmixer.gain(MIXER_SFX, 0.6);
  mainmixer.gain(MIXER_SYNTH, 1.5);
  mainmixer.gain(3, 0);

  for (int i=0; i<4; i++) {
    wavetablemixer.gain(i, 1);
    wavetablemixer2.gain(i, 1);
  }
}

float lerp(float a, float b, float x)
{ 
 return a + x * (b - a);
}

void loop() {
  
  static int karma = 0;
  static int currentLevel = -1;
  static uint32_t cowgasm_time = 0;

  // update analog sensors
  for (int i=0; i<6; i++) {
    teats[i].update();
    teats[i].synth();

    if (teats[i].m_pull_started) {
      karma += 500; 
    }
  }
  
  if (karma > 0) karma--;

  if (karma > 3000) {
     currentLevel = 2;
  } else if (karma > 0) {
     currentLevel = 1;
  } else {
     currentLevel = 0; 
  }

  if (karma > 10000) {
     playSdRaw4.play("COWGASM.RAW");
     karma = 0;
     cowgasm_time = millis();
  }

  for (int i=0; i<6; i++) {
    teats[i].render(strips[L7](i * NUM_LEDS_PER_TEAT, (i+1) * NUM_LEDS_PER_TEAT));

    // this is evil. oops.
    if (teats[i].pullDuration() > 20000 && !playSdRaw4.isPlaying()) {
       playSdRaw4.play("MADCOW.RAW");
    }
  }

  static uint8_t hue = 0;
  hue++;
  strips[EXT_STRIP].fill_rainbow(hue);
  strips[L5].fill_rainbow(hue);

  // handle background sound transitions

  for (int i=0; i<4; i++) {
    targetMixer[i] = 0;
  }
  
  if (millis() - cowgasm_time < 15000 && cowgasm_time > 0) {
    uint8_t overlay = constrain(map(millis() - cowgasm_time, 0, 15000, 0, 255), 0, 255);
    overlay = scale8(overlay, beatsin8(200));
    strips[EXT_STRIP].nblend(CRGB::White, overlay);    
    strips[L5].nblend(CRGB::White, overlay);
    karma = 0;
  } else {
    // only play background noise if not having a cowgasm
    targetMixer[currentLevel] = 1.0;
  }

  for (int i=0; i<4; i++) {
    currentMixer[i] = lerp(currentMixer[i], targetMixer[i], 0.03);
    backgroundmixer.gain(i, currentMixer[i]);
  }


  // Background tracks
  if (!playSdRaw1.isPlaying() && currentMixer[0] > 0.02) {
    playSdRaw1.play("LEVEL0.RAW");
  } else if (playSdRaw1.isPlaying() && currentMixer[0] < 0.02) {
    playSdRaw1.stop();
  }
  
  if (!playSdRaw2.isPlaying() && currentMixer[1] > 0.02) {
    playSdRaw2.play("LEVEL1.RAW");
  } else if (playSdRaw2.isPlaying() && currentMixer[1] < 0.02) {
    playSdRaw2.stop();
  }
  
  if (!playSdRaw3.isPlaying() && currentMixer[2] > 0.02) {
    playSdRaw3.play("LEVEL2.RAW");
  } else if (playSdRaw3.isPlaying() && currentMixer[2] < 0.02) {
    playSdRaw3.stop();
  }

    
  LEDS.show();
  LEDS.delay(10);
}
