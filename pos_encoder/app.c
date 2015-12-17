#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pos_encoder.h"


//Main Function

int main(void)
{
    init_devices();

    while(1)
    {
        forward_mm(100); //Moves robot forward 100mm
        stop();
        _delay_ms(500);         
        
        back_mm(100);   //Moves robot backward 100mm
        stop();         
        _delay_ms(500);
        
        left_degrees(90); //Rotate robot left by 90 degrees
        stop();
        _delay_ms(500);
        
        right_degrees(90); //Rotate robot right by 90 degrees
        stop();
        _delay_ms(500);
        
        soft_left_degrees(90); //Rotate (soft turn) by 90 degrees
        stop();
        _delay_ms(500);
        
        soft_right_degrees(90); //Rotate (soft turn) by 90 degrees
        stop();
        _delay_ms(500);

        soft_left_2_degrees(90); //Rotate (soft turn) by 90 degrees
        stop();
        _delay_ms(500);
        
        soft_right_2_degrees(90);   //Rotate (soft turn) by 90 degrees
        stop();
        _delay_ms(500);
    }
}
