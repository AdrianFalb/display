#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST        8
#define TFT_DC         9
#define TFT_MOSI      11
#define TFT_SCLK      13

// Battery colors
#define BATTERY_BODY_COLOR   ST77XX_WHITE
#define BATTERY_FILL_COLOR   ST77XX_GREEN
#define BATTERY_BORDER_COLOR ST77XX_BLACK

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

float p = 3.1415926;
enum DisplayScreens {
    NONE = 0,
    ERROR_SCREEN = -1,
    GPS_SCREEN = 1,
    SONAR_SCREEN = 2
};

DisplayScreens currentDisplayScreen = NONE;

void drawBattery(uint16_t batteryWidth, uint16_t batteryHeight, uint16_t batteryMargin, uint16_t batteryX, uint16_t batteryY, uint8_t batteryCharge) {
  // Draw battery body
  tft.fillRect(batteryX, batteryY, batteryWidth, batteryHeight, BATTERY_BODY_COLOR);
  // Draw battery fill level
  int fillWidth = map(batteryCharge, 0, 100, 0, batteryWidth - 2 * batteryMargin);

  if (batteryCharge >= 50) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_GREEN);
  } else if (batteryCharge >= 25) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_ORANGE);
  } else if (batteryCharge >= 10) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_RED);
  }

  // Draw battery border
  tft.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, BATTERY_BORDER_COLOR);
}

void updateTopBar(bool connected) {
  // Satellites
  //tft.setCursor(68, 2);
  tft.setCursor(0, 2);
  tft.setTextSize(1);  

  int numOfSatellites = 4; // tu by som bral hodnotu z nejakej get funkcie
  int idealNumOfSatellites = 3;
  if (numOfSatellites < idealNumOfSatellites) {
    tft.setTextColor(ST77XX_MAGENTA);
  } else {
    tft.setTextColor(ST77XX_GREEN);
  }
  tft.print("Satellites: ");
  tft.print(numOfSatellites);

  // Boat battery
  /*
  tft.setCursor(tft.width() - 24, 2);
  tft.setTextColor(ST77XX_WHITE);
  int boatBattery = 50; // tu by som bral hodnotu z nejakej get funkcie
  tft.print(boatBattery);
  tft.print("%");
  */

  // Connection Status
  tft.setCursor(68, 10);
  if (connected) {
    tft.print("OK");
  } else {
    tft.print("NO");
  }

  // Draw battery icons
  drawBattery(20, 8, 2, tft.width() - 24, 2, 13); // Boat Battery
  drawBattery(20, 8, 2, tft.width() - 24, 10, 29); // Joystick Battery  

  // Joystick battery
  /*
  tft.setCursor(tft.width() - 24, 10);
  int joystickBattery = 100; // tu by som bral hodnotu z nejakej get funkcie
  tft.print(joystickBattery);
  tft.print("%");
  */
}

void drawTopBar() {
  // Mohol by som to zbuchat aj tak, ze v tejto draw funkcii vykreslim vsetko co je staticke
  // a potom v oddelenej update funkcii by som len aktualizoval premenlive hodnoty
  // kazda scena by teda mohla mat taketo dve funkcie, to by sa dalo.

  int rectX = 0;
  int rectY = 0;
  int rectWidth = tft.width();
  int rectHeight = 20; // Adjust the height of the rectangle as needed
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);  

  // Satellites
  /*
  tft.setCursor(0, 2);
  tft.setTextColor(ST77XX_WHITE);  tft.setTextSize(1);
  tft.print("Satellites: ");
  */

  // Boat Battery
  tft.setCursor(tft.width() - 56, 2);
  tft.print("Boat: ");
  tft.println();
  
  // Connection status
  tft.print("Connection: ");

  // Joystick battery
  tft.setCursor(tft.width() - 80, 10);
  tft.print("Joystick: "); 
}

void drawRightMenuBar() {

  // Menu bar
  int rectX = tft.width() - 80;
  int rectY = 20;
  int rectWidth = tft.width();
  int rectHeight = tft.height();
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLUE);

  // Buttons with text
  int buttonWidth  = 60;
  int buttonHeight = 30;

  // Menu button
  tft.fillRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_BLACK);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("MENU");
  rectY = rectY + 20 + buttonHeight;

  tft.fillRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_BLACK);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("SELECT");
  rectY = rectY + 20 + buttonHeight;

  tft.fillRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_BLACK);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("CONFIRM");
  rectY = rectY + 20 + buttonHeight;

  tft.fillRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_BLACK);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("HOME");
  tft.setCursor(rectX + 14, rectY + 24 + 8);
  //tft.print("POSITION");
  rectY = rectY + 20 + buttonHeight;  
}

void drawMainScreenBackground() {  
  // Main screen
  int rectX = 0;
  int rectY = 20;
  int rectWidth = tft.width() - 80;
  int rectHeight = tft.height() - 20;
  //tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLACK);
  tft.drawRect(rectX, rectY, rectWidth, rectHeight, ST77XX_WHITE);
}

void drawErrorScreen() {
  // Main screen
  int rectX = 0;
  int rectY = 0;
  int rectWidth = tft.width();
  int rectHeight = tft.height();
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLACK);
  tft.drawRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);

  tft.fillRect(rectX + 20, rectY + 20, rectWidth - 40, rectHeight - 40, ST77XX_BLACK);
  tft.drawRect(rectX + 20, rectY + 20, rectWidth - 40, rectHeight - 40, ST77XX_RED);

  rectX = rectX + 20;
  rectY = rectY + 20;

  tft.setCursor(rectX + 90, rectY + 60);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.print("ERROR!");
  tft.setCursor(rectX + 16, rectY + 20 + 80);
  tft.print("NO CONNECTION!");
}

void updateMainScreenGpsValues() {
  const int CURSOR_NEW_LINE = 10;

  int rectX = 20;
  int rectY = 20;
  int rectWidth = tft.width(); - 80;
  int rectHeight = tft.height();

  int buttonWidth  = 180;
  int buttonHeight = 30;
   
  tft.setTextSize(1); 
  tft.setTextColor(ST77XX_WHITE);

  // GPS Position 1
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  float latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  float longitude = 73.562;
  tft.print(longitude); // tuto by som bral hodnotu z nejakej get funkcie
  rectY = rectY + 20 + buttonHeight;

  // GPS Position 2
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = 73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);
  rectY = rectY + 20 + buttonHeight;

  // GPS Position 3
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = -73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);

  rectY = rectY + 20 + buttonHeight;

  // GPS Position 4
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);  

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = -73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);

  rectY = rectY + 20 + buttonHeight;
}

void drawMainScreenGps() {
  drawMainScreenBackground();
  
  const int CURSOR_NEW_LINE = 10;

  // Menu bar
  int rectX = 20;
  int rectY = 20;
  int rectWidth = tft.width(); - 80;
  int rectHeight = tft.height();

  // Buttons with text
  int buttonWidth  = 180;
  int buttonHeight = 30;

  // Menu button
  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 1");
  rectY = rectY + 20 + buttonHeight;

  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 2");
  rectY = rectY + 20 + buttonHeight;

  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 3");
  rectY = rectY + 20 + buttonHeight;

  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 4");
  rectY = rectY + 20 + buttonHeight;
}

DisplayScreens updateDisplay(bool connected, DisplayScreens currentScreen) {
  // Update values
  if (connected) {
    updateTopBar(connected);
    updateMainScreenGpsValues();
  } else if (currentScreen != ERROR_SCREEN) {
    currentScreen = ERROR_SCREEN;
    drawErrorScreen();
  }

  // Write logic for redrawing layout when button is pressed
  // ...

  return currentScreen;
}

void initDisplay(DisplayScreens defaultScreen) {
  // Use this initializer (uncomment) if using a 2.0" ST7789 320x240 TFT:
  tft.init(240, 320);
  tft.setRotation(1); // aby bola 0,0 v lavom hornom rohu
  tft.invertDisplay(false); // for some reason the default of the display is to be inverted...
  tft.fillScreen(ST77XX_BLACK);  

  drawTopBar();
  drawRightMenuBar();

  if (defaultScreen = GPS_SCREEN) {
    drawMainScreenGps();
  }
}

void setup(void) {
  //Serial.begin(9600);
  //Serial.print(F("Hello! ST77xx TFT Test"));

  initDisplay(0);
}

void loop() {  
  bool connected = true; // hodnotu by som bral z nejakej get funkcie
  currentDisplayScreen = updateDisplay(connected, currentDisplayScreen);
}
