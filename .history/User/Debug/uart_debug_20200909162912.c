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
void print_syn(int8_t *param);
void clear(int8_t *param);

extern struct_sensor_list sensor_list[SENSOR_MAX_COUNT];


extern struct_ip_port ip_port[MAX_CLIENT_COUNT];

extern uint8_t enable_print_syn;

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
CMD_CALLBACK("print_syn",print_syn)
CMD_CALLBACK("clear",clear)
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
	uint16_t len;
	HAL_GPIO_WritePin(S3_R_GPIO_Port,S3_R_Pin,GPIO_PIN_RESET);
	HAL_DMA_Abort((&DEBUG_UART)->hdmarx);
	DEBUG_UART.RxState = HAL_UART_STATE_READY;
	DEBUG_UART.hdmarx->State = HAL_DMA_STATE_READY;
	//huart6[0] = Q_LEN-DMA1_Stream1->NDTR;
	//memcpy(debug_uart_buff+1,(char*)debug_uart_dma_buff,Q_LEN-DMA1_Stream1->NDTR);
	len = Q_LEN-DEBUG_UART.hdmarx->Instance->NDTR;
	if(len > 0)
		xStreamBufferSendFromISR(sbh_debug_rev,debug_uart_dma_buff,len,NULL);
	HAL_UART_Receive_DMA(&DEBUG_UART,debug_uart_dma_buff,Q_LEN);	 //??DMA??
	HAL_GPIO_WritePin(S3_R_GPIO_Port,S3_R_Pin,GPIO_PIN_SET);
}




void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &DEBUG_UART)
	{
		if(mutex_huart3_hdmatx != NULL)
		{
			xSemaphoreGiveFromISR( mutex_huart3_hdmatx,NULL);
			HAL_GPIO_WritePin(S4_R_GPIO_Port,S4_R_Pin,GPIO_PIN_SET);
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
			sbh_debug_send = xStreamBufferCreate(4096,3);
			if(sbh_debug_send!=NULL)
			{
				sys_start_send();
			}
			else
				osDelay( pdMS_TO_TICKS(100));
		}
		else
		{
			if( mutex_huart3_hdmatx != NULL )
			{
				if( xSemaphoreTake( mutex_huart3_hdmatx, pdMS_TO_TICKS(100) ) == pdTRUE )
				{
					rev_size = xStreamBufferReceive(sbh_debug_send,&send_data[0],128,xBlockTime);
					if(rev_size>0)
					{					
						HAL_GPIO_WritePin(S4_R_GPIO_Port,S4_R_Pin,GPIO_PIN_RESET);
						HAL_UART_Transmit_DMA(&DEBUG_UART,(uint8_t *)send_data,rev_size);
					}
					else
						xSemaphoreGive( mutex_huart3_hdmatx);
				}
				else //100MS 还没有发送成功 可能是DMA故障了   重启dma
				{
					xSemaphoreGive( mutex_huart3_hdmatx);
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
	
	for(;;)
	{
		if(str_data[0] == '\r' || str_data[0] == '\n') 
		{
			memcpy(str_data,&str_data[1],127);
		}
		else
		{
			break;
		}
		
	}

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

//func:从StreamBuffer中读一行数据，依据\r或者\n都判断为行结束 返回数据剔除\r \n 自动剔除无效的\r\n
//return  NULL 表示没有读到， 非NULL 表示字符串起点
int8_t * get_line(StreamBufferHandle_t sbh,int8_t *p_str,uint16_t size)
{
	static uint16_t str_end = 0;          // 字符串终止位置，将\r 或者\n 替换为0
	static uint16_t str_data_index = 0;   //缓冲中字符串的个数
	uint16_t rev_size;
	
	if(p_str[str_end] == 0 && str_data_index > 0) //已经成功返回过一行字符串，这里需要把这个处理过的字符串剔除
	{
		memcpy(p_str,&p_str[str_end+1],size-str_end-1);
		str_data_index -= (str_end+1);
		str_end = 0;
	} 
	if(str_data_index >= size) //缓冲区满 去掉第一个字节
	{
		memcpy(p_str,&p_str[1],size-1);	
		str_data_index--;	
		return NULL;
	}
	for(str_end=0;str_end<str_data_index;str_end++)//剔除只有\r \n 没有实体数据
	{
		if(p_str[str_end] == '\r' || p_str[str_end] == '\n')
		{
			p_str[str_end] = 0;
			if(str_end == 0) //只有\r \n 没有实体数据
				return NULL;
			else
				return p_str;
		}	
	}

	rev_size = xStreamBufferReceive(sbh_debug_rev,&p_str[str_data_index],size-str_data_index,pdMS_TO_TICKS( 10000 ));
	if(rev_size > 0)
	{
		str_data_index += rev_size;

		for(str_end=0;str_end<str_data_index;str_end++)
		{
			if(p_str[str_end] == '\r' || p_str[str_end] == '\n')
			{
				p_str[str_end] = 0;
				if(str_end == 0) //只有\r \n 没有实体数据
					return NULL;
				else
					return p_str;
			}
		}
		//没有找到\r或者\n
		str_end = 0;
		return NULL;
	}

}

void str_cmd_hanle(char *str_data)
{
	uint16_t i = 0;
	uint16_t param_start = 0;

	for(;;)
	{
		if(str_data[i] == ' ' ) //找到这个说明是带参数的命令
		{
			str_data[i] = 0;
			param_start = i+1;
			break;
		}
		if(str_data[i] == 0 ) //找到这个说明是无参数的命令
		{	
			break;
		}
		i++;			
	}
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
}

void task_uart_debug_rev_handle(void *argument)
{

	int8_t str_data[128];  //默认一条数据不能大于128  否则将会丢弃
	uint8_t str_data_index = 0;
	const TickType_t xBlockTime = pdMS_TO_TICKS( 10000 );
	uint8_t rev_size;
	int32_t glth_ret = 0;
	int8_t *p;

	memset(str_data,0,128);
	for(;;)
	{
		if(sbh_debug_rev == NULL)  //内存不够分配  延时1秒后继续申请
		{		
			sbh_debug_rev = xStreamBufferCreate(4096,2);
			if(sbh_debug_rev != NULL) 
				start_from_debug_dma_receive();
			else
				osDelay( pdMS_TO_TICKS(100));
		}
		else
		{
			p = get_line(sbh_debug_rev,str_data,128);
			if(p)
			{
				str_cmd_hanle(p);
			}
			// rev_size = xStreamBufferReceive(sbh_debug_rev,&str_data[str_data_index],128-str_data_index,xBlockTime);
			// if(rev_size > 0)
			// {
			// 	do
			// 	{
			// 		glth_ret = get_line_to_handle(str_data);
			// 	}
			// 	while(glth_ret == -1);//读取一行字符串 进行处理 多余的数据跟下一条数据合并
			// 	str_data_index = glth_ret;
			// }
		}
		
	}	
}



void debug(const char* pstr)
{
	uint16_t len;
	static SemaphoreHandle_t mutex = NULL;
	
	if(mutex == NULL)
	{
		mutex = xSemaphoreCreateMutex();
	}
	
	if(sbh_debug_send == NULL)
		return;

	if(pdTRUE == xSemaphoreTake(mutex,0))
	{
		len = strlen(pstr);
		if(len>1024)
			len = 1024;
		xStreamBufferSend(sbh_debug_send,pstr,len,0);	
		xSemaphoreGive(mutex);
	}
}

void debug_isr(const char* pstr)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(sbh_debug_send == NULL)
		return;
	xStreamBufferSendFromISR(sbh_debug_send,pstr,strlen(pstr),&xHigherPriorityTaskWoken);	
}

void callback_fun_help(int8_t *param)
{
	char *help_str = "500M_Ap help:\r\n\
[0]:setip [0-1] [x.x.x.x] [0-65535]       func:set server 0 or 1 ip and port\r\n\
[0]:setapid [12345678]  hex    			  func:set apid hex 4Bytes\r\n\
[1]:add_sensor	[0-127] [sensor id hex]   func:set sensor id 0-127 hex\r\n\
[2]:list                                  func: list sensor id and slot...\r\n\
[3]:print_syn [0-1]                       func: 0 close print syn ,1 open print syn\r\n"; 
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

void setapid(int8_t *param)
{
	
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
	sensor_list[index].slot = index+6;

}


void list_sensor(int8_t *param)
{
	int32_t i;
	for(i=0;i<SENSOR_MAX_COUNT; i++)
	{
		if(sensor_list[i].sensor_id != 0)
		{
			sprintf(debug_str,"[%d] id=%04X slot=%d lane=%d rev_e=%d lost_e=%d rssi=%d:%d snr=%d:%d rs=%d rs_1->31=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \r\n",i,sensor_list[i].sensor_id,sensor_list[i].slot,sensor_list[i].sensor_cfg.lane,
					sensor_list[i].sensor_stat.event_count,sensor_list[i].sensor_stat.lost_event_count,sensor_list[i].sensor_stat.rssi_avg[0],sensor_list[i].sensor_stat.rssi_avg[1],sensor_list[i].sensor_stat.snr_avg[0],sensor_list[i].sensor_stat.snr_avg[1],sensor_list[i].sensor_stat.resend_count[0],
					sensor_list[i].sensor_stat.resend_count[1],sensor_list[i].sensor_stat.resend_count[2],sensor_list[i].sensor_stat.resend_count[3],sensor_list[i].sensor_stat.resend_count[4],sensor_list[i].sensor_stat.resend_count[5],sensor_list[i].sensor_stat.resend_count[6],sensor_list[i].sensor_stat.resend_count[7],
					sensor_list[i].sensor_stat.resend_count[8],sensor_list[i].sensor_stat.resend_count[9],sensor_list[i].sensor_stat.resend_count[10],sensor_list[i].sensor_stat.resend_count[11],sensor_list[i].sensor_stat.resend_count[12],sensor_list[i].sensor_stat.resend_count[13],sensor_list[i].sensor_stat.resend_count[14],
					sensor_list[i].sensor_stat.resend_count[15],sensor_list[i].sensor_stat.resend_count[16],sensor_list[i].sensor_stat.resend_count[17],sensor_list[i].sensor_stat.resend_count[18],sensor_list[i].sensor_stat.resend_count[19],sensor_list[i].sensor_stat.resend_count[20],sensor_list[i].sensor_stat.resend_count[21],
					sensor_list[i].sensor_stat.resend_count[22],sensor_list[i].sensor_stat.resend_count[23],sensor_list[i].sensor_stat.resend_count[24],sensor_list[i].sensor_stat.resend_count[25],sensor_list[i].sensor_stat.resend_count[26],sensor_list[i].sensor_stat.resend_count[27],sensor_list[i].sensor_stat.resend_count[28],
					sensor_list[i].sensor_stat.resend_count[29],sensor_list[i].sensor_stat.resend_count[30],sensor_list[i].sensor_stat.resend_count[31]);
			debug(debug_str);				
		}		
	}	

}


void clear(int8_t *param)
{
	int32_t i;
	for(i=0;i<SENSOR_MAX_COUNT; i++)
	{
		if(sensor_list[i].sensor_id != 0)
		{
			memset(&sensor_list[i].sensor_stat,0,sizeof(sensor_list[i].sensor_stat));
		}
	}
	debug("clear ok\r\n");
}

void print_syn(int8_t *param)
{
	uint32_t enable = 0;

	sscanf((const char *)param,"%d",&enable);	

	enable_print_syn = enable;
	sprintf(debug_str,"print syn %d\r\n",enable_print_syn);
	debug(debug_str);			
}







