 #include <stdint.h>
 #include "heartrate_3.h"

sbit RST at GPIOC_ODR.B2;

void main() 
{    //Local Declarations
     uint8_t address = 0x58;
     uint8_t current = 63;
     dynamic_modes_t dynamic_modes;
     
       //Set up dynamic modes for hr3 init
     dynamic_modes.transmit = trans_en;
     dynamic_modes.curr_range = led_norm;
     dynamic_modes.adc_power = adc_on;
     dynamic_modes.clk_mode = osc_mode;
     dynamic_modes.tia_power = tia_off;
     dynamic_modes.rest_of_adc = rest_of_adc_off;
     dynamic_modes.afe_rx_mode = afe_rx_normal;
     dynamic_modes.afe_mode = afe_normal;
     
       //GPIO setup
     GPIO_Digital_Output( &GPIOC_BASE, _GPIO_PINMASK_2 );
     GPIO_Digital_Input(&GPIOA_BASE, _GPIO_PINMASK_0);
     GPIO_Digital_Input(&GPIOD_BASE, _GPIO_PINMASK_10);
     
       //UART Initialize
     UART1_Init( 9600 );
     UART1_Write_Text( "UART is Initialized\r\n" );

       //Toggle Reset pin
     RST = 0;
     Delay_us(50);
     RST = 1;

       //I2C Initialize
     I2C1_Init_Advanced( 400000, &_GPIO_MODULE_I2C1_PB67 );
     UART1_Write_Text( "I2C Initialized\r\n" );

       //Heart Rate 3 Initialize
     hr3_init( address, &dynamic_modes );
     UART1_Write_Text( "Heart Rate 3 Initialized\r\n" );
     
     while(1)
     {
         UART1_Write_Text( "While Loop\r\n" );
         Delay_ms( 500 );
     }

}
