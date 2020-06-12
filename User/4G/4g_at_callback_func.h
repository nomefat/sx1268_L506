#ifndef _4G_AT_CALLBACK_FUNC_H_
#define _4G_AT_CALLBACK_FUNC_H_




#define AT_CMD_ATI             "ATI\r\n"          //ATָ�� �ж�GSMģ���Ƿ�����
#define AT_CMD_AT_CSQ         "AT+CSQ\r\n"      //�ź�ǿ��
#define AT_CMD_AT_CREG  			"AT+CREG?\r\n"    //GSM�����Ƿ�ע��    +CREG: 0,1  // <stat>=1,GSM�����Ѿ�ע����
#define AT_CMD_AT_CGREG       "AT+CGREG?\r\n"   //GPRS�����Ƿ�ע��   +CGREG: 0,1    // <stat>=1,GPRS�����Ѿ�ע����
#define AT_CMD_AT_CEREG       "AT+CEREG?\r\n" 
#define AT_CMD_AT_CLOSE_CONNECT "AT+CIPCLOSE=%d\r\n"    //�ر�ָ����tcp����
#define AT_CMD_AT_SEND         "AT+QISEND=%d,%d\r\n"       //��������  ���ӱ�� ����
#define AT_CMD_4GAT_NETOPEN      "AT+NETOPEN\r\n"         //������
#define AT_CMD_4GAT_NETCLOSE     "AT+NETCLOSE\r\n"         //�ر�����
#define AT_CMD_4GAT_SET_TIMEOUT   "AT+CIPTIMEOUT=10000,10000,10000,10000\r\n"    //(netopen cipopen cipsend dns_cipopen)  timeout
#define AT_CMD_4GAT_CIPOPEN        "AT+CIPOPEN=%d,\"TCP\",\"%d.%d.%d.%d\",%d,%d\r\n"    //���ӷ�����
#define AT_CMD_4GAT_CIPSEND      "AT+CIPSEND=%d,%d\r\n"       //��������  ���ӱ�� ����   4G
#define AT_CMD_4GAT_CIPSTAT       "AT+CIPSTAT=%d\r\n"
#define AT_CMD_4GAT_CGPS_ON       "AT+CGPS=1,2\r\n"
#define AT_CMD_4GAT_CGPS_OFF       "AT+CGPS=0\r\n"
#define AT_CMD_4GAT_CGPSINFO_5       "AT+CGPSINFO=5\r\n"
#define AT_CMD_4GAT_CGPSINFO_0       "AT+CGPSINFO=0\r\n"

void callback_fun_at_ok(const char *pstr);
void callback_fun_cgreg(const char *pstr);
void callback_fun_csq(const char *pstr);
void callback_fun_ready_send(const char *pstr);
void callback_fun_creg(const char *pstr);
void callback_fun_revision(const char *pstr);
void callback_fun_call_ready(const char *pstr);
void callback_fun_qistate(const char *pstr);
void callback_fun_qisack(const char *pstr);
void callback_fun_receive(const char *pstr);
void callback_fun_cereg(const char *pstr);
void callback_fun_cipopen(const char *pstr);      //+CIPOPEN:SUCCESS,1
void callback_fun_netopen(const char *pstr);      //:SUCCESS
void callback_fun_cipsend(const char *pstr);    //:SUCCESS,1,20,4
void callback_fun_network_disconnected(const char *pstr);  //:0
void callback_fun_error(const char *pstr);
void callback_fun_server_disconnected(const char *pstr); //:0CMD_CALLBACK(
void callback_fun_ciprxget(const char *pstr);          //:SUCCESS,0,0,19,callback_fun_help(const char *pstr);
void callback_fun_cipstat(const char *pstr);
void callback_fun_cgpsinfo(const char *pstr);











#endif

