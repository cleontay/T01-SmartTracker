#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <RadioLib.h>
#include <string>
#include <cmath>
#include "boards.h"

// Constant Timing for every scan
#define SCANTIME 5

// Average Rssi Value At 1 Meter for BlueTooth device used for rssiToDistance()
#define AVGBLERSSIVALUEAT1M 55

// GroupID that distinct different group of uses, to prevent wrong info from other group (Security)
#define GROUPID 10

// Distinct ID for all the LilyGo Devices
#define DEVICENAME "ST-LGD-02"

// Total Device allowed to connect to the group and track the distance
#define TOTALDEVICEALLOWED 4

// Distance to Out of Range
#define OUTOFRANGE 2

// Bluetooth scanning variable
BLEScan* pBLEScan;

// Create an array to store the list of device added when initial paired
String deviceList[TOTALDEVICEALLOWED];

// Count for deviceList to get the total device contained
int deviceListLength = -1;

// Initialize LoRa Radio using the specific pins in ultilities.h
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

// Default transmission state
int transmissionState = RADIOLIB_ERR_NONE;

// Flag to keep track if transmission is enabled
volatile bool transmittedFlag = false;

// Flag to keep track if Received is enabled
volatile bool receivedFlag = false;

// Stores the run time of the previous LoRa Radio transmission
unsigned long transmitRunTime = 0;

// Flag to allow device adding using BLE on the initial start
volatile bool deviceRegisterFlag = true;

// Disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// Formular to calculate the RSSI signal and convert it to Distance 
double rssiToDistance(int rssi, int A, double n){
  return ceil(pow(10,(rssi - A)/(10 * n)) * 100.0) / 100.0;
}

// Function to convert int to ASSCI char
char intToAlpha(int assci){
    return assci;
}

// Data encryption function (Rotary Group Number)
String encryptData(std::string rawData){

    // Stores the encrypted data
    String encryptedData = "";

    // For each of the data character
    for(int i = 0; i<rawData.length(); i++) {
        // Get the character and add the group id to the assci number and conver it to assci and add back to the encrypted data
        encryptedData += intToAlpha(rawData.at(i) + GROUPID);
    }

    // Returns the encrypted data
    return encryptedData;
}

// Decryption Data reverse action of Rotary Group ID
String decryptData(std::string encryptedData){

    // Variable to store decrypted plain text
    String decryptedData = "";

    // For each char in the encrypted data
    for(int i = 0; i<encryptedData.length(); i++) {

        // Get the character and minus the group id and convert it back to ASSCI, then add back to the variable
        decryptedData += intToAlpha(encryptedData.at(i) - GROUPID);
    }

    // Returns the decrypted data
    return decryptedData;
}

// Validates the data if the device is valid
bool validateData(std::string decryptedData){

  // If the data starts with ST which is the device name unique code, and its not the current device
  if(decryptedData.substr(0,2) == "ST" && decryptedData != DEVICENAME){

    // For each of the device in the devicelist
    for (String deviceName : deviceList) {

      // Checks if the data is in the list of whitelisted devices
      if(strcmp(decryptedData.c_str(), deviceName.c_str()) == 0){

        // return true if valid
        return true;
      }
    }
  }

  // return false if not valid
  return false;
}

// Bluetooth callback function when scan is done
class DeviceDiscoveryCallbacks: public BLEAdvertisedDeviceCallbacks {

    // BLE scanned result returns here
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      
      // check if the device address starts with mac address "dc:54:75" to confirm that it is a lilygo device (Security)
      if(strcmp(advertisedDevice.getAddress().toString().substr (0,8).c_str(), "dc:54:75") == 0){

        // check if the number of device added to the group is below the limit before continuing and make sure that it is in the initial adding device phase
        if(deviceListLength < TOTALDEVICEALLOWED && deviceRegisterFlag){

          String temp = decryptData(advertisedDevice.getName().c_str()).c_str();
          if(strcmp(temp.substring(0,2).c_str(), "ST") == 0){
            // Increase Device List counter
            deviceListLength++;

            // Print the output in the serial console Name: _______
            Serial.printf("%i) Name: %s\n", deviceListLength, temp.c_str());

            // Added the device detected to the list of whitelisted device
            deviceList[deviceListLength] = temp.c_str();
          }
        }
        else{

          // if its not in the initial phase or when the size is over the limit

          // for each of the device found
          for (String deviceName : deviceList) {
            //RSSI <= 70 : ~1m apart
  
            //check the found device with the list of device that already been paired previously
            if(deviceName != "" && strcmp(decryptData(advertisedDevice.getName()).c_str(), deviceName.c_str()) == 0){

              // Call the rssiToDistance function to get the distance
              double distance = rssiToDistance(-advertisedDevice.getRSSI(), AVGBLERSSIVALUEAT1M, 3.9);
              if(distance > OUTOFRANGE){
                // Print out the distance of the device, Device Name: ______, Distance: _______ 
                Serial.printf("Device Name: %s, Strayed", decryptData(advertisedDevice.getName()).c_str());
              }
              else{
                // Print out the distance of the device, Device Name: ______, Distance: _______ 
                Serial.printf("Device Name: %s, Distance: %f \n", decryptData(advertisedDevice.getName()).c_str(), distance);
              }
            }
          }
        }
      }
    }
};

// LoRa Transmition Callback Function
void setTransmitFlag(void)
{
  // Check if the interrupt is enabled
  if (!enableInterrupt) {
      return;
  }

  // Stop transmission
  transmittedFlag = false;

  // Start Receiving
  receivedFlag =  true;

  // Change the callback function
  radio.clearPacketSentAction();
  radio.setPacketReceivedAction(setReceiveFlag);
}

// LoRa Receive Callback Function
void setReceiveFlag(void)
{
  // check if the interrupt is enabled
  if (!enableInterrupt) {
      return;
  }

  // we got a packet, set the flag
  receivedFlag =  true;
}


void setup() {
  // init Board runs in Board.h
  initBoard();

  // waits 1.5seconds for program to load
  delay(1500);

  // Initializing the LoRa radio
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
  radio.setPacketSentAction(setTransmitFlag);

  // Bluetooth initializing
  BLEDevice::init(encryptData(DEVICENAME).c_str());
  pBLEScan = BLEDevice::getScan(); //create new scan
  BLEDevice::startAdvertising();
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceDiscoveryCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // Send LoRa packet
  transmissionState = radio.startTransmit(encryptData(DEVICENAME).c_str()); 

  // Record the time sent
  transmitRunTime = millis();

  // Waits for 5 second before receiving
  delay(5000);

  // LoRa starts receiving
  radio.startReceive();
}

void loop() {
  // Start Scanning for devices using Bluetooth
  BLEScanResults foundDevices = pBLEScan->start(SCANTIME, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(5000);

  // Register device once and stop registering
  if(deviceRegisterFlag){
    deviceRegisterFlag = false;
  }

  // put your main code here, to run repeatedly:
  if (transmittedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    transmittedFlag = false;

    if (transmissionState != RADIOLIB_ERR_NONE){
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);
    }

    // wait a second before transmitting again
    delay(2);

    // send another one
    Serial.print(F("[SX1280] Sending another packet ...\n"));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    // transmissionState = radio.startTransmit("Hello World!");

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      int state = radio.startTransmit(byteArr, 8);
    */

    transmissionState = radio.startTransmit(encryptData(DEVICENAME).c_str());

    // we're ready to send more packets,
    // enable interrupt service routine
    enableInterrupt = true;
    delay(5000);
  }

  if (receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);

    // uint32_t counter;
    // int state = radio.readData((uint8_t *)&counter, 4);

    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int state = radio.readData(byteArr, 8);
    */

    if (state == RADIOLIB_ERR_NONE) {
        str = decryptData(str.c_str());
        if(str != "" && validateData(str.c_str())){
          // packet was successfully received
          Serial.println(F("[SX1280] Received packet!"));

          // print data of the packet
          Serial.print(F("[SX1280] Data:\t\t"));
          Serial.println(str);

          // print RSSI (Received Signal Strength Indicator)
          Serial.print(F("[SX1280] RSSI:\t\t"));
          Serial.print(rssiToDistance(radio.getRSSI(), AVGBLERSSIVALUEAT1M, 3.9));
          Serial.println(F(" m"));
        }
  #ifdef HAS_DISPLAY
    if (u8g2)
    {
        u8g2->clearBuffer();
        char buf[256];
        u8g2->drawStr(0, 12, "Received OK!");
        snprintf(buf, sizeof(buf), "Data:%s", str);
        u8g2->drawStr(0, 26, buf);
        snprintf(buf, sizeof(buf), "RSSI:%.2f", radio.getRSSI());
        u8g2->drawStr(0, 40, buf);
        snprintf(buf, sizeof(buf), "SNR:%.2f", radio.getSNR());
        u8g2->drawStr(0, 54, buf);
        u8g2->sendBuffer();
    }
  #endif

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      // Serial.println(F("[SX1280] CRC error!"));

    } else {
        // some other error occurred
        // Serial.print(F("[SX1280] Failed, code "));
        // Serial.println(state);

    }

    state = radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

  // check if time past the constant check time
  if(millis() - transmitRunTime >= (SCANTIME * 1000)){

    // update the time of transmission
    transmitRunTime = millis();

    // enable transmission
    transmittedFlag = true;
    radio.clearPacketReceivedAction();
    radio.setPacketSentAction(setTransmitFlag);
  }
}