#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// --- ПІДКЛЮЧЕННЯ (Об'єднане) ---
#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 6
#define BIN2 7
#define PWMB 5

#define TRIG 8
#define ECHO 9

// Додано для старту рукою
#define SONAR_HAND_TRIG 10
#define SONAR_HAND_ECHO 11

// Кнопка/Бампер тепер на A2 (як у коді з таймером)
#define PIN_BUMPER A2 
#define BUZZER_PIN 12
#define LED_PIN 13

// Ініціалізація дисплею
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- ЗМІННІ ---
// Коефіцієнти з твого старого коду
float factorA = 1.0;  // Правий мотор
float factorB = 0.87; // Лівий мотор

int currentSpeed = 0;
int maxSpeed = 90; 
int minSpeed = 30;

// Змінні для таймера
unsigned long startTime = 0;
unsigned long lastScreenUpdate = 0;

void setup() {
  Serial.begin(9600);

  // Піни моторів
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  
  // Піни сенсорів
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  
  // Додаткові піни для функціоналу старту
  pinMode(SONAR_HAND_TRIG, OUTPUT); pinMode(SONAR_HAND_ECHO, INPUT);
  pinMode(PIN_BUMPER, INPUT_PULLUP); // Кнопка на A2
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Запуск дисплею
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_tf);

  // === 1. ОЧІКУВАННЯ СТАРТУ (Функціонал додано) ===
  waitForStart();

  // === 2. ЗАПУСК ТАЙМЕРА ===
  startTime = millis();

  // Плавний старт (Твій код)
  for (int i = 0; i <= maxSpeed; i++) {
    currentSpeed = i;
    moveForward(currentSpeed);
    delay(40); 
  }
}

void loop() {
  // === ПЕРЕВІРКА ФІНІШУ ===
  if (digitalRead(PIN_BUMPER) == LOW) {
    finishRoutine(); // Викликаємо нову функцію фінішу
  }

  // Отримання дистанції (Основний сенсор)
  long distance = getDistance(TRIG, ECHO);
  
  // === МЕХАНІКА РУХУ (Твій код) ===
  if (distance > 0 && distance < 500) {
    if (distance < 100) {
      currentSpeed = map(distance, 5, 100, minSpeed, maxSpeed);
      currentSpeed = constrain(currentSpeed, minSpeed, maxSpeed);
    } else {
      currentSpeed = maxSpeed;
    }
    moveForward(currentSpeed);
  }

  // === ОНОВЛЕННЯ ТАЙМЕРА ===
  if (millis() - lastScreenUpdate > 200) {
    lastScreenUpdate = millis();
    updateTimerScreen();
  }

  delay(50);
}

// --- ФУНКЦІЇ ---

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999;
  
  return duration * 0.034 / 2;
}

void moveForward(int speed) {
  // Мотор А (Лівий)
  digitalWrite(AIN1, HIGH); // Перевір, чи тут не треба інверсію
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, (int)(speed * factorB)); // factorB для лівого (AIN)
  
  // Мотор B (Правий) - ТУТ Я ІНВЕРТУВАВ ЩОБ ЇХАВ ВПЕРЕД
  digitalWrite(BIN1, LOW);  // Було HIGH
  digitalWrite(BIN2, HIGH); // Було LOW
  analogWrite(PWMB, (int)(speed * factorA)); // factorA для правого (BIN)
}

// --- НОВІ ФУНКЦІЇ (Старт, Таймер, Фініш) ---

void waitForStart() {
  bool ready = false;
  
  u8g2.clearBuffer();
  u8g2.drawStr(10, 20, "READY TO RACE");
  u8g2.drawStr(10, 40, "Wave Hand (S2)");
  u8g2.drawStr(10, 50, "or Press Bumper");
  u8g2.sendBuffer();

  while (!ready) {
    // Використовуємо допоміжний сонар (10, 11)
    long handDist = getDistance(SONAR_HAND_TRIG, SONAR_HAND_ECHO);
    
    // Чекаємо руку (<15см) або кнопку (LOW)
    if ((handDist > 0 && handDist < 15) || digitalRead(PIN_BUMPER) == LOW) {
      ready = true;
      tone(BUZZER_PIN, 1000, 100);
      delay(150);
      tone(BUZZER_PIN, 2000, 300);
      delay(500); 
    }
    delay(50);
  }
}

void updateTimerScreen() {
  float currentTime = (millis() - startTime) / 1000.0;
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont11_tf);
    u8g2.setCursor(0, 10);
    u8g2.print("Speed: "); u8g2.print(currentSpeed);
    
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(20, 40);
    u8g2.print(currentTime, 1);
    u8g2.print(" s");
  } while (u8g2.nextPage());
}

void finishRoutine() {
  // Стоп мотори
  analogWrite(PWMA, 0); analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  
  // Фіксуємо час
  float finalTime = (millis() - startTime) / 1000.0;
  
  // Відображаємо
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(25, 20, "FINISH!");
    
    u8g2.setFont(u8g2_font_profont11_tf);
    u8g2.drawStr(10, 40, "Time:");
    
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(50, 45);
    u8g2.print(finalTime, 1);
    u8g2.print("s");
  } while (u8g2.nextPage());

  // Мелодія перемоги
  tone(BUZZER_PIN, 523, 100); delay(120);
  tone(BUZZER_PIN, 659, 100); delay(120);
  tone(BUZZER_PIN, 784, 100); delay(120);
  tone(BUZZER_PIN, 1047, 400); delay(400);

  // Блокування системи
  while(true) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}