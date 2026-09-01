#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>

GxEPD2_3C<GxEPD2_750c_GDEY075Z08, GxEPD2_750c_GDEY075Z08::HEIGHT / 4> display(GxEPD2_750c_GDEY075Z08(3, 5, 2, 4));

const int pins[] = {21, 9, 6, 7, 20};
const int numButtons = 5;

int tally[5] = {0, 0, 0, 0, 0};
bool lastPressed[5] = {false, false, false, false, false};

void drawTally() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    for (int i = 0; i < numButtons; i++) {
      display.setCursor(40, 60 + i * 70);
      display.print("Btn ");
      display.print(i + 1);
      display.print(": ");
      display.print(tally[i]);
    }
  } while (display.nextPage());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  display.init(115200, true, 50, false);
  display.setRotation(0);

  for (int i = 0; i < numButtons; i++) {
    pinMode(pins[i], INPUT_PULLUP);
  }

  Serial.println("Drawing initial tally...");
  drawTally();
  Serial.println("Draw complete.");
  Serial.println("Ready.");
}

void loop() {
  bool needsRedraw = false;

  for (int i = 0; i < numButtons; i++) {
    bool pressed = (digitalRead(pins[i]) == LOW);

    if (pressed && !lastPressed[i]) {
      tally[i]++;
      Serial.print("Btn ");
      Serial.print(i + 1);
      Serial.print(" total: ");
      Serial.println(tally[i]);
      needsRedraw = true;
    }

    lastPressed[i] = pressed;
  }

  if (needsRedraw) {
    Serial.println("Drawing now...");
    drawTally();
    Serial.println("Draw complete.");
    delay(200);
  }
}
