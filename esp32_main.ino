// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <math.h>
// #include <string.h>

// #define SCREEN_WIDTH   128
// #define SCREEN_HEIGHT   64
// #define BUFFER_SIZE   (SCREEN_WIDTH*SCREEN_HEIGHT/8)

// /* ----- user parameters --------------------------------------------------- */
// static const uint8_t  choice = 1;   // 0 = checker, 1 = diagonal, 2 = circles
// static const uint16_t speed  = 0; // frame delay in ms
// /* ------------------------------------------------------------------------- */

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// static uint8_t patternBuffer[BUFFER_SIZE];

// /* util -------------------------------------------------------------------- */
// static inline void setPixel(uint8_t *buf, uint8_t x, uint8_t y, bool on)
// {
//     uint16_t idx  = (y * (SCREEN_WIDTH >> 3)) + (x >> 3); // 16 bytes per row
//     uint8_t  mask = 0x80 >> (x & 7);
//     if (on)  buf[idx] |=  mask;
//     else     buf[idx] &= ~mask;
// }

// /* pattern generators ------------------------------------------------------ */
// static void genChecker(uint8_t *buf, uint16_t phase)
// {
//     memset(buf, 0, BUFFER_SIZE);
//     for (uint8_t y = 0; y < SCREEN_HEIGHT; ++y)
//         for (uint8_t x = 0; x < SCREEN_WIDTH; ++x)
//             setPixel(buf, x, y, (((x + phase) >> 3) ^ (y >> 3)) & 1);
// }

// static void genDiagonal(uint8_t *buf, uint16_t phase)
// {
//     memset(buf, 0, BUFFER_SIZE);
//     for (uint8_t y = 0; y < SCREEN_HEIGHT; ++y)
//         for (uint8_t x = 0; x < SCREEN_WIDTH; ++x)
//             setPixel(buf, x, y, (((x + y + phase) >> 3) & 1));
// }

// static void genConcentric(uint8_t *buf, uint16_t phase)
// {
//     memset(buf, 0, BUFFER_SIZE);
//     const float cx = (SCREEN_WIDTH  - 1) * 0.5f;
//     const float cy = (SCREEN_HEIGHT - 1) * 0.5f;
//     for (uint8_t y = 0; y < SCREEN_HEIGHT; ++y)
//         for (uint8_t x = 0; x < SCREEN_WIDTH;  ++x) {
//             float dx = x - cx;
//             float dy = y - cy;
//             uint8_t ring = (uint8_t)(hypotf(dx, dy) * 0.25f);   // 4-pixel rings
//             setPixel(buf, x, y, ((ring + phase) & 1));
//         }
// }

// /* dispatcher -------------------------------------------------------------- */
// static void generatePattern(uint8_t *buf, uint8_t ch, uint16_t phase)
// {
//     switch (ch) {
//         case 0: genChecker(buf,    phase); break;
//         case 1: genDiagonal(buf,   phase); break;
//         case 2: genConcentric(buf, phase); break;
//         default: genChecker(buf,   phase); break;
//     }
// }

// /* Arduino lifecycle ------------------------------------------------------- */
// void setup()
// {
//     Serial.begin(115200);
//     Wire.begin(5, 4);                                // LOLIN32: SDA-5, SCL-4
//     if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
//         Serial.println(F("SSD1306 allocation failed"));
//         for (;;) {}
//     }
//     display.clearDisplay();
// }

// void loop()
// {
//     static uint16_t phase = 0;

//    // generatePattern(patternBuffer, choice, phase++);
//      display.clearDisplay();                                   // <-- key line
//     // display.drawBitmap(0, 0, patternBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, 1);

//     display.print('TEST 123 TEST');
//     display.println();

//     display.display();
//     delay(speed);
// }