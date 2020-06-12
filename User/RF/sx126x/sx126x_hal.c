#include "main.h"
#include "sx126x.h"
#include "sx126x_hal.h"





extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi5;

#define BoardDisableIrq( ) __disable_irq()
#define BoardEnableIrq( ) __enable_irq()

SX126x_t sx126x_1;

SX126x_t sx126x_2;


SX126x_t *p_sx126x;



void init_io_spi_sx126x_x()
{
	sx126x_1.Spi = hspi4;
	sx126x_1.DIO1.GPIOx = RF1_DIO1_EXTI9_GPIO_Port;
	sx126x_1.DIO1.GPIO_Pin = RF1_DIO1_EXTI9_Pin;	
	sx126x_1.DIO2.GPIOx = RF1_DIO2_GPIO_Port;	
	sx126x_1.DIO2.GPIO_Pin = RF1_DIO2_Pin;	
	sx126x_1.DIO3.GPIOx = RF1_DIO3_GPIO_Port;	
	sx126x_1.DIO3.GPIO_Pin = RF1_DIO3_Pin;
	sx126x_1.BUSY.GPIOx = RF1_BUSY_GPIO_Port;
	sx126x_1.BUSY.GPIO_Pin = RF1_BUSY_Pin;
	sx126x_1.Nss.GPIOx = RF1_NSS_GPIO_Port;
	sx126x_1.Nss.GPIO_Pin = RF1_NSS_Pin;
	sx126x_1.Reset.GPIOx = RF1_RESET_GPIO_Port;
	sx126x_1.Reset.GPIO_Pin = RF1_RESET_Pin;
	
	
	sx126x_2.Spi = hspi5;
	sx126x_2.DIO1.GPIOx = RF2_DIO1_EXTI3_GPIO_Port;
	sx126x_2.DIO1.GPIO_Pin = RF2_DIO1_EXTI3_Pin;	
	sx126x_2.DIO2.GPIOx = RF2_DIO2_GPIO_Port;	
	sx126x_2.DIO2.GPIO_Pin = RF2_DIO2_Pin;	
	sx126x_2.DIO3.GPIOx = RF2_DIO3_GPIO_Port;	
	sx126x_2.DIO3.GPIO_Pin = RF2_DIO3_Pin;	
	sx126x_2.BUSY.GPIOx = RF2_BUSY_GPIO_Port;
	sx126x_2.BUSY.GPIO_Pin = RF2_BUSY_Pin;
	sx126x_2.Nss.GPIOx = RF2_NSS_GPIO_Port;
	sx126x_2.Nss.GPIO_Pin = RF2_NSS_Pin;
	sx126x_2.Reset.GPIOx = RF2_RESET_GPIO_Port;
	sx126x_2.Reset.GPIO_Pin = RF2_RESET_Pin;		
}


void SX126xReset( void )
{
	uint32_t delay;
    
    HAL_GPIO_WritePin( p_sx126x->Reset.GPIOx, p_sx126x->Reset.GPIO_Pin,GPIO_PIN_RESET);
    delay = 10000;
		while(delay--)
			;
    HAL_GPIO_WritePin( p_sx126x->Reset.GPIOx, p_sx126x->Reset.GPIO_Pin,GPIO_PIN_SET); // internal pull-up
    
}

void SX126xWaitOnBusy( void )
{
	uint32_t delay = 10000;
	while(delay--);
    //while( HAL_GPIO_ReadPin( p_sx126x->BUSY.GPIOx, p_sx126x->BUSY.GPIO_Pin) == GPIO_PIN_SET );
}

void SX126xWakeup( void )
{
	uint8_t data[2];
   
	BoardDisableIrq( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = RADIO_GET_STATUS;
	data[1] = 0;
	
	HAL_SPI_Transmit(&p_sx126x->Spi,data,2,1);

	HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

    // Wait for chip to be ready.
   SX126xWaitOnBusy( );

   BoardEnableIrq( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	uint8_t data[2];
	
  SX126xCheckDeviceReady( );

	data[0] = command;
	
   HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

   HAL_SPI_Transmit(&p_sx126x->Spi,data,1,1);

	 HAL_SPI_Transmit(&p_sx126x->Spi,buffer,size,1);

   HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	uint8_t data[2];
	
  SX126xCheckDeviceReady( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = command;
	data[1] = 0;
	
	HAL_SPI_Transmit(&p_sx126x->Spi,data,2,1);
	
	HAL_SPI_TransmitReceive(&p_sx126x->Spi,buffer,buffer,size,1);

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

   SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	uint8_t data[3];
    
	SX126xCheckDeviceReady( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = RADIO_WRITE_REGISTER;
	data[1] = ( address & 0xFF00 ) >> 8 ;	
	data[2] = address & 0x00FF;	

	HAL_SPI_Transmit(&p_sx126x->Spi,data,3,1);
	
	HAL_SPI_Transmit(&p_sx126x->Spi,buffer,size,1);

	HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

  SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{

	uint8_t data[4];
    
	SX126xCheckDeviceReady( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = RADIO_READ_REGISTER;
	data[1] = ( address & 0xFF00 ) >> 8 ;	
	data[2] = address & 0x00FF;	
	data[3] = 0;
	
	HAL_SPI_Transmit(&p_sx126x->Spi,data,4,1);
	
	HAL_SPI_TransmitReceive(&p_sx126x->Spi,buffer,buffer,size,1);

	HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

  SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	uint8_t data[2];
	
  SX126xCheckDeviceReady( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = RADIO_WRITE_BUFFER;
	data[1] = offset;
	
	HAL_SPI_Transmit(&p_sx126x->Spi,data,2,1);

	HAL_SPI_Transmit(&p_sx126x->Spi,buffer,size,1);

	HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

  SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	uint8_t data[3];
    
	SX126xCheckDeviceReady( );

  HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_RESET);

	data[0] = RADIO_READ_BUFFER;
	data[1] = offset ;	
	data[2] = 0;		

	HAL_SPI_Transmit(&p_sx126x->Spi,data,3,1);
	
	HAL_SPI_TransmitReceive(&p_sx126x->Spi,buffer,buffer,size,1);

	HAL_GPIO_WritePin( p_sx126x->Nss.GPIOx, p_sx126x->Nss.GPIO_Pin,GPIO_PIN_SET);

   SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetPaSelect( uint32_t channel )
{
//    if( GpioRead( &DeviceSel ) == 1 )
//    {
//        return SX1261;
//    }
//    else
    {
        return SX1262;
    }
}

void SX126xAntSwOn( void )
{
//    GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SX126xAntSwOff( void )
{
 //   GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return True;
}






