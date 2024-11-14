# UART 클래스 라이브러리

STM32 HAL 드라이버를 사용하는 UART 통신을 위한 C++ 클래스 라이브러리입니다.

## 주요 기능

- UART 송수신 버퍼 관리 (링 버퍼)
- RS485 통신 지원
- TX/RX LED 상태 표시 기능
- 인터럽트 기반 비동기 통신

## 종속성

- [cpp_tick 라이브러리](https://github.com/minimirror1/cpp_tick)
- STM32 HAL 드라이버

## 사용 방법

### 1. 초기화

```cpp
Serial serial1; // 객체 생성
serial1.init(&huart1, USART1_IRQn); // UART 초기화
// LED 초기화 (선택사항)
serial1.init_txLed(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
serial1.init_rxLed(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
// RS485 초기화 (선택사항)
serial1.init_rs485(GPIOA, GPIO_PIN_3);
```

### 2. 콜백 함수 연결
main.c 파일에 다음 콜백 함수를 추가하세요:
```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef huart) {
serial1.TxCpltCallback(huart);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef huart) {
serial1.RxCpltCallback(huart);
}
```
이미 콜백 함수를 사용하고 있는경우, 콜백 함수 내에 위 코드를 추가하세요.


### 3. 메인 루프에서 호출
주기적으로 호출되는 위치에 다음 코드를 추가하세요:

예를들어 main 함수 안에 추가할 경우 while 루프 안에 추가합니다.
```cpp
serial1.loop();
```



## 주요 메서드

### 데이터 송신
- `void txByte(uint8_t data)`: 1바이트 전송
- `void txStream(uint8_t *pData, uint32_t size)`: 여러 바이트 전송
- `void write(char c)`: 문자 전송
- `void write(const char *str)`: 문자열 전송
- `int write(uint8_t *packet, int length)`: 패킷 전송

### 데이터 수신
- `int available()`: 수신 버퍼에 있는 데이터 개수 확인
- `uint8_t read()`: 1바이트 읽기
- `BuffStatus_TypeDef popRxBuff(uint8_t *pData)`: 수신 버퍼에서 데이터 추출

## 버퍼 크기

기본 버퍼 크기는 8192바이트(4096*2)로 설정되어 있습니다. 필요한 경우 UART_BUFF_CNT 매크로를 수정하여 조정할 수 있습니다.

## 라이선스

MIT License