/*
   SK6812 led strip control.
   Based on AlexGyver project 2017 http://alexgyver.ru/
   Copyright (c) 2024 Aliaksandr B.
*/
//----------------------НАСТРОЙКИ-----------------------
#define NUM_LEDS 2           // число светодиодов в ленте
#define DI_PIN 13            // пин, к которому подключена лента
#define OFF_TIME 10          // время (секунд), через которое лента выключится при пропадаании сигнала
#define CURRENT_LIMIT 2000   // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define START_FLASHES 1      // проверка цветов при запуске (1 - включить, 0 - выключить)

#define AUTO_BRIGHT 1        // автоматическая подстройка яркости от уровня внешнего освещения (1 - включить, 0 - выключить)
#define MAX_BRIGHT 255       // максимальная яркость (0 - 255)
#define MIN_BRIGHT 50        // минимальная яркость (0 - 255)
#define BRIGHT_CONSTANT 500  // константа усиления от внешнего света (0 - 1023)
// чем МЕНЬШЕ константа, тем "резче" будет прибавляться яркость
#define COEF 0.9             // коэффициент фильтра (0.0 - 1.0), чем больше - тем медленнее меняется яркость
//----------------------НАСТРОЙКИ-----------------------

int new_bright, new_bright_f;
unsigned long bright_timer, off_timer;

#define serialRate 115200  // скорость связи с ПК
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;  // кодовое слово Ada для связи
#include <SK6812.h>
SK6812 leds(NUM_LEDS);  // создаём ленту
boolean led_state = true;  // флаг состояния ленты

void setup()
{
  leds.set_output(DI_PIN); // инициализация светодиодов
  
  //if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);

  // вспышки красным синим и зелёным при запуске (можно отключить)
  if (START_FLASHES) {
    leds.set_rgbw(0, {0, 0, 0, 255});
    leds.sync();
    delay(500);
    leds.set_rgbw(0, {0, 0, 255, 0});
    leds.sync();
    delay(500);
    leds.set_rgbw(0, {255, 0, 0, 0});
    leds.sync();
    delay(500);
    leds.set_rgbw(0, {0, 255, 0, 0});
    leds.sync();
    delay(500);
    leds.set_rgbw(0, {0, 0, 0, 0});
    leds.sync();
    delay(500);
  }

  Serial.begin(serialRate);
  Serial.print("Ada\n");     // Связаться с компом
}

void check_connection() {
  if (led_state) {
    if (millis() - off_timer > (OFF_TIME * 1000)) {
      led_state = false;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds.set_rgbw(i, {0, 0, 0, 0});
      }
      leds.sync();
    }
  }
}

void loop() {
  if (AUTO_BRIGHT) {                         // если включена адаптивная яркость
    if (millis() - bright_timer > 100) {     // каждые 100 мс
      bright_timer = millis();               // сброить таймер
      new_bright = map(analogRead(6), 0, BRIGHT_CONSTANT, MIN_BRIGHT, MAX_BRIGHT);   // считать показания с фоторезистора, перевести диапазон
      new_bright = constrain(new_bright, MIN_BRIGHT, MAX_BRIGHT);
      new_bright_f = new_bright_f * COEF + new_bright * (1 - COEF);
     // LEDS.setBrightness(new_bright_f);      // установить новую яркость
    }
  }
  if (!led_state) led_state = true;
  off_timer = millis();  

  for (i = 0; i < sizeof prefix; ++i) {
waitLoop: while (!Serial.available()) check_connection();;
    if (prefix[i] == Serial.read()) continue;
    i = 0;
    goto waitLoop;
  }

  while (!Serial.available()) check_connection();;
  hi = Serial.read();
  while (!Serial.available()) check_connection();;
  lo = Serial.read();
  while (!Serial.available()) check_connection();;
  chk = Serial.read();
  if (chk != (hi ^ lo ^ 0x55))
  {
    i = 0;
    goto waitLoop;
  }

  //memset(leds, 0, NUM_LEDS * sizeof(RGBW));
  for (int i = 0; i < NUM_LEDS; i++) {
    byte r, g, b, w;
    // читаем данные для каждого цвета
    while (!Serial.available()) check_connection();
    r = Serial.read();
    while (!Serial.available()) check_connection();
    g = Serial.read();
    while (!Serial.available()) check_connection();
    b = Serial.read();
    w=min(r, min(g,b));
    r=r-w;
    g=g-w;
    b=b-w;
    leds.set_rgbw(i, {g, r, b, w});
  }
  
  leds.sync(); // записываем цвета в ленту*/
}
