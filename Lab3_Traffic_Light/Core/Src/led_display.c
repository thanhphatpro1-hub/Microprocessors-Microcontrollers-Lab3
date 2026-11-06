/*
 * led_display.c
 *
 *  Created on: Lab 3 - CO3009 (HCMUT)
 *  Description: Implementation of 12 Traffic LEDs & 4 Multiplexed 7-Segment LEDs
 */

#include "led_display.h"
#include "timer.h"

/*
 * Bảng mã 7 đoạn chuẩn cho các chữ số từ 0 đến 9:
 * Bit 0: đoạn a | Bit 1: đoạn b | Bit 2: đoạn c | Bit 3: đoạn d
 * Bit 4: đoạn e | Bit 5: đoạn f | Bit 6: đoạn g
 * Giá trị bit = 1 nghĩa là đoạn đó cần BẬT SÁNG (xuất mức SEG_ON).
 */
static const uint8_t sevenSegFont[10] = {
    0x3FU, /* Số 0: sáng a, b, c, d, e, f    */
    0x06U, /* Số 1: sáng b, c                */
    0x5BU, /* Số 2: sáng a, b, d, e, g       */
    0x4FU, /* Số 3: sáng a, b, c, d, g       */
    0x66U, /* Số 4: sáng b, c, f, g          */
    0x6DU, /* Số 5: sáng a, c, d, f, g       */
    0x7DU, /* Số 6: sáng a, c, d, e, f, g    */
    0x07U, /* Số 7: sáng a, b, c             */
    0x7FU, /* Số 8: sáng a, b, c, d, e, f, g */
    0x6FU  /* Số 9: sáng a, b, c, d, f, g    */
};

/* Bộ đệm dữ liệu hiển thị (Output Buffer) của 4 bóng LED 7 đoạn */
static uint8_t led_buffer[NO_OF_7SEG] = {0U, 0U, 0U, 0U};

/* Chỉ số bóng LED 7 đoạn hiện tại đang được quét (0 -> 1 -> 2 -> 3 -> 0) */
static uint8_t index_led = 0U;

/**
 * @brief  Tắt toàn bộ 12 LED giao thông trên cả 2 tuyến đường
 */
void clear_all_traffic_leds(void) {
    HAL_GPIO_WritePin(LED_RED1_GPIO_Port,   LED_RED1_Pin,   LED_OFF);
    HAL_GPIO_WritePin(LED_AMBER1_GPIO_Port, LED_AMBER1_Pin, LED_OFF);
    HAL_GPIO_WritePin(LED_GREEN1_GPIO_Port, LED_GREEN1_Pin, LED_OFF);

    HAL_GPIO_WritePin(LED_RED2_GPIO_Port,   LED_RED2_Pin,   LED_OFF);
    HAL_GPIO_WritePin(LED_AMBER2_GPIO_Port, LED_AMBER2_Pin, LED_OFF);
    HAL_GPIO_WritePin(LED_GREEN2_GPIO_Port, LED_GREEN2_Pin, LED_OFF);
}

/**
 * @brief  Thiết lập màu đèn sáng cho Tuyến đường 1 và Tuyến đường 2 ở Mode 1
 * @param  road1_color: Màu đèn của Tuyến đường 1 (COLOR_RED, COLOR_AMBER, COLOR_GREEN)
 * @param  road2_color: Màu đèn của Tuyến đường 2 (COLOR_RED, COLOR_AMBER, COLOR_GREEN)
 */
void set_traffic_leds(TrafficColor_t road1_color, TrafficColor_t road2_color) {
    /* Xuất trạng thái 3 màu đèn của Tuyến đường 1 */
    HAL_GPIO_WritePin(LED_RED1_GPIO_Port,   LED_RED1_Pin,   (road1_color == COLOR_RED)   ? LED_ON : LED_OFF);
    HAL_GPIO_WritePin(LED_AMBER1_GPIO_Port, LED_AMBER1_Pin, (road1_color == COLOR_AMBER) ? LED_ON : LED_OFF);
    HAL_GPIO_WritePin(LED_GREEN1_GPIO_Port, LED_GREEN1_Pin, (road1_color == COLOR_GREEN) ? LED_ON : LED_OFF);

    /* Xuất trạng thái 3 màu đèn của Tuyến đường 2 */
    HAL_GPIO_WritePin(LED_RED2_GPIO_Port,   LED_RED2_Pin,   (road2_color == COLOR_RED)   ? LED_ON : LED_OFF);
    HAL_GPIO_WritePin(LED_AMBER2_GPIO_Port, LED_AMBER2_Pin, (road2_color == COLOR_AMBER) ? LED_ON : LED_OFF);
    HAL_GPIO_WritePin(LED_GREEN2_GPIO_Port, LED_GREEN2_Pin, (road2_color == COLOR_GREEN) ? LED_ON : LED_OFF);
}

/**
 * @brief  Đảo trạng thái (Toggle) tất cả các bóng LED của một màu được chọn (dùng cho chớp 2Hz)
 * @param  color: Màu đèn cần chớp tắt (COLOR_RED, COLOR_AMBER, hoặc COLOR_GREEN)
 */
void toggle_traffic_color_2hz(TrafficColor_t color) {
    switch (color) {
        case COLOR_RED:
            /* Đảm bảo các đèn Vàng và Xanh luôn tắt khi đang ở Mode 2 */
            HAL_GPIO_WritePin(LED_AMBER1_GPIO_Port, LED_AMBER1_Pin, LED_OFF);
            HAL_GPIO_WritePin(LED_GREEN1_GPIO_Port, LED_GREEN1_Pin, LED_OFF);
            HAL_GPIO_WritePin(LED_AMBER2_GPIO_Port, LED_AMBER2_Pin, LED_OFF);
            HAL_GPIO_WritePin(LED_GREEN2_GPIO_Port, LED_GREEN2_Pin, LED_OFF);
            /* Đảo trạng thái cả 4 bóng LED Đỏ (kênh RED1 và kênh RED2) */
            HAL_GPIO_TogglePin(LED_RED1_GPIO_Port, LED_RED1_Pin);
            HAL_GPIO_TogglePin(LED_RED2_GPIO_Port, LED_RED2_Pin);
            break;

        case COLOR_AMBER:
            /* Đảm bảo các đèn Đỏ và Xanh luôn tắt khi đang ở Mode 3 */
            HAL_GPIO_WritePin(LED_RED1_GPIO_Port,   LED_RED1_Pin,   LED_OFF);
            HAL_GPIO_WritePin(LED_GREEN1_GPIO_Port, LED_GREEN1_Pin, LED_OFF);
            HAL_GPIO_WritePin(LED_RED2_GPIO_Port,   LED_RED2_Pin,   LED_OFF);
            HAL_GPIO_WritePin(LED_GREEN2_GPIO_Port, LED_GREEN2_Pin, LED_OFF);
            /* Đảo trạng thái cả 4 bóng LED Vàng (kênh AMBER1 và kênh AMBER2) */
            HAL_GPIO_TogglePin(LED_AMBER1_GPIO_Port, LED_AMBER1_Pin);
            HAL_GPIO_TogglePin(LED_AMBER2_GPIO_Port, LED_AMBER2_Pin);
            break;

        case COLOR_GREEN:
            /* Đảm bảo các đèn Đỏ và Vàng luôn tắt khi đang ở Mode 4 */
            HAL_GPIO_WritePin(LED_RED1_GPIO_Port,   LED_RED1_Pin,   LED_OFF);
            HAL_GPIO_WritePin(LED_AMBER1_GPIO_Port, LED_AMBER1_Pin, LED_OFF);
            HAL_GPIO_WritePin(LED_RED2_GPIO_Port,   LED_RED2_Pin,   LED_OFF);
            HAL_GPIO_WritePin(LED_AMBER2_GPIO_Port, LED_AMBER2_Pin, LED_OFF);
            /* Đảo trạng thái cả 4 bóng LED Xanh (kênh GREEN1 và kênh GREEN2) */
            HAL_GPIO_TogglePin(LED_GREEN1_GPIO_Port, LED_GREEN1_Pin);
            HAL_GPIO_TogglePin(LED_GREEN2_GPIO_Port, LED_GREEN2_Pin);
            break;

        default:
            clear_all_traffic_leds();
            break;
    }
}

/**
 * @brief  Giải mã và xuất 1 chữ số (0..9) ra 7 chân phân đoạn PA0..PA6
 * @param  num: Chữ số cần hiển thị (0..9)
 */
void display7SEG(uint8_t num) {
    if (num > 9U) {
        HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, SEG_OFF);
        HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, SEG_OFF);
        return;
    }

    uint8_t mask = sevenSegFont[num];

    HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, (mask & 0x01U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, (mask & 0x02U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, (mask & 0x04U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, (mask & 0x08U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, (mask & 0x10U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, (mask & 0x20U) ? SEG_ON : SEG_OFF);
    HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, (mask & 0x40U) ? SEG_ON : SEG_OFF);
}

/**
 * @brief  Quét chọn 1 trong 4 bóng LED 7 đoạn theo phương pháp đa hợp thời gian (TDM)
 * @param  index: Thứ tự bóng LED cần bật (0, 1, 2, hoặc 3)
 */
void update7SEG(uint8_t index) {
    /* BƯỚC 1: Tắt tất cả 4 chân Enable trước khi đổi dữ liệu Segment để chống bóng mờ (Anti-Ghosting) */
    HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, EN_OFF);
    HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, EN_OFF);
    HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, EN_OFF);
    HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, EN_OFF);

    /* BƯỚC 2: Xuất mã 7 đoạn của bóng tương ứng và bật duy nhất chân Enable của bóng đó */
    switch (index) {
        case 0U:
            display7SEG(led_buffer[0]);
            HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, EN_ON);
            break;
        case 1U:
            display7SEG(led_buffer[1]);
            HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, EN_ON);
            break;
        case 2U:
            display7SEG(led_buffer[2]);
            HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, EN_ON);
            break;
        case 3U:
            display7SEG(led_buffer[3]);
            HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, EN_ON);
            break;
        default:
            break;
    }
}

/**
 * @brief  Cập nhật 2 giá trị số có 2 chữ số (00..99) vào bộ đệm 4 bóng LED 7 đoạn
 * @param  road1_or_mode: Số hiển thị trên cặp LED 0 & 1 (Thời gian Đường 1 hoặc Số thứ tự Mode)
 * @param  road2_or_value: Số hiển thị trên cặp LED 2 & 3 (Thời gian Đường 2 hoặc Giá trị đang chỉnh)
 */
void updateClockBuffer(uint8_t road1_or_mode, uint8_t road2_or_value) {
    led_buffer[0] = (road1_or_mode / 10U) % 10U;
    led_buffer[1] = road1_or_mode % 10U;
    led_buffer[2] = (road2_or_value / 10U) % 10U;
    led_buffer[3] = road2_or_value % 10U;
}

/**
 * @brief  Thực thi quét luân phiên 4 bóng LED 7 đoạn mỗi khi TIMER_7SEG_SCAN hết hạn
 */
void led_7seg_scanning_run(void) {
    if (isTimerExpired(TIMER_7SEG_SCAN)) {
        setTimer(TIMER_7SEG_SCAN, TIME_7SEG_SCAN_MS);
        update7SEG(index_led);
        index_led = (index_led + 1U) % NO_OF_7SEG;
    }
}
