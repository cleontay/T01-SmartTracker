/*
    LilyGo Ink Screen Series u8g2Fonts Test
        - Created by Lewis he
*/

// According to the board, cancel the corresponding macro definition
// #define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY_102
// #define LILYGO_EPD_DISPLAY_154
#define LILYGO_T3_S3_E_PAPER_V_1_0

#include <GxEPD.h>

#if defined(LILYGO_T5_V102) || defined(LILYGO_EPD_DISPLAY_102)
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#elif defined(LILYGO_T5_V266)
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
#elif defined(LILYGO_T5_V213)
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#elif defined(LILYGO_T3_S3_E_PAPER_V_1_0)
#include <GxDEPG0213BN/GxDEPG0213BN.h> // 2.13" b/w  form DKE GROUP
#else
// #include <GxDEPG0097BW/GxDEPG0097BW.h>     // 0.97" b/w  form DKE GROUP
// #include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
// #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
// #include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
// #include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
// #include <GxDEPG0150BN/GxDEPG0150BN.h>    // 1.51" b/w   form DKE GROUP
// #include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
// #include <GxDEPG0290R/GxDEPG0290R.h>      // 2.9" b/w/r  form DKE GROUP
// #include <GxDEPG0290B/GxDEPG0290B.h>      // 2.9" b/w    form DKE GROUP
// #include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r  form GoodDisplay
// #include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r form GoodDisplay
// #include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" b/w  form GoodDisplay 4-color
// #include <GxGDEW0213M21/GxGDEW0213M21.h>  // 2.13"  b/w Ultra wide temperature , form GoodDisplay
// #include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
// #include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w   form GoodDisplay
// #include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r form GoodDisplay
// #include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w   form GoodDisplay
// #include <GxDEPG0750BN/GxDEPG0750BN.h>    // 7.5" b/w   form DKE GROUP
#endif

#include <U8g2_for_Adafruit_GFX.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>

#define EDP_BUSY_PIN            48
#define EDP_RSET_PIN            47
#define EDP_DC_PIN              16
#define EDP_CS_PIN              7
#define EDP_CLK_PIN             5 // CLK
#define EDP_MOSI_PIN            6 // MOSI
#define EDP_MISO_PIN            3

#define USING_SOFT_SPI      //Uncomment this line to use software SPI

#if defined(USING_SOFT_SPI)
GxIO_Class io(EDP_CLK_PIN, EDP_MISO_PIN, EDP_MOSI_PIN,  EDP_CS_PIN, EDP_DC_PIN,  EDP_RSET_PIN);
#else
GxIO_Class io(SPI,  EDP_CS_PIN, EDP_DC_PIN,  EDP_RSET_PIN);
#endif
GxEPD_Class display(io, EDP_RSET_PIN, EDP_BUSY_PIN);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;


void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

#if defined(LILYGO_EPD_DISPLAY_102)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif /*LILYGO_EPD_DISPLAY_102*/

    Serial.println("SPI");
    SPI.begin(EDP_CLK_PIN, EDP_MISO_PIN, EDP_MOSI_PIN,EDP_CS_PIN);
    Serial.println("SPI DONE");

    display.init(); // enable diagnostic output on Serial
    Serial.println("init");
    u8g2Fonts.begin(display);
    Serial.println("display");
}

void loop()
{
    Serial.println("LOOP");
    u8g2Fonts.setFontMode(1);                           // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(3);                      // left to right (this is default)
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);          // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);          // apply Adafruit GFX color

    u8g2Fonts.setFont(u8g2_font_helvR14_tf);            // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall

    uint16_t x = GxEPD_WIDTH / 2  ;
    uint16_t y = GxEPD_HEIGHT / 2;

    display.fillScreen(GxEPD_WHITE);

    u8g2Fonts.setCursor(x, y);                          // start writing at this position
    u8g2Fonts.print("Hello World!");

    u8g2Fonts.setFont(u8g2_font_unifont_t_chinese2);    // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2Fonts.setCursor(x + 25, y + 32);
    u8g2Fonts.print("Hello LilyGo");

    display.update();
    delay(10000);
}