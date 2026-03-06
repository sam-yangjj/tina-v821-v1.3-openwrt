/*******************************************************************
*  File: tuya_error_code.h
*  Author: auto generate by tuya code gen system
*  Date: 2021-07-16
*  Description:this file defined the error code of tuya IOT 
*  you can change it manully if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_ERROR_CODE_H
#define TUYA_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif


typedef int OPERATE_RET;


/****************************************************************************
            the error code marco define for module GLOBAL 
****************************************************************************/
#define OPRT_OK                                            (-0x0000)  //0, 执行成功
#define OPRT_COM_ERROR                                     (-0x0001)  //-1, 通用错误
#define OPRT_INVALID_PARM                                  (-0x0002)  //-2, 无效的入参
#define OPRT_MALLOC_FAILED                                 (-0x0003)  //-3, 内存分配失败
#define OPRT_NOT_SUPPORTED                                 (-0x0004)  //-4, 不支持
#define OPRT_NETWORK_ERROR                                 (-0x0005)  //-5, 网络错误
#define OPRT_NOT_FOUND                                     (-0x0006)  //-6, 没有找到对象
#define OPRT_CR_CJSON_ERR                                  (-0x0007)  //-7, 创建json对象失败
#define OPRT_CJSON_PARSE_ERR                               (-0x0008)  //-8, json解析失败
#define OPRT_CJSON_GET_ERR                                 (-0x0009)  //-9, 获取json对象失败
#define OPRT_CR_MUTEX_ERR                                  (-0x000a)  //-10, 创建信号量失败
#define OPRT_SOCK_ERR                                      (-0x000b)  //-11, 创建socket失败
#define OPRT_SET_SOCK_ERR                                  (-0x000c)  //-12, socket设置失败
#define OPRT_SOCK_CONN_ERR                                 (-0x000d)  //-13, socket连接失败
#define OPRT_SEND_ERR                                      (-0x000e)  //-14, 发送失败
#define OPRT_RECV_ERR                                      (-0x000f)  //-15, 接收失败
#define OPRT_RECV_DA_NOT_ENOUGH                            (-0x0010)  //-16, 接收数据不完整
#define OPRT_KVS_WR_FAIL                                   (-0x0011)  //-17, KV写失败
#define OPRT_KVS_RD_FAIL                                   (-0x0012)  //-18, KV读失败
#define OPRT_CRC32_FAILED                                  (-0x0013)  //-19, CRC校验失败
#define OPRT_TIMEOUT                                       (-0x0014)  //-20, 超时
#define OPRT_INIT_MORE_THAN_ONCE                           (-0x0015)  //-21, 初始化超过一次
#define OPRT_INDEX_OUT_OF_BOUND                            (-0x0016)  //-22, 索引越界
#define OPRT_RESOURCE_NOT_READY                            (-0x0017)  //-23, 资源未完善
#define OPRT_EXCEED_UPPER_LIMIT                            (-0x0018)  //-24, 超过上限
#define OPRT_FILE_NOT_FIND                                 (-0x0019)  //-25, 文件未找到
#define OPRT_GLOBAL_ERRCODE_MAX_CNT 26


/****************************************************************************
            the error code marco define for module BASE_OS_ADAPTER 
****************************************************************************/
#define OPRT_BASE_OS_ADAPTER_REG_NULL_ERROR                (-0x0100)  //-256, 系统适配注册失败
#define OPRT_BASE_OS_ADAPTER_INIT_MUTEX_ATTR_FAILED        (-0x0101)  //-257, 初始化同步属性失败
#define OPRT_BASE_OS_ADAPTER_SET_MUTEX_ATTR_FAILED         (-0x0102)  //-258, 设置同步属性失败
#define OPRT_BASE_OS_ADAPTER_DESTROY_MUTEX_ATTR_FAILED     (-0x0103)  //-259, 销毁同步属性失败
#define OPRT_BASE_OS_ADAPTER_INIT_MUTEX_FAILED             (-0x0104)  //-260, 初始化互斥量失败
#define OPRT_BASE_OS_ADAPTER_MUTEX_LOCK_FAILED             (-0x0105)  //-261, 互斥量加锁失败
#define OPRT_BASE_OS_ADAPTER_MUTEX_TRYLOCK_FAILED          (-0x0106)  //-262, 互斥量尝试加锁失败
#define OPRT_BASE_OS_ADAPTER_MUTEX_LOCK_BUSY               (-0x0107)  //-263, 互斥量忙
#define OPRT_BASE_OS_ADAPTER_MUTEX_UNLOCK_FAILED           (-0x0108)  //-264, 互斥量解锁失败
#define OPRT_BASE_OS_ADAPTER_MUTEX_RELEASE_FAILED          (-0x0109)  //-265, 互斥量释放失败
#define OPRT_BASE_OS_ADAPTER_CR_MUTEX_ERR                  (-0x010a)  //-266, 互斥量创建失败
#define OPRT_BASE_OS_ADAPTER_MEM_PARTITION_EMPTY           (-0x010b)  //-267, 内存分区空
#define OPRT_BASE_OS_ADAPTER_MEM_PARTITION_FULL            (-0x010c)  //-268, 内存分区满
#define OPRT_BASE_OS_ADAPTER_MEM_PARTITION_NOT_FOUND       (-0x010d)  //-269, 内存分区不存在
#define OPRT_BASE_OS_ADAPTER_INIT_SEM_FAILED               (-0x010e)  //-270, 初始化信号量失败
#define OPRT_BASE_OS_ADAPTER_WAIT_SEM_FAILED               (-0x010f)  //-271, 等待信号量失败
#define OPRT_BASE_OS_ADAPTER_POST_SEM_FAILED               (-0x0110)  //-272, 释放信号量失败
#define OPRT_BASE_OS_ADAPTER_THRD_STA_UNVALID              (-0x0111)  //-273, 线程状态非法
#define OPRT_BASE_OS_ADAPTER_THRD_CR_FAILED                (-0x0112)  //-274, 线程创建失败
#define OPRT_BASE_OS_ADAPTER_THRD_JOIN_FAILED              (-0x0113)  //-275, 线程JOIN函数调用失败
#define OPRT_BASE_OS_ADAPTER_THRD_SELF_CAN_NOT_JOIN        (-0x0114)  //-276, 自身线程不能调用JOIN函数
#define OPRT_BASE_OS_ADAPTER_ERRCODE_MAX_CNT 21


/****************************************************************************
            the error code marco define for module BASE_UTILITIES 
****************************************************************************/
#define OPRT_BASE_UTILITIES_PARTITION_EMPTY                (-0x0200)  //-512, 无空闲链表
#define OPRT_BASE_UTILITIES_PARTITION_FULL                 (-0x0201)  //-513, 链表已满
#define OPRT_BASE_UTILITIES_PARTITION_NOT_FOUND            (-0x0202)  //-514, 链表未遍历到
#define OPRT_BASE_UTILITIES_ERRCODE_MAX_CNT 3


/****************************************************************************
            the error code marco define for module BASE_SECURITY 
****************************************************************************/
#define OPRT_BASE_SECURITY_CRC32_FAILED                    (-0x0300)  //-768, CRC32错误
#define OPRT_BASE_SECURITY_ERRCODE_MAX_CNT 1


/****************************************************************************
            the error code marco define for module BASE_LOG_MNG 
****************************************************************************/
#define OPRT_BASE_LOG_MNG_DONOT_FOUND_MODULE               (-0x0400)  //-1024, 未发现log模块
#define OPRT_BASE_LOG_MNG_PRINT_LOG_LEVEL_HIGHER           (-0x0401)  //-1025, log级别低
#define OPRT_BASE_LOG_MNG_FORMAT_STRING_FAILED             (-0x0402)  //-1026, log字符串格式化失败
#define OPRT_BASE_LOG_MNG_LOG_SEQ_OPEN_FILE_FAIL           (-0x0403)  //-1027, 打开日志序文件失败
#define OPRT_BASE_LOG_MNG_LOG_SEQ_WRITE_FILE_FAIL          (-0x0404)  //-1028, 写日志序文件失败
#define OPRT_BASE_LOG_MNG_LOG_SEQ_FILE_FULL                (-0x0405)  //-1029, 日志序文件满
#define OPRT_BASE_LOG_MNG_LOG_SEQ_FILE_NOT_EXIST           (-0x0406)  //-1030, 日志序文件不存在
#define OPRT_BASE_LOG_MNG_LOG_SEQ_NAME_INVALIDE            (-0x0407)  //-1031, 日志序名称无效
#define OPRT_BASE_LOG_MNG_LOG_SEQ_CREATE_FAIL              (-0x0408)  //-1032, 日志序创建失败
#define OPRT_BASE_LOG_MNG_ERRCODE_MAX_CNT 9


/****************************************************************************
            the error code marco define for module BASE_WORKQ 
****************************************************************************/
#define OPRT_BASE_WORKQ_ERRCODE_MAX_CNT 0


/****************************************************************************
            the error code marco define for module BASE_TIMEQ 
****************************************************************************/
#define OPRT_BASE_TIMEQ_TIMERID_EXIST                      (-0x0600)  //-1536, 定时器ID已存在
#define OPRT_BASE_TIMEQ_TIMERID_NOT_FOUND                  (-0x0601)  //-1537, 未找到指定定时器ID
#define OPRT_BASE_TIMEQ_TIMERID_UNVALID                    (-0x0602)  //-1538, 定时器ID非法
#define OPRT_BASE_TIMEQ_GET_IDLE_TIMERID_ERROR             (-0x0603)  //-1539, 获取空闲定时器ID错误
#define OPRT_BASE_TIMEQ_ERRCODE_MAX_CNT 4


/****************************************************************************
            the error code marco define for module BASE_MSGQ 
****************************************************************************/
#define OPRT_BASE_MSGQ_NOT_FOUND                           (-0x0700)  //-1792, 消息未找到
#define OPRT_BASE_MSGQ_LIST_EMPTY                          (-0x0701)  //-1793, 列表为空
#define OPRT_BASE_MSGQ_LIST_FULL                           (-0x0702)  //-1794, 列表为满
#define OPRT_BASE_MSGQ_ERRCODE_MAX_CNT 3


/****************************************************************************
            the error code marco define for module MID_HTTP 
****************************************************************************/
#define OPRT_MID_HTTP_BUF_NOT_ENOUGH                       (-0x0800)  //-2048, 缓冲区长度不足
#define OPRT_MID_HTTP_URL_PARAM_OUT_LIMIT                  (-0x0801)  //-2049, URL长度超出限制
#define OPRT_MID_HTTP_OS_ERROR                             (-0x0802)  //-2050, 系统错误
#define OPRT_MID_HTTP_PR_REQ_ERROR                         (-0x0803)  //-2051, 准备请求错误
#define OPRT_MID_HTTP_SD_REQ_ERROR                         (-0x0804)  //-2052, 发送请求错误
#define OPRT_MID_HTTP_RD_ERROR                             (-0x0805)  //-2053, 读取错误
#define OPRT_MID_HTTP_AD_HD_ERROR                          (-0x0806)  //-2054, 添加头错误
#define OPRT_MID_HTTP_GET_RESP_ERROR                       (-0x0807)  //-2055, 获取应答错误
#define OPRT_MID_HTTP_AES_INIT_ERR                         (-0x0808)  //-2056, AES初始化错误
#define OPRT_MID_HTTP_AES_OPEN_ERR                         (-0x0809)  //-2057, AES打开错误
#define OPRT_MID_HTTP_AES_SET_KEY_ERR                      (-0x080a)  //-2058, AES设置KEY错误
#define OPRT_MID_HTTP_AES_ENCRYPT_ERR                      (-0x080b)  //-2059, AES加密错误
#define OPRT_MID_HTTP_CR_HTTP_URL_H_ERR                    (-0x080c)  //-2060, 创建HTTP URL头错误
#define OPRT_MID_HTTP_HTTPS_HANDLE_FAIL                    (-0x080d)  //-2061, HTTPS句柄错误
#define OPRT_MID_HTTP_HTTPS_RESP_UNVALID                   (-0x080e)  //-2062, HTTPS无效应答
#define OPRT_MID_HTTP_NO_SUPPORT_RANGE                     (-0x080f)  //-2063, 不支持断点续传
#define OPRT_MID_HTTP_ERRCODE_MAX_CNT 16


/****************************************************************************
            the error code marco define for module MID_MQTT 
****************************************************************************/
#define OPRT_MID_MQTT_DEF_ERR                              (-0x0900)  //-2304, 定义失败
#define OPRT_MID_MQTT_INVALID_PARM                         (-0x0901)  //-2305, 参数无效
#define OPRT_MID_MQTT_MALLOC_FAILED                        (-0x0902)  //-2306, 内存申请失败
#define OPRT_MID_MQTT_DNS_PARSED_FAILED                    (-0x0903)  //-2307, DNS解析失败
#define OPRT_MID_MQTT_SOCK_CREAT_FAILED                    (-0x0904)  //-2308, socket创建失败
#define OPRT_MID_MQTT_SOCK_SET_FAILED                      (-0x0905)  //-2309, socket set失败
#define OPRT_MID_MQTT_TCP_CONNECD_FAILED                   (-0x0906)  //-2310, tcp连接失败
#define OPRT_MID_MQTT_TCP_TLS_CONNECD_FAILED               (-0x0907)  //-2311, tcp tls连接失败
#define OPRT_MID_MQTT_PACK_SEND_FAILED                     (-0x0908)  //-2312, 包发送失败
#define OPRT_MID_MQTT_RECV_DATA_FORMAT_WRONG               (-0x0909)  //-2313, 接收数据格式错误
#define OPRT_MID_MQTT_MSGID_NOT_MATCH                      (-0x090a)  //-2314, 接收数据msgid未找到
#define OPRT_MID_MQTT_START_TM_MSG_ERR                     (-0x090b)  //-2315, 开始事件msg错误
#define OPRT_MID_MQTT_OVER_MAX_MESSAGE_LEN                 (-0x090c)  //-2316, 超过消息最大长度
#define OPRT_MID_MQTT_PING_SEND_ERR                        (-0x090d)  //-2317, ping发送失败
#define OPRT_MID_MQTT_PUBLISH_TIMEOUT                      (-0x090e)  //-2318, 发布超时
#define OPRT_MID_MQTT_ERRCODE_MAX_CNT 15


/****************************************************************************
            the error code marco define for module MID_TLS 
****************************************************************************/
#define OPRT_MID_TLS_NET_SOCKET_ERROR                      (-0x0a00)  //-2560, Failed to open a socket
#define OPRT_MID_TLS_NET_CONNECT_ERROR                     (-0x0a01)  //-2561, The connection to the given server / port failed.
#define OPRT_MID_TLS_UNKNOWN_HOST_ERROR                    (-0x0a02)  //-2562, Failed to get an IP address for the given hostname.
#define OPRT_MID_TLS_CONNECTION_ERROR                      (-0x0a03)  //-2563, TLS连接失败
#define OPRT_MID_TLS_DRBG_ENTROPY_ERROR                    (-0x0a04)  //-2564, mbedtls随机种子生成失败
#define OPRT_MID_TLS_X509_ROOT_CRT_PARSE_ERROR             (-0x0a05)  //-2565, X509根证书解析失败
#define OPRT_MID_TLS_X509_DEVICE_CRT_PARSE_ERROR           (-0x0a06)  //-2566, X509设备证书解析失败
#define OPRT_MID_TLS_CTR_DRBG_ENTROPY_SOURCE_ERROR         (-0x0a07)  //-2567, The entropy source failed
#define OPRT_MID_TLS_PK_PRIVATE_KEY_PARSE_ERROR            (-0x0a08)  //-2568, 秘钥解析失败
#define OPRT_MID_TLS_ERRCODE_MAX_CNT 9


/****************************************************************************
            the error code marco define for module SVC_WIFI_NETCFG 
****************************************************************************/
#define OPRT_SVC_WIFI_NETCFG_RECV_CONTINUE                 (-0x0b00)  //-2816, 继续接收配网包
#define OPRT_SVC_WIFI_NETCFG_ERRCODE_MAX_CNT 1


/****************************************************************************
            the error code marco define for module SVC_ONLINE_LOG 
****************************************************************************/
#define OPRT_SVC_ONLINE_LOG_ERRCODE_MAX_CNT 0


/****************************************************************************
            the error code marco define for module SVC_MF_TEST 
****************************************************************************/
#define OPRT_SVC_MF_TEST_UPDATE_DATA_LEN_EXECED            (-0x0d00)  //-3328, 升级数据长度超过处理上限
#define OPRT_SVC_MF_TEST_UPDATE_CRC_ERROR                  (-0x0d01)  //-3329, 升级crc校验失败
#define OPRT_SVC_MF_TEST_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module SVC_DP 
****************************************************************************/
#define OPRT_SVC_DP_ALREADY_PROCESS                        (-0x0e00)  //-3584, DP已经处理
#define OPRT_SVC_DP_ID_NOT_FOUND                           (-0x0e01)  //-3585, DP ID没有发现
#define OPRT_SVC_DP_TP_NOT_MATCH                           (-0x0e02)  //-3586, DP 类型未匹配
#define OPRT_SVC_DP_DEVICE_NOT_BINDED                      (-0x0e03)  //-3587, 设备未绑定
#define OPRT_SVC_DP_TYPE_PROP_ILLEGAL                      (-0x0e04)  //-3588, 类型属性不合法
#define OPRT_SVC_DP_NW_INVALID                             (-0x0e05)  //-3589, 网络无效
#define OPRT_SVC_DP_SECURITY_VERIFY_ERR                    (-0x0e06)  //-3590, 安全校验失败
#define OPRT_SVC_DP_REPORT_FINISH                          (-0x0e07)  //-3591, DP上报已结束
#define OPRT_SVC_DP_ERRCODE_MAX_CNT 8


/****************************************************************************
            the error code marco define for module BASE_EVENT 
****************************************************************************/
#define OPRT_BASE_EVENT_INVALID_EVENT_NAME                 (-0x0f00)  //-3840, 无效的事件名
#define OPRT_BASE_EVENT_INVALID_EVENT_DESC                 (-0x0f01)  //-3841, 无效的事件描述
#define OPRT_BASE_EVENT_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module SVC_TIMER_TASK 
****************************************************************************/
#define OPRT_SVC_TIMER_TASK_LOAD_INVALID_CJSON             (-0x1000)  //-4096, K/V中保存的数据JSON格式错误
#define OPRT_SVC_TIMER_TASK_LOAD_INVALID_DATA              (-0x1001)  //-4097, K/V中保存的数据缺少cnt字段
#define OPRT_SVC_TIMER_TASK_UPDATE_LAST_FETCH_INVALID      (-0x1002)  //-4098, 云端返回的数据缺少lastFetchTime字段
#define OPRT_SVC_TIMER_TASK_UPDATE_TIMER_CNT_INVALID       (-0x1003)  //-4099, 云端返回的数据缺少count字段
#define OPRT_SVC_TIMER_TASK_UPDATE_TIMER_CNT_EXCEED        (-0x1004)  //-4100, 云端返回的定时任务数量超过30个
#define OPRT_SVC_TIMER_TASK_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module SVC_HTTP 
****************************************************************************/
#define OPRT_SVC_HTTP_NOT_ENCRYPT_RET                      (-0x1100)  //-4352, 结果未加密
#define OPRT_SVC_HTTP_FILL_URL_H_ERR                       (-0x1101)  //-4353, 构造header错误
#define OPRT_SVC_HTTP_FILL_URL_FULL_ERR                    (-0x1102)  //-4354, 构造整个URL错误
#define OPRT_SVC_HTTP_FILL_DATA_ERR                        (-0x1103)  //-4355, 构造data错误
#define OPRT_SVC_HTTP_URL_CFG_AI_SPEAKER_ERR               (-0x1104)  //-4356, 音响配置失败
#define OPRT_SVC_HTTP_URL_CFG_URL_ERR                      (-0x1105)  //-4357, httpUrl/mqttUrl字段缺失
#define OPRT_SVC_HTTP_URL_CFG_URL2IP_ERR                   (-0x1106)  //-4358, httpUrl/mqttUrl解析错误
#define OPRT_SVC_HTTP_URL_CFG_URL2IP_SELF_ERR              (-0x1107)  //-4359, httpsSelfUrl/mqttsSelfUrl解析错误
#define OPRT_SVC_HTTP_URL_CFG_URL2IP_VERIFY_ERR            (-0x1108)  //-4360, httpsVerifyUrl/mqttsVerifyUrl解析错误
#define OPRT_SVC_HTTP_URL_CFG_URL2IP_PSK_ERR               (-0x1109)  //-4361, httpsPSKUrl/mqttsPSKUrl解析错误
#define OPRT_SVC_HTTP_RECV_ERR                             (-0x110a)  //-4362, 接收数据错误
#define OPRT_SVC_HTTP_RECV_DA_NOT_ENOUGH                   (-0x110b)  //-4363, 接收数据不足
#define OPRT_SVC_HTTP_API_VERIFY_FAILED                    (-0x110c)  //-4364, 数据校验错误
#define OPRT_SVC_HTTP_GW_NOT_EXIST                         (-0x110d)  //-4365, 网关信息不存在
#define OPRT_SVC_HTTP_API_TOKEN_EXPIRE                     (-0x110e)  //-4366, TOKEN过期
#define OPRT_SVC_HTTP_DEV_RESET_FACTORY                    (-0x110f)  //-4367, 设备需要恢复出厂
#define OPRT_SVC_HTTP_DEV_NEED_REGISTER                    (-0x1110)  //-4368, 设备未注册
#define OPRT_SVC_HTTP_ORDER_EXPIRE                         (-0x1111)  //-4369, 订单已过期
#define OPRT_SVC_HTTP_NOT_EXISTS                           (-0x1112)  //-4370, 不存在
#define OPRT_SVC_HTTP_SIGNATURE_ERROR                      (-0x1113)  //-4371, 签名错误
#define OPRT_SVC_HTTP_API_VERSION_WRONG                    (-0x1114)  //-4372, API版本错误
#define OPRT_SVC_HTTP_DEVICE_REMOVED                       (-0x1115)  //-4373, 设备已移除
#define OPRT_SVC_HTTP_DEV_ALREADY_BIND                     (-0x1116)  //-4374, 设备已经绑定
#define OPRT_SVC_HTTP_REMOTE_API_RUN_UNKNOW_FAILED         (-0x1117)  //-4375, 无法识别API
#define OPRT_SVC_HTTP_FORMAT_STRING_FAILED                 (-0x1118)  //-4376, 字符串格式化错误
#define OPRT_SVC_HTTP_API_DECODE_FAILED                    (-0x1119)  //-4377, 数据解密失败
#define OPRT_SVC_HTTP_SERV_VRFY_FAIL                       (-0x111a)  //-4378, 服务端校验失败
#define OPRT_SVC_HTTP_ERRCODE_MAX_CNT 27


/****************************************************************************
            the error code marco define for module SVC_LAN 
****************************************************************************/
#define OPRT_SVC_LAN_SOCKET_FAULT                          (-0x1500)  //-5376, socket错误
#define OPRT_SVC_LAN_SEND_ERR                              (-0x1501)  //-5377, socket发送错误
#define OPRT_SVC_LAN_NO_CLIENT_CONNECTED                   (-0x1502)  //-5378, 没有可以上报的局域网设备连接
#define OPRT_SVC_LAN_ERRCODE_MAX_CNT 3


/****************************************************************************
            the error code marco define for module SVC_LAN_LINKAGE 
****************************************************************************/
#define OPRT_SVC_LAN_LINKAGE_SOCK_CREAT_ERR                (-0x1600)  //-5632, socket创建失败
#define OPRT_SVC_LAN_LINKAGE_SET_SOCK_ERR                  (-0x1601)  //-5633, socket set失败
#define OPRT_SVC_LAN_LINKAGE_SOCK_CONN_ERR                 (-0x1602)  //-5634, socket连接失败
#define OPRT_SVC_LAN_LINKAGE_SEND_ERR                      (-0x1603)  //-5635, 发送失败
#define OPRT_SVC_LAN_LINKAGE_RECV_ERR                      (-0x1604)  //-5636, 接收失败
#define OPRT_SVC_LAN_LINKAGE_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module SVC_MQTT 
****************************************************************************/
#define OPRT_SVC_MQTT_CMD_NOT_EXEC                         (-0x1700)  //-5888, 命令未执行
#define OPRT_SVC_MQTT_CMD_OUT_OF_TIME                      (-0x1701)  //-5889, 命令未在规定时间内执行
#define OPRT_SVC_MQTT_GW_MQ_OFFLILNE                       (-0x1702)  //-5890, MQTT离线
#define OPRT_SVC_MQTT_ERRCODE_MAX_CNT 3


/****************************************************************************
            the error code marco define for module SVC_PEGASUS 
****************************************************************************/
#define OPRT_SVC_PEGASUS_DECODE_FAILED                     (-0x1800)  //-6144, 解码失败
#define OPRT_SVC_PEGASUS_DONOT_FOUND_MODULE                (-0x1801)  //-6145, 模块未找到
#define OPRT_SVC_PEGASUS_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module SVC_UPGRADE 
****************************************************************************/
#define OPRT_SVC_UPGRADE_APP_NOT_READY                     (-0x1900)  //-6400, 应用尚未就绪
#define OPRT_SVC_UPGRADE_NO_VALID_FIRMWARE                 (-0x1901)  //-6401, 升级信息字段校验失败
#define OPRT_SVC_UPGRADE_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module SVC_API_IOT 
****************************************************************************/
#define OPRT_SVC_API_IOT_DISCONNECTED_WITH_ROUTER          (-0x1a00)  //-6656, 路由器断开
#define OPRT_SVC_API_IOT_DEV_NOT_BIND                      (-0x1a01)  //-6657, 设备未绑定
#define OPRT_SVC_API_IOT_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module SVC_DEVOS 
****************************************************************************/
#define OPRT_SVC_DEVOS_NOT_EXISTS                          (-0x1c00)  //-7168, 不存在
#define OPRT_SVC_DEVOS_SCMA_INVALID                        (-0x1c01)  //-7169, SCMA无效
#define OPRT_SVC_DEVOS_DEV_DP_CNT_INVALID                  (-0x1c02)  //-7170, 设备DP数量无效
#define OPRT_SVC_DEVOS_NO_AUTHENTICATION                   (-0x1c03)  //-7171, 无授权
#define OPRT_SVC_DEVOS_ROUTER_NOT_FIND                     (-0x1c04)  //-7172, 路由器未找到
#define OPRT_SVC_DEVOS_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module BASE_DB 
****************************************************************************/
#define OPRT_BASE_DB_FLASH_NOT_ENOUGH_PAGE                 (-0x1d00)  //-7424, flash页不够
#define OPRT_BASE_DB_ERRCODE_MAX_CNT 1


/****************************************************************************
            the error code marco define for module LINK_CORE 
****************************************************************************/
#define OPRT_LINK_CORE_NET_SOCKET_ERROR                    (-0x1e00)  //-7680, Failed to open a socket
#define OPRT_LINK_CORE_NET_CONNECT_ERROR                   (-0x1e01)  //-7681, The connection to the given server / port failed.
#define OPRT_LINK_CORE_UNKNOWN_HOST_ERROR                  (-0x1e02)  //-7682, Failed to get an IP address for the given hostname.
#define OPRT_LINK_CORE_TLS_CONNECTION_ERROR                (-0x1e03)  //-7683, TLS连接失败
#define OPRT_LINK_CORE_DRBG_ENTROPY_ERROR                  (-0x1e04)  //-7684, mbedtls随机种子生成失败
#define OPRT_LINK_CORE_X509_ROOT_CRT_PARSE_ERROR           (-0x1e05)  //-7685, X509根证书解析失败
#define OPRT_LINK_CORE_X509_DEVICE_CRT_PARSE_ERROR         (-0x1e06)  //-7686, X509设备证书解析失败
#define OPRT_LINK_CORE_PK_PRIVATE_KEY_PARSE_ERROR          (-0x1e07)  //-7687, 秘钥解析失败
#define OPRT_LINK_CORE_HTTP_CLIENT_HEADER_ERROR            (-0x1e08)  //-7688, HTTP头初始化失败
#define OPRT_LINK_CORE_HTTP_CLIENT_SEND_ERROR              (-0x1e09)  //-7689, HTTP请求发送失败
#define OPRT_LINK_CORE_HTTP_RESPONSE_BUFFER_EMPTY          (-0x1e0a)  //-7690, HTTPb buffer为空
#define OPRT_LINK_CORE_HTTP_GW_NOT_EXIST                   (-0x1e0b)  //-7691, 网关不存在，可能设备已被删除
#define OPRT_LINK_CORE_ERRCODE_MAX_CNT 12


/****************************************************************************
            the error code marco define for module SVC_BT 
****************************************************************************/
#define OPRT_SVC_BT_API_TRSMITR_CONTINUE                   (-0x1f00)  //-7936, 传输未结束
#define OPRT_SVC_BT_API_TRSMITR_ERROR                      (-0x1f01)  //-7937, 传输错误
#define OPRT_SVC_BT_NETCFG_ERROR_ACK                       (-0x1f02)  //-7938, bt命令出错，给app发送ack
#define OPRT_SVC_BT_ERRCODE_MAX_CNT 3


/****************************************************************************
            the error code marco define for module SVC_NETMGR 
****************************************************************************/
#define OPRT_SVC_NETMGR_NEED_FACTORY_RESET                 (-0x2000)  //-8192, 网络初始化配置校验失败，需要恢复出厂设置
#define OPRT_SVC_NETMGR_ERRCODE_MAX_CNT 1


/****************************************************************************
            the error code marco define for module OS_ADAPTER_MUTEX 
****************************************************************************/
#define OPRT_OS_ADAPTER_MUTEX_ERRCODE                      (-0x6500)  //-25856, mutex错误码起始
#define OPRT_OS_ADAPTER_MUTEX_CREAT_FAILED                 (-0x6501)  //-25857, 创建失败
#define OPRT_OS_ADAPTER_MUTEX_LOCK_FAILED                  (-0x6502)  //-25858, lock失败
#define OPRT_OS_ADAPTER_MUTEX_UNLOCK_FAILED                (-0x6503)  //-25859, unlock失败
#define OPRT_OS_ADAPTER_MUTEX_RELEASE_FAILED               (-0x6504)  //-25860, 释放失败
#define OPRT_OS_ADAPTER_MUTEX_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module OS_ADAPTER_SEM 
****************************************************************************/
#define OPRT_OS_ADAPTER_SEM_ERRCODE                        (-0x6600)  //-26112, semaphore错误码起始
#define OPRT_OS_ADAPTER_SEM_CREAT_FAILED                   (-0x6601)  //-26113, 创建失败
#define OPRT_OS_ADAPTER_SEM_WAIT_FAILED                    (-0x6602)  //-26114, wait失败
#define OPRT_OS_ADAPTER_SEM_POST_FAILED                    (-0x6603)  //-26115, post失败
#define OPRT_OS_ADAPTER_SEM_RELEASE_FAILED                 (-0x6604)  //-26116, 释放失败
#define OPRT_OS_ADAPTER_SEM_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module OS_ADAPTER_QUEUE 
****************************************************************************/
#define OPRT_OS_ADAPTER_QUEUE_ERRCODE_QUEUE                (-0x6700)  //-26368, queue错误码起始
#define OPRT_OS_ADAPTER_QUEUE_CREAT_FAILED                 (-0x6701)  //-26369, 创建失败
#define OPRT_OS_ADAPTER_QUEUE_SEND_FAIL                    (-0x6702)  //-26370, send失败
#define OPRT_OS_ADAPTER_QUEUE_RECV_FAIL                    (-0x6703)  //-26371, recv失败
#define OPRT_OS_ADAPTER_QUEUE_ERRCODE_MAX_CNT 4


/****************************************************************************
            the error code marco define for module OS_ADAPTER_THRD 
****************************************************************************/
#define OPRT_OS_ADAPTER_THRD_ERRCODE                       (-0x6800)  //-26624, thread错误码起始
#define OPRT_OS_ADAPTER_THRD_CREAT_FAILED                  (-0x6801)  //-26625, 创建失败
#define OPRT_OS_ADAPTER_THRD_RELEASE_FAILED                (-0x6802)  //-26626, 释放失败
#define OPRT_OS_ADAPTER_THRD_JUDGE_SELF_FAILED             (-0x6803)  //-26627, 判断是否self失败
#define OPRT_OS_ADAPTER_THRD_ERRCODE_MAX_CNT 4


/****************************************************************************
            the error code marco define for module OS_ADAPTER 
****************************************************************************/
#define OPRT_OS_ADAPTER_ERRCODE_WIFI                       (-0x6a00)  //-27136, Wi-Fi错误码起始
#define OPRT_OS_ADAPTER_COM_ERROR                          (-0x6a01)  //-27137, 通用异常
#define OPRT_OS_ADAPTER_INVALID_PARM                       (-0x6a02)  //-27138, 参数非法
#define OPRT_OS_ADAPTER_MALLOC_FAILED                      (-0x6a03)  //-27139, 内存分配失败
#define OPRT_OS_ADAPTER_NOT_SUPPORTED                      (-0x6a04)  //-27140, 不支持操作
#define OPRT_OS_ADAPTER_NETWORK_ERROR                      (-0x6a05)  //-27141, 网络错误
#define OPRT_OS_ADAPTER_AP_NOT_FOUND                       (-0x6a06)  //-27142, AP没有找到
#define OPRT_OS_ADAPTER_AP_SCAN_FAILED                     (-0x6a07)  //-27143, AP扫描失败
#define OPRT_OS_ADAPTER_AP_RELEASE_FAILED                  (-0x6a08)  //-27144, AP释放失败
#define OPRT_OS_ADAPTER_CHAN_SET_FAILED                    (-0x6a09)  //-27145, 信道设置失败
#define OPRT_OS_ADAPTER_CHAN_GET_FAILED                    (-0x6a0a)  //-27146, 信道获取失败
#define OPRT_OS_ADAPTER_IP_GET_FAILED                      (-0x6a0b)  //-27147, IP获取失败
#define OPRT_OS_ADAPTER_MAC_SET_FAILED                     (-0x6a0c)  //-27148, MAC设置失败
#define OPRT_OS_ADAPTER_MAC_GET_FAILED                     (-0x6a0d)  //-27149, MAC获取失败
#define OPRT_OS_ADAPTER_WORKMODE_SET_FAILED                (-0x6a0e)  //-27150, 工作模式设置失败
#define OPRT_OS_ADAPTER_WORKMODE_GET_FAILED                (-0x6a0f)  //-27151, 工作模式获取失败
#define OPRT_OS_ADAPTER_SNIFFER_SET_FAILED                 (-0x6a10)  //-27152, SNIFFER设置失败
#define OPRT_OS_ADAPTER_AP_START_FAILED                    (-0x6a11)  //-27153, AP启动失败
#define OPRT_OS_ADAPTER_AP_STOP_FAILED                     (-0x6a12)  //-27154, AP停止失败
#define OPRT_OS_ADAPTER_APINFO_GET_FAILED                  (-0x6a13)  //-27155, AP信息获取失败
#define OPRT_OS_ADAPTER_FAST_CONN_FAILED                   (-0x6a14)  //-27156, 快连失败
#define OPRT_OS_ADAPTER_CONN_FAILED                        (-0x6a15)  //-27157, 连接失败
#define OPRT_OS_ADAPTER_DISCONN_FAILED                     (-0x6a16)  //-27158, 断开失败
#define OPRT_OS_ADAPTER_RSSI_GET_FAILED                    (-0x6a17)  //-27159, RSSI获取失败
#define OPRT_OS_ADAPTER_BSSID_GET_FAILED                   (-0x6a18)  //-27160, BSSID获取失败
#define OPRT_OS_ADAPTER_STAT_GET_FAILED                    (-0x6a19)  //-27161, 状态获取失败
#define OPRT_OS_ADAPTER_CCODE_SET_FAILED                   (-0x6a1a)  //-27162, CCODE设置失败
#define OPRT_OS_ADAPTER_MGNT_SEND_FAILED                   (-0x6a1b)  //-27163, 管理包发送失败
#define OPRT_OS_ADAPTER_MGNT_REG_FAILED                    (-0x6a1c)  //-27164, 管理包回调注册失败
#define OPRT_OS_ADAPTER_WF_LPMODE_SET_FAILED               (-0x6a1d)  //-27165, Wi-Fi低功耗设置失败
#define OPRT_OS_ADAPTER_CPU_LPMODE_SET_FAILED              (-0x6a1e)  //-27166, CPU低功耗模式设置失败
#define OPRT_OS_ADAPTER_ERRCODE_MAX_CNT 31


/****************************************************************************
            the error code marco define for module OS_ADAPTER_FLASH 
****************************************************************************/
#define OPRT_OS_ADAPTER_FLASH_ERRCODE                      (-0x6b00)  //-27392, flash错误码起始
#define OPRT_OS_ADAPTER_FLASH_READ_FAILED                  (-0x6b01)  //-27393, 读取flash失败
#define OPRT_OS_ADAPTER_FLASH_WRITE_FAILED                 (-0x6b02)  //-27394, 写入flash失败
#define OPRT_OS_ADAPTER_FLASH_ERASE_FAILED                 (-0x6b03)  //-27395, 擦除flash失败

#define OPRT_OS_ADAPTER_FLASH_ADDR_INVALID                 (-0x6b04) // -27396, flash 地址错误
#define OPRT_OS_ADAPTER_FLASH_LOCKED                       (-0x6b05) // -27397, flash 未解锁
#define OPRT_OS_ADAPTER_NOT_ERASE                          (-0x6b06) // -27398, flash 未擦除
#define OPRT_OS_ADAPTER_TIMEOUT                            (-0x6b07) // -27399, flash 操作超时
#define OPRT_OS_ADAPTER_DATA_CHACK_FAIL                    (-0x6b08) // -27400, flash 数据校验错误

#define OPRT_OS_ADAPTER_FLASH_ERRCODE_MAX_CNT 9



/****************************************************************************
            the error code marco define for module OS_ADAPTER_OTA 
****************************************************************************/
#define OPRT_OS_ADAPTER_OTA_ERRCODE                        (-0x6c00)  //-27648, OTA错误码起始
#define OPRT_OS_ADAPTER_OTA_START_INFORM_FAILED            (-0x6c01)  //-27649, 升级启动通知失败
#define OPRT_OS_ADAPTER_OTA_PKT_SIZE_FAILED                (-0x6c02)  //-27650, 升级包尺寸非法
#define OPRT_OS_ADAPTER_OTA_PROCESS_FAILED                 (-0x6c03)  //-27651, 升级包下载写入失败
#define OPRT_OS_ADAPTER_OTA_VERIFY_FAILED                  (-0x6c04)  //-27652, 升级包校验失败
#define OPRT_OS_ADAPTER_OTA_END_INFORM_FAILED              (-0x6c05)  //-27653, 升级结束通知失败
#define OPRT_OS_ADAPTER_OTA_ERRCODE_MAX_CNT 6


/****************************************************************************
            the error code marco define for module OS_ADAPTER_WD 
****************************************************************************/
#define OPRT_OS_ADAPTER_WD_ERRCODE                         (-0x6d00)  //-27904, watch dog错误码起始
#define OPRT_OS_ADAPTER_WD_INIT_FAILED                     (-0x6d01)  //-27905, watch dog初始化失败
#define OPRT_OS_ADAPTER_WD_ERRCODE_MAX_CNT 2


/****************************************************************************
            the error code marco define for module OS_ADAPTER_GPIO 
****************************************************************************/
#define OPRT_OS_ADAPTER_GPIO_ERRCODE                       (-0x6e00)  //-28160, gpio错误码起始
#define OPRT_OS_ADAPTER_GPIO_INOUT_SET_FAILED              (-0x6e01)  //-28161, GPIO INOUT设置失败
#define OPRT_OS_ADAPTER_GPIO_MODE_SET_FAILED               (-0x6e02)  //-28162, GPIO 模式设置失败
#define OPRT_OS_ADAPTER_GPIO_WRITE_FAILED                  (-0x6e03)  //-28163, GPIO 写入失败
#define OPRT_OS_ADAPTER_GPIO_IRQ_INIT_FAILED               (-0x6e04)  //-28164, GPIO 中断初始化失败
#define OPRT_OS_ADAPTER_GPIO_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module OS_ADAPTER_UART 
****************************************************************************/
#define OPRT_OS_ADAPTER_UART_ERRCODE                       (-0x6f00)  //-28416, gpio错误码起始
#define OPRT_OS_ADAPTER_UART_INIT_FAILED                   (-0x6f01)  //-28417, UART初始化失败
#define OPRT_OS_ADAPTER_UART_DEINIT_FAILED                 (-0x6f02)  //-28418, UART释放失败
#define OPRT_OS_ADAPTER_UART_SEND_FAILED                   (-0x6f03)  //-28419, UART发送失败
#define OPRT_OS_ADAPTER_UART_READ_FAILED                   (-0x6f04)  //-28420, UART接收失败
#define OPRT_OS_ADAPTER_UART_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module OS_ADAPTER_I2C 
****************************************************************************/
#define OPRT_OS_ADAPTER_I2C_ERRCODE                        (-0x7000)  //-28672, i2c错误码起始
#define OPRT_OS_ADAPTER_I2C_OPEN_FAILED                    (-0x7001)  //-28673, I2C 打开失败
#define OPRT_OS_ADAPTER_I2C_CLOSE_FAILED                   (-0x7002)  //-28674, I2C 关闭失败
#define OPRT_OS_ADAPTER_I2C_READ_FAILED                    (-0x7003)  //-28675, I2C 读取失败
#define OPRT_OS_ADAPTER_I2C_WRITE_FAILED                   (-0x7004)  //-28676, I2C 写入失败
#define OPRT_OS_ADAPTER_I2C_ERRCODE_MAX_CNT 5


/****************************************************************************
            the error code marco define for module OS_ADAPTER_BT 
****************************************************************************/
#define OPRT_OS_ADAPTER_BLE_ERRCODE                         (-0x7100)  //-28928, Bluetooth 错误码起始
#define OPRT_OS_ADAPTER_BLE_HANDLE_ERROR                    (-0x7101)  //-28929, Ble 句柄错误
#define OPRT_OS_ADAPTER_BLE_BUSY                            (-0x7102)  //-28930, Ble 繁忙
#define OPRT_OS_ADAPTER_BLE_TIMEOUT                         (-0x7103)  //-28931, Ble 超时
#define OPRT_OS_ADAPTER_BLE_RESERVED1                       (-0x7104)  //-28932, Ble Reserved Error Code 1
#define OPRT_OS_ADAPTER_BLE_RESERVED2                       (-0x7105)  //-28933, Ble Reserved Error Code 2
#define OPRT_OS_ADAPTER_BLE_RESERVED3                       (-0x7106)  //-28934, Ble Reserved Error Code 3
#define OPRT_OS_ADAPTER_BLE_INIT_FAILED                     (-0x7107)  //-28935, Ble 初始化失败
#define OPRT_OS_ADAPTER_BLE_DEINIT_FAILED                   (-0x7108)  //-28936, Ble 释放失败
#define OPRT_OS_ADAPTER_BLE_GATT_CONN_FAILED                (-0x7109)  //-28937, Ble GATT连接失败
#define OPRT_OS_ADAPTER_BLE_GATT_DISCONN_FAILED             (-0x710A)  //-28938, Ble GATT断开失败
#define OPRT_OS_ADAPTER_BLE_ADV_START_FAILED                (-0x710B)  //-28939, Ble 开启广播失败
#define OPRT_OS_ADAPTER_BLE_ADV_STOP_FAILED                 (-0x710C)  //-28940, Ble 停止广播失败
#define OPRT_OS_ADAPTER_BLE_SCAN_START_FAILED               (-0x710D)  //-28941, Ble 开启扫描失败
#define OPRT_OS_ADAPTER_BLE_SCAN_STOP_FAILED               (-0x710E)  //-28942, Ble 停止扫描失败
#define OPRT_OS_ADAPTER_BLE_SVC_DISC_FAILED                 (-0x710F)  //-28943, Ble 服务发现失败
#define OPRT_OS_ADAPTER_BLE_CHAR_DISC_FAILED                (-0x7110)  //-28944, Ble 特征值发现失败
#define OPRT_OS_ADAPTER_BLE_DESC_DISC_FAILED                (-0x7111)  //-28945, Ble 特征值描述符发现失败
#define OPRT_OS_ADAPTER_BLE_NOTIFY_FAILED                   (-0x7112)  //-28946, Ble Peripheral Notify失败
#define OPRT_OS_ADAPTER_BLE_INDICATE_FAILED                 (-0x7113)  //-28947, Ble Peripheral Indicate失败
#define OPRT_OS_ADAPTER_BLE_WRITE_FAILED                    (-0x7114)  //-28948, Ble Central 写失败
#define OPRT_OS_ADAPTER_BLE_READ_FAILED                     (-0x7115)  //-28949, Ble Central 读失败
#define OPRT_OS_ADAPTER_BLE_MTU_REQ_FAILED                  (-0x7116)  //-28950, Ble Central MTU请求失败
#define OPRT_OS_ADAPTER_BLE_MTU_REPLY_FAILED                (-0x7117)  //-28951, Ble Peripheral MTU 响应失败
#define OPRT_OS_ADAPTER_BLE_CONN_PARAM_UPDATE_FAILED        (-0x7118)  //-28952, Ble 连接参数更新失败
#define OPRT_OS_ADAPTER_BLE_CONN_RSSI_GET_FAILED            (-0x7119)  //-28953, Ble 连接信号强度获取失败

#define OPRT_OS_ADAPTER_BLE_MESH_ERRCODE                    (-0x7160)  //-29024, Ble Mesh错误码起始
#define OPRT_OS_ADAPTER_BLE_MESH_INVALID_OPCODE             (-0x7161)  //-29025, Ble Mesh 无效的opcode
#define OPRT_OS_ADAPTER_BLE_MESH_INVALID_ELEMENT            (-0x7162)  //-29026, Ble Mesh 无效的element
#define OPRT_OS_ADAPTER_BLE_MESH_INVALID_MODEL              (-0x7163)  //-29027, Ble Mesh 无效的model
#define OPRT_OS_ADAPTER_BLE_MESH_INVALID_ADDR               (-0x7164)  //-29028, Ble Mesh 无效的source, virtual或destination address
#define OPRT_OS_ADAPTER_BLE_MESH_INVALID_INDEX              (-0x7165)  //-29029, Ble Mesh 无效的序号，如：appkey index, netkey index等
#define OPRT_OS_ADAPTER_BLE_MESH_NO_MEMORY                  (-0x7166)  //-29030, Ble Mesh 内存占用已满，如：发包过快导致底层缓存过多等
#define OPRT_OS_ADAPTER_BLE_MESH_APPKEY_NOT_BOUND_MODEL     (-0x7167)  //-29031, Ble Mesh Appkey未绑定错误
#define OPRT_OS_ADAPTER_BLE_MESH_RESERVED1                  (-0x7168)  //-29032, Ble Mesh Reserved Error Code 1
#define OPRT_OS_ADAPTER_BLE_MESH_RESERVED2                  (-0x7169)  //-29033, Ble Mesh Reserved Error Code 2
#define OPRT_OS_ADAPTER_BLE_MESH_RESERVED3                  (-0x716A)  //-29034, Ble Mesh Reserved Error Code 3
#define OPRT_OS_ADAPTER_BLE_MESH_RESERVED4                  (-0x716B)  //-29035, Ble Mesh Reserved Error Code 4
#define OPRT_OS_ADAPTER_BLE_MESH_RESERVED5                  (-0x716C)  //-29036, Ble Mesh Reserved Error Code 5
#define OPRT_OS_ADAPTER_BLE_MESH_PROVISION_FAIL             (-0x716D)  //-29037, Ble Mesh 配网失败
#define OPRT_OS_ADAPTER_BLE_MESH_COMPO_GET_FAIL             (-0x716E)  //-29038, Ble Mesh 获取Composition数据失败
#define OPRT_OS_ADAPTER_BLE_MESH_MODEL_BIND_FAIL            (-0x716F)  //-29039, Ble Mesh 绑定model失败
#define OPRT_OS_ADAPTER_BLE_MESH_APPKEY_ADD_FAIL            (-0x7170)  //-29040, Ble Mesh Appkey添加失败
#define OPRT_OS_ADAPTER_BLE_MESH_NETKEY_ADD_FAIL            (-0x7171)  //-29041, Ble Mesh Netkey添加失败
#define OPRT_OS_ADAPTER_BLE_MESH_APPKEY_BIND_FAIL           (-0x7172)  //-29042, Ble Mesh Appkey绑定失败

#define OPRT_OS_ADAPTER_BT_ERRCODE_MAX_CNT 45

/****************************************************************************
            the error code marco define for Zigbee 
****************************************************************************/
#define OPRT_OS_ADAPTER_ZG_ERRCODE                         (-0x8000)  //-32768, Ble Mesh错误码起始
#define OPRT_OS_ADAPTER_ZG_SEND_FAILED                     (-0x8001)  //-32769, Zigbee 数据发送失败
#define OPRT_OS_ADAPTER_ZG_SEND_BUSY                       (-0x8002)  //-32770, Zigbee 数据发送忙
#define OPRT_OS_ADAPTER_ZG_ATTR_READ_FAILED                (-0x8003)  //-32771, Zigbee 读取attribute 失败
#define OPRT_OS_ADAPTER_ZG_ATTR_WRITE_FAILED               (-0x8004)  //-32772, Zigbee 写入attribute 失败
#define OPRT_OS_ADAPTER_ZG_EP_REG_FAILED                   (-0x8005)  //-32773, Zigbee endpoint 注册失败
#define OPRT_OS_ADAPTER_ZG_SCAN_FAILED                     (-0x8006)  //-32774, Zigbee 启动扫描失败
#define OPRT_OS_ADAPTER_ZG_STOP_SCAN_FAILED                (-0x8007)  //-32775, Zigbee 停止扫描失败
#define OPRT_OS_ADAPTER_ZG_LEAVE_FAILED                    (-0x8008)  //-32776, Zigbee 离网失败

#define OPRT_OS_ADAPTER_ZG_ERRCODE_MAX_CNT 9

#define TKL_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x)){\
        tkl_log_output(__FILE__);\
        tkl_log_output("\n");\
        return (y);\
    }\
}while(0)

#define TKL_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       tkl_log_output(#func);\
       tkl_log_output("\n");\
       return (rt);\
    }\
    tkl_log_output(#func);\
    tkl_log_output("\n");\
}while(0)

#define TUYA_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s null", #x);\
        return (y);\
    }\
}while(0)


#define TUYA_CHECK_NULL_GOTO(x, label)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s null", #x);\
        goto label;\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        PR_ERR("ret:%d", rt);\
}while(0)


#define TUYA_CALL_ERR_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("ret:%d", rt);\
        goto label;\
    }\
}while(0)


#define TUYA_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       PR_ERR("ret:%d", rt);\
       return (rt);\
    }\
}while(0)


#define TUYA_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("ret:%d", rt);\
        return (y);\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_RETURN_VAL(func, y, point)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("ret:%d", rt);\
        INSERT_ERROR_LOG_SEQ_DEC((point), rt);\
        return (y);\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_RETURN(func, point)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("ret:%d", rt);\
        INSERT_ERROR_LOG_SEQ_DEC((point), rt);\
        return (rt);\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("ret:%d", rt);\
        INSERT_ERROR_LOG_SEQ_DEC((point), rt);\
        goto label;\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)) {\
        PR_ERR("ret:%d", rt);\
        INSERT_ERROR_LOG_SEQ_DEC((point), rt);\
    }\
}while(0)


#define TUYA_CHECK_NULL_LOG_SEQ_RETURN(x, y, point)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s null", #x);\
        INSERT_ERROR_LOG_SEQ_DEC((point), y);\
        return (y);\
    }\
}while(0)


#define TUYA_CHECK_NULL_LOG_SEQ_GOTO(x, point, label)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s null", #x);\
        INSERT_ERROR_LOG_SEQ_NULL((point));\
        goto label;\
    }\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
