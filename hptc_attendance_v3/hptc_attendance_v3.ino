/* ------------------------------------------------------------------------
   Created by: Arya Tiwarekar
   ------------------------------------------------------------------------
   Download Resources
   ------------------------------------------------------------------------
   Preferences--> Aditional boards Manager URLs :
   For ESP8266 and NodeMCU - Board Version 2.6.3
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ------------------------------------------------------------------------
   HTTPS Redirect Library:
   https://github.com/jbuszkie/HTTPSRedirect
   Example Arduino/ESP8266 code to upload data to Google Sheets
   ------------------------------------------------------------------------*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
String response = " ";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
//---------------------------------------------------------------------------------------------------------
// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbx1VvOVU9o0AN4BC1q5m2x9kc8l7cALSq0b3hePpV8S6iI1nfKtb03wTHHxejPXnzU1mA";
//---------------------------------------------------------------------------------------------------------
// Enter network credentials:
const char* ssid     = "Arya";
const char* password = "String args[]";
//---------------------------------------------------------------------------------------------------------
// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String sheet_name = "\"Sheet1\"";
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\":" + sheet_name+ ", \"values\": ";
String payload = "";
//---------------------------------------------------------------------------------------------------------
// Google Sheets setup (do not edit)
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "4C 0D 17 8C F1 30 7C 3A 6F 9B 8E B4 83 0E 5C BD ED 17 3E 95";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------
// Declare variables that will be published to Google Sheets
String student_id;
String student_name;
//------------------------------------------------------------
int blocks[] = {4, 5};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
//------------------------------------------------------------
#define RST_PIN  0  //D3
#define SS_PIN   2  //D4
#define BUZZER   15  //D2
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//------------------------------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

//------------------------------------------------------------

/****************************************************************************************************
   setup Function
****************************************************************************************************/
void setup() {
  //----------------------------------------------------------
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');
  //----------------------------------------------------------
  SPI.begin();
  //----------------------------------------------------------
  //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0, 1); //col=0 row=0
  lcd.print("WiFi...");
  //----------------------------------------------------------
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  //----------------------------------------------------------
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0, 0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0, 1); //col=0 row=0
  lcd.print("Google ");
  delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    //*************************************************
    if (retval == 1) {
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      lcd.clear();
      lcd.setCursor(0, 0); //col=0 row=0
      lcd.print(msg);
      delay(2000);
      break;
    }
    //*************************************************
    else
      Serial.println("Connection failed. Retrying...");
    //*************************************************
  }
  //----------------------------------------------------------
  if (!flag) {
    //____________________________________________
    lcd.clear();
    lcd.setCursor(0, 0); //col=0 row=0
    lcd.print("Connection fail");
    //____________________________________________
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
    //____________________________________________
  }
  //----------------------------------------------------------
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  //----------------------------------------------------------
  pinMode(BUZZER, OUTPUT);
  timeClient.begin();
  timeClient.setTimeOffset(19800);
}

/****************************************************************************************************
   loop Function
****************************************************************************************************/
void loop() {
  //----------------------------------------------------------------
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  sheet_name = "\"" + currentMonthName + " " + currentYear + "\"";
  String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\":" + sheet_name+ ", \"values\": ";
  
  static bool flag = false;
  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr) {
    if (!client->connected())
    {
      client->connect(host, httpsPort);
    }
  }
  else {
    Serial.println("Error creating client object!");
  }
  //----------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to HPTC");
  lcd.setCursor(0, 1); //col=0 row=0
  lcd.print("Scan your Tag");

  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));

  //----------------------------------------------------------------
  String values = "", data;
  /*
    //creating payload - method 1
    //----------------------------------------------------------------
    ReadDataFromBlock(blocks[0], readBlockData); //student id
    data = String((char*)readBlockData); data.trim();
    student_id = data;
    //----------------------------------------------------------------
    ReadDataFromBlock(blocks[1], readBlockData); //first name
    data = String((char*)readBlockData); data.trim();
    first_name = data;
    //----------------------------------------------------------------
    ReadDataFromBlock(blocks[2], readBlockData); //last name
    data = String((char*)readBlockData); data.trim();
    last_name = data;
    //----------------------------------------------------------------
    ReadDataFromBlock(blocks[3], readBlockData); //phone number
    data = String((char*)readBlockData); data.trim();
    phone_number = data;
    //----------------------------------------------------------------
    ReadDataFromBlock(blocks[4], readBlockData); //address
    data = String((char*)readBlockData); data.trim();
    address = data; data = "";
    //----------------------------------------------------------------
    values = "\"" + student_id + ",";
    values += first_name + ",";
    values += last_name + ",";
    values += phone_number + ",";
    values += address + "\"}";
    //----------------------------------------------------------------*/
  //creating payload - method 2 - More efficient
  for (byte i = 0; i < total_blocks; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    //*************************************************
    if (i == 0) {
      data = String((char*)readBlockData);
      data.trim();
      student_id = data;
      values = "\"" + data + ",";
    }

    else if (i == total_blocks - 1) {
      data = String((char*)readBlockData);
      data.trim();
      values += data + "\"}";
    }
    //*************************************************
    else {
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
    }
  }
  ReadDataFromBlock(blocks[1], readBlockData);
  student_name = String((char*)readBlockData);
  student_name.trim();
  //----------------------------------------------------------------
  // Create json object string to send to Google Sheets
  // values = "\"" + value0 + "," + value1 + "," + value2 + "\"}"
  payload = payload_base + values;
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  //----------------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0, 0); //col=0 row=0
  lcd.print("Publishing Data");
  lcd.setCursor(0, 1); //col=0 row=0
  lcd.print("Please Wait...");
  //----------------------------------------------------------------
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if (client->POST(url, host, payload)) {
    // do stuff here if publish was successful
    response = client->getResponseBody();
    response.trim();
    Serial.println(response);
    if(response == "1"){
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }
    else if(response == "2"){
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }
    else if(response == "3"){
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
    }
    
    lcd.clear();
    lcd.setCursor(0, 0); //col=0 row=0
    lcd.print("Name: " + student_name);
    lcd.setCursor(0, 1); //col=0 row=0
    lcd.print("Thank You!");
    
  }
  //----------------------------------------------------------------
  else {
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
    lcd.clear();
    lcd.setCursor(0, 0); //col=0 row=0
    lcd.print("Failed.");
    lcd.setCursor(0, 1); //col=0 row=0
    lcd.print("Try Again");
  }
  //----------------------------------------------------------------
  // a delay of several seconds is required before publishing again
  delay(5000);
  payload = "";
}


/****************************************************************************************************

****************************************************************************************************/
/****************************************************************************************************
   ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  //----------------------------------------------------------------------------
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");
  }
  //----------------------------------------------------------------------------
}
