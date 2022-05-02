#include <string.h>
#include "bluetooth.h"
#include "usart.h"
#include "can.h"
#include "storage.h"

#define BT_BUF_LEN 50
#define SEND_BUF_LEN 1000

// Commands:
const char* cmd1 = "read can1\n";
const char* cmd2 = "read can2\n";
const char* cmd3 = "delete can1\n";
const char* cmd4 = "delete can2\n";
const char* cmd5 = "reset\n";
const char* cmd6 = "baud ";
const char* cmd7 = "can1 ";
const char* cmd8 = "can2 ";
const char* cmd9 = "250k\n";
const char* cmd10 = "500k\n";
const char* cmd11 = "1000k\n";

// Response messages:
const char *message1 = "Command {\n";
const char *message2 = "} unknown.\n";
const char *message3 = "ok\n";

_Atomic volatile unsigned int bt_flag = 0;
uint8_t bt_buf[BT_BUF_LEN];
uint16_t bt_recv_len = 0;
uint8_t send_buf[SEND_BUF_LEN];

int init_bt() {
    HAL_UARTEx_ReceiveToIdle_IT(&huart2, bt_buf, BT_BUF_LEN);
    return 0;
}

void read_bt_single(unsigned int channel){
    close_fil(channel);
    open_fil(channel);
    uint32_t filesize = get_file_size(channel);
    HAL_UART_Transmit(&huart2, (uint8_t *) &filesize + 3, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *) &filesize + 2, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *) &filesize + 1, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *) &filesize + 0, 1, HAL_MAX_DELAY);
    while(1){
        unsigned int br = 0;
        read_storage(channel, (BYTE*)send_buf, SEND_BUF_LEN, &br);
        HAL_UART_Transmit(&huart2, send_buf, br, HAL_MAX_DELAY);
        if(br < SEND_BUF_LEN) break;
    }
    find_eof();
}

void unknown_cmd(){
    HAL_UART_Transmit(&huart2, (uint8_t *) message1, strlen(message1), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, bt_buf, bt_recv_len, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *) message2, strlen(message2), HAL_MAX_DELAY);
}

int handle_bt() {
    if(bt_flag == 1) {
        can_panic();
        bt_flag = 0;
        if(bt_recv_len == strlen(cmd1) && strncmp((char*)bt_buf, cmd1, strlen(cmd1)) == 0){
            read_bt_single(0);
        }
        else if(bt_recv_len == strlen(cmd2) && strncmp((char*)bt_buf, cmd2, strlen(cmd2)) == 0){
            read_bt_single(1);
        }
        else if(bt_recv_len == strlen(cmd3) && strncmp((char*)bt_buf, cmd3, strlen(cmd3)) == 0){
            delete_file(0);
            HAL_UART_Transmit(&huart2, (uint8_t *) message3, strlen(message3), HAL_MAX_DELAY);
        }
        else if(bt_recv_len == strlen(cmd4) && strncmp((char*)bt_buf, cmd4, strlen(cmd4)) == 0){
            delete_file(1);
            HAL_UART_Transmit(&huart2, (uint8_t *) message3, strlen(message3), HAL_MAX_DELAY);
        }
        else if(bt_recv_len == strlen(cmd5) && strncmp((char*)bt_buf, cmd5, strlen(cmd5)) == 0){
            HAL_UART_Transmit(&huart2, (uint8_t *) message3, strlen(message3), HAL_MAX_DELAY);
            HAL_NVIC_SystemReset();
        }
        else if(bt_recv_len >= strlen(cmd6) && strncmp((char*)bt_buf, cmd6, strlen(cmd6)) == 0){
            uint8_t* cmd_new = bt_buf + strlen(cmd6);
            unsigned int recv_len_new = bt_recv_len - strlen(cmd6);
            int channel = -1;
            int cfg = -1;
            if(recv_len_new >= strlen(cmd7) && strncmp((char*)cmd_new, cmd7, strlen(cmd7)) == 0){
                cmd_new += strlen(cmd7);
                recv_len_new -= strlen(cmd7);
                channel = 0;
            }
            else if(recv_len_new >= strlen(cmd8) && strncmp((char*)cmd_new, cmd8, strlen(cmd8)) == 0){
                cmd_new += strlen(cmd8);
                recv_len_new -= strlen(cmd8);
                channel = 1;
            }
            if(channel != -1){
                if(recv_len_new == strlen(cmd9) && strncmp((char*)cmd_new, cmd9, strlen(cmd9)) == 0){
                    cfg = 0;
                }
                else if(recv_len_new == strlen(cmd10) && strncmp((char*)cmd_new, cmd10, strlen(cmd10)) == 0){
                    cfg = 1;
                }
                else if(recv_len_new == strlen(cmd11) && strncmp((char*)cmd_new, cmd11, strlen(cmd11)) == 0){
                    cfg = 2;
                }
                if(cfg != -1){
                    HAL_UART_Transmit(&huart2, (uint8_t *) message3, strlen(message3), HAL_MAX_DELAY);
                    set_can_cfg(channel, cfg);
                }
                else unknown_cmd();
            }
            else unknown_cmd();
        }
        else {
           unknown_cmd();
        }
        HAL_UARTEx_ReceiveToIdle_IT(&huart2, bt_buf, BT_BUF_LEN);
    }
    return 0;
}

void handle_bt_irq(uint16_t len) {
    bt_recv_len = len;
    bt_flag = 1;
}