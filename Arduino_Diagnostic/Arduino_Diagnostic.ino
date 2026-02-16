// Діагностика Arduino UNO/Nano
void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Чекаємо підключення Serial
  
  Serial.println("=== Діагностика Arduino ===");
  Serial.println();
  
  // 1. Тест Serial
  Serial.println("[OK] Serial працює");
  
  // 2. Інформація про плату
  Serial.print("Тактова частота: ");
  Serial.print(F_CPU / 1000000);
  Serial.println(" MHz");
  
  // 3. Тест цифрових пінів
  Serial.println("\n--- Тест цифрових пінів ---");
  for (int pin = 2; pin <= 13; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(50);
    pinMode(pin, INPUT_PULLUP);
    int state = digitalRead(pin);
    Serial.print("Pin D");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(state == HIGH ? "OK" : "FAIL");
    digitalWrite(pin, LOW);
  }
  
  // 4. Тест аналогових входів
  Serial.println("\n--- Тест аналогових пінів ---");
  for (int pin = A0; pin <= A5; pin++) {
    pinMode(pin, INPUT);
    int value = analogRead(pin);
    Serial.print("Pin A");
    Serial.print(pin - A0);
    Serial.print(": ");
    Serial.print(value);
    Serial.println(value < 1000 ? " OK" : " (можливий шум)");
  }
  
  // 5. Тест PWM
  Serial.println("\n--- Тест PWM пінів ---");
  int pwmPins[] = {3, 5, 6, 9, 10, 11};
  for (int i = 0; i < 6; i++) {
    pinMode(pwmPins[i], OUTPUT);
    analogWrite(pwmPins[i], 128);
    delay(100);
    analogWrite(pwmPins[i], 0);
    Serial.print("PWM Pin D");
    Serial.print(pwmPins[i]);
    Serial.println(": OK");
  }
  
  // 6. Тест вбудованого LED
  Serial.println("\n--- Тест вбудованого LED (D13) ---");
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  Serial.println("LED мигає? Якщо так - OK");
  
  // 7. Вимірювання напруги живлення (приблизно)
  Serial.println("\n--- Перевірка живлення ---");
  long vcc = readVcc();
  Serial.print("VCC: ");
  Serial.print(vcc);
  Serial.print(" mV ");
  if (vcc > 4500 && vcc < 5500) {
    Serial.println("OK");
  } else {
    Serial.println("УВАГА: напруга поза нормою!");
  }
  
  Serial.println("\n=== Діагностика завершена ===");
}

void loop() {
  // Постійне мигання LED для підтвердження роботи
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

// Функція для вимірювання напруги живлення
long readVcc() {
  long result;
  // Читаємо внутрішнє опорне джерело 1.1V
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Розрахунок VCC в мВ
  return result;
}
