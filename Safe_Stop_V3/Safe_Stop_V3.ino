// --- ПІНИ ---
// Драйвер TB6612FNG
const int AIN1 = 2;
const int AIN2 = 4;
const int BIN1 = 6;
const int BIN2 = 7;
const int PWMA = 3;
const int PWMB = 5;
const int STBY = -1; // Якщо підключено до 5V, то не використовуємо пін

// Сонари
const int TRIG_MAIN = 8;  // Передній
const int ECHO_MAIN = 9;
const int TRIG_START = 10; // Верхній (для старту рукою)
const int ECHO_START = 11;

// Вхід
const int PIN_INPUT_A2 = A2; // Кнопка + Кінцевики
const int BUZZER = 12;

// --- НАЛАШТУВАННЯ (Tuning) ---
// Коефіцієнти для вирівнювання моторів (0.0 - 1.0)
// Якщо їде криво, зменшуй число для того мотора, який крутиться швидше
const float LEFT_FACTOR = 0.92; 
const float RIGHT_FACTOR = 1.0; 

// Швидкості (0-255)
const int SPEED_MAX = 40;    // Основна швидкість (експериментуй, щоб циліндр не впав)
const int SPEED_MIN = 20;     // Швидкість "доповзання" до коробки
const int SPEED_START = 20;   // З чого починаємо розгін

// Дистанції (в см)
const int DIST_START_BRAKING = 60; // Відстань до коробки, коли починаємо гальмувати
const int DIST_HAND_START = 35;    // Відстань для сонара старту

void setup() {
  Serial.begin(9600);

  // Налаштування пінів моторів
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  
  // Сенсори
  pinMode(TRIG_MAIN, OUTPUT);
  pinMode(ECHO_MAIN, INPUT);
  pinMode(TRIG_START, OUTPUT);
  pinMode(ECHO_START, INPUT);
  
  pinMode(PIN_INPUT_A2, INPUT_PULLUP); // Використовуємо внутрішній резистор
  pinMode(BUZZER, OUTPUT);
  
  // Спочатку стоїмо
  stopMotors();
  beep(100); 
}

void loop() {
  // 1. ОЧІКУВАННЯ СТАРТУ
  waitForStart();

  // 2. ПЛАВНИЙ РОЗГІН
  // Розганяємося від SPEED_START до SPEED_MAX за певний час
  for (int s = SPEED_START; s <= SPEED_MAX; s += 1) {
    drive(s);
    delay(30); // Чим менше затримка, тим різкіший розгін
  }

  // 3. ЇЗДА ДО ЗОНИ ГАЛЬМУВАННЯ
  while (getDistance(TRIG_MAIN, ECHO_MAIN) > DIST_START_BRAKING) {
    drive(SPEED_MAX);
    // Тут можна додати PID по лініях, якщо підключиш сенсори правильно
    delay(10); 
  }

  // 4. ПЛАВНЕ ГАЛЬМУВАННЯ
  // Скидаємо швидкість від MAX до MIN
  for (int s = SPEED_MAX; s >= SPEED_MIN; s -= 1) {
    drive(s);
    delay(15);
  }

  // 5. ДОПОВЗАННЯ ДО КОРОБКИ (Creep Mode)
  // Їдемо повільно, поки не спрацює кінцевик на A2
  // digitalRead поверне LOW, коли кінцевик замкнеться на землю
  while (digitalRead(PIN_INPUT_A2) == HIGH) {
    drive(SPEED_MIN);
    
    // Аварійна зупинка, якщо сонар бачить, що ми вже впритул (на випадок якщо кінцевик не натиснувся)
    if (getDistance(TRIG_MAIN, ECHO_MAIN) < 2) { 
      break; 
    }
  }

  // 6. ФІНІШ
  stopMotors();
  beep(500); // Довгий сигнал
  
  // Зациклюємо, щоб робот нікуди не поїхав знову
  while(1); 
}

// --- ДОПОМІЖНІ ФУНКЦІЇ ---

// Функція керування моторами з корекцією курсу
void drive(int speed) {
  // Обмежуємо значення 0-255
  if (speed > 255) speed = 255;
  if (speed < 0) speed = 0;

  int speedA = speed * LEFT_FACTOR; // Лівий
  int speedB = speed * RIGHT_FACTOR; // Правий

  // Лівий мотор (перевір полярність, якщо їде назад - поміняй місцями HIGH/LOW)
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, speedA);

  // Правий мотор
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, speedB);
}

void stopMotors() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);
  
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 0);
}

// Очікування команди старту
void waitForStart() {
  bool ready = false;
  while (!ready) {
    // Перевірка кнопки (A2 тягне до землі при натисканні)
    if (digitalRead(PIN_INPUT_A2) == LOW) {
      ready = true;
      delay(200); // Антидрязгот
    }
    
    // Перевірка верхнього сонара (помах руки)
    if (getDistance(TRIG_START, ECHO_START) < DIST_HAND_START && getDistance(TRIG_START, ECHO_START) > 0) {
      ready = true;
      beep(100);
      delay(500); // Дати час прибрати руку
    }
  }
}

// Отримання дистанції в см
int getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long duration = pulseIn(echo, HIGH, 25000); // Таймаут 25мс (щоб не вішати код)
  if (duration == 0) return 999; // Якщо нічого не бачимо, вважаємо що далеко
  
  return duration * 0.034 / 2;
}

void beep(int duration) {
  digitalWrite(BUZZER, HIGH);
  delay(duration);
  digitalWrite(BUZZER, LOW);
}