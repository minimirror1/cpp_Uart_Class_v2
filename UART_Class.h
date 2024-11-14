/*
 * UART_Class.h
 *
 *  Created on: Dec 13, 2023
 *      Author: minim
 */

#ifndef INC_UART_Class_H_
#define INC_UART_Class_H_

#include "main.h"
#include "cpp_tick.h"

/*
종속성
1. cpp_tick.h
https://github.com/minimirror1/cpp_tick

포팅.
Uart callback 연결 코드
1. main.c 에서 main 함수 아래 코드 추가 (다른곳도 가능)
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	serial1.TxCpltCallback(huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	serial1.RxCpltCallback(huart);
}

2. 루프 안에 추가할 코드 (주기적으로 호출되는 위치에 추가)
serial1.loop();
*/


#define UART_BUFF_CNT 4096*2

typedef enum _BuffStatus_TypeDef {
    BUFF_EMPTY = 0,
    BUFF_CONTAIN
} BuffStatus_TypeDef;

#pragma pack(1)
struct SerialByteBuff {
    uint8_t buff[UART_BUFF_CNT];
    uint32_t front;
    uint32_t rear;
};
#pragma pack()

class Serial {
private:
    struct GPIO_Ctr {
        GPIO_TypeDef *Port_;
        uint16_t Pin_;
        GPIO_PinState OnState_;
        GPIO_PinState OffState_;
        uint32_t t_Off;
        bool init;
    };

    /* property */
    UART_HandleTypeDef *huart_;
    IRQn_Type UART_IRQn_;
    SerialByteBuff txBuff;
    SerialByteBuff rxBuff;
    uint8_t rxData_;
    GPIO_Ctr txLed;
    GPIO_Ctr rxLed;
    Tick rxLed_off_Tick;
    GPIO_Ctr rs485;

    /* private member functions */
    void rxLed_on();
    void rxLed_off();
    void txLed_on();
    void txLed_off();
    void rs485_txMode();
    void rs485_rxMode();
    void pushTxBuff(uint8_t data);
    void rxAppointment();
    int ring_buffer_usage(SerialByteBuff* rb);

	/* LED control */
    void rxLed_Check();

public:
    Serial();
    ~Serial();

    /* initialization functions */
    void init(UART_HandleTypeDef *huart, IRQn_Type UART_IRQn);
    void init_txLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState);
    void init_rxLed(GPIO_TypeDef *Port, uint16_t Pin, GPIO_PinState OnState);
    void init_rs485(GPIO_TypeDef *Port, uint16_t Pin);

    /* UART control */
	void loop();
    void rxAppointCheck();
    void RxCpltCallback(UART_HandleTypeDef *huart);
    BuffStatus_TypeDef popRxBuff(uint8_t *pData);
    void txByte(uint8_t data);
    void txStream(uint8_t *pData, uint32_t size);
    void txBuffCheck_Transmit();
    void TxCpltCallback(UART_HandleTypeDef *huart);

    /* DXL interface */
    int available();
    void write(char c);
    void write(const char *str);
    int write(uint8_t *packet, int length);
    uint8_t read();
};

#endif /* INC_UART_Class_H_ */
