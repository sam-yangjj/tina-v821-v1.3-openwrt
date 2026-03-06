/*
 * NVRAM variable manipulation (Linux kernel half)
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c,v 1.1 2007/06/08 07:38:05 arthur Exp $
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/mm.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/fs.h>
#include <bcmnvram.h>
#include <linux/cdev.h>
#include <linux/blkdev.h>

#include "../common/aes.h"
#include "../common/uni_base64.h"

#include "../common/nvram_core.c"

#define PROC_NVRAM_NAME	"nvram"

typedef struct nvram_k_wait_queue{
	wait_queue_head_t wq;
	int attention;
    int iswait;
}nvram_k_wait_queue_t;

typedef struct anvram_param_list_s {
    struct list_head list;
    anvram_ioctl_t nvram_ioctl;
}anvram_param_list_t;

struct nvram_k_wait_queue kwait;
static LIST_HEAD(nvram_param_list);


extern int nvram_flash_write_zbuf(char *zbuf, u_int32_t len);
extern int nvram_flash_read_zbuf(char *zbuf, u_int32_t len);
extern int _nvram_purge_unlocked(void);
int nvram_commit(void);

/* Globals */
static DEFINE_SPINLOCK(nvram_lock);
static DEFINE_MUTEX(nvram_sem);
static int nvram_major = -1;
static struct proc_dir_entry *g_pdentry = NULL;
static char *nvram_values = NULL;
static unsigned long nvram_offset = 0;
static struct cdev cdev;
static struct class *nvram_class = NULL;

static char aes128_key[16] = {
	KEY1,
	KEY2,
	KEY3,
	KEY4,
	KEY5,
	KEY6,
	KEY7,
	KEY8,
	KEY9,
	KEY10,
	KEY11,
	KEY12,
	KEY13,
	KEY14,
	KEY15,
	KEY16
};



struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name,
		const char *value, int is_temp)
{
	struct nvram_tuple *rt = t;
	uint32_t val_len = strlen(value);

	/* Copy name */
	if (!rt) {
		if (!(rt = kmalloc(sizeof(struct nvram_tuple) + strlen(name) + 1,
						GFP_ATOMIC)))
			return NULL;
		rt->name = (char *) &rt[1];
		strcpy(rt->name, name);
		rt->value = NULL;
		rt->val_len = 0;
		rt->flag = 0;
		rt->next = NULL;
	}

	/* Mark for temp tuple */
	rt->val_tmp = (is_temp) ? 1 : 0;

	/* Copy value */
	if (!rt->value || strcmp(rt->value, value)) {
		if (!rt->value || val_len > rt->val_len) {
			if ((nvram_offset + val_len + 1) >= NVRAM_VALUES_SPACE) {
				if (rt != t)
					kfree(rt);
				return NULL;
			}
			rt->value = &nvram_values[nvram_offset];
			rt->val_len = val_len;
			nvram_offset += (val_len + 1);
		}

		strcpy(rt->value, value);
	}

	return rt;
}

void _nvram_free(struct nvram_tuple *t)
{
	if (t)
		kfree(t);
}

void _nvram_reset(void)
{
	nvram_offset = 0;
	if (nvram_values)
		memset(nvram_values, 0, NVRAM_VALUES_SPACE);
}

// Note: user need free out
static int nvram_encode(const char *in, char **out)
{
	int ret;
	unsigned char *encode_output = NULL;
	unsigned int encode_len;
	char *base64_output = NULL;
	char aes_key[16]={0};
	int i;

    	for(i=0; i<16; i++)
	{
		if(aes128_key[i] <=0)
		{
			printk("invalid aes128_key %d\n", aes128_key[i]);
			return -EINVAL;
		}
		else if(aes128_key[i] <=9)
		{
			aes_key[i] = aes128_key[i]+48;
		}
		else if(aes128_key[i] >=10 && aes128_key[i] <=35)
		{
			aes_key[i] = aes128_key[i]+65-10;
		}
		else if(aes128_key[i] >=50 && aes128_key[i] <=75)
		{
			aes_key[i] = aes128_key[i]+97-50;
		}
		else if(aes128_key[i] >=101 && aes128_key[i] <=115)
		{
			aes_key[i] = aes128_key[i]+33-101;
		}
		else if(aes128_key[i] >=121 && aes128_key[i] <=127)
		{
			aes_key[i] = aes128_key[i]+58-121;
		}
		else if(aes128_key[i] >=128 && aes128_key[i] <=133)
		{
			aes_key[i] = aes128_key[i]+91-128;
		}
		else if(aes128_key[i] >=141 && aes128_key[i] <=144)
		{
			aes_key[i] = aes128_key[i]+123-141;
		}
		else
		{
			printk("invalid aes128_key %d\n", aes128_key[i]);
			return -EINVAL;
		}
	}



	ret = aes128_data_encode((unsigned char*)in, strlen(in),
			&encode_output, &encode_len, aes_key);//AES_128_KEY
	if (ret != 0 || encode_len <= 0) {
		pr_err("encode failed\n");
		return ret;
	}

	base64_output = kmalloc((encode_len/3)*4 + ((encode_len%3)?4:0) + 20 + 1,
			GFP_KERNEL);
	if (NULL == base64_output) {
		pr_err("alloc %d bytes fails\n", encode_len);
		return -ENOMEM;
	}

	tuya_base64_encode(encode_output, base64_output, encode_len);
	kfree(encode_output);

	*out = base64_output;

	pr_debug("encode done len: %d\n", strlen(base64_output));

	return 0;
}

// Note: user need free out
static int nvram_decode(const char *in, char **out)
{
	int ret;
	unsigned char *decode_output = NULL;
	unsigned int decode_len = 0;
	char *base64_output = NULL;
	char aes_key[16]={0};
	int i;


    	for(i=0; i<16; i++)
	{
		if(aes128_key[i] <=0)
		{
			printk("invalid aes128_key %d\n", aes128_key[i]);
			return -EINVAL;
		}
		else if(aes128_key[i] <=9)
		{
			aes_key[i] = aes128_key[i]+48;
		}
		else if(aes128_key[i] >=10 && aes128_key[i] <=35)
		{
			aes_key[i] = aes128_key[i]+65-10;
		}
		else if(aes128_key[i] >=50 && aes128_key[i] <=75)
		{
			aes_key[i] = aes128_key[i]+97-50;
		}
		else if(aes128_key[i] >=101 && aes128_key[i] <=115)
		{
			aes_key[i] = aes128_key[i]+33-101;
		}
		else if(aes128_key[i] >=121 && aes128_key[i] <=127)
		{
			aes_key[i] = aes128_key[i]+58-121;
		}
		else if(aes128_key[i] >=128 && aes128_key[i] <=133)
		{
			aes_key[i] = aes128_key[i]+91-128;
		}
		else if(aes128_key[i] >=141 && aes128_key[i] <=144)
		{
			aes_key[i] = aes128_key[i]+123-141;
		}
		else
		{
			printk("invalid aes128_key %d\n", aes128_key[i]);
			return -EINVAL;
		}
	}



	base64_output = kmalloc(strlen(in), GFP_KERNEL);
	if (NULL == base64_output) {
		pr_err("alloc %d bytes fails\n", strlen(in));
		return -ENOMEM;
	}

	ret = tuya_base64_decode(in, base64_output);

	ret = aes128_data_decode(base64_output, ret, &decode_output,
			&decode_len, aes_key);//AES_128_KEY
	if (ret != 0 || decode_len <= 0) {
		pr_err("decode failed\n");
		return ret;
	}

	kfree(base64_output);

	if (strlen(decode_output) > decode_len)
		decode_output[decode_len] = 0;

	*out = decode_output;

	//pr_debug("decode done len: %d\n", strlen(decode_output));

	return 0;
}

int kernel_nvram_decode(const char *in, char *out)
{
	int ret = 0;
	char *dec_out = NULL;

	if (nvram_decode(in, &dec_out))
		return -1;

	memcpy(out, dec_out, strlen(dec_out));

	if (dec_out)
		kfree(dec_out);

	return ret;
}

static int user_nvram_decode(const char *in, char *out)
{
	int ret = 0;
	char *dec_out = NULL;

	if (nvram_decode(in, &dec_out))
		return -1;

	if (copy_to_user(out, dec_out, strlen(dec_out)))
		ret = -EFAULT;

	if (dec_out)
		kfree(dec_out);

	return ret;
}

static int nvram_set_temp(const char *name, const char *value, int is_temp)
{
	int ret;
	unsigned long flags;
	char *out = NULL;

	if (!name)
		return -EINVAL;

	// Check early write
	if (nvram_major < 0)
		return 0;

	if (nvram_encode(value, &out) != 0)
		return -1;

	spin_lock_irqsave(&nvram_lock, flags);
	if ((ret = _nvram_set(name, out, is_temp)) && ret != -2) {
		struct nvram_header *header;
		/* Consolidate space and try again */
		if ((header = kzalloc(NVRAM_SPACE, GFP_ATOMIC))) {
			if (_nvram_generate(header, 1) == 0)
				ret = _nvram_set(name, out, is_temp);
			kfree(header);
		}
	}
	spin_unlock_irqrestore(&nvram_lock, flags);

	if (out)
		kfree(out);

	if (!ret && (is_temp & ATTR_PERSIST))
		ret = nvram_commit();

	return ret;
}

int nvram_set(const char *name, const char *value, int attr)
{
	return nvram_set_temp(name, value, attr);
}

int nvram_unset(const char *name)
{
	unsigned long flags;
	int ret;

	if (!name)
		return -EINVAL;

	// Check early write
	if (nvram_major < 0)
		return 0;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_unset(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

char *nvram_get_internal(const char *name)
{
	unsigned long flags;
	char *value;

	if (!name)
		return NULL;

	// Check early read
	if (nvram_major < 0)
		return NULL;

	spin_lock_irqsave(&nvram_lock, flags);
	value = _nvram_get(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return value;
}

int nvram_get(const char *name, char *value)
{
	unsigned long flags;
	char *enc_val;

	if (!name || !value)
		return -1;

	// Check early read
	if (nvram_major < 0)
		return -1;

	spin_lock_irqsave(&nvram_lock, flags);
	enc_val = _nvram_get(name);
	spin_unlock_irqrestore(&nvram_lock, flags);

	if (!enc_val)
		return 0;

	return kernel_nvram_decode(enc_val, value);
}

int nvram_getall(char *buf, int count, int include_temp)
{
	unsigned long flags;
	int ret;

	if (!buf || count < 1)
		return -EINVAL;

	memset(buf, 0, count);

	// Check early write
	if (nvram_major < 0)
		return 0;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_getall(buf, count, include_temp);
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

int nvram_commit(void)
{
	unsigned long flags;
	static unsigned char *bufw = NULL;
    static unsigned char *bufr = NULL;
	int ret;

	// Check early commit
	if (nvram_major < 0)
		return 0;

	if (!bufw) {
        bufw = kzalloc(NVRAM_SPACE, GFP_KERNEL);
        if (!bufw) {
            pr_err("nvram_commit: out of memory v1\n");
            return -ENOMEM;
        }
	}
    memset(bufw, 0, NVRAM_SPACE);

	if (!bufr) {
        bufr = kzalloc(NVRAM_SPACE, GFP_KERNEL);
        if (!bufr) {
            pr_err("nvram_commit: out of memory v1\n");
            return -ENOMEM;
        }
	}
    memset(bufr, 0, NVRAM_SPACE);

	/* Regenerate NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_generate((struct nvram_header *)bufw, 0);
	spin_unlock_irqrestore(&nvram_lock, flags);
	if (ret)
		goto done;

	mutex_lock(&nvram_sem);

	/* Check partition unchanged */
	if (bufr) {
		ret = nvram_flash_read_zbuf(bufr, NVRAM_SPACE);
		if (ret == 0 && memcmp(bufw, bufr, NVRAM_SPACE) == 0)
			goto skip_write;
	}

	/* Write partition up to end of data area */
	ret = nvram_flash_write_zbuf(bufw, NVRAM_SPACE);
	if (ret) {
		pr_err("nvram_commit: write error\n");
	}

skip_write:
	mutex_unlock(&nvram_sem);

done:
    /*
	kfree(bufw);
	if (bufr)
		kfree(bufr);
    */
	return ret;
}

int nvram_clear(void)
{
	unsigned long flags;

	// Check early clear
	if (nvram_major < 0)
		return 0;

	/* Reset NVRAM */
	spin_lock_irqsave(&nvram_lock, flags);
	_nvram_uninit();
	spin_unlock_irqrestore(&nvram_lock, flags);

	return 0;
}

int nvram_erase(void)
{
	unsigned long flags;
	int ret;

	// Check early clear
	if (nvram_major < 0)
		return 0;

	spin_lock_irqsave(&nvram_lock, flags);
	ret = _nvram_purge_unlocked();
	spin_unlock_irqrestore(&nvram_lock, flags);

	return ret;
}

/* case1: nvramwait.cnt=0, can be block, and wake up when any nvram key.
 * case2: param=xxx, value_len=0, can be block, and wake up when the param:xxx to be set.
 * case3: param=xxx, value=yyy, can be block, and wake up when the param to be set except yyy
 * tips: when wake up, the param list will be delet.
*/
int nvram_watch(anvram_kwait_t __user *pnvr)
{
	int ret;
    anvram_kwait_t nvramwait;
    anvram_param_t *nvramparam;
    anvram_param_list_t *paramlist;
    anvram_param_list_t *param_list;
    anvram_param_list_t *tmp;
    struct list_head *plist;
    int condition;
    int i;

	// Check early clear
	if (nvram_major < 0)
		return 0;

	if (!pnvr)
	{
		return -EINVAL;
	}

	if (copy_from_user(&nvramwait, pnvr, sizeof(anvram_kwait_t)))
	{
		printk("%s %d\n", __func__, __LINE__);
		return -EFAULT;
	}

    if(!nvramwait.cnt)
    {
	    printk("%s %d\n", __func__, __LINE__);
        goto WATCH;
    }

    nvramparam = kmalloc(sizeof(anvram_param_t)*nvramwait.cnt, GFP_KERNEL);
    if(!nvramparam)
    {
        return -ENOMEM;
    }

    condition=0;
    for(i=0; i<nvramwait.cnt; i++)
    {
        if (copy_from_user(nvramparam+i, nvramwait.paramp+i, sizeof(anvram_param_t)))
        {
            printk("%s %d\n", __func__, __LINE__);
            return -EFAULT;
        }

        paramlist = kmalloc(sizeof(anvram_param_list_t), GFP_KERNEL);
        if(!paramlist)
	    {
            printk("%s %d\n", __func__, __LINE__);
            return -ENOMEM;
	    }

        // add param
        paramlist->nvram_ioctl.param = kmalloc(nvramparam[i].nvram_ioctl.len_param+1, GFP_KERNEL);
        if(!paramlist->nvram_ioctl.param)
	    {
            printk("%s %d\n", __func__, __LINE__);
            return -ENOMEM;
	    }
        paramlist->nvram_ioctl.len_param = nvramparam[i].nvram_ioctl.len_param;

        if (copy_from_user(paramlist->nvram_ioctl.param, nvramparam[i].nvram_ioctl.param, nvramparam[i].nvram_ioctl.len_param))
	    {
            return -EFAULT;
        }
        paramlist->nvram_ioctl.param[paramlist->nvram_ioctl.len_param] = '\0';

        // add value
        if(nvramparam[i].nvram_ioctl.len_value == 0)
        {
            paramlist->nvram_ioctl.value=NULL;
            paramlist->nvram_ioctl.len_value=0;
        }
        else
        {
            paramlist->nvram_ioctl.value= kmalloc(nvramparam[i].nvram_ioctl.len_value+1, GFP_KERNEL);
            if(!paramlist->nvram_ioctl.value)
            {
                printk("%s %d\n", __func__, __LINE__);
                return -ENOMEM;
            }
            paramlist->nvram_ioctl.len_value= nvramparam[i].nvram_ioctl.len_value;

            if (copy_from_user(paramlist->nvram_ioctl.value, nvramparam[i].nvram_ioctl.value, nvramparam[i].nvram_ioctl.len_value))
            {
                return -EFAULT;
            }
            paramlist->nvram_ioctl.value[paramlist->nvram_ioctl.len_value] = '\0';
        }

        printk("%s %d watch list add [%s:%s]\n", __func__, __LINE__, paramlist->nvram_ioctl.param, paramlist->nvram_ioctl.value);

        // avoid the same nvram param
        list_for_each(plist, &nvram_param_list)
        {
            param_list = list_entry(plist, anvram_param_list_t, list);
            if((param_list->nvram_ioctl.len_param == paramlist->nvram_ioctl.len_param) && !strncmp(param_list->nvram_ioctl.param, paramlist->nvram_ioctl.param, param_list->nvram_ioctl.len_param))
            {
                condition=1;
                break;
            }
        }
        if(condition==1)
        {
            printk("%s %d set nvram [%s] is already exit.\n", __func__, __LINE__, paramlist->nvram_ioctl.param);
            condition=0;
        }
        else
        {
            list_add(&paramlist->list, &nvram_param_list);
        }
    }

    kfree(nvramparam);

WATCH:
    kwait.iswait = 1;
	ret = wait_event_interruptible(kwait.wq, kwait.attention);
	kwait.attention = 0;
    kwait.iswait = 0;
	printk("%s %d wake up!\n", __func__, __LINE__);

    // after wake up, list del wait queue
	list_for_each_entry_safe(param_list, tmp, &nvram_param_list, list) {
	    if(param_list->nvram_ioctl.len_param)
            kfree(param_list->nvram_ioctl.param);
        if(param_list->nvram_ioctl.len_value)
            kfree(param_list->nvram_ioctl.value);
		list_del(&param_list->list);
		kfree(param_list);
	}

	return ret;
}


EXPORT_SYMBOL(nvram_get);
EXPORT_SYMBOL(nvram_set);
EXPORT_SYMBOL(nvram_unset);
EXPORT_SYMBOL(nvram_commit);

/* User mode interface below */
int user_nvram_set(anvram_ioctl_t __user *pnvr)
{
	int ret;
	char param[NVRAM_MAX_PARAM_LEN];
	anvram_ioctl_t _nvr,*nvr= &_nvr;
	char tmp[64], *value;
    anvram_param_list_t *param_list;
    struct list_head *plist;

	if (!pnvr)
		return -EINVAL;

	if (copy_from_user(nvr,pnvr,sizeof(anvram_ioctl_t)))
		return -EFAULT;
	if (nvr->size != sizeof(anvram_ioctl_t))
		return -EINVAL;

	if (nvr->len_param > (NVRAM_MAX_PARAM_LEN-1) || nvr->len_param < 0)
		return -EOVERFLOW;

	if (!nvr->param)
		return -EINVAL;

	if (copy_from_user(param, nvr->param, nvr->len_param))
		return -EFAULT;

	param[nvr->len_param] = '\0';
	if (!param[0])
		return -EINVAL;

	if (nvr->len_value > (NVRAM_MAX_VALUE_LEN-1) || nvr->len_value < 0)
		return -EOVERFLOW;

	value = tmp;
	if ((nvr->len_value+1) > sizeof(tmp)) {
		if (!(value = kmalloc(nvr->len_value+1, GFP_KERNEL)))
			return -ENOMEM;
	}

	if (nvr->len_value > 0 && nvr->value) {
		if (copy_from_user(value, nvr->value, nvr->len_value)) {
			ret = -EFAULT;
			goto done;
		}
		value[nvr->len_value] = '\0';
	} else {
		value[0] = '\0';
	}

	if (nvr->value)
	{
		ret = nvram_set_temp(param, value, nvr->is_temp);

        // if list is empty, wake up always.
        if(list_empty(&nvram_param_list) && kwait.iswait)
        {
            kwait.attention = 1;
            wake_up(&kwait.wq);
        }
        else
        {
            list_for_each(plist, &nvram_param_list)
            {
                param_list = list_entry(plist, anvram_param_list_t, list);

                if((param_list->nvram_ioctl.len_param == nvr->len_param) && !strncmp(param_list->nvram_ioctl.param, param, param_list->nvram_ioctl.len_param))
                {
                    if((param_list->nvram_ioctl.len_value) && (param_list->nvram_ioctl.len_value== nvr->len_value) && !strncmp(param_list->nvram_ioctl.value, value, param_list->nvram_ioctl.len_value))
                    {
                        printk("%s %d set nvram [%s:%s], the value is same.\n", __func__, __LINE__, param_list->nvram_ioctl.param, param_list->nvram_ioctl.value);
                    }
                    else
                    {
                        kwait.attention = 1;
                        wake_up(&kwait.wq);
                        break;
                    }
                }
            }
        }
    }
    else
        ret = nvram_unset(param);

done:
	if (value != tmp)
		kfree(value);

	return ret;
}

int user_nvram_get(anvram_ioctl_t __user *pnvr)
{
	int len, ret;
	char param[NVRAM_MAX_PARAM_LEN];
	char *value;
	anvram_ioctl_t _nvr,*nvr= &_nvr;

	if (!pnvr)
		return -EINVAL;
	if (copy_from_user(nvr,pnvr,sizeof(anvram_ioctl_t)))
		return -EFAULT;

	if (nvr->size != sizeof(anvram_ioctl_t))
		return -EINVAL;

	if (nvr->len_value < 1)
		return -EINVAL;

	if (nvr->len_param > (NVRAM_MAX_PARAM_LEN-1) || nvr->len_param < 0)
		return -EINVAL;

	if (nvr->len_param > 0 && nvr->param) {
		if (copy_from_user(param, nvr->param, nvr->len_param))
			return -EFAULT;
		param[nvr->len_param] = '\0';
	} else {
		param[0] = '\0';
	}

	ret = 0;

	if (param[0] == '\0') {
		if (nvr->len_value < NVRAM_SPACE) {
			nvr->len_value = NVRAM_SPACE;
			return -EOVERFLOW;
		}

		if (!(value = (char*)kzalloc(NVRAM_SPACE, GFP_KERNEL)))
			return -ENOMEM;

		ret = nvram_getall(value, NVRAM_SPACE, nvr->is_temp);
		pr_debug("%s: ret: %d, value: %s\n", __func__, ret, value);
		if (ret == 0) {
			nvr->len_value = NVRAM_SPACE;
			if (copy_to_user(nvr->value, value, NVRAM_SPACE))
				ret = -EFAULT;
		}
		kfree(value);
	} else {
		value = nvram_get_internal(param);
		if (value) {
			len = strlen(value) + 1;
			if (nvr->len_value < len)
				ret = -EOVERFLOW;
			else if (user_nvram_decode(value, nvr->value))
			//else if (copy_to_user(nvr->value, value, len))
				ret = -EFAULT;
			nvr->len_value = len;
		} else {
			nvr->len_value = -1;
#ifdef RK3308
			copy_to_user(pnvr,nvr,sizeof(anvram_ioctl_t));
#endif
		}
	}

	return ret;
}

static long dev_nvram_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	switch(req)
	{
	case NVRAM_IOCTL_COMMIT:
		return nvram_commit();
	case NVRAM_IOCTL_CLEAR:
		return nvram_clear();
	case NVRAM_IOCTL_SET:
		return user_nvram_set((anvram_ioctl_t __user *)arg);
	case NVRAM_IOCTL_GET:
		return user_nvram_get((anvram_ioctl_t __user *)arg);
	case NVRAM_IOCTL_ERASE:
		return nvram_erase();
	case NVRAM_IOCTL_WATCH:
		return nvram_watch((anvram_kwait_t __user *)arg);
	}

	return -EINVAL;
}

static int nvram_ver_seq_show(struct seq_file *m, void *v)
{
	struct block_device *bdev;

	seq_printf(m, "nvram driver : v%s\n", NVRAM_DRIVER_VERSION);
	seq_printf(m, "nvram space  : %d\n", NVRAM_SPACE);
	seq_printf(m, "major number : %d\n", nvram_major);

	bdev = blkdev_get_by_dev(MKDEV(NVRAM_FLASH_MAJOR, NVRAM_FLASH_MINOR),
				      FMODE_READ, NULL);
	if (!IS_ERR(bdev)) {
		seq_printf(m, "Flash\n");
		seq_printf(m, "  index      : %d\n", bdev->bd_part->partno);
		seq_printf(m, "  name       : %s\n", bdev->bd_disk->disk_name);
		seq_printf(m, "  size       : %lu\n", bdev->bd_part->nr_sects << 9);
		seq_printf(m, "  erasesize  : %u\n", bdev->bd_block_size);
		seq_printf(m, "  writesize  : %u\n", bdev->bd_block_size);

		blkdev_put(bdev, FMODE_READ);
	}

	return 0;
}

static int nvram_ver_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvram_ver_seq_show, NULL);
}

static const struct file_operations nvram_ver_seq_fops = {
	.open		= nvram_ver_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int dev_nvram_open(struct inode *inode, struct file * file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int dev_nvram_release(struct inode *inode, struct file * file)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations dev_nvram_fops = {
	owner:		THIS_MODULE,
	open:		dev_nvram_open,
	release:	dev_nvram_release,
	unlocked_ioctl:	dev_nvram_ioctl,
};

static void dev_nvram_exit(void)
{
	if (nvram_class) {
		device_destroy(nvram_class, MKDEV(NVRAM_MAJOR, 0));
		class_destroy(nvram_class);
	}

	cdev_del(&cdev);

	if (g_pdentry) {
		remove_proc_entry(PROC_NVRAM_NAME, NULL);
		g_pdentry = NULL;
	}

	if (nvram_major >= 0) {
		unregister_chrdev(nvram_major, "nvram");
		nvram_major = -1;
	}

	_nvram_uninit();

	if (nvram_values) {
		kfree(nvram_values);
		nvram_values = NULL;
	}
}

static int __init dev_nvram_init(void)
{
	int ret, check_res = 1;
	const char *istatus;
	struct nvram_header *header;
	dev_t dev;

	/* Initialize hash table lock */
	spin_lock_init(&nvram_lock);

	nvram_values = kzalloc(NVRAM_VALUES_SPACE, GFP_KERNEL);
	if (!nvram_values)
		return -ENOMEM;

	/* Initialize hash table */
	header = kzalloc(NVRAM_SPACE, GFP_KERNEL);
	if (header) {
		ret = nvram_flash_read_zbuf((unsigned char*)header, NVRAM_SPACE);
		if (ret == 0)
			check_res = _nvram_init(header);

		kfree(header);
	}

	/* initialize cdev with file operations */
	dev = MKDEV(NVRAM_MAJOR, 0);
        cdev_init(&cdev, &dev_nvram_fops);
        cdev.owner = THIS_MODULE;
        cdev.ops = &dev_nvram_fops;
        /* add cdev to the kernel */
        ret = cdev_add(&cdev, dev, 1);
        if (ret) {
		pr_err("NVRAM: unable to add character device\n");
                goto err;
        }

	/* Register char device */
	ret = register_chrdev(NVRAM_MAJOR, "nvram", &dev_nvram_fops);
	if (ret < 0) {
		pr_err("NVRAM: unable to register character device\n");
		goto err;
	}

	nvram_class = class_create(THIS_MODULE, "nvram");
	if (!nvram_class) {
		pr_err("NVRAM: unable to create class\n");
		goto err;
	}
	device_create(nvram_class, NULL, MKDEV(NVRAM_MAJOR, 0), NULL, "nvram");

	nvram_major = NVRAM_MAJOR;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	g_pdentry = proc_create(PROC_NVRAM_NAME, S_IRUGO, NULL, &nvram_ver_seq_fops);
#else
	g_pdentry = create_proc_entry(PROC_NVRAM_NAME, S_IRUGO, NULL);
	if (g_pdentry)
		g_pdentry->proc_fops = &nvram_ver_seq_fops;
#endif
	if (!g_pdentry) {
		ret = -ENOMEM;
		goto err;
	}

	istatus = "MTD is empty";
	if (check_res == 0)
		istatus = "OK";
	else if (check_res == -1)
		istatus = "Signature FAILED!";
	else if (check_res == -2)
		istatus = "Size underflow!";
	else if (check_res == -3)
		istatus = "Size overflow!";
	else if (check_res == -4)
		istatus = "CRC FAILED!";

	pr_info("ASUS NVRAM, v%s. Available space: %d. Integrity: %s\n",
		NVRAM_DRIVER_VERSION, NVRAM_SPACE, istatus);

    init_waitqueue_head(&kwait.wq);

	return 0;

err:
	dev_nvram_exit();
	return ret;
}

late_initcall(dev_nvram_init);
module_exit(dev_nvram_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION(NVRAM_DRIVER_VERSION);
