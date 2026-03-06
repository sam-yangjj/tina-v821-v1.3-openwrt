#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "tkl_ir.h"


#define TY_IR_TEST_FEQ              38000
#define TY_IR_TEST_MAX              (1024*2)

#define IR_DATA_S   560
#define IR_DATA_0   560
#define IR_DATA_1   1680

//红外起始信号
#define IR_CMD_START_1    9000
#define IR_CMD_START_2    4500
//结束帧
#define IR_CMD_STOP_1     9000
#define IR_CMD_STOP_2     2200
//红外结束信号
#define IR_CMD_STOP_3     560
#define IR_CMD_STOP_4     50000
//红外信号误差
#define IR_TIME_ALW       250

const unsigned short  ir_cmd_start[] = {IR_CMD_START_1, IR_CMD_START_2};
const unsigned short  ir_cmd_stop[]  = {IR_CMD_STOP_1, IR_CMD_STOP_2, IR_CMD_STOP_3, IR_CMD_STOP_4};

int ty_ir_send_proc(unsigned short *ir_time_data, unsigned short data_len)
{
    int i;
    OPERATE_RET ret = 0;

    printf("%s data_len: %d \n", __func__, data_len);

    TY_IR_BUF_T code = {0};

    code.buf = malloc(sizeof(int)*TY_IR_TEST_MAX);
    code.cnts = data_len;
    code.size = sizeof(int)*TY_IR_TEST_MAX;

    if(NULL == code.buf) {
        printf("Malloc ir_data_s.codelist failed!\n");
        return -1;
    }
    memset(code.buf, 0,code.size);

    for (i=0; i<data_len; i++) {
        code.buf[i] = ir_time_data[i];
    }

    if (0 == code.cnts%2) {
        code.buf[code.cnts++] = 10;
    }

    for (i=0; i<code.cnts; i++) {
        printf("%d ", code.buf[i]);
    }

    printf("\r\n");

    ret = tkl_ir_init(IR_ID_1);

    if(ret) {
        free(code.buf);
	printf("tkl_ir_init(%d) fail\n",IR_ID_1);
        return -1; 
    }


    ret = tkl_ir_set_tx_carrier(IR_ID_1,TY_IR_TEST_FEQ);

    if(ret) {
        free(code.buf);
	printf("tkl_ir_set_tx_carrier(%d) fail\n",IR_ID_1);
        return -1; 
    }

    printf("hal_ir_set_tx_carrier ret = %d\n", ret);


    ret = tkl_ir_send(IR_ID_1,&code); 

    free(code.buf);

    printf("%s ret = %d\n", __func__, ret);

    tkl_ir_deinit(IR_ID_1);
    
    return ret;
}

static unsigned short ty_ir_send_pkg(char  *str,  unsigned short  *trsdata,  unsigned short str_len)
{
    unsigned short  trsdata_len;
    int  i, j;

    if (str == NULL) {
        return -1;
    }

     //ir_start_cmd
     memcpy(trsdata, ir_cmd_start, 4);
     trsdata += 2;

	//ir_data_cmd
     for (i = 0; i < str_len; i++) {
         for (j = 0; j < 8; j++) {
	     *trsdata = IR_DATA_S;
	     if (((*str >> j) & 0x01) == 1) {
	          *(trsdata + 1) = IR_DATA_1;
	     } else {
	         *(trsdata + 1) = IR_DATA_0;
	     }
	     trsdata += 2;
	}
	str++;
     }

     //ir_stop_cmd
     memcpy(trsdata, ir_cmd_stop, 8);
     trsdata_len = 2 + str_len * 8 * 2 + 4;

     return trsdata_len;
}

static int ty_ir_send_str(char  *ir_buf, int  ir_len)
{
    int  ret = -1;
    unsigned short  ir_time_len;
    unsigned short  *ir_time_buf;

    ir_time_buf = (unsigned short  *)malloc(TY_IR_TEST_MAX);

    if (NULL == ir_time_buf) {
        printf("malloc error\n");
	return -1;
    }

    memset(ir_time_buf, 0, TY_IR_TEST_MAX);
    printf("%s send_str:%s, len:%d\n", __func__, ir_buf, ir_len);

    ir_time_len = ty_ir_send_pkg(ir_buf, ir_time_buf, ir_len);
    ret  = ty_ir_send_proc(ir_time_buf, ir_time_len);

    free(ir_time_buf);

    return ret;
}
static int compare_value(unsigned short value1, unsigned short  value2)
{
    if (value1 > value2) {
        if ((value1 - value2) < IR_TIME_ALW) {
	     return 1;
	} else {
	     return 0;
	}
    } else {
        if ((value2 - value1) < IR_TIME_ALW) {
            return 1;
        } else {
            return  0;
        }
    }
}

static  int ty_ir_rec_to_string(TY_IR_BUF_T *raw_code,char  *recv_buf, int  *buf_size)
{
    int  *trsdata;
    int  str_len;
    unsigned short ir_data_len;
    unsigned char  ir_data_buf = 0;
    unsigned char  i = 0, j;
    unsigned char  ir_rev_buf[32] = {0};
    unsigned char  ir_rev_len = 0;

    if (!raw_code || recv_buf == NULL|| buf_size == NULL) {
        printf("ir into error\n");
        return -1;
    }

    trsdata = raw_code->buf;
    str_len = raw_code->cnts;

    while (!(compare_value(*trsdata, IR_CMD_START_1) && compare_value(*(trsdata + 1), IR_CMD_START_2))) {
        trsdata++;
        str_len--;

        if (str_len == 1) {
            printf("not found ir start cmd\n");
            return -1;
        }
    }

    trsdata += 2;
    ir_data_len = str_len - 2;

    while (ir_data_len / 16) {
        for (j = 0; j < 8; j++) {
            if ((compare_value(*trsdata, IR_DATA_S) && compare_value(*(trsdata + 1), IR_DATA_1))) {
	        ir_data_buf |= (0x01 << j);
            } else if ((compare_value(*trsdata, IR_DATA_S) && compare_value(*(trsdata + 1), IR_DATA_0))) {
	        ir_data_buf &= ~(0x01 << j);
	    } else {
		goto end_data;
	    }

            trsdata += 2;
	    ir_data_len -= 2;
	}

        ir_rev_buf[i] = ir_data_buf;
        ir_data_buf = 0;
        i++;
    }

end_data:
    ir_rev_len = i;
    printf("rev_buf_len:%d\n", ir_rev_len);
    while (!((compare_value(*trsdata, IR_CMD_STOP_1) && compare_value(*(trsdata + 1), IR_CMD_STOP_2)))) {
        trsdata++;
        ir_data_len--;
        if (ir_data_len == 1) {
            printf("not found ir stop cmd1\n");
            return -1;
        }
    }

    if (compare_value(*(trsdata + 2), IR_CMD_STOP_3)) {
        ir_data_buf |= (0x01 << j);
    } else {
        printf("not found ir stop cmd2\n");
        return -1;
    }
    printf("[Tydyha]ir_rev_buf:%s *recv_buf:%p recv_buf:%p\n", ir_rev_buf, *recv_buf, recv_buf);
    memcpy(recv_buf, ir_rev_buf, ir_rev_len);
    *buf_size = ir_rev_len;
    return 0;
}

// linux infrared test start

struct member {
    int num;
    char *str;
};

static void *ty_ir_send_test(void *arg)
{
    int snd_times = 10;
    struct member *temp;
    temp = (struct member *)arg;
    char *snd_str = temp->str;

    printf("snd_str:%s\n", temp->str);

    if (!snd_str) {
        printf("%s err, invalid\n");
        return NULL;
    }

    do {
        ty_ir_send_str(snd_str, strlen(snd_str));
        if (--snd_times > 0) {
            usleep(300 * 1000);
        }
    } while(snd_times > 0);

    return NULL;
}

static void *ty_ir_recv_test(void *arg)
{
    int size = 20;
    struct member *temp;
    temp = (struct member *)arg;
    char *recv_buf = temp->str;
    TY_IR_BUF_T code = {0};
    int ret = 0;

    if (NULL == recv_buf) {
        printf("%s err invalid parm\n", __func__);
        return NULL ;
    }

    printf("recv_buf:%p &recv_buf:%p\n", recv_buf, &recv_buf);
    printf("%s begin\n", __func__);


    code.buf = malloc(sizeof(int)*TY_IR_TEST_MAX);
    if(NULL == code.buf) {
        printf("%s malloc failed.\n", __func__);
        return NULL ;
    }
    code.cnts = 0;
    code.size = sizeof(int)*TY_IR_TEST_MAX;
    memset(code.buf,0, code.size);

    if((ret = tkl_ir_init(IR_ID_0)) != 0) {
       printf("tkl_ir_init = %d\n",ret);
       return NULL;
    }

    ret = tkl_ir_recv(IR_ID_0,&code,20000); // 20s

    if(ret < 0) {
        printf("ir_ata_recv failed, errcode = %d\n", ret);
        goto exit;
    }

    if (code.cnts < 6) {
        printf("data recv count=%d, is invailed\n", code.cnts);
        ret = -1;
        goto exit;
    }

    ty_ir_rec_to_string(&code, recv_buf, &size);

    printf("6661\n");
    printf("recv_buf: %s\n", recv_buf);

exit:
    if(NULL != code.buf) {
        free(code.buf);
        code.buf = NULL;
    }

    printf("%s ret = %d\n", __func__, ret);
    tkl_ir_deinit(IR_ID_0);

    return NULL  ;
}

int main(int argc,char **argv)
{
    int ret;
    char *send_str = "AC64CF576C37";
    char *recv_str = NULL;
    pthread_t send_tid;
    struct member *s, *r;

    recv_str = malloc(sizeof(int)*TY_IR_TEST_MAX);

    s = (struct member *)malloc(sizeof(struct member));
    s->str = send_str;
   //memset(s, 0, sizeof(struct member));
   //memcpy(s->str, send_str, strlen(send_str));

    r = (struct member *)malloc(sizeof(struct member));
   //memset(r, 0, sizeof(struct member));
    r->str = recv_str;
    printf("r->str:%p recv_str:%p\n", r->str, recv_str); //0x2f5560e4
    // create two threads infrared send and recv
    //pthread_t recv_tid;

    if(pthread_create(&send_tid, NULL, ty_ir_send_test, (void*)s) != 0) {
        perror("pthread_create");
        exit(1);
    }
/*
    if(pthread_create(&recv_tid, NULL, ty_ir_recv_test, (void*)r) != 0) {
        perror("pthread_create");
	exit(1);
    }
*/
    // join the threads, though it'll never be excuted.
    pthread_join(send_tid, NULL);
    //pthread_join(recv_tid, NULL);
    printf("main: recv_str:%s\n", recv_str);
    if (!strcmp(send_str, recv_str))
        ret = 0;
    else
        ret = -1;

    return 0;
}
