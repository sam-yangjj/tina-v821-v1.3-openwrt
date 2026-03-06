/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <alsa/asoundlib.h>

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>

#include <aactd/common.h>
#include <aactd/communicate.h>

#include "local_post_equal.h"
#include "post_equal.h"

#define LINE_STR_BUF_LEN_MAX 1024

#define TUNING_COM_BUF_LEN_DEFAULT 1024
#define UNIX_DOMAIN_SOCKET_PATH "/tmp/aactd.eq_sw.sock"

#define EPOLL_MAXEVENTS 1


static int parse_config_to_eq_prms(const char *config_file, eq_remote_prms_t *prms, int *enabled)
{
    int ret;
    FILE *fp = NULL;
    char line_str[LINE_STR_BUF_LEN_MAX];
    int temp_int;
    int type;
    int frequency;
    int gain;
    float Q;
    int index = 0;

    if (!config_file || !prms) {
        SNDERR("Invalid config_file or prms");
        ret = -1;
        goto out;
    }

    fp = fopen(config_file, "r");
    if (!fp) {
        SNDERR("Failed to open %s (%s)", strerror(errno));
        ret = -1;
        goto out;
    }

    while (fgets(line_str, LINE_STR_BUF_LEN_MAX, fp)) {
        if (sscanf(line_str, "channels=%d", &temp_int) == 1) {
            prms->chan = temp_int;
        } else if (sscanf(line_str, "enabled=%d", &temp_int) == 1) {
            *enabled = temp_int;
        } else if (sscanf(line_str, "bin_num=%d", &temp_int) == 1) {
            prms->biq_num = temp_int;
        } else if (sscanf(line_str, "samplerate=%d", &temp_int) == 1) {
            prms->sampling_rate = temp_int;
        } else if (sscanf(line_str, "params=%d %d %d %f",
                    &type, &frequency, &gain, &Q) == 4) {
            prms->core_prms[index].type = type;
            prms->core_prms[index].fc = frequency;
            prms->core_prms[index].G = gain;
            prms->core_prms[index].Q = Q;
            ++index;
        }
    }
    ret = 0;

    fclose(fp);
out:
    return ret;
}

static void print_eq_prms(const eq_remote_prms_t *prms)
{
    int i;
    for (i = 0; i < prms->biq_num; ++i) {
        const eq_core_prms_t *core_prms = &prms->core_prms[i];
        SNDERR(" [Biquad%02i] type: %i, freq: %d, gain: %d, Q: %.2f",
                i + 1, core_prms->type, core_prms->fc, core_prms->G, core_prms->Q);
    }
}

static void print_eq_prms_pc(const eq_prms_t_pc *prms)
{
    int i;
    for (i = 0; i < MAX_FILTER_N; ++i) {
        const eq_core_prms_t *core_prms = &prms->core_prms[i];
        SNDERR(" [BQ%02i] type: %i, freq: %d, gain: %d, Q: %.2f, enabled: %d",
                i + 1, core_prms->type, core_prms->fc, core_prms->G,
                core_prms->Q, prms->enable_[i]);
    }
}

static int aactd_com_eq_sw_data_to_eq_prms(const struct aactd_com_eq_sw_data *data,
        eq_prms_t_pc *prms)
{
    int ret;
    int i;

    for (i = 0; i < MAX_FILTER_N; ++i) {
        prms->enable_[i] = 0;
    }

    if (data->filter_num > MAX_FILTER_N) {
        SNDERR("Too many filters");
        ret = -1;
        goto out;
    }

    for (i = 0; i < data->filter_num; ++i) {
        prms->core_prms[i].type = data->filter_args[i].type;
        prms->core_prms[i].fc = data->filter_args[i].frequency;
        prms->core_prms[i].G = data->filter_args[i].gain;
        prms->core_prms[i].Q = (float)(data->filter_args[i].quality) / 100;
        prms->enable_[i] = data->filter_args[i].enabled;
    }

    ret = 0;
out:
    return ret;
}

int post_equal_rpbuf_is_available(snd_pcm_postequal_t *preaec, const char* name)
{
    snd_pcm_postequal_t *awaec = (snd_pcm_postequal_t *)preaec;
	rpbuf_buffer_t * buffer = NULL;
	int i = 0;
	int ret = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awaec->rp_buffer[i]), name, strlen(name) + 1)) {
			buffer = awaec->rp_buffer[i];
			break;
		}
	}

	if (i >= POST_EQ_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
		return 0;
	}

	mutex_eq = awaec->mutex_send_eq[i];

    pthread_mutex_lock(&mutex_eq);
	ret = rpbuf_buffer_is_available(buffer);
    pthread_mutex_unlock(&mutex_eq);

	return ret;
}


snd_pcm_sframes_t post_equal_transfer(snd_pcm_postequal_t *posteq, const char* name,
        const void *data, unsigned long data_offset,
        unsigned long size)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)posteq;
	rpbuf_buffer_t * buffer = NULL;
	int ret = -1;
	void *buf_addr;
	size_t len;
	int i = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awequal->rp_buffer[i]), name, strlen(name) + 1)) {
			buffer = awequal->rp_buffer[i];
			break;
		}
	}

	if (i >= POST_EQ_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
		return 0;
	}

	mutex_eq = awequal->mutex_send_eq[i];

    pthread_mutex_lock(&mutex_eq);

    if (!buffer) {
        SNDERR("Invalid buffer");
        pthread_mutex_unlock(&mutex_eq);
        return 0;
    }

	if (awequal->eq_enabled) {

		len = rpbuf_buffer_len(buffer);
		if (len < size) {
			SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(buffer), len, size);
			pthread_mutex_unlock(&mutex_eq);
			return 0;
		}

		buf_addr = rpbuf_buffer_addr(buffer);

	    memcpy(buf_addr + data_offset, data, size);

		ret = rpbuf_transmit_buffer(buffer, data_offset, size);
		if (ret < 0) {
			SNDERR("transmit buffer %s error\n", rpbuf_buffer_name(buffer));
			pthread_mutex_unlock(&mutex_eq);
        	return ret;
		}
    }

    pthread_mutex_unlock(&mutex_eq);

    return size;
}

int post_equal_recv(snd_pcm_postequal_t *posteq, const char* name,
        void *data, unsigned long *data_offset,
        unsigned long *size, int recv_time)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)posteq;
	rpbuf_buffer_t * buffer = NULL;
	int ret = -1;
	void *buf_addr;
	size_t len;
	int i = 0;
	pthread_mutex_t mutex_eq;

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		if(!strncmp(rpbuf_buffer_name(awequal->rp_buffer[i]), name, strlen(name))) {
			buffer = awequal->rp_buffer[i];
			break;
		}
	}

	if (i >= POST_EQ_RP_BUF_CNT) {
		SNDERR("It doesn't match the name %s", name);
        return 0;
	}

	mutex_eq = awequal->mutex_recv_eq[i];

    pthread_mutex_lock(&mutex_eq);

	if (!buffer) {
        SNDERR("Invalid buffer");
        pthread_mutex_unlock(&mutex_eq);
        return 0;
    }

	if (awequal->eq_enabled) {

		buf_addr = rpbuf_buffer_addr(buffer);

		ret = rpbuf_receive_buffer(buffer, (unsigned int *)data_offset, (unsigned int *)size, recv_time);
		if (ret < 0) {
			if (ret != -ETIMEDOUT)
				printf("receive buffer %s error, ret=%d\n", rpbuf_buffer_name(buffer), ret);
			else
				printf("receive buffer %s timeout\n", rpbuf_buffer_name(buffer));
			goto err_out;
		}

		len = rpbuf_buffer_len(buffer);
		if (len < *size) {
			SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(buffer), len, *size);
			goto err_out;
		}

		memcpy(data, buf_addr + *data_offset, *size);
	}

	pthread_mutex_unlock(&mutex_eq);

	return 0;
err_out:
	pthread_mutex_unlock(&mutex_eq);
	return ret;
}

static int connect_to_tuning_server(void)
{
    int ret;
    int socket_fd;
    struct sockaddr_un server_addr;
    int flags;

    socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        SNDERR("Failed to create socket (%s)", strerror(errno));
        ret = -1;
        goto err_out;
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, UNIX_DOMAIN_SOCKET_PATH);

    ret = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        SNDERR("Failed to connect to server %s", UNIX_DOMAIN_SOCKET_PATH);
        goto close_fd;
    }

    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        SNDERR("F_GETFL error (%s)", strerror(errno));
        ret = -1;
        goto close_fd;
    }
    flags |= O_NONBLOCK;
    ret = fcntl(socket_fd, F_SETFL, flags);
    if (ret < 0) {
        SNDERR("F_SETFL error (%s)", strerror(errno));
        ret = -1;
        goto close_fd;
    }

    return socket_fd;

close_fd:
    close(socket_fd);
err_out:
    return ret;
}

static void *try_connecting_thread(void *arg)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)arg;

    while (!awequal->should_stop) {
        pthread_mutex_lock(&awequal->mutex_tuning_fd);
        while (awequal->tuning_fd >= 0) {
            pthread_cond_wait(&awequal->cond_try_connecting, &awequal->mutex_tuning_fd);
            if (awequal->should_stop) {
                pthread_mutex_unlock(&awequal->mutex_tuning_fd);
                goto out;
            }
        }
        awequal->tuning_fd = connect_to_tuning_server();
        if (awequal->tuning_fd < 0) {
            SNDERR("Failed to connect to tuning server, try again");
            pthread_mutex_unlock(&awequal->mutex_tuning_fd);
            sleep(1);
            continue;
        }
        pthread_cond_signal(&awequal->cond_connected);
        pthread_mutex_unlock(&awequal->mutex_tuning_fd);
    }

	SNDERR("exit try_connecting_thread");

out:
    pthread_exit(NULL);
}

static void *tuning_thread(void *arg)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)arg;
    int ret;
	void *buf_addr;
	size_t len;

    int epoll_fd;
    struct epoll_event event;
    struct epoll_event avail_events[EPOLL_MAXEVENTS];
    struct epoll_event *p_avail_event;
    int nfds;
    uint32_t data_offset = 0;
    uint32_t size = 0;
    uint32_t recv_time = 100;

    ssize_t read_bytes;
    uint8_t *remote_com_buf = NULL;
    unsigned int remote_com_actual_len;
    struct aactd_com remote_com = {
        .data = NULL,
    };
    uint8_t checksum_cal;

    struct aactd_com_eq_sw_data remote_data = {
        .filter_args = NULL,
    };
    eq_prms_t_pc remote_prms;

    remote_com_buf = malloc(awequal->tuning_com_buf_len);
    if (!remote_com_buf) {
        SNDERR("Failed to allocate memory for remote_com_buf");
        goto out;
    }
    remote_com.data = remote_com_buf + sizeof(struct aactd_com_header);

    remote_data.filter_args = malloc(awequal->tuning_com_buf_len);
    if (!remote_data.filter_args) {
        SNDERR("Failed to allocate memory for remote_data.filter_args");
        goto free_com_buf;
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        SNDERR("Failed to create epoll instance (%s)", strerror(errno));
        goto free_filter_args;
    }

    while (!awequal->should_stop) {
        pthread_mutex_lock(&awequal->mutex_tuning_fd);
        while (awequal->tuning_fd < 0) {
            pthread_cond_wait(&awequal->cond_connected, &awequal->mutex_tuning_fd);
            if (awequal->should_stop) {
                pthread_mutex_unlock(&awequal->mutex_tuning_fd);
                goto close_epoll_fd;
            }
        }

        event.data.fd = awequal->tuning_fd;
        event.events = EPOLLET | EPOLLIN | EPOLLRDHUP;
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, awequal->tuning_fd, &event);
        if (ret < 0) {
            SNDERR("EPOLL_CTL_ADD error (%s)", strerror(errno));
            pthread_mutex_unlock(&awequal->mutex_tuning_fd);
            break;
        }
        pthread_mutex_unlock(&awequal->mutex_tuning_fd);

        while (1) {
            nfds = epoll_wait(epoll_fd, avail_events, EPOLL_MAXEVENTS, -1);
            if (nfds < 0) {
                SNDERR("epoll_wait error (%s)", strerror(errno));
                continue;
            }

            if (nfds > 1) {
                SNDERR("More than one file descriptor is ready");
                continue;
            }
            p_avail_event = &avail_events[0];

            pthread_mutex_lock(&awequal->mutex_tuning_fd);

            if (p_avail_event->data.fd != awequal->tuning_fd) {
                SNDERR("Unexpected file descriptor");
                goto wait_for_next_event;
            }
            if (p_avail_event->events & EPOLLERR) {
                SNDERR("EPOLLERR. Connection with tuning server may be closed");
                goto rebuild_connection;
            }
            if (p_avail_event->events & EPOLLRDHUP) {
                SNDERR("Connection with tuning server closed");
                goto rebuild_connection;
            }
            if (!(p_avail_event->events & EPOLLIN)) {
                SNDERR("No EPOLLIN event");
                goto wait_for_next_event;
            }

            /* Read header */
            read_bytes = aactd_readn(awequal->tuning_fd, remote_com_buf,
                    sizeof(struct aactd_com_header));
            if (read_bytes < 0) {
                SNDERR("Failed to read from tuning server");
                goto rebuild_connection;
            } else if (read_bytes == 0) {
                SNDERR("Connection with tuning server closed");
                goto rebuild_connection;
            } else if ((size_t)read_bytes < sizeof(struct aactd_com_header)) {
                SNDERR("Failed to read header from tuning server, try again");
                goto wait_for_next_event;
            }

            aactd_com_buf_to_header(remote_com_buf, &remote_com.header);

            if (remote_com.header.flag != AACTD_COM_HEADER_FLAG) {
                SNDERR("Incorrect header flag: 0x%x\n", remote_com.header.flag);
                goto wait_for_next_event;
            }

            remote_com_actual_len =
                sizeof(struct aactd_com_header) + remote_com.header.data_len + 1;

            /* Read data & checksum */
            read_bytes = aactd_readn(awequal->tuning_fd, remote_com.data,
                    remote_com.header.data_len + 1);
            if (read_bytes < 0) {
                SNDERR("Failed to read from tuning server");
                goto rebuild_connection;
            } else if (read_bytes == 0) {
                SNDERR("Connection with tuning server closed");
                goto rebuild_connection;
            } else if ((size_t)read_bytes < remote_com.header.data_len + 1) {
                SNDERR("Failed to read header data and checksum from tuning server, "
                        "try again");
                goto wait_for_next_event;
            }

            /* Verify checksum */
            remote_com.checksum = *(remote_com.data + remote_com.header.data_len);
            checksum_cal = aactd_calculate_checksum(remote_com_buf, remote_com_actual_len - 1);
            if (remote_com.checksum != checksum_cal) {
                SNDERR("Checksum error (got: 0x%x, calculated: 0x%x), discard and try again",
                        remote_com.checksum, checksum_cal);
                goto wait_for_next_event;
            }

            ret = aactd_com_eq_sw_buf_to_data(remote_com.data, &remote_data);
            if (ret < 0) {
                SNDERR("Failed to convert remote data buffer to data");
                goto wait_for_next_event;
            }
            ret = aactd_com_eq_sw_data_to_eq_prms(&remote_data, &remote_prms);
            if (ret < 0) {
                SNDERR("Failed to convert remote data to eq prms");
                goto wait_for_next_event;
            }

            pthread_mutex_lock(&awequal->mutex_send_eq[SEND_RPBUFF]);
            awequal->eq_enabled = remote_data.global_enabled;
            /*
             * NOTE: Use original "chan" and "sampling_rate", because they
             * won't be set by remote server.
             */
            remote_prms.chan = awequal->eq_remote_prms.chan;
            remote_prms.sampling_rate = awequal->eq_remote_prms.sampling_rate;

            len = rpbuf_buffer_len(awequal->rp_buffer[RESET_RPBUFF]);
            if (len < sizeof(eq_prms_t_pc)) {
                SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(awequal->rp_buffer[RESET_RPBUFF]), len, sizeof(eq_prms_t_pc));
                pthread_mutex_unlock(&awequal->mutex_send_eq[SEND_RPBUFF]);
                goto out;
            }

            buf_addr = rpbuf_buffer_addr(awequal->rp_buffer[RESET_RPBUFF]);
            memcpy(buf_addr, &remote_prms, sizeof(eq_prms_t_pc));

            ret = rpbuf_transmit_buffer(awequal->rp_buffer[RESET_RPBUFF], 0, sizeof(eq_prms_t_pc));
            if (ret < 0) {
                SNDERR("transmit buffer %s error\n", rpbuf_buffer_name(awequal->rp_buffer[RESET_RPBUFF]));
                pthread_mutex_unlock(&awequal->mutex_send_eq[SEND_RPBUFF]);
                goto wait_for_next_event;
            }

            ret = rpbuf_receive_buffer(awequal->rp_buffer[RESET_RPBUFF], (unsigned int *)&data_offset, (unsigned int *)&size, recv_time);
            if (ret < 0) {
                if (ret != -ETIMEDOUT)
                    printf("receive buffer %s error, ret=%d\n", rpbuf_buffer_name(awequal->rp_buffer[RESET_RPBUFF]), ret);
                else
                    printf("receive buffer %s timeout\n", rpbuf_buffer_name(awequal->rp_buffer[RESET_RPBUFF]));
                pthread_mutex_unlock(&awequal->mutex_send_eq[SEND_RPBUFF]);
                goto wait_for_next_event;
            }

            pthread_mutex_unlock(&awequal->mutex_send_eq[SEND_RPBUFF]);

            if (awequal->verbose) {
                SNDERR("Data received from remote server:");
                aactd_com_print_content(&remote_com);
                SNDERR("Parameters updated:");
                print_eq_prms_pc(&remote_prms);
            }

wait_for_next_event:
            pthread_mutex_unlock(&awequal->mutex_tuning_fd);
            continue;

rebuild_connection:
            pthread_mutex_unlock(&awequal->mutex_tuning_fd);
            break;
        }

        pthread_mutex_lock(&awequal->mutex_tuning_fd);
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, awequal->tuning_fd, NULL);
        if (ret < 0) {
            SNDERR("EPOLL_CTL_DEL error (%s)", strerror(errno));
            pthread_mutex_unlock(&awequal->mutex_tuning_fd);
            break;
        }
        close(awequal->tuning_fd);
        awequal->tuning_fd = -1;
        pthread_cond_signal(&awequal->cond_try_connecting);
        pthread_mutex_unlock(&awequal->mutex_tuning_fd);
    }

	SNDERR("exit tunnig thread");

close_epoll_fd:
    close(epoll_fd);
free_filter_args:
    free(remote_data.filter_args);
free_com_buf:
    free(remote_com_buf);
out:
    pthread_exit(NULL);
}

int post_equal_hw_params(snd_pcm_postequal_t *posteq, unsigned int channels, unsigned int rate, unsigned int bits)
{
    int ret;
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)posteq;
	void *buf_addr;
	unsigned int offset = 0;
	size_t len;

    if (!awequal) {
        ret = -1;
        goto out;
    }

    ret = parse_config_to_eq_prms(awequal->config_file,
            &awequal->eq_remote_prms, &awequal->eq_enabled);
    if (ret < 0) {
        SNDERR("parse_config_to_eq_prms failed");
        goto out;
    }
    /*
     * Overwrite the "chan" and "sampling_rate" parameters with ALSA parameters.
     * Therefore, these two parameters in config file are useless actually.
     */
    awequal->eq_remote_prms.chan = channels;
    awequal->eq_remote_prms.sampling_rate = rate;
    awequal->eq_remote_prms.bits_per_sample = bits;

    if (awequal->verbose) {
        SNDERR("Parse from config file %s", awequal->config_file);
        SNDERR("  GlobalEnable: %d", awequal->eq_enabled);
        print_eq_prms(&awequal->eq_remote_prms);
    }

    if (awequal->tuning_support) {
        ret = pthread_create(&awequal->try_connecting_thread_id, NULL,
                try_connecting_thread, (void *)awequal);
        if (ret != 0) {
            SNDERR("Failed to create try connecting thread");
            ret = -1;
            goto out;
        }

        ret = pthread_create(&awequal->tuning_thread_id, NULL,
                tuning_thread, (void *)awequal);
        if (ret != 0) {
            SNDERR("Failed to create tuning thread");
            ret = -1;
            goto out;
        }
    }

	len = rpbuf_buffer_len(awequal->rp_buffer[START_RPBUFF]);
	if (len < sizeof(eq_remote_prms_t)) {
		SNDERR("buffer %s len %lu is less than input size %lu\n", rpbuf_buffer_name(awequal->rp_buffer[START_RPBUFF]), len, sizeof(eq_remote_prms_t));
		goto out;
	}

	buf_addr = rpbuf_buffer_addr(awequal->rp_buffer[START_RPBUFF]);

	memcpy(buf_addr, &awequal->eq_remote_prms, sizeof(eq_remote_prms_t));

	ret = rpbuf_transmit_buffer(awequal->rp_buffer[START_RPBUFF], offset, sizeof(eq_remote_prms_t));
	if (ret < 0) {
		SNDERR("transmit buffer %s error\n", rpbuf_buffer_name(awequal->rp_buffer[START_RPBUFF]));
		goto out;
	}

    ret = 0;
out:
    return ret;
}

int post_equal_hw_free(snd_pcm_postequal_t *posteq)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)posteq;

    if (!awequal) {
        return 0;
    }

    if (awequal->tuning_support) {
        pthread_mutex_lock(&awequal->mutex_tuning_fd);
        if (awequal->tuning_fd >= 0) {
            /*
             * try_connecting_thread may block on pthread_cond_wait(cond_try_connecting),
             * use signal to wake it up;
             * tuning_thread may block on epoll_wait(), use shutdown() to
             * trigger epoll_wait() to return. After finding the connection
             * has been shut down, the tuning_fd will be closed in tuning_thread.
             */
            pthread_cond_signal(&awequal->cond_try_connecting);
            shutdown(awequal->tuning_fd, SHUT_RDWR);
        } else {
            /*
             * tuning_thread may block on pthread_cond_wait(cond_connected),
             * use signal to wake it up.
             */
            pthread_cond_signal(&awequal->cond_connected);
        }
        awequal->should_stop = 1;
        pthread_mutex_unlock(&awequal->mutex_tuning_fd);

        pthread_join(awequal->try_connecting_thread_id, NULL);
        pthread_join(awequal->tuning_thread_id, NULL);
    }

	SNDERR("hw free exit");

    return 0;
}

int post_equal_close(snd_pcm_postequal_t *posteq)
{
    snd_pcm_postequal_t *awequal = (snd_pcm_postequal_t *)posteq;
	int i;

    if (!awequal) {
        return 0;
    }

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		if (awequal->rp_buffer[i]) {
			rpbuf_free_buffer(awequal->rp_buffer[i]);
	        awequal->rp_buffer[i] = NULL;
    	}
	}

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awequal->mutex_recv_eq[i]);
	}
	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awequal->mutex_send_eq[i]);
	}

    pthread_mutex_destroy(&awequal->mutex_tuning_fd);
    pthread_cond_destroy(&awequal->cond_connected);
    pthread_cond_destroy(&awequal->cond_try_connecting);
    if (awequal->config_file) {
        free(awequal->config_file);
        awequal->config_file = NULL;
    }
    free(awequal);
    awequal = NULL;
    return 0;
}

static int get_int_parm(snd_config_t *n, const char *id, const char *str,
        int *value_ret)
{
    long value;
    int err;

	//SNDERR("get_int_parm id %s ", id);

    if (strcmp(id, str) != 0) {
        return 1;
    }
    err = snd_config_get_integer(n, &value);
    if (err < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return err;
    }
    *value_ret = value;
    return 0;
}

static int get_string_param(snd_config_t *n, const char *id, const char *str,
        const char **value_ret)
{
    const char *value = NULL;
    int err;

	//SNDERR("get_string_param id %s ", id);

    if (strcmp(id, str) != 0) {
        return 1;
    }
    err = snd_config_get_string(n, &value);
    if (err < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return err;
    }
    *value_ret = value;
    return 0;
}

static int get_bool_parm(snd_config_t *n, const char *id, const char *str,
        int *value_ret)
{
    int value;


	//SNDERR("get_bool_parm id %s ", id);

    if (strcmp(id, str) != 0) {
        return 1;
    }

    value = snd_config_get_bool(n);
    if (value < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return value;
    }
    *value_ret = value;
    return 0;
}


int post_equal_open(snd_pcm_postequal_t **posteq, const char *pcm_name, rpbuf_arg_post_eq *eq_arg, int ctrl_id, unsigned int sync, size_t len)

{
    snd_config_iterator_t index, next;
    snd_config_t *slave_conf = NULL;
    snd_pcm_postequal_t *awequal= NULL;
	snd_config_t *pcm_conf = NULL;
	snd_config_t *top = NULL;
    snd_config_t *conf = NULL;
	const char *pcm_id = NULL;

    int err = 0;
    const char *config_file = NULL;
    int tuning_support = 0;
    int tuning_com_buf_len = TUNING_COM_BUF_LEN_DEFAULT;
    int verbose = 0;
	int i;


	if (!strlen(pcm_name) || !strlen(eq_arg->aeq_config_name) || !strlen(eq_arg->aeq_reset_config_name) || !strlen(eq_arg->aeq_in_name) ||
		!strlen(eq_arg->aeq_out_name)) {
		SNDERR("name is all null");
		return -1;
	}

	SNDERR("pcm_name %s , aeq_config_name %s aeq_reset_config_name %s aeq_in_name %s aeq_out_name %s !\n", \
			pcm_name, eq_arg->aeq_config_name, eq_arg->aeq_reset_config_name, eq_arg->aeq_in_name, eq_arg->aeq_out_name);

	err = snd_config_update_ref(&top);
	if (err < 0)
		return err;

	err = snd_config_search_definition(top, "pcm", pcm_name, &pcm_conf);
	if (err < 0) {
		SNDERR("Unknown PCM %s", pcm_name);
		return -1;
	}

	snd_config_get_id(pcm_conf, &pcm_id);

	SNDERR("pcm_conf id %s , pcm_conf type %d\n", pcm_id, snd_config_get_type(pcm_conf));

	err = snd_config_search_definition(pcm_conf, "slave", "pcm", &conf);
	if (err < 0) {
		SNDERR("Unknown PCM %s", pcm_name);
		return -1;
	}

	snd_config_get_id(conf, &pcm_id);

	SNDERR("pcm_conf id %s , pcm_conf type %d\n", pcm_id, snd_config_get_type(conf));

    snd_config_for_each(index, next, conf) {
        snd_config_t *n = snd_config_iterator_entry(index);
        const char *id;
        if (snd_config_get_id(n, &id) < 0) {
            continue;
        }

		//SNDERR("conf id %s, conf type %d\n", id, snd_config_get_type(n));

        if (strcmp(id, "comment") == 0
                || strcmp(id, "type") == 0
                || strcmp(id, "hint") == 0) {
            continue;
        }
        if (strcmp(id, "slave") == 0) {
            slave_conf = n;
            continue;
        }
        err = get_string_param(n, id, "config_file", &config_file);
        if (err <= 0) {
            goto ok;
        }
        err = get_bool_parm(n, id, "tuning_support", &tuning_support);
        if (err <= 0) {
            goto ok;
        }
        err = get_int_parm(n, id, "tuning_com_buf_len", &tuning_com_buf_len);
        if (err <= 0) {
            goto ok;
        }
        err = get_bool_parm(n, id, "verbose", &verbose);
        if (err <= 0) {
            goto ok;
        }
        SNDERR("Unknown field %s", id);
        err = -EINVAL;
    ok:
        if (err < 0) {
            goto error;
        }
    }

    if (!slave_conf) {
        SNDERR("No slave defined for awequal");
        err = -EINVAL;
        goto error;
    }

	SNDERR("tuning_support %d , tuning_com_buf_len %d , verbose %d, config_file %s !\n", \
				tuning_support, tuning_com_buf_len, verbose, config_file);


    awequal = calloc(1, sizeof(*awequal));
    if (!awequal) {
        err = -ENOMEM;
        goto error;
    }

    if (!config_file) {
        config_file = "/etc/awequal.conf";
    }
    awequal->config_file = (char *)malloc(strlen(config_file) + 1);
    if (!awequal->config_file) {
        err = -ENOMEM;
        goto error;
    }
    strcpy(awequal->config_file, config_file);

    awequal->tuning_support = tuning_support;
    awequal->tuning_com_buf_len = tuning_com_buf_len;
    awequal->verbose = verbose;

	snd_config_delete(pcm_conf);

	snd_config_unref(top);

    err = pthread_cond_init(&awequal->cond_try_connecting, NULL);
    if (err < 0) {
        SNDERR("pthread_cond_init error");
        goto error;
    }
    err = pthread_cond_init(&awequal->cond_connected, NULL);
    if (err < 0) {
        SNDERR("pthread_cond_init error");
        goto destroy_cond_try_connecting;
    }
    err = pthread_mutex_init(&awequal->mutex_tuning_fd, NULL);
    if (err < 0) {
        SNDERR("pthread_mutex_init error");
        goto destroy_cond_connected;
    }
    awequal->tuning_fd = -1;

    awequal->should_stop = 0;

	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
	    err = pthread_mutex_init(&awequal->mutex_send_eq[i], NULL);
	    if (err < 0) {
	        SNDERR("pthread_mutex_init error");
	        goto destroy_mutex_eq;
	    }
	}

    for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
	    err = pthread_mutex_init(&awequal->mutex_recv_eq[i], NULL);
	    if (err < 0) {
	        SNDERR("pthread_mutex_init error");
	        goto destroy_mutex_recv_eq;
	    }
	}

    /* It will be block to wait the remote process to alloc buffer, the waiting time is 15s */
	awequal->rp_buffer[START_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, eq_arg->aeq_config_name, sizeof(eq_remote_prms_t));
	if (!awequal->rp_buffer[START_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_config_name, sizeof(eq_remote_prms_t));
		err = -1;
        goto destroy_alloc_buffer;
	}

	awequal->rp_buffer[RESET_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, eq_arg->aeq_reset_config_name, sizeof(eq_prms_t_pc));
	if (!awequal->rp_buffer[RESET_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_reset_config_name, sizeof(eq_prms_t_pc));
		err = -1;
        goto destroy_alloc_buffer;
	}

	awequal->rp_buffer[SEND_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, eq_arg->aeq_in_name, len);
	if (!awequal->rp_buffer[SEND_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_in_name, len);
		err = -1;
        goto destroy_alloc_buffer;
	}

	if (sync) {
		err = rpbuf_set_sync_buffer(awequal->rp_buffer[SEND_RPBUFF], sync);
		if (err < 0) {
			SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_in_name, len);
			goto destroy_alloc_buffer;
		}
	}

	awequal->rp_buffer[RECV_RPBUFF] = rpbuf_alloc_buffer(ctrl_id, eq_arg->aeq_out_name, len);
	if (!awequal->rp_buffer[RECV_RPBUFF]) {
		SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_out_name, len);
		err = -1;
        goto destroy_alloc_buffer;
	}
/*
	if (sync) {
		err = rpbuf_set_sync_buffer(awequal->rp_buffer[RECV_RPBUFF], sync);
		if (err < 0) {
			SNDERR("rpbuf_alloc_buffer for name %s (len: %d) failed\n", eq_arg->aeq_out_name, len);
			goto destroy_alloc_buffer;
		}
	}
*/

	*posteq = awequal;

    return 0;

destroy_alloc_buffer:
	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		if (awequal->rp_buffer[i]) {
			rpbuf_free_buffer(awequal->rp_buffer[i]);
	        awequal->rp_buffer[i] = NULL;
    	}
	}
destroy_mutex_recv_eq:
	for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awequal->mutex_recv_eq[i]);
	}
destroy_mutex_eq:
    for (i = 0; i < POST_EQ_RP_BUF_CNT; ++i) {
		pthread_mutex_destroy(&awequal->mutex_send_eq[i]);
	}
destroy_mutex_tuning_fd:
    pthread_mutex_destroy(&awequal->mutex_tuning_fd);
destroy_cond_connected:
    pthread_cond_destroy(&awequal->cond_connected);
destroy_cond_try_connecting:
    pthread_cond_destroy(&awequal->cond_try_connecting);
error:
    if (awequal) {
        if (awequal->config_file) {
            free(awequal->config_file);
        }
        free(awequal);
    }
    return err;
}

