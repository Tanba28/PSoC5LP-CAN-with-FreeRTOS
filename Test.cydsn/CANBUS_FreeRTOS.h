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
#ifndef CANBUS_FREERTOS_H
#define CANBUS_FREERTOS_H
    
#include "project.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
    
#define CAN_RX_MAILBOX_SHIFT(i) 0x01 << (i) 

/* Public Function *************************************************/
void vCanBusStart();
void vCanBusStop();
void vCanBusEnable();
void vCanBusDisable();
void vCanBusTransmit(uint32_t id,uint8_t dataSize, uint8_t *data);
void vCanBusRead(uint8_t mailboxNum, uint8_t *data );

#endif
/* [] END OF FILE */
