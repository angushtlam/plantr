#include <DHT.h>
#include <Adafruit_NeoPixel.h>

#define DHPIN 2

DHT dht(DHPIN, DHT11);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6 , NEO_GRB + NEO_KHZ800);
#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0         //deviation compensate
#define LED 13
#define pHMax 6.0
#define pHMin 5.5
#define acidPump 9
#define basePump 10
#define fan1 7
#define fan2 8

boolean fanOn = false;

String nextTemp = "";
String nextpH = "";
String nextHumidity = "";


int UPDATE_DELAY = 2000;

int ms = 0;
int nextUpdateIn = 0;

int delaypH = 60000; // 1 minute delay
int delayCount = 0;

void setup() {
  Serial.begin(9600); // Argument must be the same as the Python file

  pinMode(acidPump, OUTPUT);
  pinMode(basePump, OUTPUT);
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SensorPin, INPUT);
  
  strip.begin();
  for (int i = 0; i < 60; i++) {
    strip.setPixelColor(i, 255, 0, 255);
  }
  strip.show();

  digitalWrite(acidPump, HIGH);
  digitalWrite(basePump, HIGH);
  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, HIGH);
  
}

void loop() {
  serialEvent();

  // Wait a few seconds between measurements.
  if (nextUpdateIn > UPDATE_DELAY) {
     nextUpdateIn = 0;
    /**
    GETS TEMPERATURE
    */
  
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    nextTemp = ((String)f) + "";  // save to print later
  
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      //Serial.println("Failed to read from DHT sensor!");
      return;
    }
  
    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    //if (delayCount > delaypH) {
      /**
      GETS PH VALUE
      */
      // put your main code here, to run repeatedly:
        float sensorRaw = analogRead(SensorPin);
        float voltage = sensorRaw*5.0/1024;
        float pHValue = 3.5*voltage;
        //Serial.println(pHValue);
        nextpH = ((String) pHValue) + "";
    
      //pumpAcidBase(pHValue);
      
      delayCount = 0;
    //}
    
    runFans(t);
  }
  
  delay(1);
  ms += 1;
  nextUpdateIn += 1;
  delayCount += 1;

  if (ms > 10000) {
    ms = 0;
    Serial.print("unfrzunfrzunfrzunfrz\n");
  }
}

void serialEvent() {
  while (Serial.available() > 0) {
    Serial.println("temp:" + nextTemp + ",ph:" + nextpH + ",humid:" + nextHumidity);
    ms = 0;
  }
}

void pumpAcidBase(float pH) {
  if (pH > pHMax) {
    //Serial.println("Running Acid");
    digitalWrite(acidPump, LOW);
    //delay(1500);
    digitalWrite(acidPump, HIGH);
    return;
  }
  else if (pH < pHMin) {
    //Serial.println("Running Base");
    digitalWrite(basePump, LOW);
    //delay(1500);
    digitalWrite(basePump, HIGH);
    return;
  }
  else {
    return;
  }
}

void runFans(float temp) {
  if (temp > 75 && !fanOn) {
    digitalWrite(fan1, LOW);
    digitalWrite(fan2, LOW);
    fanOn = true;
    //Serial.println("Fans running");
  }  else if (temp < 70 && fanOn) {
    digitalWrite(fan1, HIGH);
    digitalWrite(fan2, HIGH);
    fanOn = false;
    //Serial.println("Fans not on");
  }
  
  else {
    return;
  }
}

