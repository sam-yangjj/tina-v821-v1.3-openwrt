#include "tkl_wifi.h"
#include "tuya_cloud_types.h"
#include "wifimg.h"
#include "wifi_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "librpbuf.h"
#include "librpmsg.h"
#include "local.h"
#include <unistd.h>
#include <fcntl.h>

// #define DEBUG_VERSION
#define SCAN_RES_LEN 50
#define SSID_MAX_LEN 32
#define PSK_MAX_LEN  64

#define BAND_NOME     0
#define BAND_2_4G     1
#define BAND_5G       2

static WF_WK_MD_E wf_mode = WWM_POWERDOWN;

WIFI_EVENT_CB tkl_wifi_status_cb = NULL;

static wifi_secure_t tkl_get_secure_type(wifi_sec key_mgmt);
static UCHAR_T tkl_wmg_key_mgmt_to_security(wifi_sec key_mgmt);

static void uint8tochar(char *mac_addr_char, uint8_t *mac_addr_uint8)
{
	sprintf(mac_addr_char,"%02x:%02x:%02x:%02x:%02x:%02x",
						(unsigned char)mac_addr_uint8[0],
						(unsigned char)mac_addr_uint8[1],
						(unsigned char)mac_addr_uint8[2],
						(unsigned char)mac_addr_uint8[3],
						(unsigned char)mac_addr_uint8[4],
						(unsigned char)mac_addr_uint8[5]);
	mac_addr_char[17] = '\0';
}

static VOID_T wifi_status_cb(WF_EVENT_E event, VOID_T *arg)
{
	if (tkl_wifi_status_cb) {
		tkl_wifi_status_cb(event, arg);
	} else {
		printf("Not register wifi status callback.\n");
	}
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_init(WIFI_EVENT_CB cb)
{
	int iret = 0;
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	// if (cb == NULL)
	// {
	// 	printf("callbask func is null");
	// 	return OPRT_INVALID_PARM;
	// }

	iret = wifimanager_init();
	if (iret < 0) {
		printf("TKL WiFi : wifi init failed");
		return OPRT_COM_ERROR;
	}

	wmg_set_debug_level(WMG_MSG_INFO);
	printf("wmg_set_debug_level ==================== WMG_MSG_INFO\n");


	tkl_wifi_status_cb = cb;

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_deinit(VOID_T)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	iret = wifimanager_deinit();
	if (iret < 0)
	{
		printf("TKL WiFi : wifi init failed");
		return OPRT_COM_ERROR;
	}
	return OPRT_OK;
}

static uint32_t freq_to_channel(uint32_t freq)
{
	int band;
	uint32_t channel = 0;
	if ((freq >= 5180) && (freq <= 5825)) {
		band = BAND_5G;
	} else if ((freq >= 2407) && (freq <= 2484)) {
		band = BAND_2_4G;
	} else {
		band = BAND_NOME;
	}
	switch (band) {
	case BAND_2_4G:
		if (freq == 2484) {
			channel = 14;
		} else if (freq == 2407) {
			channel = 0;
		} else if ((freq <= 2472) && (freq > 2407)) {
			if (((freq - 2407) % 5) == 0) {
				channel = ((freq - 2407) / 5);
			} else {
				channel = 1000;
			}
		} else {
			channel = 1000;
		}
		break;
	case BAND_5G:
		if (((freq - 5180) % 5) == 0) {
			channel = ((freq - 5180) / 5) + 36;
		} else {
			channel = 1000;
		}
		break;
	case BAND_NOME:
	default:
		channel = 1000;
		break;
	}
	return channel;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_scan_ap(CONST SCHAR_T *ssid, AP_IF_S **ap_ary, UINT_T *num)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	wmg_status_t ret = WMG_STATUS_FAIL;
	wifi_scan_result_t scan_res[SCAN_RES_LEN] = {0};
	int i, bss_num = 0;
	AP_IF_S *s_aps = NULL;

	if (tkl_wifi_set_work_mode(WWM_STATION)) {
		printf("wifi set world mode failed\n");
		return OPRT_INVALID_PARM;
	}

	scan_res->scan_action = true;
	ret = wifi_get_scan_results(scan_res, ssid, &bss_num, SCAN_RES_LEN);
	if (ret == WMG_STATUS_SUCCESS) {
		printf("==Wi-Fi scan success!\n");
		bss_num = (bss_num > SCAN_RES_LEN ? SCAN_RES_LEN : bss_num);
		s_aps = (AP_IF_S *)malloc(bss_num  * sizeof(AP_IF_S));
		if (s_aps == NULL) {
			printf("malloc s_aps fail\n");
			return OPRT_OS_ADAPTER_MALLOC_FAILED;
		}
		*num = bss_num;
		for (i = 0; i < bss_num; i++) {
			s_aps[i].rssi = scan_res[i].rssi;
			s_aps[i].channel = freq_to_channel(scan_res[i].freq);
			memcpy(s_aps[i].bssid, scan_res[i].bssid, sizeof(scan_res[i].bssid));
			memcpy(s_aps[i].ssid, scan_res[i].ssid, SSID_MAX_LEN + 1);
			s_aps[i].s_len = strlen(scan_res[i].ssid);
			s_aps[i].security = tkl_wmg_key_mgmt_to_security(scan_res[i].key_mgmt);
		}
	} else {
		printf("==Wi-Fi scan failed!\n");
		return OPRT_COM_ERROR;
	}
	*ap_ary = s_aps;

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_release_ap(AP_IF_S *ap)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (ap == NULL)
	{
		printf("wrong param");
		return OPRT_INVALID_PARM;
	}

	printf("#####:  func=%s, line=%d, ap=%p\n", __func__, __LINE__, ap);
	free(ap);
	ap = NULL;

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_cur_channel(CONST UCHAR_T chan)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	int i;
	wifi_ap_config_t ap_config;
	char ssid_buf[SSID_MAX_LEN + 1] = {0};
	char psk_buf[PSK_MAX_LEN + 1] = {0};

	/* judge mode */
	WF_WK_MD_E use_mode = 0;
	tkl_wifi_get_work_mode(&use_mode);
	if(use_mode != WWM_SOFTAP) {
		printf("not ap mode,can't set channel!\n");
		return -1;
	}

	ap_config.ssid = ssid_buf;
	ap_config.psk = psk_buf;
	ap_config.dev_list = (wifi_sta_dev_member_t *)malloc(sizeof(wifi_sta_dev_member_t) * STA_MAX_NUM);
	if (ap_config.dev_list != NULL) {
		ap_config.sta_dev_list_num = STA_MAX_NUM;
	} else {
		printf( "%s %d: malloc error\n", __func__, __LINE__ );
		iret = -1;
		goto results;
	}

	/* get old config */
	iret = wifi_ap_get_config(&ap_config);
	if(iret != WMG_STATUS_SUCCESS) {
		printf("wifi_ap_get_config erro!\n");
		goto results;
	}
	/* set new channel */
	ap_config.channel = chan;

	/* stop ap mode */
	iret = tkl_wifi_stop_ap();
	if(iret != WMG_STATUS_SUCCESS) {
		printf("tkl_wifi_stop_ap erro!\n");
		goto results;
	}

	/* set new config */
	iret = wifi_ap_enable(&ap_config);
	if (iret == WMG_STATUS_SUCCESS) {
		printf("ap enable success, ssid=%s, psk=%s, sec=%d, channel=%d\n", ap_config.ssid, ap_config.psk, ap_config.sec, ap_config.channel);
	} else {
		printf("ap enable failed\n");
		goto results;
	}

results:
	if (ap_config.dev_list != NULL) {
		free(ap_config.dev_list);
		ap_config.dev_list = NULL;
		ap_config.sta_dev_list_num = 0;
	}

	if (iret != 0)
	{
		printf("set channel failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_cur_channel(UCHAR_T *chan)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	int i;
	wifi_sta_info_t wifi_sta_info;
	wifi_ap_config_t ap_config;
	char ssid_buf[SSID_MAX_LEN + 1] = {0};
	char psk_buf[PSK_MAX_LEN + 1] = {0};
	WF_WK_MD_E use_mode = 0;
	tkl_wifi_get_work_mode(&use_mode);

	if(use_mode == WWM_STATION) {
		iret = wifi_sta_get_info(&wifi_sta_info);
		if (iret == WMG_STATUS_SUCCESS)
		{
			*chan = freq_to_channel(wifi_sta_info.freq);
		} else {
			printf("get channel failed");
			return OPRT_COM_ERROR;
		}
	} else if (use_mode == WWM_SOFTAP) {
		/* need memory to store! */
		ap_config.ssid = ssid_buf;
		ap_config.psk = psk_buf;
		ap_config.dev_list = (wifi_sta_dev_member_t *)malloc(sizeof(wifi_sta_dev_member_t) * STA_MAX_NUM);
		if (ap_config.dev_list != NULL) {
			ap_config.sta_dev_list_num = STA_MAX_NUM;
		} else {
			printf( "%s %d: malloc error\n", __func__, __LINE__ );
			iret = -1;
			goto results;
		}

		iret = wifi_ap_get_config(&ap_config);
		if(iret == 0) {
			*chan = ap_config.channel;
		} else {
			printf("wifi_ap_get_config erro!\n");
			goto results;
		}
	}

results:
	if (ap_config.dev_list != NULL) {
		free(ap_config.dev_list);
		ap_config.dev_list = NULL;
		ap_config.sta_dev_list_num = 0;
	}

	if (iret < 0) {
		printf("get ip failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_sniffer(CONST BOOL_T en, CONST SNIFFER_CALLBACK cb)
{
	int iret = 0;

#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (iret < 0)
	{
		printf("set wifi sniffer failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

/**
* @brief get wifi ipv6 info.when wifi works in
*        ap+station mode, wifi has two ips.
*
* @param[in]       wf          wifi function type
* @param[out]      ip          the ip addr info
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_wifi_get_ipv6(CONST WF_IF_E wf, NW_IP_TYPE type, NW_IP_S *ip)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_ip(CONST WF_IF_E wf, NW_IP_S *ip)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d, mode:%d\n", __func__, __LINE__,wf);
#endif
	int iret = 0;

	if (ip == NULL) {
		printf( "%s %d: invalid param\n", __func__, __LINE__ );
		return OPRT_INVALID_PARM;
	}

	if ( TY_AF_INET != ip->type ) { //仅支持IPV4，TY_AF_INET6待添加
		printf( "%s %d: invalid param type=%d\n", __func__, __LINE__, ip->type );
		return OPRT_INVALID_PARM;
	}

	if (wf == WF_AP) {
		wifi_ap_config_t ap_config;
		char ssid_buf[SSID_MAX_LEN + 1] = {0};
		char psk_buf[PSK_MAX_LEN + 1] = {0};
		char tmp_buf[16] = {0};

		ap_config.ssid = ssid_buf;
		ap_config.psk = psk_buf;
		ap_config.dev_list = (wifi_sta_dev_member_t *)malloc(sizeof(wifi_sta_dev_member_t) * STA_MAX_NUM);
		if (ap_config.dev_list != NULL) {
			ap_config.sta_dev_list_num = STA_MAX_NUM;
		} else {
			printf( "%s %d: malloc error\n", __func__, __LINE__ );
			return OPRT_INVALID_PARM;
		}

		iret = wifi_ap_get_config(&ap_config);
		if (iret == WMG_STATUS_SUCCESS) {
			sprintf(ip->nwipstr, "%d.%d.%d.%d",
				ap_config.ip_addr[0],
				ap_config.ip_addr[1],
				ap_config.ip_addr[2],
				ap_config.ip_addr[3]);

			sprintf(ip->nwgwstr, "%d.%d.%d.%d",
				ap_config.gw_addr[0],
				ap_config.gw_addr[1],
				ap_config.gw_addr[2],
				ap_config.gw_addr[3]);

			sprintf(ip->nwmaskstr, "255.255.255.0");

#ifdef DEBUG_VERSION
			printf("ap mode: ip:%s,mask:%s,gw:%s\n", ip->nwipstr, ip->nwmaskstr, ip->nwgwstr);
#endif
		}
		
		if (ap_config.dev_list != NULL) {
			free(ap_config.dev_list);
			ap_config.dev_list = NULL;
			ap_config.sta_dev_list_num = 0;
		}
	}

	if (wf == WF_STATION) {
		wifi_sta_info_t wifi_sta_info;
		if (tkl_wifi_set_work_mode(WWM_STATION)) {
			printf("wifi set world mode failed\n");
			return OPRT_INVALID_PARM;
		}

		memset(&wifi_sta_info, 0, sizeof(wifi_sta_info_t));
		iret = wifi_sta_get_info(&wifi_sta_info);
		if (iret == WMG_STATUS_SUCCESS) {
			sprintf(ip->nwipstr,"%d.%d.%d.%d",
				wifi_sta_info.ip_addr[0],
				wifi_sta_info.ip_addr[1],
				wifi_sta_info.ip_addr[2],
				wifi_sta_info.ip_addr[3]);

			sprintf(ip->nwgwstr,"%d.%d.%d.%d",
				wifi_sta_info.gw_addr[0],
				wifi_sta_info.gw_addr[1],
				wifi_sta_info.gw_addr[2],
				wifi_sta_info.gw_addr[3]);

			strcpy(ip->nwmaskstr,"255.255.255.0"); //TODO
#ifdef DEBUG_VERSION
			printf("sta mode: ip:%s,mask:%s,gw:%s\n",ip->nwipstr,ip->nwmaskstr,ip->nwgwstr);
#endif
		}

	}

	if (iret < 0) {
		printf("get ip failed");
		return OPRT_COM_ERROR;
	}
	return OPRT_OK;
}

// TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_ip(CONST WF_IF_E wf, NW_IP_S *ip)
// {
// #ifdef DEBUG_VERSION
// 	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
// #endif
// 	int iret = 0;

// 	if (!ip) {
// 		printf("arg is null");
// 		return OPRT_COM_ERROR;
// 	}

// 	if (iret < 0) {
// 		printf("set ip failed");
// 		return OPRT_COM_ERROR;
// 	}

// 	return OPRT_OK;
// }

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_mac(CONST WF_IF_E wf, CONST NW_MAC_S *mac)/* unsupport */
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	uint8_t mac_addr[6] = {0};
	memcpy(mac_addr,mac->mac,sizeof(mac_addr));
	iret = wifi_set_mac(NULL, mac_addr);
	if (iret < 0) {
		printf("SET MAC failed");
		return OPRT_COM_ERROR;
	} else {
		printf("WiFi SET MAC success, MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
		       mac->mac[0], mac->mac[1], mac->mac[2], mac->mac[3], mac->mac[4], mac->mac[5]);
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_mac(CONST WF_IF_E wf, NW_MAC_S *mac)/* unsupport */
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	uint8_t mac_addr[6] = {0};
	char mac_addr_char[18] = {0};
	iret = wifi_get_mac(NULL, mac_addr);
	if (iret < 0) {
		printf("GET MAC failed");
		return OPRT_COM_ERROR;
	} else {
		memcpy(mac->mac,mac_addr,sizeof(mac_addr));
		printf("GET MAC success, MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
		       mac->mac[0], mac->mac[1], mac->mac[2], mac->mac[3], mac->mac[4], mac->mac[5]);
	}

	return OPRT_OK;
}

static char sta_msg_cb_char[32] = "sta msg cb arg";

void wifi_msg_cb(wifi_msg_data_t *msg)
{
	char msg_cb_arg[32] = "NULL";
	char *msg_cb_arg_p;
	//msg cb test, need to pass static parameters
	if (msg->private_data) {
		msg_cb_arg_p = (char *)msg->private_data;
	} else {
		msg_cb_arg_p = msg_cb_arg;
	}
	printf("tkl_wifi cb\n");
	switch(msg->id) {
		case WIFI_MSG_ID_DEV_STATUS:
			printf("dev ");
			switch(msg->data.d_status) {
				case WLAN_STATUS_DOWN:
					printf("down ");
					break;
				case WLAN_STATUS_UP:
					printf("up ");
					break;
				default:
					printf("unknow ");
					break;
			}
			printf("arg:%s\n", msg_cb_arg_p);
			break;
		case WIFI_MSG_ID_STA_CN_EVENT:
			printf("sta event ");
			switch(msg->data.event) {
				case WIFI_DISCONNECTED:
					printf("disconnect ");
					wifi_status_cb(WFE_DISCONNECTED, NULL);
					break;
				case WIFI_SCAN_STARTED:
					printf("scan started ");
					break;
				case WIFI_SCAN_FAILED:
					printf("scan failed ");
					break;
				case WIFI_SCAN_RESULTS:
					printf("scan results ");
					break;
				case WIFI_NETWORK_NOT_FOUND:
					printf("network not found ");
					break;
				case WIFI_PASSWORD_INCORRECT:
					printf("password incorrect ");
					break;
				case WIFI_AUTHENTIACATION:
					printf("authentiacation ");
					break;
				case WIFI_AUTH_REJECT:
					printf("auth reject ");
					break;
				case WIFI_ASSOCIATING:
					printf("associating ");
					break;
				case WIFI_ASSOC_REJECT:
					printf("assoc reject ");
					break;
				case WIFI_ASSOCIATED:
					printf("associated ");
					break;
				case WIFI_4WAY_HANDSHAKE:
					printf("4 way handshake ");
					break;
				case WIFI_GROUNP_HANDSHAKE:
					printf("grounp handshake ");
					break;
				case WIFI_GROUNP_HANDSHAKE_DONE:
					printf("handshake done ");
					break;
				case WIFI_CONNECTED:
					printf("connected ");
					break;
				case WIFI_CONNECT_TIMEOUT:
					wifi_status_cb(WFE_CONNECT_FAILED, NULL);
					printf("connect timeout ");
					break;
				case WIFI_DEAUTH:
					printf("deauth ");
					break;
				case WIFI_DHCP_START:
					printf("dhcp start ");
					break;
				case WIFI_DHCP_TIMEOUT:
					printf("dhcp timeout ");
					break;
				case WIFI_DHCP_SUCCESS:
					wifi_status_cb(WFE_CONNECTED, NULL);
					printf("dhcp success ");
					break;
				case WIFI_TERMINATING:
					printf("terminating ");
					break;
				case WIFI_UNKNOWN:
				default:
					printf("unknow ");
					break;
			}
			printf("arg:%s\n", msg_cb_arg_p);
			break;
		case WIFI_MSG_ID_STA_STATE_CHANGE:
			printf("sta state ");
			switch(msg->data.state) {
				case WIFI_STA_IDLE:
					printf("idle ");
					break;
				case WIFI_STA_CONNECTING:
					printf("connecting ");
					break;
				case WIFI_STA_CONNECTED:
					printf("connected ");
					break;
				case WIFI_STA_OBTAINING_IP:
					printf("obtaining ip ");
					break;
				case WIFI_STA_NET_CONNECTED:
					printf("net connected ");
					break;
				case WIFI_STA_DHCP_TIMEOUT:
					printf("dhcp timeout ");
					break;
				case WIFI_STA_DISCONNECTING:
					printf("disconnecting ");
					break;
				case WIFI_STA_DISCONNECTED:
					printf("disconnected ");
					break;
				default:
					printf("unknow ");
					break;
			}
			printf("arg:%s\n", msg_cb_arg_p);
			break;
		default:
			break;
	}
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_work_mode(CONST WF_WK_MD_E mode)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d, mode:%d\n", __func__, __LINE__,mode);
#endif
	int iret = 0;
	WF_WK_MD_E use_mode = 0;
	tkl_wifi_get_work_mode(&use_mode);
	if (use_mode == mode)
		return OPRT_OK;

	switch(mode) {
		case WWM_SNIFFER:
			//iret = wifi_on(WIFI_MONITOR); //TODO
			return OPRT_COM_ERROR;
		case WWM_STATION:
			iret = wifi_on(WIFI_STATION);
			wifi_register_msg_cb(wifi_msg_cb, sta_msg_cb_char);
			break;
		case WWM_SOFTAP:
			iret = wifi_on(WIFI_AP);
			break;
		case WWM_STATIONAP:
			iret = wifi_on(WIFI_STATION_AP);
			break;
		case WWM_UNKNOWN:
			printf("wifi WIFI_MODE_UNKNOWN!\n");
			break;
	}

	if (iret < 0) {
		printf("WIFI SET work mode failed:%d\n",mode);
		return OPRT_COM_ERROR;
	}

	wf_mode = mode;

	return OPRT_OK;
}

OPERATE_RET tkl_wifi_set_work_mode_off(CONST WF_WK_MD_E mode)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d, mode:%d\n", __func__, __LINE__,mode);
#endif
	int iret = 0;
	switch(mode) {
		case WWM_SNIFFER:
			//iret = wifi_off(WIFI_MONITOR); //TODO
			return OPRT_COM_ERROR;
		case WWM_STATION:
			iret = wifi_off(WIFI_STATION);
			break;
		case WWM_SOFTAP:
			iret = wifi_off(WIFI_AP);
			break;
		case WWM_STATIONAP:
			iret = wifi_off(WIFI_STATION_AP);
			break;
		case WWM_UNKNOWN:
			iret = wifi_off(WIFI_MODE_UNKNOWN);
			break;
	}
	if (iret < 0) {
		printf("WIFI SET work mode off failed:%d\n",mode);
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_work_mode(WF_WK_MD_E *mode)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	*mode = wf_mode;
#ifdef DEBUG_VERSION
	printf("current_mode:%d\n", *mode);
#endif
	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_start_ap(CONST WF_AP_CFG_IF_S *cfg)
{
	int iret = 0;
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	wifi_ap_config_t ap_config;

	memset(&ap_config, '\0', sizeof(ap_config));
	if (cfg == NULL) {
		printf("invalid parm");
		return OPRT_INVALID_PARM;
	}

	sscanf(cfg->ip.nwipstr, "%d.%d.%d.%d",
	       &ap_config.ip_addr[0],
	       &ap_config.ip_addr[1],
	       &ap_config.ip_addr[2],
	       &ap_config.ip_addr[3]);
	sscanf(cfg->ip.nwgwstr, "%d.%d.%d.%d",
	       &ap_config.gw_addr[0],
	       &ap_config.gw_addr[1],
	       &ap_config.gw_addr[2],
	       &ap_config.gw_addr[3]);

	ap_config.ssid = cfg->ssid;
	ap_config.psk = cfg->passwd;
	ap_config.channel = cfg->chan;

	switch (cfg->md) {
		case WAAM_OPEN:
			ap_config.sec = WIFI_SEC_NONE;
		break;
		case WAAM_WEP:
			ap_config.sec = WIFI_SEC_WEP;
		break;
		case WAAM_WPA_PSK:
			ap_config.sec = WIFI_SEC_WPA_PSK;
		break;
		case WAAM_WPA2_PSK:
			ap_config.sec = WIFI_SEC_WPA2_PSK;
		break;
		case WAAM_WPA_WPA2_PSK:
			ap_config.sec = WIFI_SEC_WPA2_PSK_SHA256;
		break;
		case WAAM_WPA_WPA3_SAE:
			ap_config.sec = WIFI_SEC_WPA3_PSK;
		break;
		case WAAM_UNKNOWN:
			ap_config.sec = WIFI_SEC_NONE;
		break;
	}

	if (tkl_wifi_set_work_mode(WWM_SOFTAP)) {
		printf("wifi set world mode failed\n");
		return OPRT_INVALID_PARM;
	}

	iret = wifi_ap_enable(&ap_config);
	if (iret == 0) {
		printf("ap enable success, ssid=%s, psk=%s, sec=%d, channel=%d\n", ap_config.ssid, ap_config.psk, ap_config.sec, ap_config.channel);
	} else {
		printf("ap enable failed\n");
		return OPRT_INVALID_PARM;
	}
	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_stop_ap(VOID_T)
{
	int iret = 0;
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	iret = wifi_ap_disable();
	if (iret == 0) {
		printf("WIFI STOP softap success\n");
	} else {
		printf("WIFI STOP softap failed\n");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_connected_ap_info(FAST_WF_CONNECTED_AP_INFO_T **fast_ap_info)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	char ssid_buf[SSID_MAX_LEN + 1] = {0};
	char psk_buf[PSK_MAX_LEN + 1] = {0};
	// uint8_t device_list[STA_MAX_NUM][6] = {0};
	int i;
	wifi_ap_config_t ap_config[1]={0};
	char mac_addr_char[18] = {0};

	/* type can't confirm */
	/* FAST_WF_CONNECTED_AP_INFO_T *ap_info_t = NULL;
	ap_info_t = (FAST_WF_CONNECTED_AP_INFO_T *)malloc(sizeof(FAST_WF_CONNECTED_AP_INFO_T) + sizeof(wifi_ap_config_t));
	if(ap_info_t == NULL) {
		printf("WIFI : molloc memory fali!\n");
		return -1;
	}
	ap_info_t->len = sizeof(wifi_ap_config_t);
	ap_config = (wifi_ap_config_t*)&(ap_info_t->data[0]); */

	memset(ap_config, 0, sizeof(wifi_ap_config_t));

	ap_config->ssid = ssid_buf;
	ap_config->psk = psk_buf;
	ap_config->dev_list = (wifi_sta_dev_member_t *)malloc(sizeof(wifi_sta_dev_member_t) * STA_MAX_NUM);
	if (ap_config->dev_list != NULL) {
		ap_config->sta_dev_list_num = STA_MAX_NUM;
	} else {
		iret = -1;
		printf("WIFI : molloc memory fali!\n");
		goto results;
	}

	iret = wifi_ap_get_config(ap_config);
	if(iret == 0) {
		printf("get ap config success:\nssid=%s\npsk=%s\nkey_mgmt=%d\nsec=%d\nchannel=%d\n",
				ap_config->ssid, ap_config->psk,
				tkl_get_secure_type(ap_config->key_mgmt),
				ap_config->sec,ap_config->channel);
		printf("ap ip_addr: %d.%d.%d.%d\n", ap_config->ip_addr[0],
						    ap_config->ip_addr[1],
						    ap_config->ip_addr[2],
						    ap_config->ip_addr[3]);
		printf("ap gw_addr: %d.%d.%d.%d\n", ap_config->gw_addr[0],
						    ap_config->gw_addr[1],
						    ap_config->gw_addr[2],
						    ap_config->gw_addr[3]);
		printf("sta_num=%d\n", ap_config->sta_num);

		// for (i = 0; i < ap_config->sta_num; i++) {
		// 	printf("device mac: %2X:%2X:%2X:%2X:%2X:%2X \n",
		// 				    ap_config->dev_list[i][0],
		// 				    ap_config->dev_list[i][1],
		// 				    ap_config->dev_list[i][2],
		// 				    ap_config->dev_list[i][3],
		// 				    ap_config->dev_list[i][4],
		// 				    ap_config->dev_list[i][5]);
		// }
		for (i = 0; i < ap_config->sta_num; i++) {
			memset(mac_addr_char, 0, 18);
			uint8tochar(mac_addr_char, ap_config->dev_list[i].sta_mac);
			WMG_INFO("device%d: %s\n", i + 1, mac_addr_char);
		}
	}

	if (iret < 0) {
		printf("TKL WIFI get connect ap info failed");
		goto results;
	}

	/* *fast_ap_info = ap_info_t; */
results:
	if (ap_config->dev_list != NULL) {
		free(ap_config->dev_list);
		ap_config->dev_list = NULL;
		ap_config->sta_dev_list_num = 0;
	}

	if (iret) {
		return OPRT_COM_ERROR;
	} else {
		return OPRT_OK;
	}
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_station_fast_connect(CONST FAST_WF_CONNECTED_AP_INFO_T *fast_ap_info)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	return OPRT_COM_ERROR;
	if (iret < 0) {
		printf("WIFI fast connect failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

static char * key_mgmt_to_char(char *key_mgmt_buf, wifi_sec key_mgmt)
{
	if (key_mgmt & WIFI_SEC_WEP) {
		strcat(key_mgmt_buf, "[WEP]");
	}
	if (key_mgmt & WIFI_SEC_WPA_PSK) {
		strcat(key_mgmt_buf, "[WPA]");
	}
	if (key_mgmt & WIFI_SEC_WPA2_PSK) {
		strcat(key_mgmt_buf, "[WPA2]");
	}
	if (key_mgmt & WIFI_SEC_WPA2_PSK_SHA256) {
		strcat(key_mgmt_buf, "[WPA2-SHA]");
	}
	if (key_mgmt & WIFI_SEC_WPA3_PSK) {
		strcat(key_mgmt_buf, "[WPA3]");
	}
	if (key_mgmt & WIFI_SEC_EAP) {
		strcat(key_mgmt_buf, "[EAP]");
	}
	if (key_mgmt == WIFI_SEC_NONE) {
		strcat(key_mgmt_buf, "[NONE]");
	}
	return key_mgmt_buf;
}


static wifi_secure_t tkl_get_secure_type(wifi_sec key_mgmt)
{
	if (key_mgmt & WIFI_SEC_WPA_PSK) {
		return WIFI_SEC_WPA_PSK;
	} else if (key_mgmt & WIFI_SEC_WPA2_PSK) {
		return WIFI_SEC_WPA2_PSK;
	} else if (key_mgmt & WIFI_SEC_WPA2_PSK_SHA256) {
		return WIFI_SEC_WPA2_PSK_SHA256;
	} else if (key_mgmt & WIFI_SEC_WPA3_PSK) {
		return WIFI_SEC_WPA3_PSK;
	} else if (key_mgmt & WIFI_SEC_WEP) {
		return WIFI_SEC_WEP;
	}
}

int g_scan_try = 0;
#define G_SCAN_TRY 5

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_station_connect(CONST SCHAR_T *ssid, CONST SCHAR_T *passwd)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
#define SCAN_TRY 2
	int iret = 0;
	wifi_scan_result_t scan_res[SCAN_RES_LEN] = {0};
	int i, bss_num = 0, get_scan_results_flag = 0, scan_try = 0;
	wifi_sta_cn_para_t cn_para = {0};

	wifi_sta_info_t wifi_sta_info = {0};
	wifi_wmg_state_t wmg_state = {0};

	if (tkl_wifi_set_work_mode(WWM_STATION)) {
		printf("wifi set world mode failed\n");
		return OPRT_INVALID_PARM;
	}

	memset(&wifi_sta_info, 0, sizeof(wifi_sta_info_t));
	wifi_get_wmg_state(&wmg_state);
	if ((wmg_state.current_mode & WIFI_STATION) &&
			wmg_state.sta_state == WIFI_STA_NET_CONNECTED) {
		wifi_sta_get_info(&wifi_sta_info);
		if (!strcmp(wifi_sta_info.ssid, ssid)) {
			WMG_INFO("==wifi already connect to %s\n",wifi_sta_info.ssid);
			return OPRT_OK;
		}
	}

scan:
	memset(scan_res, 0, sizeof(scan_res));
	iret = wifi_get_scan_results(scan_res, ssid, &bss_num, SCAN_RES_LEN);
	if (iret) {
		printf("scan failed.\n");
		return OPRT_COM_ERROR;
	}

	if (bss_num >= SCAN_RES_LEN)
		bss_num = SCAN_RES_LEN;

	for (i = 0; i< bss_num; i++) {
		if (!strcmp(scan_res[i].ssid, ssid)) {
			printf("==The first get scan results success %s\n", scan_res[i].ssid);
			get_scan_results_flag = 1;
			cn_para.sec = tkl_get_secure_type(scan_res[i].key_mgmt);
		}
	}
	if (get_scan_results_flag == 0) {
		scan_try ++;
		if (scan_try >= SCAN_TRY) {
			printf("scan ssid:%s failed, g_scan_try = %d, Max = %d\n", ssid, g_scan_try, G_SCAN_TRY);
			if (++g_scan_try >= G_SCAN_TRY) {
				printf("scan ssid:%s failed, will enter ultra standby, 300s wakeup!\n", ssid);
				if (tkl_wifi_set_work_mode_off(WWM_STATION)) {
					printf("wifi set world mode off failed\n");
					return OPRT_INVALID_PARM;
				}
				sleep(3); // wait wifimanager
				tkl_wifi_enter_ultra_standby_timeout(300 * 1000); //300s is 5 minutes
			}
			return OPRT_COM_ERROR;
		}
		goto scan;
	}

	cn_para.ssid = ssid;
	cn_para.password = passwd;
	cn_para.fast_connect = 1;

	printf("connect to sta(%s) with pask(%s)\n",
			cn_para.ssid, (cn_para.sec != WIFI_SEC_NONE) ? passwd : NULL);

	iret = wifi_sta_connect(&cn_para);
	if (iret == 0) {
		printf("Wi-Fi station connect successful");
	} else {
		printf("WIFI station connect ap failed");
		return OPRT_COM_ERROR;
	}
	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_station_disconnect(VOID_T)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	iret = wifi_sta_disconnect();
	if (iret < 0) {
		printf("TKL WIFI sta disconnect failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_station_get_conn_ap_rssi(SCHAR_T *rssi)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	wifi_sta_info_t wifi_sta_info;
	memset(&wifi_sta_info, 0, sizeof(wifi_sta_info_t));

	iret = wifi_sta_get_info(&wifi_sta_info);
	if (iret == WMG_STATUS_SUCCESS)
	{
		*rssi = wifi_sta_info.rssi;
	} else {
		printf("get connect ap's rssi failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_get_bssid(UCHAR_T *mac)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	int iret = 0;
	wifi_sta_info_t wifi_sta_info;
	memset(&wifi_sta_info, 0, sizeof(wifi_sta_info_t));

	iret = wifi_sta_get_info(&wifi_sta_info);
	if (iret == WMG_STATUS_SUCCESS) {
		memcpy(mac, wifi_sta_info.bssid, sizeof(wifi_sta_info.bssid));
	} else {
		printf("TKL WIFI Get bssid failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_station_get_status(WF_STATION_STAT_E *stat)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	wmg_status_t iret = WMG_STATUS_FAIL;
	wifi_wmg_state_t wmg_state;
	iret = wifi_get_wmg_state(&wmg_state);
	if (iret == WMG_STATUS_SUCCESS) {
		switch (wmg_state.sta_state) {
			case WIFI_STA_IDLE:
				*stat = WSS_IDLE;
				break;
			case WIFI_STA_CONNECTING:
				*stat = WSS_CONNECTING;
				break;
			case WIFI_STA_CONNECTED:
				*stat = WSS_CONN_SUCCESS;
				break;
			case WIFI_STA_OBTAINING_IP:/* unmatched */
				*stat = WSS_IDLE;
				break;
			case WIFI_STA_NET_CONNECTED:
				*stat = WSS_GOT_IP;
				break;
			case WIFI_STA_DHCP_TIMEOUT:
				*stat = WSS_DHCP_FAIL;
				break;
			case WIFI_STA_DISCONNECTING:/* unmatched */
				*stat = WSS_IDLE;
				break;
			case WIFI_STA_DISCONNECTED:
				*stat = WSS_CONN_FAIL;
				break;
			case WIFI_STA_BUSY_TIMEOUT:
				*stat = WSS_NO_AP_FOUND;
		}
	}

#ifdef DEBUG_VERSION
	printf("wifi state:%d\n",*stat);
#endif

	if (iret < 0) {
		printf("TKL WIFI Get status failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_country_code(CONST COUNTRY_CODE_E ccode) /* unsupport */
{
	int iret = -1;

#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (iret < 0) {
		printf("set country code failed");
		return OPRT_COM_ERROR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_send_mgnt(CONST UCHAR_T *buf, CONST UINT_T len)
{
	int iret = 0;

#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (iret < 0) {
		printf("WIFI send mgnt failed");
		return OPRT_SEND_ERR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_register_recv_mgnt_callback(CONST BOOL_T enable, CONST WIFI_REV_MGNT_CB recv_cb)
{
	int iret = 0;

#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (iret < 0) {
		printf("WIFI recv mgnt failed");
		return OPRT_RECV_ERR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_rf_calibrated(VOID_T)/* unsupport */
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	printf("set rf calibrated is not support");
	return OPRT_COM_ERROR;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_set_lp_mode(CONST BOOL_T enable, CONST UCHAR_T dtim)/* unsupport */
{
	int iret = 0;

#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (iret < 0) {
		printf("WiFi Set lp mode failed");
		return OPRT_RECV_ERR;
	}

	return OPRT_OK;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_wifi_ioctl(WF_IOCTL_CMD_E cmd, VOID *args)
{
	return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_wifi_get_rtos_version_to_path(CHAR_T *buf, CHAR_T *path)
{
#ifdef DEBUG_VERSION
	printf("#####:  func=%s, line=%d\n", __func__, __LINE__);
#endif
	if (wifi_send_expand_cmd("xrlink:gversion:", path, buf) == WMG_STATUS_SUCCESS) {
		printf("Get RTOS Version Success\n");
	}
	return OPRT_OK;
}

/* tkl_wifi_rpbuf_send */
OPERATE_RET tkl_wifi_rpbuf_send(CONST UCHAR_T *rpbuf_name, CONST UINT_T rpbuf_len, VOID *args, CONST UINT_T data_len)
{
	int ret = -1;
	int ctrl_id = 0;  /* default id 0 */
	void *buf_addr = NULL;
	rpbuf_buffer_t *buffer = NULL;

	buffer = rpbuf_alloc_buffer(ctrl_id, rpbuf_name, rpbuf_len);
	if (!buffer) {
		printf("rpbuf_alloc_buffer for buffer %s (len: %zd) failed\n", rpbuf_name, rpbuf_len);
		ret = -1;
	}
	buf_addr = rpbuf_buffer_addr(buffer);
	rpbuf_set_sync_buffer(buffer, 1);
	memcpy(buf_addr, args, data_len);
	/* printf("buffer->buf_fd = %d\n", buffer->buf_fd); */
	ret = rpbuf_transmit_buffer(buffer, 0, data_len);
	if (ret < 0) {
		printf("transmit buffer %s error\n", rpbuf_name);
		goto free_buffer;
	}

free_buffer:
	rpbuf_free_buffer(buffer);
	return ret;
}

/* tkl_wifi_rpbuf_recv */
OPERATE_RET tkl_wifi_rpbuf_recv(CONST UCHAR_T *rpbuf_name, CONST UINT_T rpbuf_len, VOID *recv_buf, UINT_T *data_len)
{
	int ret = -1;
	unsigned int offset = 0;
	int ctrl_id = 0;  /* default id 0 */
	void *buf_addr = NULL;
	rpbuf_buffer_t *buffer = NULL;

	buffer = rpbuf_alloc_buffer(ctrl_id, rpbuf_name, rpbuf_len);
	if (!buffer) {
		printf("rpbuf_alloc_buffer for buffer %s (len: %zd) failed\n", rpbuf_name, rpbuf_len);
		ret = -1;
	}
	buf_addr = rpbuf_buffer_addr(buffer);
	/* printf("buffer->buf_fd = %d\n", buffer->buf_fd); */
	ret = rpbuf_receive_buffer(buffer, &offset, data_len, -1);
	if (ret < 0) {
		printf("receive buffer error %s, ret=%d\n", buffer->info.name, ret);
		goto free_buffer;
	}
	memcpy(recv_buf, buf_addr + offset, *data_len);
	for (int i = 0; i < *data_len; ++i) {
		printf(" 0x%x", ((char *)recv_buf)[i]);
	}
	sleep(2);
free_buffer:
	rpbuf_free_buffer(buffer);
	return ret;
}

//rpmsg
const char *ctrl_name = "e907_rproc@43030000";
const char *ept_name = "rpmsg_lp";
struct rpmsg_ept_instance {
	int ept_id;
	int ept_fd;
};

struct rpmsg_ept_instance *eptdev;

OPERATE_RET tkl_wifi_rpmsg_init(void)
{
	int ept_id = -1;
	int ret = 0;
	char ept_file_path[64];
	char send_buf[RPMSG_DATA_MAX_LEN] = { 0 };

	ret = rpmsg_alloc_ept(ctrl_name, ept_name);
	if (ret < 0) {
		printf("[lp_rpmsg]: rpmsg_alloc_ept for ept %s (%s) failed\n", ept_name, ctrl_name);
		ret = -1;
		goto out;
	}
	ept_id = ret;

	eptdev = malloc(sizeof(*eptdev));
	if (!eptdev) {
		printf("[lp_rpmsg]: malloc eptdev for rpmsg%d failed\r\n", ept_id);
		ret = -ENOMEM;
		goto free_ept;
	}
	memset(eptdev, 0, sizeof(*eptdev));
	eptdev->ept_id = ept_id;

	return 0;
free_ept:
	ret = rpmsg_free_ept(ctrl_name, ept_id);
	if (ret) {
		printf("[lp_rpmsg]: rpmsg_free_ept for /dev/rpmsg%d (%s) failed\n", ept_id, ctrl_name);
		ret = -1;
		goto out;
	}
	ret = 0;
	printf("[lp_rpmsg]: Delete rpmsg endpoint file: /dev/rpmsg%d\n", ept_id);
out:
	return ret;
}

OPERATE_RET tkl_wifi_rpmsg_send(VOID *args, CONST UINT_T data_len)
{
	int ret = -1;
	char ept_file_path[64] = { 0 };

	snprintf(ept_file_path, sizeof(ept_file_path),
		"/dev/rpmsg%d", eptdev->ept_id);
again:
	eptdev->ept_fd = open(ept_file_path, O_RDWR);
	if (eptdev->ept_fd < 0) {
		printf("[lp_rpmsg]: open %s failed, try again now\r\n", ept_file_path);
		usleep(30 * 1000);
		goto again;
	}

	ret = write(eptdev->ept_fd, args, data_len);
	if (ret < 0) {
		printf("[lp_rpmsg]: %s:%d write file error=%s\n", __FUNCTION__, __LINE__, strerror(errno));
		if (errno == EPIPE) {
			printf("[lp_rpmsg]: %s ept has been removed\r\n", ept_name);
		}
		return -1;
	}
	return ret;
}

OPERATE_RET tkl_wifi_rpmsg_recv(VOID *recv_buf, UINT_T data_len)
{
	int ret = -1;
	char ept_file_path[64] = { 0 };

	snprintf(ept_file_path, sizeof(ept_file_path),
		"/dev/rpmsg%d", eptdev->ept_id);
again:
	eptdev->ept_fd = open(ept_file_path, O_RDWR);
	if (eptdev->ept_fd < 0) {
		printf("[lp_rpmsg]: open %s failed\r\n", ept_file_path);
		usleep(30 * 1000);
		goto again;
	}

	ret = read(eptdev->ept_fd, recv_buf, data_len);
	if (ret < 0) {
		printf("[lp_rpmsg]: %s:%d read file error=%s\n", __FUNCTION__, __LINE__, strerror(errno));
		if (errno == EPIPE) {
			printf("[lp_rpmsg]: %s ept has been removed\r\n", ept_name);
		}
		return -1;
	}
	return ret;
}

OPERATE_RET tkl_wifi_rpmsg_deinit(void)
{
	if (eptdev == NULL) {
		printf("[lp_rpmsg]:%s eptdev is NULL\n", __FUNCTION__);
		return 0;
	}
	close(eptdev->ept_fd);
	int ret = -1;
	ret = rpmsg_free_ept(ctrl_name, eptdev->ept_id);
	if (ret) {
		printf("[lp_rpmsg]: rpmsg_free_ept for /dev/rpmsg%d (%s) failed\n", eptdev->ept_id, ctrl_name);
		ret = -1;
	}
	free(eptdev);
	printf("[lp_rpmsg]: Delete rpmsg endpoint file: /dev/rpmsg%d\n", eptdev->ept_id);
	return ret;
}

OPERATE_RET tkl_wifi_enter_ultra_standby(VOID)
{
	// system("echo 8 > /proc/sys/kernel/printk;");
	// system("echo N > /sys/module/printk/parameters/console_suspend;");
	// system("echo Y > /sys/module/kernel/parameters/initcall_debug;");
	// system("echo 1 > /sys/power/pm_debug_messages;");
	// system("echo mem > /sys/power/state;");

	// exit(0);

	system("echo 1 > /sys/class/ae350_standby/use_ultra_standby;");
	// system("echo 5000 > /sys/class/ae350_standby/time_to_wakeup_ms;");
	execlp("/sbin/poweroff", "poweroff", NULL);
	perror("Failed to execute poweroff");
	return 0;
}

OPERATE_RET tkl_wifi_enter_ultra_standby_timeout(int timeout_ms)
{
	char tmp_shell_buf[128] = { 0 };
	system("echo 1 > /sys/class/ae350_standby/use_ultra_standby;");
	// system("echo +300 >/sys/class/rtc/rtc0/wakealarm;");
	// system("echo 300000 > /sys/class/ae350_standby/time_to_wakeup_ms;");
	if (timeout_ms >= 0xffffffff) {
		printf("%s: timeout_ms max is 0xffffffff\n", __func__);
		timeout_ms =  0xffffffff;
	}
	sprintf(tmp_shell_buf, "echo %d > /sys/class/ae350_standby/time_to_wakeup_ms;", timeout_ms);
	system(tmp_shell_buf);
	execlp("/sbin/poweroff", "poweroff", NULL);
	perror("Failed to execute poweroff");
	return 0;
}

static UCHAR_T tkl_wmg_key_mgmt_to_security(wifi_sec key_mgmt)
{
/* for WF_AP_AUTH_MODE_E */
	if (key_mgmt & WIFI_SEC_WPA_PSK) {
		return WAAM_WPA_PSK;
	} else if (key_mgmt & WIFI_SEC_WPA2_PSK) {
		return WAAM_WPA2_PSK;
	} else if (key_mgmt & WIFI_SEC_WPA2_PSK_SHA256) {
		return WAAM_WPA_WPA2_PSK;
	} else if (key_mgmt & WIFI_SEC_WPA3_PSK) {
		return WAAM_WPA_WPA3_SAE;
	} else if (key_mgmt & WIFI_SEC_WEP) {
		return WAAM_WEP;
	} else {
		return WAAM_UNKNOWN;
	}
}
