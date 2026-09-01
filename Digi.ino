#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold24pt7b.h>

GxEPD2_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display(GxEPD2_750_GDEY075T7(2, 3, 4, 5));

void setup() {
  Serial.begin(115200);
  delay(1000);

  display.init(115200, true, 50, false);
  display.setRotation(0);

  Serial.println("Clearing display...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  delay(500);

  Serial.println("Drawing scoreboard...");
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(30, 100);
    display.print("A: 0");
    display.setCursor(30, 200);
    display.print("B: 0");
  } while (display.nextPage());

  Serial.println("Done.");
}

void loop() {
}
