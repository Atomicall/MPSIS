#include <msp430.h>
#include "HAL_Dogs102x6.h"
#include "ff.h"
#include "structure.h"
#include "CTS_Layer.h"
#include <stdlib.h>

/* ������� ��� �������������� � GPIO */

#define GPIO_DIR_OUTPUT(...)  GPIO_DIR_OUTPUT_SUB(__VA_ARGS__)
#define GPIO_DIR_OUTPUT_SUB(port, pin) (P##port##DIR |= (1 << (pin)))

#define GPIO_DIR_INPUT(...)  GPIO_DIR_INPUT_SUB(__VA_ARGS__)
#define GPIO_DIR_INPUT_SUB(port, pin) (P##port##DIR &= ~(1 << (pin)))

#define GPIO_PULLUP(...) GPIO_PULLUP_SUB(__VA_ARGS__)
#define GPIO_PULLUP_SUB(port, pin) P##port##REN |= (1 << (pin)); \
                                   P##port##OUT |= (1 << (pin))
#define GPIO_PULLDOWN(...) GPIO_PULLDOWN_SUB(__VA_ARGS__)
#define GPIO_PULLDOWN_SUB(port, pin) P##port##REN |= (1 << (pin)); \
                                     P##port##OUT &= ~(1 << (pin))

#define GPIO_NOPULL(...) GPIO_NOPULL_SUB(__VA_ARGS__)
#define GPIO_NOPULL_SUB(port, pin) (P##port##REN &= ~(1 << (pin)))

#define GPIO_READ_PIN(...) GPIO_READ_PIN_SUB(__VA_ARGS__)
#define GPIO_READ_PIN_SUB(port, pin) ((P##port##IN & (1 << (pin))) ? 1 : 0)

#define GPIO_WRITE_PIN(...) GPIO_WRITE_PIN_SUB(__VA_ARGS__)
#define GPIO_WRITE_PIN_SUB(port, pin, value) (P##port##OUT = (P##port##OUT & ~(1 << (pin))) | (value << (pin)))

#define GPIO_TOGGLE_PIN(...) GPIO_TOGGLE_PIN_SUB(__VA_ARGS__)
#define GPIO_TOGGLE_PIN_SUB(port, pin) (P##port##OUT ^= (1 << (pin)))

#define GPIO_TRIG_EDGE_FALLING(...) GPIO_TRIG_EDGE_FALLING_SUB(__VA_ARGS__)
#define GPIO_TRIG_EDGE_FALLING_SUB(port, pin) (P##port##IES |= (1 << (pin)))

#define GPIO_TRIG_EDGE_RISING(...) GPIO_TRIG_EDGE_RISING_SUB(__VA_ARGS__)
#define GPIO_TRIG_EDGE_RISING_SUB(port, pin) (P##port##IES &= ~(1 << (pin)))

#define GPIO_INTERRUPT_ENABLE(...) GPIO_INTERRUPT_ENABLE_SUB(__VA_ARGS__)
#define GPIO_INTERRUPT_ENABLE_SUB(port, pin) P##port##IFG &= ~(1 << (pin)); \
                                             P##port##IE |= (1 << (pin))

#define GPIO_INTERRUPT_DISABLE(...) GPIO_INTERRUPT_DISABLE_SUB(__VA_ARGS__)
#define GPIO_INTERRUPT_DISABLE_SUB(port, pin) (P##port##IE &= ~(1 << (pin)))

#define GPIO_PERIPHERAL(...) GPIO_PERIPHERAL_SUB(__VA_ARGS__)
#define GPIO_PERIPHERAL_SUB(port, pin) (P##port##SEL |= (1 << (pin)))

#define GPIO_CLEAR_IT_FLAG(...) GPIO_CLEAR_IT_FLAG_SUB(__VA_ARGS__)
#define GPIO_CLEAR_IT_FLAG_SUB(port, pin) (P##port##IFG &= ~(1 << (pin)))

/********************************************************************/

// �������� ������� PAD1
const struct Element PAD1 =
{   //CB0
    .inputBits = CBIMSEL_0,
    .maxResponse = 250,
    .threshold = 125
};
// �������� ������� PAD5
const struct Element PAD5 =
{   //CB4
    .inputBits = CBIMSEL_4,
    .maxResponse = 1900,
    .threshold = 475
};







void read_termo_sensor() {
//  Cma3000_zAccel = Cma3000_readRegister(DOUTZ);
//  __delay_cycles(50 * 25);
//  int16_t tmp = (int16_t)(80 - (Cma3000_zAccel * M_PI / 2));
//  if (tmp < 0) {
//    termoSensor = 0;
//  } else if (tmp > 180) {
//    termoSensor = 180;
//  } else {
//    termoSensor = (uint16_t) tmp;
//  }

    while (ADC12CTL0 & ADC12BUSY);
    ADC12CTL0 |= ADC12ENC | ADC12SC;
    int val_TERM = ADC12MEM0;
    termoSensor = (uint16_t)val_TERM;

}

void setupADC()
{
// Turn on ADC and enable multiple conversions
ADC12CTL0 = ADC12SHT0_2 | ADC12ON | ADC12MSC | ADC12REFON | ADC12REF2_5V;
// Sampling timer, single sequence
ADC12CTL1 |= ADC12SHP | ADC12CONSEQ_1;
//ADC12CTL1 |= ADC12SSEL0 | ADC12SSEL1; //adc clock select (SMCLK)
ADC12CTL2 &= ~ADC12SR; //set sample rate buffer to 200ksps
// 12-bit conversion
ADC12CTL2 |= ADC12RES_2;
// Enable ADC interrupt on MEM1
ADC12IE |= ADC12IE1;
// A10 - termal
ADC12MCTL0 |= ADC12INCH_10 | SELREF_0 | ADC12EOS;
}







// C�������� �������� PAD1 ��� ���������� CTS_Layer.h
const struct Sensor Sensor1 =
{
    .halDefinition = RO_COMPB_TA1_TA0,
    .numElements = 1,
    .baseOffset = 0,
    .cbpdBits = 0x0001, //CB0
    .arrayPtr[0] = &PAD1,
    .cboutTAxDirRegister = (uint8_t *)&P1DIR,
    .cboutTAxSelRegister = (uint8_t *)&P1SEL,
    .cboutTAxBits = BIT6, // P1.6
    // ���������� �������
    .measGateSource = TIMER_ACLK,
    .sourceScale = TIMER_SOURCE_DIV_0,
    /*  50 ACLK/1 ������ or 50*1/32Khz = 1.5ms  */
    .accumulationCycles = 50
};

// C�������� �������� PAD5 ��� ���������� CTS_Layer.h
const struct Sensor Sensor5 =
{
    .halDefinition = RO_COMPB_TA1_TA0,
    .numElements = 1,
    .baseOffset = 4,
    .cbpdBits = 0x0010, //CB4
    .arrayPtr[0] = &PAD5,
    .cboutTAxDirRegister = (uint8_t *)&P1DIR,
    .cboutTAxSelRegister = (uint8_t *)&P1SEL,
    .cboutTAxBits = BIT6, // P1.6
    // ���������� �������
    .measGateSource = TIMER_ACLK,
    .sourceScale = TIMER_SOURCE_DIV_0,
    /*  50 ACLK/1 ������ or 272*1/32Khz = 8.5ms  */
    .accumulationCycles = 272
};

/***************************************************/

#define LED_ON  1
#define LED_OFF 0

/* ����� � ���� ������������ ������� */

#define LED1_PORT   1
#define LED1_PIN    0

#define BUTT2_PORT  2
#define BUTT2_PIN   2

// ������ � ������� ��� ��������� �������������� ����������
#define DRAW_TEXT_ROW 7
#define INFO_TEXT "PRESSED UNPRESSED"
#define LINE_Y  53

// ��� ����� ��� �������� ������
#define FILE_NAME "buffer.bin"

// �����
#define BUFFER_SIZE  50
#define BUFFER_COUNT (BUFFER_SIZE / 2)
volatile uint16_t buffer[BUFFER_COUNT];
volatile uint8_t index = 0;

// ����������������� ������� ��� ��������� PAD5 �� CTS_HAL
void TI_CTS_RO_COMPB_TA1_TA0_HAL_CUSTOM(const struct Sensor *group, uint8_t index)
{
    // ������������� ��� CBOUT.
    *(group->cboutTAxDirRegister) |= (group->cboutTAxBits);
    *(group->cboutTAxSelRegister) |= (group->cboutTAxBits);

    // �������� �������� ���������� Vcc,
    // Vcc*(0x18+1)/32 for CBOUT = 1 and Vcc*((0x04+1)/32 for CBOUT = 0 (����� �������� ������������ � CTS_HAL)
    CBCTL2 = CBRS_1 + CBREF14 + CBREF13 + CBREF02;
    // ��������� ������� ����� �����
    CBCTL3 |= (group->cbpdBits);

    // TimerA2 ������������ ��� ������� ������� ���������
    TA2CCR0 = group->accumulationCycles;
    TA2CTL = group->measGateSource + group->sourceScale;
    // �������� ����������
    CBCTL1 = CBON;

    // �������� ���� ������������ ���� �����������
    CBCTL0 = CBIMEN + (group->arrayPtr[index])->inputBits;

    // ������ A1 ������������ ��� ��������� �������������� ������ �������, ������� ���������� � TACLK.
    // �������� TACLK, ����������� ����.
    TA1CTL = TASSEL__TACLK+MC__CONTINUOUS+TACLR;
    // ���������� ���� ����������
    TA1CTL &= ~TAIFG;
    // ������ ������� 2
    TA2CTL |= (TACLR + MC__UP);
}

// ���������� �������� ��� ���������
uint16_t Get_Draw_Value(uint8_t index)
{
   uint16_t data_range = PAD5.maxResponse - PAD5.threshold;
   int16_t value = buffer[index] - PAD5.threshold;
   if (value < 0)
   {
       value = 0;
   }

   uint16_t draw_value = (uint16_t)((float)DOGS102x6_X_SIZE * (float)value / (float)data_range);
   return draw_value;

}

uint16_t main(void)
{
    // ��������� ����������� �������
    WDTCTL = WDTPW + WDTHOLD;

    FATFS fs;
    FIL file;

    GPIO_DIR_OUTPUT(LED1_PORT, LED1_PIN);
    GPIO_WRITE_PIN(LED1_PORT, LED1_PIN, LED_OFF);
    GPIO_DIR_INPUT(BUTT2_PORT, BUTT2_PIN);
    GPIO_PULLUP(BUTT2_PORT, BUTT2_PIN);

    // ������������� ������
    Dogs102x6_init();
    Dogs102x6_backlightInit();
    Dogs102x6_setBacklight(255);
    Dogs102x6_clearScreen();
    Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
//    Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, INFO_TEXT, DOGS102x6_DRAW_NORMAL);

    // ������������� ������� �������� ��� ��������� ������
    TI_CAPT_Init_Baseline(&Sensor1);
    TI_CAPT_Update_Baseline(&Sensor1,5);
    TI_CAPT_Init_Baseline(&Sensor5);
    TI_CAPT_Update_Baseline(&Sensor5,5);

    // ������������ �����
    FRESULT res = f_mount(0, &fs);
    if (res == FR_NO_FILESYSTEM)
    {
        f_mkfs(0, 0, 512);
    }

    // ������ DMA �� ���������� TA2CCR0 CCIFG(�� ��������)
    DMACTL0 = DMA0TSEL_5;
    // ��������� ���������, ��������� DMA, ���������� ����������
    // ��� ���������� dst, src, ������ ������ 16 ���.
    DMA0CTL = DMADT_0+DMAEN+DMAIE;
    // ������ = 1
    DMA0SZ = 1;

    // �������� - �������� �������� TA1.
    __data16_write_addr((unsigned short) &DMA0SA,(unsigned long) &TA1R);

    // ���������� - � ������� �������.
    __data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &buffer[index]);
    TI_CTS_RO_COMPB_TA1_TA0_HAL_CUSTOM(&Sensor5, 0);

    uint8_t first_press_PAD = 0;
    uint8_t no_press_PAD = 0;
    uint8_t first_press_S2 = 0;
    uint8_t no_press_S2 = 0;
    uint8_t file_draw = 0;
    UINT bw = 0;
    while (1)
    {

        // ���� � ����� LPMO � ����������� ����������
        __bis_SR_register(LPM0_bits+GIE);

        // ������ �������� �� �����
        if (file_draw == 0)
        {
            uint16_t draw_value = Get_Draw_Value(index);
            uint8_t x_pos = index * 2;
            if (x_pos % 16 == 0)
            {
                if (x_pos == 48)
                {
                    Dogs102x6_clearRow(x_pos / 8);
                    Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
                }
                else
                {
                    Dogs102x6_clearRow(x_pos / 8);
                    Dogs102x6_clearRow((x_pos / 8 + 1) % 8);

                }
            }
            Dogs102x6_pixelDraw(draw_value, x_pos, DOGS102x6_DRAW_NORMAL);
            Dogs102x6_pixelDraw(draw_value, x_pos + 1, DOGS102x6_DRAW_NORMAL);

            uint8_t string[128];
            uint16_t val = buffer[index];
            uint8_t i = 0;
            while(val > 0)
            {
                string[i++] = val % 10 + '0';
                val/= 10;
            }
            string[i] = '\0';
            i--;
            uint8_t j = 0;
            for (i; i >= j; i--)
            {
                uint8_t temp = string[i];
                string[i] = string[j];
                string[j] = temp;
                j++;
            }
//            itoa(10, string, 127);
            Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, string, DOGS102x6_DRAW_NORMAL);
        }

        // ��������� �������� ������ �� ���� �� ������� PAD2 � ������� �� �����
        struct Element * keypressed = 0;
        keypressed = (struct Element *)TI_CAPT_Buttons(&Sensor1);
        if (keypressed == 0)
        {
            no_press_PAD = 0;
        }
        if(keypressed && no_press_PAD == 0)
        {
            if (first_press_PAD == 0)
            {
                first_press_PAD = 1;
            }
            else if(first_press_PAD == 1)
            {
                GPIO_WRITE_PIN(LED1_PORT, LED1_PIN, LED_ON);
                no_press_PAD = 1;
                first_press_PAD = 0;
                file_draw ^= 1;
                if (file_draw)
                {
                    f_open(&file, FILE_NAME, FA_READ);
                    f_read(&file, buffer, BUFFER_SIZE, &bw);

                    Dogs102x6_clearScreen();
                    Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
                    Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, INFO_TEXT, DOGS102x6_DRAW_NORMAL);
                    f_close(&file);
                    uint16_t i = 0;
                    for (i = 0; i < BUFFER_COUNT; i++)
                    {
                        uint16_t draw_value = Get_Draw_Value(i);
                        Dogs102x6_pixelDraw(draw_value, 2 * i, DOGS102x6_DRAW_NORMAL);
                        Dogs102x6_pixelDraw(draw_value, 2 * i + 1, DOGS102x6_DRAW_NORMAL);
                    }
                }
                else
                {
                    Dogs102x6_clearScreen();
                    Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
                    Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, INFO_TEXT, DOGS102x6_DRAW_NORMAL);
                }
                GPIO_WRITE_PIN(LED1_PORT, LED1_PIN, LED_OFF);
            }
        }
        // ������� �������� ������ � ���� �� ������� S2
        uint8_t value_S2 = !GPIO_READ_PIN(BUTT2_PORT, BUTT2_PIN);

        if (value_S2 == 0)
        {
          no_press_S2 = 0;
        }
        if(value_S2 && no_press_S2 == 0)
        {
            if (first_press_S2 == 0)
            {
              first_press_S2 = 1;
            }
            else if(first_press_S2 == 1)
            {
                no_press_S2 = 1;
                first_press_S2 = 0;
                GPIO_WRITE_PIN(LED1_PORT, LED1_PIN, LED_ON);
                f_open(&file, FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
                f_write(&file, buffer, BUFFER_SIZE, &bw);
                f_close(&file);
                GPIO_WRITE_PIN(LED1_PORT, LED1_PIN, LED_OFF);
            }

        }

        index++;
        if (index == BUFFER_COUNT)
        {
            index = 0;
        }
        // ��������� ����� ���������� DMA
        __data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &buffer[index]);
        // ������ DMA
        DMA0CTL |= DMAEN;

        // ������ A1 ������������ ��� ��������� �������������� ������ �������, ������� ���������� � TACLK.
        // �������� TACLK, ����������� ����.
        TA1CTL = TASSEL__TACLK+MC__CONTINUOUS+TACLR;
        // ���������� ���� ����������
        TA1CTL &= ~TAIFG;
        // ������ ������� 2
        TA2CTL |= (TACLR + MC__UP);

    }
}

#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
    switch(__even_in_range(DMAIV,16))
    {
        // ���������� DMA0IFG
        case 2:
            // ��������� ��������
            TA1CTL &= ~MC__CONTINUOUS;
            TA2CTL &= ~MC__UP;
            // ����� �� LPM0
            _bic_SR_register_on_exit(LPM0_bits);
            break;
        default: break;
    }
}
