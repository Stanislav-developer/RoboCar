#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// --- ПІДКЛЮЧЕННЯ ---
#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 6
#define BIN2 7
#define PWMB 5

#define PIN_IR_FL A0 
#define PIN_IR_FR A1 

#define SONAR_HAND_TRIG 10
#define SONAR_HAND_ECHO 11
#define PIN_BUMPER A2
#define BUZZER_PIN 12
#define LED_PIN 13

// --- НАЛАШТУВАННЯ ШВИДКОСТІ ---
// Якщо вилітає з траси - зменшуй BASE_SPEED
// Якщо недокручує повороти - збільшуй REVERSE_SPEED

const int BASE_SPEED = 100;     // Швидкість на прямій (0-255)
const int TURN_SPEED = 200;     // Швидкість зовнішнього колеса на повороті
const int REVERSE_SPEED = 0;  // Швидкість внутрішнього колеса НАЗАД (для різкості)

// Налаштування датчика (1 або 0)
const int LINE_TRIGGER_STATE = 1; 

// Коефіцієнти (якщо один мотор сильніший)
float factorLeft = 1.0;  
float factorRight = 1.0; 

unsigned long startTime = 0;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  // Налаштування пінів
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  
  pinMode(PIN_IR_FL, INPUT);
  pinMode(PIN_IR_FR, INPUT);
  
  pinMode(SONAR_HAND_TRIG, OUTPUT); pinMode(SONAR_HAND_ECHO, INPUT);
  pinMode(PIN_BUMPER, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Дисплей ініціалізуємо, але використовуємо тільки на старті
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont11_tf);

  // Очікування
  waitForStart();

  // Старт гонки
  startTime = millis();
}

void loop() {
  // Цей цикл має виконуватися максимально швидко!
  // Жодних delay, Serial.print чи u8g2 тут!

  // 1. Читаємо датчики
  // digitalRead працює швидко (кілька мікросекунд)
  bool lineL = (digitalRead(PIN_IR_FL) == LINE_TRIGGER_STATE);
  bool lineR = (digitalRead(PIN_IR_FR) == LINE_TRIGGER_STATE);

  // 2. Логіка (Швидке дерево рішень)
  if (lineL && !lineR) {
    turnLeft();
    delay(70);
  } 
  else if (!lineL && lineR) {
    turnRight();
    delay(70);
  } 
  else {
    // Якщо обидва на білому АБО обидва на чорному - їдемо прямо
    // Це економить час на зайві перевірки
    moveStraight();
  }

  // 3. Перевірка фінішу (Бампер)
  // Перевіряємо в кінці циклу
  if (digitalRead(PIN_BUMPER) == LOW) {
    finishRoutine();
  }
}

// --- АГРЕСИВНІ ФУНКЦІЇ РУХУ ---

void moveStraight() {
  // Лівий Вперед
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  analogWrite(PWMA, (int)(BASE_SPEED * factorLeft));
  
  // Правий Вперед (Інвертований: LOW/HIGH)
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, (int)(BASE_SPEED * factorRight));
}

void turnLeft() {
  // Різкий поворот вліво
  
  // Лівий НАЗАД (Гальмування двигуном / Реверс)
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); // Реверс
  analogWrite(PWMA, REVERSE_SPEED);

  // Правий ВПЕРЕД (Максимум)
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, TURN_SPEED);
}

void turnRight() {
  // Різкий поворот вправо
  
  // Лівий ВПЕРЕД (Максимум)
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  analogWrite(PWMA, TURN_SPEED);

  // Правий НАЗАД (Гальмування двигуном / Реверс)
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); // Реверс (інвертована логіка)
  analogWrite(PWMB, REVERSE_SPEED);
}

void stopMotors() {
  analogWrite(PWMA, 0); analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}

// --- ДОПОМІЖНІ ФУНКЦІЇ ---

long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 15000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void waitForStart() {
  bool ready = false;
  u8g2.firstPage();
  do {
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(15, 20, "RACE MODE");
      u8g2.setFont(u8g2_font_profont11_tf);
      u8g2.drawStr(10, 40, "Display OFF in run");
      u8g2.drawStr(10, 55, "Wave Hand to GO!");
  } while (u8g2.nextPage());

  while (!ready) {
    long d = getDistance(SONAR_HAND_TRIG, SONAR_HAND_ECHO);
    // Кнопка також працює як старт
    if ((d > 0 && d < 15) || digitalRead(PIN_BUMPER) == LOW) {
      ready = true;
      tone(BUZZER_PIN, 1000, 100); delay(150);
      tone(BUZZER_PIN, 3000, 500); delay(500); 
    }
  }
  // Очищаємо екран перед стартом, щоб не світився дарма
  u8g2.clearDisplay(); 
}

void finishRoutine() {
  stopMotors();
  float finalTime = (millis() - startTime) / 1000.0;
  
  // Тут вже можна вмикати дисплей
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(25, 20, "FINISH!");
    u8g2.setCursor(40, 45);
    u8g2.print(finalTime, 3); // 3 знаки після коми для точності
    u8g2.print("s");
  } while (u8g2.nextPage());

  tone(BUZZER_PIN, 523, 150); delay(200);
  tone(BUZZER_PIN, 784, 150); delay(200);
  tone(BUZZER_PIN, 1047, 400); 
  
  while(true);
}