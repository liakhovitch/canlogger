#include <stdint-gcc.h>
#include "storage.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "can.h"


extern struct circularBuffer buf1;
extern struct circularBuffer buf2;
FIL fil1;
FIL fil2;
static char can1[20] = "can1.csv";
static char can2[20] = "can2.csv";

//fres returns FR_OK(0) on
int init_storage(FATFS *FatFs){
	volatile FRESULT fres;
	HAL_Delay(100);
	fres = f_mount(FatFs, "", 1);
	if(fres != FR_OK){
		return fres;
	}
	fres = f_open(&fil1, can1, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	if(fres != FR_OK){
		return fres;
	}
	fres = f_open(&fil2, can2, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	if(fres != FR_OK){
		return fres;
	}
    find_eof();
	return 0;
}

//This clears the buffers and closes the file buffers
//Then runs until the circularBuffers are empty
//Only Run when you are finished recording CAN data
int flush_storage(){
    FRESULT fres;
    struct bufCell cell;
    int buf_state;

    uint16_t sid;
    uint32_t eid;
    uint8_t data[8];
    uint8_t dat_len;

    //Clears buf1
    fres = f_lseek(&fil1, f_size(&fil1));
    if(fres != FR_OK){
    	f_close(&fil1);
    	return fres;
    }
    else{

    	buf_state = buf_get(&buf1, &cell);
    	while(buf_state == 0){
    		parse_packet(&cell, &sid, &eid, data, &dat_len);

    		fres = f_printf(&fil1, "%u,%u,", sid, eid);

    		for(int i = 0; i < dat_len; i++){
    			fres = f_printf(&fil1, "%u,", data[i]);
    		}
    		f_printf(&fil1, "\n");
    		buf_state = buf_get(&buf1, &cell);
    	}
   	}

    //Clears buf2
    fres = f_lseek(&fil2, f_size(&fil2));
    if(fres != FR_OK){
    	f_close(&fil2);
    	return fres;
    }
    else{
    	buf_state = buf_get(&buf2, &cell);
    	while(buf_state == 0){
    		parse_packet(&cell, &sid, &eid, data, &dat_len);
    		fres = f_printf(&fil2, "%u,%u,", sid, eid);

    		for(int i = 0; i < dat_len; i++){
    			fres = f_printf(&fil2, "%u,", data[i]);
    		}
    		f_printf(&fil2, "\n");
    		buf_state = buf_get(&buf2, &cell);
    	}
    }


    f_close(&fil1);
    f_close(&fil2);


    return 0;
}


//Attempts to pop one value on the buffer for each buffer
//Assumes pointers are at the end of file
int pop_buf(){
    struct bufCell cell;
    int buf_state;

    uint16_t sid;
    uint32_t eid;
    uint8_t data[8];
    uint8_t dat_len;


    //Flush buffer
    //Assumes file is open and points to end of the file
	buf_state = buf_get(&buf1, &cell);
	if(buf_state == 0){
		parse_packet(&cell, &sid, &eid, data, &dat_len);

		f_printf(&fil1, "%u,%u,", sid, eid);

		for(int i = 0; i < dat_len; i++){
			f_printf(&fil1, "%u,", data[i]);
		}
		f_printf(&fil1, "\n");
	}

	buf_state = buf_get(&buf2, &cell);
	if(buf_state == 0){
		parse_packet(&cell, &sid, &eid, data, &dat_len);

		f_printf(&fil2, "%u,%u,", sid, eid);
		for(int i = 0; i < dat_len; i++){
			f_printf(&fil2, "%u,", data[i]);
		}
		f_printf(&fil2, "\n");
	}
	return 0;
}

int read_storage(uint8_t can_ch_slct, BYTE * buffer, UINT btr, UINT * br){
	FIL *fil;
	volatile FRESULT fres;

	if(can_ch_slct == 0){
		fil = &fil1;
	}
	else if(can_ch_slct ==1){
		fil = &fil2;
	}

	fres = f_read(fil, buffer, btr, br);
	return fres;
}


//File pointer must be open to work
uint32_t get_file_size(uint8_t can_ch_slct){
	//Simple f_size() wrapper;
	uint32_t file_size = 0;
	if(can_ch_slct == 0){
		file_size = f_size(&fil1);
	}
	else if(can_ch_slct == 1){
		file_size = f_size(&fil2);
	}
	return file_size;
}

int delete_file(uint8_t can_ch_slct){
	FRESULT fres;
    close_fil(can_ch_slct);
	if(can_ch_slct == 0){
		fres = f_unlink(can1);
	}
	else if(can_ch_slct == 1){
		fres = f_unlink(can2);
	}
    open_fil(can_ch_slct);
    find_eof();
	return fres;
}

int close_fil(unsigned int can_ch_slct){
    if(!can_ch_slct) f_close(&fil1);
	else f_close(&fil2);
	return 0;
}

int open_fil(unsigned int can_ch_slct){
	FRESULT fres;
    if(!can_ch_slct){
        fres = f_open(&fil1, can1, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
        if(fres != FR_OK){
            return fres;
        }
    }else{
        fres = f_open(&fil2, can2, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
        if(fres != FR_OK){
            return fres;
        }
    }
	return 0;
}

int find_eof(){
	FRESULT fres;
	fres = f_lseek(&fil1, f_size(&fil1));
	if(fres != FR_OK){
		f_close(&fil1);
		return fres;
	}
	fres = f_lseek(&fil2, f_size(&fil2));
	if(fres != FR_OK){
		f_close(&fil2);
		return fres;
	}
	return 0;
}

int demount_storage(){
    FRESULT fres;
    HAL_Delay(100);
	fres = f_mount(NULL, "", 0);
    return fres;
}

void handle_unmount() {
    if(HAL_GPIO_ReadPin(USR_BTN_GPIO_Port, USR_BTN_Pin)){
        can_panic();
        close_fil(0);
        close_fil(1);
        demount_storage();
        while (1) {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            HAL_Delay(20);
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            HAL_Delay(1000);
            if(!HAL_GPIO_ReadPin(USR_BTN_GPIO_Port, USR_BTN_Pin)){
                NVIC_SystemReset();
            }
        }
    }
}

// Generate some data and push it to buf1 and buf2. To be used for testing only.
void test_generate_data(){
    struct bufCell cell;
    uint8_t test_dat[8] = {2, 3, 5, 7, 11, 13, 17, 19};
    construct_packet(&cell, 69, 420, test_dat, 8);
    buf_put(&buf1, &cell);
    buf_put(&buf2, &cell);
}
