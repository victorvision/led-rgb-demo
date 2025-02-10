#include "LumenProtocol.h"

extern "C" void lumen_write_bytes(uint8_t* data, uint32_t length) {
  Serial.write(data, length);
}

extern "C" uint16_t lumen_get_byte() {
  if (Serial.available()) {
    return Serial.read();
  }
  return DATA_NULL;
}

#define LCM_BAUD_RATE 115200  // Data transmission rate

// Defining addresses and pins as constants
const uint8_t RED_PIN = 9;                      // Pin for controlling the red LED
const uint8_t GREEN_PIN = 10;                   // Pin for controlling the green LED
const uint8_t BLUE_PIN = 11;                    // Pin for controlling the blue LED
const uint8_t RED_SLIDER_ADDRESS = 30;          // Address for the red slider
const uint8_t GREEN_SLIDER_ADDRESS = 50;        // Address for the green slider
const uint8_t BLUE_SLIDER_ADDRESS = 40;         // Address for the blue slider
const uint8_t PALETTE_COLORS_ADDRESS = 206;     // Address for RGB-565 color palette
const uint8_t HEX_COLOR_DISPLAY_ADDRESS = 200;  // Address for displaying the hex color

// Buffer to store the hexadecimal color string
char hexColorString[10];

// Variables to store color intensities
uint8_t colorLedRed = 0;
uint8_t colorLedGreen = 0;
uint8_t colorLedBlue = 0;
uint16_t color565Palette = 0;  // RGB-565 palette color

// Lumen packets to control sliders and display colors
lumen_packet_t paletteColors = { PALETTE_COLORS_ADDRESS, kS16 };
lumen_packet_t hexColorDisplay = { HEX_COLOR_DISPLAY_ADDRESS, kString };

// Function to update the RGB-565 color palette and send the packet
void updatePaletteColor() {
  color565Palette = ((colorLedRed >> 3) << 11) | ((colorLedGreen >> 2) << 5) | (colorLedBlue >> 3);
  paletteColors.data._s16 = color565Palette;
  lumen_write_packet(&paletteColors);  // Send the updated palette packet
}

// Function to update the hexadecimal color on the display
void updateHexColor() {
  snprintf(hexColorString, sizeof(hexColorString), "#%02X%02X%02X", colorLedRed, colorLedGreen, colorLedBlue);
  strncpy(hexColorDisplay.data._string, hexColorString, sizeof(hexColorDisplay.data._string));
  lumen_write_packet(&hexColorDisplay);  // Send the packet with the hexadecimal color
}

// Function to process the received packet and update the LEDs
void processPacket(lumen_packet_t* packet) {    
  if (packet->address == RED_SLIDER_ADDRESS) {
    colorLedRed = packet->data._s32;
    analogWrite(RED_PIN, colorLedRed);
  } else if (packet->address == GREEN_SLIDER_ADDRESS) {
    colorLedGreen = packet->data._s32;
    analogWrite(GREEN_PIN, colorLedGreen);
  } else if (packet->address == BLUE_SLIDER_ADDRESS) {
    colorLedBlue = packet->data._s32;
    analogWrite(BLUE_PIN, colorLedBlue);
  }

  // Update the palette and hex color after each change
  updatePaletteColor();
  updateHexColor();
}

void setup() {
  delay(1000);
  Serial.begin(LCM_BAUD_RATE);
  Serial.print("\r\n Begin");

  // Set the pins as outputs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void loop() {
  // Check if there are available packets
  while (lumen_available() > 0) {
    lumen_packet_t* currentPacket = lumen_get_first_packet();
    processPacket(currentPacket);  // Process the received packet
  }
}
