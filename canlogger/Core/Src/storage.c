#include <stdint-gcc.h>
#include "storage.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


extern struct circularBuffer buf1;
extern struct circularBuffer buf2;

int init_storage(FATFS *FatFs){
	FRESULT fres;
	HAL_Delay(1000);
	fres = f_mount(FatFs, "", 1);
	if(fres != FR_OK){
		while(1){
		    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		        HAL_Delay(100);
		        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		        HAL_Delay(100);

		}
	}
	return fres;
}

//This bare bones opens two discrete files one for each CAN
//Then runs until the circularBuffers are empty
int flush_storage(FATFS *FatFs){
    FRESULT fres;
    FIL fil;
    struct bufCell cell;
    int buf_state;

    uint16_t sid;
    uint16_t eid;
    uint8_t data[8];
    uint8_t dat_len;
    uint8_t temp = 160;

    char buffer[250];
    UINT bytewrote;

    test_generate_data();

    //Clears buf1
    fres = f_open(&fil, "can1.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil, f_size(&fil));
    	if(fres != FR_OK){
    		f_close(&fil);
    		return fres;
    	}
    	/*else{
    	    BYTE readBuf[31];
    	    UINT bytesWrote;
    	    strncpy((char*)readBuf, "This is a test message\n", 30);
    	    HAL_Delay(1000);
    	    fres = f_write(&fil, readBuf, 30, &bytesWrote);
    	    for(int x = 1; x<1000; x++){
    	       	fres = f_write(&fil, readBuf, 30, &bytesWrote);
    	    }

    	}*/
    	else{
    	//Perform file write here.
    	//Finish by calling f_close on fil
    		buf_state = buf_get(&buf1, &cell);
    		if(buf_state == 1){
    		    while(1){
    			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    		        HAL_Delay(100);
    		        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    		        HAL_Delay(100);
    		    }
    		}
    		else{

    			parse_packet(&cell, &sid, &eid, data, &dat_len);
    			int len = sprintf(buffer, "%d, %d, %d, \n  ", sid, eid, temp);
    			fres = f_write(&fil, buffer, len, &bytewrote);
    		}
    	}
    }
    f_close(&fil);
    //Clears buf2
    fres = f_open(&fil, "can2.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil, f_size(&fil));
    	if(fres != FR_OK){
    		f_close(&fil);
    		return fres;
    	}
    	else{
    	    	    BYTE readBuf[31];
    	    	    UINT bytesWrote;
    	    	    strncpy((char*)readBuf, "This is a test message\n", 30);
    	    	    HAL_Delay(1000);
    	    	    fres = f_write(&fil, readBuf, 30, &bytesWrote);
    	    	    for(int x = 1; x<1000; x++){
    	    	       	fres = f_write(&fil, readBuf, 30, &bytesWrote);
    	    	    }

    		//Perform file write here
    		//Finish by calling f_close on fil
    	/*	buf_state = buf_get(&buf2, &cell);
    	    		if(buf_state == 1){
    	    		    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    	    		        HAL_Delay(100);
    	    		        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    	    		        HAL_Delay(100);
    	    		}
    	    		else{
    	    			parse_packet(&cell, &sid, &eid, data, &dat_len);
    	    			int len = sprintf(buffer, '%d,', sid);
    	    			fres = f_write(&fil, buffer, len, &byteswrote);
    	    		}
    	}*/}
    }
    f_close(&fil);
    return 0;
}

// Generate some data and push it to buf1 and buf2. To be used for testing only.
void test_generate_data(){
    struct bufCell cell;
    uint8_t test_dat[8] = {2, 3, 5, 7, 11, 13, 17, 19};
    construct_packet(&cell, 69, 420, test_dat, 8);
    buf_put(&buf1, &cell);
    buf_put(&buf2, &cell);
}
