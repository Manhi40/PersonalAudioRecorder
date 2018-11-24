/* 
 * File:   app_sdcard_write.h
 * Author: Eryk Zagorski
 *
 * Created on November 23, 2018, 9:26 PM
 */

#ifndef APP_SDCARD_WRITE_H
#define	APP_SDCARD_WRITE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum{
 
APP_SDCARD_WRITE_STATE_CARD_MOUNT,
APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET,
APP_SDCARD_WRITE_STATE_READ_FILE_SIZE,
APP_SDCARD_WRITE_STATE_CARD_READ,            
APP_SDCARD_WRITE_STATE_ERROR,
APP_SDCARD_WRITE_STATE_MKDIR

} APP_SDCARD_WRITE_STATES;

typedef struct{
    APP_SDCARD_WRITE_STATES state;
    SYS_FS_HANDLE fileHandle;
    int32_t fileSize;
    int32_t currentFilePosition;
}APP_SDCARD_WRITE_DATA;


void APP_SDCARD_WRITE_Tasks(void);

void APP_SDCARD_WRITE_Initialize(void);





#ifdef	__cplusplus
}
#endif

#endif	/* APP_SDCARD_WRITE_H */

