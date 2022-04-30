#include <string.h>
#include "bluetooth.h"
#include "usart.h"
#include "can.h"

#define BT_BUF_LEN 50

// Commands:
const char* cmd1 = "read can1\n";
const char* cmd2 = "read can2\n";

// Response messages:
const char *message1 = "Command {\n";
const char *message2 = "} unknown.\n";
const char *test_msg1 = "Command understood.\n";

_Atomic volatile unsigned int bt_flag = 0;
uint8_t bt_buf[BT_BUF_LEN];
uint16_t bt_recv_len = 0;

int init_bt() {
    HAL_UARTEx_ReceiveToIdle_IT(&huart2, bt_buf, BT_BUF_LEN);
    return 0;
}

int handle_bt() {
    if(bt_flag == 1) {
        bt_flag = 0;
        if(bt_recv_len == strlen(cmd1) && strncmp((char*)bt_buf, cmd1, strlen(cmd1)) == 0){
            HAL_UART_Transmit(&huart2, (uint8_t *) test_msg1, strlen(test_msg1), HAL_MAX_DELAY);
        }
        else if(bt_recv_len == strlen(cmd2) && strncmp((char*)bt_buf, cmd2, strlen(cmd2)) == 0){
            HAL_UART_Transmit(&huart2, (uint8_t *) test_msg1, strlen(test_msg1), HAL_MAX_DELAY);
        }
        else {
            HAL_UART_Transmit(&huart2, (uint8_t *) message1, strlen(message1), HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, bt_buf, bt_recv_len, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, (uint8_t *) message2, strlen(message2), HAL_MAX_DELAY);
        }
        HAL_UARTEx_ReceiveToIdle_IT(&huart2, bt_buf, BT_BUF_LEN);
    }
    return 0;
}

void handle_bt_irq(uint16_t len) {
    bt_recv_len = len;
    bt_flag = 1;
}