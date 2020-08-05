#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <math.h>
#include <string.h>
#include "sx126x.h"
#include "sx126x_hal.h"
#include "radio.h"
#include "rf.h"
#include "stdio.h"








const uint32_t ch_freq_list[32] = {470000000, 471000000, 472000000, 473000000, 474000000, 475000000, 476000000, 477000000, 478000000, 479000000,
 480000000, 481000000, 482000000, 483000000, 484000000, 485000000, 486000000, 487000000, 488000000, 489000000, 490000000, 491000000, 492000000,
  493000000, 494000000, 495000000, 496000000, 497000000, 498000000, 499000000, 500000000, 501000000};

// uint8_t ch_group[8][JUMP_CH_COUNT] = 
// {
//     {29, 4, 23, 9, 22, 20, 27, 1, 10, 13, 5, 6, 25, 18, 30, 11, 14, 24, 7, 15, 3, 2, 8, 12, 28, 17, 16, 19, 26, 21},
//     {23, 1, 18, 17, 15, 13, 24, 20, 6, 22, 27, 5, 9, 12, 11, 7, 19, 28, 3, 25, 21, 30, 29, 10, 16, 8, 4, 26, 2, 14},
//     {27, 12, 16, 30, 22, 14, 29, 6, 10, 3, 4, 20, 25, 28, 21, 9, 7, 13, 17, 23, 2, 8, 19, 1, 15, 18, 5, 24, 11, 26},
//     {8, 25, 15, 4, 13, 23, 11, 2, 29, 9, 26, 21, 7, 19, 24, 10, 27, 17, 12, 18, 14, 16, 6, 20, 3, 5, 22, 30, 1, 28},
//     {15, 22, 12, 29, 19, 26, 18, 13, 1, 14, 27, 24, 6, 20, 23, 21, 8, 11, 2, 4, 10, 25, 16, 5, 9, 7, 17, 3, 30, 28},
//     {27, 15, 24, 7, 2, 22, 18, 12, 10, 3, 21, 16, 1, 28, 4, 19, 30, 29, 13, 20, 11, 25, 5, 8, 9, 26, 14, 6, 23, 17},
//     {11, 12, 2, 20, 3, 15, 10, 6, 24, 22, 18, 25, 19, 23, 1, 8, 17, 13, 26, 27, 30, 28, 4, 9, 16, 29, 14, 5, 21, 7},
//     {28, 17, 6, 5, 30, 10, 1, 29, 23, 4, 16, 18, 11, 2, 24, 22, 21, 19, 25, 20, 13, 27, 14, 8, 9, 7, 26, 12, 15, 3},
// };


uint8_t ch_group[8][JUMP_CH_COUNT] = 
{
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
};

uint32_t now_ch;

struct_rf_status_manage rf_status_manage[2];

uint32_t random_num;

int8_t rssi;

uint8_t enable_print_syn;

RadioError_t RadioError;

RadioEvents_t Radio_1_Events;
RadioEvents_t Radio_2_Events;

volatile uint32_t rf_slot_count;
volatile uint8_t rf_slot;
uint32_t rf_timer_sec;


struct_rf_syn rf_syn;
struct_rf_updata rf_updata;
struct_rf_ack rf_ack;

struct_sensor_list sensor_list[SENSOR_MAX_COUNT];

extern RadioStatus_t RadioStatus;
extern osThreadId_t rf_callbackHandle;
extern SX126x_t *p_sx126x;

extern char debug_str[1024];

extern TIM_HandleTypeDef htim2;

extern void debug_isr(const char* pstr);
extern void debug(const char* pstr);


void OnTxDone( void );

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

void OnTxTimeout( void );

void OnRxTimeout( void );																																																																																																																																																			

void OnRxError( void );

void OnCadDone( bool channelActivityDetected);


void OnTxDone2( void );

void OnRxDone2( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

void OnTxTimeout2( void );

void OnRxTimeout2( void );

void OnRxError2( void );

void OnCadDone2( bool channelActivityDetected);


int32_t rf_set_ch(uint8_t rf_index,uint32_t freq);

void rf_rev_packet_insert_list(uint8_t rf_index,void *pdata,uint8_t size, int16_t rssi, int8_t snr);

void rf_event_first_handle(uint8_t index);

//按照分车阈值来处理未处理的事件
void fcyz_delay_handle(uint8_t index);


int32_t calc_tow_event_time(SNP_EVENT_t e1,uint32_t rev_slot_count_1,SNP_EVENT_t e2,uint32_t rev_slot_count_2);

void poll_event_need_handle(void);

void event_stat_handle(uint8_t index);

int32_t calc_event_to_now_time(SNP_EVENT_t e1,uint32_t rev_slot_count_1);


/*** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
unsigned short const crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

unsigned short  crc16_byte(unsigned short crc, const unsigned char data)
{
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}
/***
 * crc16 - compute the CRC-16 for the data buffer
 * @crc:	previous CRC value
 * @buffer:	data pointer
 * @len:	number of bytes in the buffer
 *
 * Returns the updated CRC value.
 */
unsigned short crc16(unsigned short crc, unsigned char const *buffer,int len)
{
	while (len--)
		crc = crc16_byte(crc, *buffer++);
	return crc;
}







void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;;
	
	if(GPIO_PIN_3 == GPIO_Pin)
	{
		if(rf_callbackHandle != NULL)
		{
			xTaskNotifyFromISR(rf_callbackHandle,BIT_RF2,eSetBits,&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
	}
	else if(GPIO_PIN_9 == GPIO_Pin)
	{
		if(rf_callbackHandle != NULL)
		{		
			xTaskNotifyFromISR(rf_callbackHandle,BIT_RF1,eSetBits,&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );		
		}
	}
}




void rf_init(uint8_t rf_index)
{
	HAL_GPIO_WritePin(L506_PWR_EN_GPIO_Port,L506_PWR_EN_Pin,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(RF1_LED_CRC_ERROR_GPIO_Port,RF1_LED_CRC_ERROR_Pin,GPIO_PIN_RESET);
	
	rf_status_manage[rf_index].rf_work_status = 0;

	if(rf_index == RF1)	
	{
		Radio_1_Events.TxDone = OnTxDone;
		Radio_1_Events.RxDone = OnRxDone;
		Radio_1_Events.TxTimeout = OnTxTimeout;
		Radio_1_Events.RxTimeout = OnRxTimeout;
		Radio_1_Events.RxError = OnRxError;
		Radio_1_Events.CadDone = OnCadDone;	
		Radio.Init(&Radio_1_Events);

		USE_RF_1
		RadioStatus = SX126xGetStatus();
		rf_status_manage[RF1].rf_work_status = RadioStatus.Fields.ChipMode;		
		rf_status_manage[RF1].rf_last_rx_tick = xTaskGetTickCount();
		rf_status_manage[RF1].rf_init_ok = 1;
		rf_status_manage[RF1].reboot_count++;		
	}
	else if(rf_index == RF2)
	{
		Radio_2_Events.TxDone = OnTxDone2;
		Radio_2_Events.RxDone = OnRxDone2;
		Radio_2_Events.TxTimeout = OnTxTimeout2;
		Radio_2_Events.RxTimeout = OnRxTimeout2;
		Radio_2_Events.RxError = OnRxError2;
		Radio_2_Events.CadDone = OnCadDone2;
		
		Radio.Init(&Radio_2_Events);
	
		USE_RF_2
		RadioStatus = SX126xGetStatus();
		rf_status_manage[RF2].rf_work_status = RadioStatus.Fields.ChipMode;		
		rf_status_manage[RF2].rf_last_rx_tick = xTaskGetTickCount();
		rf_status_manage[RF2].rf_init_ok = 1;
		rf_status_manage[RF2].reboot_count++;			
	}

	//random_num = SX126xGetRandom();


	
}



uint32_t get_now_jump_ch()
{
	now_ch = ch_freq_list[ch_group[rf_syn.jump_ch_group][rf_syn.head.packet_seq%30]];
	return now_ch;
}


int32_t rf_set_ch(uint8_t rf_index,uint32_t freq)
{
	if(rf_index == RF1 && rf_status_manage[RF1].rf_init_ok == 1)
	{
		USE_RF_1
		Radio.SetChannel(freq);
		Radio.RxBoosted(0);
	}
	else if(rf_index == RF2 && rf_status_manage[RF2].rf_init_ok == 1)
	{
		USE_RF_2
		Radio.SetChannel(freq);		
		Radio.RxBoosted(0);		
	}
	else
		return -1;
	
	return 0;
}


int32_t rf_send(uint8_t rf_index,void *pdata,uint8_t size)
{
	if(rf_index == RF1)
	{
		if(rf_status_manage[RF1].rf_init_ok == 1)
		{
			USE_RF_1
			HAL_GPIO_WritePin(RF1_LED_1_GPIO_Port,RF1_LED_1_Pin,GPIO_PIN_RESET);			
			rf_status_manage[RF1].rf_send_count++;
			rf_status_manage[RF1].rssi = Radio.Rssi(MODEM_LORA);
			rf_status_manage[RF1].rssi_add += rf_status_manage[RF1].rssi;
			rf_status_manage[RF1].rssi_count++;			
			Radio.Send(pdata,size);
			RadioStatus = SX126xGetStatus();
			rf_status_manage[RF1].rf_work_status = RadioStatus.Fields.ChipMode;
		}
	}
	else if(rf_index == RF2)
	{
		if(rf_status_manage[RF2].rf_init_ok == 1)
		{	
			USE_RF_2			
			HAL_GPIO_WritePin(RF2_LED_2_GPIO_Port,RF2_LED_2_Pin,GPIO_PIN_RESET);					
			rf_status_manage[RF2].rf_send_count++;			
			rf_status_manage[RF2].rssi = Radio.Rssi(MODEM_LORA);	
			rf_status_manage[RF2].rssi_add += rf_status_manage[RF2].rssi;
			rf_status_manage[RF2].rssi_count++;					
			Radio.Send(pdata,size);
			RadioStatus = SX126xGetStatus();
			rf_status_manage[RF2].rf_work_status = RadioStatus.Fields.ChipMode;	
		}			
	}	
	else
		return -1;
	
	return 0;	
}












																					

void OnTxDone( void )
{
	HAL_GPIO_WritePin(RF1_LED_1_GPIO_Port,RF1_LED_1_Pin,GPIO_PIN_SET);
	rf_status_manage[RF1].rf_send_ok_count++;
	rf_set_ch(RF1,get_now_jump_ch());
	Radio.RxBoosted( 0);
	RadioStatus = SX126xGetStatus();
	rf_status_manage[RF1].rf_work_status = RadioStatus.Fields.ChipMode;	
	if(rf_slot == 0 && enable_print_syn == 0)
		return;	
	sprintf(debug_str,"rf0 send ok mode=%d\r\n",rf_status_manage[RF1].rf_work_status);
	debug_isr(debug_str);	
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	//Radio.Standby( );
	Radio.RxBoosted( 0);
	RadioStatus = SX126xGetStatus();
	rf_status_manage[RF1].rf_work_status = RadioStatus.Fields.ChipMode;  	
	rf_status_manage[RF1].rf_last_rx_tick = xTaskGetTickCount();
	rf_rev_packet_insert_list(RF1,payload,size,  rssi,  snr);
}

void OnTxTimeout( void )
{
    Radio.Standby( );

}

void OnRxTimeout( void )
{
   // Radio.Standby( );

}

void OnRxError( void )
{
    Radio.Standby( );
	debug("**********************************************rf0 hal crc error\r\n");
}

void OnCadDone( bool channelActivityDetected)
{
    Radio.Standby( );

}


void OnTxDone2( void )
{
	HAL_GPIO_WritePin(RF2_LED_2_GPIO_Port,RF2_LED_2_Pin,GPIO_PIN_SET);
	rf_status_manage[RF2].rf_send_ok_count++;	
	rf_set_ch(RF2,get_now_jump_ch());
	Radio.RxBoosted( 0);
	RadioStatus = SX126xGetStatus();
	rf_status_manage[RF2].rf_work_status = RadioStatus.Fields.ChipMode; 
	if(rf_slot == 1 && enable_print_syn == 0)
		return;
	sprintf(debug_str,"rf1 send ok mode=%d\r\n",rf_status_manage[RF2].rf_work_status);
	debug_isr(debug_str);		
}

void OnRxDone2( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	//Radio.Standby( );
  	Radio.RxBoosted(0);
	RadioStatus = SX126xGetStatus();
	rf_status_manage[RF2].rf_work_status = RadioStatus.Fields.ChipMode;  	
	rf_status_manage[RF2].rf_last_rx_tick = xTaskGetTickCount();
	rf_rev_packet_insert_list(RF2,payload,size,  rssi,  snr);
}

void OnTxTimeout2( void )
{
    Radio.Standby( );

}

void OnRxTimeout2( void )
{
    Radio.Standby( );
	
}

void OnRxError2( void )
{
    Radio.Standby( );
	debug("**********************************************rf1 hal crc error\r\n");
}

void OnCadDone2( bool channelActivityDetected)
{
    Radio.Standby( );

}

//监控rf 出错后进行重启
void monitor_rf_status()
{
	uint32_t now_tick;
	
	now_tick = xTaskGetTickCount();
	
	if((now_tick-rf_status_manage[RF1].rf_last_rx_tick) > RF_NO_RX_REBOOT_TICKS ||
		(rf_status_manage[RF1].rf_send_count - rf_status_manage[RF1].rf_send_ok_count) > RF_SEND_ERROR_REBOOT_COUNT)
	{
		rf_status_manage[RF1].rf_init_ok = 0;
		rf_status_manage[RF1].rf_send_count = 0;
		rf_status_manage[RF1].rf_send_ok_count = 0;
	}
	if((now_tick-rf_status_manage[RF2].rf_last_rx_tick) > RF_NO_RX_REBOOT_TICKS||
		(rf_status_manage[RF2].rf_send_count - rf_status_manage[RF2].rf_send_ok_count) > RF_SEND_ERROR_REBOOT_COUNT)
	{
		rf_status_manage[RF2].rf_init_ok = 0;
		rf_status_manage[RF2].rf_send_count = 0;
		rf_status_manage[RF2].rf_send_ok_count = 0;		
	}
	
	if(rf_status_manage[RF1].rf_init_ok == 0)
	{
		rf_init(RF1);
	}
	if(rf_status_manage[RF2].rf_init_ok == 0)
	{
		rf_init(RF2);
	}	
}

void task_rf_callback(void *argument)
{
	uint32_t notify_value;
	
	extern TIM_HandleTypeDef htim2;
	HAL_TIM_Base_Start_IT(&htim2);
	
	for(;;)
	{
		xTaskNotifyWait(0xffffffff,0xffffffff,&notify_value,pdMS_TO_TICKS(1000));

		monitor_rf_status();
		if(notify_value & BIT_RF1)
		{
			USE_RF_1
			Radio.IrqProcess(&Radio_1_Events);
			continue;
		}																																																																																																																													
		if(notify_value & BIT_RF2)
		{
			USE_RF_2
			Radio.IrqProcess(&Radio_2_Events);			
			continue;
		}
		//超时 处理

		// times++;
		// if(times%2)
		// 	rf_send(RF1,"1234567890123",13);
		// else
		// 	rf_send(RF2,"1234567890123",13);
	}
}



void rf_send_syn(uint8_t rf_index)
{
	if(rf_index == RF1)
	{		
		rf_syn.head.packet_type = RF1_AP_SYN;
		if(++rf_syn.head.packet_seq>=PACKET_SEQ_MAX)
			rf_syn.head.packet_seq = 0;	
		rf_set_ch(rf_index,ch_freq_list[0]);	
	}
	else if(rf_index == RF2)
	{		
		rf_syn.head.packet_type = RF2_AP_SYN;
		rf_set_ch(rf_index,ch_freq_list[31]);
	}
	rf_syn.head.cmd = CMD_NONE;
	rf_syn.head.dev_id = DEV_ID&0xffff;;
	rf_syn.jump_ch_group = DEV_ID%8;
	rf_syn.head.crc = crc16(0,(uint8_t *)&rf_syn.head.dev_id,sizeof(rf_syn)-2);

	rf_send(rf_index,&rf_syn,sizeof(rf_syn));	
	if(enable_print_syn)
	{
		sprintf(debug_str,"rf_%d: [%d %d]send syn id=%04X GRP=%d    ",rf_index,rf_slot,htim2.Instance->CNT/84,rf_syn.head.dev_id,rf_syn.jump_ch_group);
		debug_isr(debug_str);	
	}	
}


void rf_send_updata(uint8_t rf_index)
{
	if(rf_updata.addr == 0)
		return;

	if(rf_index == RF1)
	{		
		rf_updata.head.packet_type = RF1_AP_UPDATA;	
	}
	else if(rf_index == RF2)
	{		
		rf_updata.head.packet_type = RF2_AP_UPDATA;
	}
	rf_set_ch(rf_index,get_now_jump_ch());
	rf_updata.head.cmd = CMD_NONE;
	rf_updata.head.dev_id = DEV_ID&0xffff;;
	rf_updata.head.packet_seq = rf_syn.head.packet_seq;

	rf_updata.head.crc = crc16(0,(uint8_t *)&rf_updata.head.dev_id,sizeof(rf_updata)-2);

	rf_send(rf_index,&rf_updata,sizeof(rf_updata));		
}


void rf_send_ack(uint8_t rf_index)
{
	if(rf_ack.ack_bit[0] == 0 && rf_ack.ack_bit[1] == 0 && rf_ack.ack_bit[2] == 0 && rf_ack.ack_bit[3] == 0 
				&& rf_ack.ack_bit[4] == 0 && rf_ack.ack_bit[5] == 0 && rf_ack.ack_bit[6] == 0)
		return;

	if(rf_index == RF1)
	{		
		rf_ack.head.packet_type = RF1_AP_ACK;	
	}
	else if(rf_index == RF2)
	{		
		rf_ack.head.packet_type = RF2_AP_ACK;
	}
	rf_set_ch(rf_index,get_now_jump_ch());
	rf_ack.jump_ch_group = DEV_ID%8;
	rf_ack.head.dev_id = DEV_ID&0xffff;
	rf_ack.head.packet_seq = rf_syn.head.packet_seq;

	rf_ack.head.crc = crc16(0,(uint8_t*)&rf_ack.head.dev_id,sizeof(rf_ack)-2);

	rf_send(rf_index,&rf_ack,sizeof(rf_ack));	
	sprintf(debug_str,"rf_%d: [%d %d]send ack %x %x %x %x %x %x %x sensor_id=%X slot=%d    ",rf_index,rf_slot,htim2.Instance->CNT/84,rf_ack.ack_bit[0],rf_ack.ack_bit[1],rf_ack.ack_bit[2],
			rf_ack.ack_bit[3],rf_ack.ack_bit[4],rf_ack.ack_bit[5],rf_ack.ack_bit[6],rf_ack.sensor_id,rf_ack.slot);
	debug_isr(debug_str);	
	if(rf_index == RF2)
	{		
		memset(rf_ack.ack_bit,0,7);
		rf_ack.sensor_id = 0;
		rf_ack.slot = 0;
		rf_ack.head.cmd = CMD_NONE;
	}		
}

void timer_sec(uint32_t t)
{
	if(rf_slot_count%256 == 0) //1秒
	{
		rf_timer_sec++;
		HAL_GPIO_TogglePin(S1_R_GPIO_Port,S1_R_Pin);
	}
}

//每个时间槽1/256 秒
void rf_timer(void)
{
	
	rf_slot_count++;
	rf_slot = rf_slot_count%64;
	
	timer_sec(rf_slot_count);
	
	switch(rf_slot)
	{
		case 0:    //同步包
			rf_send_syn(RF1);
			break;
		case 1:    //同步包
			rf_send_syn(RF2);
			break;			
		case 2:     //1 2 升级包
			rf_send_updata(RF1);
			break;	
		case 4:     //1 2 升级包
			rf_send_updata(RF2);
			break;	
	
		case 60:   //ack包
			rf_send_ack(RF1);
			break;		
		case 62:   //ack包
			rf_send_ack(RF2);
			break;				
	}
	poll_event_need_handle();		
	
}

int32_t find_sensor_index(uint16_t sensor_id)
{
	int32_t i;
	for(i=0;i<SENSOR_MAX_COUNT; i++)
	{
		if(sensor_list[i].sensor_id == sensor_id)
			return i;		
	}	
	return -1;
}


void set_ack_packet_bit(uint8_t slot)
{
	uint8_t index = slot - SENSOR_SLOT_BEGIN;

	rf_ack.ack_bit[index/8] |= (1<<(index%8));

}

void rf_rev_packet_insert_list(uint8_t rf_index,void *pdata,uint8_t size, int16_t rssi, int8_t snr)
{
	uint16_t crc;
	int32_t index = 0;
	int8_t new_packet_insert_flag = 0;

	struct_rf_syn *p_rf_syn = (struct_rf_syn *)pdata;
	struct_rf_updata *p_rf_updata = (struct_rf_updata *)pdata;
	struct_rf_ack *p_rf_ack = (struct_rf_ack *)pdata;
	struct_rf_event *p_rf_event = (struct_rf_event *)pdata;
	struct_rf_stat *p_rf_stat = (struct_rf_stat *)pdata;		

	struct_rf_event *p_rf_event_list ;

	if(size <3)
		return;

	crc = crc16(0,(uint8_t *)pdata+2,size-2);

	if(crc != p_rf_syn->head.crc )
	{
		sprintf(debug_str,"rf_%d:**********************************************soft crc error id=%04X type=%d\r\n",rf_index,p_rf_event->head.packet_seq,p_rf_event->head.dev_id);
		debug(debug_str);
		return;
	}

	if(p_rf_syn->head.packet_type == RF1_AP_SYN || p_rf_syn->head.packet_type == RF2_AP_SYN)
	{

	}
	else if(p_rf_updata->head.packet_type == RF1_AP_UPDATA || p_rf_updata->head.packet_type == RF2_AP_UPDATA)
	{

	}
	else if(p_rf_ack->head.packet_type == RF1_AP_ACK || p_rf_ack->head.packet_type == RF2_AP_ACK)
	{

	}	
	else if(p_rf_syn->head.packet_type == RF1_AP_ACK || p_rf_syn->head.packet_type == RF2_AP_ACK)
	{

	}
	else if(p_rf_event->head.packet_type == RF_S_EVENT)//事件包
	{
		index = find_sensor_index(p_rf_event->head.dev_id); //找到索引
		if(index != -1)
		{
			set_ack_packet_bit(rf_slot);
			if(size >= 14)
				sprintf(debug_str,"rf_%d: [%d:%d]rev event sensor_id=%X slot=%d->%d seq=%d resend=%d e1=%04X e2=%04X e3=%04X \r\n",rf_index,rssi,snr,p_rf_event->head.dev_id,rf_slot,htim2.Instance->CNT/84,p_rf_event->head.packet_seq,p_rf_event->resend_count,p_rf_event->event[0].uiAll,p_rf_event->event[1].uiAll,p_rf_event->event[2].uiAll);
			else if(size >= 12)
				sprintf(debug_str,"rf_%d: [%d:%d]rev event sensor_id=%X slot=%d->%d seq=%d resend=%d e1=%04X e2=%04X \r\n",rf_index,rssi,snr,p_rf_event->head.dev_id,rf_slot,htim2.Instance->CNT/84,p_rf_event->head.packet_seq,p_rf_event->resend_count,p_rf_event->event[0].uiAll,p_rf_event->event[1].uiAll);
			else
				sprintf(debug_str,"rf_%d: [%d:%d]rev event sensor_id=%X slot=%d->%d seq=%d resend=%d e1=%04X \r\n",rf_index,rssi,snr,p_rf_event->head.dev_id,rf_slot,htim2.Instance->CNT/84,p_rf_event->head.packet_seq,p_rf_event->resend_count,p_rf_event->event[0].uiAll);
			
			debug(debug_str);	

			if(sensor_list[index].sensor_data.sensor_event[0].size == 0) //第一包
			{
				sensor_list[index].sensor_data.sensor_event[0].size = size;  //前后数据数组都放入这一条数据
				sensor_list[index].sensor_data.sensor_event[1].size = size;						
				memcpy(sensor_list[index].sensor_data.sensor_event[0].event_packet,pdata,size);
				memcpy(sensor_list[index].sensor_data.sensor_event[1].event_packet,pdata,size); 
				new_packet_insert_flag = 1;	
			}
			else //非第一条数据
			{
				p_rf_event_list = (struct_rf_event *)&sensor_list[index].sensor_data.sensor_event[1].event_packet; 
				if(p_rf_event_list->head.packet_seq != p_rf_event->head.packet_seq)//新数据,严格按照包序号区分重复数据
				{
					sensor_list[index].sensor_data.sensor_event[1].size = size;           //新数据来了直接扔进sensor_event[1]  ， 所以必须保证这里不能有没有处理过的数据
					sensor_list[index].sensor_data.sensor_event[1].rev_slot_count = rf_slot_count;																																																																																									
					memcpy(sensor_list[index].sensor_data.sensor_event[1].event_packet,pdata,size); //新数据 需要触发数据处理
					new_packet_insert_flag = 1;	
					if(p_rf_event->resend_count < sensor_list[index].sensor_stat.last_resend_times) //当前包的重传次数小于记录的重传次数 说明已经传输成功，把上次的重传次数记下来
					{
						sensor_list[index].sensor_stat.resend_count[0] += sensor_list[index].sensor_stat.last_resend_times;
						if(sensor_list[index].sensor_stat.last_resend_times>=31)
							sensor_list[index].sensor_stat.resend_count[31]++;
						else
						{
							sensor_list[index].sensor_stat.resend_count[sensor_list[index].sensor_stat.last_resend_times]++;
						}
						
					}
				}
				else //包序号相同 即是重复数据	
				{
					if(sensor_list[index].sensor_data.sensor_event[1].rev_slot_count == rf_slot_count) //同一帧不同rf收到的重复数据
					{

					}
					else //不同帧的重复数据，需要做重复数据统计
					{
						/* code */
					}
					
				}
				sensor_list[index].sensor_stat.last_resend_times = p_rf_event->resend_count;
				sensor_list[index].sensor_stat.rssi[rf_index] = rssi;
				sensor_list[index].sensor_stat.snr[rf_index] = snr;
				sensor_list[index].sensor_stat.rssi_add[rf_index] += rssi;
				sensor_list[index].sensor_stat.snr_add[rf_index] += snr;
				sensor_list[index].sensor_stat.rssi_count[rf_index]++;	
				sensor_list[index].sensor_stat.rssi_avg[rf_index] = sensor_list[index].sensor_stat.rssi_add[rf_index]/sensor_list[index].sensor_stat.rssi_count[rf_index];	
				sensor_list[index].sensor_stat.snr_avg[rf_index] = sensor_list[index].sensor_stat.snr_add[rf_index]/sensor_list[index].sensor_stat.rssi_count[rf_index];	
								
			}
			if(new_packet_insert_flag == 1)//插入了新数据
			{
				rf_event_first_handle(index);
			}
			
		}
		else  //收到不在list中的sensor 事件包 先不做处理
		{
			/* code */
		}
		
	}	
	else if(p_rf_stat->head.packet_type == RF_S_STAT)
	{
		index = find_sensor_index(p_rf_stat->head.dev_id); //找到索引
		if(index != -1)
		{
			set_ack_packet_bit(rf_slot);
			sprintf(debug_str,"rf_%d: [%d:%d]rev stat sensor_id=%X slot=[%d->%d]->%d seq=%d rev_rssi=%d bandid=%04X battery=%d hv=%d sv=%d\r\n",rf_index,rssi,snr,p_rf_stat->head.dev_id,rf_slot,p_rf_stat->slot,htim2.Instance->CNT/84,p_rf_stat->head.packet_seq,p_rf_stat->rx_rssi,p_rf_stat->band_id,p_rf_stat->battery,p_rf_stat->h_version,p_rf_stat->s_version);
			
			debug(debug_str);	

			
			if(sensor_list[index].slot != p_rf_stat->slot || p_rf_stat->band_id != (DEV_ID&0XFFFF)) //时间槽错误或者绑定ID错误
			{
				rf_ack.sensor_id = p_rf_stat->head.dev_id;
				rf_ack.slot = sensor_list[index].slot;		
				rf_ack.head.cmd = CMD_SET_SLOT;		
			}
			sensor_list[index].sensor_cfg_rev.battery = p_rf_stat->battery;
			sensor_list[index].sensor_cfg_rev.rx_rssi = p_rf_stat->rx_rssi;
			sensor_list[index].sensor_cfg_rev.h_version = p_rf_stat->h_version;
			sensor_list[index].sensor_cfg_rev.s_version = p_rf_stat->s_version;									
		}
		else
		{
			if(p_rf_stat->band_id == (DEV_ID&0XFFFF))  //不在list中 但是绑定ID相同   下发解绑命令
			{
				set_ack_packet_bit(rf_slot);
				rf_ack.sensor_id = p_rf_stat->head.dev_id;
				rf_ack.head.cmd = CMD_CLEAR_BAND_ID;
				rf_ack.slot = 0;					 //时间槽为0 表示解绑该sensor
			}
		}
		
	}
	else if(p_rf_stat->head.packet_type == RF_S_UPDATA_ACK)
	{
		index = find_sensor_index(p_rf_event->head.dev_id); //找到索引
		if(index != -1)
		{
			set_ack_packet_bit(rf_slot);
		}
		else
		{
			/* code */
		}
		
	}			
}





//确认收到新的事件包，后调用该函数，
// 1.取出正确的事件包
// 2.做第一次处理
void rf_event_first_handle(uint8_t index)
{
	struct_rf_event *p_event_packet_1;
	struct_rf_event *p_event_packet_2;
	int8_t i;
	int8_t seq_cha;
	int8_t event_num;
	uint8_t lost_event_count;
	uint8_t size_2;
	uint32_t event_time;

	p_event_packet_1 = (struct_rf_event *)(sensor_list[index].sensor_data.sensor_event[0].event_packet);
	p_event_packet_2 = (struct_rf_event *)(sensor_list[index].sensor_data.sensor_event[1].event_packet);

	size_2 = sensor_list[index].sensor_data.sensor_event[1].size;

	//取出有效事件，严格按照前后包的包序号判断，包序号差一表示有一个有效事件
	seq_cha = p_event_packet_2->head.packet_seq - p_event_packet_1->head.packet_seq;
	if(seq_cha < 0)
		seq_cha += 256; //应该有的事件数

	event_num = (size_2 - 8)/2; //实际有的事件数
	lost_event_count = seq_cha - event_num;
	if(lost_event_count>0)
		sensor_list[index].sensor_stat.lost_event_count += lost_event_count;

	if(event_num>seq_cha && seq_cha!=0) //抛弃重复数据 ,seq_cha==0是第一包数据  不做该处理
		event_num = seq_cha;
	
	sensor_list[index].sensor_stat.event_count += event_num; //统计实际收到的事件数量

	for(i=event_num-1;i>=0;i--)  //事件约定最新的在数组前面，所以从后面开始取数据
	{
		if(sensor_list[index].event_calc.now_sensor_onoff == 0 && sensor_list[index].event_calc.now_event_need_handle == 0) //没有ON OFF 状态  收到的第一包数据
		{
			sensor_list[index].event_calc.event[0] = p_event_packet_2->event[i];  //把确定on off的事件放在计算用的事件里
			sensor_list[index].event_calc.rev_slot_count[0] = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count; //收到事件包的时间
			if(p_event_packet_2->event[0].blIsOn == E_ON)
				sensor_list[index].event_calc.now_sensor_onoff = S_ON;
			else
				sensor_list[index].event_calc.now_sensor_onoff = S_OFF;	
			sensor_list[index].event_calc.onoff_time_ms = 100;	
			sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成
			event_stat_handle(index);
		}
		else
		{
			if(sensor_list[index].event_calc.now_event_need_handle == 0) //表示没有待处理的事件
			{
				if(sensor_list[index].event_calc.now_sensor_onoff == S_OFF)  //OFF状态已经确认 说明已经经过了分车阈值的时间  ON肯定是有效的
				{
					if(p_event_packet_2->event[i].blIsOn == E_ON)  //OFF->ON
					{
						sensor_list[index].event_calc.event[1] = p_event_packet_2->event[i]; 					//event[1] 存放待处理的事件
						sensor_list[index].event_calc.rev_slot_count[1] = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count;

						event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
						p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
						sensor_list[index].event_calc.onoff_time_ms = event_time;

						sensor_list[index].event_calc.now_event_need_handle = 1;  //event 没有处理完成	
						fcyz_delay_handle(index);  //可能事件延迟送达，在这里可以进行立刻确认
						
					}
					else //OFF->OFF   可能是丢了一个ON事件  或者是ON事件被分车阈值过滤掉了
					{

					}

				}
				else  // ON 状态已经被确认
				{
					if(p_event_packet_2->event[i].blIsOn == E_ON) //可能是丢了一个OFF事件 或者 OFF事件被过滤掉了 ，按理说ON也会同时被过滤掉 所以不可能是这种情况
					{
						
					}
					else //ON->OFF   先把OFF事件放到未确定的事件里  等待分车阈值的确认
					{						
						sensor_list[index].event_calc.event[1] = p_event_packet_2->event[i]; //event[1] 存放待处理的事件
						sensor_list[index].event_calc.rev_slot_count[1] = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count;

						event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
						p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
						sensor_list[index].event_calc.onoff_time_ms = event_time;

						sensor_list[index].event_calc.now_event_need_handle = 1;  //event 没有处理完成	
						fcyz_delay_handle(index);   //可能事件延迟送达，在这里可以进行立刻确认
						 
					}	
				}
				
			}
			else  //有待处理的事件，提前结算事件
			{
				if(sensor_list[index].event_calc.event[1].blIsOn == E_OFF) //待处理的数据是OFF  
				{
					if(p_event_packet_2->event[i].blIsOn == E_ON) //新来的是ON事件
					{
						//计算两个事件的实际时间，通过接收时间和事件中的时间戳来综合计算
						event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
						p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
						
						if(event_time > sensor_list[index].sensor_cfg.off_to_on_min_time) //大于分车阈值 OFF有效 OFF没有时间输出  ON有效
						{
							sensor_list[index].event_calc.event[0] = sensor_list[index].event_calc.event[1];
							sensor_list[index].event_calc.rev_slot_count[0] = sensor_list[index].event_calc.rev_slot_count[1];						
							sensor_list[index].event_calc.now_sensor_onoff = S_OFF;
							sensor_list[index].event_calc.onoff_time_ms = event_time;
							sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成
							event_stat_handle(index);

							//ON转化为待处理事件
							sensor_list[index].event_calc.event[1] = p_event_packet_2->event[i]; 					//event[1] 存放待处理的事件
							sensor_list[index].event_calc.rev_slot_count[1] = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count;

							event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
							p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
							sensor_list[index].event_calc.onoff_time_ms = event_time;

							sensor_list[index].event_calc.now_event_need_handle = 1;  //event 没有处理完成	
							fcyz_delay_handle(index);  //可能事件延迟送达，在这里可以进行立刻确认
												

						}
						else  //小于分车阈值   OFF  ON 全部丢弃
						{
							sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成	相当于把当前event[1] 中未确认的OFF事件丢弃  当前的ON事件也不处理 丢弃了
						}
						
					}
					else  //新来的是OFF   OFF->OFF   ON丢失
					{
						//只丢弃现有的事件  未确认的OFF事件不作处理
					}
					
				}
				else //未处理的是ON事件  
				{
					if(p_event_packet_2->event[i].blIsOn == E_OFF) //新来的是OFF事件 立即确认ON事件
					{					
						sensor_list[index].event_calc.event[0] = sensor_list[index].event_calc.event[1];
						sensor_list[index].event_calc.rev_slot_count[0] = sensor_list[index].event_calc.rev_slot_count[1];						
						sensor_list[index].event_calc.now_sensor_onoff = S_ON;

						event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
						p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
						sensor_list[index].event_calc.onoff_time_ms = event_time;
						event_stat_handle(index);
						sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成	
						
						//OFF事件转换为待处理事件
						sensor_list[index].event_calc.event[1] = p_event_packet_2->event[i]; 					//event[1] 存放待处理的事件
						sensor_list[index].event_calc.rev_slot_count[1] = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count;

						event_time = calc_two_event_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1],
						p_event_packet_2->event[i],sensor_list[index].sensor_data.sensor_event[1].rev_slot_count);
						sensor_list[index].event_calc.onoff_time_ms = event_time;

						sensor_list[index].event_calc.now_event_need_handle = 1;  //event 没有处理完成	
						fcyz_delay_handle(index);  //可能事件延迟送达，在这里可以进行立刻确认	
					}
					else //新来的是ON事件  表示中间丢了一个OFF
					{
						//只丢弃现有的事件  未确认的ON事件不作处理
					}
										
				}
				
			}
			
		}						
	}
	//把第二包数据挪到第一包上
	memcpy(p_event_packet_1,p_event_packet_2,size_2); 
	sensor_list[index].sensor_data.sensor_event[0].size = sensor_list[index].sensor_data.sensor_event[1].size;
	sensor_list[index].sensor_data.sensor_event[0].rev_slot_count = sensor_list[index].sensor_data.sensor_event[1].rev_slot_count;		
	
}


void poll_event_need_handle(void)
{
	int32_t index;
	for(index=0;index<SENSOR_MAX_COUNT; index++)
	{
		if(sensor_list[index].event_calc.now_event_need_handle == 1)
		{
			fcyz_delay_handle(index);
		}	
	}	


}

//定时器中调用  按照分车阈值来处理未处理的事件 可能事件延迟送达，在这里可以进行立刻确认
void fcyz_delay_handle(uint8_t index)
{
	int32_t ms_to_now;

	if(sensor_list[index].event_calc.now_event_need_handle == 1)
	{
		//计算事件的持续的实际时间，通过接收时间和事件中的时间戳来综合计算
		ms_to_now = calc_event_to_now_time(sensor_list[index].event_calc.event[1],sensor_list[index].event_calc.rev_slot_count[1]);
		if(sensor_list[index].event_calc.event[1].blIsOn == E_OFF)
		{
			if(ms_to_now > sensor_list[index].sensor_cfg.off_to_on_min_time) //大于分车阈值 OFF有效 OFF没有时间输出  ON有效
			{
				sensor_list[index].event_calc.event[0] = sensor_list[index].event_calc.event[1];
				sensor_list[index].event_calc.rev_slot_count[0] = sensor_list[index].event_calc.rev_slot_count[1];						
				sensor_list[index].event_calc.now_sensor_onoff = S_OFF;				
				event_stat_handle(index);
				sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成					
			}
		}
		else
		{
			if(ms_to_now > sensor_list[index].sensor_cfg.on_delay) //大于on delay
			{
				sensor_list[index].event_calc.event[0] = sensor_list[index].event_calc.event[1];
				sensor_list[index].event_calc.rev_slot_count[0] = sensor_list[index].event_calc.rev_slot_count[1];						
				sensor_list[index].event_calc.now_sensor_onoff = S_ON;
				event_stat_handle(index);
				sensor_list[index].event_calc.now_event_need_handle = 0;  //event 已经处理完成					
			}			
		}
		
	}

}


//计算两个事件的实际时间，通过接收时间和事件中的时间戳来综合计算
int32_t calc_two_event_time(SNP_EVENT_t e1,uint32_t rev_slot_count_1,SNP_EVENT_t e2,uint32_t rev_slot_count_2)
{
	int32_t rev_time_cha;  //数据接收相差的时间
	int32_t ms_cha,ms_1,ms_2;

	if(rev_slot_count_2 > rev_slot_count_1)
		rev_time_cha = rev_slot_count_2 - rev_slot_count_1;
	else //时间槽计数器值满归0 或者别的异常则无法处理
	{
		rev_time_cha = (0xffffffff - rev_slot_count_2) + rev_slot_count_1;
	}
	
	rev_time_cha = rev_time_cha/256/30;    //表示间隔几个30秒

	ms_2 = (int32_t)((e2.bmSec * 1024) + e2.bmMs)*1000/1024;
	ms_1 = (int32_t)((e1.bmSec * 1024) + e1.bmMs)*1000/1024;
	ms_cha = ms_2 - ms_1;
	if(ms_cha<0)
		ms_cha += 30000;

	if((ms_cha/30) < rev_time_cha)
		ms_cha += (rev_time_cha - (ms_cha/30))*30000;
	
	return ms_cha;
}

//计算事件到现在的时间
int32_t calc_event_to_now_time(SNP_EVENT_t e1,uint32_t rev_slot_count_1)
{
	int32_t now_sec,now_ms;
	int32_t ms_cha;

	now_sec = rf_syn.head.packet_seq%30;
	now_ms = (rf_slot_count%64)*1000/256;

	ms_cha = (now_sec*1000+now_ms) - (int32_t)((e1.bmSec * 1024) + e1.bmMs)*1000/1024;
	if(ms_cha < 0)
		ms_cha += 30000;

	return ms_cha;
}

//完成ON OFF状态的统计和输出工作
void event_stat_handle(uint8_t index)
{

}



















