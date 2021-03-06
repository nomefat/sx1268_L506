#ifndef RF_H_
#define RF_H_






#define RF1 0
#define RF2 1


#define BIT_RF1 (1)
#define BIT_RF2 (1<<1)


#define JUMP_CH_COUNT    					30        //频道总数
//#define DEV_ID         						20201001
#define FRAME_SLOT_COUNT        			64   				//64个slot表示一帧
#define RF_NO_RX_REBOOT_TICKS   			(100*120)			//120秒

#define RF_SEND_ERROR_REBOOT_COUNT   		10

#define SENSOR_MAX_COUNT 					128

#define PACKET_SEQ_MAX                      120

#define SENSOR_SLOT_BEGIN                   6        //SENSOR 时间槽起始
#define SENSOR_SLOT_END                    59        //SENSOR 时间槽结束



// rf状态记录表
typedef struct _rf_status_manage
{
	int8_t rf_init_ok;
	uint32_t reboot_count;
	int8_t rf_work_status;         //rf 所处的tx rx sb 等操作模式
	uint32_t rf_send_count;        //发送的包数
	uint32_t rf_send_ok_count;	   //发送ok的包数
	volatile uint32_t rf_last_rx_tick;  //最后一条收到数据的tick点
	int8_t rssi;
	int32_t rssi_add;
	int32_t rssi_count;
}struct_rf_status_manage;


#pragma anon_unions
#pragma pack(1)

/*****************AP-sensor 通信协议****************************/


typedef struct _rf_head
{
	uint16_t crc;
	uint16_t dev_id;
	struct 
	{
		uint8_t packet_type:4;
		#define RF1_AP_SYN          1
		#define RF1_AP_UPDATA       2
		#define RF1_AP_ACK          3

		#define RF2_AP_SYN          4
		#define RF2_AP_UPDATA       5
		#define RF2_AP_ACK          6

		#define RF_S_EVENT         10
		#define RF_S_STAT          11
		#define RF_S_UPDATA_ACK    12

		uint8_t cmd:4;
		#define CMD_NONE           0
		#define CMD_SET_SLOT       1
		#define CMD_ADJUST         2
		#define CMD_SET_UPDATA_MODE  3
		#define CMD_CLEAR_BAND_ID   4
	};
	uint8_t packet_seq;
}struct_rf_head;



typedef struct _rf_syn
{
	struct_rf_head head;
	uint8_t jump_ch_group;
}struct_rf_syn;


typedef struct _rf_updata
{
	struct_rf_head head;
	uint16_t addr;
	uint8_t data[32];
}struct_rf_updata;



typedef struct _rf_ack
{
	struct_rf_head head;
	uint8_t jump_ch_group;
	uint16_t sensor_id;
	uint8_t slot;
	uint8_t ack_bit[7];
}struct_rf_ack;


typedef union _SNP_EVENT_t
{
	uint16_t		uiAll;
	struct
	{
		uint16_t	bmMs	:	10,		// ???, 'SNP_REF_TIME_US'?32?: 0.9765625ms.
				bmSec	:	5,		// ??.
				blIsOn	:	1;		// ??'ON'/'OFF', 'ON' = 1.
		#define E_ON 1
		#define E_OFF 0
	};
} SNP_EVENT_t;

typedef struct _rf_event
{
	struct_rf_head head;
	uint8_t slot;
	uint8_t resend_count;
	int8_t background_rssi;
	SNP_EVENT_t event[1]; //可变长度 10 12 14 
}struct_rf_event;


typedef struct _rf_stat
{
	struct_rf_head head;
	uint8_t slot;
	uint16_t band_id;
	uint8_t battery;
	int8_t rx_rssi;
	union{
		uint16_t addr;
		struct{
		uint8_t h_version;
		uint8_t s_version;
		};
	};	
}struct_rf_stat;




typedef struct _sensor_list
{
	uint16_t sensor_id;
	struct 
	{
		uint16_t on_delay;          //ON事件 延迟一段时间后输出，
		uint16_t off_to_on_min_time;  //分车阈值  表示OFF到ON的最小时间  如果小于这个时间 表示OFF是无效的
		uint8_t lane;      //车道号    从1开始
		uint8_t lane_index;  //车道里的序号   从0开始
		uint16_t distance; //距离前一个地磁的距离   毫米
		uint8_t direction; //东南西北 方向
		uint8_t lane_direction; //直行  左转 右转  掉头
	}sensor_cfg; //参数配置
	uint8_t slot;
	struct 
	{	
		struct 
		{
			uint8_t size;
			uint32_t rev_slot_count;
			uint8_t event_packet[16];  //存储两条事件包，每条最大4个事件
		}sensor_event[2];
		struct_rf_stat rf_stat_packet;		
	}sensor_data;	//数据包记录
	struct 
	{
		SNP_EVENT_t event[2]; //上一个事件（已经处理过）  和 未处理的事件（）
		uint32_t rev_slot_count[2];  //收到的时间
		uint8_t now_sensor_onoff;
		int32_t onoff_time_ms;
		#define S_ON 1
		#define S_OFF 2
		uint8_t now_event_need_handle; //是否有事件需要处理
		/* data */
	}event_calc;   //过车数据计算结果以及过程数据
	
	struct 
	{
		int8_t rssi[2];
		int32_t rssi_add[2];
		int8_t snr[2];
		int32_t snr_add[2];		
		int32_t rssi_count[2];		/* data */
		int8_t rssi_avg[2];
		int8_t snr_avg[2];
		uint8_t last_resend_times; //记录一下当前sensor上报的重传次数
		uint16_t event_count; //实际收到的事件个数
		uint16_t lost_event_count; //通过包序号计算出丢了的事件个数
		uint16_t resend_count[32];    //0表示重传总数
	}sensor_stat;  //sensor 状态统计
	
	struct
	{
		uint8_t battery;
		int8_t rx_rssi;
		union{
			uint16_t addr;
			struct{
			uint8_t h_version;
			uint8_t s_version;
			};
		};			
	}sensor_cfg_rev;
	
}struct_sensor_list;




#pragma pack()

#endif

