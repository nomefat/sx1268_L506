#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "string.h"
#include "semphr.h"
#include "stdio.h"
#include "4g_at_callback_func.h"
#include "stdio.h"
#include "4g_app.h"



extern TickType_t xBlockTime;
extern char debug_str[1024];
extern struct_gprs_stat gprs_stat;
extern StreamBufferHandle_t sbh_debug_send;


extern void debug(const char* pstr);



void callback_fun_at_ok(const char *pstr)
{
	char *start_str;
	if(gprs_stat.ati_ok == 0)
	{
		gprs_stat.ati_ok = 1;
		gprs_stat.send_at_cmd_times = 0;
		start_str = "4g_get:ati ok\r\n";
	}
	else
		start_str = "4g_get:ok\r\n";
	debug(start_str);		
}


void callback_fun_cgreg(const char *pstr)
{
	char *start_str = "4g_get:cgreg ok\r\n";
	gprs_stat.cgreg_ok = 1;
	gprs_stat.send_at_cmd_times = 0;
	debug(start_str);		
}


void callback_fun_csq(const char *pstr)
{

}

void callback_fun_ready_send(const char *pstr)
{
	xSemaphoreGive(gprs_stat.sem_binary_buf_send_steps);
}

void callback_fun_creg(const char *pstr)
{
	char *start_str = "4g_get:creg ok\r\n";
	gprs_stat.creg_ok = 1;
	gprs_stat.send_at_cmd_times = 0;
	debug(start_str);				
}
void callback_fun_revision(const char *pstr)
{
	strcpy(gprs_stat.revision,pstr);
	sprintf(debug_str,"4g_get:%s\r\n",pstr);
	
	debug(debug_str);		
}

void callback_fun_connect_ok(uint8_t client)
{
	gprs_stat.con_client[client].connect_ok = G4_CONNECT_OK;
	gprs_stat.con_client[client].connect_fail_times = 0;
	sprintf(debug_str,"4g_get: AT_Connect_%d 连接成功\r\n",client);
	debug(debug_str);
	if(gprs_stat.con_client[0].send_len>0)
		xSemaphoreGive( gprs_stat.con_client[0].sem_binary_buf_has_data);
	else
		xSemaphoreGive( gprs_stat.con_client[0].sem_binary_buf_no_data);

}

void callback_fun_connect_fail(uint8_t client)
{
	gprs_stat.con_client[client].connect_ok = G4_CONNECT_FAIL;
	gprs_stat.con_client[client].connect_fail_times++;
	gprs_stat.con_client[client].send_at_cmd_timeout = 0;
	gprs_stat.con_client[client].reconnect_time = xTaskGetTickCount() + pdMS_TO_TICKS(1000*10)+pdMS_TO_TICKS(1000*10)*gprs_stat.con_client[client].connect_fail_times;
	sprintf(debug_str,"4g_get: AT_Connect_%d 连接失败次数=%d 下次重连ticks=%d\r\n",client,gprs_stat.con_client[client].connect_fail_times,gprs_stat.con_client[0].reconnect_time);
	debug(debug_str);			
}

void callback_fun_client_closed_0(const char *pstr)
{

}
void callback_fun_client_closed_1(const char *pstr)
{

}
void callback_fun_client_closed_2(const char *pstr)
{

}
void callback_fun_call_ready(const char *pstr)
{

}
void callback_fun_qistate(const char *pstr)
{

}
void callback_fun_qisack(const char *pstr)
{

}
void callback_fun_receive(const char *pstr)
{

}
void callback_fun_cereg(const char *pstr)
{
	char *start_str = "4g_get:cereg ok\r\n";
	gprs_stat.cereg_ok = 1;
	gprs_stat.send_at_cmd_times = 0;
	debug(start_str	);
}
//+CIPOPEN:SUCCESS,1
void callback_fun_cipopen(const char *pstr)
{
	if(strcmp(pstr,"SUCCESS,0") == 0)
	{
		callback_fun_connect_ok(0);
		
	}
	else if(strcmp(pstr,"SUCCESS,1") == 0)
	{
		callback_fun_connect_ok(1);
		
	}
	else if(strcmp(pstr,"SUCCESS,2") == 0)
	{
		callback_fun_connect_ok(2);
		
	}	
	if(strcmp(pstr,"FAIL,0") == 0)
	{
		callback_fun_connect_fail(0);
	}
	else if(strcmp(pstr,"FAIL,1") == 0)
	{
		callback_fun_connect_fail(1);

	}
	else if(strcmp(pstr,"FAIL,2") == 0)
	{
		callback_fun_connect_fail(2);

	}		
}      
//:SUCCESS
void callback_fun_netopen(const char *pstr)
{
	if(strcmp(pstr,"SUCCESS") == 0)
	{	
		char *start_str = "4g_get:netopen ok\r\n";
		gprs_stat.netopen = 1;
		xBlockTime = 10;
		gprs_stat.send_at_cmd_times = 0;
		debug(start_str);	
	}
	else
	{
		xBlockTime = 100;
		char *start_str = "4g_get:netopen fail\r\n";	
		debug(start_str);			
	}
}  

//:SUCCESS,1,20,4
void callback_fun_cipsend(const char *pstr)
{
	uint8_t client;
	uint16_t send_set;
	uint16_t send_ok;
	
	char str_buf[16];
	char str_buf_int[16];
	memset(str_buf,0,16);
	memset(str_buf_int,0,16);
	
	sscanf(pstr,"%[^,],%s",str_buf,str_buf_int);
	sscanf(str_buf_int,"%d,%d,%d",&client,&send_set,&send_ok);

	
	if(strcmp(str_buf,"SUCCESS") == 0)
	{
		sprintf(debug_str,"4g_get: client_%d send ok=%d %d\r\n",client,send_set,send_ok);
		debug(debug_str);			
		xSemaphoreGive(gprs_stat.sem_binary_buf_send_steps);
	}
} 


void callback_fun_network_disconnected(const char *pstr)
{
	uint8_t i;
	char *start_str = "4g_get:netopen disconnected\r\n";
	
	for(i=0;i<MAX_CLIENT_COUNT;i++)
	{
		gprs_stat.con_client[i].connect_ok = 0;	
	}

	gprs_stat.netopen = 0;
	debug(start_str);		
}  


//:0
void callback_fun_error(const char *pstr)
{

}



void callback_fun_server_disconnected(const char *pstr)
{
	uint8_t client;
	
	sscanf(pstr,"%d",&client);
	
	if(client<MAX_CLIENT_COUNT)
	{
		gprs_stat.con_client[client].connect_ok = 0;	
	}
	sprintf(debug_str,"4g_get: Server close client=%d\r\n",client);
	debug(debug_str);				
} 


//:0CMD_CALLBACK(
void callback_fun_ciprxget(const char *pstr)
{
	char str_buf[16];
	char str_buf_int[16];
	uint8_t client;
	uint8_t nouse;
	uint16_t len;
	
	memset(str_buf,0,16);
	memset(str_buf_int,0,16);
	
	sscanf(pstr,"%[^,],%s",str_buf,str_buf_int);
	sscanf(str_buf_int,"%d,%d,%d,",&nouse,&client,&len);
	if(client<MAX_CLIENT_COUNT && len<2048)
	{
	  gprs_stat.now_who_rx = client;
		gprs_stat.now_rx_len = len;	
	}
	sprintf(debug_str,"4g_get: client_%d rev data len=%d\r\n",client,len);
	debug(debug_str);		
}          
//:SUCCESS,0,0,19,callback_fun_help(const char *pstr)


void callback_fun_cipstat(const char *pstr)
{

}
void callback_fun_cgpsinfo(const char *pstr)
{
	debug("gps_get:");
	debug(pstr);
	debug("\r\n");
	//gprs_stat.gps_need_open_flag = GPS_OK;
}



