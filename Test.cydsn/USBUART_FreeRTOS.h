/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef USBUART_FREERTOS_H
#define USBUART_FREERTOS_H
    
#include "project.h"
#include "stdio.h"
    
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Public Function *************************************************/
void vUSBUARTStart();/* USBUART Start Wrapper Prototype*/
void vUSBUARTPutString(const char string[],const uint8_t ucLen);
void vUSBUARTGetString(char * cString,uint8_t ucLen);

/* Static Function *************************************************/
static void vUSBUARTSetup();

/* Static Tx Prototype Function */
static void vUSBUARTPutChar(const char ch);
static void vUSBUARTTxTask();

/* StaticRx Prototype Function */

static void vUSBUARTGetChar(char *ch);
static void vUSBUARTRxTask();
    
/* FreeRTOS Hundle */
static QueueHandle_t xUartTxFifoQue;
static QueueHandle_t xUartRxFifoQue;

static SemaphoreHandle_t xUartTxBinarySemaphor;
static SemaphoreHandle_t xUartRxBinarySemaphor;

void vUSBUARTStart(){
    USBUART_Start(0, USBUART_DWR_VDDD_OPERATION);
    //500ms TimeOut
    for(uint8_t i=0;i<100;i++){
        if(0u != USBUART_bGetConfiguration()){
            USBUART_CDC_Init();
            break;
        }
        CyDelay(5);
    }
    vUSBUARTSetup();
    
    xTaskCreate(vUSBUARTTxTask,"UART_TX",100,NULL,3,NULL);
    xTaskCreate(vUSBUARTRxTask,"UART_RX",100,NULL,3,NULL);
}

/* USBUART Setup*/
static void vUSBUARTSetup(){
    xUartTxFifoQue = xQueueCreate(64,1);//USBUART tx buffer queue.
    xUartRxFifoQue = xQueueCreate(64,1);//USBUART rx buffer queue.
    xUartTxBinarySemaphor = xSemaphoreCreateBinary();//Binary semaphore for tx buffer prevent write conflict.
    xUartRxBinarySemaphor = xSemaphoreCreateBinary();//Binary semaphore for rx buffer prevent Read conflict.
    
    xSemaphoreGive(xUartTxBinarySemaphor);
    xSemaphoreGive(xUartRxBinarySemaphor);
}

/* Tx Function */
void vUSBUARTPutString(const char *cString, const uint8_t ucLen){    
    xSemaphoreTake(xUartTxBinarySemaphor,portMAX_DELAY);//Prevent queue write conflict.
    for(uint8_t ucCount=0; ucCount<ucLen; ucCount++){
        vUSBUARTPutChar(cString[ucCount]);
    }    
    xSemaphoreGive(xUartTxBinarySemaphor);
}

static void vUSBUARTPutChar(const char ch){
    xQueueSendToBack(xUartTxFifoQue,&ch,portMAX_DELAY);
}

static void vUSBUARTTxTask(){
    char cUartTxBuffer[64];//One packet tx buffer
    uint8_t ucUartTxCounter = 0;
    uint8_t ucUartTxQueCounter = 0;
    
    TickType_t xTick = xTaskGetTickCount();
    for(;;){
        ucUartTxQueCounter = uxQueueMessagesWaiting(xUartTxFifoQue);
        for(ucUartTxCounter=0;ucUartTxCounter<ucUartTxQueCounter;ucUartTxCounter++){
            xQueueReceive(xUartTxFifoQue,&cUartTxBuffer[ucUartTxCounter],0);
        }        
        
        if(ucUartTxQueCounter != 0){
            if(USBUART_CDCIsReady()){
                USBUART_PutData((uint8_t*)cUartTxBuffer,ucUartTxQueCounter);
                ucUartTxCounter = 0;
            }           
        }
        vTaskDelayUntil(&xTick,5);
    }
}

/* Rx Function */
void vUSBUARTGetString(char * cString,uint8_t ucLen){
    xSemaphoreTake(xUartRxBinarySemaphor,portMAX_DELAY);//Prevent queue read conflict.
    for(uint8_t ucCount=0; ucCount<ucLen; ucCount++){
        vUSBUARTGetChar(&cString[ucCount]);
    }  
    xSemaphoreGive(xUartRxBinarySemaphor);
}
static void vUSBUARTGetChar(char *ch){
    xQueueReceive(xUartRxFifoQue,ch,portMAX_DELAY);
}

static void vUSBUARTRxTask(){
    char cUartRxBuffer[64];//One packet rx buffer
    uint8_t ucUartRxCounter = 0;
    uint8_t ucUartRxQueCounter = 0;
    
    TickType_t xTick = xTaskGetTickCount();
        
    for(;;){
        ucUartRxQueCounter = uxQueueMessagesWaiting(xUartRxFifoQue);
        if(ucUartRxQueCounter < 64 && ucUartRxCounter > 0){            
            xQueueSendToBack(xUartRxFifoQue,&cUartRxBuffer[ucUartRxCounter-1],portMAX_DELAY);
            ucUartRxCounter--;
        }       
        
        if(ucUartRxCounter < 64){         
            if (USBUART_DataIsReady()) {
                cUartRxBuffer[ucUartRxCounter] = USBUART_GetChar();
                ucUartRxCounter++;
            }
        }
        vTaskDelayUntil(&xTick,5);
    }
}

#endif
/* [] END OF FILE */
