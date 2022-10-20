#include <msp430.h>
volatile long int sysMillis = 0;
// debounce (times)
volatile int DEBOUCE_TRESHOLD = 2;
// Led blink poll period
volatile int ledBlinkPollPeriod = 500;
// button poll period
volatile int buttonPollPeriod = 5;
// Leds off poll period
volatile char ledOffPollPeriod = 100;
// Is button pressed or released poll period
volatile char pressReleasePollPeriod = 50;

int leds[5] = { BIT1, BIT2, BIT3, BIT4, BIT5 };
volatile char ledsPrevStates[5] = { };
volatile int ledIndex = 0;
volatile char ledsWereOn = 0;

volatile char buttonState, button_debounce_counter;
volatile char buttonPrevState;
volatile char isButtonPressed = 0;
volatile char isButtonReleased = 1;

void setupPins()
{
    // S1 input
    P1DIR &= ~BIT7;
    // pull enable
    P1REN |= BIT7;
    //pull-up
    P1OUT |= BIT7;

    //led 1 on p1.0
    //out
    P1DIR |= BIT1;
    //off
    P1OUT &= ~BIT1;

    //led 2 on 1.1
    //out
    P1DIR |= BIT5;
    //off
    P1OUT &= ~BIT5;

    //led 3 on 1.2
    //out
    P1DIR |= BIT2;
    //off
    P1OUT &= ~BIT2;
    //led 3 on 1.3
    //out
    P1DIR |= BIT3;
    //off
    P1OUT &= ~BIT3;
    //led 4 on 1.4
    //out
    P1DIR |= BIT4;
    //off
    P1OUT &= ~BIT4;
}

// �.� ��� ������ ������� clk
// base MCLK = 1MHz
// ������ � 1ms = 1000 ����� ��� ������������ 1,
// ������ TAxCCR0 = 500 ��� ������������ 2 - ���� ID3 = 01

// TASSEL__SMCLK 10
// MC_1 = ���������� ������� ������� = 01 - ������ ���� �� TAxCCR0
// TACLR - ��������� ���������
//
void setupTimers()
{
    TA2CCR0 = 500 - 1;
    // ��������� ���������� ������� �� ���������� �������� TA1CCR0
    TA2CCTL0 = CCIE;
    // ��������� ������ ������ ������� Timer_A
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
    static long int prevLedBlinkPollEntry = 0;
    static long int prevButtonPollEntry = 0;
    static long int prevLedOffPollEntry = 0;
    static long int prevPressReleasePollEntry = 0;

    if (sysMillis - prevButtonPollEntry >= buttonPollPeriod)
    {
        prevButtonPollEntry = sysMillis;
        if (!(P1IN & BIT7))
        {
            if (button_debounce_counter > DEBOUCE_TRESHOLD)
            {
                buttonState = 1;
                button_debounce_counter = 0;
            }
            else
            {
                button_debounce_counter++;
            }
        }
        else
        {
            button_debounce_counter = 0;
            buttonState = 0;
        }

    }

    if (sysMillis - prevLedBlinkPollEntry >= ledBlinkPollPeriod
            && isButtonPressed)
    {
        if (!ledsWereOn)
        {
            char i;
            for (i = 0; i < 5; i++)
            {
                if (ledsPrevStates[i])
                {
                    P1OUT |= leds[i];
                }

            }
            ledsWereOn = 1;
        }
        prevLedBlinkPollEntry = sysMillis;
        P1OUT ^= leds[ledIndex];
        ledIndex++;
        if (ledIndex > 4)
            ledIndex = 0;
    }

    if (sysMillis - prevPressReleasePollEntry >= pressReleasePollPeriod)
    {
        prevPressReleasePollEntry = sysMillis;
        if (buttonPrevState != buttonState)
        {
            if (buttonState)
            {
                isButtonPressed = 1;
                isButtonReleased = 0;
            }
            if (!buttonState)
            {
                isButtonPressed = 0;
                isButtonReleased = 1;
            }
        }
        buttonPrevState = buttonState;
    }

    if (sysMillis - prevLedOffPollEntry >= ledOffPollPeriod)
    {
        if (isButtonReleased)
        {
            if (ledsWereOn)
            {
                char i;
                for (i = 0; i < 5; i++)
                {
                    ledsPrevStates[i] = P1IN & leds[i];
                }
                ledsWereOn = 0;
            }
            P1OUT &= ~BIT1;
            P1OUT &= ~BIT2;
            P1OUT &= ~BIT3;
            P1OUT &= ~BIT4;
            P1OUT &= ~BIT5;
        }
    }
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