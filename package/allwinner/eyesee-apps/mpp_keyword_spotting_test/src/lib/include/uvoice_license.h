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
#ifndef UVOICE_LICENSE_PARAM_DEF_H

#define UVOICE_LICENSE_PARAM_DEF_H

/*
 * 用户需要实现的回调函数，用来把https_post_body通过HTTPS协议，POST到UVoice的授权服务器
 * 服务器的返回数据，需要通过此函数return回来。
 * 当下的服务器地址为：
 * https://srv01.51asr.com:8007/asrsn_active2"
 * 因此，这个回调函数的行为应该和以下curl命令等效：
 * curl -k -s --data-binary $https_post_body https://srv01.51asr.com:8007/asrsn_active2"
 */
typedef const char* (*uvoice_auth_callback_t)(const char* https_post_body);

/*--------------------------------------------------------------------------------------*/
/*
 * 结构体说明如下：
 * license: UVoice发布的授权码
 * license_path:此参数在activate_type=0的时候表示服务器授权设备成功后，授权信息写入的文件。
 *              如果activate_type=1,用flash信息进行授权，此时，这个参数表示flash对应的设备文件。
 * uuid：设备的唯一码
 * activate_type:记录授权信息的类型，0：表示以文件方式记录，1：flash,2：efuse
 * auth_cb:用户需要实现的回调函数，参考上边函数指针的说明
 */
typedef struct uv_activate_param {
        char *license;
        char *license_path;
        char *uuid;
        int  activate_type;
        uvoice_auth_callback_t auth_cb;
} uv_activate_param;

#endif

