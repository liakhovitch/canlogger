#include <stdint-gcc.h>
#include "storage.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


extern struct circularBuffer buf1;
extern struct circularBuffer buf2;

//fres returns FR_OK(0) on
int init_storage(FATFS *FatFs){
	FRESULT fres;
	HAL_Delay(100);
	fres = f_mount(FatFs, "", 1);
	return fres;
}

//This bare bones opens two discrete files one for each CAN
//Then runs until the circularBuffers are empty
int flush_storage(FATFS *FatFs){
    FRESULT fres;
    FIL fil1;
    FIL fil2;
    struct bufCell cell;
    int buf_state;

    uint16_t sid;
    uint16_t eid;
    uint8_t data[8];
    uint8_t dat_len;

    //Clears buf1
    fres = f_open(&fil1, "can1.csv", FA_WRITE | FA_OPEN_ALWAYS);
    fres = f_open(&fil2, "can2.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil1, f_size(&fil1));
    	if(fres != FR_OK){
    		f_close(&fil1);
    		return fres;
    	}
    	else{
    	//Perform file write here.
    	//Finish by calling f_close on fil
    		buf_state = buf_get(&buf1, &cell);
    		while(buf_state == 0){
    			parse_packet(&cell, &sid, &eid, data, &dat_len);
    			fres = f_printf(&fil1, "%4x, %4x,", sid, eid);
    			for(int i = 0; i < dat_len; i++){
    				fres = f_printf(&fil1, "%2x", data[i]);
    			}
    			f_printf(&fil1, ",\n");
    			buf_state = buf_get(&buf1, &cell);
    		}

    	}
    }
    //Clears buf2
    f_close(&fil1);
    fres = f_open(&fil2, "can2.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil2, f_size(&fil2));
    	if(fres != FR_OK){
    		f_close(&fil2);
    		return fres;
    	}
    	else{
    	//Perform file write here.
    	//Finish by calling f_close on fil
    	buf_state = buf_get(&buf2, &cell);
    	f_printf(&fil2, "%d, \n", buf_state);
    	while(buf_state == 0){
    		parse_packet(&cell, &sid, &eid, data, &dat_len);
    		fres = f_printf(&fil2, "test, test,\n");
    		fres = f_printf(&fil2, "%4x, %4x,", sid, eid);
    		for(int i = 0; i < dat_len; i++){
    			fres = f_printf(&fil2, "%2x", data[i]);
    		}
    		f_printf(&fil2, ",\n");
    		buf_state = buf_get(&buf2, &cell);
    		}
    	}
    }
    else{
        while (1) {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            HAL_Delay(500);
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            HAL_Delay(500);
        }
    }
    f_close(&fil2);
    return 0;
}

int flush_buf(int can_ch_slct, FIL *fil){
	FRESULT fres;

	struct circularBuffer* buf_select;
    struct bufCell cell;
    int buf_state;

    uint16_t sid;
    uint16_t eid;
    uint8_t data[8];
    uint8_t dat_len;

    //select can buffer
    if(can_ch_slct == 1){
    	buf_select = &buf1;
    }
    else if(can_ch_slct == 2){
    	buf_select = &buf2;
    }
    else{
    	return 1;
    }

    //Flush buffer
    //Assumes file is open and points to end of the file
	buf_state = buf_get(buf_select, &cell);
	while(buf_state == 0){
		parse_packet(&cell, &sid, &eid, data, &dat_len);
		HAL_Delay(10);
		fres = f_printf(fil, "%16b, %16b,", sid, eid);
		for(int i = 0; i < dat_len; i++){
			fres = f_printf(fil, "%8b", data[i]);
		}
		f_printf(fil, ",\n");
		buf_state = buf_get(buf_select, &cell);
	}
	return fres;
}





int demount_storage(){
    FRESULT fres;
    HAL_Delay(100);
	fres = f_mount(NULL, "", 0);
    return fres;
}

// Generate some data and push it to buf1 and buf2. To be used for testing only.
void test_generate_data(){
    struct bufCell cell;
    uint8_t test_dat[8] = {2, 3, 5, 7, 11, 13, 17, 19};
    construct_packet(&cell, 69, 420, test_dat, 8);
    buf_put(&buf1, &cell);
    buf_put(&buf2, &cell);
}
