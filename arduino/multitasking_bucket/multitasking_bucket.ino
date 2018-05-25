#include <DHT.h>

#define DHPIN 2

DHT dht(DHPIN, DHT11);
// Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6 , NEO_GRB + NEO_KHZ800);
#define sensorPin A0            //pH meter Analog output to Arduino Analog Input 0         //deviation compensate
#define LED 13
#define pHMax 7.5
#define pHMin 5.5
#define acidPump 9
#define basePump 10
#define fan1 7
#define fan2 8

// Use protection
boolean TEMP_AUTOMATION = true;
boolean FAN_DEMO = false;
int FAN_DEMO_INTERVAL_IN_MILLIS = 10000; // Turns the fan on and off with the interval given. Ignored if FORCE_FAN_ON is false.

boolean PH_AUTOMATION = true;
boolean PH_DEMO = false; // Overrides automation if true. It pumps a little bit, hopefully not enough to matter.

// These are unchanging variable to determine how long it would take use the next loop.
// final long LOOP_DELAY = 1000; // Run the loop once every second.
// final long TIMEOUT = 10000; // Try to unfreeze if Arduino has not received any new information since 10 sec ago.

long NEXT_UPDATE_TEMP = 5000; // 5 sec delay
long NEXT_UPDATE_PH = 60000; // 1 min delay
long NEXT_UPDATE_HUMIDITY = 5000; // 5 sec delay
long NEXT_UPDATE_AUTOMATION = 60000; // 1 min delay. This is for setting the millis of when values should change.
long NEXT_UPDATE_AUTOMATION_ADJUSTMENTS = 10; // 0.01 sec delay. This is for actually making the adjustments to the system.

// This variable stores the incoming byte from the serial.
int incomingData = -1;

// These values are to be send when Python interpreter calls for an update.
float currentTemp = -1.0;
float currentpH = -1.0;
float currentHumidity = -1.0;

// These keep track of the system millis and time to update.
long currentMillis = 0;

long updateTempInMillis = 0;
long updatepHInMillis = 0;
long updateHumidityInMillis = 0;
long updateAutomationInMillis = 0;
long updateAutomationAdjustmentsInMillis = 0;

// These are values to keep on the fans or pH pumps
long pHAcidPumpOnInMillis = 0;
long pHAcidPumpOffInMillis = 0;
long pHBasePumpOnInMillis = 0;
long pHBasePumpOffInMillis = 0;
long fanOnInMillis = 0;
long fanOffInMillis = 0;
boolean isFanOn = false;

void setup() {
  // Start a serial connection.
  Serial.begin(9600); // Argument must be the same as the Python file

  // Brett did this
  pinMode(acidPump, OUTPUT);
  pinMode(basePump, OUTPUT);
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(sensorPin, INPUT);

  /*
  strip.begin();
  for (int i = 0; i < 60; i++) {
    strip.setPixelColor(i, 255, 0, 255);
  }
  strip.show();
   */
   
  digitalWrite(acidPump, HIGH);
  digitalWrite(basePump, HIGH);
  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, HIGH);
  // End Brett did this
  
  // Update the current millis of the Arduino
  currentMillis = millis();

  // Don't automatically start the automation. Give the system time to get ready.
  updateAutomationInMillis = millis() + (NEXT_UPDATE_AUTOMATION);
  updateAutomationAdjustmentsInMillis = millis() + (NEXT_UPDATE_AUTOMATION_ADJUSTMENTS);
}

// The loop handles getting new values.
// The values SHOULD NOT BE SENT unless Python serial asks for it by sending the byte 1.
// This only handles the internal values that Arduino is currently getting.
void loop() {
  // Update every loop.
  currentMillis = millis();

  // Temperature data update block
  // If enough time has passed to update the temperature:
  if (currentMillis > updateTempInMillis) {
    // Ignore update if the value is null.
    // If not, convert the value to a String and store the value.
    float temp = getNewTemp();
    
    if (!isnan(temp)) {
        currentTemp = temp;
    }
    
    // Update next temperature time.
    updateTempInMillis = millis() + NEXT_UPDATE_TEMP; // Update the var for the next humidity update.
  }

  // Humidity data update block
  // If enough time has passed to update the humidity:
  if (currentMillis > updateHumidityInMillis) {
    // Ignore update if the value is null.
    // If not, convert the value to a String and store the value.
    float humidity = getNewHumidity();
    if (!isnan(humidity)) {
        currentHumidity = humidity;
    }
    
    // Update next humidity time.
    updateHumidityInMillis = millis() + NEXT_UPDATE_HUMIDITY; // Update the var for the next humidity update.
  }

  // pH data update block
  // If enough time has passed to update the pH:
  if (currentMillis > updatepHInMillis) {
    float pH = getNewpH();
    if (!isnan(pH)) {
        currentpH = pH;
    }
    
    // Update next pH time.
    updatepHInMillis = millis() + NEXT_UPDATE_PH; // Update the var for the next pH update.
  }

  // Automation: automatically adjust the temperature and pH level of the system.
  // If it is time to run the automation check.
  if (currentMillis > updateAutomationInMillis) {
    adjustpH(currentpH);
    adjustTemp(currentTemp);

    printSerial();
    
    // Update the automation millis value.
    updateAutomationInMillis = millis() + NEXT_UPDATE_AUTOMATION;
  }

  // Handle the actual changes based on the millis set.
  if (currentMillis > updateAutomationAdjustmentsInMillis) {
    makeAutomationAdjustments();
    
    // Update the automation adjustments millis value.
    updateAutomationAdjustmentsInMillis = millis() + NEXT_UPDATE_AUTOMATION_ADJUSTMENTS;
  }
  
  // Get any information from the serial port.
  //checkForSerialConnection();
}

// Broken
void checkForSerialConnection() {
  while (Serial.available() > 0) { // POSSIBLE PROGRAM LOCK.
    incomingData = Serial.read(); // Read the data.

    //if (incomingData == 1) { // 1 is the data designated to give data to the Python interpreter.
      Serial.println("temp:" + ((String) currentTemp) + ",ph:" + ((String) currentpH) + ",humid:" + ((String) currentHumidity));
      //incomingData = -1; // Reset the value so it doesn't keep sending data.
      
    //}
    
  }
}

void printSerial() {
  Serial.println("temp:" + ((String) currentTemp) + ",ph:" + ((String) currentpH) + ",humid:" + ((String) currentHumidity));
}

// Returns the humidity from the sensor.
float getNewHumidity() {
  // Reading temperature or humidity takes about 250ms
  // Sensor readings may be as old as 2 seconds ago. (it's a slow sensor)
  return dht.readHumidity(); // Read humidity from sensor.
  
}

// Returns the temperature from the sensor.
float getNewTemp() {
  // Reading temperature or humidity takes about 250ms
  // Sensor readings may be as old as 2 seconds ago. (it's a slow sensor)
  //float temp = dht.readTemperature(); // Read temperature as Celsius
  float temp = dht.readTemperature(true); // Read temperature as Fahrenheit
  
  return temp;
  
}

// Returns the pH from the sensor.
float getNewpH() {
  // I don't know what these do but it's the same as before.
  float sensorRaw = analogRead(sensorPin);
  float voltage = (sensorRaw * 5.0) / 1024;
  float pHValue = 3.5 * voltage;

  return pHValue;
  
}

// Make timing adjustments to the pH based on the current settings and pH.
void adjustpH(float pH) {
  if (PH_DEMO) {
    pHAcidPumpOnInMillis = millis();
    pHAcidPumpOffInMillis = millis() + 50;
    pHBasePumpOnInMillis = millis() + 100;
    pHBasePumpOffInMillis = millis() + 150;
  
  } else if (PH_AUTOMATION && pH > 0) {
    if (pH > pHMax) { // The pH level is basic. Add acid.
      pHAcidPumpOnInMillis = millis();
      pHAcidPumpOffInMillis = millis() + 1500;
        
    } else if (pH < pHMin) { // The pH level is acidic. Add base.
      pHBasePumpOnInMillis = millis();
      pHBasePumpOffInMillis = millis() + 1500;
      
    }
  }  
}

// Make timing adjustments to the fans based on the current settings and temperature.
void adjustTemp(float temp) {
  if (FAN_DEMO) {
    fanOnInMillis = currentMillis;
    fanOffInMillis = currentMillis + FAN_DEMO_INTERVAL_IN_MILLIS;
    
  } else if (TEMP_AUTOMATION && temp > 0) {
    if (temp > 80 && !isFanOn) { // ~75F
      // Turn on the fans
      fanOnInMillis = currentMillis;
      
    }  else if (temp < 75 && isFanOn) { // ~70F
      // Turn off the fans.
      fanOffInMillis = currentMillis;
      
    }
  }
}

void makeAutomationAdjustments() {
  // Turn on acid pump.
  if (pHAcidPumpOnInMillis != -1 && currentMillis > pHAcidPumpOnInMillis) {
    digitalWrite(acidPump, HIGH);
    pHAcidPumpOnInMillis = -1; // Prevent the function from changing the value.
    
  }

  // Turn off acid pump.
  if (pHAcidPumpOffInMillis != -1 && currentMillis > pHAcidPumpOffInMillis) {
    digitalWrite(acidPump, LOW);
    pHAcidPumpOffInMillis = -1; // Prevent the function from changing the value.
    
  }

  // Turn on base pump.
  if (pHBasePumpOnInMillis != -1 && currentMillis > pHBasePumpOnInMillis) {
    digitalWrite(basePump, HIGH);
    pHBasePumpOnInMillis = -1; // Prevent the function from changing the value.
    
  }

  // Turn off base pump.
  if (pHBasePumpOffInMillis != -1 && currentMillis > pHBasePumpOffInMillis) {
    digitalWrite(basePump, LOW);
    pHBasePumpOffInMillis = -1; // Prevent the function from changing the value.
    
  }

  // Turn on fans.
  if (fanOnInMillis != -1 && !isFanOn && currentMillis > fanOnInMillis) {
    digitalWrite(fan1, HIGH);
    digitalWrite(fan2, HIGH);
    
    isFanOn = true;
    fanOnInMillis = -1; // Prevent the function from changing the value.
    
  }

  // Turn off fans.
  if (fanOffInMillis != -1 && isFanOn && currentMillis > fanOffInMillis) {
    digitalWrite(fan1, LOW);
    digitalWrite(fan2, LOW);

    isFanOn = false;
    fanOffInMillis = -1; // Prevent the function from changing the value.
    
  } 
}
