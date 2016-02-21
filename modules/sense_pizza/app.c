#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../custom_delay/custom_delay.h"
#include "../buzzer/buzzer.h"
#include "../sharp_sensor/SharpSensor.h"
#include "../color_sensor/color_sensor.h"
#include "../lcd/lcd.h"

int main() {
    InitSenses();
    int sharp;
    while(1) {
        sharp = SharpAdcConversion(11);
        LcdPrintf("value: %d", sharp);
    }
}
    