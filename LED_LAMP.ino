#include <avr/sleep.h>
#include "FastLED.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///////////////////        Настройки ходов и т.п.       //////////////////////
//////////////////////////////////////////////////////////////////////////////

#define LEFT        7     // Кнопка 1 (LEFT)
#define RIGHT       8     // Кнопка 2 (RIGHT)
#define SENSOR      2     // Датчик движения/освещенности

#define POWER       3     // Пин подключеного зарядного

#define BATTERY     A0    // Пин напрямую на батарею

#define MAX_BAT     967   // Аналоговое число полной зарядки 4.2В
#define MIN_BAT     200   // Аналоговое число низкой зарядки 2.2В

#define BTN_UP      HIGH  // Нормальное состояние не нажатой кнопки
#define BTN_DOWN    LOW   // Состояние нажатой кнопки

#define SENSOR_OFF  HIGH  // Нормальное состояние датчика
#define SENSOR_ON   LOW   // Срабатывание датчика

#define POWER_OFF   HIGH  // Нормальное состояние при отключеной зарядке
#define POWER_ON    LOW   // Нормальное состояние при включеной зарядке

#define NUM_LEDS    2     // Колличество светодиодов
#define DATA_PIN    4     // Пин DI светодиодной ленты

#define SPEED_UPD   200  // Скорость отправки данных о яркости в работе (в миллисекундах)
#define SPD_CHARG   10   // Скорость отправки данных о яркости при зарядке (в миллисекундах)

#define PUSH_TIME   2000 // Длительность удерживания кнопок (в секундах)

#define TIME_WORK   20   // Время свечения в режиме по датчику (в секундах)

#define WITE_MODE   10   // Время ожидания включения какого либо режима, отличного от 0 - выключено (в секундах)

//////////////////////////////////////////////////////////////////////////////
//////////////////////     Далее менять ничего не нужно    ///////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CRGB leds[NUM_LEDS];

// Яркость по цветам 30%
int colors_30[][3] = 
{
    {76, 76 ,76},       // 0 - Белый
    {76, 0 ,0},         // 1 - Красный
    {76, 49 ,0},        // 2 - Оранжевый
    {29, 30 ,30},       // 3 - Желтый
    {0, 76 ,0},         // 4 - Зеленый
    {31, 51 ,76},       // 5 - Голубой
    {0, 0 ,76},         // 6 - Синий
    {41, 0 ,76},        // 7 - Фиолетовый
    {0, 0 ,0}           // 8 - Выключено
};

// Яркость по цветам 60%
int colors_60[][3] = 
{
    {153, 153 ,153},    // 0 - Белый
    {153, 0 ,0},        // 1 - Красный
    {153, 99 ,0},       // 2 - Оранжевый
    {36, 60 ,60},       // 3 - Желтый
    {0, 153 ,0},        // 4 - Зеленый
    {38, 102 ,153},     // 5 - Голубой
    {0, 0 ,153},        // 6 - Синий
    {83, 0 ,153},       // 7 - Фиолетовый
    {0, 0 ,0}           // 8 - Выключено
};

// Яркость по цветам 100%
int colors_100[][3] = 
{
    {255, 255 ,255},    // 0 - Белый
    {255, 0 ,0},        // 1 - Красный
    {255, 165 ,0},      // 2 - Оранжевый
    {60, 100 ,100},     // 3 - Желтый
    {0, 255 ,0},        // 4 - Зеленый
    {66, 170 ,255},     // 5 - Голубой
    {0, 0 ,255},        // 6 - Синий
    {139, 0 ,255},      // 7 - Фиолетовый
    {0, 0 ,0}           // 8 - Выключено
};


int mode = 0;           // Текущий режим  (0 - выключено, 1 - постоянное свечение, 2 - включение по датчику)
int light = 0;          // Текущая яркость (номер яркости -- название массива цветов с заданной яркостью (0 - 100%, 1 - 60%, 2 -30%))
int color = 8;          // Текущий цвет (номер цвета, где 8 - выключено)

int charged_color = 0;  // Яркость при зарядке (текущая яркость при зарядке)
bool charged_reverse = 0; // Реверс свечения при зарядке

bool charged = false;   // Состояние зарядки (подключено ли зарядное - есть ли питание на пине зарядки)

bool flag_left = false;
bool flag_right = false;
bool flag_power = false;
bool long_click = false;

unsigned long current = 0;
unsigned long pushdown = 0;
unsigned long timework = 0;
unsigned long witemode = 0;


int sleepStatus = 0;             // Переменная для хранения статуса (спим, проснулись) - не используется в коде
int LedPin=13;                   // Светодиод

int sleep_mode = 0; // Текущий вид сна ( 0 - выключен, 1 - по датчику движения (пин 2), 2 - по подключеной зарядке (пин 3) )

bool charged_not_full = true;



// Прерывание сработает после пробуждения
void wakeUpNow()        
{
    // Если ышли из сна
    if (sleepStatus)
    {
        sleep_disable();
        digitalWrite(LedPin, HIGH);  // Включаем светодиод для индикации что ардуино не спит
        sleepStatus = 0;             // В переменную заносим статус бодрствования
    }
    else
    {
        // Если ардуино просто включилась
        
        // Включаем режим сна по датчику и по зарядке
        modesleep(3);
    }
}

void setup() 
{   
    pinMode(LedPin, OUTPUT); digitalWrite(LedPin, HIGH);
    pinMode(LEFT, INPUT); digitalWrite(LEFT, BTN_UP);
    pinMode(RIGHT, INPUT); digitalWrite(RIGHT, BTN_UP);
    pinMode(SENSOR, INPUT); digitalWrite(SENSOR, SENSOR_OFF);
    pinMode(POWER, INPUT); digitalWrite(POWER, POWER_OFF);

    witemode = millis();
    
    FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS); 
    attachInterrupt(1, wakeUpNow, FALLING);
}

void sleepNow()         // Функция увода ардуины в спячку.
{
    digitalWrite(LedPin, LOW);             // Выключаем светодиод
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // Здесь устанавливается режим сна
    sleep_enable();                        // Включаем sleep-бит в регистре mcucr. Теперь возможен слип   
    sleepStatus = 1;                       // В переменную заносим статус сна
    sleep_mode();                          // Здесь устройство перейдет в режим сна!!!
  
    timework = millis();
    witemode = millis();
}

void showLeds()
{
    // Отправка данных на светодиоды (частота меняется в шапке, ля избежания мерцания и подтормаживаний)
    if(millis() - current > SPEED_UPD)
    {
        for(int i = 0; i < NUM_LEDS; i++)
        {
            if(light == 0) leds[i] = CRGB(colors_100[color][0] ,colors_100[color][1] , colors_100[color][2]); 
            if(light == 1) leds[i] = CRGB(colors_60[color][0] ,colors_60[color][1] , colors_60[color][2]); 
            if(light == 2) leds[i] = CRGB(colors_30[color][0] ,colors_30[color][1] , colors_30[color][2]); 
        }
    
        FastLED.show();
        current = millis();
    }
}
void modesleep(int s)
{
    if(s == 0) // выключаем оба прерывания
    {
        detachInterrupt(0);
        detachInterrupt(1);
    }
    else if(s == 1) // включаем прерывание по датчику
    {
        detachInterrupt(1);
        attachInterrupt(0, wakeUpNow, FALLING);
    }
    else if(s == 2) // включаем прерывание по зарядке
    {
        detachInterrupt(0);
        attachInterrupt(1, wakeUpNow, FALLING);        
    }
    else if(s == 3) // включаем оба прерывания
    {
        attachInterrupt(0, wakeUpNow, FALLING);
        attachInterrupt(1, wakeUpNow, FALLING);
    }
}

void loop()
{
    // Если включили зарядку
    if(digitalRead(POWER) == POWER_ON)
    {
        // Режим выключено
        if(mode == 0)
        {
            flag_power = true;
    
            // Если напряжение меньше --- 4.2В (MAX_BAT)
            if(analogRead(BATTERY) < MAX_BAT && charged_not_full == true) 
            {
                // Моргаем свотодиодами..что идет зарядка
    
                if(charged_reverse == false) {
                    if(charged_color < 76) 
                        charged_color++;
                    else
                        charged_reverse = true;
                } else {
                    if(charged_color > 0) 
                        charged_color--;
                    else
                        charged_reverse = false;
                }

                if(charged_not_full == true)
                    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0 ,charged_color, 0); 
                else
                    for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(charged_color,0 , 0); 
            }
            else
            {   
                // Включаем все зеленым что заряжен полностью на 30% (2 режим яркости)
                for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0 ,76, 0);  
                
                charged_not_full = false;   
            }

            FastLED.show();
            delay(SPD_CHARG);
            
            if(analogRead(BATTERY) < MIN_BAT) 
                modesleep(2); // Уходим спать в ожидании зарядки
            else
                modesleep(3); // Уходим спать в режим ожидания
        }
        else
        {
            flag_power = false;
        }
    }
    else
    {
        // Если батарея разряжена
        if(analogRead(BATTERY) < MIN_BAT) 
        {
            charged_not_full = true;
            
            for(int i = 0; i < 3; i++)
            {
                for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(76, 0, 0); 
                FastLED.show();
    
                delay(100);

                for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0); 
                FastLED.show();

                delay(100);
            }

            mode = 0;
            modesleep(2);
            sleepNow();
        }
    }

    if(analogRead(BATTERY) < MAX_BAT - 100) charged_not_full = true;

    // Если выключили зарядку и текущий режим выключено ( 0 - режим)
    if(digitalRead(POWER) == POWER_OFF && flag_power == true)
    {
        for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0 ,0, 0);        
        FastLED.show();
        
        flag_power = false;
        charged_color = 0;
        charged_reverse = false;

        if(analogRead(BATTERY) < MIN_BAT) 
            modesleep(2); // Уходим спать в ожидании зарядки
        else
            modesleep(3); // Уходим спать в режим ожидания

        mode = 0;
        sleepNow();      
    }
    
    // Выключили зарядку и текущий режим включено (1, 2 режимы)
    if(digitalRead(POWER) == POWER_OFF && flag_power == true && mode > 0)
    {
        flag_power = false;       
        charged_color = 0;
        charged_reverse = false;

        // Если батарея по прежнему разряжена
        if(analogRead(BATTERY) < MIN_BAT) 
        {
            modesleep(2); // Уходим спать в ожидании зарядки   
           
            mode = 0;
            sleepNow();   
        }   
    }

    



    // Считываем короткое нажатие кнопки LEFT (Меняет режим)
    if(digitalRead(LEFT) == BTN_DOWN && flag_left == false) 
    {
        flag_left = true;
        delay(100);
    }
    
    if(digitalRead(LEFT) == BTN_UP && flag_left == true) 
    {
        if(mode == 2) 
        {
            mode = 0; 
        }
        else 
        {
            mode++;
            if(color == 8) color = 0;
        }
        
        timework = millis();
        flag_left = false;        
        delay(100);
    }

    // Считываем короткое нажатие кнопки RIGHT (Меняет цвет/яркость)
    if(digitalRead(RIGHT) == BTN_DOWN)
    {
        delay(100);
        
        if(long_click == false && flag_right == false) 
        {
            flag_right = true;
            long_click = true;
            pushdown = millis();
        }
        
        // Произошло удержание более 2 сек (PUSH_TIME)
        if(millis() - pushdown > PUSH_TIME)
        {
            // Устанавливаем таймер еще на 2 сек
            pushdown = millis(); 
            
            light++;
            long_click = false;
            flag_right = true;

            if(light > 2) light = 0;

            delay(500);
        }
    }
    
    if(digitalRead(RIGHT) == BTN_UP)
    {
        if(mode > 0) // Если лента не светит какой смысл менят цвет
        {
            if(long_click == true) 
            {
                if(color == 7) color = 0; else color++;
            }
        }
        
        delay(100);
        long_click = false;
        flag_right = false;
    }

    







    // Если выбран 0 режим (выключено)
    if(mode == 0)
    {
        for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0); 
        FastLED.show();

        // Если время ожидания выбора режима истекло
        if(millis() - witemode > WITE_MODE * 1000)
        {
            if(digitalRead(POWER) == POWER_OFF)
            {
                modesleep(3);
                sleepNow();
            }
        }
    }

    // Если выбран 1 режим (постоянное свечение)
    if(mode == 1)
    {
        modesleep(3);
        showLeds();
    }

    // Если выбран 2 режим (по датчику)
    if(mode == 2)
    {
        showLeds();
       
        if(millis() - timework > TIME_WORK * 1000)
        {
            for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0); 
            FastLED.show(); 

            modesleep(3);

            sleepNow();           
        }
    }
}
