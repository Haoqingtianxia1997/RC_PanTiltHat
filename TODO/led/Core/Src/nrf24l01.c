#include "nrf24l01.h"

extern SPI_HandleTypeDef hspi1;

// SPI读写一个字节
uint8_t SPI_RW(uint8_t dat)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&hspi1, &dat, &rx_data, 1, 1000);
    return rx_data;
}

// 写寄存器
void NRF24_Write_Reg(uint8_t reg, uint8_t value)
{
    NRF24_CSN_LOW();
    SPI_RW(W_REGISTER | reg);
    SPI_RW(value);
    NRF24_CSN_HIGH();
}

// 读寄存器
uint8_t NRF24_Read_Reg(uint8_t reg)
{
    uint8_t reg_val;
    NRF24_CSN_LOW();
    SPI_RW(R_REGISTER | reg);
    reg_val = SPI_RW(NOP);
    NRF24_CSN_HIGH();
    return reg_val;
}

// 写多字节数据
void NRF24_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t datalen)
{
    NRF24_CSN_LOW();
    SPI_RW(W_REGISTER | reg);
    for(uint8_t i = 0; i < datalen; i++)
    {
        SPI_RW(pBuf[i]);
    }
    NRF24_CSN_HIGH();
}

// 读多字节数据
void NRF24_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t datalen)
{
    NRF24_CSN_LOW();
    SPI_RW(R_REGISTER | reg);
    for(uint8_t i = 0; i < datalen; i++)
    {
        pBuf[i] = SPI_RW(NOP);
    }
    NRF24_CSN_HIGH();
}

// 检查nRF24L01是否存在
uint8_t NRF24_Check(void)
{
    uint8_t test_addr[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t read_addr[5] = {0};
    
    // 写测试地址
    NRF24_Write_Buf(TX_ADDR, test_addr, 5);
    // 读回地址
    NRF24_Read_Buf(TX_ADDR, read_addr, 5);
    
    // 比较地址
    for(uint8_t i = 0; i < 5; i++)
    {
        if(read_addr[i] != test_addr[i])
            return 1; // 检测失败
    }
    return 0; // 检测成功
}

// 初始化nRF24L01为接收模式
void NRF24_Init(void)
{
    // uint8_t addr[5] = {0x30, 0x30, 0x30, 0x30, 0x31}; // "00001"对应的ASCII码
		uint8_t addr[5] = {'0', '0', '0', '0', '1'};
	
    NRF24_CE_LOW();
    
    NRF24_Write_Reg(CONFIG, 0x0F);        // 配置：16位CRC，接收模式，上电
    NRF24_Write_Reg(EN_AA, 0x01);         // 使能通道0自动应答
    NRF24_Write_Reg(EN_RXADDR, 0x01);     // 使能通道0接收地址
    NRF24_Write_Reg(SETUP_AW, 0x03);      // 5字节地址宽度
    NRF24_Write_Reg(SETUP_RETR, 0x1A);    // 重传延时500us，重传10次
    NRF24_Write_Reg(RF_CH, 76);           // 76号频道（匹配Arduino）
    NRF24_Write_Reg(RF_SETUP, 0x06);      // 发射功率0dBm，传输速率1Mbps（匹配Arduino）
    
    NRF24_Write_Buf(RX_ADDR_P0, addr, 5); // 设置接收地址为"00001"
    NRF24_Write_Reg(RX_PW_P0, 12);        // 接收数据长度12字节（3个float）
    
    NRF24_Write_Reg(STATUS, 0x70);        // 清除所有中断标志
    NRF24_Write_Reg(FLUSH_RX, NOP);       // 清空RX FIFO
}

// 设置为接收模式
void NRF24_RX_Mode(void)
{
    NRF24_CE_LOW();
    NRF24_Write_Reg(CONFIG, 0x0F);  // 接收模式
    NRF24_CE_HIGH();
    HAL_Delay(1);
}

// 接收数据
uint8_t NRF24_RX_Data(uint8_t *rxbuf)
{
    uint8_t sta = NRF24_Read_Reg(STATUS);
    
    if(sta & 0x40) // 接收到数据
    {
        NRF24_CSN_LOW();
        SPI_RW(R_RX_PAYLOAD);
        for(uint8_t i = 0; i < 12; i++)
        {
            rxbuf[i] = SPI_RW(NOP);
        }
        NRF24_CSN_HIGH();
        
        NRF24_Write_Reg(STATUS, sta); // 清除中断标志
        return 0; // 接收成功
    }
    return 1; // 没有数据
}
