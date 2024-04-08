#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>

#define I2C_SDA                     18
#define I2C_SCL                     17
#define OLED_RST                    UNUSE_PIN

#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              3
#define RADIO_MOSI_PIN              6
#define RADIO_CS_PIN                7
#define RADIO_DIO1_PIN              9
#define RADIO_DIO2_PIN              33
#define RADIO_DIO3_PIN              34
#define RADIO_RST_PIN               8
#define RADIO_BUSY_PIN              36

#define RADIO_RX_PIN                21
#define RADIO_TX_PIN                10

#define SDCARD_MOSI                 11
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13

#define BOARD_LED                   37
#define LED_ON                      HIGH

#define BAT_ADC_PIN                 1
#define BUTTON_PIN                  0

#define HAS_DISPLAY

SPIClass SDSPI(HSPI);
#if  defined(HAS_DISPLAY) && !defined(EDP_DISPLAY)
#include <U8g2lib.h>
#endif

#ifndef DISPLAY_MODEL
#define DISPLAY_MODEL U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#endif

DISPLAY_MODEL *u8g2 = nullptr;

SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

uint32_t counter = 0;

void setFlag(void)
{
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    transmittedFlag = true;
}

void setFlag2(void)
{
  Serial.print("here man");
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    transmittedFlag = true;
}

void initBoard(){
  Serial.begin(115200);
  Serial.println("InitBoard");
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

  #if defined(HAS_DISPLAY)
    Wire.begin(I2C_SDA, I2C_SCL);
    
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
        Serial.println("Started OLED");
        u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
        u8g2->begin();
        u8g2->clearBuffer();
        u8g2->setFlipMode(0);
        u8g2->setFontMode(1); // Transparent
        u8g2->setDrawColor(1);
        u8g2->setFontDirection(0);
        u8g2->firstPage();
        do {
            u8g2->setFont(u8g2_font_inb19_mr);
            u8g2->drawStr(0, 30, "LilyGo");
            u8g2->drawHLine(2, 35, 47);
            u8g2->drawHLine(3, 36, 47);
            u8g2->drawVLine(45, 32, 12);
            u8g2->drawVLine(46, 33, 12);
            u8g2->setFont(u8g2_font_inb19_mf);
            u8g2->drawStr(58, 60, "LoRa");
        } while ( u8g2->nextPage() );
        u8g2->sendBuffer();
        u8g2->setFont(u8g2_font_fur11_tf);
        delay(3000);
    }
  #endif
}

void setup() {
  // put your setup code here, to run once:
  // put your setup code here, to run once:
  initBoard();
  delay(1500);
  Serial.print(F("[SX1280] Initializing ... "));
  int state = radio.begin();
  #ifdef HAS_DISPLAY
    if (u8g2) {
      if (state != RADIOLIB_ERR_NONE) {
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Initializing: FAIL!");
        u8g2->sendBuffer();
      }
      else{
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Initializing: Successful!");
        u8g2->sendBuffer();
      }
    }
  #endif

  #if defined(RADIO_RX_PIN) && defined(RADIO_TX_PIN)
    //Set ANT Control pins
    radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
  #endif

  int8_t TX_Power = 13;

  if (radio.setOutputPower(TX_Power) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
      Serial.println(F("Selected output power is invalid for this module!"));
      while (true);
  }

  // set carrier frequency to 2410.5 MHz
  if (radio.setFrequency(2400.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
      Serial.println(F("Selected frequency is invalid for this module!"));
      while (true);
  }

  // set bandwidth to 203.125 kHz
  if (radio.setBandwidth(203.125) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
      Serial.println(F("Selected bandwidth is invalid for this module!"));
      while (true);
  }

  // set spreading factor to 10
  if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
      Serial.println(F("Selected spreading factor is invalid for this module!"));
      while (true);
  }

  // set coding rate to 6
  if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE) {
      Serial.println(F("Selected coding rate is invalid for this module!"));
      while (true);
  }

  // set the function that will be called
  // when packet transmission is finished
  radio.setDio1Action(setFlag);

  // start transmitting the first packet
  Serial.print(F("[SX1280] Sending first packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long
  // transmissionState = radio.startTransmit("Hello World!");

  // you can also transmit byte array up to 256 bytes long
  // byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
  //                   0x89, 0xAB, 0xCD, 0xEF
  //                  };
  // state = radio.startTransmit(byteArr, 8);


  transmissionState = radio.startTransmit("Hello Word!"); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if (transmittedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    transmittedFlag = false;

    if (transmissionState == RADIOLIB_ERR_NONE) {
            // packet was successfully sent
      Serial.println(F("transmission finished!"));
      radio.setDio1Action(setFlag2);

      // NOTE: when using interrupt-driven transmit method,
      //       it is not possible to automatically measure
      //       transmission data rate using getDataRate()
      #ifdef HAS_DISPLAY
        if (u8g2) {
            u8g2->clearBuffer();
            u8g2->drawStr(0, 12, "Transmitting: OK!");
            u8g2->drawStr(0, 30, ("TX:" + String(counter)).c_str());
            u8g2->sendBuffer();
        }
      #endif
    } else {
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);
    }

    // wait a second before transmitting again
    delay(2);

    // send another one
    Serial.print(F("[SX1280] Sending another packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    // transmissionState = radio.startTransmit("Hello World!");

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      int state = radio.startTransmit(byteArr, 8);
    */

    transmissionState = radio.startTransmit("Hello Word!");

    counter++;
    // we're ready to send more packets,
    // enable interrupt service routine
    enableInterrupt = true;
    delay(5000);
  }
}
