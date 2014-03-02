/*
 * Read the UID value from the card, and broadcast it to a central controller for verification. .
 *
 * Tim Stephens 5 January 2014
 */

#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#include <RF24.h>
//#include "printf.h"
#include <aes.h>
#include <AESLib.h>


RF24 radio(9,10); //The CE, CSN pins on the radio. 

#define SS_PIN 7
#define RST_PIN 6

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.


uint8_t key[] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15  };
void setup() {

  // init radio for writing on channel 76
  radio.begin();
  //radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(0x4c);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.openReadingPipe(1,0xF0F0F0F0D2LL); 
  radio.enableDynamicPayloads();
  radio.powerUp();

  Serial.begin(9600);	// Initialize serial communications with the PC
  while(!Serial);  //Make sure that we can see the diagnostics before we continue. 
  Serial.println("Begin");
  SPI.begin();		// Init SPI bus
  mfrc522.PCD_Init();	// Init MFRC522 card
  Serial.println("Initialised and ready to broadcast.");
}

void loop() {
  char outBuffer[32] = ""; //Somewhere to store the data.
  char receivePayload[32] = "";
  char readTimeout=0;
  int salt;  //Pad the data that's sent with something extra so that it's not the same every time (even for the same card)
  //Serial.println("start");
  radio.printDetails();
  delay(200);
  
  //Look for new cards
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  //Select a new card
  if (! mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  // Dump UID
  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  } 
  Serial.println();
  
  //Send some sort of identifier so that we cn use this card scanner for other applications than just entry (like controlling the beer fridge). 
  salt = millis() %1000;
  sprintf(outBuffer, "d1%X %X %X %X%i\0", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3], salt);
  
  Serial.print("outBuffer=");
  Serial.println(outBuffer);
  //Encrypt the data.
  
  aes128_enc_single(key, outBuffer);

  Serial.print("Encrypted Data: ");
  Serial.println(outBuffer);
  
  delay(200);  //Why is there a delay here? 
  
  //bool RF24::write( const void* buf, uint8_t len )
  
  bool success = radio.write(outBuffer, 16);
  Serial.print("radio.write ");g
  Serial.println(success, DEC);
  // ========================================================================
  /* For the door to be unlocked, the controller needs to respond to the transmitted key within about 0.2s. 
     The code is never going to reach this point unless a key has been scanned, so you'd need to scan a card 
     and then immediately send the 'open' command before the main controller could respond. If there's no card 
     scanned, the door will never open.
     */
 // ========================================================================
  
  radio.startListening();
 //======================
     unsigned long started_waiting_at = millis();

 
 bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 500 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      Serial.println("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
     uint8_t len = radio.getDynamicPayloadSize();
    radio.read(receivePayload, len);
    }
 
 
 //=======================
  /*while(readTimeout < 400)  //This introduces some delay into the unlocking procedure, so isn't ideal -- should probably unlock the door immediately after a suitable payload is received. 
 {
   while(radio.available()) {
    uint8_t len = radio.getDynamicPayloadSize();
    radio.read(receivePayload, len);
  }
  delay(2);
  readTimeout++;
}
  */
 
    
  radio.stopListening();
    Serial.print("Received a ");
    Serial.print(receivePayload);
    Serial.println(";");
    
    //Now need to decrypt the payload
    //Use the handy AESLib function to overwrite in place 
     aes128_dec_single(key, receivePayload);
    Serial.print("Decrypted Data");
    Serial.println(receivePayload);
    
    if(receivePayload[0] == '1') {
       tone(3, 4000, 20);
    } else {
      tone(3, 200, 100);
    }
  delay(1000);
}




