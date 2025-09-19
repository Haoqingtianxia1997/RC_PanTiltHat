#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "main.h"

// nRF24L01寄存器地址
#define CONFIG          0x00
#define EN_AA           0x01
#define EN_RXADDR       0x02
#define SETUP_AW        0x03
#define SETUP_RETR      0x04
#define RF_CH           0x05
#define RF_SETUP        0x06
#define STATUS          0x07
#define RX_ADDR_P0      0x0A
#define RX_ADDR_P1      0x0B
#define TX_ADDR         0x10
#define RX_PW_P0        0x11
#define RX_PW_P1        0x12
#define FIFO_STATUS     0x17

// nRF24L01指令
#define R_RX_PAYLOAD    0x61
#define W_TX_PAYLOAD    0xA0
#define FLUSH_TX        0xE1
#define FLUSH_RX        0xE2
#define REUSE_TX_PL     0xE3
#define R_REGISTER      0x00
#define W_REGISTER      0x20
#define NOP             0xFF

// GPIO控制宏
#define NRF24_CSN_LOW()   HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_RESET)
#define NRF24_CSN_HIGH()  HAL_GPIO_WritePin(GPIOA, CSN_Pin, GPIO_PIN_SET)
#define NRF24_CE_LOW()    HAL_GPIO_WritePin(GPIOA, CE_Pin, GPIO_PIN_RESET)
#define NRF24_CE_HIGH()   HAL_GPIO_WritePin(GPIOA, CE_Pin, GPIO_PIN_SET)

// 函数声明
void NRF24_Init(void);
uint8_t NRF24_Check(void);
void NRF24_RX_Mode(void);
uint8_t NRF24_RX_Data(uint8_t *rxbuf);
void NRF24_Write_Reg(uint8_t reg, uint8_t value);
uint8_t NRF24_Read_Reg(uint8_t reg);

#endif
