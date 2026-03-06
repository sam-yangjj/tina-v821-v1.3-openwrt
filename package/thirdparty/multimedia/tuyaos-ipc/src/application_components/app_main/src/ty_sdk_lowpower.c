/*********************************************************************************
  *Copyright(C),2015-2020,
  *TUYA
  *www.tuya.comm
  *FileName:    lowpower demo
**********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_sdk_init.h"
#include "ty_sdk_common.h"
#include "tkl_video_in.h"
#include "tkl_media.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_event.h"
#include "tkl_video_enc.h"

#define COMM_LEN 30
typedef struct {
	TUYA_IP_ADDR_T ip;
	int port;
	char devid[COMM_LEN];
	char local_key[COMM_LEN];
}tuya_lowpower_info;

tuya_lowpower_info tuya_lp_info;

#define RPBUF_NAME "rpbuf_tuya"
#define RPBUF_LEN 128
extern VOID IPC_APP_update_awake_status(BOOL_T true_false);

#ifdef ENABLE_DEMO_LOWPOWER

/*
---------------------------------------------------------------------------------
Low power access reference code
en:TRUE is sleep      FALSE is wake
---------------------------------------------------------------------------------
*/
#define PR_TRACE
#define MAXBUF 512

OPERATE_RET TUYA_APP_LOW_POWER_START(CHAR_T *devbuf, CHAR_T *keybuf, TUYA_IP_ADDR_T ip, INT_T port, CHAR_T *domain_name)
{
    INT_T ret=1;
    INT_T i=0;
    INT_T low_power_socket =-1;
    CHAR_T wakeData[36] = {0};
    INT_T wake_data_len = SIZEOF(wakeData);
    INT_T fail_cnt = 0;

    // demo 写死 ipv4 模式，如果是IPV6  请按情况适配下面API；优先使用域名解析地址
    TUYA_IP_ADDR_T tmp_ip_addr = {0};
    ret = tal_net_gethostbyname(domain_name,TY_AF_INET,&tmp_ip_addr);
    if (0 != ret){
      PR_ERR(" domain resolution failed [%s]", domain_name);
    }else{
      ret = tuya_ipc_low_power_server_connect(tmp_ip_addr, port, devbuf, strlen(devbuf), keybuf, strlen(keybuf));
      if (0 != ret){
        PR_ERR(" retry connect failed [%u]", tmp_ip_addr.u_addr.ip4);
      }
    }
    // 如果域名解析 IP 连接失败，尝试使用接口返回IP地址
    while(0 != ret && fail_cnt < 3)
    {
        ret = tuya_ipc_low_power_server_connect(ip, port, devbuf, strlen(devbuf), keybuf, strlen(keybuf));
        fail_cnt++;
    }

    if (0 != ret){
      PR_ERR(" ip connect file , break");
      return -1;
    }
    PR_DEBUG("power_server_connect over.\n");

    while(low_power_socket == -1)
    {
       low_power_socket= tuya_ipc_low_power_socket_fd_get();
    }

    tuya_ipc_low_power_wakeup_data_get(wakeData, &wake_data_len);

    PR_DEBUG("wake up date is { ");
    for(i=0;i<wake_data_len;i++)
    {
        PR_DEBUG("0x%x ",wakeData[i]);
    }
    PR_DEBUG(" }\n");

    CHAR_T heart_beat[12] = {0};
    INT_T heart_beat_len = SIZEOF(heart_beat);
    tuya_ipc_low_power_heart_beat_get(heart_beat,&heart_beat_len);
    PR_DEBUG("heart beat data is { ");

    for(i=0;i<heart_beat_len;i++)
    {
        PR_DEBUG("0x%x ",heart_beat[i]);
    }
    PR_DEBUG(" }\n");

    fd_set rfds;
    struct timeval tv;
    INT_T retval, maxfd = -1;

    INT_T len=0;
    CHAR_T recBuf[MAXBUF]={0};
    INT_T heart_timeout=5;
    INT_T user_set_timeout=10;
    while(1)
    {
        FD_ZERO(&rfds);
        FD_SET(0,&rfds);
        maxfd=0;
        FD_SET(low_power_socket,&rfds);
        if (low_power_socket > maxfd)
        {
          maxfd = low_power_socket;
        }
        tv.tv_sec = user_set_timeout;//default 10 seconds;
        tv.tv_usec = 0;

        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
          PR_DEBUG("Will exit and the select is error! %s", strerror(errno));
          break;
        }
        else if (retval == 0)
        {
          PR_DEBUG("============send heart beat==============\n");
          len = send(low_power_socket, heart_beat, heart_beat_len, 0);
          if (len < 0)
          {
              PR_DEBUG("socket =%d %d\n",low_power_socket,errno);
              break;
          }
          else
          {
            PR_DEBUG("News: %d \t send, sent a total of %d bytes!\n",
                    heart_beat_len, len);
          }

          continue;
        }
        else
        {
            if (FD_ISSET(low_power_socket, &rfds))
            {
              bzero(recBuf, MAXBUF);
              PR_DEBUG("============recv data==============\n");
              len = recv(low_power_socket, recBuf, MAXBUF, 0);
              if (len > 0)
              {
                  PR_DEBUG("Successfully received the message: is {");
                  for(i=0;i<len;i++)
                      PR_DEBUG("0x%02x ",recBuf[i]);
                  PR_DEBUG("}\n");
                  if(strncmp(recBuf,wakeData,wake_data_len)==0)
                  {
                      //TODO  启动SDK
                      PR_DEBUG("recve data is wake up\n");
                  }

              }
              else
              {
                if (len < 0)
                    PR_DEBUG("Failed to receive the message! \
                          The error code is %d, error message is '%s'\n",
                          errno, strerror(errno));
                else
                    PR_DEBUG("Chat to terminate len=0x%x!\n",len);

                break;
              }
            }

        }
    }

    return 0;
}

#endif
VOID uninit_machine_status(VOID)
{
	int ret;

	ret = tkl_vi_detect_stop(0, TKL_MEDIA_DETECT_TYPE_MOTION);
	ret |= tkl_vi_detect_stop(0, TKL_MEDIA_DETECT_TYPE_HUMAN);
	if (ret != 0) {
		printf("close detect failed\n", ret);
	}
	printf("close detect\n");

	if (E_STORAGE_START == tuya_ipc_ss_get_status()) {
		tuya_ipc_stop_storage(E_ALARM_SD_STORAGE);
		printf("close sd records\n");
		usleep(2000*1000);
	}

	tkl_vi_uninit();
	printf("close vi\n");
	usleep(100*1000);

	printf("update_machine_status end\n");
}

int TUYA_IPC_low_power_sample(VOID)
{
	TUYA_IP_ADDR_T ip = { 0 };
	int port = 0;
	char domain_name[64] = {0};
	int ret = tuya_ipc_get_low_power_server_v2(domain_name, sizeof(domain_name), &ip, &port);
	if (ret != 0) {
		PR_ERR("get low power ip  error %d\n", ret);
		goto exit_err;
	}

	char devid[COMM_LEN] = { 0 };
	int id_len = COMM_LEN;
	ret = tuya_ipc_get_device_id(devid, &id_len);
	if (ret != 0) {
		PR_ERR("get devide error %d\n", ret);
		goto exit_err;
	}
	char local_key[COMM_LEN] = { 0 };
	int key_len = COMM_LEN;
	ret = tuya_ipc_get_local_key(local_key, &key_len);
	if (ret != 0) {
		PR_ERR("get local key  error %d\n", ret);
		goto exit_err;
	}

	//   /* 47:101:10:151 variable */
	// printf("TUYA ip = %d.%d.%d.%d\n", (ip.u_addr.ip4 >> 24) & 0xff, (ip.u_addr.ip4 >> 16) & 0xff, (ip.u_addr.ip4 >> 8) & 0xff, ip.u_addr.ip4 & 0xff);
	// printf("TUYA port = %d\n", port);  /* 443 */
	// printf("TUYA deviceID = %s\n", devid);  /* 6c536b059bc0b2d6e6twxh */
	// printf("TUYA local_key = %s\n", local_key);  /* crEFrTkEEFPmR8ex variable */

	memset(&tuya_lp_info, 0, sizeof(tuya_lp_info));
	tuya_lp_info.ip = ip;
	tuya_lp_info.port = port;
	memcpy(tuya_lp_info.devid, devid, COMM_LEN);
	memcpy(tuya_lp_info.local_key, local_key, COMM_LEN);

	ret = tkl_wifi_rpmsg_init();
	if (ret) {
		printf("tkl_wifi_rpmsg_init faild \n");
		goto exit_err;
	}
	usleep(30 * 1000);

	uninit_machine_status();

	/* close led for power consumption*/
	system("killall devctrl");
	system("echo 0 > /sys/class/gpio/gpio103/value");
	system("echo 103 > /sys/class/gpio/unexport");
	system("echo 0 > /sys/class/gpio/gpio111/value");
	system("echo 111 > /sys/class/gpio/unexport");
	system("echo 0 > /sys/class/gpio/gpio112/value");
	system("echo 112 > /sys/class/gpio/unexport");
	system("echo 0 > /sys/class/gpio/gpio113/value");
	system("echo 113 > /sys/class/gpio/unexport");

	IPC_APP_update_awake_status(false);
	usleep(10*1000);

	// tkl_wifi_rpbuf_send(RPBUF_NAME, RPBUF_LEN, &tuya_lp_info, sizeof(tuya_lp_info));
	tkl_wifi_rpmsg_send(&tuya_lp_info, sizeof(tuya_lp_info));
	system("ip route del default");  //disable network

	tkl_wifi_deinit();  /* fix re-mqtt when standby */

	char tuya_tmp_buf[RPBUF_LEN] = { 0 };
	int tuya_tmp_len = 0;
	tuya_tmp_len = tkl_wifi_rpmsg_recv(tuya_tmp_buf, sizeof(tuya_tmp_buf));
	// tkl_wifi_rpbuf_recv(RPBUF_NAME, RPBUF_LEN, tuya_tmp_buf, &tuya_tmp_len);  /* Unvalidated， only recv */
	printf("tkl_wifi_rpmsg_recv: tuya_tmp_buf = %s, tuya_tmp_len = %d\n", tuya_tmp_buf, tuya_tmp_len);

	tkl_wifi_enter_ultra_standby();

#ifdef ENABLE_DEMO_LOWPOWER
	TUYA_APP_LOW_POWER_START(devid, local_key, ip, port, domain_name);
#endif
exit_err:
	return ret;
}
