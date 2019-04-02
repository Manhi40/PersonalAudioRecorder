/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END



#include "app.h"
#include "app_sdcard_write.h"
#include "wav_format_container.h"
#include "encoder.h"



#define NUM_PACKETS_TO_ONE_PAGE  10
#define AUDIO_ENCODE_SAMPLE_RATE 16000



typedef struct {
    uint8_t *buffer;
    uint32_t len;
} BUFFER;




APP_DATA appData;

const HAR_ENCODER *runtimeEncoderInst;

static uint32_t _audio_frame_count = 0;
static StreamInfo si;

static uint32_t encoded_data_size;

extern APP_SDCARD_WRITE_DATA appSDcardWriteData;




/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void ){
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    appData.dmaBuffer = &appData.pingBuf;
    appData.sdBuffer = &appData.pongBuf;
    
    encoded_data_size =0;

    appData.channelHandle = SYS_DMA_ChannelAllocate(DMA_CHANNEL_0);
    SYS_DMA_ChannelSetup(appData.channelHandle, SYS_DMA_CHANNEL_OP_MODE_AUTO , DMA_TRIGGER_ADC_1);
    SYS_DMA_ChannelTransferAdd(appData.channelHandle, (const void*)&ADC1BUF0, 2, appData.dmaBuffer,bufferSize*2,2);
    
    PLIB_DMA_ChannelXINTSourceFlagClear(DMA_ID_0, DMA_CHANNEL_0, DMA_INT_DESTINATION_DONE);
    PLIB_DMA_ChannelXINTSourceEnable(DMA_ID_0, DMA_CHANNEL_0, DMA_INT_DESTINATION_DONE);
    SYS_DMA_ChannelEnable(appData.channelHandle);
        
    DRV_ADC_Open();
    DRV_ADC_Start();
    
    APP_SDCARD_WRITE_Initialize();
    
    //DRV_ADC_Initialize();
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    uint32_t size;
    uint32_t outsize = 0;
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            DRV_ADC_Open();
            
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
                appData.state = APP_STATE_INIT_ENCODER;
            }
            break;
        }
        
        case APP_STATE_IDLE:
        {
            break;
        }
        
        case APP_STATE_ADC_WAIT:
        {
            asm("nop");
            break;
        }
        
        case APP_STATE_INIT_ENCODER:
        {
                    runtimeEncoderInst = &pcmEncoderInst;
                    si.sample_rate = AUDIO_ENCODE_SAMPLE_RATE;
                    si.channel = 1; // mono
                    si.bit_depth = 16;// I don't know if 10 bit is supported, check later
                    si.bps = si.sample_rate * si.channel * si.bit_depth;
                    if (runtimeEncoderInst->enc_init(si.channel, si.sample_rate)) {
                        appData.state = APP_STATE_CONSTRUCT_WAV_HEADER;
                    }
                    break;
        }
        
        case APP_STATE_PROCESS_DATA:
        {
            outsize = 0;
            runtimeEncoderInst->enc_one_frame(appData.sdBuffer,bufferSize,&appData.writeBuf[0],&outsize);
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_CONSTRUCT_WAV_HEADER:    //might be able to reuse header, try this later
        {
            size = wav_riff_fill_header(appData.pheader, PCM, &si, 33554432);
            appSDcardWriteData.headerWrite = 1;
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            APP_SDCARD_WRITE_Tasks();
            
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}



/*******************************************************************************
 End of File
 */
