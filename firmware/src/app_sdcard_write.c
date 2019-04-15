/* 
 * File:   app_sdcard_write.c
 * Author: Eryk Zagorski
 *
 * Created on November 23, 2018, 9:24 PM
 */

#include "app_sdcard_write.h"

extern APP_DATA appData;
APP_SDCARD_WRITE_DATA appSDcardWriteData;

void APP_SDCARD_WRITE_Initialize(void){
    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_MOUNT;
    appSDcardWriteData.currentFilePosition = 0;
    appSDcardWriteData.dataParser.nElements = 0;
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB7 = 1;
    CNPUBbits.CNPUB7 = 1;
    strcpy(appSDcardWriteData.currentFileName, "file0.wav");
    appSDcardWriteData.fileCount = 0;
    //strcpy(appSDcardWriteData.dataParser.buffer, "this is a test");
}

static bool APP_SDCARD_WRITE_Write_SDCard(
    const DRV_HANDLE handle,
    uint8_t* const pBuffer,
    const uint16_t bytesToWrite,
    uint16_t*const pNumBytesWrote
)
{
    uint32_t nBytesWrote = 0;
    bool isSuccess = true;
    nBytesWrote = SYS_FS_FileWrite(handle, (void *)pBuffer, bytesToWrite);
    if((int32_t)nBytesWrote == -1){
        if(SYS_FS_FileEOF(handle)){
            isSuccess = false;
        }
    }
    *pNumBytesWrote = (uint16_t)nBytesWrote;
    return isSuccess;
}



void APP_SDCARD_WRITE_Tasks(void){
    switch(appSDcardWriteData.state){
        case APP_SDCARD_WRITE_STATE_CARD_MOUNT:
            
            //waits for SDcard to be mounted
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive", FAT, 0, NULL) != 0){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_MOUNT;
            }
            else{
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET;
            }
            break;
        case APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET:
            
            //checks if SDcard is properly mounted
            if(SYS_FS_CurrentDriveSet("/mnt/myDrive") == SYS_FS_RES_FAILURE){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
            }
            //if properly mounted, open "test.txt"
            else{
                appSDcardWriteData.fileHandle = SYS_FS_FileOpen(appSDcardWriteData.currentFileName, (SYS_FS_FILE_OPEN_WRITE));
                if(appSDcardWriteData.fileHandle == SYS_FS_HANDLE_INVALID){
                    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
                }
                
                // if all is good, move onto reading the file size
                else{
                    if(appSDcardWriteData.headerWrite){
                    appSDcardWriteData.state = APP_SDCARD_WRITE_HEADER;
                    appSDcardWriteData.headerWrite = 0;
                }
                else{
                    appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_WRITE;
                    }
                }
            }
            break;
        case APP_SDCARD_WRITE_STATE_READ_FILE_SIZE:
            //read the file size
            appSDcardWriteData.fileSize = SYS_FS_FileSize(appSDcardWriteData.fileHandle);
            //if unsuccessful, move to error state
            if(appSDcardWriteData.fileSize == -1){
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_ERROR;
            }
            //on success, move onto writing to the card
            else{
                appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_WRITE;
            }
            break;
         
        case APP_SDCARD_WRITE_HEADER:
        {
            uint16_t nBytesWrote = 0;
            SYS_FS_FileSeek(appSDcardWriteData.fileHandle, appSDcardWriteData.currentFilePosition, SYS_FS_SEEK_SET);
            if(APP_SDCARD_WRITE_Write_SDCard(
                    appSDcardWriteData.fileHandle, 
                    appData.pheader,
                    1024,
                    &nBytesWrote)){
                appSDcardWriteData.currentFilePosition += nBytesWrote;
            }
            appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_WRITE;
            
        }
        break;
        
        
        case APP_SDCARD_WRITE_STATE_CARD_WRITE:
        {
            uint16_t nBytesWrote = 0;
            uint16_t nBytesToWrite = 0;

            //if statement checks if we still have data to write
            if(sizeof(*appData.sdBuffer) > 0){
                
                nBytesToWrite = bufferSize*2;

                
                //writes data and checks if successful, if not, data writing is done

                SYS_FS_FileSeek(appSDcardWriteData.fileHandle, appSDcardWriteData.currentFilePosition, SYS_FS_SEEK_SET);
                if(APP_SDCARD_WRITE_Write_SDCard(
                        appSDcardWriteData.fileHandle,
                        appData.sdBuffer,
                        nBytesToWrite, &nBytesWrote)){
                    appSDcardWriteData.currentFilePosition += nBytesWrote;
                    
                }
                //appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_WRITE;
                if(!PORTBbits.RB7){
                    LATBbits.LATB0 = 1;
                    LATBbits.LATB1 = 0;
                }
                SYS_FS_FileSync(appSDcardWriteData.fileHandle);
                if(!PORTBbits.RB7){
                    LATBbits.LATB0 = 0;
                    LATBbits.LATB1 = 1;
                }
                else{
                    LATBbits.LATB0 = 0;
                    LATBbits.LATB1 = 0;
                }
                //appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET;
                if(appSDcardWriteData.currentFilePosition > FILESIZE)
                    appSDcardWriteData.state = APP_SDCARD_WRITE_INC_FILENAME;
                appData.state = APP_STATE_ADC_WAIT;
            }
        }
            break;
            
        case APP_SDCARD_WRITE_INC_FILENAME:
        {
            char tempNum[20];
            appSDcardWriteData.fileCount++;
            itoa(appSDcardWriteData.fileCount,tempNum,10);
            strcpy(appSDcardWriteData.currentFileName, "file");
            strcat(appSDcardWriteData.currentFileName, tempNum);
            strcat(appSDcardWriteData.currentFileName, ".wav");
            SYS_FS_FileClose(appSDcardWriteData.fileHandle);
            appData.state = APP_STATE_CONSTRUCT_WAV_HEADER;
            appSDcardWriteData.state = APP_SDCARD_WRITE_STATE_CARD_CURRENT_DRIVE_SET;
            
        }
        break;
        default:
        {}
        break;
    }
}
