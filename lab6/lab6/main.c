#include <msp430.h>
#include "HAL_Dogs102x6.h"
#include "HAL_Cma3000.h"
#include "ff.h"
#include "structure.h"
#include "CTS_Layer.h"
#include <stdlib.h>
#include <math.h>
/* Функция для взаимодействия с GPIO */
#define GPIO_DIR_INPUT(...) GPIO_DIR_INPUT_SUB(__VA_ARGS__)
#define GPIO_DIR_INPUT_SUB(port, pin) (P##port##DIR &= ~(1 << (pin)))
#define GPIO_PULLUP(...) GPIO_PULLUP_SUB(__VA_ARGS__)
#define GPIO_PULLUP_SUB(port, pin) P##port##REN |= (1 << (pin)); \
P##port##OUT |= (1 << (pin))
#define GPIO_READ_PIN(...) GPIO_READ_PIN_SUB(__VA_ARGS__)
#define GPIO_READ_PIN_SUB(port, pin) ((P##port##IN & (1 << (pin))) ? 1 : 0)
#define GPIO_WRITE_PIN(...) GPIO_WRITE_PIN_SUB(__VA_ARGS__)
#define GPIO_WRITE_PIN_SUB(port, pin, value) (P##port##OUT = (P##port##OUT & ~(1 << (pin))) | (value << (pin)))
//описание свойств PAD2
const struct Element PAD2 =
{ //CB0
.inputBits = CBIMSEL_1,
.maxResponse = 250,
.threshold = 125
};
//структура описания PAD2 для библиотеки CTS_Layer.h
const struct Sensor Sensor1 =
{
.halDefinition = RO_COMPB_TA1_TA0,
.numElements = 1,
.baseOffset = 1,
.cbpdBits = 0x0001, //CB1 ????
.arrayPtr[0] = &PAD2,
.cboutTAxDirRegister = (uint8_t *)&P1DIR,
.cboutTAxSelRegister = (uint8_t *)&P1SEL,
.cboutTAxBits = BIT6, // P1.6
// информация таймера
.measGateSource = TIMER_ACLK,
.sourceScale = TIMER_SOURCE_DIV_0,
/* 50 ACLK/1 циклов or 50*1/32Khz = 1.5ms */
.accumulationCycles = 50
};
//кнопка S1
#define S1_PORT 1
#define S1_PIN 7
// данные и позиции для рисования дополнительной информации
#define DRAW_TEXT_ROW 7
#define LINE_Y 45
// имя файла для хранения буфера
#define FILE_NAME "buffer.bin"
//буфер
#define BUFFER_SIZE 80
#define BUFFER_COUNT (BUFFER_SIZE / 2)
volatile uint16_t buffer[BUFFER_COUNT];
volatile uint8_t index = 0;
//отслеживание состояния нажатия кнопок и файла
uint8_t first_press_PAD = 0;
uint8_t no_press_PAD = 0;
uint8_t first_press_S1 = 0;
uint8_t no_press_S1 = 0;
uint8_t file_draw = 0;
UINT bw = 0;
//значение оси Z акселерометра
uint16_t termo_sensor_value = 0;
//получаем данные термодатчика
void read_termo_sensor()
{
    while (ADC12CTL0 & ADC12BUSY);
        ADC12CTL0 |= ADC12ENC | ADC12SC;
        int val_TERM = ADC12MEM0;
termo_sensor_value =(uint16_t)(val_TERM / 17.7);
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

// вычисления значения для рисования
uint16_t get_draw_value(uint8_t index)
{
uint16_t draw_value = (uint16_t)((float)45 * (float)buffer[index] / 180);
return draw_value;
}
void draw()
{
uint16_t val = buffer[index];
char str[40] = "";
int8_t i = 1;
do {
str[i++] = (char)(val % 10 + '0');
val = val / 10;
} while (val > 0 && index < 40);
uint8_t j = 0;
for (i; i >= j; i--) {
char temp = str[i];
str[i] = str[j];
str[j] = temp;
j++;
}
str[0] = 'T';
Dogs102x6_clearRow(6);
Dogs102x6_stringDraw(DRAW_TEXT_ROW - 1, 0, str, DOGS102x6_DRAW_NORMAL);
}
void draw_from_file(FIL file)
{
no_press_PAD = 1;
first_press_PAD = 0;
file_draw ^= 1;
if (file_draw)
{
f_open(&file, FILE_NAME, FA_READ);
f_read(&file, buffer, BUFFER_SIZE, &bw);
char str[40] = "Loaded from file";
Dogs102x6_clearScreen();
Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, str, DOGS102x6_DRAW_NORMAL);
f_close(&file);
uint16_t i = 0;
for (i = 0; i < BUFFER_COUNT; i++)
{
uint16_t draw_value = get_draw_value(i);
Dogs102x6_pixelDraw(i * 2, 45 - draw_value, DOGS102x6_DRAW_NORMAL);
Dogs102x6_pixelDraw(i * 2 + 1, 45 - draw_value, DOGS102x6_DRAW_NORMAL);
}
} else {
Dogs102x6_clearScreen();
Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
}
}
void write_to_file(FIL file)
{
if (first_press_S1 == 0)
first_press_S1 = 1;
else if(first_press_S1 == 1)
{
no_press_S1 = 1;
first_press_S1 = 0;
f_open(&file, FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
f_write(&file, buffer, BUFFER_SIZE, &bw);
f_close(&file);
char str[40] = "Saved to file";
Dogs102x6_clearScreen();
Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
Dogs102x6_stringDraw(DRAW_TEXT_ROW, 0, str, DOGS102x6_DRAW_NORMAL);
}
}
uint16_t main(void)
{
//остановка сторожевого таймера
WDTCTL = WDTPW + WDTHOLD;
//инициализация S1
GPIO_DIR_INPUT(S1_PORT, S1_PIN);
GPIO_PULLUP(S1_PORT, S1_PIN);
//настройка АЦП
setupADC();
//инициализация экрана
Dogs102x6_init();
Dogs102x6_backlightInit();
Dogs102x6_setBacklight(255);
Dogs102x6_clearScreen();
Dogs102x6_horizontalLineDraw(0, DOGS102x6_X_SIZE - 1, LINE_Y, DOGS102x6_DRAW_NORMAL);
// Инициализация базовых значения для сенсорных кнопок
TI_CAPT_Init_Baseline(&Sensor1);
TI_CAPT_Update_Baseline(&Sensor1,5);
//инициализация акселерометра
Cma3000_init();
FATFS fs; //файловая система
FIL file; //файл для буфера
// монтирование диска
FRESULT res = f_mount(0, &fs);
if (res == FR_NO_FILESYSTEM) {
f_mkfs(0, 0, 512);
}
//инициализация DMA
DMACTL0 = DMA0TSEL_5; // пересылка по TA2CCR0.IFG
// одиночная пересылка, включение DMA, разрешение прерываний
// нет инкремента dst, src, размер данных 16 бит.
DMA0CTL = DMADT_0+DMAEN+DMAIE;
// Размер = 1
DMA0SZ = 1;
// DMA0SA - источник (ось Z акселерометра)
__data16_write_addr((unsigned short) &DMA0SA,(unsigned long) &termo_sensor_value);
// DMA0DA - назначение (элемент буфера)
__data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &buffer[index]);
TA2CCR0 = 200;
TA2CTL = TIMER_ACLK + TIMER_SOURCE_DIV_0;
TA2CTL |= (TACLR + MC__UP);
while (1)
{
//вход в режим LPMO с разрешением прерываний
__bis_SR_register(LPM0_bits+GIE);
//рисуем значение на экран
if (file_draw == 0)
{
draw();
}
read_termo_sensor();
//считываем значения буфера из файл по нажатию PAD2 и выводим на экран
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
draw_from_file(file);
}
}
//заносим значения буфера в файл по нажатию S1
uint8_t value_S1 = !GPIO_READ_PIN(S1_PORT, S1_PIN);
if (value_S1 == 0)
{
no_press_S1 = 0;
}
if(value_S1 && no_press_S1 == 0)
{
write_to_file(file);
}
index++;
if (index == BUFFER_COUNT)
{
index = 0;
}
//обновляем адрес назначения DMA
__data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &buffer[index]);
//запуск DMA
DMA0CTL |= DMAEN;
//запуск Таймера 2
TA2CTL |= (TACLR + MC__UP);
}
}
#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
switch(__even_in_range(DMAIV,16))
{
//прерывание DMA0IFG
case 2:
TA2CTL &= ~MC__UP; //остановка таймера
_bic_SR_register_on_exit(LPM0_bits); // выход из LPM0
break;
default: break;
}
}
