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

void vEchoBackTask();
volatile uint32_t temp;


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    vFreeRTOSSetup();
    vUSBUARTStart();
    vCanBusStart();
    vCanBusEnable();
    xTaskCreate(vEchoBackTask,"test4",1000,NULL,3,NULL);//Echo back test task

    //CyIntSetVector(CAN_ISR_NUMBER, ISR_CAN);    
    
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

void vEchoBackTask(){
    uint8_t buf[8];    
    char debug[128];
    for(;;){      
        LED_0_Write(~LED_0_Read());
        vCanBusRead(0,buf);
        sprintf(debug,"%x %x %x\r\n",buf[0],buf[1],buf[2]);
        vUSBUARTPutString(debug,strlen(debug));
        
    }    
}
/* [] END OF FILE */
