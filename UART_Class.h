/*
 * UART_Class.h
 *
 *  Created on: Dec 13, 2023
 *      Author: minim
 */

#ifndef INC_UART_Class_H_
#define INC_UART_Class_H_

#include "main.h"
#include "../cpp_tick/cpp_tick.h"	//외부 의존성

#define UART_BUFF_CNT 4096

typedef enum _BuffStatus_TypeDef{
	BUFF_EMPTY = 0,
	BUFF_CONTAIN
}BuffStatus_TypeDef;


#pragma pack(1)
struct SerialByteBuff{
	uint8_t buff[UART_BUFF_CNT];
	uint32_t front;
	uint32_t rear;
};
#pragma pack()


/*
porting 1. init 함수에 CUBE_MX초기화가 끝난 huart와 _IRQn 을 전달한다.
porting 2. while loop 에 Parsing_Process 를 추가한다.
Serial example_1(1);
int cpp_main()
{
	example_1.init(&huart1, USART1_IRQn);
	while (1){
		example_1.Parsing_Process(); //<<--
	}
}

porting 3. HAL_UART_  CpltCallback  안에 TxCpltCallback, RxCpltCallback 를 추가한다.
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	example_1.TxCpltCallback(huart);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	example_1.RxCpltCallback(huart);
}


 */

class Serial{
private :

	struct GPIO_Ctr{
		GPIO_TypeDef *Port_;
		uint16_t Pin_;
		GPIO_PinState OnState_;
		GPIO_PinState OffState_;
		uint32_t t_Off;
		bool init;		//false : 미사용, true : 사용
	};

	/* property */
	//driver
	UART_HandleTypeDef *huart_;
	IRQn_Type UART_IRQn_;

	//tx rx
	SerialByteBuff txBuff;
	SerialByteBuff rxBuff;
	uint8_t rxData_;

	//gpio
	GPIO_Ctr txLed;
	GPIO_Ctr rxLed;
	Tick rxLed_off_Tick;
	GPIO_Ctr rs485;

	/* LED */
	void rxLed_on(){
		if(rxLed.init != true) return;
		HAL_GPIO_WritePin(rxLed.Port_, rxLed.Pin_, rxLed.OnState_);
	}
	void rxLed_off(){
		if(rxLed.init != true) return;
		HAL_GPIO_WritePin(rxLed.Port_, rxLed.Pin_, rxLed.OffState_);
	}
	void txLed_on(){
		if(txLed.init != true) return;
		HAL_GPIO_WritePin(txLed.Port_, txLed.Pin_, txLed.OnState_);
		rxLed_off_Tick.tickUpdate();
	}
	void txLed_off(){
		if(txLed.init != true) return;
		HAL_GPIO_WritePin(txLed.Port_, txLed.Pin_, txLed.OffState_);
	}

	/* RS485 */
	void rs485_txMode(){
		if(rs485.init != true) return;
		HAL_GPIO_WritePin(rs485.Port_, rs485.Pin_, GPIO_PIN_SET);
	}
	void rs485_rxMode(){
		if(rs485.init != true) return;
		HAL_GPIO_WritePin(rs485.Port_, rs485.Pin_, GPIO_PIN_RESET);
	}

public :
	Serial(){
		txLed.init = false;
		rxLed.init = false;
		rs485.init = false;
	}//전역으로 설정하고 HAL init 후 Serial init 호출하여야 한다.
	~Serial(){}

	/* init */
	void init(UART_HandleTypeDef *huart, IRQn_Type UART_IRQn){
		huart_ = huart;
		UART_IRQn_ = UART_IRQn;
		rxBuff.front = 0;
		rxBuff.rear = 0;
		txBuff.front = 0;
		txBuff.rear = 0;
		rxAppointment();
	}
	void init_txLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState){
		txLed.init = true;
		txLed.Port_ = Port;
		txLed.Pin_ = Pin;
		txLed.OnState_ = OnState;
		txLed.OffState_ = (OnState == GPIO_PIN_SET)? GPIO_PIN_RESET : GPIO_PIN_SET;
		txLed_off();
	}
	void init_rxLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState){
		rxLed.init = true;
		rxLed.Port_ = Port;
		rxLed.Pin_ = Pin;
		rxLed.OnState_ = OnState;
		rxLed.OffState_ = (OnState == GPIO_PIN_SET)? GPIO_PIN_RESET : GPIO_PIN_SET;
		rxLed_off();
	}
	void init_rs485(GPIO_TypeDef *Port, uint16_t Pin){
		rs485.init = true;
		rs485.Port_ = Port;
		rs485.Pin_ = Pin;
		rs485_rxMode();
	}

	void rxLed_Check(){
		if(rxLed_off_Tick.delay(5))
			rxLed_off();
	}

	/* uart 수신 대기 */
	void rxAppointment(){
		//HAL_UART_Receive_DMA(huart_, &rxData_, 1);
		HAL_UART_Receive_IT(huart_, &rxData_, 1);
	}

	void rxAppointCheck(){
		if(huart_->RxState ==HAL_UART_STATE_READY){
			HAL_UART_Receive_IT(huart_, &rxData_, 1);
		}
	}

	/* rx buffer */
	void RxCpltCallback(UART_HandleTypeDef *huart){
		if(huart != huart_)
			return;
		rxBuff.buff[rxBuff.front] = rxData_;
		if(++rxBuff.front >= UART_BUFF_CNT)
			rxBuff.front = 0;
		rxAppointment();
		rxLed_on();
	}
	BuffStatus_TypeDef popRxBuff(uint8_t *pData){
		if(rxBuff.front == rxBuff.rear)
			return BUFF_EMPTY;
		*pData = rxBuff.buff[rxBuff.rear];
		if(++rxBuff.rear >= UART_BUFF_CNT)
			rxBuff.rear = 0;
		return BUFF_CONTAIN;
	}

	/* tx buffer */
	void pushTxBuff(uint8_t data){
		txBuff.buff[txBuff.front] = data;
		if(++txBuff.front >= UART_BUFF_CNT)
			txBuff.front = 0;
	}
	//미사용
//	BuffStatus_TypeDef popTxBuff(uint8_t *pData){
//		if(txBuff.front == txBuff.rear)
//			return BUFF_EMPTY;
//		*pData = txBuff.buff[txBuff.rear];
//		if(++txBuff.rear >= UART_BUFF_CNT)
//			txBuff.rear = 0;
//	}

	/* tx option */
	void txByte(uint8_t data){
		pushTxBuff(data);
		txBuffCheck_Transmit();
	}
	void txStream(uint8_t *pData, uint32_t size){
		uint32_t ptr = 0;
		while(size --)
			pushTxBuff(pData[ptr++]);
		txBuffCheck_Transmit();
	}

	/* send handle */
	void txBuffCheck_Transmit(){
		uint32_t size = 0;

		if (txBuff.front == txBuff.rear){
			rs485_rxMode();
			txLed_off();
			return;
		}

		if (txBuff.front > txBuff.rear)
		{
			size = txBuff.front - txBuff.rear;
		}
		else
		{
			size = UART_BUFF_CNT - txBuff.rear;
		}
		rs485_txMode();
		txLed_on();

		//HAL_NVIC_DisableIRQ(UART_IRQn_);
		//HAL_UART_STATE_READY 가 아니면 버퍼에 담았다가 다음에 전송한다.
		HAL_UART_Transmit_IT(huart_, &txBuff.buff[txBuff.rear], size);
		//HAL_NVIC_EnableIRQ(UART_IRQn_);
	}

	void TxCpltCallback(UART_HandleTypeDef *huart){
		if(huart != huart_)
			return;
		txBuff.rear += huart_->TxXferSize;
		txBuff.rear %= UART_BUFF_CNT;
//		if(txBuff.rear >= UART_BUFF_CNT)
//		{
//			txBuff.rear -= UART_BUFF_CNT;
//		}

		txBuffCheck_Transmit();
	}

	//-------DXL
	int ring_buffer_usage(SerialByteBuff* rb) {
	    if(rb->rear <= rb->front) {
	        return rb->front - rb->rear;
	    } else {
	        return (UART_BUFF_CNT - rb->rear) + rb->front + 1;
	    }
	}

	int available(){
		return ring_buffer_usage(&rxBuff);
	}

	void write(char c){
		txByte(c);
	}
	void write(const char *str){
		int ptr = 0;
		while (str[ptr] != 0)
			pushTxBuff(str[ptr++]);
		txBuffCheck_Transmit();
	}
	int write(uint8_t *packet, int length){
		int ptr = 0;
		while (ptr != length)
			pushTxBuff(packet[ptr++]);
		txBuffCheck_Transmit();
		return ptr;
	}


	uint8_t read(){
		uint8_t ret = 0;
		popRxBuff(&ret);
		return ret;
	}
};




#endif /* INC_UART_Class_H_ */
