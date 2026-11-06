/*
 * input_reading.c
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Button Reading, 3-Stage Debouncing, and >1s Hold Detection
 */

#include "input_reading.h"

/* Bảng tra cứu Port và Pin của 3 nút nhấn để quét bằng vòng lặp */
static GPIO_TypeDef* const buttonPort[NO_OF_BUTTONS] = {
    BUTTON_MODE_GPIO_Port,
    BUTTON_INC_GPIO_Port,
    BUTTON_SET_GPIO_Port
};

static const uint16_t buttonPin[NO_OF_BUTTONS] = {
    BUTTON_MODE_Pin,
    BUTTON_INC_Pin,
    BUTTON_SET_Pin
};

/* Bộ đệm lưu kết quả cuối cùng của nút nhấn sau khi đã lọc chống dội */
static GPIO_PinState buttonBuffer[NO_OF_BUTTONS] = {
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED
};

/* Ba tầng bộ đệm phục vụ lọc chống dội phím (3-stage Debouncing Buffers) */
static GPIO_PinState debounceButtonBuffer0[NO_OF_BUTTONS] = {
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED
};

static GPIO_PinState debounceButtonBuffer1[NO_OF_BUTTONS] = {
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED
};

static GPIO_PinState debounceButtonBuffer2[NO_OF_BUTTONS] = {
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED,
    BUTTON_IS_RELEASED
};

/* Cờ báo hiệu nút nhấn đã được giữ liên tục trên 1 giây */
static uint8_t flagForButtonPress1s[NO_OF_BUTTONS] = {0U, 0U, 0U};

/* Bộ đếm số lần ngắt khi nút nhấn đang được giữ */
static uint16_t counterForButtonPress1s[NO_OF_BUTTONS] = {0U, 0U, 0U};

/**
 * @brief  Đọc trạng thái 3 nút nhấn và lọc chống dội 3 tầng.
 *         Hàm được gọi định kỳ trong ngắt Timer 2 (HAL_TIM_PeriodElapsedCallback).
 */
void button_reading(void) {
    for (uint8_t i = 0U; i < NO_OF_BUTTONS; i++) {
        /* Dịch chuyển lịch sử mẫu đọc qua 3 tầng đệm */
        debounceButtonBuffer2[i] = debounceButtonBuffer1[i];
        debounceButtonBuffer1[i] = debounceButtonBuffer0[i];
        debounceButtonBuffer0[i] = HAL_GPIO_ReadPin(buttonPort[i], buttonPin[i]);

        /* Chỉ chấp nhận trạng thái mới khi cả 3 lần lấy mẫu liên tiếp đều đồng nhất */
        if ((debounceButtonBuffer0[i] == debounceButtonBuffer1[i]) &&
            (debounceButtonBuffer1[i] == debounceButtonBuffer2[i])) {

            buttonBuffer[i] = debounceButtonBuffer0[i];

            if (buttonBuffer[i] == BUTTON_IS_PRESSED) {
                /* Nếu nút đang được nhấn giữ, đếm tiến tới ngưỡng 1 giây */
                if (counterForButtonPress1s[i] < DURATION_FOR_AUTO_INCREASING) {
                    counterForButtonPress1s[i]++;
                } else {
                    /* Đã nhấn giữ đủ 1 giây -> Bật cờ báo nhấn giữ 1s */
                    flagForButtonPress1s[i] = 1U;
                }
            } else {
                /* Nút đã nhả ra -> Đặt lại bộ đếm và xóa cờ nhấn giữ 1s */
                counterForButtonPress1s[i] = 0U;
                flagForButtonPress1s[i]    = 0U;
            }
        }
    }
}

/**
 * @brief  Kiểm tra xem một nút nhấn có đang được nhấn hay không (Program 1.3)
 * @param  index: Chỉ số của nút nhấn (0: MODE, 1: INC, 2: SET)
 * @retval 1 nếu đang được nhấn, 0 nếu đang nhả hoặc vượt quá chỉ số
 */
unsigned char is_button_pressed(unsigned char index) {
    if (index >= NO_OF_BUTTONS) {
        return 0U;
    }
    return (buttonBuffer[index] == BUTTON_IS_PRESSED) ? 1U : 0U;
}

/**
 * @brief  Kiểm tra xem một nút nhấn có được giữ quá 1 giây hay không (Program 1.4)
 * @param  index: Chỉ số của nút nhấn (0: MODE, 1: INC, 2: SET)
 * @retval 1 nếu nhấn giữ > 1s, 0 nếu chưa đủ 1s, 0xFF nếu vượt quá chỉ số
 */
unsigned char is_button_pressed_1s(unsigned char index) {
    if (index >= NO_OF_BUTTONS) {
        return 0xFFU;
    }
    return (flagForButtonPress1s[index] == 1U) ? 1U : 0U;
}
