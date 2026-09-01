#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// Display driver setup for the 7.5" 3-colour e-paper panel (UC8179 controller).
// Constructor argument order is (CS, DC, RST, BUSY) — confirmed pin mapping:
// CS = GPIO3, DC = GPIO5, RST = GPIO2, BUSY = GPIO4
GxEPD2_3C<GxEPD2_750c_GDEY075Z08, GxEPD2_750c_GDEY075Z08::HEIGHT / 4> display(GxEPD2_750c_GDEY075Z08(3, 5, 2, 4));

// Physical GPIO pins for the 5 tactile buttons, in board layout order.
const int PIN_BUTTON_RIGHT = 21;
const int PIN_BUTTON_LEFT = 9;
const int PIN_BUTTON_MIDDLE = 6;
const int PIN_BUTTON_MID_RIGHT = 7;
const int PIN_BUTTON_MID_LEFT = 20;

// Which physical button does which game function.
// Right/left score points, middle buttons handle reset/undo/serve.
const int BUTTON_PLAYER_A_POINT = 0;
const int BUTTON_PLAYER_B_POINT = 1;
const int BUTTON_RESET_SCORE = 2;
const int BUTTON_UNDO_LAST_POINT = 3;
const int BUTTON_TOGGLE_SERVE = 4;

// Live game state.
int scoreA = 0;
int scoreB = 0;
int lastPlayerToScore = -1;
bool isPlayerAServing = true;

// Set by the button interrupts, cleared once handled in loop().
// volatile because these are written inside an ISR and read in loop().
volatile bool buttonPressPending[5] = {false, false, false, false, false};
volatile unsigned long lastPressTimeMs[5] = {0, 0, 0, 0, 0};
const unsigned long debounceDelayMs = 60;

// Which of the 7 segments (top, top-right, bottom-right, bottom,
// bottom-left, top-left, middle) are lit for each digit 0-9.
const bool sevenSegmentLookup[10][7] = {
  {1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

// Interrupt handler shared by all 5 buttons. Runs the instant a button
// is pressed, even while the display is mid-refresh and loop() is blocked.
// Just records that a press happened; the real handling happens later
// in loop() once the refresh is done.
void IRAM_ATTR onButtonPressed(int buttonIndex) {
  unsigned long now = millis();
  if (now - lastPressTimeMs[buttonIndex] > debounceDelayMs) {
    buttonPressPending[buttonIndex] = true;
    lastPressTimeMs[buttonIndex] = now;
  }
}

// Each pin needs its own tiny wrapper function because attachInterrupt()
// can't take arguments directly.
void IRAM_ATTR isrButtonRight() { onButtonPressed(BUTTON_PLAYER_A_POINT); }
void IRAM_ATTR isrButtonLeft() { onButtonPressed(BUTTON_PLAYER_B_POINT); }
void IRAM_ATTR isrButtonMiddle() { onButtonPressed(BUTTON_RESET_SCORE); }
void IRAM_ATTR isrButtonMidRight() { onButtonPressed(BUTTON_UNDO_LAST_POINT); }
void IRAM_ATTR isrButtonMidLeft() { onButtonPressed(BUTTON_TOGGLE_SERVE); }

// Draws one digit (0-9) as seven filled rectangles, at position (x, y)
// with the given width/height/stroke thickness. Used instead of a text
// font because the built-in fonts look blocky and unclear when scaled
// up this large on e-ink.
void drawSevenSegmentDigit(int digit, int x, int y, int width, int height, int strokeThickness, uint16_t color) {
  int halfHeight = height / 2;

  bool showTop = sevenSegmentLookup[digit][0];
  bool showTopRight = sevenSegmentLookup[digit][1];
  bool showBottomRight = sevenSegmentLookup[digit][2];
  bool showBottom = sevenSegmentLookup[digit][3];
  bool showBottomLeft = sevenSegmentLookup[digit][4];
  bool showTopLeft = sevenSegmentLookup[digit][5];
  bool showMiddle = sevenSegmentLookup[digit][6];

  if (showTop) {
    display.fillRect(x + strokeThickness, y, width - 2 * strokeThickness, strokeThickness, color);
  }
  if (showTopRight) {
    display.fillRect(x + width - strokeThickness, y, strokeThickness, halfHeight, color);
  }
  if (showBottomRight) {
    display.fillRect(x + width - strokeThickness, y + halfHeight, strokeThickness, halfHeight, color);
  }
  if (showBottom) {
    display.fillRect(x + strokeThickness, y + height - strokeThickness, width - 2 * strokeThickness, strokeThickness, color);
  }
  if (showBottomLeft) {
    display.fillRect(x, y + halfHeight, strokeThickness, halfHeight, color);
  }
  if (showTopLeft) {
    display.fillRect(x, y, strokeThickness, halfHeight, color);
  }
  if (showMiddle) {
    display.fillRect(x + strokeThickness, y + halfHeight - strokeThickness / 2, width - 2 * strokeThickness, strokeThickness, color);
  }
}

// Draws a 2-digit score (e.g. "07") centered horizontally on centerX,
// vertically centered on centerY.
void drawTwoDigitScore(int score, int centerX, int centerY) {
  int digitWidth = 70;
  int digitHeight = 160;
  int strokeThickness = 16;
  int gapBetweenDigits = 20;

  int tensDigit = (score / 10) % 10;
  int onesDigit = score % 10;

  int totalWidth = digitWidth * 2 + gapBetweenDigits;
  int startX = centerX - (totalWidth / 2);
  int startY = centerY - (digitHeight / 2);

  drawSevenSegmentDigit(tensDigit, startX, startY, digitWidth, digitHeight, strokeThickness, GxEPD_WHITE);
  drawSevenSegmentDigit(onesDigit, startX + digitWidth + gapBetweenDigits, startY, digitWidth, digitHeight, strokeThickness, GxEPD_WHITE);
}

// Draws the entire screen: black background, both scores, serve
// indicator dot, and status text (e.g. "deuce point").
void drawFullScoreboard() {
  display.fillScreen(GxEPD_BLACK);

  drawTwoDigitScore(scoreA, 200, 240);
  drawTwoDigitScore(scoreB, 600, 240);

  int serveIndicatorX = isPlayerAServing ? 200 : 600;
  display.fillCircle(serveIndicatorX, 380, 10, GxEPD_WHITE);

  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_WHITE);

  const char* statusText = "";
  if (scoreA >= 20 && scoreB >= 20) {
    statusText = "deuce point";
  }

  int16_t boundsX, boundsY;
  uint16_t textWidth, textHeight;
  display.getTextBounds(statusText, 0, 0, &boundsX, &boundsY, &textWidth, &textHeight);
  display.setCursor(400 - (textWidth / 2), 430);
  display.print(statusText);
}

// Runs a full-screen refresh. This is what causes the visible
// black/white flashing — that's the e-paper controller's own clearing
// waveform, not something this code triggers separately.
void refreshDisplay() {
  display.setFullWindow();
  display.firstPage();
  do {
    drawFullScoreboard();
  } while (display.nextPage());
}

// Applies the effect of a single button press to the game state.
// Does not touch the display — that happens once, after all pending
// button presses for this loop iteration have been applied.
void applyButtonPress(int buttonIndex) {
  if (buttonIndex == BUTTON_PLAYER_A_POINT) {
    scoreA++;
    lastPlayerToScore = BUTTON_PLAYER_A_POINT;
    isPlayerAServing = true;
    Serial.print("Player A: ");
    Serial.println(scoreA);

  } else if (buttonIndex == BUTTON_PLAYER_B_POINT) {
    scoreB++;
    lastPlayerToScore = BUTTON_PLAYER_B_POINT;
    isPlayerAServing = false;
    Serial.print("Player B: ");
    Serial.println(scoreB);

  } else if (buttonIndex == BUTTON_RESET_SCORE) {
    scoreA = 0;
    scoreB = 0;
    lastPlayerToScore = -1;
    isPlayerAServing = true;
    Serial.println("Score reset.");

  } else if (buttonIndex == BUTTON_UNDO_LAST_POINT) {
    if (lastPlayerToScore == BUTTON_PLAYER_A_POINT && scoreA > 0) {
      scoreA--;
      Serial.println("Undo Player A point.");
    } else if (lastPlayerToScore == BUTTON_PLAYER_B_POINT && scoreB > 0) {
      scoreB--;
      Serial.println("Undo Player B point.");
    }
    lastPlayerToScore = -1;

  } else if (buttonIndex == BUTTON_TOGGLE_SERVE) {
    isPlayerAServing = !isPlayerAServing;
    Serial.println("Serve toggled.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  display.init(115200, true, 50, false);
  display.setRotation(0);

  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MIDDLE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MID_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MID_LEFT, INPUT_PULLUP);

  // FALLING because buttons are wired with INPUT_PULLUP: the pin reads
  // HIGH when idle and drops to LOW the instant the button is pressed.
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_RIGHT), isrButtonRight, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_LEFT), isrButtonLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_MIDDLE), isrButtonMiddle, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_MID_RIGHT), isrButtonMidRight, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_MID_LEFT), isrButtonMidLeft, FALLING);

  Serial.println("Drawing initial score...");
  refreshDisplay();
  Serial.println("Ready.");
}

void loop() {
  bool anyButtonWasPressed = false;

  // Check all 5 buttons for a pending press flag set by the interrupts.
  // Flags are cleared immediately so each press is only handled once.
  for (int i = 0; i < 5; i++) {
    if (buttonPressPending[i]) {
      buttonPressPending[i] = false;
      applyButtonPress(i);
      anyButtonWasPressed = true;
    }
  }

  if (anyButtonWasPressed) {
    Serial.println("Drawing now...");
    refreshDisplay();
    Serial.println("Draw complete.");
  }
}
