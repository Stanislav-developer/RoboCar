// Піни для драйвера TB6612FNG
#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 6
#define BIN2 7
#define PWMB 5

// Піни ультразвукового датчика
#define TRIG 8
#define ECHO 9

// Калібрувальні коефіцієнти швидкості
int speedA = 50; // Правий мотор
int speedB = 50; // Лівий мотор

// Параметри поведінки
#define OBSTACLE_DISTANCE 50 // Відстань спрацювання в см
#define CRUISE_SPEED 50     // Швидкість руху вперед
#define TURN_SPEED 50       // Швидкість повороту

void setup() {
  // Налаштування моторів
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  
  // Налаштування сонара
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  Serial.begin(9600);
  Serial.println("Робот-пилосос запущено!");
  
  delay(2000);
}

void loop() {
  // Вимірюємо відстань
  long distance = getDistance();
  
  Serial.print("Відстань: ");
  Serial.print(distance);
  Serial.println(" см");
  
  if (distance < OBSTACLE_DISTANCE && distance > 0) {
    // Перешкода близько!
    Serial.println("Перешкода! Маневрую...");
    
    // 1. Зупинка
    stopMotors();
    delay(200);
    
    // 2. Назад
    moveBackward();
    delay(500);
    
    // 3. Зупинка
    stopMotors();
    delay(200);
    
    // 4. Поворот (випадково вліво або вправо)
    if (random(0, 2) == 0) {
      turnLeft();
      Serial.println("Повертаю вліво");
    } else {
      turnRight();
      Serial.println("Повертаю вправо");
    }
    delay(random(400, 800)); // Випадковий час повороту
    
    // 5. Зупинка
    stopMotors();
    delay(200);
    
  } else {
    // Шлях вільний - їдемо вперед
    moveForward();
  }
  
  delay(50); // Невелика затримка між вимірами
}

// Функція для вимірювання відстані
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duration = pulseIn(ECHO, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  
  if (distance == 0 || distance > 400) {
    return 999; // Перешкода дуже далеко
  }
  
  return distance;
}

// Рух вперед
void moveForward() {
  // Мотор A (правий) - вперед
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, CRUISE_SPEED);
  
  // Мотор B (лівий) - вперед
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, CRUISE_SPEED);
}

// Рух назад
void moveBackward() {
  // Мотор A (правий) - назад
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, CRUISE_SPEED);
  
  // Мотор B (лівий) - назад
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, CRUISE_SPEED);
}

// Поворот вліво (правий мотор вперед, лівий назад)
void turnLeft() {
  // Мотор A (правий) - вперед
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, TURN_SPEED);
  
  // Мотор B (лівий) - назад
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, TURN_SPEED);
}

// Поворот вправо (лівий мотор вперед, правий назад)
void turnRight() {
  // Мотор A (правий) - назад
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, TURN_SPEED);
  
  // Мотор B (лівий) - вперед
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, TURN_SPEED);
}

// Зупинка моторів
void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}