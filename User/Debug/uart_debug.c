#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "string.h"
#include "semphr.h"
#include "stdio.h"
#include "4g_app.h"
#include "rf.h"










extern UART_HandleTypeDef DEBUG_UART;


void callback_fun_help(int8_t *param);
void sys_start_send(void);
void set_server(int8_t *param);
void add_sensor(int8_t *param);
void list_sensor(int8_t *param);


extern struct_sensor_list sensor_list[SENSOR_MAX_COUNT];


extern struct_ip_port ip_port[MAX_CLIENT_COUNT];

#define CCMRAM  __attribute__((section("CCMRAM")))



StreamBufferHandle_t sbh_debug_rev = NULL;
StreamBufferHandle_t sbh_debug_send = NULL;


SemaphoreHandle_t mutex_huart3_hdmatx = NULL; 


#define Q_LEN 512           //队列长度
uint8_t debug_uart_dma_buff[Q_LEN];       //队列数组


CCMRAM char debug_str[1024];



struct _cmd_list{
	char *cmd;
	void (*func)(int8_t *param);
};

#define CMD_CALLBACK_LIST_BEGIN const struct _cmd_list cmd_list[] = {NULL,NULL,
#define CMD_CALLBACK_LIST_END NULL,NULL};
#define CMD_CALLBACK(cmd_string,callback)	cmd_string,callback,


//在此处添加你的命令字符串和回调函数
CMD_CALLBACK_LIST_BEGIN

CMD_CALLBACK("?",callback_fun_help)		
CMD_CALLBACK("setip",set_server)	
CMD_CALLBACK("add_sensor",add_sensor)
CMD_CALLBACK("list",list_sensor)

CMD_CALLBACK_LIST_END



/**********************************************************************************************
***func: 用于单片第一次开启DMA接收
***     
***date: 2017/6/9
*** nome
***********************************************************************************************/
void start_from_debug_dma_receive()
{
	SET_BIT((&DEBUG_UART)->Instance->CR1, USART_CR1_IDLEIE);  //????????
	HAL_UART_Receive_DMA(&DEBUG_UART,debug_uart_dma_buff,Q_LEN);	 //??DMA??
}




/**********************************************************************************************
***func:串口空闲中断回调函数
***     空闲中断用来判断一包数据的结束
***date: 2017/6/9
*** nome
***********************************************************************************************/
void uart_from_debug_idle_callback()
{
	HAL_DMA_Abort((&DEBUG_UART)->hdmarx);
	DEBUG_UART.RxState = HAL_UART_STATE_READY;
	DEBUG_UART.hdmarx->State = HAL_DMA_STATE_READY;
	//huart6[0] = Q_LEN-DMA1_Stream1->NDTR;
	//memcpy(debug_uart_buff+1,(char*)debug_uart_dma_buff,Q_LEN-DMA1_Stream1->NDTR);
	
	xStreamBufferSendFromISR(sbh_debug_rev,debug_uart_dma_buff,Q_LEN-DEBUG_UART.hdmarx->Instance->NDTR,NULL);
	HAL_UART_Receive_DMA(&DEBUG_UART,debug_uart_dma_buff,Q_LEN);	 //??DMA??

}




void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &DEBUG_UART)
	{
		if(mutex_huart3_hdmatx != NULL)
		{
			xSemaphoreGiveFromISR( mutex_huart3_hdmatx,NULL);
		}
	}
}



void task_uart_debug_send(void *argument)
{
	uint8_t send_data[128];
	uint8_t rev_size;
	const TickType_t xBlockTime = pdMS_TO_TICKS( 10000 );
	
	mutex_huart3_hdmatx = xSemaphoreCreateBinary();
	xSemaphoreGive(mutex_huart3_hdmatx);
	for(;;)
	{
		if(sbh_debug_send == NULL)
		{			
			sbh_debug_send = xStreamBufferCreate(1024,3);
			if(sbh_debug_send!=NULL)
			{
				sys_start_send();
			}
			else
				osDelay( pdMS_TO_TICKS(100));
		}
		else
		{
			rev_size = xStreamBufferReceive(sbh_debug_send,&send_data[0],128,xBlockTime);
			if(rev_size>0)
			{
				if( mutex_huart3_hdmatx != NULL )
				{
					if( xSemaphoreTake( mutex_huart3_hdmatx, pdMS_TO_TICKS(100) ) == pdTRUE )
					{
						HAL_UART_Transmit_DMA(&DEBUG_UART,(uint8_t *)send_data,rev_size);
					}
					else //100MS 还没有发送成功 可能是DMA故障了   重启dma
					{

					}
				}
			}
		}
	}
}



int32_t get_line_to_handle(uint8_t *str_data)
{
	uint8_t param_start = 0;
	uint8_t str_end = 0;
	int32_t i;
	int8_t find_str_fun = 0;
	int32_t ret;
	
	for(i=0;i<128;i++)
	{
		if(str_data[i] == ' ' && param_start == 0) //找到这个说明是带参数的命令
		{
			str_data[i] = 0;
			param_start = i+1;
		}
		else if((str_data[i] == '\r' || str_data[i] == '\n') && i>0) //至少有一个字节有效
		{
			find_str_fun = 1;
			str_data[i] = 0;
			str_end = i+1;
			break;
		}
		else if(str_data[i] == 0) //0表示无效字符串
		{
			break;
		}
	}
	
	if(find_str_fun == 1) //有完整的命令字符串
	{
		for(i=0;i<sizeof(cmd_list)/sizeof(struct _cmd_list);i++)
		{
			if(strcmp((char *)str_data,cmd_list[i].cmd)==0)
			{
				if(param_start == 0)
					cmd_list[i].func(NULL);
				else
					cmd_list[i].func((int8_t *)&str_data[param_start]);
				break;
			}
		}
		//已经执行完回调函数  判断一下是否还有别的命令被读回
		if(str_data[str_end] == '\r' || str_data[str_end] == '\n') //清理上条命令遗留的回车换行符
			str_end++;
		
		if(str_end>127 || str_data[str_end] == 0) //到buf末尾 或者遇到0 都表示无有效字符串了
		{
			memset(str_data,0,128);
			ret = 0;
		}			
		else
		{
			memcpy(str_data,&str_data[str_end],128-str_end);
			ret = -1; // 还可能有命令  需要继续调用
			
		}
		
		
	}
	else
	{
		if(str_end>127) //找到最后也没有找到一条有效的字符串   清零缓冲
		{
			memset(str_data,0,128);
			ret = 0;
		}
		else  //没有找到最后 没有找到有效字符串  数据不丢弃 继续接收
		{
			ret = i;
		}
	}
	return ret;
}



void task_uart_debug_rev_handle(void *argument)
{

	uint8_t str_data[128];  //默认一条数据不能大于128  否则将会丢弃
	uint8_t str_data_index = 0;
	const TickType_t xBlockTime = pdMS_TO_TICKS( 10000 );
	uint8_t rev_size;
	int32_t glth_ret = 0;

	memset(str_data,0,128);
	for(;;)
	{
		if(sbh_debug_rev == NULL)  //内存不够分配  延时1秒后继续申请
		{		
			sbh_debug_rev = xStreamBufferCreate(1024,2);
			if(sbh_debug_rev != NULL) 
				start_from_debug_dma_receive();
			else
				osDelay( pdMS_TO_TICKS(100));
		}
		else
		{
			rev_size = xStreamBufferReceive(sbh_debug_rev,&str_data[str_data_index],128-str_data_index,xBlockTime);
			if(rev_size > 0)
			{
				do
				{
					glth_ret = get_line_to_handle(str_data);
				}
				while(glth_ret == -1);//读取一行字符串 进行处理 多余的数据跟下一条数据合并
				str_data_index = glth_ret;
			}
		}
		
	}	
}



void debug(const char* pstr)
{
	if(sbh_debug_send == NULL)
		return;
	xStreamBufferSend(sbh_debug_send,pstr,strlen(pstr),1);	
}


void callback_fun_help(int8_t *param)
{
	char *help_str = "500M_Ap help:\r\n\
[0]:setip [0-1] [x.x.x.x] [0-65535]       func:set server 0 or 1 ip and port\r\n";

	debug(help_str);	
}


void sys_start_send()
{
	char *start_str = "500M_Ap Start\r\n";
	debug(start_str);	
}



void set_server(int8_t *param)
{
	uint32_t index;
	uint32_t ip[4];
	uint32_t port;
	char *start_str ;
	sscanf((const char *)param,"%d %d.%d.%d.%d %d",&index,&ip[0],&ip[1],&ip[2],&ip[3],&port);
	if(index >=MAX_CLIENT_COUNT)
	{
		start_str = "Error:server index 0 or 1\r\n";
		debug(start_str);	
		return;
	}
	if(port > 65535)
	{
		start_str = "Error:server port must < 65536\r\n";
		debug(start_str);	
		return;
	}	
	if(ip[0]>255 || ip[1]>255 || ip[2]>255 || ip[3]>255)
	{
		start_str = "Error:server ip must < 256\r\n";
		debug(start_str);	
		return;
	}		
	ip_port[index].ip = (ip[0]<<24)|(ip[1]<<16)|(ip[2]<<8)|ip[3];
	ip_port[index].port = port;
	sprintf(debug_str,"s[%d] ip=%d.%d.%d.%d port=%d\r\n",index,ip[0],ip[1],ip[2],ip[3],port);
	debug(debug_str);	
}

void add_sensor(int8_t *param)
{
	uint32_t index;
	uint16_t sensor_id;
	char *start_str ;
	
	sscanf((const char *)param,"%d %x",&index,&sensor_id);	

	if(index >= 128)
	{
		start_str = "Error:sensor index must <128\r\n";
		debug(start_str);	
		return;
	}	
	sensor_list[index].sensor_id = sensor_id;
	sensor_list[index].slot = index;

}


void list_sensor(int8_t *param)
{
	int32_t i;
	for(i=0;i<SENSOR_MAX_COUNT; i++)
	{
		if(sensor_list[i].sensor_id != 0)
		{
			sprintf(debug_str,"[%d] id=%04X slot=%d lane=%d\r\n",i,sensor_list[i].sensor_id,sensor_list[i].slot,sensor_list[i].sensor_cfg.lane);
			debug(debug_str);				
		}		
	}	

}









