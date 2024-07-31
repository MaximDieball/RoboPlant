#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define MOISTURE_SENSOR_PIN A2
#define LIGHT_PIN A4

#define BUTTON_ONE 8
#define BUTTON_TWO 11

#define DATA_LENGTH 50

#define BUZZER_PIN 4

bool buttonOne = false;
bool buttonTwo = false;

int lightData[DATA_LENGTH];
int lightLvl = 0;  // -1 Schlafen
int moistData[DATA_LENGTH];
int moistLvl = 0;  // -1 Zu wenig Wasser 1 viel Wasser
int scene = 0;

int animationY = SCREEN_HEIGHT;  // Start position for animation
long animationLastUpdate = 0;    // Time when the animation was last updated

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);

  pinMode(BUTTON_ONE, INPUT);
  pinMode(BUTTON_TWO, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialisierung des Displays
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  // Erzeuge eine Liste von DATA_LENGTH Elementen
  for (int i = 0; i < DATA_LENGTH; i++) {
    lightData[i] = 0;
    moistData[i] = 0;
  }
}

// last speichert die letzte laufzeit
long last = 0;

void loop() {
  // erfassen der daten
  int lightDataD = int(analogRead(LIGHT_PIN));
  int moistDataD = int(analogRead(MOISTURE_SENSOR_PIN));

  //button logik
  if (digitalRead(BUTTON_ONE)) {
    if (!buttonOne) {
      buttonOne = true;
      if (scene == 0) {
        scene = 2;
      } else {
        scene = 0;
      }
      Serial.println("Moisture");
    }
  } else {
    buttonOne = false;
  }

  // button logik
  if (digitalRead(BUTTON_TWO)) {
    if (!buttonTwo) {
      buttonTwo = true;
      if (scene == 1) {
        scene = 2;
      } else {
        scene = 1;
      }
      Serial.println("Light");
    }
  } else {
    buttonTwo = false;
  }

  // laufzeit nehnen
  if (millis() - last > 10) {
    last = millis();
    // daten speichern 
    addLightData(lightDataD);
    addMoistData(moistDataD);
    // turn off buzzer
    digitalWrite(BUZZER_PIN, LOW);
    // abhängig von der scene anzeigen von date oder gesicht
    switch (scene) {
      case 0:
        drawGraph(lightData);
        break;
      case 1:
        drawGraph(moistData);
        break;
      case 2:
        drawFace();
        break;
      default:
        break;
    }
  }

  // nehmen der laufzeit für animation
  if (millis() - animationLastUpdate > 500) {
    animationLastUpdate = millis();
    if (lightLvl == -1 || moistLvl == 1) {
      animationY -= 8;
      if (animationY < -8) animationY = SCREEN_HEIGHT;
    }
  }
}

// saving moist data in array
void addMoistData(int d) {
  Serial.println(d);
  for (int i = 0; i < DATA_LENGTH - 1; i++) {
    moistData[i] = moistData[i + 1];
  }
  moistData[DATA_LENGTH - 1] = d;  //map(500 - d, 0, 500, 0, SCREEN_HEIGHT / 2);
  if (d > 570) {                   // checking
    moistLvl = -1;
  } else if (d < 360) {
    moistLvl = 1;
  } else {
    moistLvl = 0;
  }
}

// saving light data in array
void addLightData(int d) {
  for (int i = 0; i < DATA_LENGTH - 1; i++) {
    lightData[i] = lightData[i + 1];
  }
  lightData[DATA_LENGTH - 1] = d;
  //Serial.println(d);
  if (d < 770 && lightData[DATA_LENGTH - 2] < 770 && lightData[DATA_LENGTH - 3] < 770 && lightData[DATA_LENGTH - 4] < 770) {  // check if light is low for 4 frames
    lightLvl = -1;
  } else if(d > 775 && lightData[DATA_LENGTH - 2] > 775) {
    lightLvl = 0;
  }
}

void drawGraph(int data2[]) {
  display.clearDisplay();

  // Zeichne den Graphen
  display.drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, SSD1306_WHITE);
  for (int i = 1; i < DATA_LENGTH; i++) {
    int x1 = (i - 1);
    int y1 = SCREEN_HEIGHT / 2 - data2[i - 1] + 730;
    int x2 = i;
    int y2 = SCREEN_HEIGHT / 2 - data2[i] + 730;
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // anzeigen des genauen wertes
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  switch (scene) {
    case 1:
      display.println("Moisture Data:");
      break;
    case 0:
      display.println("Light Data:");
      break;
    default:
      break;
  }
  display.setCursor(SCREEN_WIDTH - 30, 0);
  display.println(data2[DATA_LENGTH - 1]);
  // Anzeige aktualisieren
  display.display();
}

void drawFace() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(33, 20);

  if (lightLvl == -1) {
    // Sleeping face
    display.clearDisplay();
    display.print("- -");
    display.setCursor(30, 40);
    display.print("---_");

    // Draw the zzz animation
    display.setTextSize(1);
    display.setCursor(80, animationY);
    display.print("Z");
    display.setCursor(90, animationY - 8);
    display.print("Z");
    display.setCursor(100, animationY - 16);
    display.print("Z");
  }

  if (moistLvl == -1) {
    digitalWrite(BUZZER_PIN, HIGH);  // turn on buzzer
    // draw sad face
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(33, 20);
    display.print("T T");
    display.setCursor(30, 40);
    display.print("___");
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // turn off buzzer
  }
  if (moistLvl == 1) {
    // Draw the bubble animation using animation var
    display.setCursor(80, animationY + 7);
    display.print("o");
    display.setCursor(50, animationY - 18);
    display.print("o o");
    display.setCursor(100, animationY - 33);
    display.print("o");
    display.setCursor(122, animationY - 55);
    display.print("o");
  }

  if (moistLvl != -1 && lightLvl != -1) {
    // Normal happy face
    display.setCursor(33, 20);
    display.print("0 0");
    display.setCursor(30, 40);
    display.print("\\___/");
  }


  display.display();
}
