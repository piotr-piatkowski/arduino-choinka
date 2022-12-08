#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

#define LED 7

#define NLEDS 16
#define PIN_LED 4
#define PIN_BTN 3

Adafruit_NeoPixel pixels(NLEDS, PIN_LED, NEO_RGB + NEO_KHZ800);

long lastClick = 0;

const int MIN_CLICK_DELAY = 200;
const int MODES_NUM = 5;
int currentMode = 0;

#define RANDOM_COLORS_LEN 20
uint32_t randomColors[RANDOM_COLORS_LEN];

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif
    pixels.begin();
    pinMode(PIN_BTN, INPUT_PULLUP);
    for(int i = 0; i < RANDOM_COLORS_LEN; i++) {
        int r = 0, g = 0, b = 0;        
        int rr = random(1, 8);
        if (rr & 0x01)
            r = random(128);
        if (rr & 0x02)
            g = random(128);
        if (rr & 0x04)
            b = random(128);
        randomColors[i] = pixels.Color(r, g, b);
    }
}

int led_row(int pnum) {
    pnum = pnum % 8;
    switch(pnum) {
        case 7: 
            return 0;
        case 6:
        case 5: 
            return 1;
        case 4:
        case 2: 
            return 2;
        case 3: 
            return 3;
        case 1:
        case 0:
            return 4;
        default:
            return 5;
    }
}

uint32_t getColor(int mode, long now, int pnum) {
    double v1 = (sin(now / 1000.0 + pnum) + 1.0) / 2.0;
    double v2 = (sin(now / 1000.0 + pnum + 3) + 1.0) / 2.0;
    int row = led_row(pnum);
    switch(mode) {
        // red/orange slow change
        case 4:
            if (v2 > v1 / 2)
                v2 = v1 / 2;
            return pixels.Color(v1 * 128, v2 * 64, 0);
        // blue/green slow change
        case 1:
            return pixels.Color(0, v1 * 128, v2 * 128);
        // white only random on/off
        case 2:
            if (v1 > v2 / 2) 
                return pixels.Color(96, 96, 96);
            else
                return pixels.Color(10, 10, 10);
        // falling blue snow
        case 3: {
            int step = (now / 100) % 10;
            pnum = pnum % 8;
            if (row == step)
                return pixels.Color(0, 0, 128);
            else if (row == step - 1)
                return pixels.Color(0, 0, 64);
            else if (row == step - 2)
                return pixels.Color(0, 0, 32);
            else if (row == step - 3)
                return pixels.Color(0, 0, 16);
            else
                return pixels.Color(0, 0, 0);
        }
        // random colors, changing each second
        case 0: {
            int offset = ((now / 1000) + pnum * 13) % RANDOM_COLORS_LEN;
            return randomColors[offset];
        }
        default:
            return pixels.Color(0, 0, 0);
    }
}

void loop() {

    long now = millis();

    // TODO - check voltage and disable if too low

    if (!digitalRead(PIN_BTN)) {
        if (now - lastClick > MIN_CLICK_DELAY) {
            currentMode++;
            if (currentMode >= MODES_NUM)
                currentMode = 0;
            for (int i = 0; i < 3; i++) {
                pixels.clear();
                pixels.fill(pixels.Color(0, 128, 0), 0, currentMode + 1);
                pixels.show();
                delay(300);
                pixels.clear();
                pixels.show();
                delay(200);
            }
        }
        lastClick = now;

    }
        
    for(int i = 0; i < NLEDS; i++)
        pixels.setPixelColor(i, getColor(currentMode, now, i));
    pixels.show();

}
