#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "ty_sdk_common.h"
#include "ty_dp_define.h"

typedef enum {
	cmd_key_short, // �����̰�
	cmd_key_long,  // ��������
	cmd_bat_info,  // �����Ϣ
	cmd_pir_trigger, // pir��⵽��
	cmd_pir_detect,  // pir�����
}devctrl_cmd_t;

static int devctrl_client_fd = -1;
static int bat_percent = 80, bat_mode = 1;
static int pir_status = 0;

STATIC INT_T s_mqtt_status = 0;
typedef VOID (*ON_IPC_STATUS_CHANGE)(VOID *arg);
typedef struct {
	TUYA_IPC_STATUS_E stat;
	ON_IPC_STATUS_CHANGE cb;
}IPC_STATUS_CHANGE_CB_MAP_T;


STATIC VOID __on_register_fail(VOID *arg)
{
	PR_DEBUG("%s, get registe fail status.", __func__);
	ledkey_set_mode(led_offline);
	return;
}

STATIC VOID __on_netcfg_start(VOID *arg)
{

	PR_DEBUG("%s, netcfg start. You can start your own netcfg task now.", __func__);
	ledkey_set_mode(led_wifi_ap);
	return;
}

STATIC VOID __on_netcfg_stop(VOID *arg)
{

	PR_DEBUG("%s, netcfg stop. You can stop your own netcfg task now.", __func__);
	return;
}

STATIC VOID __on_wifi_connect(VOID *arg)
{
	ledkey_set_mode(led_connect);
	return;
}

STATIC VOID __on_status_offline(VOID *arg)
{

	PR_INFO("offline: network status MQTT disconnected"); //according to requirements of shadow device, do not modify!!!
	ledkey_set_mode(led_offline);
	return;
}

STATIC VOID __on_status_fw_upgrade_start(VOID *arg)
{

	PR_INFO("fw upgrade: recv start cmd"); 
	return;
}

STATIC VOID __on_status_online(VOID *arg)
{

	PR_INFO("online: network status MQTT connected"); //according to requirements of shadow device, do not modify!!!
	s_mqtt_status = 1;
	ledkey_set_mode(led_online);
	return;
}


static IPC_STATUS_CHANGE_CB_MAP_T status_change_cb_map[] = {
	{TUYA_IPC_STATUS_REGISTER,          NULL},
	{TUYA_IPC_STATUS_REGISTER_FAILED,   __on_register_fail},
	{TUYA_IPC_STATUS_ACTIVED,           NULL},
	{TUYA_IPC_STATUS_RESET,             NULL},
	{TUYA_IPC_STATUS_NETCFG_START,      __on_netcfg_start},
	{TUYA_IPC_STATUS_NETCFG_STOP,       __on_netcfg_stop},
	{TUYA_IPC_STATUS_WIFI_STA_UNCONN,   NULL},
	{TUYA_IPC_STATUS_WIFI_STA_CONN,     __on_wifi_connect},
	{TUYA_IPC_STATUS_WIRE_UNCONN,       NULL},
	{TUYA_IPC_STATUS_WIRE_CONN,         NULL}, 
	{TUYA_IPC_STATUS_ONLINE,            __on_status_online},
	{TUYA_IPC_STATUS_OFFLINE,           __on_status_offline},    
	{TUYA_IPC_STATUS_UPGRADE_START,     __on_status_fw_upgrade_start}, 
};

VOID TUYA_IPC_Status_Changed_cb(IN TUYA_IPC_STATUS_GROUP_E changed_group, IN CONST TUYA_IPC_STATUS_E status[TUYA_IPC_STATUS_GROUP_MAX])
{
	TUYA_IPC_STATUS_E cur_status = status[changed_group];
	INT_T i;
	PR_DEBUG("status chaged: group[%d] status[%d]", changed_group, cur_status);
	
	for(i = 0; i < SIZEOF(status_change_cb_map)/SIZEOF(IPC_STATUS_CHANGE_CB_MAP_T); i++) {
		if(status_change_cb_map[i].stat == cur_status) {
			if(status_change_cb_map[i].cb) {
				status_change_cb_map[i].cb(NULL);
			} else {
				PR_DEBUG("status ignore. cb is null.");
			}
		}
	}    
}

INT_T TUYA_IPC_sd_status_upload(INT_T status)
{
	IPC_APP_report_sd_status_changed(status);
	return 0;
}

VOID TUYA_IPC_qrcode_shorturl_cb(CHAR_T* shorturl)
{
	if(shorturl)
		PR_INFO("shorturl: %s}", shorturl); //according to requirements of shadow device, do not modify!!!

	return;
}


/* 
Callback when the user clicks on the APP to remove the device
*/
VOID TUYA_IPC_Reset_System_CB(GW_RESET_TYPE_E type)
{
	PR_INFO("reset ipc success. please restart the ipc %d\n", type);

	/* Developers need to restart IPC operations */
	system("rm -rf /etc/tuya_user.db");
	system("rm -rf /etc/tuya_enckey.db");
	system("rm -rf /etc/tuya_user.db_bak");
	// system("rm -rf /etc/tuya_dp_cfg.json");
	sleep(1);
	system("sync && reboot");
}

VOID TUYA_IPC_Restart_Process_CB(VOID)
{
	PR_INFO("sdk internal restart request. please restart the ipc\n");
	//TODO
	/* Developers need to implement restart operations. Restart the process or restart the device. */
}

INT_T TUYA_IPC_Get_MqttStatus()
{
	return s_mqtt_status;
}


INT_T IPC_APP_get_battery_percent(VOID)
{
	return bat_percent;
}

void IPC_APP_set_pir_status(int status)
{
	pir_status = status;
}

INT_T IPC_APP_get_pir_status(VOID)
{
	return pir_status;
}

CHAR_T *IPC_APP_get_power_mode(VOID)
{
	static char bat_str[5];
	sprintf(bat_str, "%d", bat_mode);
	return bat_str;
}

#if 1
//
int client_create_thread(int *pfd, int port, \
	void *(rec_thread)(void *arg), \
	void *(send_thread)(void *arg))
{
	struct sockaddr_in t_sockaddr;
	int res = -1;

	*pfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*pfd < 0) {
		res = -1;
		printf("client_create port:[%d] fd:[%d]\n", port, *pfd);
		goto _end;
	}

	memset(&t_sockaddr, 0, sizeof(struct sockaddr_in));

	t_sockaddr.sin_family = AF_INET;
	t_sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, "localhost", &t_sockaddr.sin_addr);

	res = connect(*pfd, (struct sockaddr *)&t_sockaddr, sizeof(struct sockaddr));
	if (res < 0) {
		printf("client_create connect port:[%d] res:[%d]\n", port, res);
		goto _end;
	}

	pthread_t tid;

	if (send_thread != NULL) {
		res = pthread_create(&tid, NULL, send_thread, pfd);
		if (res < 0) {
			goto _end;
		}
	}

	if (rec_thread != NULL) {
		res = pthread_create(&tid, NULL, rec_thread, pfd);
		if (res < 0) {
			goto _end;
		}
	}

_end:
	printf("client_create_thread res:%d\n", res);
	return res;
}

static int ctrl_cmd_handle(char *src, char *dst)
{
	static uint8_t bat_first_report = 0;
	int percent, mode;

	const char *cmd[] = {
		[cmd_key_short] = "$$key,short",
		[cmd_key_long] = "$$key,long",
		[cmd_bat_info] = "$$bat,",
		[cmd_pir_trigger] = "$$pir,trigger",
		[cmd_pir_detect] = "$$pir,detect",
	};

	for (int i = 0; i < (sizeof(cmd) / sizeof(cmd[0])); i++) {
		if (strncmp(src, cmd[i], strlen(cmd[i])) == 0) {
			switch (i) {
			case cmd_key_short:
				printf("ctrl_cmd_handle->short\n");
				break;
			case cmd_key_long:
				printf("ctrl_cmd_handle->long\n");
				tkl_audio_play_aac(0, 0, "/etc/tone2.aac");
				sleep(1);
				TUYA_IPC_Reset_System_CB(0);
				break;
			case cmd_bat_info:
				sscanf(src, "$$bat,%d,%d", &mode, &percent);
				if ((bat_mode != mode) || (bat_percent != percent) || (bat_first_report == 0)) {
					bat_mode = mode;
					bat_percent = percent;
					bat_first_report = 1;
					printf("bat info mode=%d, percent=%d, %d-%d\n", mode, percent, bat_mode, bat_percent);
					IPC_APP_update_battery_status();
				}
				break;
			case cmd_pir_trigger:
				PR_INFO("cmd_pir_trigger...\n");
				IPC_APP_set_pir_status(1);
				break;
			case cmd_pir_detect:
				PR_INFO("cmd_pir_detect...\n");
				IPC_APP_set_pir_status(0);
				break;
			}
		}
	}

	return 0;
}

void *thread_devctrl_rec(void *arg)
{
	int *pfd = (int *)arg;
	char *pbuff = NULL;
	int buflen = 0;
	int bufsize = 256;
	int len = 0;

	pbuff = malloc(bufsize);
	if (pbuff == NULL) {
		goto _end;
	}

	while (1) {
		// ˢ�¾��
		devctrl_client_fd = *pfd;
		// ��������(����)
		len = recv(*pfd, pbuff, bufsize, 0);
		if (len < 0) {
			break;
		}
		// ������������
		pbuff[len] = 0;
		ctrl_cmd_handle(pbuff, NULL);
		// ˢ�¾��
		devctrl_client_fd = *pfd;
	}

_end:
	free(pbuff);
	printf("thread_devctrl_rec end.\n");
	return NULL;
}

// ����LED״̬
void ledkey_set_mode(led_status_t state)
{
	//$$led,0
	if (devctrl_client_fd >= 0) {
		char cmdstr[10];
		sprintf(cmdstr, "$$led,%d", state);
		write(devctrl_client_fd, cmdstr, strlen(cmdstr));
	}
}

// �ֶ����Ѳ���
void *thread_siren_play(void *arg)
{
	while(1){
		if (IPC_APP_get_siren_onoff()){
			tkl_audio_play_aac(0, 0, "/etc/beep_siren.aac");
		}
		sleep(1);
	}
}

//�豸����
int devctrl_init(void)
{
	int res = client_create_thread(&devctrl_client_fd, 8000, thread_devctrl_rec, NULL);
	printf("devctrl_client_fd res=%d\n", res);

	pthread_t tid;
	pthread_create(&tid, NULL, thread_siren_play, NULL);

	return 0;
}
#endif

