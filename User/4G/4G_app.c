#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "string.h"
#include "semphr.h"
#include "queue.h"
#include "stdio.h"
#include "4g_at_callback_func.h"
#include "4g_app.h"
#include "message_buffer.h"



#define ALL_CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD 5   //所有链接都连接失败得次数 大于后重启模块 150秒

#define CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD 10 //单个有效链接连接失败次数  大于后重启模块   1000秒

#define CCMRAM  __attribute__((section("CCMRAM")))

#define GPRS_RCV_DMA_BUFF_LENGTH (1024+512)  // 4G串口DMA接收缓冲

#define CLIENT_0_MB_BUFF_LEN (1024*64)


extern UART_HandleTypeDef huart2;
extern char debug_str[1024];
extern struct_gprs_stat gprs_stat;
extern StreamBufferHandle_t sbh_debug_send;


struct _ip_port ip_port[MAX_CLIENT_COUNT];



extern void debug(const char* pstr);


uint32_t client_connect_handle(uint8_t client);
static uint8_t gps_handle(void);


uint8_t get_str_data[256];  //默认一条字符串数据不能大于128  否则将会丢弃，用回车换行符来判断

uint8_t gprs_receive_dma_buff[GPRS_RCV_DMA_BUFF_LENGTH];


StreamBufferHandle_t sbh_4g_str_rev = NULL;

uint8_t gprs_receive_data_buff[GPRS_RCV_DMA_BUFF_LENGTH]; //一条应用数据的长度


StaticMessageBuffer_t xMessageBufferStruct_client_0;
CCMRAM uint8_t client_0_mb_buff[CLIENT_0_MB_BUFF_LEN];

MessageBufferHandle_t client_0_mbh = NULL;

MessageBufferHandle_t client_1_mbh = NULL;




/**********************************************************************************************
***func: 用于单片第一次开启DMA接收
***     
***date: 2017/6/9
*** nome
***********************************************************************************************/
void start_from_gprs_dma_receive()
{
	SET_BIT((&huart2)->Instance->CR1, USART_CR1_IDLEIE);  //打开串口空闲中断
	HAL_UART_Receive_DMA(&huart2,gprs_receive_dma_buff,GPRS_RCV_DMA_BUFF_LENGTH);	 //打开DMA接收
}




/**********************************************************************************************
***func:串口空闲中断回调函数
***     空闲中断用来判断一包数据的结束
***date: 2017/6/9
*** nome
***********************************************************************************************/
void uart_from_gprs_idle_callback()
{
	HAL_DMA_Abort((&huart2)->hdmarx);
	huart2.RxState = HAL_UART_STATE_READY;
	huart2.hdmarx->State = HAL_DMA_STATE_READY;
	
	xStreamBufferSendFromISR(sbh_4g_str_rev,gprs_receive_dma_buff,GPRS_RCV_DMA_BUFF_LENGTH-DMA1_Stream5->NDTR,NULL);
	
	HAL_UART_Receive_DMA(&huart2,gprs_receive_dma_buff,GPRS_RCV_DMA_BUFF_LENGTH);	 //打开DMA接收
		

	
}



void grps_power_off()
{
	HAL_GPIO_WritePin(L506_PWR_EN_GPIO_Port,L506_PWR_EN_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_1_8V_GPIO_Port,EN_1_8V_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(L506_LOGIC_OE_GPIO_Port,L506_LOGIC_OE_Pin,GPIO_PIN_SET);
}

void grps_power_on()
{
	HAL_GPIO_WritePin(EN_1_8V_GPIO_Port,EN_1_8V_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(L506_LOGIC_OE_GPIO_Port,L506_LOGIC_OE_Pin,GPIO_PIN_SET);	
	HAL_GPIO_WritePin(L506_PWR_EN_GPIO_Port,L506_PWR_EN_Pin,GPIO_PIN_SET);

	HAL_GPIO_WritePin(L506_PWR_KEY_GPIO_Port,L506_PWR_KEY_Pin,GPIO_PIN_RESET);
	

}

void gprs_uart_send_string(char *pstr)
{
	int num = 0;
	char *ptr = pstr;
	while(*ptr++)
	{
		num++;
		if(num>5000)return;
	}
	
	HAL_UART_Transmit_DMA(&huart2,(uint8_t *)pstr,num);
	
}

int32_t g4_uart_send_data(void *pdata,uint16_t len)
{
	if(len>1500)
		return -1;
	
	HAL_UART_Transmit_DMA(&huart2,(uint8_t *)pdata,len);	
	return 0;
}




/************************************************************************************************************************/

struct _cmd_list{
	char *cmd;
	void (*func)(const char *param);
};

#define CMD_CALLBACK_LIST_BEGIN const struct _cmd_list cmd_list_4g[] = {NULL,NULL,
#define CMD_CALLBACK_LIST_END NULL,NULL};
#define CMD_CALLBACK(cmd_string,callback)	cmd_string,callback,

//在此处添加你的命令字符串和回调函数
CMD_CALLBACK_LIST_BEGIN

CMD_CALLBACK("OK",callback_fun_at_ok)
CMD_CALLBACK("+CGREG",callback_fun_cgreg)
CMD_CALLBACK("+CSQ",callback_fun_csq)
CMD_CALLBACK(">",callback_fun_ready_send)
CMD_CALLBACK("+CREG",callback_fun_creg)
CMD_CALLBACK("Revision",callback_fun_revision)
CMD_CALLBACK("Call Ready",callback_fun_call_ready)
CMD_CALLBACK("+CEREG",callback_fun_cereg)
CMD_CALLBACK("+CIPOPEN",callback_fun_cipopen)      //+CIPOPEN:SUCCESS,1
CMD_CALLBACK("+NETOPEN" ,callback_fun_netopen)      //:SUCCESS
CMD_CALLBACK("+CIPSEND",callback_fun_cipsend)    //:SUCCESS,1,20,4
CMD_CALLBACK("+NETWORK DISCONNECTED",callback_fun_network_disconnected)  //:0
CMD_CALLBACK("ERROR",callback_fun_error)
CMD_CALLBACK("+SERVER DISCONNECTED" , callback_fun_server_disconnected) //:0CMD_CALLBACK(
CMD_CALLBACK("+CIPRXGET",callback_fun_ciprxget)          //:SUCCESS,0,0,19,callback_fun_help)
CMD_CALLBACK("+CIPSTAT",callback_fun_cipstat)
CMD_CALLBACK("+CGPSINFO",callback_fun_cgpsinfo)
CMD_CALLBACK_LIST_END

struct_gprs_stat gprs_stat;

char at_cmd_conn[50];
void g4_at_tcp_conn(int num,unsigned int ip,unsigned short port)
{
	static uint16_t localport = 0;
	memset(at_cmd_conn,0,50);

	sprintf(at_cmd_conn,AT_CMD_4GAT_CIPOPEN,num,(ip>>24)&0xff,(ip>>16)&0xff,(ip>>8)&0xff,ip&0xff,port,localport++);

	gprs_uart_send_string(at_cmd_conn);

}

void g4_at_tcp_close(int num)
{

	memset(at_cmd_conn,0,50);

	sprintf(at_cmd_conn,AT_CMD_AT_CLOSE_CONNECT,num);

	gprs_uart_send_string(at_cmd_conn);

}


void gprs_start_send_cmd(uint8_t num)
{
	memset(at_cmd_conn,0,50);

	sprintf(at_cmd_conn,AT_CMD_4GAT_CIPSEND,num,gprs_stat.con_client[num].send_len);

	gprs_uart_send_string(at_cmd_conn);  //启动发送
}

/*
return: 处理完所有数据 返回0    未处理完返回未处理得数据长度，处理完得数据已经清除


*/
int32_t get_4g_line_to_handle(uint8_t *str_data,uint16_t len)
{
	uint8_t param_start = 0;
	uint8_t str_end = 0;
	int32_t i;
	int8_t find_str_fun = 0;
	int32_t ret;
	
	for(i=0;i<len;i++)
	{
		if(str_data[i] == ':' && param_start == 0) //找到这个说明是带参数的命令
		{
			str_data[i] = 0;
			param_start = i+1;
		}
		else if(str_data[i] == '\r' || str_data[i] == '\n') //命令结尾
		{
			if(i>0)//至少有一个有效字节
			{
				find_str_fun = 1;
				str_data[i] = 0;
				str_end = i+1;
				break;
			}
			else
			{
				if(len>1)
				{
					memcpy(str_data,&str_data[1],len-1);
					return (len-1); // 还可能有命令  需要继续调用				
				}			
				else
					return 0;
			}
		}
		else if(str_data[i] == 0) //0表示无效字符串
		{
			memcpy(str_data,&str_data[i+1],len-(i+1));			 //把后面得有效数据 copy到前面 等待下次调用		
			return len-(i+1);
		}
		else if(str_data[i] == '>')
		{		
			memcpy(str_data,&str_data[i+1],len-(i+1));				
			callback_fun_ready_send(NULL);
			return len-(i+1);
		}
	}
	
	if(find_str_fun == 1) //有完整的命令字符串
	{
		for(i=0;i<sizeof(cmd_list_4g)/sizeof(struct _cmd_list);i++)
		{
			if(strcmp((char *)str_data,cmd_list_4g[i].cmd)==0)
			{
				if(param_start == 0)
					cmd_list_4g[i].func(NULL);
				else
					cmd_list_4g[i].func((const char *)&str_data[param_start]);
				break;
			}
		}
		//已经执行完回调函数  判断一下是否还有别的命令被读回
		if(str_data[str_end] == '\r' || str_data[str_end] == '\n') //清理上条命令遗留的回车换行符
			str_end++;
		
			if(str_end>=len ) //到buf末尾 表示无有效数据了
			{
				memset(str_data,0,256);
				ret = 0;
			}			
			else
			{
				memcpy(str_data,&str_data[str_end],len-str_end);
				ret = len-str_end; // 还可能有命令  需要继续调用				
			}	
			return ret;
	}
	else
	{
		return len;
	}

}

void init_4g_status_flag()
{
	uint32_t i;
	
	gprs_stat.ati_ok = 0;
	gprs_stat.creg_ok = 0;
	gprs_stat.cgreg_ok = 0;
	gprs_stat.cereg_ok = 0;
	gprs_stat.netopen = 0;
	gprs_stat.set_timeout = 0;
	gprs_stat.now_who_rx = -1;
	gprs_stat.now_who_working = -1;
	gprs_stat.send_steps = 0;
	for(i=0;i<MAX_CLIENT_COUNT;i++)
	{
		gprs_stat.con_client[i].connect_fail_times = 0;
		gprs_stat.con_client[i].connect_ok = 0;
		gprs_stat.con_client[i].reconnect_send_cmd_error_times = 0;
		gprs_stat.con_client[i].send_at_cmd_timeout = 0;
		gprs_stat.con_client[i].send_cmd_error_times = 0;
	}	
	
	ip_port[0].ip = (219<<24)|(239<<16)|(83<<8)|74;
	ip_port[1].ip = (219<<24)|(239<<16)|(83<<8)|74;
	ip_port[2].ip = (219<<24)|(239<<16)|(83<<8)|74;
	ip_port[0].port = 1521;
	ip_port[1].port = 40010;
	ip_port[2].port = 36004;
}

int32_t g4_status_manage()
{
	uint8_t ret;
	uint8_t i;
	
	char *start_str = NULL;
	if(gprs_stat.reboot_flag == GPRS_REBOOT_SETP0) //关闭模块
	{
		grps_power_off();
		init_4g_status_flag();
		gprs_stat.reboot_flag = GPRS_REBOOT_SETP1;
		start_str = "4g_set:power off\r\n";
		debug(start_str);				
		return 200;
	}
	else if(gprs_stat.reboot_flag == GPRS_REBOOT_SETP1) //打开模块
	{
		grps_power_on();
		gprs_stat.reboot_flag = GPRS_REBOOT_SETP2;
		start_str = "4g_set:power on\r\n";
		debug(start_str);				
		return 100;
	}
	else if(gprs_stat.reboot_flag == GPRS_REBOOT_SETP2) //打开模块
	{
		HAL_GPIO_WritePin(L506_PWR_KEY_GPIO_Port,L506_PWR_KEY_Pin,GPIO_PIN_SET);
		gprs_stat.reboot_flag = GPRS_REBOOT_SETP3;
		start_str = "4g_set:power_key on\r\n";
		debug(start_str);				
		return 200;
	}	

// 已开机
	if(gprs_stat.ati_ok == 0)
	{
		gprs_uart_send_string(AT_CMD_ATI);
		start_str = "4g_set:send ati\r\n";
		debug(start_str);	
		gprs_stat.send_at_cmd_times++;
		if(gprs_stat.send_at_cmd_times>30)	
		{
			gprs_stat.send_at_cmd_times = 0;
			gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;			//关GSM电源
		}
		return 100;
	}
	else if(gprs_stat.cgreg_ok == 0)
	{
		gprs_uart_send_string(AT_CMD_AT_CGREG);
		start_str = "4g_set:send at cgreg\r\n";
		debug(start_str);		
		if(gprs_stat.send_at_cmd_times>30)	
		{
			gprs_stat.send_at_cmd_times = 0;
			gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;			//关GSM电源		
		}
		return 100;
	}	
	else if(gprs_stat.cereg_ok == 0)
	{
		gprs_uart_send_string(AT_CMD_AT_CEREG);
		start_str = "4g_set:send at cereg\r\n";
		debug(start_str);	
		if(gprs_stat.send_at_cmd_times>30)	
		{
			gprs_stat.send_at_cmd_times = 0;
			gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;			//关GSM电源		
		}
		return 100;
	}		
	if(gprs_stat.set_timeout == 0)                            //设置超时 不存在失败 
	{
		gprs_stat.set_timeout = 1;			//关GSM电源
		gprs_uart_send_string(AT_CMD_4GAT_SET_TIMEOUT);  
		start_str = "4g_set:send at timeout\r\n";
		debug(start_str);			
		return 1;
	}	
	if(gprs_stat.netopen == 0)                            //打开网络
	{
		if(gprs_stat.send_at_cmd_times>5)	
		{
			gprs_stat.send_at_cmd_times = 0;
			gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;			//关GSM电源
		}
		gprs_uart_send_string(AT_CMD_4GAT_NETOPEN);  
		start_str = "4g_set:send at netopen\r\n";
		debug(start_str);			
		return 1000;
	}		
	
//	ret = gps_handle();
//	if(ret > 0)
//		return ret;	
	
	for(i=0;i<MAX_CLIENT_COUNT;i++)
	{
		ret = client_connect_handle(i);
		if(ret > 0)
			return ret;
	}
		
	return 100;
}


uint8_t gps_handle(void)
{
	static TickType_t time;
	char *start_str = NULL;
	
	if(gprs_stat.gps_need_open_flag == GPS_NEED_START)  //发送打开GPS的命令
	{
		gprs_stat.gps_need_open_flag = GPS_START_ING;
		gprs_uart_send_string(AT_CMD_4GAT_CGPS_ON);
		start_str = "4g_set:send at open gps\r\n";
		debug(start_str);				
		time = xTaskGetTickCount();
		return 2;
	}	
	else if(gprs_stat.gps_need_open_flag == GPS_START_ING) //发送配置5秒返回一次数据的命令
	{
		gprs_stat.gps_need_open_flag = GPS_WAIT_ING;
		gprs_uart_send_string(AT_CMD_4GAT_CGPSINFO_5); 
		start_str = "4g_set:send at report gps 5s\r\n";
		debug(start_str);		
		return 2;
	}
	else if(gprs_stat.gps_need_open_flag == GPS_WAIT_ING)  //等待 超时计数
	{
		if((xTaskGetTickCount() - time) > 100*300) //300秒没有成功 重启一下gps
		{
			gprs_stat.gps_need_open_flag = GPS_NEED_START;
			gprs_uart_send_string(AT_CMD_4GAT_CGPSINFO_0);
			osDelay(2);
			gprs_uart_send_string(AT_CMD_4GAT_CGPS_OFF);
			gprs_stat.gps_need_open_flag = GPS_NEED_START;
			start_str = "4g_set:timeout so send at close gps\r\n";
			debug(start_str);					
			return 100;
		}	
		return 0;
	}	
	else if(gprs_stat.gps_need_open_flag == GPS_OK)  //等待 超时计数
	{	
		if(xTaskGetTickCount() > gprs_stat.gps_restart_time)
		{
			gprs_stat.gps_need_open_flag = GPS_NEED_START;
			start_str = "4g_set:timeout so send at restart gps\r\n";
			debug(start_str);					
		}
	}
	return 0;
}

int32_t is_all_client_connect_fail_timeout()
{
	uint8_t i;
	for(i=0;i<MAX_CLIENT_COUNT;i++)
	{
		if(gprs_stat.con_client[i].connect_fail_times < ALL_CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD&&
			(ip_port[i].ip != 0 && ip_port[i].port != 0))  //有效连接得连接失败次数小于最大次数 则不成立
		{
			return -1;
		}
	}
	//满足重启模块条件
	sprintf(debug_str,"4g_set: 全部有效链接 连接失败次数大于%d 重启模块\r\n",ALL_CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD);
	debug(debug_str);		
	gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;
	return 0;
}

uint32_t client_connect_handle(uint8_t client)
{

	uint32_t ret = 0;
	
	if(client>=MAX_CLIENT_COUNT)
		return 0;
	
	if(ip_port[client].ip == 0 || ip_port[client].port == 0)
		return 0;
	
	if(gprs_stat.con_client[client].connect_ok == G4_CONNECT_NONE)  //未连接
	{
		gprs_stat.con_client[client].connect_ok = G4_CONNECT_ING;
		gprs_stat.con_client[client].send_at_cmd_timeout = xTaskGetTickCount();
		g4_at_tcp_conn(client,ip_port[client].ip,ip_port[client].port); 
		sprintf(debug_str,"4g_set: AT_Connect_%d ip=%d.%d.%d.%d port=%d\r\n",client,(ip_port[client].ip>>24)&0xff,(ip_port[client].ip>>16)&0xff,
				(ip_port[client].ip>>8)&0xff,(ip_port[client].ip&0xff),ip_port[client].port);
		debug(debug_str);			
		ret = 10;
	}
	else if(gprs_stat.con_client[client].connect_ok == G4_CONNECT_ING)   //正在连接
	{
		if((xTaskGetTickCount() - gprs_stat.con_client[client].send_at_cmd_timeout) > pdMS_TO_TICKS(1000*10)) //大于10秒还灭有连接成功 表示连接失败
		{
			gprs_stat.con_client[client].connect_ok = G4_CONNECT_FAIL;
			gprs_stat.con_client[client].connect_fail_times++;
			gprs_stat.con_client[client].send_at_cmd_timeout = 0;
			//10秒 每失败一次增加10秒
			gprs_stat.con_client[client].reconnect_time = xTaskGetTickCount() + pdMS_TO_TICKS(1000*10)+pdMS_TO_TICKS(1000*10)*gprs_stat.con_client[client].connect_fail_times;			
			
			sprintf(debug_str,"4g_set: AT_Connect_%d 超时无反应 失败次数=%d 下次重连ticks=%d\r\n",client,gprs_stat.con_client[client].connect_fail_times,gprs_stat.con_client[client].reconnect_time);
			debug(debug_str);			
		}
		
	}
	else if(gprs_stat.con_client[client].connect_ok == G4_CONNECT_FAIL)   //连接失败    1：收到模块返回的连接失败指令  2：连接中超时
	{
		if(is_all_client_connect_fail_timeout() == 0) //所有有效链接连接失败数超出限制
			return 1; //重启模块
		if(gprs_stat.con_client[client].connect_fail_times > CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD)
		{
			//满足重启模块条件
			sprintf(debug_str,"4g_set: client_%d 连接失败次数大于%d 重启模块\r\n",client,CLIENT_CONNECT_FAIL_TIMES_TO_RESTART_MOD);
			debug(debug_str);				
			gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;
			return 1;			
		}
		if(xTaskGetTickCount() > gprs_stat.con_client[client].reconnect_time) //大于重连时间  执行重连操作
		{
			gprs_stat.con_client[client].connect_ok = G4_CONNECT_NONE;
			gprs_stat.con_client[client].send_at_cmd_timeout = 0;		
			g4_at_tcp_close(client);	
			return 100;
		}
		
	}	
	else if(gprs_stat.con_client[client].connect_ok == G4_CONNECT_ERROR)   //连接失败    1：收到模块返回的连接失败指令  2：连接中超时
	{
		gprs_stat.con_client[client].connect_ok = G4_CONNECT_NONE;
		gprs_stat.con_client[client].send_at_cmd_timeout = 0;			
		g4_at_tcp_close(client);
		sprintf(debug_str,"4g_set: AT_Connect_%d 错误 关闭链接\r\n",client);
		debug(debug_str);
		return 100;
	}		
	else if(gprs_stat.con_client[client].connect_ok == G4_CONNECT_OK)  //已经连接成功
	{

	}
	return ret;
}





TickType_t xBlockTime = 10;
/***************************************************************************

4G模块串口数据处理线程：
1.阻塞在4G串口接收streambuf上 处理AT命令
2.利用超时机制维护4G模块状态，维护多个链接得connect状态
3.分发服务器下发得数据到各自链接得streambuf上



******************************************************************************/
void task_4g_uart_rev_handle(void *argument)
{
	uint8_t i;
	uint16_t str_data_index = 0;
	uint16_t read_max_len = 256;
	uint16_t rev_size;
	uint16_t write_size;
	int32_t glth_ret = 0;

	memset(get_str_data,0,256);
	init_4g_status_flag();
	gprs_stat.reboot_flag = GPRS_REBOOT_SETP1;
								 
  for(;;)
  {
		if(sbh_4g_str_rev == NULL)  //内存不够分配  延时1秒后继续申请
		{		
			sbh_4g_str_rev = xStreamBufferCreate(8196,1);
			if(sbh_4g_str_rev != NULL) 
				start_from_gprs_dma_receive();
			else
				osDelay( pdMS_TO_TICKS(100));
		}
		else
		{
			rev_size = xStreamBufferReceive(sbh_4g_str_rev,&get_str_data[str_data_index],read_max_len,xBlockTime);
			if(rev_size > 0)
			{
				rev_size += str_data_index;
				if(gprs_stat.now_who_rx < 0) //非服务器数据
				{
					for(;;)
					{
						glth_ret = get_4g_line_to_handle(get_str_data,rev_size);
						if(glth_ret<rev_size && glth_ret!=0)//有未处理完得数据
						{
							rev_size = glth_ret;
							if(gprs_stat.now_who_rx >= 0)//表示之后得数据属于服务器下发得数据
							{
								if(rev_size>gprs_stat.now_rx_len) //数据全部在该数组中
								{
									write_size = xStreamBufferSend(gprs_stat.con_client[gprs_stat.now_who_rx].sbh_4g_data_rev,get_str_data,gprs_stat.now_rx_len,10);	
									memcpy(get_str_data,&get_str_data[gprs_stat.now_rx_len],rev_size-gprs_stat.now_rx_len);
									sprintf(debug_str,"client_%d: write0 data len=%d %d\r\n",gprs_stat.now_who_rx,gprs_stat.now_rx_len,write_size);
									rev_size = rev_size-gprs_stat.now_rx_len;
									debug(debug_str);										
									gprs_stat.now_rx_len = 0;
									gprs_stat.now_who_rx = -1;								
									continue;
								}
								else  //一部分数据在数组中
								{
									write_size = xStreamBufferSend(gprs_stat.con_client[gprs_stat.now_who_rx].sbh_4g_data_rev,get_str_data,rev_size,10);	
									gprs_stat.now_rx_len = gprs_stat.now_rx_len - rev_size;						
									rev_size = 0;
									glth_ret = 0;
									sprintf(debug_str,"client_%d: write1 data len=%d %d\r\n",gprs_stat.now_who_rx,rev_size,write_size);
									debug(debug_str);											
									break;
								}
							}
						}
						else
							break;					
					}
					//返回数据长度小于接收长度，表示有未处理的数据，
					str_data_index = glth_ret;      //返回长度等于接收长度表示数据有遗留 而且不是一条完整得命令
					read_max_len = 256-str_data_index;
					rev_size = 0;
					continue;
				}
				
				if(gprs_stat.now_who_rx >= 0)//表示之后得数据属于服务器下发得数据
				{
					if(gprs_stat.now_rx_len <= 0 || gprs_stat.now_who_rx>=MAX_CLIENT_COUNT) //异常
					{
						gprs_stat.now_rx_len = 0;
						gprs_stat.now_who_rx = -1;
					}
					else
					{
						write_size = xStreamBufferSend(gprs_stat.con_client[gprs_stat.now_who_rx].sbh_4g_data_rev,get_str_data,rev_size,10);
						sprintf(debug_str,"client_%d: write2 data len=%d %d\r\n",gprs_stat.now_who_rx,rev_size,write_size);
						debug(debug_str);	
						gprs_stat.now_rx_len -= rev_size;	

						xBlockTime = 100;	
						read_max_len = gprs_stat.now_rx_len;
						if(read_max_len>256)
							read_max_len = 256;
						if(gprs_stat.now_rx_len<=0)
						{
							gprs_stat.now_rx_len = 0;
							gprs_stat.now_who_rx = -1;	
							read_max_len = 256;							
						}
						
						str_data_index = 0;
					}
				}
			}
			else
			 {
				if(gprs_stat.now_who_rx >= 0 && gprs_stat.now_rx_len>0)//表示之后得数据属于服务器下发得数据
				{		
					gprs_stat.now_rx_len = 0;
					gprs_stat.now_who_rx = -1;	
					read_max_len = 256;						
				}
				 xBlockTime = g4_status_manage();
			 }
			if(str_data_index>=256 ||read_max_len <= 0)
			{
				str_data_index = 0;
				read_max_len = 256;
				memset(get_str_data,0,256);
			}			 
		}
  }
  /* USER CODE END StartDefaultTask */
}


//启动4g模块流程 把buf数据发送出去
int32_t start_send_client_buf(uint8_t client)
{
	uint8_t times = 0;
	if(gprs_stat.con_client[client].send_len == 0 )
	{
		xSemaphoreTake( gprs_stat.con_client[client].sem_binary_buf_no_data,0); 
		xSemaphoreGive( gprs_stat.con_client[client].sem_binary_buf_no_data); //发送无数据得信号量
		return 0;
	}
	
	for(times=0;times<3;times++) //尝试3次 发送不成功则把链接关闭
	{
		sprintf(at_cmd_conn,AT_CMD_4GAT_CIPSEND,client,gprs_stat.con_client[client].send_len);
		gprs_uart_send_string(at_cmd_conn);
		debug(at_cmd_conn);
		if(pdTRUE == xSemaphoreTake(gprs_stat.sem_binary_buf_send_steps,3000 / portTICK_PERIOD_MS)) //超时3秒  等待串口返回 >
		{
			//已经收到>
			g4_uart_send_data(gprs_stat.con_client[client].buf,gprs_stat.con_client[client].send_len);
			if(pdTRUE == xSemaphoreTake(gprs_stat.sem_binary_buf_send_steps,5000 / portTICK_PERIOD_MS)) //超时5秒  等待串口返回 send ok
			{
				//已经收到send ok
				xSemaphoreGive( gprs_stat.con_client[client].sem_binary_buf_no_data); //发送无数据得信号量
				gprs_stat.con_client[client].send_len = 0;
				return 0;
			}
			else
			{
				sprintf(debug_str,"client_error_%d: no send ok times=%d\r\n",client,times);
				debug(debug_str);	
				
			}
		 }
		 else
		 {
			 
				sprintf(debug_str,"client_error_%d: no > times=%d\r\n",client,times);
				debug(debug_str);	
			 //未已经收到>  强制发送数据
				g4_uart_send_data(gprs_stat.con_client[client].buf,gprs_stat.con_client[client].send_len);
				if(pdTRUE == xSemaphoreTake(gprs_stat.sem_binary_buf_send_steps,5000 / portTICK_PERIOD_MS)) //超时5秒  等待串口返回 send ok
				{
					//已经收到send ok
					xSemaphoreGive( gprs_stat.con_client[client].sem_binary_buf_no_data); //发送无数据得信号量
					gprs_stat.con_client[client].send_len = 0;
					return 0;
				}
				else
				{
					sprintf(debug_str,"client_error_%d: no send ok times=%d\r\n",client,times);
					debug(debug_str);	
					
				}			 
		 }
		 
	}
	//尝试3次  失败  关闭链接
	gprs_stat.con_client[client].connect_ok = G4_CONNECT_ERROR;
	gprs_stat.reboot_flag = GPRS_REBOOT_SETP0;
	return -1;
}


//线程
// 监控所有链接中的发送buf  有数据后执行发送流程
void task_t4G_data_send(void *argument)
{
	uint8_t i;
	
	QueueSetHandle_t queue_set_client_buf_has_data;
	QueueSetMemberHandle_t xActivatedMember;
	
	
	queue_set_client_buf_has_data = xQueueCreateSet(MAX_CLIENT_COUNT);	
	gprs_stat.sem_binary_buf_send_steps =  xSemaphoreCreateBinary();
	for(i=0;i<MAX_CLIENT_COUNT;i++)
	{
		//if(ip_port[i].ip>0)
		{
			gprs_stat.con_client[i].sbh_4g_data_rev = xStreamBufferCreate(8196,2); //4g接收服务器数据缓冲
			gprs_stat.con_client[i].sem_binary_buf_has_data = xSemaphoreCreateBinary();
			gprs_stat.con_client[i].sem_binary_buf_no_data = xSemaphoreCreateBinary();
			xQueueAddToSet(gprs_stat.con_client[i].sem_binary_buf_has_data,queue_set_client_buf_has_data);
		}
	}


	for(;;)
	{
		xActivatedMember = xQueueSelectFromSet( queue_set_client_buf_has_data,
                                                10000 / portTICK_PERIOD_MS ); //阻塞等待所有链接得buf有效
		
		if(xActivatedMember == NULL) //超时无数据 做一些异常检查
		{
			
			continue;
		}
		
		for(i=0;i<MAX_CLIENT_COUNT;i++)
		{
			if( xActivatedMember == gprs_stat.con_client[i].sem_binary_buf_has_data ) //数据有效
			{
				xSemaphoreTake( xActivatedMember, 0); //取走信号量
				if(gprs_stat.con_client[i].send_len > _4G_BUFF_LEN) //长度异常  清空长度  发送无数据得信号量
				{
					xSemaphoreGive( gprs_stat.con_client[i].sem_binary_buf_no_data);
					gprs_stat.con_client[i].send_len = 0;
					break;
				}
				if(gprs_stat.con_client[i].connect_ok != G4_CONNECT_OK) //未连接ok  不处理 等待连接ok得时候处理
				{
					break;
				}					
				start_send_client_buf(i);
				break;
			}
		}
	}

}




void task_client_0(void *argument)
{
	uint16_t rev_size;
	
	uint8_t g4_data[1500];
	
	for(;;)
	{
		if(gprs_stat.con_client[0].sbh_4g_data_rev == NULL)
		{
			osDelay(100);
			continue;
		}
		rev_size = xStreamBufferReceive(gprs_stat.con_client[0].sbh_4g_data_rev,g4_data,1500,1000);
		if(rev_size>0)
		{
			sprintf(debug_str,"client_0: get data len=%d\r\n",rev_size);
			debug(debug_str);
			
			if(pdTRUE == xSemaphoreTake(gprs_stat.con_client[0].sem_binary_buf_no_data,100))
			{
				memcpy(gprs_stat.con_client[0].buf,g4_data,rev_size);
				xSemaphoreGive( gprs_stat.con_client[0].sem_binary_buf_has_data);
				gprs_stat.con_client[0].send_len = rev_size;
			}
			else
			{
				if(gprs_stat.con_client[0].send_len == 0)
					xSemaphoreGive(gprs_stat.con_client[0].sem_binary_buf_no_data);
			}
		}
	}
}















