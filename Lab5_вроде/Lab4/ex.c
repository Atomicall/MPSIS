#include <msp430.h>
volatile long int sysMillis = 0;

void setupPins()
{
    // Pot input

    P8DIR = |BIT8;
    P8OUT |= BIT8;
    P6SEL |=

    //led 1 on p1.0
    //out
    P1DIR |= BIT1;
    //off
    P1OUT &= ~BIT1;

    //led 3 on 1.2
    //out
    P1DIR |= BIT2;
    //off
    P1OUT &= ~BIT2;
}

// т.к нет других входных clk
// base MCLK = 1MHz
// период в 1ms = 1000 тиков при предделителе 1,
// значит TAxCCR0 = 500 при предделителе 2 - биты ID3 = 01

// TASSEL__SMCLK 10
// MC_1 = Управление режимом таймера = 01 - прямой счет до TAxCCR0
// TACLR - начальное обнуление
//
void setupTimers()
{
    TA2CCR0 = 500 - 1;
    // Разрешаем прерывание таймера по достижению значения TA1CCR0
    TA2CCTL0 = CCIE;
    // Настройка режима работы таймера Timer_A
    TA2CTL = TASSEL__SMCLK | ID_1 | MC_1 | TACLR;
    _enable_interrupt();
}

// 1 ms interrupt
#pragma vector = TIMER2_A0_VECTOR
__interrupt void CCR0_ISR(void)
{
    sysMillis++;
}

void taskManager()
{
    return;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    setupPins();
    setupTimers();
    while (1)
    {
        taskManager();
    }

}
