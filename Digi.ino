#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

GxEPD2_3C<GxEPD2_750c_GDEY075Z08, GxEPD2_750c_GDEY075Z08::HEIGHT / 4> display(GxEPD2_750c_GDEY075Z08(3, 5, 2, 4));

const int PIN_BTN0 = 21;
const int PIN_BTN1 = 9;
const int PIN_BTN2 = 6;
const int PIN_BTN3 = 7;
const int PIN_BTN4 = 20;

const int BTN_PLAYER_A = 0;
const int BTN_PLAYER_B = 1;
const int BTN_RESET = 2;
const int BTN_UNDO = 3;
const int BTN_SERVE_TOGGLE = 4;

int scoreA = 0;
int scoreB = 0;
int lastScorer = -1;
bool servingA = true;

volatile bool pendingPress[5] = { false, false, false, false, false };
volatile unsigned long lastInterruptTime[5] = { 0, 0, 0, 0, 0 };
const unsigned long debounceMs = 60;

const bool segmentMap[10][7] = {
  { 1, 1, 1, 1, 1, 1, 0 },
  { 0, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 0, 1, 1, 0, 1 },
  { 1, 1, 1, 1, 0, 0, 1 },
  { 0, 1, 1, 0, 0, 1, 1 },
  { 1, 0, 1, 1, 0, 1, 1 },
  { 1, 0, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 0, 1, 1 }
};

void IRAM_ATTR handlePress(int index) {
  unsigned long now = millis();
  if (now - lastInterruptTime[index] > debounceMs) {
    pendingPress[index] = true;
    lastInterruptTime[index] = now;
  }
}

void IRAM_ATTR isr0() {
  handlePress(0);
}
void IRAM_ATTR isr1() {
  handlePress(1);
}
void IRAM_ATTR isr2() {
  handlePress(2);
}
void IRAM_ATTR isr3() {
  handlePress(3);
}
void IRAM_ATTR isr4() {
  handlePress(4);
}

void drawDigit(int digit, int x, int y, int w, int h, int thickness, uint16_t color) {
  int halfH = h / 2;

  if (segmentMap[digit][0]) {
    display.fillRect(x + thickness, y, w - 2 * thickness, thickness, color);
  }
  if (segmentMap[digit][1]) {
    display.fillRect(x + w - thickness, y, thickness, halfH, color);
  }
  if (segmentMap[digit][2]) {
    display.fillRect(x + w - thickness, y + halfH, thickness, halfH, color);
  }
  if (segmentMap[digit][3]) {
    display.fillRect(x + thickness, y + h - thickness, w - 2 * thickness, thickness, color);
  }
  if (segmentMap[digit][4]) {
    display.fillRect(x, y + halfH, thickness, halfH, color);
  }
  if (segmentMap[digit][5]) {
    display.fillRect(x, y, thickness, halfH, color);
  }
  if (segmentMap[digit][6]) {
    display.fillRect(x + thickness, y + halfH - thickness / 2, w - 2 * thickness, thickness, color);
  }
}

void drawTwoDigitScore(int score, int centerX, int centerY) {
  int digitW = 70;
  int digitH = 160;
  int thickness = 16;
  int gap = 20;

  int tens = (score / 10) % 10;
  int ones = score % 10;

  int totalWidth = digitW * 2 + gap;
  int startX = centerX - (totalWidth / 2);
  int startY = centerY - (digitH / 2);

  drawDigit(tens, startX, startY, digitW, digitH, thickness, GxEPD_WHITE);
  drawDigit(ones, startX + digitW + gap, startY, digitW, digitH, thickness, GxEPD_WHITE);
}

void drawScore() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);

    drawTwoDigitScore(scoreA, 200, 240);
    drawTwoDigitScore(scoreB, 600, 240);

    int serveCX = servingA ? 200 : 600;
    display.fillCircle(serveCX, 380, 10, GxEPD_WHITE);

    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    const char* status = "";
    if (scoreA >= 20 && scoreB >= 20) {
      status = "deuce point";
    }
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(400 - (w / 2), 430);
    display.print(status);

  } while (display.nextPage());
}

void handleButtonAction(int index) {
  if (index == BTN_PLAYER_A) {
    scoreA++;
    lastScorer = BTN_PLAYER_A;
    servingA = true;
    Serial.print("Player A: ");
    Serial.println(scoreA);
  } else if (index == BTN_PLAYER_B) {
    scoreB++;
    lastScorer = BTN_PLAYER_B;
    servingA = false;
    Serial.print("Player B: ");
    Serial.println(scoreB);
  } else if (index == BTN_RESET) {
    scoreA = 0;
    scoreB = 0;
    lastScorer = -1;
    servingA = true;
    Serial.println("Score reset.");
  } else if (index == BTN_UNDO) {
    if (lastScorer == BTN_PLAYER_A && scoreA > 0) {
      scoreA--;
      Serial.println("Undo Player A point.");
    } else if (lastScorer == BTN_PLAYER_B && scoreB > 0) {
      scoreB--;
      Serial.println("Undo Player B point.");
    }
    lastScorer = -1;
  } else if (index == BTN_SERVE_TOGGLE) {
    servingA = !servingA;
    Serial.println("Serve toggled.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  display.init(115200, true, 50, false);
  display.setRotation(0);

  pinMode(PIN_BTN0, INPUT_PULLUP);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_BTN3, INPUT_PULLUP);
  pinMode(PIN_BTN4, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_BTN0), isr0, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN1), isr1, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN2), isr2, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN3), isr3, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN4), isr4, FALLING);

  Serial.println("Drawing initial score...");
  drawScore();
  Serial.println("Ready.");
}

void loop() {
  bool needsRedraw = false;

  for (int i = 0; i < 5; i++) {
    if (pendingPress[i]) {
      pendingPress[i] = false;
      handleButtonAction(i);
      needsRedraw = true;
    }
  }

  if (needsRedraw) {
    Serial.println("Drawing now...");
    drawScore();
    Serial.println("Draw complete.");
  }
}