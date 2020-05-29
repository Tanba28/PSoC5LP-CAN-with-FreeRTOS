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
#include "CANBUS_FreeRTOS.h"

CY_ISR_PROTO(ISR_CAN);

static QueueHandle_t xCanRxFifoQue[16];

static SemaphoreHandle_t xCanRxBinarySemaphor;

static void vCanBusSetup();
static void vCanBusRxTask();

void vCanBusStart(){
    CAN_Start();
    vCanBusSetup();
}

void vCanBusStop(){
    CAN_Stop();
}

void vCanBusEnable(){
    CAN_EN_Write(0);
}

void vCanBusDisable(){
    CAN_EN_Write(1);
}

void vCanBusRead(uint8_t mailboxNum, uint8_t *data){
    xQueueReceive(xCanRxFifoQue[mailboxNum],data,portMAX_DELAY);
    
}

void vCanBusTransmit(uint32_t id,uint8_t dataSize, uint8_t *data){
    CAN_TX_MSG message;
    CAN_DATA_BYTES_MSG messageData;
    
    message.id = id;
    message.rtr = 0;
    message.ide = 0;
    message.dlc = dataSize;
    message.irq = 1;
    message.msg = &messageData;
    
    for(int i=0;i<dataSize;i++){
        messageData.byte[i] = data[i];
    }
    CAN_SendMsg(&message);
}

CY_ISR(ISR_CAN){
    BaseType_t xHigherPriorityTaskWoken;
    
    xHigherPriorityTaskWoken = pdFALSE;
    
    CAN_INT_SR_REG.byte[1u] = CAN_RX_MESSAGE_MASK;
    
    xSemaphoreGiveFromISR(xCanRxBinarySemaphor, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void vCanBusSetup(){  
    //Rxバッファ用のキュー初期化。メールボックスに1つにつきキュー1つ
    for(uint8_t i=0;i<CAN_NUMBER_OF_RX_MAILBOXES;i++){
        xCanRxFifoQue[i] = xQueueCreate(4,8);
    }  
    
    xCanRxBinarySemaphor = xSemaphoreCreateBinary();//Rxの割り込みトリガ用
    
    xTaskCreate(vCanBusRxTask,"CAN_RX",100,NULL,3,NULL);
    
    //割り込みの割り当て
    CyIntSetVector(CAN_ISR_NUMBER, ISR_CAN);
}

static void vCanBusRxTask(){
    uint16_t canRxBufReg;
    uint8_t canDataBuf[8];
    for(;;){
        //割り込みが入るまでブロック
        xSemaphoreTake(xCanRxBinarySemaphor,portMAX_DELAY);
        
        //受信したメールボックスを調べる
        canRxBufReg = CY_GET_REG16((reg16*)&CAN_BUF_SR_REG);             
        for(uint8_t mailboxNum=0;mailboxNum<CAN_NUMBER_OF_RX_MAILBOXES;mailboxNum++){
            if((canRxBufReg & CAN_RX_MAILBOX_SHIFT(mailboxNum)) != 0){
                for(uint8_t byteNum=0;byteNum<8;byteNum++){
                    canDataBuf[byteNum] = CAN_RX_DATA_BYTE(mailboxNum,byteNum);
                }
                xQueueSendToBack(xCanRxFifoQue[mailboxNum],canDataBuf,portMAX_DELAY);
                    
                //ACK送信
                CAN_RX_ACK_MESSAGE(mailboxNum);
            }
        }
    }
}

/* [] END OF FILE */
