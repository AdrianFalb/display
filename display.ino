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

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

enum DisplayScreens {
    NONE = 0,
    ERROR_SCREEN = -1,
    GPS_SCREEN = 1,
    SONAR_SCREEN = 2
};

struct DisplayGpsSelectionData {
  uint8_t position;
  uint8_t nextPos;
  uint16_t rectX;
  uint16_t rectY;
};

struct DisplayState {
  DisplayScreens currentScreen;
  bool menuButtonPressed;
  bool selectButtonPressed;
  bool confirmButtonPressed;
  bool homeButtonPressed;
};

const uint8_t displayButtonWidth  = 180;
const uint8_t displayButtonHeight = 30;

// Circular buffer to hold lake floor data
uint16_t displayLakeFloorBuffer[240];

struct DisplayGpsSelectionData *currentGpsSelection;
struct DisplayState *displayState;

void setup(void) {
  Serial.begin(9600);
  initDisplay(SONAR_SCREEN);
}

void loop() {  
  bool connected = true; // hodnotu by som bral z nejakej get funkcie
  updateDisplay(connected, displayState);
}

/// Function to shift lake floor buffer to the left by one pixel
void displayShiftLakeFloorBuffer(int amount) {
    for (int i = amount; i < tft.width() - 80; i++) {
        displayLakeFloorBuffer[i - amount] = displayLakeFloorBuffer[i];
    }
    // Fill the shifted portion with default values (bottom of the screen)
    for (int i = tft.width() - 80 - amount; i < tft.width() - 80; i++) {
        displayLakeFloorBuffer[i] = tft.height();
    }
}

void drawFishFinder(float sonarData) {

    // Calculate point position based on sonar data
    const float maxDistance = 600.0;  // Maximum distance (adjust based on your sonar)
    const uint8_t SCROLL_AMOUNT = 4;
    const uint8_t LAKE_FLOOR_HEIGHT = 20;
    const uint8_t TOP_BAR_MARGIN = 40;

    // Map sonar data to lake floor Y coordinate within the drawing area
    uint16_t lakeFloorY = map(sonarData, 0, 600, tft.height(), TOP_BAR_MARGIN);

    // Shift lake floor buffer to the left by one pixel
    displayShiftLakeFloorBuffer(SCROLL_AMOUNT);

    // Update the newest lake floor Y coordinate at the rightmost position
    displayLakeFloorBuffer[tft.width() - 80 - 1] = lakeFloorY;

    // Clear display and redraw the lake floor
    drawMainScreenBackground(ST77XX_BLACK);

    // Draw lake floor lines with specified width (SCROLL_AMOUNT)
    for (int x = 0; x < tft.width() - 80 - SCROLL_AMOUNT; x++) {
        int yStart = displayLakeFloorBuffer[x];
        int yEnd = tft.height() - 1;  // Draw lines from lake floor to the bottom of the screen

        // Calculate line start and end points
        int startY = min(yStart, yEnd);
        int endY = max(yStart, yEnd);

        // Draw a horizontal line with specified width (SCROLL_AMOUNT) from startY to endY at position x
        tft.drawFastHLine(x, startY, SCROLL_AMOUNT, ST77XX_RED);

        // Ensure no overlap by drawing additional lines if needed
        while (yEnd > startY) {
            startY++;
            tft.drawFastHLine(x, startY, SCROLL_AMOUNT, ST77XX_RED);
        }
    }
}

void drawBattery(uint16_t batteryWidth, uint16_t batteryHeight, uint16_t batteryMargin, uint16_t batteryX, uint16_t batteryY, uint8_t batteryCharge) {
  /// Battery colors
  const uint16_t BATTERY_BODY_COLOR = ST77XX_WHITE;
  const uint16_t BATTERY_FILL_COLOR = ST77XX_GREEN;
  const uint16_t BATTERY_BORDER_COLOR = ST77XX_BLACK;

  /// Draw battery body
  tft.fillRect(batteryX, batteryY, batteryWidth, batteryHeight, BATTERY_BODY_COLOR);

  /// Draw battery fill level
  uint16_t fillWidth = map(batteryCharge, 0, 100, 0, batteryWidth - 2 * batteryMargin);

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
  uint8_t rectX = 90;
  uint8_t rectY = 0;
  uint8_t rectWidth = 100;
  uint8_t rectHeight = 15;
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
}

void drawTopBar() {
  int rectX = 0;
  int rectY = 0;
  int rectWidth = tft.width();
  int rectHeight = 20;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_RED);

  /// Boat Battery
  tft.setCursor(tft.width() - 56, 2);
  tft.print("Boat: ");
  tft.println();
  
  /// Connection status
  tft.print("Connection: ");

  /// Joystick battery
  tft.setCursor(tft.width() - 80, 10);
  tft.print("Joystick: "); 
}

void drawRightMenuBar() {
  /// Menu bar
  uint16_t rectX = tft.width() - 80;
  uint16_t rectY = 20;
  uint16_t rectWidth = tft.width();
  uint16_t rectHeight = tft.height();
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, ST77XX_BLUE);

  /// Buttons with text
  uint8_t buttonWidth  = 60;
  uint8_t buttonHeight = 30;

  /// Menu button
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
  rectY = rectY + 20 + buttonHeight;  
}

void drawMainScreenBackground(uint16_t backgroundColor) {  
  /// Main screen
  uint16_t rectX = 0;
  uint16_t rectY = 20;
  uint16_t rectWidth = tft.width() - 80;
  uint16_t rectHeight = tft.height() - 20;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, backgroundColor);
  tft.drawRect(rectX, rectY, rectWidth, rectHeight, ST77XX_WHITE);
}

void drawErrorScreen() {
  uint16_t rectX = 0;
  uint16_t rectY = 0;
  uint16_t rectWidth = tft.width();
  uint16_t rectHeight = tft.height();
  
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

void selectMainScreenGps(struct DisplayGpsSelectionData *selectionState) {

  uint8_t rectX = selectionState->rectX;
  uint8_t rectY = selectionState->rectY;

  if (selectionState->position == 1)
    tft.drawRect(rectX, rectY + 3*20 + 3*displayButtonHeight, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);
  else 
    tft.drawRect(rectX, rectY - 20 - displayButtonHeight, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);  

  /// Highlight 
  tft.drawRect(rectX, rectY, displayButtonWidth, displayButtonHeight, ST77XX_YELLOW);

  selectionState->rectX = rectX;
  selectionState->rectY = rectY;
}

void updateMainScreenGpsValues() {
  const int CURSOR_NEW_LINE = 10;

  uint16_t rectX = 20;
  uint16_t rectY = 20;
  uint16_t rectWidth = tft.width(); - 80;
  uint16_t rectHeight = tft.height();
   
  tft.setTextSize(1); 
  tft.setTextColor(ST77XX_WHITE);

  /// GPS Position 1
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  float latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  float longitude = 73.562;
  tft.print(longitude); // tuto by som bral hodnotu z nejakej get funkcie
  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 2
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = 73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);
  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 3
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = -73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);

  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 4
  tft.setCursor(rectX + 14, rectY + 24 + CURSOR_NEW_LINE);  

  tft.print("Lat: ");
  latitude = 40.780; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(latitude);

  tft.print(", Long: ");
  longitude = -73.562; // tuto by som bral hodnotu z nejakej get funkcie
  tft.print(longitude);

  rectY = rectY + 20 + displayButtonHeight;
}

void updateMainScreenSonarValues() {
  // Random value between 0 and 600 (replace with real data)
  float sonarData = random(0, 600);

  // Draw fish finder point based on sonar data
  drawFishFinder(sonarData);

  tft.setCursor(4, tft.height() - 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  const int CURSOR_NEW_LINE = 10;
  uint16_t rectX = 20;
  uint16_t rectY = 40;
  uint16_t rectWidth = tft.width(); - 80;
  uint16_t rectHeight = tft.height() - 20;

  tft.drawLine(rectX, rectY, rectX, rectHeight, ST77XX_WHITE);

  uint16_t height = 600;
  tft.print(height);
  tft.print(" cm");

  uint16_t numberOfFish = 12;
  tft.setCursor(tft.width() - 120, 24);
  tft.print("Fish: "); 
  tft.setCursor(tft.width() - 120, 32);
  tft.print(numberOfFish);
}

void drawMainScreenSonar() {
  drawMainScreenBackground(ST77XX_BLUE);  
}

void drawMainScreenGps() {
  drawMainScreenBackground(ST77XX_BLACK);
  
  const int CURSOR_NEW_LINE = 10;
  uint16_t rectX = 20;
  uint16_t rectY = 20;
  uint16_t rectWidth = tft.width(); - 80;
  uint16_t rectHeight = tft.height();

  /// GPS Position 1
  tft.drawRect(rectX + 10, rectY + 20, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 1");
  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 2
  tft.drawRect(rectX + 10, rectY + 20, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 2");
  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 3
  tft.drawRect(rectX + 10, rectY + 20, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 3");
  rectY = rectY + 20 + displayButtonHeight;

  /// GPS Position 4
  tft.drawRect(rectX + 10, rectY + 20, displayButtonWidth, displayButtonHeight, ST77XX_WHITE);
  tft.setCursor(rectX + 14, rectY + 24);
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  tft.print("GPS POSITION 4");
  rectY = rectY + 20 + displayButtonHeight;
}

void updateDisplay(bool connected, struct DisplayState *state) {

  DisplayScreens screen = state->currentScreen;

  /// Screen switching logic with buttons ================
  uint8_t menuButtonState = digitalRead(DISPLAY_BUTTON_MENU);
  uint8_t selectionButtonState = digitalRead(DISPLAY_BUTTON_SELECT);
  uint8_t confirmButtonState = digitalRead(DISPLAY_BUTTON_CONFIRM);
  uint8_t homeButtonState = digitalRead(DISPLAY_BUTTON_HOME);

  if (menuButtonState == LOW && !state->menuButtonPressed) {

    if (state->currentScreen == GPS_SCREEN) {
      state->currentScreen = SONAR_SCREEN;
      drawMainScreenSonar();

    } else if (state->currentScreen == SONAR_SCREEN) {
      state->currentScreen = GPS_SCREEN;
      drawMainScreenGps();

      selectMainScreenGps(currentGpsSelection);
      currentGpsSelection->position = 1;
      currentGpsSelection->nextPos = 2;
      currentGpsSelection->rectX = 30;
      currentGpsSelection->rectY = 40;
    }
    state->menuButtonPressed = true;  
  } else if (menuButtonState == HIGH) {    
    state->menuButtonPressed = false;
  } 

  if (selectionButtonState == LOW && state->currentScreen == GPS_SCREEN && !state->selectButtonPressed) {
    
    switch(currentGpsSelection->nextPos) {
      case 1:        
        currentGpsSelection->position = 1;
        currentGpsSelection->rectX = 30;
        currentGpsSelection->rectY = 40;    
        selectMainScreenGps(currentGpsSelection);
        currentGpsSelection->nextPos += 1;
        break;
      case 2:
        currentGpsSelection->position += 1;
        currentGpsSelection->rectY = currentGpsSelection->rectY + 20 + displayButtonHeight;
        selectMainScreenGps(currentGpsSelection);
        currentGpsSelection->nextPos += 1;
        break;
      case 3:
        currentGpsSelection->position += 1;
        currentGpsSelection->rectY = currentGpsSelection->rectY + 20 + displayButtonHeight;    
        selectMainScreenGps(currentGpsSelection);
        currentGpsSelection->nextPos += 1;
        break;
      case 4:
        currentGpsSelection->position += 1;
        currentGpsSelection->rectY = currentGpsSelection->rectY + 20 + displayButtonHeight; 
        selectMainScreenGps(currentGpsSelection);
        currentGpsSelection->nextPos = 1;
        break;
    }
    state->selectButtonPressed = true;
  } else if (selectionButtonState == HIGH) {    
    state->selectButtonPressed = false;   
  }

  if (confirmButtonState == LOW && state->currentScreen == GPS_SCREEN && !state->confirmButtonPressed) {
    tft.drawRect(currentGpsSelection->rectX, currentGpsSelection->rectY, displayButtonWidth, displayButtonHeight, ST77XX_GREEN);
    Serial.print("Chosen position: "); Serial.println(currentGpsSelection->position);

    state->confirmButtonPressed = true;
  } else if (confirmButtonState == HIGH) {
    state->confirmButtonPressed = false;
  }

  if (homeButtonState == LOW && !state->homeButtonPressed) {
    Serial.println("Going home!");
    state->homeButtonPressed = true;

  } else if (homeButtonState == HIGH) {
    state->homeButtonPressed = false;
  }

  /// Update values ================
  if (connected) {

    if (screen != state->currentScreen)
      updateTopBarBackground();

    updateTopBar(connected, state->currentScreen);
    
    if (state->currentScreen == GPS_SCREEN) {
      updateMainScreenGpsValues();
    } else if (state->currentScreen == SONAR_SCREEN) {
      updateMainScreenSonarValues();      
    }

  } else if (!connected && state->currentScreen != ERROR_SCREEN) {
    state->currentScreen = ERROR_SCREEN;
    drawErrorScreen();
  } 
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

  /// Init structs
  displayState = new DisplayState;
  if (displayState == nullptr) {
      // Memory allocation failed
      // Handle error
  } else {
      // Memory allocation successful
      displayState->currentScreen = NONE;
      displayState->menuButtonPressed = false;
      displayState->selectButtonPressed = false;
      displayState->confirmButtonPressed = false;
      displayState->homeButtonPressed = false;
  }

  currentGpsSelection = new DisplayGpsSelectionData;
  if (currentGpsSelection == nullptr) {
      // Memory allocation failed
      // Handle error
  } else {
      // Memory allocation successful
      currentGpsSelection->position = 1;
      currentGpsSelection->nextPos = 2;
      currentGpsSelection->rectX = 30;
      currentGpsSelection->rectY = 40;
  }  

  /// Initialize lake floor buffer with default values (bottom of the screen)
  for (int i = 0; i < tft.width() - 80; i++) {
    displayLakeFloorBuffer[i] = tft.height();
  }

  switch(defaultScreen) {
    case GPS_SCREEN:
      drawMainScreenGps();
      displayState->currentScreen = GPS_SCREEN;
      selectMainScreenGps(currentGpsSelection);
      currentGpsSelection->nextPos = 2;
      break;

    case SONAR_SCREEN:
      drawMainScreenSonar();
      displayState->currentScreen = SONAR_SCREEN;
      selectMainScreenGps(currentGpsSelection);
      currentGpsSelection->nextPos = 2;
      break;

    default:
      displayState->currentScreen = NONE;
      break;
  }
}
