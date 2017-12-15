#include <driver/adc.h>
#include <SimpleRingBuffer.h>

#define SERIAL_BAUD 115200
/*
....###....##.....##.########..####..#######.....##.....##....###....########.
...##.##...##.....##.##.....##..##..##.....##....##.....##...##.##...##.....##
..##...##..##.....##.##.....##..##..##.....##....##.....##..##...##..##.....##
.##.....##.##.....##.##.....##..##..##.....##....##.....##.##.....##.########.
.#########.##.....##.##.....##..##..##.....##.....##...##..#########.##...##..
.##.....##.##.....##.##.....##..##..##.....##......##.##...##.....##.##....##.
.##.....##..#######..########..####..#######........###....##.....##.##.....##
/*
//#define AUDIO_TIMING_VAL 125 /* 8,000 hz */
//#define AUDIO_TIMING_VAL 83 /* 12,000 hz */
#define AUDIO_TIMING_VAL 62 /* 16kHz */
//#define AUDIO_TIMING_VAL 30 /* 16,000 hz */
//#define AUDIO_TIMING_VAL 50  /* 20,000 hz */

/*
.########.####.##.....##.########.########.
....##.....##..###...###.##.......##.....##
....##.....##..####.####.##.......##.....##
....##.....##..##.###.##.######...########.
....##.....##..##.....##.##.......##...##..
....##.....##..##.....##.##.......##....##.
....##....####.##.....##.########.##.....##
*/
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer(){
    // Increment the counter and set the time of ISR
    portENTER_CRITICAL_ISR(&timerMux);
  
    //read audio from ESP32 ADC 
    uint16_t value = adc1_get_voltage(ADC1_CHANNEL_0);
    // value = map(value, 0, 4095, 0, 255);
    Serial.println(value);

    portEXIT_CRITICAL_ISR(&timerMux);
    // Give a semaphore that we can check in the loop
    xSemaphoreGiveFromISR(timerSemaphore, NULL);
    // It is safe to use digitalRead/Write here if you want to toggle an output
}

/*
..######..########.########.##.....##.########.
.##....##.##..........##....##.....##.##.....##
.##.......##..........##....##.....##.##.....##
..######..######......##....##.....##.########.
.......##.##..........##....##.....##.##.......
.##....##.##..........##....##.....##.##.......
..######..########....##.....#######..##.......
 */
void setup() {
    int mySampleRate = AUDIO_TIMING_VAL;
    Serial.begin(SERIAL_BAUD);
/* 
..######..########.########.##.....##.########........###....########...######.
.##....##.##..........##....##.....##.##.....##......##.##...##.....##.##....##
.##.......##..........##....##.....##.##.....##.....##...##..##.....##.##......
..######..######......##....##.....##.########.....##.....##.##.....##.##......
.......##.##..........##....##.....##.##...........#########.##.....##.##......
.##....##.##..........##....##.....##.##...........##.....##.##.....##.##....##
..######..########....##.....#######..##...........##.....##.########...######. 
*/
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_11db);

// SETUP TIMER
    timerSemaphore = xSemaphoreCreateBinary();
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, mySampleRate, true);
    timerAlarmEnable(timer);
}

/*
.##........#######...#######..########.
.##.......##.....##.##.....##.##.....##
.##.......##.....##.##.....##.##.....##
.##.......##.....##.##.....##.########.
.##.......##.....##.##.....##.##.......
.##.......##.....##.##.....##.##.......
.########..#######...#######..##.......
*/
void loop() {
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
        portENTER_CRITICAL(&timerMux);
        portEXIT_CRITICAL(&timerMux);
        // Print it
        //Serial.print(".");
    }
}
