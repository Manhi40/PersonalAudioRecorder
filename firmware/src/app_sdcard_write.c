/* 
 * File:   app_sdcard_write.c
 * Author: Eryk Zagorski
 *
 * Created on November 23, 2018, 9:24 PM
 */

#include "app_sdcard_write.h"


APP_SDCARD_WRITE_DATA appSDcardWriteData;

void APP_SDCARD_WRITE_Initialize(void){
    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_MOUNT;
    appSDcardWriteData.currentFilePosition = 0;
}

static bool APP_SDCARD_WIRTE_Read_SDCard(
    const DRV_HANDLE handle,
    uint8_t* const pBuffer,
    const uint16_t requestedBytes,
    uint16_t*const pNumBytesRead
)
{
    
}



void APP_SDCARD_WRITE_Tasks(void){
    switch(appSDcardWriteData.state){
        case APP_SDCARD_WRITE_STATE_CARD_MOUNT:
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive", FAT, 0, NULL) != 0){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_MOUNT;
            }
            else{
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET;
            }
            break;
        case APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET:
            if(SYS_FS_CurrentDriveSet("/mnt/myDrive") == SYS_FS_RES_FAILURE){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
            }
            else{
                appSDcardWriteData.fileHandle = SYS_FS_FileOpen("test.txt", (SYS_FS_FILE_OPEN_READ));
                if(appSDcardWriteData.fileHandle == SYS_FS_HANDLE_INVALID){
                    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
                }
                else{
                    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_READ_FILE_SIZE;
                }
            }
            break;
        case APP_SDCARD_WRITE_STATE_READ_FILE_SIZE:
            appSDcardWriteData.fileSize = SYS_FS_FileSize(appSDcardWriteData.fileHandle);
            if(appSDcardWriteData.fileSize == -1){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
            }
            else{
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_READ;
            }
            break;
        case APP_SDCARD_WRITE_STATE_CARD_READ:
        {
            uint16_t nBytesRead = 0;
            uint16_t nBytesParsed = 0;
            uint8_t data[5];
            if(appSDcardWriteData.currentFilePosition < appSDcardWriteData.fileSize){
                //READ DATA BOYYYSSS
                SYS_FS_FileRead(appSDcardWriteData.fileHandle,&data,5);
                
            }
            SYS_FS_FileWrite(appSDcardWriteData.fileHandle,"WE HAVE WRITING?",16);
        }
        default:
        {}
        break;
    }
}