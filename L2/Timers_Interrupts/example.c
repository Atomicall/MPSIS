#include <msp430.h>

/*  Глобальные переменные  */
int i=0,period=30,p_1=15,p_0=0,step=1;

/*  Объявление функций  */

void main(void) {
        WDTCTL = WDTPW + WDTHOLD;    // отключаем сторожевой таймер

        P1DIR |= BIT0; // установка пина 0 порта 1 в режим выхода (управление светодиодом  LED1)
       // P1OUT&=~BIT0;

    //    P1OUT = 0;
        P1DIR &= ~BIT7; // установка пина 7 порта 1 в режим входа ( кнопка s1)
        P1REN |= BIT7; // включение резистора подтяжки для пина 1.7
        P1OUT |= BIT7;

        P2DIR &= ~BIT2; // установка пина 2.2 в режим входа ( кнопка  s2)
        P2REN |= BIT2; // включение резистора подтяжки для пина 2.2
        P2OUT |= BIT2;


        TA1CCR0 = 1250 - 1;    // Период в 62,500 цикла, от 0 до 62,499.
        TA1CCTL0 = CCIE;        // Разрешаем прерывание таймера по достижению значения CCR0.
        TA1CTL = TASSEL__SMCLK | ID_3 | MC_1 | TACLR /*| TAIE*/; // Настройка режима работы таймера Timer_A:
                                        // TASSEL_2 - источник тактов SMCLK (SubMainCLocK),
                                        // по умолчанию настроенных на работу от DCO
                                        // ID_3 - делитель частоты на 8, от 1MHz это будет 125kHz
                                        // MC_1 - режим прямого счёта (до TACCR0)
                                        // TACLR - начальное обнуление таймера

        P1IES |= BIT7; // прерывание по переходу 1->0 выбирается битом IESx = 1.
        P1IFG &= ~BIT7; // Для предотвращения немедленного вызова прерывания,


        P2IES |= BIT2; // прерывание по переходу 1->0 выбирается битом IESx = 1.
        P2IFG &= ~BIT2; // Для предотвращения немедленного вызова прерывания,
                        // обнуляем его флаг для P1.3 до разрешения прерываний
        P1IE |= BIT7;  // Разрешаем прерывания для P1.3
        P2IE |= BIT2;

        //_enable_interrupt();
        __bis_SR_register(LPM0_bits + GIE);

        while(1) {
            // Ничего не делаем, ждем прерывание. Идеальное место для использования
        }         // экономных режимов работы микроконтроллера!
} // main

/*  Обработчики прерываний  */



/*  Обработчики прерываний  */
#pragma vector = PORT1_VECTOR
__interrupt void P1_7_ISR(void) {


        if(P1IFG & BIT7)
        {
            //P1IFG &= ~BIT7;  // обнуляем флаг прерывания
            if(p_1>0)
                {
                    p_1-=step;

                    if(p_1<0) p_1=0;

                    p_0=period-p_1;
                }
            //delay();
            unsigned int n;
            for (n=0; n<30000; n++)
            {
                n++;
                n--;
            }

            P1IFG &= ~BIT7;  // обнуляем флаг прерывания
            return;

        }
        P1IFG = 0;// Возможно в этом нет необходимости, но обнуляем
        return;

} // P1_7_ISR

#pragma vector = PORT2_VECTOR
__interrupt void P2_2_ISR(void) {


    if(P2IFG & BIT2)
    {
        //P2IFG &= ~BIT2;  // обнуляем флаг прерывания
        if(p_1<period)
            {
                p_1+=step;

                if(p_1>period) p_1=period;

                p_0=period-p_1;
            }
        //delay();
        unsigned int n;
        for (n=0; n<30000; n++)
        {
            n++;
            n--;
        }
        P2IFG &= ~BIT2;  // обнуляем флаг прерывания
    //  P2IFG &= ~BIT2;  // обнуляем флаг прерывания
        return;

    }
    P2IFG = 0;// Возможно в этом нет необходимости, но обнуляем
    return;

} // P2_2_ISR

#pragma vector = TIMER1_A0_VECTOR
__interrupt void CCR0_ISR(void) {
    //i++;
    if(i==0 && i!= p_1)P1OUT|=BIT0;
    if(i==p_1 && i!= period)P1OUT&=~BIT0;
    if(i==period)i=-1;
    i++;
    return;

} // CCR0_ISR
