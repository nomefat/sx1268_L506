#ifndef SX126X_HAL_H_
#define SX126X_HAL_H_

#include "main.h"
#include "sx126x.h"




typedef struct _gpio
{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
}struct_gpio;



typedef struct SX126x_s
{
    struct_gpio        Reset;
    struct_gpio        BUSY;
    struct_gpio        DIO1;
    struct_gpio        DIO2;
    struct_gpio        DIO3;
		struct_gpio        Nss;
    SPI_HandleTypeDef        Spi;
    PacketParams_t PacketParams;
    PacketStatus_t PacketStatus;
    ModulationParams_t ModulationParams;
}SX126x_t;


extern SX126x_t sx126x_1;

extern SX126x_t sx126x_2;


extern SX126x_t *p_sx126x;


#define USE_RF_1 do{p_sx126x = &sx126x_1;}while(0);
#define USE_RF_2 do{p_sx126x = &sx126x_2;}while(0);


void init_io_spi_sx126x_x(void);

/*!
 * \brief HW Reset of the radio
 */
void SX126xReset( void );

/*!
 * \brief Blocking loop to wait while the Busy pin in high
 */
void SX126xWaitOnBusy( void );

/*!
 * \brief Wakes up the radio
 */
void SX126xWakeup( void );

/*!
 * \brief Send a command that write data to the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [in]  buffer        Buffer to be send to the radio
 * \param [in]  size          Size of the buffer to send
 */
void SX126xWriteCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

/*!
 * \brief Send a command that read data from the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [out] buffer        Buffer holding data from the radio
 * \param [in]  size          Size of the buffer
 */
void SX126xReadCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

/*!
 * \brief Write a single byte of data to the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 * \param [in]  value         The data to be written in radio's memory
 */
void SX126xWriteRegister( uint16_t address, uint8_t value );

/*!
 * \brief Read a single byte of data from the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 *
 * \retval      value         The value of the byte at the given address in radio's memory
 */
uint8_t SX126xReadRegister( uint16_t address );

/*!
 * \brief Sets the radio output power.
 *
 * \param [IN] power Sets the RF output power
 */
void SX126xSetRfTxPower( int8_t power );

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
uint8_t SX126xGetPaSelect( uint32_t channel );

/*!
 * \brief Initializes the RF Switch I/Os pins interface
 */
void SX126xAntSwOn( void );

/*!
 * \brief De-initializes the RF Switch I/Os pins interface
 *
 * \remark Needed to decrease the power consumption in MCU low power modes
 */
void SX126xAntSwOff( void );

/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
bool SX126xCheckRfFrequency( uint32_t frequency );



#endif

