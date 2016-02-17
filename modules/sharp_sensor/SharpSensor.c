//#define __OPTIMIZE__ -O0
//#define F_CPU 14745600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "SharpSensor.h"

//ADC pin configuration
void SharpAdcPinConfig() {
	DDRF = 0x00; //set PORTF direction as input
	PORTF = 0x00; //set PORTF pins floating
	DDRK = 0x00; //set PORTK direction as input
	PORTK = 0x00; //set PORTK pins floating
}

void SharpPortInit() {
	SharpAdcPinConfig();
}

//Function to Initialize ADC
void SharpAdcInit() {
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

void SharpInitDevices() {
    cli(); //Clears the global interrupt
    SharpPortInit();  //Initializes all the ports
	SharpAdcInit();
    sei();   // Enables the global interrupt
}

//This Function accepts the Channel Number and returns the corresponding Analog Value 
unsigned char SharpAdcConversion(unsigned char Ch) {
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;  			
	ADMUX= 0x20| Ch;	   		
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

// This Function calculates the actual distance in millimeters(mm) from the input
// analog value of Sharp Sensor. 
unsigned int SharpGp2d12Estimation(unsigned char adc_reading) {
	float distance;
	unsigned int distance_int;
	distance = (int)(10.00*(2799.6*(1.00/(pow(adc_reading,1.1546)))));
	distance_int = (int)distance;
	if(distance_int>800)
	{
		distance_int=800;
	}
	return distance_int;
}

// default_dist is used when we can't tell what block we're detecting 
// because it is between two threshold values (small+threshold and medium-threshold)
int SharpGetBlockSize(int distance, int default_dist) {
    int threshold = 13;
	int no_block_threshold = 16;
	// All experimental averages
    int no_block_height = 320;
    int small_height = 262;
    int medium_height = 234;
    int large_height = 204;

    int ret = 0;

    if (fabs(distance - no_block_height) < no_block_threshold) {
        ret = 0;
    }
    else if(fabs(distance - small_height) < threshold) {
        ret = 6;
    }
    else if(fabs(distance - medium_height) < threshold) {
        ret = 9;
    }
    else if(fabs(distance - large_height) < threshold) {
        ret = 12;
    }
	else {
		ret = default_dist;
	}

    return ret;
}

char SharpGetBlockType() {
	int sharp = SharpAdcConversion(11);
    int value = SharpGp2d12Estimation(sharp);
    int block_size = SharpGetBlockSize(value, block_size); // Get block size from sharp sensor distance (in mm)
	char type;
	switch (block_size) {
		case 0: {
			type = 'n'; //no pizza at all
			break;
		}
		case 6: {
			type = 's'; //small pizza
			break;
		}
		case 9: {
			type = 'm'; //medium pizza
			break;
		}
		case 12: {
			type = 'l'; //large pizza
			break;
		}
		default:
			type = 'd'; //default value
	}
	return type;
}
