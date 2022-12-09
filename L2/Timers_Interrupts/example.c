#include <msp430.h>

/*  ���������� ����������  */
int i=0,period=30,p_1=15,p_0=0,step=1;

/*  ���������� �������  */

void main(void) {
        WDTCTL = WDTPW + WDTHOLD;    // ��������� ���������� ������

        P1DIR |= BIT0; // ��������� ���� 0 ����� 1 � ����� ������ (���������� �����������  LED1)
       // P1OUT&=~BIT0;

    //    P1OUT = 0;
        P1DIR &= ~BIT7; // ��������� ���� 7 ����� 1 � ����� ����� ( ������ s1)
        P1REN |= BIT7; // ��������� ��������� �������� ��� ���� 1.7
        P1OUT |= BIT7;

        P2DIR &= ~BIT2; // ��������� ���� 2.2 � ����� ����� ( ������  s2)
        P2REN |= BIT2; // ��������� ��������� �������� ��� ���� 2.2
        P2OUT |= BIT2;


        TA1CCR0 = 1250 - 1;    // ������ � 62,500 �����, �� 0 �� 62,499.
        TA1CCTL0 = CCIE;        // ��������� ���������� ������� �� ���������� �������� CCR0.
        TA1CTL = TASSEL__SMCLK | ID_3 | MC_1 | TACLR /*| TAIE*/; // ��������� ������ ������ ������� Timer_A:
                                        // TASSEL_2 - �������� ������ SMCLK (SubMainCLocK),
                                        // �� ��������� ����������� �� ������ �� DCO
                                        // ID_3 - �������� ������� �� 8, �� 1MHz ��� ����� 125kHz
                                        // MC_1 - ����� ������� ����� (�� TACCR0)
                                        // TACLR - ��������� ��������� �������

        P1IES |= BIT7; // ���������� �� �������� 1->0 ���������� ����� IESx = 1.
        P1IFG &= ~BIT7; // ��� �������������� ������������ ������ ����������,


        P2IES |= BIT2; // ���������� �� �������� 1->0 ���������� ����� IESx = 1.
        P2IFG &= ~BIT2; // ��� �������������� ������������ ������ ����������,
                        // �������� ��� ���� ��� P1.3 �� ���������� ����������
        P1IE |= BIT7;  // ��������� ���������� ��� P1.3
        P2IE |= BIT2;

        //_enable_interrupt();
        __bis_SR_register(LPM0_bits + GIE);

        while(1) {
            // ������ �� ������, ���� ����������. ��������� ����� ��� �������������
        }         // ��������� ������� ������ ����������������!
} // main

/*  ����������� ����������  */



/*  ����������� ����������  */
#pragma vector = PORT1_VECTOR
__interrupt void P1_7_ISR(void) {


        if(P1IFG & BIT7)
        {
            //P1IFG &= ~BIT7;  // �������� ���� ����������
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

            P1IFG &= ~BIT7;  // �������� ���� ����������
            return;

        }
        P1IFG = 0;// �������� � ���� ��� �������������, �� ��������
        return;

} // P1_7_ISR

#pragma vector = PORT2_VECTOR
__interrupt void P2_2_ISR(void) {


    if(P2IFG & BIT2)
    {
        //P2IFG &= ~BIT2;  // �������� ���� ����������
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
        P2IFG &= ~BIT2;  // �������� ���� ����������
    //  P2IFG &= ~BIT2;  // �������� ���� ����������
        return;

    }
    P2IFG = 0;// �������� � ���� ��� �������������, �� ��������
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
