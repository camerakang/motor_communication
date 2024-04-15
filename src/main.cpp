/*
   RadioLib nRF24 Transmit Example

   This example transmits packets using nRF24 2.4 GHz radio module.
   Each packet contains up to 32 bytes of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Packet delivery is automatically acknowledged by the receiver.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#nrf24

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>
void IRAM_ATTR setFlag(void);
// nRF24 has the following connections:
#define MISO_24G 6
#define MOSI_24G 5
#define CS_24G 3
#define SCK_24G 4
#define IRQ_24G 7
#define CE_24G 2
SPIClass radio_spi(HSPI);
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE0);
nRF24 radio = new Module(CS_24G, IRQ_24G, CE_24G, RADIOLIB_NC, radio_spi, spiSettings);
// nRF24 radio = new Module(3, 7, 2);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// nRF24 radio = RadioShield.ModuleA;

void setup()
{
  Serial.begin(115200);
  pinMode(IRQ_24G, INPUT);
  // initialize nRF24 with default settings
  Serial.print(F("[nRF24] Initializing ... "));
  radio_spi.begin(SCK_24G, MISO_24G, MOSI_24G, CS_24G);
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

  // set receive pipe 0 address
  // NOTE: address width in bytes MUST be equal to the
  //       width set in begin() or setAddressWidth()
  //       methods (5 by default)
  Serial.print(F("[nRF24] Setting address for receive pipe 0 ... "));
  byte addr[] = {0x01, 0x23, 0x45, 0x67, 0x89};
  state = radio.setReceivePipe(0, addr);
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  // set the function that will be called
  // when new packet is received
  radio.setPacketReceivedAction(setFlag);

  // start listening
  Serial.print(F("[nRF24] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {

    while (true)
    {
      Serial.print(F("failed, code "));
      Serial.println(state);
    };
  }

  // if needed, 'listen' mode can be disabled by calling
  // any of the following methods:
  //
  // radio.standby()
  // radio.sleep()
  // radio.transmit();
  // radio.receive();
  // radio.readData();
}

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void  setFlag(void)
{
  // we got a packet, set the flag
  receivedFlag = true;
  String str1;
  radio.readData(str1);
}

void loop()
{
  String str1;

  // check if the flag is set

  if (receivedFlag)
  {
    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);

    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int numBytes = radio.getPacketLength();
      int state = radio.readData(byteArr, numBytes);
    */

    if (state == RADIOLIB_ERR_NONE)
    {
      // packet was successfully received
      Serial.println(F("[nRF24] Received packet!"));

      // print data of the packet
      Serial.print(F("[nRF24] Data:\t\t"));
      Serial.println(str);
    }
    else
    {
      // some other error occurred
      Serial.print(F("[nRF24] Failed, code "));
      Serial.println(state);
    }

    // put module back to listen mode
    radio.startReceive();
  }
}
