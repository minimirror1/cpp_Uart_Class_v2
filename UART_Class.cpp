#include "UART_Class.h"

/* LED 관련 함수 구현 */
void Serial::rxLed_on() {
    if(rxLed.init != true) return;
    HAL_GPIO_WritePin(rxLed.Port_, rxLed.Pin_, rxLed.OnState_);
}

void Serial::rxLed_off() {
    if(rxLed.init != true) return;
    HAL_GPIO_WritePin(rxLed.Port_, rxLed.Pin_, rxLed.OffState_);
}

void Serial::txLed_on() {
    if(txLed.init != true) return;
    HAL_GPIO_WritePin(txLed.Port_, txLed.Pin_, txLed.OnState_);
    rxLed_off_Tick.tickUpdate();
}

void Serial::txLed_off() {
    if(txLed.init != true) return;
    HAL_GPIO_WritePin(txLed.Port_, txLed.Pin_, txLed.OffState_);
}

/* RS485 관련 함수 구현 */
void Serial::rs485_txMode() {
    if(rs485.init != true) return;
    HAL_GPIO_WritePin(rs485.Port_, rs485.Pin_, GPIO_PIN_SET);
}

void Serial::rs485_rxMode() {
    if(rs485.init != true) return;
    HAL_GPIO_WritePin(rs485.Port_, rs485.Pin_, GPIO_PIN_RESET);
}

/* 생성자 및 소멸자 */
Serial::Serial() {
    txLed.init = false;
    rxLed.init = false;
    rs485.init = false;
}

Serial::~Serial() {}

/* 초기화 함수들 */
void Serial::init(UART_HandleTypeDef *huart, IRQn_Type UART_IRQn) {
    huart_ = huart;
    UART_IRQn_ = UART_IRQn;
    rxBuff.front = 0;
    rxBuff.rear = 0;
    txBuff.front = 0;
    txBuff.rear = 0;
    rxAppointment();
}

void Serial::init_txLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState) {
    txLed.init = true;
    txLed.Port_ = Port;
    txLed.Pin_ = Pin;
    txLed.OnState_ = OnState;
    txLed.OffState_ = (OnState == GPIO_PIN_SET)? GPIO_PIN_RESET : GPIO_PIN_SET;
    txLed_off();
}

void Serial::init_rxLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState) {
    rxLed.init = true;
    rxLed.Port_ = Port;
    rxLed.Pin_ = Pin;
    rxLed.OnState_ = OnState;
    rxLed.OffState_ = (OnState == GPIO_PIN_SET)? GPIO_PIN_RESET : GPIO_PIN_SET;
    rxLed_off();
}

void Serial::init_rs485(GPIO_TypeDef *Port, uint16_t Pin) {
    rs485.init = true;
    rs485.Port_ = Port;
    rs485.Pin_ = Pin;
    rs485_rxMode();
}

void Serial::rxLed_Check() {
    if(rxLed_off_Tick.delay(5))
        rxLed_off();
}

void Serial::rxAppointment() {
    HAL_UART_Receive_IT(huart_, &rxData_, 1);
}

void Serial::rxAppointCheck() {
    if(huart_->RxState == HAL_UART_STATE_READY) {
        HAL_UART_Receive_IT(huart_, &rxData_, 1);
    }
}

void Serial::RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart != huart_)
        return;
    rxBuff.buff[rxBuff.front] = rxData_;
    if(++rxBuff.front >= UART_BUFF_CNT)
        rxBuff.front = 0;
    rxAppointment();
    rxLed_on();
}

BuffStatus_TypeDef Serial::popRxBuff(uint8_t *pData) {
    if(rxBuff.front == rxBuff.rear)
        return BUFF_EMPTY;
    *pData = rxBuff.buff[rxBuff.rear];
    if(++rxBuff.rear >= UART_BUFF_CNT)
        rxBuff.rear = 0;
    return BUFF_CONTAIN;
}

void Serial::pushTxBuff(uint8_t data) {
    txBuff.buff[txBuff.front] = data;
    if(++txBuff.front >= UART_BUFF_CNT)
        txBuff.front = 0;
}

void Serial::txByte(uint8_t data) {
    pushTxBuff(data);
    txBuffCheck_Transmit();
}

void Serial::txStream(uint8_t *pData, uint32_t size) {
    uint32_t ptr = 0;
    while(size--)
        pushTxBuff(pData[ptr++]);
    txBuffCheck_Transmit();
}

void Serial::txBuffCheck_Transmit() {
    uint32_t size = 0;

    if (txBuff.front == txBuff.rear) {
        rs485_rxMode();
        txLed_off();
        return;
    }

    if (txBuff.front > txBuff.rear) {
        size = txBuff.front - txBuff.rear;
    } else {
        size = UART_BUFF_CNT - txBuff.rear;
    }
    rs485_txMode();
    txLed_on();

    HAL_UART_Transmit_IT(huart_, &txBuff.buff[txBuff.rear], size);
}

void Serial::TxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart != huart_)
        return;
    txBuff.rear += huart_->TxXferSize;
    txBuff.rear %= UART_BUFF_CNT;
    txBuffCheck_Transmit();
}

int Serial::ring_buffer_usage(SerialByteBuff* rb) {
    if(rb->rear <= rb->front) {
        return rb->front - rb->rear;
    } else {
        return (UART_BUFF_CNT - rb->rear) + rb->front + 1;
    }
}

int Serial::available() {
    return ring_buffer_usage(&rxBuff);
}

void Serial::write(char c) {
    txByte(c);
}

void Serial::write(const char *str) {
    int ptr = 0;
    while (str[ptr] != 0)
        pushTxBuff(str[ptr++]);
    txBuffCheck_Transmit();
}

int Serial::write(uint8_t *packet, int length) {
    int ptr = 0;
    while (ptr != length)
        pushTxBuff(packet[ptr++]);
    txBuffCheck_Transmit();
    return ptr;
}

uint8_t Serial::read() {
    uint8_t ret = 0;
    popRxBuff(&ret);
    return ret;
}

