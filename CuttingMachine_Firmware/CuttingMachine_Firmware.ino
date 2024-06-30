#include <Wire.h>

#include <Adafruit_GFX.h> // Library for GFX
#include <Adafruit_SSD1306.h> // Library for the OLED Screen
#include <avr/wdt.h> // Library for Watchdog
#include <avr/sleep.h> // Library to Sleep

#include "Assets.h" // Library of Images and Bitmaps
#include "Actions.h" // Library of most machine actions

#define version "1.0.5" // Several Updates
#define cuttype "RES" // RES or LED set for Different Types

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET - 1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, & Wire, OLED_RESET);

// Define buttons
#define BUTTON_UP A0
#define BUTTON_DOWN A1
#define BUTTON_BACK A2
#define BUTTON_SELECT A3
// Define Sensors
#define SENSOR_TAPE 13
#define SENSOR_UNIT 12

#define CUT_STEP 2 // Cutter Motor Stepper Pin
#define CUT_DIR 5 // Cutter Motor Direction Pin
#define FEED_STEP 3 // Feeder Motor Stepper Pin
#define FEED_DIR 6 // Feeder Motor Direction Pin
#define ENABLE 8 // Motor Enable Pin

int menuIndex = 0;
int submenuIndex = 0;
bool inSubMenu = false;
unsigned long lastActivityTime = 0;
const unsigned long inactivityTimeout = 600000;  // 10 Minutes in milliseconds

// Default Variable Settings
int setSize = 1; // Size of the units |*|*|*|*|*|
int setUnits = 1; // How many pieces get cut by default
int setInterval = 4; // Tape Interval in millimeters

// Debounce settings
const unsigned long debounceDelay = 300; // milliseconds
unsigned long lastDebounceTime = 10;

void setup() {
  // Setup Buttons used for Navigation (Up, Down, Back, Select)
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  // Setup PIN for Tape Presence Sensor
  pinMode(SENSOR_TAPE, INPUT);
  // Setup PINs for Motor Controls for Feed and Cut
  pinMode(CUT_STEP, OUTPUT);
  pinMode(CUT_DIR, OUTPUT);
  pinMode(FEED_STEP, OUTPUT);
  pinMode(FEED_DIR, OUTPUT);
  // Setup PIN for enableing Motors
  pinMode(ENABLE, OUTPUT);
  // Disable Motors until we enter Main Menu HIGH == DISABLED
  digitalWrite(ENABLE, HIGH);

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  // Display Firmware and Logo
  FirmwareDisplay();      //Display Firmware Version
  LogoDisplay();          //Display Logo Bootscreen
}

  // Define Frame Control Variable for graphics
  int frame=0;

void loop() {
  // Watch for SELECT Button to enter the Menu
  if (readButton(BUTTON_SELECT) == true) {
    lastActivityTime = millis();
    navigateMenu();
  }
  // Disable Motors if the UP Button was pressed in Idle Screen
  if (readButton(BUTTON_UP) == true) { 
    digitalWrite(ENABLE, HIGH); 
    lastActivityTime = millis();
  }
  // Watch for inactivity and enter sleep mode
  if (millis() - lastActivityTime > inactivityTimeout) {
    enterSleepMode();
  }
  // Display the Idle Screen Details
  display.clearDisplay();
  display.drawBitmap(78, 0, frames[frame], FRAME_WIDTH, FRAME_HEIGHT, 1);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println(F("IDLE"));
  display.setTextSize(1);
  display.print(F("Size: "));
  display.println(setSize);
  display.print(F("Units: "));
  display.println(setUnits);
  // Show the state of the Tape Sensor
  if (digitalRead(SENSOR_TAPE) == LOW) {
    display.println(F("Sensor: LOW"));
  } else {
    display.println(F("Sensor: HIGH"));
  }
  // Show the state of Unit Sensor
  if (digitalRead(SENSOR_UNIT) == LOW) {
    display.println(F("Sensor: LOW"));
  } else {
    display.println(F("Sensor: HIGH"));
  }
  // Show the state of the Motors
  if (digitalRead(ENABLE) == LOW) {
    display.println(F("Motors: ENABLED"));
  } else {
    display.println(F("Motors: DISABLED"));
  }
  display.println(F("Press SELECT to Start"));
  display.display();
  frame = (frame + 1) % FRAME_COUNT;
  delay(FRAME_DELAY);
}


// Process to handle Button Press Bounces (Smooth UI Handling)
bool readButton(int buttonPin) {
  if (digitalRead(buttonPin) == LOW) {
    // Do no detect the press unless its longer than debounceDelay value
    if (millis() - lastDebounceTime > debounceDelay) {
      lastDebounceTime = millis();
      return true;
    }
  }
  return false;
}

// Firmware Screen display
void FirmwareDisplay() { display.clearDisplay(); display.setTextColor(WHITE); display.setTextSize(1); display.print(F("Firmware: ")); display.println(F(version)); display.display(); delay(1000); }
// Logo Boot display
void LogoDisplay() { display.clearDisplay(); display.drawBitmap(0, 0, SCLOGO, 128, 64, SSD1306_WHITE); display.display(); delay(1500);}

// Main Menu Navigation Routine
void navigateMenu() {
  // Enable Motors if the Menu is entered (sticky)
  digitalWrite(ENABLE, LOW);
  while (true) {
    displayMenu();
    if (readButton(BUTTON_SELECT) == true) {
      if (inSubMenu) {
        executeSubmenuOption();
      } else {
        enterSubMenu();
      }
    } else if (readButton(BUTTON_BACK) == true) {
      if (inSubMenu) {
        inSubMenu = false;
        submenuIndex = 0;
      } else {
        //displayIdleScreen();
        return;
      }
    } else if (readButton(BUTTON_UP) == true) {
      if (inSubMenu) {
        submenuIndex = (submenuIndex > 0) ? submenuIndex - 1 : getMaxSubMenuIndex();
      } else {
        menuIndex = (menuIndex > 0) ? menuIndex - 1 : 4; // Adjust this for Menu Number
      }
    } else if (readButton(BUTTON_DOWN) == true) {
      if (inSubMenu) {
        submenuIndex = (submenuIndex < getMaxSubMenuIndex()) ? submenuIndex + 1 : 0;
      } else {
        menuIndex = (menuIndex < 4) ? menuIndex + 1 : 0; // Adjust this for Menu Number
      }
    }
  }
}

void displayMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  if (!inSubMenu) {
    display.setTextSize(2);
    display.println(F("MAIN MENU"));
    display.setTextSize(1);
    if (menuIndex == 0) display.println(F("> CUT!"));
    else display.println(F("  Cut!"));
    if (menuIndex == 1) display.println(F("> SETTINGS"));
    else display.println(F("  Settings"));
    if (menuIndex == 2) display.println(F("> CALIBRATE"));
    else display.println(F("  Calibrate"));
    if (menuIndex == 3) display.println(F("> LOAD TAPE"));
    else display.println(F("  Load Tape"));
    if (menuIndex == 4) display.println(F("> RESET"));
    else display.println(F("  Reset"));
  } else {
    if (menuIndex == 0) {
      display.setTextSize(2);
      display.println(F("START CUT!"));
      display.setTextSize(1);
      if (submenuIndex == 0) {
        display.setTextSize(3);
        display.println(F("> GO!"));
      } else {
        display.setTextSize(3);
        display.println(F("  Go"));
      }
      if (submenuIndex == 1) {
        display.setTextSize(1);
        display.println(F("> TEST CUT"));
      } else {
        display.setTextSize(1);
        display.println(F("  Test Cut"));
      }
    }
    if (menuIndex == 1) {
      display.setTextSize(2);
      display.println(F("SETTINGS"));
      display.setTextSize(1);
      if (submenuIndex == 0) { 
        display.print(F("> Set Size: "));
        display.println(setSize);
      } else { 
        display.print(F("  Set Size: "));
        display.println(setSize);
      }
      if (submenuIndex == 1) {
        display.print(F("> Set Units: "));
        display.println(setUnits);
      } else {
        display.print(F("  Set Units: "));
        display.println(setUnits);
      }
    }
    if (menuIndex == 2) {
      display.setTextSize(2);
      display.println(F("CALIBRATE"));
      display.setTextSize(1);
      if (submenuIndex == 0) display.println(F("> ADJUST FWD"));
      else display.println(F("  Adjust Fwd"));
      if (submenuIndex == 1) display.println(F("> ADJUST REV"));
      else display.println(F("  Adjust Rev"));
    }
    if (menuIndex ==3) {
      display.setTextSize(2);
      display.println(F("LOAD TAPE"));
      display.setTextSize(1);
      if (submenuIndex == 0) display.println(F("> LOAD TAPE"));
      else display.println(F("  Load Tape"));
      if (submenuIndex == 1) display.println(F("> UNLOAD TAPE"));
      else display.println(F("  Unload Tape"));
    }
      if (menuIndex ==4) {
      display.setTextSize(2);
      display.println("RESET?");
      display.setTextSize(1);
      if (submenuIndex == 0) display.println("> NO");
      else display.println("  No");
      if (submenuIndex == 1) display.println("> YES");
      else display.println("  Yes");
    }
  }
  display.display();
}

void enterSubMenu() {
  inSubMenu = true;
  submenuIndex = 0;
}

int getMaxSubMenuIndex() {
  switch (menuIndex) {
  case 0:
    return 1; // CUT! menu has 2 options: Go and Test Cut
  case 1:
    return 1; // Settings menu has 2 options: Set Size and Set Units
  case 2:
    return 1; // Calibrate menu has 2 options: Adjust FWD and Adjust REV
  case 3:
    return 1; // LoadTape Menu has 2 options
  case 4:
    return 1; // Reset menu has 2 options
  default:
    return 0;
  }
}

void executeSubmenuOption() {
  switch (menuIndex) {
  case 0: // CUT!
    if (submenuIndex == 0) runCutProgram();
    else if (submenuIndex == 1) runCutTest();
    break;
  case 1: // Settings
    if (submenuIndex == 0) changeSize();
    else if (submenuIndex == 1) changeUnits();
    break;
  case 2: // Calibrate
    if (submenuIndex == 0) runAdjustFWD();
    else if (submenuIndex == 1) runAdjustREV();
    break;
  case 3: // LoadTape Menu
    if (submenuIndex == 0) loadTape();
    else if (submenuIndex == 1) unloadTape();
    break;
  case 4: // Reset Menu
    if (submenuIndex == 0) resetNo();
    else if (submenuIndex == 1) resetYes();
    break;
  }
}

/// Sleep Mode Process
void enterSleepMode() {
  digitalWrite(ENABLE, HIGH);  // Disable Motors so they last longer
  lastActivityTime = millis();
  inSubMenu=false;
}

void resetNo()  { inSubMenu = false; }
void resetYes() {
  display.clearDisplay(); display.setCursor(0, 0); display.setTextSize(2); display.println(F("RESETTING!")); display.display();
  delay(2000);
  // Reboot Device
  wdt_enable(WDTO_15MS);
  while(1);
}

// Main Cutting Program
void runCutProgram() {
  display.clearDisplay();
  // Main process for Cutting
  for (int rp = 0; rp < setUnits; rp++) {
    // Display Cut Status (Units Cut)
    display.setCursor(0, 0); display.setTextSize(2); display.println("CUTTING"); display.setTextSize(3); display.setCursor(0, 24); display.print(" = "); display.println(rp + 1); display.display();
    // Sensor Check for Tape Existence (Stop and Display Error)
    if (digitalRead(SENSOR_TAPE) == LOW) {
      // Display Error Message
      display.clearDisplay(); display.setCursor(4, 7); display.setTextSize(2); display.println("!STOPPING!"); display.setTextSize(1); display.println("\n    Load or Check\n   component tape."); display.display();
      delay(500);
      // Blink the Display
      for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
          display.invertDisplay(true);
        } else {
          display.invertDisplay(false);
        }
        display.display();
        delay(500);
      }
      // Stop and Return if the Sensor is tripped
      return;
    }
    FEED(setSize);  // Feed the Tape based on the Size Selected
    CUT();  // Cut the tape after the feed is complete
    lastActivityTime = millis();  // Keep from entering sleepMode while cutting
  }
  // Clear the cut head
  ClearFeeder(); // Clears the Feed Head
  delay(200);
  inSubMenu = false;
}

// This will cut one component and make one unit
// This is just for testing
void runCutTest() {
  FEED(1);
  CUT();
  ClearFeeder();
  delay(500);
  inSubMenu = false;
}

void ClearFeeder() {
  int ClearCount = 10;
  digitalWrite(FEED_DIR, LOW); // Set Forward
  for (int y = 0; y < ClearCount; y++) {
    digitalWrite(FEED_STEP, HIGH);
    delayMicroseconds(1700); // Fast Rotation
    digitalWrite(FEED_STEP, LOW);
    delayMicroseconds(1700); // Fast Rotation
  }
  digitalWrite(FEED_DIR, HIGH); // Set Reverse
  for (int y = 0; y < ClearCount; y++) {
    digitalWrite(FEED_STEP, HIGH);
    delayMicroseconds(1700); // Fast Rotation
    digitalWrite(FEED_STEP, LOW);
    delayMicroseconds(1700); // Fast Rotation
  }
  digitalWrite(FEED_DIR, LOW);
}

// Main CUT Process (One Full Rotation)
void CUT() {
  int cutsteps = 800;
  for (int x = 0; x < cutsteps; x++) {
    digitalWrite(CUT_STEP, HIGH);
    delayMicroseconds(90); // Fast Rotation
    digitalWrite(CUT_STEP, LOW);
    delayMicroseconds(90); // Fast Rotation
    display.clearDisplay();
  }
  delay(100);
}

// Main FEED Process
void FEED(int feedcount) {
  int feedsteps = feedcount * 10;
  digitalWrite(FEED_DIR, LOW); // Set Feed Forward for Feed
  for (int y = 0; y < feedsteps; y++) {
    digitalWrite(FEED_STEP, HIGH);
    delayMicroseconds(700); // Fast Rotation
    digitalWrite(FEED_STEP, LOW);
    delayMicroseconds(700); // Fast Rotation
  }
  delay(100);
}

// Future Tape Loading
void loadTape() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("LOAD Function not\nimplemented"));
  display.display();
  delay(2000);
}
// Future Tape Unloading
void unloadTape() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("UNLOAD Function not\nimplemented"));
  display.display();
  delay(2000);
}

void changeSize() {
  while (true) {
    if (readButton(BUTTON_UP) == true && setSize < 1000) setSize++;
    else if (readButton(BUTTON_DOWN) == true && setSize > 1) setSize--;
    //delay(150);
    if (readButton(BUTTON_SELECT) == true) break;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("SET SIZE\n");
    display.setCursor(4, 17);
    display.setTextSize(3);
    display.println(setSize);
    // Display the size of the unit
    if (setSize >= 1) { display.drawBitmap(0, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 2) { display.drawBitmap(7, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 3) { display.drawBitmap(14, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 4) { display.drawBitmap(21, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 5) { display.drawBitmap(28, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 6) { display.drawBitmap(35, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 7) { display.drawBitmap(42, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 8) { display.drawBitmap(49, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 9) { display.drawBitmap(56, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 10) { display.drawBitmap(63, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 11) { display.drawBitmap(70, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 12) { display.drawBitmap(77, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 13) { display.drawBitmap(84, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 14) { display.drawBitmap(91, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 15) { display.drawBitmap(98, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 16) { display.drawBitmap(105, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 17) { display.drawBitmap(112, 51, RES, 7, 14, SSD1306_WHITE); }
    if (setSize >= 18) { display.drawBitmap(119, 51, RES, 7, 14, SSD1306_WHITE); }
    display.display();
  }
}

void changeUnits() {
  while (true) {
    if (readButton(BUTTON_UP) == true && setUnits < 1000) setUnits++;
    else if (readButton(BUTTON_DOWN) == true && setUnits > 1) setUnits--;
    //delay(100);
    if (readButton(BUTTON_SELECT) == true) break;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("SET UNITS\n");
    display.setCursor(4, 17);
    display.setTextSize(3);
    display.println(setUnits);
    display.display();
  }
}

void runAdjustFWD() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("ADJUSTING");
  display.setTextSize(3);
  display.println(">>> FWD");
  display.display();
  digitalWrite(FEED_DIR, LOW);
  digitalWrite(FEED_STEP, HIGH);
  delayMicroseconds(1000);
  digitalWrite(FEED_STEP, LOW);
  delay(150);
  displayMenu();
}

void runAdjustREV() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("ADJUSTING");
  display.setTextSize(3);
  display.println("<<< REV");
  display.display();
  digitalWrite(FEED_DIR, HIGH);
  digitalWrite(FEED_STEP, HIGH);
  delayMicroseconds(1000);
  digitalWrite(FEED_STEP, LOW);
  delay(150);
  displayMenu();
}