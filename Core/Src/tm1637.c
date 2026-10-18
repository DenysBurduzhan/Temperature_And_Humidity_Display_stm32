
#include "tm1637.h"


#define TM1637_DELAY_US 10


static const uint8_t segmentMap[] = {
    0x3F,
    0x06,
    0x5B,
    0x4F,
    0x66,
    0x6D,
    0x7D,
    0x07,
    0x7F,
    0x6F
};


static GPIO_TypeDef *TM1637_CLK_PORT;
static uint16_t TM1637_CLK_PIN;
static GPIO_TypeDef *TM1637_DIO_PORT;
static uint16_t TM1637_DIO_PIN;


void TM1637_DelayUs(uint32_t us) {
    volatile uint32_t count = us * (SystemCoreClock / 1000000) / 8;
    while (count--) {}
}


void TM1637_SetDIOOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TM1637_DIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStruct);
}


void TM1637_SetDIOInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TM1637_DIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStruct);
}


void TM1637_Start(void) {
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_SET);
    TM1637_DelayUs(TM1637_DELAY_US);
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_RESET);
    TM1637_DelayUs(TM1637_DELAY_US);
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_RESET);
    TM1637_DelayUs(TM1637_DELAY_US);
}


void TM1637_Stop(void) {
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_RESET);
    TM1637_DelayUs(TM1637_DELAY_US);
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_SET);
    TM1637_DelayUs(TM1637_DELAY_US);
    HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, GPIO_PIN_SET);
}


void TM1637_WriteByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(TM1637_DIO_PORT, TM1637_DIO_PIN, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        TM1637_DelayUs(TM1637_DELAY_US);
        HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_SET);
        TM1637_DelayUs(TM1637_DELAY_US);
        data >>= 1;
    }

    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_RESET);
    TM1637_DelayUs(TM1637_DELAY_US);
    HAL_GPIO_WritePin(TM1637_CLK_PORT, TM1637_CLK_PIN, GPIO_PIN_SET);
    TM1637_DelayUs(TM1637_DELAY_US);
}


void TM1637_Init(GPIO_TypeDef *clkPort, uint16_t clkPin, GPIO_TypeDef *dioPort, uint16_t dioPin) {
    TM1637_CLK_PORT = clkPort;
    TM1637_CLK_PIN = clkPin;
    TM1637_DIO_PORT = dioPort;
    TM1637_DIO_PIN = dioPin;


    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = clkPin | dioPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(clkPort, &GPIO_InitStruct);
    TM1637_SetBrightness(7);
}

void TM1637_SetBrightness(uint8_t brightness) {
    if (brightness > 7) brightness = 7;
    TM1637_Start();
    TM1637_WriteByte(0x88 | brightness);
    TM1637_Stop();
}

void TM1637_DisplayDigit(uint8_t digit, uint8_t position) {
    if (digit > 9) digit = 0;
    TM1637_Start();
    TM1637_WriteByte(0x44);
    TM1637_Stop();
    TM1637_Start();
    TM1637_WriteByte(0xC0 + position);
    TM1637_WriteByte(segmentMap[digit]);
    TM1637_Stop();
}


void TM1637_ClearDisplay(void) {
    TM1637_Start();
    TM1637_WriteByte(0x44);
    TM1637_Stop();

    for (uint8_t position = 0; position < 4; position++) {
        TM1637_Start();
        TM1637_WriteByte(0xC0 + position);
        TM1637_WriteByte(0x00);
        TM1637_Stop();
    }
}


void TM1637_DisplayNumber(int16_t number) {
    if (number > 9999 || number < -999) return;

    uint8_t digits[4] = {0};
    uint8_t pos = 0;
    uint8_t isNegative = 0;

    if (number < 0) {
        isNegative = 1;
        number = -number;
    }


    TM1637_ClearDisplay();


    do {
        digits[pos++] = number % 10;
        number /= 10;
    } while (number > 0 && pos < 4);


    for (uint8_t i = 0; i < 4; i++) {
        uint8_t displayPos = 3 - i;
        TM1637_DisplayDigit(digits[i], displayPos);
    }
}
