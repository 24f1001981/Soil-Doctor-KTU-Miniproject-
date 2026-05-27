#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

#define RXD2 16
#define TXD2 17
#define MAX485_CTRL 4
#define BTN_PIN 18

LiquidCrystal_I2C lcd(0x27, 16, 2);

//  STATES 
typedef enum {
  WELCOME,
  MODE_SELECT,
  MODE1_INSERT,
  MODE1_ANALYZE,
  MODE1_NP,
  MODE1_KPH,
  MODE1_TEMP_EC,
  MODE1_SOILMOISTURE,
  MODE1_RESULT,
  MODE2_SELECT_CROP,
  MODE2_RESULT
} State;

State currentState = WELCOME;
bool dataUpdated = false;

//  BUTTON 
#define SHORT_PRESS_TIME 500
#define LONG_PRESS_TIME 800
#define DEBOUNCE_TIME 40

bool buttonState = HIGH;
bool lastReading = HIGH;
bool longPressTriggered = false;
unsigned long buttonPressTime = 0;
unsigned long lastDebounceTime = 0;

//  SENSOR VALUES
int nitrogen = 0;
int phosphorus = 0;
int potassium = 0;
float ph = 0;
float temperature = 0;
float ec = 0;
float soil_moisture = 0;

//  CROPS 
const char* crops[] = {
  "Tomato","Red Amaranthus","Chilli","Okra","Cowpea"
};
#define CROP_COUNT 5
uint8_t modeIndex = 0;
uint8_t cropIndex = 0;


// CRC FUNCTION 

uint16_t modbusCRC(uint8_t *buf, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= buf[pos];
    for (int i = 0; i < 8; i++) {
      if (crc & 1) {
        crc >>= 1;
        crc ^= 0xA001;
      } else crc >>= 1;
    }
  }
  return crc;
}

// SENSOR COMMUNICATION 
// modbus frame structure

bool sendModbusRequest(uint16_t startReg, uint16_t numRegs, uint8_t *response, int expectedBytes) {

  uint8_t request[8];
  request[0] = 0x01;
  request[1] = 0x03;
  request[2] = startReg >> 8;
  request[3] = startReg & 0xFF;
  request[4] = numRegs >> 8;
  request[5] = numRegs & 0xFF;


  uint16_t crc = modbusCRC(request, 6);
  request[6] = crc & 0xFF;
  request[7] = crc >> 8;

// requesting data

  digitalWrite(MAX485_CTRL, HIGH);
  delay(5);
  Serial2.write(request, 8);
  Serial2.flush();
  digitalWrite(MAX485_CTRL, LOW);

  delay(150);

// received reading
  int bytesRead = Serial2.readBytes(response, expectedBytes);
  if (bytesRead != expectedBytes) return false;

// comparing crc

  uint16_t receivedCRC = (response[expectedBytes-1] << 8) | response[expectedBytes-2];
  uint16_t calculatedCRC = modbusCRC(response, expectedBytes-2);
  if (receivedCRC != calculatedCRC) return false;

  return true;
}

// reading sensor values 

bool readSoilSensor() {

  uint8_t response1[11];
  uint8_t response2[13];

  // Moisture, Temp, EC
  if (!sendModbusRequest(0x0000, 0x0003, response1, 11))
    return false;
  if (response1[2] != 6) return false;

  soil_moisture = (response1[3] << 8 | response1[4]) / 10.0;
  temperature   = (int16_t)(response1[5] << 8 | response1[6]) / 10.0;
  ec            = (response1[7] << 8 | response1[8]);
  Serial.print("RAW EC: ");
  Serial.println(ec);
  

  // NPK,ph
  
  if (!sendModbusRequest(0x0003, 0x0004, response2, 13))
    return false;
  
  Serial.println();
  if (response2[2] != 8) return false;
  ph         = (response2[3] << 8 | response2[4]) / 10.0;
  nitrogen   = (response2[5] << 8 | response2[6]);
  phosphorus = (response2[7] << 8 | response2[8]);
  potassium  = (response2[9] << 8 | response2[10]);

  return true;


}


//  SETUP

void setup() {
  
  Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2);
  pinMode(MAX485_CTRL, OUTPUT);
  digitalWrite(MAX485_CTRL, LOW);

  pinMode(BTN_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  delay(10000);  // sensor stabilization
  showWelcome();
}

void loop() {
  handleButton();
}


//  BUTTON LOGIC 

void handleButton() {

  bool reading = digitalRead(BTN_PIN);

  if (reading != lastReading)
    lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {

    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        buttonPressTime = millis();
        longPressTriggered = false;
      } else {
        if (!longPressTriggered &&
            millis() - buttonPressTime < SHORT_PRESS_TIME)
          shortPress();
      }
    }
  }

  if (buttonState == LOW && !longPressTriggered) {
    if (millis() - buttonPressTime >= LONG_PRESS_TIME) {
      longPress();
      longPressTriggered = true;
    }
  }

  lastReading = reading;
}


// SHORT PRESS 

void shortPress() {

  switch (currentState) {

    case MODE_SELECT:
      modeIndex ^= 1;
      showModeSelect();
      break;

    case MODE1_NP:
      showKPH();
      break;

    case MODE1_KPH:
      showTempEC();
      break;

    case MODE1_TEMP_EC:
      showSoilMoisture();
      break;

    case  MODE1_SOILMOISTURE:
      showCropResult();
      break;

    case MODE1_RESULT:
      showModeSelect();
      break;

    case MODE2_SELECT_CROP:
      cropIndex = (cropIndex + 1) % CROP_COUNT;
      showCropSelect();
      break;

    case MODE2_RESULT:
      showModeSelect();
      break;
      

    default:
      break;
  }
}


// LONG PRESS 


void longPress() {

  switch (currentState) {

    case MODE_SELECT:
      if (modeIndex == 0)
        showInsert();
      else
        showCropSelect();
      break;

    case MODE1_INSERT:
      showAnalyze();
      break;

    case MODE2_SELECT_CROP:
      showFertilizer();
      break;

    default:
      break;
  }
}



//  UI FUNCTIONS 

void showWelcome() {
  lcd.clear();
  lcd.print("Soil Doctor");
  lcd.setCursor(0,1);
  lcd.print("Initializing");
  delay(1500);
  showModeSelect();
}

void showModeSelect() {
  currentState = MODE_SELECT;
  dataUpdated = false;
  lcd.clear();
  lcd.print("Select Mode");
  lcd.setCursor(0,1);
  lcd.print(modeIndex==0?">Crop  Fert":" Crop >Fert");
}

void showInsert() {
  currentState = MODE1_INSERT;
  lcd.clear();
  lcd.print("Insert Device");
  lcd.setCursor(0,1);
  lcd.print("Into Soil");
}

void showAnalyze() {
  currentState = MODE1_ANALYZE;
  lcd.clear();
  lcd.print("Analyzing...");
  if (!readSoilSensor()) {
    lcd.clear();
    lcd.print("Sensor Error");
    delay(2000);
    showModeSelect();
    return;
  }
  dataUpdated = true;
  delay(1000);
  showNP();
}

void showNP() {
  currentState = MODE1_NP;
  lcd.clear();
  lcd.print("N:");
  lcd.print(nitrogen);
  lcd.print(" ppm");
  lcd.setCursor(0,1);
  lcd.print("P:");
  lcd.print(phosphorus);
  lcd.print(" ppm");
}

void showKPH() {
  currentState = MODE1_KPH;
  lcd.clear();
  lcd.print("K:");
  lcd.print(potassium);
  lcd.print(" ppm");
  lcd.setCursor(0,1);
  lcd.print("pH:");
  lcd.print(ph,1);
}

void showTempEC() {
  currentState = MODE1_TEMP_EC;
  lcd.clear();
  lcd.print("T:");
  lcd.print(temperature);
  lcd.write(0xDF);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("EC:");
  lcd.print(ec);
  lcd.print(" uS/cm");
}

void showSoilMoisture(){
  currentState = MODE1_SOILMOISTURE;
  lcd.clear();
  lcd.print("Soil Moisture:");
  lcd.setCursor(0, 1);
  lcd.print(soil_moisture);
  lcd.print(" %");

}

void showCropResult() {
  currentState = MODE1_RESULT;
  lcd.clear();
  lcd.print("Best Crop:");
  lcd.setCursor(0,1);
  lcd.print(recommendCrop());
}

void showCropSelect() {
  currentState = MODE2_SELECT_CROP;
  lcd.clear();
  lcd.print("Select Crop");
  lcd.setCursor(0,1);
  lcd.print(">");
  lcd.print(crops[cropIndex]);
}

void showFertilizer() {
  if (!dataUpdated) {
    if (!readSoilSensor()) {
      lcd.clear();
      lcd.print("Sensor Error");
      delay(2000);
      showModeSelect();
      return;
    }
    dataUpdated = true;
  }
  currentState = MODE2_RESULT;
  fertilizerRecommendationLCD(crops[cropIndex], nitrogen, phosphorus, potassium);
}


// DECISION LOGIC 

const char* recommendCrop() {
// SENSOR NOT INSERTED CHECK
  if (soil_moisture < 5 && nitrogen == 0 && phosphorus == 0 && potassium == 0) {
    return "Insert Sensor";
  }

  if (soil_moisture <= 57.5) return "Cowpea";
  if (potassium <= 160)
    return (ec <= 1600) ? "Red Amaranthus" : "Okra";
  return (soil_moisture <= 65) ? "Chilli" : "Tomato";
}

// FERTILIZER RULE ENGINE 

void fertilizerRecommendationLCD(const char* crop, int N, int P, int K) {

  lcd.clear();

  // TOMATO 
  if (strcmp(crop, "Tomato") == 0) {

    if (N < 80) print2("Low Nitrogen", "Apply Urea");
    else if (N > 120) print2("High Nitrogen", "Avoid N fert");

    else if (P < 50) print2("Low Phosphor", "Apply DAP/SSP");
    else if (P > 80) print2("High Phosphor", "Stop P apply");

    else if (K < 180) print2("Low Potassium", "Apply MOP");
    else if (K > 220) print2("High Potash", "Avoid Potash");

    else print2("NPK Optimal", "No Fertilizer");
  }

  // CHILLI
  else if (strcmp(crop, "Chilli") == 0) {

    if (N < 60) print2("Low Nitrogen", "Apply Urea");
    else if (N > 100) print2("High Nitrogen", "Reduce N");

    else if (P < 40) print2("Low Phosphor", "Apply SSP");
    else if (P > 70) print2("High Phosphor", "Avoid P fert");

    else if (K < 150) print2("Low Potassium", "K Sulphate");
    else if (K > 200) print2("High Potash", "Avoid Potash");

    else print2("NPK Optimal", "Balanced soil");
  }

  //  OKRA 
  else if (strcmp(crop, "Okra") == 0) {

    if (N < 50) print2("Low Nitrogen", "Apply Urea");
    else if (N > 90) print2("High Nitrogen", "Stop N fert");

    else if (P < 40) print2("Low Phosphor", "Apply DAP");
    else if (P > 70) print2("High Phosphor", "Avoid P fert");

    else if (K < 140) print2("Low Potassium", "Apply MOP");
    else if (K > 190) print2("High Potash", "Avoid Potash");

    else print2("NPK Optimal", "Healthy soil");
  }

  //  COWPEA 

  else if (strcmp(crop, "Cowpea") == 0) {

    if (N < 30) print2("Low Nitrogen", "Bio manure");
    else if (N > 60) print2("High Nitrogen", "Avoid N fert");

    else if (P < 40) print2("Low Phosphor", "Apply SSP");
    else if (P > 70) print2("High Phosphor", "Stop P apply");

    else if (K < 100) print2("Low Potassium", "Apply Potash");
    else if (K > 160) print2("High Potash", "Avoid Potash");

    else print2("NPK Optimal", "No fertilizer");
  }

  //  RED AMARANTHUS 
  else if (strcmp(crop, "Red Amaranthus") == 0) {

    if (N < 70) print2("Low Nitrogen", "Apply Urea");
    else if (N > 110) print2("High Nitrogen", "Avoid N fert");

    else if (P < 45) print2("Low Phosphor", "Apply SSP");
    else if (P > 75) print2("High Phosphor", "Avoid P fert");

    else if (K < 160) print2("Low Potassium", "Apply Potash");
    else if (K > 210) print2("High Potash", "Avoid Potash");

    else print2("NPK Optimal", "Balanced soil");
  }

  // DEFAULT SAFETY 
  else {
    print2("Crop Error", "Not Found");
  }
}


// LCD HELPER 

void print2(const char* l1,const char* l2){
  lcd.clear();
  lcd.print(l1);
  lcd.setCursor(0,1);
  lcd.print(l2);
}