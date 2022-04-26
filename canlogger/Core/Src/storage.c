#include <stdint-gcc.h>
#include "storage.h"
#include "globals.h"


extern struct circularBuffer buf1;
extern struct circularBuffer buf2;

int init_storage(FATFS *FatFs){
	FRESULT fres;
	HAL_Delay(1000);
	fres = f_mount(FatFs, "", 1);
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

    char buffer[250];
    uint8_t byteswrote;


    //Clears buf1
    fres = f_open(&fil, "can1.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil, f_size(&fil));
    	if(fres != FR_OK){
    		f_close(&fil);
    		return fres;
    	}
    	else{
    	//Perform file write here.
    	//Finish by calling f_close on fil
    		buf_state = buf_get(&buf1, &cell);
    		if(buf_state == 1){
    			//break;
    		}
    		else{
    			parse_packet(&cell, &sid, &eid, data, &dat_len);
    			int len = sprintf(buffer, '%d,', sid);
    			fres = f_write(&fil, buffer, len, &byteswrote);
    		}
    	}
    }
    //Clears buf2
    fres = f_open(&fil, "can2.csv", FA_WRITE | FA_OPEN_ALWAYS);
    if(fres == FR_OK){
    	fres = f_lseek(&fil, f_size(&fil));
    	if(fres != FR_OK){
    		f_close(&fil);
    		return fres;
    	}
    	else{
    		//Perform file write here
    		//Finish by calling f_close on fil
    	}
    }
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
