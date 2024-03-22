#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST        8
#define TFT_DC         9
#define TFT_MOSI      11
#define TFT_SCLK      13

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

enum DisplayScreens {
    NONE = 0,
    ERROR_SCREEN = -1,
    GPS_SCREEN = 1,
    SONAR_SCREEN = 2
};

DisplayScreens currentDisplayScreen = NONE;

void drawBattery(uint16_t batteryWidth, uint16_t batteryHeight, uint16_t batteryMargin, uint16_t batteryX, uint16_t batteryY, uint8_t batteryCharge) {
  /// Battery colors
  const uint16_t BATTERY_BODY_COLOR = ST77XX_WHITE;
  const uint16_t BATTERY_FILL_COLOR = ST77XX_GREEN;
  const uint16_t BATTERY_BORDER_COLOR = ST77XX_BLACK;

  /// Draw battery body
  tft.fillRect(batteryX, batteryY, batteryWidth, batteryHeight, BATTERY_BODY_COLOR);

  /// Draw battery fill level
  int fillWidth = map(batteryCharge, 0, 100, 0, batteryWidth - 2 * batteryMargin);

  if (batteryCharge >= 50) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_GREEN);
  } else if (batteryCharge >= 25) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_ORANGE);
  } else if (batteryCharge >= 10) {
    tft.fillRect(batteryX + batteryMargin, batteryY + batteryMargin, fillWidth, batteryHeight - 2 * batteryMargin, ST77XX_RED);
  }

  /// Draw battery border
  tft.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, BATTERY_BORDER_COLOR);
}

void updateTopBar(bool connected, DisplayScreens currentScreen) {
  /// Satellites
  //tft.setCursor(68, 2);
  tft.setCursor(0, 2);
  tft.setTextSize(1);  

  uint16_t numOfSatellites = 8; // tu by som bral hodnotu z nejakej get funkcie
  const uint8_t idealNumOfSatellites = 7;
  const uint8_t useableNumOfSatellites = 5;

  if (numOfSatellites < useableNumOfSatellites) {
    tft.setTextColor(ST77XX_MAGENTA); // bieda

  } else if (numOfSatellites < idealNumOfSatellites && numOfSatellites >= useableNumOfSatellites) {
    tft.setTextColor(ST77XX_ORANGE); // take da sa

  } else {
    tft.setTextColor(ST77XX_GREEN); // dobre
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

  /// Connection Status
  tft.setCursor(72, 10);
  if (connected) {
    tft.setTextColor(ST77XX_GREEN);
    tft.print("OK");
  } else {
    tft.setTextColor(ST77XX_MAGENTA);
    tft.print("NO");
  }

  /// Draw battery icons
  drawBattery(20, 8, 2, tft.width() - 24, 2, 13); // Boat Battery
  drawBattery(20, 8, 2, tft.width() - 24, 10, 29); // Joystick Battery

  /// Current Screen
  switch (currentScreen) {
    case GPS_SCREEN:
      tft.setCursor(100, 2);
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("GPS Menu");
      break;

    case SONAR_SCREEN:
      tft.setCursor(100, 2);
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("SONAR Menu");
      break;

    default:
      tft.setCursor(100, 2);
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("No screen selected");
      break;
  } 

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
  int rectX = 0;
  int rectY = 0;
  int rectWidth = tft.width();
  int rectHeight = tft.height();
  
  /// Fill background
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLACK);
  tft.drawRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);

  /// Draw inner rectangle
  tft.fillRect(rectX + 20, rectY + 20, rectWidth - 40, rectHeight - 40, ST77XX_BLACK);
  tft.drawRect(rectX + 20, rectY + 20, rectWidth - 40, rectHeight - 40, ST77XX_RED);

  rectX = rectX + 20;
  rectY = rectY + 20;

  /// Display error message
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);

  tft.setCursor(rectX + 90, rectY + 60);  
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
  int rectX = 20;
  int rectY = 20;
  int rectWidth = tft.width(); - 80;
  int rectHeight = tft.height();

  int buttonWidth  = 180;
  int buttonHeight = 30;

  /// GPS Position 1
  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 1");
  rectY = rectY + 20 + buttonHeight;

  /// GPS Position 2
  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 2");
  rectY = rectY + 20 + buttonHeight;

  /// GPS Position 3
  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 3");
  rectY = rectY + 20 + buttonHeight;

  /// GPS Position 4
  tft.drawRect(rectX + 10, rectY + 20, buttonWidth, buttonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 4");
  rectY = rectY + 20 + buttonHeight;
}

DisplayScreens updateDisplay(bool connected, DisplayScreens currentScreen) {

  /// Update values
  if (connected) {
    updateTopBar(connected, currentScreen);
    
    if (currentScreen == GPS_SCREEN) {
      updateMainScreenGpsValues();
    }

  } else if (!connected && currentScreen != ERROR_SCREEN) {
    currentScreen = ERROR_SCREEN;
    drawErrorScreen();
  }

  // Write logic for redrawing layout when button is pressed
  // ...

  return currentScreen;
}

void initDisplay(DisplayScreens defaultScreen) {
  tft.init(240, 320);
  tft.setRotation(1); /// To ensure that 0, 0 is in the top left corner...
  tft.invertDisplay(false); /// for some reason the default of the display is to be inverted...
  tft.fillScreen(ST77XX_BLACK);  

  drawTopBar();
  drawRightMenuBar();

  switch(defaultScreen) {
    case GPS_SCREEN:
      drawMainScreenGps();
      currentDisplayScreen = GPS_SCREEN;
      break;

    case SONAR_SCREEN:
      // drawMainScreenSonar();
      currentDisplayScreen = SONAR_SCREEN;
      break;

    default:
      currentDisplayScreen = NONE;
      break;
  }
}

void setup(void) {
  //Serial.begin(9600);
  //Serial.print(F("Hello! ST77xx TFT Test"));

  initDisplay(GPS_SCREEN);
}

void loop() {  
  bool connected = true; // hodnotu by som bral z nejakej get funkcie
  currentDisplayScreen = updateDisplay(connected, currentDisplayScreen);
}
