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

void vFreeRTOSSetup();

void vUartEchoBackTask();

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    vFreeRTOSSetup();
    vUSBUARTStart();
    
    xTaskCreate(vUartEchoBackTask,"test4",1000,NULL,3,NULL);//Echo back test task
    
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

void vUartEchoBackTask(){
    char buf;    
    
    for(;;){      
        LED_0_Write(~LED_0_Read());
        vUSBUARTGetString(&buf,1);
        vUSBUARTPutString(&buf,1);
        //vTaskDelay(1000);
    }    
}
/* [] END OF FILE */
