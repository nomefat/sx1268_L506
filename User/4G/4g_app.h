#ifndef G4_APP_H_
#define G4_APP_H_



#define MAX_CLIENT_COUNT 3  //最大链接数量

typedef struct _ip_port
{
	uint32_t ip;
	uint16_t port;
}struct_ip_port;




typedef struct _struct_gprs_stat{
	signed char ati_ok;
	signed char creg_ok;       
	signed char cgreg_ok;  	
	signed char cereg_ok;          //4G网络注册标志位
	signed char set_timeout;         //设置超时时间标志位
	signed char netopen;             //打开网络标志位
	signed char send_at_cmd_times;
	signed char now_who_rx;         //最后一条数据是哪个链接收到的
	unsigned short now_rx_len;      //最后一条数据的长度
	signed char now_read_who_txrx;    //现在在读取哪个链接的收发数据长度
	volatile signed char now_who_working;  //指示现在那个链接正在发送数据
	volatile signed char send_steps;       //指示发送数据的状态   
	#define GPRS_SEND_WAIT_S 1           //已经发送 cipsend命令，等待>
	#define GPRS_SEND_WAIT_SENDOK 2      //已经发送数据  等待模块返回send ok  +cipsend:success,1,20,4
	#define GPRS_SEND_OK            3     //发送完成
	SemaphoreHandle_t  sem_binary_buf_send_steps; //用于发送数据得过程	
	signed char gps_need_open_flag;
	#define GPS_NEED_START    0
	#define GPS_START_ING     1
	#define GPS_WAIT_ING      2
	#define GPS_OK         	  3
	unsigned int gps_restart_time;
 	char csq;              //信号质量
	char reboot_flag;
#define GPRS_REBOOT_SETP0 0               //执行关闭GSM模块电源指令
#define GPRS_REBOOT_SETP1 1               //执行打开GSM模块电源指令
#define GPRS_REBOOT_SETP2 2               //GSM已经上电 不执行操作	
#define GPRS_REBOOT_SETP3 3               //GSM已经上电 不执行操作	
	char revision[32];
	struct {
		char connect_ok;
		#define G4_CONNECT_NONE  0
		#define G4_CONNECT_ING   1
		#define G4_CONNECT_FAIL  2
		#define G4_CONNECT_ERROR 3
		#define G4_CONNECT_OK    4
		unsigned char connect_fail_times;	//连接失败的次数（包含直接返回连接失败和无反应超时）,多链接可做参考 如果全部连接失败超过一定次数 重启模块 
																				//如果单单该链接连接失败 可以考虑更多次数后重启模块
		unsigned int reconnect_time;  //重连时间
		unsigned int send_at_cmd_timeout; //发送AT指令的超时  (包括 connect指令  启动发送指令等待 >   发送完成指令等待send ok)
		unsigned char send_cmd_error_times;    //发送AT指令失败得次数,超时后关闭链接
		unsigned char reconnect_send_cmd_error_times;    //重新连接后依然发送失败得次数  超时后重启模块
		unsigned int tx_len;              //发送了多少字节
		unsigned int rx_len;  		        //接收了多少字节
		
		unsigned char last_data_send_time;
		#define _4G_BUFF_LEN 1400
		unsigned short send_len;
		SemaphoreHandle_t  sem_binary_buf_no_data;  //通知上层应用buf没有数据 可以写数据
		SemaphoreHandle_t  sem_binary_buf_has_data; //接收上层应用通知buf有数据 可以发送
		StreamBufferHandle_t sbh_4g_data_rev ;
		signed char buf[_4G_BUFF_LEN];
	}con_client[MAX_CLIENT_COUNT];

}struct_gprs_stat;




int32_t start_send_client_buf(uint8_t client);







#endif

