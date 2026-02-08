#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 6
#define BIN2 7
#define PWMB 5

#define TRIG 8
#define ECHO 9
#define BUTTON_PIN 11
#define BUZZER_PIN 12

// Коефіцієнти балансу моцій (від 0.0 до 1.0)
float factorA = 1.0; // Правий мотор
float factorB = 0.97; // Лівий мотор (наприклад, якщо він швидший)

int currentSpeed = 0;
int maxSpeed = 90; 
int minSpeed = 35;

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.begin(9600);

  // Плавний старт з меншим кроком для м'якості
  for (int i = 0; i <= maxSpeed; i++) {
    currentSpeed = i;
    moveForward(currentSpeed);
    delay(40); 
  }
}

void loop() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    stopAndAlarm();
  }

  long distance = getDistance();
  
  if (distance > 0 && distance < 500) {
    if (distance < 100) {
      currentSpeed = map(distance, 5, 100, minSpeed, maxSpeed);
      currentSpeed = constrain(currentSpeed, minSpeed, maxSpeed);
    } else {
      currentSpeed = maxSpeed;
    }
    moveForward(currentSpeed);
  }

  delay(50);
}

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duration = pulseIn(ECHO, HIGH, 25000);
  if (duration == 0) return 999;
  
  return duration * 0.034 / 2;
}

void moveForward(int speed) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  // Застосування коефіцієнта до мотора А
  analogWrite(PWMA, (int)(speed * factorA));
  
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  // Застосування коефіцієнта до мотора В
  analogWrite(PWMB, (int)(speed * factorB));
}

void stopAndAlarm() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  
  // Цикл для коротких сигналів замість довгого списку delay
  for(int i = 0; i < 4; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  }
  
  while(true) {
    // Система заблокована
  }
}