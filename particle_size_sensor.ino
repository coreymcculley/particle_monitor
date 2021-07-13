// Created by VueVille.com based on the Adafruit code for PMS5003 PM2.5 sensor
// Read the full how-to article at https://www.vueville.com/arduino/arduino-air-quality-sensor/

// Connect the PMS5003 like this if you are using a JST to DIP 2.54mm standard spacing adaptor board (easier):
// VCC to 5V (do not use Arduino to supply this voltage), GND to common ground with Arduino
// Arduino pin #8 to TX pin on sensor (this is incoming data from the sensor), leave pin #9 disconnected even though it is defined in the code below

// Connect the 1602A LCD like this if you are using an I2C module (easier)
// VCC to 5V (do not use Arduino to supply this voltage), GND to common ground with Arduino
// SDA to pin A4
// SCL to pin A5

#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <Math.h>
SoftwareSerial pmsSerial(2, 3);

#include <Wire.h>


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 12 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0;
unsigned long interval = 1000; //Refreshes the LCD once every 1000ms (1 second) without holding up the Arduino processor like delay() does. If you use a delay(), the PMS5003 will produce checksum errors

void setup() {
  // debugging output, don't forget to set serial monitor to 115200 baud
  Serial.begin(115200);

  // sensor baud rate is 9600
  pmsSerial.begin(9600);

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(50);
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;

void loop() {
  // Serial output code begins
  if (readPMSdata(&pmsSerial))
  {
      
    // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");



    int pm25 = data.pm25_standard;
    int pm100 = data.pm100_standard;

    //  Serial.print(pm25);
    //  Serial.println();
    //  Serial.print(pm100);
    //  Serial.println();

    //Conditionals for ring Color. PM2.5 > 35 Red, PM10 > 150 White, else Blue

    if (pm100 >= 150) {
      for (int i = 0; i < 12; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // white color.
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
    }
    else if (pm25 >= 35) {
      for (int i = 0; i < 12; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 000, 000)); // red color.
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
    }
    else {
      for (int i = 0; i < 12; i++) {
        pixels.setPixelColor(i, pixels.Color(000, 000, 255)); // white color.
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
    }
  }
}

// Code for calculating the sensor readings
boolean readPMSdata(Stream * s) {
  if (! s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  /* debugging
    for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
  */

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
