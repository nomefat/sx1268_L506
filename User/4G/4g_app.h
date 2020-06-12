#ifndef G4_APP_H_
#define G4_APP_H_



#define MAX_CLIENT_COUNT 3  //�����������

typedef struct _ip_port
{
	uint32_t ip;
	uint16_t port;
}struct_ip_port;




typedef struct _struct_gprs_stat{
	signed char ati_ok;
	signed char creg_ok;       
	signed char cgreg_ok;  	
	signed char cereg_ok;          //4G����ע���־λ
	signed char set_timeout;         //���ó�ʱʱ���־λ
	signed char netopen;             //�������־λ
	signed char send_at_cmd_times;
	signed char now_who_rx;         //���һ���������ĸ������յ���
	unsigned short now_rx_len;      //���һ�����ݵĳ���
	signed char now_read_who_txrx;    //�����ڶ�ȡ�ĸ����ӵ��շ����ݳ���
	volatile signed char now_who_working;  //ָʾ�����Ǹ��������ڷ�������
	volatile signed char send_steps;       //ָʾ�������ݵ�״̬   
	#define GPRS_SEND_WAIT_S 1           //�Ѿ����� cipsend����ȴ�>
	#define GPRS_SEND_WAIT_SENDOK 2      //�Ѿ���������  �ȴ�ģ�鷵��send ok  +cipsend:success,1,20,4
	#define GPRS_SEND_OK            3     //�������
	SemaphoreHandle_t  sem_binary_buf_send_steps; //���ڷ������ݵù���	
	signed char gps_need_open_flag;
	#define GPS_NEED_START    0
	#define GPS_START_ING     1
	#define GPS_WAIT_ING      2
	#define GPS_OK         	  3
	unsigned int gps_restart_time;
 	char csq;              //�ź�����
	char reboot_flag;
#define GPRS_REBOOT_SETP0 0               //ִ�йر�GSMģ���Դָ��
#define GPRS_REBOOT_SETP1 1               //ִ�д�GSMģ���Դָ��
#define GPRS_REBOOT_SETP2 2               //GSM�Ѿ��ϵ� ��ִ�в���	
#define GPRS_REBOOT_SETP3 3               //GSM�Ѿ��ϵ� ��ִ�в���	
	char revision[32];
	struct {
		char connect_ok;
		#define G4_CONNECT_NONE  0
		#define G4_CONNECT_ING   1
		#define G4_CONNECT_FAIL  2
		#define G4_CONNECT_ERROR 3
		#define G4_CONNECT_OK    4
		unsigned char connect_fail_times;	//����ʧ�ܵĴ���������ֱ�ӷ�������ʧ�ܺ��޷�Ӧ��ʱ��,�����ӿ����ο� ���ȫ������ʧ�ܳ���һ������ ����ģ�� 
																				//�����������������ʧ�� ���Կ��Ǹ������������ģ��
		unsigned int reconnect_time;  //����ʱ��
		unsigned int send_at_cmd_timeout; //����ATָ��ĳ�ʱ  (���� connectָ��  ��������ָ��ȴ� >   �������ָ��ȴ�send ok)
		unsigned char send_cmd_error_times;    //����ATָ��ʧ�ܵô���,��ʱ��ر�����
		unsigned char reconnect_send_cmd_error_times;    //�������Ӻ���Ȼ����ʧ�ܵô���  ��ʱ������ģ��
		unsigned int tx_len;              //�����˶����ֽ�
		unsigned int rx_len;  		        //�����˶����ֽ�
		
		unsigned char last_data_send_time;
		#define _4G_BUFF_LEN 1400
		unsigned short send_len;
		SemaphoreHandle_t  sem_binary_buf_no_data;  //֪ͨ�ϲ�Ӧ��bufû������ ����д����
		SemaphoreHandle_t  sem_binary_buf_has_data; //�����ϲ�Ӧ��֪ͨbuf������ ���Է���
		StreamBufferHandle_t sbh_4g_data_rev ;
		signed char buf[_4G_BUFF_LEN];
	}con_client[MAX_CLIENT_COUNT];

}struct_gprs_stat;




int32_t start_send_client_buf(uint8_t client);







#endif

