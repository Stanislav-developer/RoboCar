// Піни ультразвукового датчика
#define TRIG 8
#define ECHO 9

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  Serial.println("Ультразвуковий датчик готовий");
}

void loop() {
  // Вимірюємо відстань
  long distance = getDistance();
  
  // Виводимо в Serial
  Serial.print("Відстань: ");
  Serial.print(distance);
  Serial.println(" см");
  
  delay(200); // Затримка між вимірами
}

// Функція для вимірювання відстані
long getDistance() {
  // Очищаємо Trig
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  
  // Генеруємо імпульс 10 мкс
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  // Читаємо час відбиття
  long duration = pulseIn(ECHO, HIGH, 30000); // таймаут 30мс
  
  // Обчислюємо відстань (см)
  // Швидкість звуку 340 м/с = 0.034 см/мкс
  // Ділимо на 2, бо звук йде туди і назад
  long distance = duration * 0.034 / 2;
  
  // Якщо таймаут - повертаємо 0
  if (distance == 0) {
    return 999; // Перешкода дуже далеко або немає
  }
  
  return distance;
}