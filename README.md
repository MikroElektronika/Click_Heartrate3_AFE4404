![MikroE](http://www.mikroe.com/img/designs/beta/logo_small.png)

![Heartrate 3 Click](http://cdn.mikroe.com/img/click/heartrate-3/heartrate-3.png)

---
[Product Page](http://www.mikroe.com/click/heartrate-3)

[Manual Page](http://docs.mikroe.com/Heartrate_3_click)

[Learn Page](http://learn.mikroe.com/micro-controllers-have-a-heart-too/)

---

### General Description

Description Coming Soon...


---

### Example

#### Configuration
* MCU:             STM32F107VC
* Dev.Board:       EasyMx Pro v7
* Oscillator:      72 Mhz internal
* Ext. Modules:    Heartrate 3 click
* SW:              MikroC PRO for ARM 4.7.4

```

/*      Functions
 ****************************/
// HeartRate 3 GPIO
sbit RST at GPIOC_ODR.B2;

void system_setup( void );
void setup_interrupt();

char  uart_text[20] = {0};
uint64_t int_count = 0;      //Used by timer to calibrate sampling freq.

void main()
{    
     //Local Declarations
     uint16_t rate     = 0;
     char txt[15]      = {0};

     system_setup();     // GPIO / HeartRate 3 / UART / I2C Setups
     Delay_ms(200);
     initStatHRM();      // Initializes values to 0
     setup_interrupt();  // Setup interrupt handler

       while(1)
       {   
           rate = hr3_get_heartrate();
           
           IntToStr( rate, uart_text );
           UART1_Write_Text( uart_text );
           UART1_Write_Text( "\r\n" );
       }

}

void system_setup( void )
{
       //Local Declarations
     char            text[40] = { 0 };
     dynamic_modes_t dynamic_modes;
     uint8_t         address = 0x58;
    
        //Set up dynamic modes for Heart Rate 3 Initialization
     dynamic_modes.transmit = trans_dis;                      //Transmitter disabled
     dynamic_modes.curr_range = led_double;                   //LED range 0 - 100
     dynamic_modes.adc_power = adc_on;                        //ADC on
     dynamic_modes.clk_mode = osc_mode;                       //Use internal Oscillator
     dynamic_modes.tia_power = tia_off;                       //TIA off
     dynamic_modes.rest_of_adc = rest_of_adc_off;             //Rest of ADC off
     dynamic_modes.afe_rx_mode = afe_rx_normal;               //Normal Receiving on AFE
     dynamic_modes.afe_mode = afe_normal;                     //Normal AFE functionality

       //GPIO setup
     GPIO_Digital_Output( &GPIOC_BASE, _GPIO_PINMASK_2 );
     GPIO_Digital_Input( &GPIOA_BASE, _GPIO_PINMASK_0 );
     GPIO_Digital_Input( &GPIOD_BASE, _GPIO_PINMASK_10 );

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


}

void setup_interrupt() 
{
  GPIO_Digital_Output(&GPIOE_BASE, _GPIO_PINMASK_HIGH);  // Enable digital output on PORTD
  GPIOE_ODR = 0xAAAA;
  GPIO_Digital_Input(&GPIOD_BASE, _GPIO_PINMASK_10);

  RCC_APB2ENR.AFIOEN = 1;              // Enable clock for alternate pin functions
  AFIO_EXTICR3 = 0x0300;               // PD10 as External interrupt
  EXTI_RTSR = 0x00000400;              // Set interrupt on Rising edge
  EXTI_IMR |= 0x00000400;              // Set mask
  NVIC_IntEnable(IVT_INT_EXTI15_10);   // Enable External interrupt
  EnableInterrupts();                  // Enables the processor interrupt.
}

void ExtInt() iv IVT_INT_EXTI15_10 ics ICS_AUTO {
   EXTI_PR.B10 = 1;                             // clear flag
   int_count++;
   statHRMAlgo( hr3_get_led1_amb1_val() );      // Give led1 ambient value to heartrate function. ( 100 times a second )

}
```


