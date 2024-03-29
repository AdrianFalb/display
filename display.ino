#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST        8
#define TFT_DC         9
#define TFT_MOSI      11
#define TFT_SCLK      13

#define DISPLAY_BUTTON_MENU 4
#define DISPLAY_BUTTON_SELECT 3
#define DISPLAY_BUTTON_CONFIRM 2
#define DISPLAY_BUTTON_HOME 5

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

enum DisplayScreens {
    NONE = 0,
    ERROR_SCREEN = -1,
    GPS_SCREEN = 1,
    SONAR_SCREEN = 2
};

struct GpsSelection {
  uint8_t position;
  uint8_t nextPos;
  uint16_t rectX;
  uint16_t rectY;
};

const uint8_t buttonWidth  = 180;
const uint8_t buttonHeight = 30;

DisplayScreens currentDisplayScreen = NONE;
GpsSelection currentGpsSelection = {1, 2, 30, 40};

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  initDisplay(GPS_SCREEN);  
}

void loop() {  
  bool connected = true; // hodnotu by som bral z nejakej get funkcie
  currentDisplayScreen = updateDisplay(connected, currentDisplayScreen);
}

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

void updateTopBarBackground() {
  int rectX = 90;
  int rectY = 0;
  int rectWidth = 100;
  int rectHeight = 15; // Adjust the height of the rectangle as needed
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);
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
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLACK);
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

GpsSelection selectMainScreenGps(GpsSelection selectionState) {

  uint8_t rectX = selectionState.rectX;
  uint8_t rectY = selectionState.rectY;

  if (selectionState.position == 1)
    tft.drawRect(rectX, rectY + 3*20 + 3*buttonHeight, buttonWidth, buttonHeight, ST77XX_WHITE);
  else 
    tft.drawRect(rectX, rectY - 20 - buttonHeight, buttonWidth, buttonHeight, ST77XX_WHITE);  

  /// Highlight 
  tft.drawRect(rectX, rectY, buttonWidth, buttonHeight, ST77XX_YELLOW);

  selectionState.rectX = rectX;
  selectionState.rectY = rectY;

  return selectionState;
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

void drawMainScreenSonar() {
  // Main screen
  int rectX = 0;
  int rectY = 20;
  int rectWidth = tft.width() - 80;
  int rectHeight = tft.height() - 20;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);
  tft.drawRect(rectX, rectY, rectWidth, rectHeight, ST77XX_WHITE);
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

  DisplayScreens screen = currentScreen;

  // Write logic for redrawing layout when button is pressed
  // ...
  uint8_t menuButtonState = digitalRead(DISPLAY_BUTTON_MENU);
  uint8_t GpsSelectionState = digitalRead(DISPLAY_BUTTON_SELECT);
  uint8_t confirmButtonState = digitalRead(DISPLAY_BUTTON_CONFIRM);
  uint8_t homeButtonState = digitalRead(DISPLAY_BUTTON_HOME);

  if (menuButtonState == LOW) {

    if (currentScreen == GPS_SCREEN) {
      currentScreen = SONAR_SCREEN;
      drawMainScreenSonar();      

    } else if (currentScreen == SONAR_SCREEN) {
      currentScreen = GPS_SCREEN;
      drawMainScreenGps();

      currentGpsSelection = selectMainScreenGps(currentGpsSelection);    
      currentGpsSelection.position = 0;
      currentGpsSelection.rectX = 30;
      currentGpsSelection.rectY = 40;
    }
  }

  if (GpsSelectionState == LOW && currentScreen == GPS_SCREEN) {
    
    switch(currentGpsSelection.nextPos) {
      case 1:        
        currentGpsSelection.position = 1;
        currentGpsSelection.rectX = 30;
        currentGpsSelection.rectY = 40;    
        currentGpsSelection = selectMainScreenGps(currentGpsSelection);
        currentGpsSelection.nextPos += 1;
        break;
      case 2:
        currentGpsSelection.position += 1;
        currentGpsSelection.rectY = currentGpsSelection.rectY + 20 + buttonHeight;
        currentGpsSelection = selectMainScreenGps(currentGpsSelection);
        currentGpsSelection.nextPos += 1;
        break;
      case 3:
        currentGpsSelection.position += 1;
        currentGpsSelection.rectY = currentGpsSelection.rectY + 20 + buttonHeight;    
        currentGpsSelection = selectMainScreenGps(currentGpsSelection);
        currentGpsSelection.nextPos += 1;
        break;
      case 4:
        currentGpsSelection.position += 1;
        currentGpsSelection.rectY = currentGpsSelection.rectY + 20 + buttonHeight; 
        currentGpsSelection = selectMainScreenGps(currentGpsSelection);
        currentGpsSelection.nextPos = 1;
        break;
    }    
  }

  if (confirmButtonState == LOW && currentScreen == GPS_SCREEN) {
    tft.drawRect(currentGpsSelection.rectX, currentGpsSelection.rectY, buttonWidth, buttonHeight, ST77XX_GREEN);
    Serial.print("Chosen position: "); Serial.println(currentGpsSelection.position);
  }

  if (homeButtonState == LOW) {
    Serial.println("Going home!");
  }

  /// Update values
  if (connected) {

    if (screen != currentScreen)
      updateTopBarBackground();

    updateTopBar(connected, currentScreen);
    
    if (currentScreen == GPS_SCREEN) {
      updateMainScreenGpsValues();
    }

  } else if (!connected && currentScreen != ERROR_SCREEN) {
    currentScreen = ERROR_SCREEN;
    drawErrorScreen();
  }

  return currentScreen;
}

void initDisplay(DisplayScreens defaultScreen) {
  /// Init Pins
  pinMode(DISPLAY_BUTTON_MENU, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_SELECT, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_CONFIRM, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_HOME, INPUT_PULLUP);
  
  /// Init display
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
      currentGpsSelection = selectMainScreenGps(currentGpsSelection);
      currentGpsSelection.nextPos = 2;
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
