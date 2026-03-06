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

//#define LOG_NDEBUG 0
//#define LOG_TAG "sample_common_adec"
#include <utils/plat_log.h>

#include <stdio.h>
#include <string.h>

#include "sample_common_adec.h"

/**
  parse wav file header, and set file poiter to data chunk(after "data" and length field).

  @return: wav header size(include "data" and length field)
*/
int ParseWavHeader(FILE *pWavFile, int *pChnNum, int *pSampleRate, int *pBitsPerSample)
{
    struct WaveHeader{
        int riff_id;
        int riff_sz;
        int riff_fmt;
        int fmt_id;
        int fmt_sz;
        short audio_fmt;
        short num_chn;
        int sample_rate;
        int byte_rate;
        short block_align;
        short bits_per_sample;
//        int data_id;
//        int data_sz;
    } header;
    struct RIFFSubChunkHeader
    {
        int mSubChunkId;
        int mSubChunkSize;
    } stSubChunkHeader;
        
    int nHeaderSize;
    int nRdLen = fread(&header, 1, sizeof(struct WaveHeader), pWavFile);
    if(nRdLen != sizeof(struct WaveHeader))
    {
        aloge("fatal error! read file wrong!");
    }
    if(header.fmt_sz != 16)
    {
        alogd("Be careful! wav header fmt_sz[%d]!= 16, need adjust filepos by fseek [%d]bytes", header.fmt_sz, header.fmt_sz - 16);
        fseek(pWavFile, header.fmt_sz - 16, SEEK_CUR);
    }
    fread(&stSubChunkHeader, 1, sizeof(struct RIFFSubChunkHeader), pWavFile);
    //find data sub-chunk
    while(1)
    {
        if(0 != strncmp((char*)&stSubChunkHeader.mSubChunkId, "data", 4))
        {
            alogd("Be careful! wav header has chunk:[0x%x], size:%d, skip", stSubChunkHeader.mSubChunkId, stSubChunkHeader.mSubChunkSize);
            fseek(pWavFile, stSubChunkHeader.mSubChunkSize, SEEK_CUR);
            //read next sub-chunk
            fread(&stSubChunkHeader, 1, sizeof(struct RIFFSubChunkHeader), pWavFile);
        }
        else
        {
            break;
        }
    }
    if(pChnNum)
    {
        *pChnNum = header.num_chn;
    }
    if(pSampleRate)
    {
        *pSampleRate = header.sample_rate;
    }
    if(pBitsPerSample)
    {
        *pBitsPerSample = header.bits_per_sample;
    }
    nHeaderSize = ftell(pWavFile);
    return nHeaderSize;
}

