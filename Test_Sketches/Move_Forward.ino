// Піни для драйвера TB6612FNG
#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 6
#define BIN2 7
#define PWMB 5

// Калібрувальні коефіцієнти швидкості
int speedA = 150; // Правий мотор
int speedB = 150; // Лівий мотор (зменшуємо, бо він швидший)

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  
  delay(2000);
}

void loop() {
  // Рух вперед з каліброваними швидкостями
  // Мотор A (правий) - вперед
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, speedA);
  
  // Мотор B (лівий) - вперед
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, speedB);
}