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
#include "project.h"

#include "stdio.h"
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"

#include "USBUART_FreeRTOS.h"
#include "CANBUS_FreeRTOS.h"

void vFreeRTOSSetup();

void vCanIsrReadTask();
void vCanPerodicReadTask();
void vCanIsrTransmitTask();
void vCanPeriodicTransmitTask();
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    vFreeRTOSSetup();
    vUSBUARTStart();
    vCanBusStart();
    vCanBusEnable();
    xTaskCreate(vCanIsrReadTask,"test4",1000,NULL,3,NULL);
    xTaskCreate(vCanPerodicReadTask,"test4",1000,NULL,3,NULL);
    xTaskCreate(vCanIsrTransmitTask,"test4",1000,NULL,3,NULL);
    xTaskCreate(vCanPeriodicTransmitTask,"test4",1000,NULL,3,NULL);
    
    vTaskStartScheduler();

    for(;;)
    {
        /* Place your application code here. */
        
    }
}

void vFreeRTOSSetup(){
    extern void xPortPendSVHandler( void );
    extern void xPortSysTickHandler( void );
    extern void vPortSVCHandler( void );
    extern cyisraddress CyRamVectors[];

    CyRamVectors[ 11 ] = ( cyisraddress ) vPortSVCHandler;
    CyRamVectors[ 14 ] = ( cyisraddress ) xPortPendSVHandler;
    CyRamVectors[ 15 ] = ( cyisraddress ) xPortSysTickHandler;
}

void vCanIsrReadTask(){
    uint8_t buf[8];    
    char debug[128];
    
    for(;;){
        LED_1_Write(~LED_1_Read());
        vCanBusRead(0,buf);
        sprintf(debug,"RX-ISR      MAILBOX:0 %c\r\n",buf[0]);
        vUSBUARTPutString(debug,strlen(debug));
    }
}

void vCanPerodicReadTask(){
    uint8_t buf[8];    
    char debug[128];
    
    for(;;){
        LED_1_Write(~LED_1_Read());
        vCanBusRead(1,buf);
        sprintf(debug,"RX-Periodic MAILBOX:1 %c\r\n",buf[0]);
        vUSBUARTPutString(debug,strlen(debug));
    }
}

void vCanIsrTransmitTask(){
    uint8_t buf[8];    
    char debug[128];
    for(;;){      
        LED_0_Write(~LED_0_Read());
        vUSBUARTGetString((char*)buf,1);
        vCanBusTransmit(0x001,8,buf);
        sprintf(debug,"TX-ISR      MAILBOX:0 %c\r\n",buf[0]);
        vUSBUARTPutString(debug,strlen(debug));        
    }    
}

void vCanPeriodicTransmitTask(){
    uint8_t buf[8];    
    char debug[128];
    
    buf[0] = 'N';
    TickType_t xTick = xTaskGetTickCount();
    for(;;){     
        vCanBusTransmit(0x002,1,buf);
        sprintf(debug,"TX-Periodic MAILBOX:1 %c\r\n",buf[0]);
        vUSBUARTPutString(debug,strlen(debug));    
        
        vTaskDelayUntil(&xTick,10000);
    }    
}
/* [] END OF FILE */
