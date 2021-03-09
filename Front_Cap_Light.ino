
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <JC_Button.h>

MPU6050 mpu6050(Wire);

// Set up the LED strip
#define NUM_LEDS 20
#define LED_PIN 3                      // Output pin fro LED strip
#define COLOR_ORDER RGB
#define LED_TYPE WS2812
#define BRIGHTNESS 255


uint32_t time, timePrev, prevTime;     // Variables for time control
bool ledState = false;
bool skip = false;
Button Button(4, 50);                  // create Button object and attach to pin 4 w/ 50mS debounce
uint8_t gHue = 0;                      // rotating "base color" used by many of the patterns
uint8_t gPattern = 0;
uint8_t gLastPattern;
// Define the array of leds
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette1;
CRGBPalette16 currentPalette2;

void setup() {

    Button.begin();

    // LED strip initialization
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );

    setupStripedPalette1( CRGB::Red, CRGB::Blue, CRGB::Yellow, CRGB::Green );
    setupStripedPalette2( CRGB::Red, CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB:: Gray);

    // Initialize the serial port
    Serial.begin(115200);              // Remember to set this same baud rate to the serial monitor

    Wire.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);

    gPattern = EEPROM.read(0);

    Serial.println();
    Serial.println("Entering run mode");

} // end of void setup()


void loop() {

    time = millis();
    mpu6050.update();
    Button.read();

    if (Button.wasReleased() || skip) {
        gPattern++;
        skip = false;
        if (gPattern > 10) gPattern = 0;
        EEPROM.write(0, gPattern);
    }

    switch (gPattern) {

        case 0:

            //   All Black
            //Serial.println("All Black");
            allBlack();
            break;

        case 1:

            //   All White
            //Serial.println("All White");
            allWhite();
            break;

        case 2:

            //   Rainbow Twinkle
            //Serial.println("Rainbow Twinkle");
            runRainbow();
            twinkle();
            break;

        case 3:

            //   Solid colour Twinkle
            //Serial.println("Solid Twinkle");
            runColor();
            twinkle();
            break;

        case 4:

            //   Christmas 1
            //Serial.println("Christmas");
            Christmas1();
            break;

        case 5:

            //  Christmas 2
            //Serial.println("Christmas 2");
            Christmas2();
            break;

        case 6:

            //  halloweenEyes
            //Serial.println("halloweenEyes");
            halloweenEyes(1, 3, 20000);
            break;

        case 7:

            //   Scanner
            //Serial.println("Scanner");
            Scanner();
            break;

        case 8:

            //   Confetti
            //Serial.println("Confetti");
            EVERY_N_MILLISECONDS(50) {
                confetti();
            }
            EVERY_N_MILLISECONDS(20) gHue++;
            FastLED.show();
            break;

        case 9:

            //   BPM
            //Serial.println("bpm");
            EVERY_N_MILLISECONDS(50) {
                bpm();
            }
            EVERY_N_MILLISECONDS(20) gHue++;
            FastLED.show();
            break;

        case 10:

            //   Spirit Level
            //Serial.println("Spirit Level");
            spiritLevel();
            break;


    } // End of switch(gPattern)

} // End of void loop()


void halloweenEyes(uint8_t eyeWidth, uint8_t eyeSpace, uint32_t maxProb) {

    static uint8_t leftEye;
    static uint8_t rightEye;
    uint32_t prob = random(0, maxProb);
    if (gLastPattern != gPattern) {
        FastLED.clear();
        FastLED.show();
        gLastPattern = gPattern;
    }
    //    Serial.println(prob);
    // Open the eyes
    if ((prob < 1) && !ledState) {
        leftEye  = random( 0, NUM_LEDS + 1 - (2 * eyeWidth) - eyeSpace );
        rightEye = leftEye + eyeWidth + eyeSpace;
        Serial.print("Open  "); Serial.print("leftEye = "); Serial.print(leftEye);
        Serial.print("  rightEye = "); Serial.println(rightEye);

        for (uint8_t i = 0; i < eyeWidth; i++) {
            for (uint8_t l = leftEye; l < leftEye + eyeWidth; l++) {
                leds[l] = CHSV(0, 255, 255);
            }
            for (uint8_t r = rightEye; r < rightEye + eyeWidth; r++) {
                leds[r] = CHSV(0, 255, 255);
            }
        }
        FastLED.show();
        ledState = true;
    }

    if (prob == maxProb / 2 && ledState) {
        Serial.print("Blink  "); Serial.print("leftEye = "); Serial.print(leftEye);
        Serial.print("  rightEye = "); Serial.println(rightEye);
        for (uint8_t i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);
        FastLED.show();
        delay (80);
        for (uint8_t i = 0; i < eyeWidth; i++) {
            for (uint8_t l = leftEye; l < leftEye + eyeWidth; l++) {
                leds[l] = CHSV(0, 255, 255);
            }
            for (uint8_t r = rightEye; r < rightEye + eyeWidth; r++) {
                leds[r] = CHSV(0, 255, 255);
            }
        }
        FastLED.show();
    }

    // Close the eyes
    if ((prob > maxProb - 3) && ledState) {
        if (random(0, 2) == 0) {            // 50/50 chance of slow/fast close
            for (uint8_t i = 0; i < 150; i++) {
                for (uint8_t i = 0; i < NUM_LEDS; i++) leds[i].fadeToBlackBy(4);
                FastLED.show();
            }
        } else {
            for (uint8_t i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(0, 0, 0);
            FastLED.show();
        }
        ledState = false;
    }
} // End of void halloweenEyes(uint8_t eyeWidth, uint8_t eyeSpace, uint32_t maxProb)

void Christmas1() {

    static uint8_t startIndex = 0;
    startIndex = startIndex - 2;  /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 12, /* higher = narrower stripes */
                  currentPalette1, 255, NOBLEND); // Alternate LINEARBLEND

    FastLED.show();
    FastLED.delay(1000 / 60);

} // End of void Christmas1()

void Christmas2() {

    static uint8_t startIndex = 0;
    startIndex = startIndex + 1;  /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 8, /* higher = narrower stripes */
                  currentPalette2, 255, LINEARBLEND); // Alternate NOBLEND

    FastLED.show();
    FastLED.delay(1000 / 60);

} // End of void Christmas2()

void spiritLevel() {

    const int16_t flashInterval = 500;                // Spirit level out of range flash interval
    float yy = NUM_LEDS / 2 + mpu6050.getAngleY();    // Reference to the middle LED of the strip
    uint8_t y = yy + .5;                              // Round the angle to an integer
    /*
        Serial.print("Xº: "); Serial.print(mpu6050.getAngleX());
        Serial.print("   |   "); Serial.print("Yº: "); Serial.print(mpu6050.getAngleY());
        Serial.print("   |   "); Serial.print("yy:"); Serial.print(yy);
        Serial.print("   |   "); Serial.print("y: "); Serial.println(y);
    */
    // Light the appropriate LEDs in the strip
    switch (y) {  // One case statement for each degree of angle

        case 0:  // -10 degrees
            FastLED.clear();
            FastLED.show();
            leds[y] = CHSV(0, 255, 255);
            leds[y + 1] = CHSV(0, 255, 255);
            FastLED.show();
            break;

        case 1:  // -9 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(0, 255, 255);
            leds[y] = CHSV(0, 255, 255);
            leds[y + 1] = CHSV(0, 255, 255);
            FastLED.show();
            break;

        case 2:  // -8 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(0, 255, 255);
            leds[y] = CHSV(0, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 3:  // -7 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(0, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 4:  // -6 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 5:  // -5 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 6:  // -4 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 7:  // -3 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 8:  // -2 degree
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(96, 255, 255);
            FastLED.show();
            break;

        case 9:  // -1 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(96, 255, 255);
            leds[y + 1] = CHSV(96, 255, 255);
            FastLED.show();
            break;

        case 10:  // 0 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(96, 255, 255);
            leds[y] = CHSV(96, 255, 255);
            leds[y + 1] = CHSV(96, 255, 255);
            FastLED.show();
            break;

        case 11:  // 1 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(96, 255, 255);
            leds[y] = CHSV(96, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 12:  // 2 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(96, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 13:  // 3 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(64, 255, 255);
            FastLED.show();
            break;

        case 14:  // 4 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(64, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 15:  // 5 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(64, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 16:  // 6 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(32, 255, 255);
            FastLED.show();
            break;

        case 17:  // 7 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(32, 255, 255);
            leds[y + 1] = CHSV(0, 255, 255);
            FastLED.show();
            break;

        case 18:  // 8 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(32, 255, 255);
            leds[y] = CHSV(0, 255, 255);
            leds[y + 1] = CHSV(0, 255, 255);
            FastLED.show();
            break;

        case 19:  // 9 degrees
            FastLED.clear();
            FastLED.show();
            leds[y - 1] = CHSV(0, 255, 255);
            leds[y] = CHSV(0, 255, 255);
            FastLED.show();
            break;

        default: // Flash the outside LEDs when out of range / > +/- 10 degrees
            if (time - prevTime >= flashInterval) {
                prevTime = time;
                if (ledState == true) {
                    leds[0] = CHSV(0, 255, 255);
                    leds[1] = CHSV(0, 255, 255);
                    leds[2] = CHSV(0, 255, 255);
                    leds[17] = CHSV(0, 255, 255);
                    leds[18] = CHSV(0, 255, 255);
                    leds[19] = CHSV(0, 255, 255);
                    FastLED.show();
                    ledState = false;
                } else {
                    FastLED.clear();
                    FastLED.show();
                    ledState = true;
                }
            }

    } // End of switch (y)

} // End of void spiritLevel()

void Scanner() {

    int8_t i;
    // First slide the led in one direction
    for (i = 0; i < NUM_LEDS; i++) {
        // Set the i'th led to red
        leds[i + 3] = CHSV(0, 255, 112);
        leds[i + 2] = CHSV(0, 255, 160);
        leds[i + 1] = CHSV(0, 255, 208);
        leds[i] = CHSV(0, 255, 255);
        // Show the leds
        FastLED.show();
        // now that we've shown the leds, fade them to black
        fadeAll();
        // Wait a little bit before we loop around and do it again
        FastLED.delay(40);
        Button.read();
        if (Button.wasReleased()) {
            skip = true;
            break;
        }
    }
    FastLED.delay(40);

    // Now go in the other direction.
    for (i = (NUM_LEDS) - 1; i >= 0; i--) {
        // Set the i'th led to red
        leds[i] = CHSV(0, 255, 255);
        leds[i - 1] = CHSV(0, 255, 224);
        leds[i - 2] = CHSV(0, 255, 160);
        leds[i - 3] = CHSV(0, 255, 112);
        // Show the leds
        FastLED.show();
        // now that we've shown the leds, fade them to black
        fadeAll();
        // Wait a little bit before we loop around and do it again
        FastLED.delay(40);
        Button.read();
        if (Button.wasReleased() || skip) {
            skip = true;
            break;
        }
    }
    FastLED.delay(40);

} // End of void Scanner()

void allWhite() {

    fill_solid(leds, NUM_LEDS, CHSV(255, 0, 255));
    FastLED.show();

} // End of void allWhite()

void allBlack() {

    FastLED.clear();
    FastLED.show();

} // End of void allBlack()

void runColor() {

    static uint8_t hue;
    if (time - timePrev >= 30) {
        timePrev = time;
        fill_solid(leds, NUM_LEDS, CHSV(hue++, 255, 255));
        FastLED.show();
    }

} // End of void runColor()

void runRainbow() {

    static uint8_t hue;
    if (time - timePrev >= 30) {
        timePrev = time;
        fill_rainbow(leds, NUM_LEDS, hue++);
        FastLED.show();
    }

} // End of void runRainbow()

void twinkle() {

    //create random twinkle
    uint8_t twinkle = random(NUM_LEDS);
    uint8_t prob = random(1, NUM_LEDS + 1);
    if (prob == 2) {
        leds[twinkle] = CRGB::White;
    }
    FastLED.show();

} // End of void twinkle()

void confetti() {                          // Random colored speckles that blink in and fade smoothly.

    fadeToBlackBy(leds, NUM_LEDS, 10);
    uint16_t pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);

} // End of void confetti()


void bpm() {                               // Colored stripes pulsing at a defined Beats-Per-Minute.

    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    CRGBPalette16 palette = PartyColors_p;

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }

} // End of void bpm()


void fadeAll() {
    for (uint8_t i = 0; i < NUM_LEDS; i++) leds[i].fadeToBlackBy(128);
}

void setupStripedPalette1( CRGB R, CRGB B, CRGB Y, CRGB G) {
    currentPalette1 = CRGBPalette16( R, R, R, R, B, B, B, B, Y, Y, Y, Y, G, G, G, G );
}

void setupStripedPalette2( CRGB R, CRGB B, CRGB Y, CRGB G, CRGB W ) {
    currentPalette2 = CRGBPalette16( R, R, R, B, B, B, G, G, G, G, Y, Y, Y, W, W, W );
}
