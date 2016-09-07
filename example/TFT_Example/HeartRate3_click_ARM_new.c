#include <stdint.h>
#include "heartrate_3.h"
#include "resources.h"


// HeartRate 3 GPIO
sbit RST at GPIOC_ODR.B2;

// TFT module connections
unsigned int TFT_DataPort at GPIOE_ODR;
sbit TFT_RST at GPIOE_ODR.B8;
sbit TFT_RS at GPIOE_ODR.B12;
sbit TFT_CS at GPIOE_ODR.B15;
sbit TFT_RD at GPIOE_ODR.B10;
sbit TFT_WR at GPIOE_ODR.B11;
sbit TFT_BLED at GPIOE_ODR.B9;
// End TFT module connections

void system_setup( void );
static void Display_Init( void );
void setup_interrupt();
void InitTimer2();

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
     Display_Init();
     InitTimer2();      // Used to calibrate sampling frequency

       while(1)
       {   
           rate = hr3_get_heartrate();                                                   //Rate takes around 10 seconds to have accurate reading ( could implement Timer )
           //Write to TFT
           TFT_Set_Font(&TFT_defaultFont, CL_WHITE, FO_HORIZONTAL);
           TFT_Write_Text(txt, 200, 120);                                                // Display inverted string on TFT
           LongToStr(rate,txt);                                                          // Conversion Integer to String
           TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
           TFT_Write_Text(txt, 200, 120);                                                // Display string on TFT
           //Write to UART
           
           //sprintl( uart_text, "%llu\r\n", led1 );
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

static void Display_Init(){
  TFT_Init_ILI9341_8bit(320, 240);
  TFT_Set_Pen(CL_WHITE, 1);
  TFT_Set_Brush(1,CL_WHITE,0,0,0,0);
  TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("Heart Rate 3 click", 75, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("EasyMx PRO v7 for STM32", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Image(20, 60, heartrate_bmp, 1);
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

// This timer was used to calibrate sampling frequency for heartrate..
// Timing is right on and when MCU was at room temp, ADC_RDY was triggered
// exactly 100 times a second. ( what we want )
void InitTimer2(){    // 1 second
  RCC_APB1ENR.TIM2EN = 1;
  TIM2_CR1.CEN = 0;
  TIM2_PSC = 1;
  TIM2_ARR = 35999;
  NVIC_IntEnable(IVT_INT_TIM2);
  TIM2_DIER.UIE = 1;
  TIM2_CR1.CEN = 1;
}

void Timer2_interrupt() iv IVT_INT_TIM2 {
  TIM2_SR.UIF = 0;

  //sprintl( uart_text, "Counter: %llu \r\n", int_count );
  //UART1_Write_Text( uart_text );
  //int_count = 0;
}