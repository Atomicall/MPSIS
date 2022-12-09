#include <msp430.h>
#define WAIT_CYCLES 30000 // magic number. „тобы перестало им быть , нужно узнать MCLK
// https://stackoverflow.com/questions/34258254/msp430-delay-cycles-is-slow-mclk-might-be-slow
volatile long int i = 0;
volatile char bState = 0;
void main(void)
{
    // stop wd
    WDTCTL = WDTPW | WDTHOLD;

    // S1 input
    P1DIR &= ~BIT7;
    // pull enable
    P1REN |= BIT7;
    //pull-up
    P1OUT |= BIT7;

    //led 1 on p1.0
    //out
    P1DIR |= BIT1;
//    //en
//    P1OUT |= BIT0;
    //off
      P1OUT &= ~BIT1;

    //led 2 on 8.1
    //out
    P1DIR |=BIT5;
    //off
     P1OUT &=~BIT5;



     //led 3 on 1.2
        //out
        P1DIR |=BIT2;
        //off
        P1OUT &=~BIT2;
        //led 3 on 1.2
        //out
       P1DIR |=BIT3;
        //off
       P1OUT &=~BIT3;
       //led 3 on 1.2
       //out
       P1DIR |=BIT4;
        //off
       P1OUT &=~BIT4;




    while(1)
       {
           while (bState = (P1IN & BIT7)){
               P1OUT &= ~BIT1;
               P1OUT &=~BIT2;
               P1OUT &=~BIT3;
               P1OUT &=~BIT4;
               P1OUT &=~BIT5;
           }
           for (i=0; i<WAIT_CYCLES; i++){
           }
           P1OUT^=BIT1;
           for (i=0; i<WAIT_CYCLES; i++);
           P1OUT^=BIT2;
           for (i=0; i<WAIT_CYCLES; i++);
           P1OUT^=BIT3;
           for (i=0; i<WAIT_CYCLES; i++);
           P1OUT^=BIT4;
           for (i=0; i<WAIT_CYCLES; i++);
           P1OUT^=BIT5;


       }
}
