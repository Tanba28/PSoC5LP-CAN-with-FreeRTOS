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

static QueueHandle_t xCanTxFifoQue;
static QueueHandle_t xCanRxFifoQue[16];

static SemaphoreHandle_t xCanTxBinarySemaphor;
static SemaphoreHandle_t xCanRxBinarySemaphor;

static void vCanBusSetup();
static void vCanBusRxTask();
static void vCanBusTxTask();

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

CY_ISR(ISR_CAN){
    BaseType_t xHigherPriorityTaskWoken;
    
    xHigherPriorityTaskWoken = pdFALSE;
    
    CAN_INT_SR_REG.byte[1u] = CAN_RX_MESSAGE_MASK;
    
    xSemaphoreGiveFromISR(xCanRxBinarySemaphor, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void vCanBusSetup(){
    //割り込みの割り当て
    //CyIntSetVector(CAN_ISR_NUMBER, ISR_CAN);
    
    //Txバッファ用のキュー初期化
    xCanTxFifoQue = xQueueCreate(4,sizeof(CAN_TX_MSG));
    
    //Rxバッファ用のキュー初期化。メールボックスに1つにつきキュー1つ
    for(uint8_t i=0;i<CAN_NUMBER_OF_RX_MAILBOXES;i++){
        xCanRxFifoQue[i] = xQueueCreate(4,8);
    }  
    
    xCanTxBinarySemaphor = xSemaphoreCreateBinary();//Txの書き込み干渉防止用
    xCanRxBinarySemaphor = xSemaphoreCreateBinary();//Rxの割り込みトリガ用
    
    xSemaphoreGive(xCanTxBinarySemaphor);
    //xSemaphoreGive(xCanRxBinarySemaphor);
    
    xTaskCreate(vCanBusRxTask,"CAN_RX",200,NULL,3,NULL);
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

static void vCanBusTxTask();



void vCanBusRead(uint8_t mailboxNum, uint8_t *data){
    xQueueReceive(xCanRxFifoQue[mailboxNum],data,portMAX_DELAY);
    
}



/* [] END OF FILE */
