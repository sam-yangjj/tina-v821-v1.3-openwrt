#include "tkl_audio.h"

#include "plat_log.h"
#include "media/mpi_sys.h"
#include "mm_common.h"
#include "mm_comm_clock.h"
#include "mm_comm_aio.h"
#include <mpi_adec.h>
#include <mpi_clock.h>
#include <tsemaphore.h>
#include <ClockCompPortIndex.h>
#include <confparser.h>
#include <base_list.h>
#include <mpi_ao.h>


typedef struct SampleADecConfig
{
    // char mCompressAudioFilePath[MAX_FILE_PATH_SIZE];    //compressed audio file source
    //char mDecompressAudioFilePath[MAX_FILE_PATH_SIZE];  //target pcm file
    PAYLOAD_TYPE_E mType;                         /*the type of payload*/
    int mSampleRate;
    int mChannelCnt;
    int mBitWidth;
    int mHeaderLen;
    int mFrameSize;
    int mFirstPktLen;
}SampleADecConfig;

typedef struct ADTSHeader{
    unsigned int SyncWord:12;
    unsigned int MpegVer:1;
    unsigned int Layer:2;
    unsigned int ProtectionAbsent:1;
    unsigned int Profile:2;
    unsigned int FreqIdx:4;
    unsigned int PrivateStream:1;
    unsigned int ChnConf:3;
    unsigned int Originality:1;
    unsigned int Home:1;
    unsigned int CRStream:1;
    unsigned int CRStart:1;
    unsigned int FrameLen:13;    //include this header(size: ProtectionAbsent==1?7:9)
    unsigned int BufferFullness:11;
    unsigned int NumFrames:2;
    unsigned int CRC:16;
}ADTSHeader;

typedef struct SampleADecStreamNode
{
    AUDIO_STREAM_S mAStream;
    struct list_head mList;
}SampleADecStreamNode;

typedef struct SampleADecStreamManager
{
    struct list_head mIdleList; //SampleADecStreamNode
    struct list_head mUsingList;
    int mNodeCnt;
    pthread_mutex_t mLock;
    AUDIO_STREAM_S* (*PrefetchFirstIdleStream)(void *pThiz);
    int (*UseStream)(void *pThiz, AUDIO_STREAM_S *pStream);
    int (*ReleaseStream)(void *pThiz, unsigned int nFrameId);
}SampleADecStreamManager;

typedef struct SampleADecContext
{
    SampleADecConfig mConfigPara;

    FILE *mFpAudioFile; //src
    FILE *mFpPcmFile;   //dst
    SampleADecStreamManager mStreamManager;
    pthread_mutex_t mWaitStreamLock;
    int mbWaitStreamFlag;
    cdx_sem_t mSemStreamCome;
    cdx_sem_t mSemEofCome;

    MPP_SYS_CONF_S mSysConf;
    ADEC_CHN mADecChn;
    //MPP_CHN_S mADecChn;
    ADEC_CHN_ATTR_S mADecAttr;

    AUDIO_DEV mAIODev; 
    AO_CHN mAOChn;
    CLOCK_CHN mClkChn;
    AIO_ATTR_S mAIOAttr;
    CLOCK_CHN_ATTR_S mClockChnAttr;

    int ao_eof_flag;
}SampleADecContext;


AUDIO_STREAM_S* SampleADecStreamManager_PrefetchFirstIdleStream(void *pThiz)
{
	// Init output frame buffer
	AUDIO_STREAM_S *pStreamInfo = NULL;
	
	// Get handle of StreamManager
    SampleADecStreamManager *pStreamManager = (SampleADecStreamManager*)pThiz;
    
	// Prefetch frame buffer
    pthread_mutex_lock(&pStreamManager->mLock);
    if (!list_empty(&pStreamManager->mIdleList))
    {// if mIdleList has node, get the first one
        SampleADecStreamNode* pFirstNode = list_first_entry(&pStreamManager->mIdleList, SampleADecStreamNode, mList);
        pStreamInfo = &pFirstNode->mAStream;  //only prefetch, don't change status
    }
    else
    {
        pStreamInfo = NULL;
    }
    pthread_mutex_unlock(&pStreamManager->mLock);
	
    return pStreamInfo;
}

int SampleADecStreamManager_UseStream(void *pThiz, AUDIO_STREAM_S *pStream)
{
    int ret = 0;
    SampleADecStreamManager *pStreamManager = (SampleADecStreamManager*)pThiz;
    if (NULL == pStream)
    {
        aloge("fatal error! pNode == NULL!");
        return -1;
    }
	
    pthread_mutex_lock(&pStreamManager->mLock);
    SampleADecStreamNode *pFirstNode = list_first_entry_or_null(&pStreamManager->mIdleList, SampleADecStreamNode, mList);
    if (pFirstNode)
    {
        if (&pFirstNode->mAStream == pStream)
        {
            list_move_tail(&pFirstNode->mList, &pStreamManager->mUsingList);
        }
        else
        {
            aloge("fatal error! node is not match [%p]!=[%p]", pStream, &pFirstNode->mAStream);
            ret = -1;
        }
    }
    else
    {
        aloge("fatal error! idle list is empty");
        ret = -1;
    }
    pthread_mutex_unlock(&pStreamManager->mLock);
    return ret;
}

int SampleADecStreamManager_ReleaseStream(void *pThiz, unsigned int nStreamId)
{
    int ret = 0;
    SampleADecStreamManager *pStreamManager = (SampleADecStreamManager*)pThiz;
	
    pthread_mutex_lock(&pStreamManager->mLock);
    int bFindFlag = 0;
    SampleADecStreamNode *pEntry, *pTmp;
    list_for_each_entry_safe(pEntry, pTmp, &pStreamManager->mUsingList, mList)
    {
        if(pEntry->mAStream.mId == nStreamId)
        {
            list_move_tail(&pEntry->mList, &pStreamManager->mIdleList);
            bFindFlag = 1;
            break;
        }
    }
    if(0 == bFindFlag)
    {
        aloge("fatal error! StreamId[%d] is not find", nStreamId);
        ret = -1;
    }
    pthread_mutex_unlock(&pStreamManager->mLock);
    return ret;
}

int initSampleADecStreamManager(SampleADecStreamManager *pStreamManager, int nStreamNum)
{
    memset(pStreamManager, 0, sizeof(SampleADecStreamManager));
	
    int err = pthread_mutex_init(&pStreamManager->mLock, NULL);
    if (err!=0)
    {
        aloge("fatal error! pthread mutex init fail!");
    }
	
    INIT_LIST_HEAD(&pStreamManager->mIdleList);
    INIT_LIST_HEAD(&pStreamManager->mUsingList);

	// Create node and malloc buffer
    int i;
    SampleADecStreamNode *pNode = NULL;
    for (i = 0; i < nStreamNum; i++)
    {
        pNode = malloc(sizeof(SampleADecStreamNode));
        memset(pNode, 0, sizeof(SampleADecStreamNode));
        pNode->mAStream.mId     = i;
        pNode->mAStream.mLen    = 4096;    // max size
        pNode->mAStream.pStream = malloc(pNode->mAStream.mLen);
        list_add_tail(&pNode->mList, &pStreamManager->mIdleList);
    }
    pStreamManager->mNodeCnt = nStreamNum;

	// Attach options 
    pStreamManager->PrefetchFirstIdleStream = SampleADecStreamManager_PrefetchFirstIdleStream;
    pStreamManager->UseStream               = SampleADecStreamManager_UseStream;
    pStreamManager->ReleaseStream           = SampleADecStreamManager_ReleaseStream;
	
    return 0;
}

int destroySampleADecStreamManager(SampleADecStreamManager *pStreamManager)
{
	// Check, mUsingList should be empty
    if (!list_empty(&pStreamManager->mUsingList))
    {
        aloge("fatal error! why using list is not empty");
    }
	
	// Check, All node should locates in mIdleList
    int cnt = 0;
    struct list_head *pList;
    list_for_each(pList, &pStreamManager->mIdleList)
    {
        cnt++;
    }
    if(cnt != pStreamManager->mNodeCnt)
    {
        aloge("fatal error! Stream count is not match [%d]!=[%d]", cnt, pStreamManager->mNodeCnt);
    }
	
	// Release buffers
    SampleADecStreamNode *pEntry, *pTmp;
    list_for_each_entry_safe(pEntry, pTmp, &pStreamManager->mIdleList, mList)
    {
        free(pEntry->mAStream.pStream);
        list_del(&pEntry->mList);
        free(pEntry);
    }
	
	// Release other resouces
    pthread_mutex_destroy(&pStreamManager->mLock);
	
    return 0;
}

int initSampleADecContext(SampleADecContext *pContext)
{
    memset(pContext, 0, sizeof(SampleADecContext));
    int err = pthread_mutex_init(&pContext->mWaitStreamLock, NULL);
    if (err!=0)
    {
        aloge("fatal error! pthread mutex init fail!");
    }
	
	err = 0;
    err += cdx_sem_init(&pContext->mSemStreamCome, 0);
    err += cdx_sem_init(&pContext->mSemEofCome, 0);
    if (err!=0)
    {
        aloge("cdx sem init fail!");
    }
    return 0;
}

int destroySampleADecContext(SampleADecContext *pContext)
{
    pthread_mutex_destroy(&pContext->mWaitStreamLock);
    cdx_sem_deinit(&pContext->mSemStreamCome);
    cdx_sem_deinit(&pContext->mSemEofCome);
    return 0;
}

static ERRORTYPE SampleADecCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    ERRORTYPE ret = SUCCESS;
    SampleADecContext *pContext = (SampleADecContext*)cookie;
    if (MOD_ID_ADEC == pChn->mModId)
    {
        if (pChn->mChnId != pContext->mADecChn)
        {
            aloge("fatal error! ADec chnId[%d]!=[%d]", pChn->mChnId, pContext->mADecChn);
        }
        switch (event)
        {
            case MPP_EVENT_RELEASE_AUDIO_BUFFER:
            {
                AUDIO_STREAM_S *pAStream = (AUDIO_STREAM_S*)pEventData;
                pContext->mStreamManager.ReleaseStream(&pContext->mStreamManager, pAStream->mId);
				
                pthread_mutex_lock(&pContext->mWaitStreamLock);
                if (pContext->mbWaitStreamFlag)
                {
                    pContext->mbWaitStreamFlag = 0;
                    cdx_sem_up(&pContext->mSemStreamCome);
                }
                pthread_mutex_unlock(&pContext->mWaitStreamLock);
                break;
            }
            case MPP_EVENT_NOTIFY_EOF:
            {
                alogd("ADec channel notify APP that decode complete!");

                cdx_sem_signal(&pContext->mSemEofCome); 
                
                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_ADEC_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else if(MOD_ID_AO == pChn->mModId)
    {
        if(pChn->mChnId != pContext->mAOChn)
        {
            aloge("fatal error! AO chnId[%d]!=[%d]", pChn->mChnId, pContext->mAOChn);
        }
        switch(event)
        {
            case MPP_EVENT_NOTIFY_EOF:
            {
                alogd("AO channel notify APP that play complete!");
                
                pContext->ao_eof_flag = 1;

                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_AO_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else
    {
        aloge("fatal error! why modId[0x%x]?", pChn->mModId);
        ret = FAILURE;
    }
    return ret;
}


int map_FreqIdx_to_SampleRate(int idx)
{
    switch (idx)
    {
    case 0:
        return 96000;
    case 1:
        return 88200;
    case 2:
        return 64000;
    case 3:
        return 48000;
    case 4:
        return 44100;
    case 5:
        return 32000;
    case 6:
        return 24000;
    case 7:
        return 22050;
    case 8:
        return 16000;
    case 9:
        return 12000;
    case 10:
        return 11025;
    case 11:
        return 8000;
    case 12:
        return 7370;
    case 13:
    case 14:
    case 15:
    default:
        aloge("wrong frequence idx [%d]", idx);
        return 8000;
    }
}

void parseADTSHeader(SampleADecConfig *pConf, ADTSHeader *header)
{
    pConf->mType = PT_AAC;
    unsigned char *ptr = (unsigned char*)header;
    pConf->mHeaderLen = ((ptr[1]&0x01)==1) ? 7:9;
    pConf->mBitWidth = 16;
    pConf->mChannelCnt = (ptr[2]&0x1)<<3 | (ptr[3]&0xc0)>>6;
    int idx = (ptr[2]&0x3c)>>2;
    pConf->mSampleRate = map_FreqIdx_to_SampleRate(idx);
    pConf->mFirstPktLen = ((ptr[3]&0x03)<<11) | (ptr[4]<<3) | ((ptr[5]&0xe0)>>5);
}

int parseAudioFile(SampleADecConfig *pConf, FILE *fp)
{
    int ret = 0;
    unsigned char buf[4];
	if(pConf->mType == PT_G711U || pConf->mType == PT_G711A)
    {
		pConf->mHeaderLen = 1024;
       fseek(fp, 0, SEEK_SET);
	}
	else if(pConf->mType == PT_G726 || pConf->mType == PT_G726U)
	{
		pConf->mHeaderLen = 512;
       fseek(fp, 0, SEEK_SET);
	}
	else
	{
		fseek(fp, 0, SEEK_SET);
		fread(buf, 1, 4, fp);
		if ((buf[0] == 0xff) && (buf[1]>>4 == 0xf))
		{
			alogd("audio file type is AAC!");
			fseek(fp, 0, SEEK_SET);
			ADTSHeader header;
			fread(&header, 1, 9, fp);
			parseADTSHeader(pConf, &header);
			fseek(fp, pConf->mFirstPktLen, SEEK_SET);

			fseek(fp, 0, SEEK_SET);
		}
		else
		{
			aloge("unknown audio file! can not decode other type now!");
			ret = -1;
		}
	}

    return ret;
}

int extractStreamPacket(AUDIO_STREAM_S *pStreamInfo, FILE *fp, SampleADecConfig *pConf)
{
    static int id;
    static long long pts;
    int read_len, pkt_sz,bs_sz;
    ADTSHeader header; 

	if(pConf->mType == PT_G711U || pConf->mType == PT_G711A)
	{
		bs_sz = 1024;// pkt_sz - pConf->mHeaderLen;
	}
	else if(pConf->mType == PT_G726 || pConf->mType == PT_G726U)
	{
		bs_sz = 512;// pkt_sz - pConf->mHeaderLen;
	}
	else
	{
		read_len = fread(&header, 1, pConf->mHeaderLen, fp);
		if (0 == read_len)
		{
			alogd("zjx_err:%d",pConf->mHeaderLen);
			return 0;
		}
		unsigned char *ptr = (unsigned char *)&header;
		pkt_sz = ((ptr[3]&0x03)<<11) | (ptr[4]<<3) | ((ptr[5]&0xe0)>>5);
		bs_sz = pkt_sz - pConf->mHeaderLen;
	}
    read_len = fread(pStreamInfo->pStream, 1, bs_sz, fp);

//    aloge("zjx_frm:%d-%d",bs_sz,read_len);
    pStreamInfo->mLen = read_len;
    pStreamInfo->mId  = id++;
    pStreamInfo->mTimeStamp = pts;
    pts += 23;
    return read_len;
}


int tkl_audio_play_aac(INT32_T card, TKL_AO_CHN_E chn, char *FilePath)
{
    alogd("enter");
    int result;
    SampleADecContext stContext;
    initSampleADecContext(&stContext);

    // init Stream manager
    initSampleADecStreamManager(&stContext.mStreamManager, 5);

    stContext.mConfigPara.mType = 37;//aac
    stContext.mConfigPara.mSampleRate = 16000;
    stContext.mConfigPara.mBitWidth = 16;
    stContext.mConfigPara.mChannelCnt = 1;

    stContext.mFpAudioFile = fopen(FilePath, "rb");
    if (!stContext.mFpAudioFile) {
        aloge("fatal error! can't open pcm file[%s]", FilePath);
        result = -1;
        goto _exit;
    } else {
        result = parseAudioFile(&stContext.mConfigPara, stContext.mFpAudioFile);
        if (result) {
            aloge("some wang happened! need exit!!!");
            goto _exit;
        }
    }

    memset(&stContext.mAIOAttr, 0, sizeof(AIO_ATTR_S));
	stContext.mAIOAttr.mChnCnt = stContext.mConfigPara.mChannelCnt;
	stContext.mAIOAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)stContext.mConfigPara.mSampleRate;
	stContext.mAIOAttr.enBitwidth = (AUDIO_BIT_WIDTH_E)(stContext.mConfigPara.mBitWidth/8-1);
    stContext.mAIODev = 0; 
    //AW_MPI_AO_SetPubAttr(stContext.mAIODev,stContext.mAOChn, &stContext.mAIOAttr);
    //AW_MPI_AO_Enable(stContext.mAIODev,stContext.mAOChn); 
    ERRORTYPE ret;
    BOOL bSuccessFlag = FALSE;

    stContext.mAOChn = 0;
    while(stContext.mAOChn < AIO_MAX_CHN_NUM)
    {
        ret = AW_MPI_AO_CreateChn(stContext.mAIODev, stContext.mAOChn);
        if(SUCCESS == ret)
        {
            bSuccessFlag = TRUE;
            alogd("create ao channel[%d] success!", stContext.mAOChn);
            break;
        }
        else if (ERR_AO_EXIST == ret)
        {
            alogd("ao channel[%d] exist, find next!", stContext.mAOChn);
            stContext.mAOChn++;
        }
        else if(ERR_AO_NOT_ENABLED == ret)
        {
            aloge("audio_hw_ao not started!");
            break;
        }
        else
        {
            aloge("create ao channel[%d] fail! ret[0x%x]!", stContext.mAOChn, ret);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        stContext.mAOChn = MM_INVALID_CHN;
        aloge("fatal error! create ao channel fail!");
        goto _exit;
    }

    //register vo callback
    MPPCallbackInfo voCallback = {&stContext, SampleADecCallbackWrapper};
    AW_MPI_AO_RegisterCallback(stContext.mAIODev, stContext.mAOChn, &voCallback); 
    
    // AW_MPI_AO_SetDevVolume(stContext.mAIODev,50);
    //config adec chn attr.
    stContext.mADecAttr.mType         = stContext.mConfigPara.mType;
    stContext.mADecAttr.sampleRate    = stContext.mConfigPara.mSampleRate;
	stContext.mADecAttr.channels      = stContext.mConfigPara.mChannelCnt;
	stContext.mADecAttr.bitsPerSample = stContext.mConfigPara.mBitWidth;
    if(stContext.mConfigPara.mType == PT_G726 || stContext.mConfigPara.mType == PT_G726U)
	{
		//stContext.mADecAttr.g726_src_enc_type = 1;
		stContext.mADecAttr.bitRate = 32000;
	}

    //create ADec channel.
    stContext.mADecChn = 0;
    while (stContext.mADecChn < ADEC_MAX_CHN_NUM)
    {
        ret = AW_MPI_ADEC_CreateChn(stContext.mADecChn, &stContext.mADecAttr);
        if(SUCCESS == ret)
        {
            bSuccessFlag = TRUE;
            alogd("create ADec channel[%d] success!", stContext.mADecChn);
            break;
        }
        else if (ERR_ADEC_EXIST == ret)
        {
            alogd("ADec channel[%d] exist, find next!", stContext.mADecChn);
            stContext.mADecChn++;
        }
        else
        {
            aloge("create ADec channel[%d] fail! ret[0x%x]!", stContext.mADecChn, ret);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        stContext.mADecChn = MM_INVALID_CHN;
        aloge("fatal error! create ADec channel fail!");
    }
	
	// Set callback functions
    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)&stContext;
    cbInfo.callback = (MPPCallbackFuncType)&SampleADecCallbackWrapper;
    AW_MPI_ADEC_RegisterCallback(stContext.mADecChn, &cbInfo);


    MPP_CHN_S AdecChn = {MOD_ID_ADEC, stContext.mAIODev, stContext.mADecChn};
    MPP_CHN_S AOChn = {MOD_ID_AO, stContext.mAIODev, stContext.mAOChn};

    AW_MPI_SYS_Bind(&AdecChn, &AOChn); 
	
    // Start ADec chn.
    AW_MPI_ADEC_StartRecvStream(stContext.mADecChn);

    bSuccessFlag = FALSE;
    stContext.mClockChnAttr.nWaitMask = 0;
    stContext.mClockChnAttr.nWaitMask |= 1<<CLOCK_PORT_INDEX_AUDIO;
    stContext.mClkChn = 0;
    while(stContext.mClkChn < CLOCK_MAX_CHN_NUM)
    {
        ret = AW_MPI_CLOCK_CreateChn(stContext.mClkChn, &stContext.mClockChnAttr);
        if(SUCCESS == ret)
        {
            bSuccessFlag = TRUE;
            alogd("create clock channel[%d] success!", stContext.mClkChn);
            break;
        }
        else if(ERR_CLOCK_EXIST == ret)
        {
            alogd("clock channel[%d] is exist, find next!", stContext.mClkChn);
            stContext.mClkChn++;
        }
        else
        {
            alogd("create clock channel[%d] ret[0x%x]!", stContext.mClkChn, ret);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        stContext.mClkChn = MM_INVALID_CHN;
        aloge("fatal error! create clock channel fail!");
    }

    //bind clock and ao
    MPP_CHN_S ClockChn = {MOD_ID_CLOCK, 0, stContext.mClkChn};
//    AW_MPI_SYS_Bind(&ClockChn, &AOChn);

    //start clk chn.
    AW_MPI_CLOCK_Start(stContext.mClkChn);
    //start ao chn.
    AW_MPI_AO_StartChn(stContext.mAIODev, stContext.mAOChn); 

    // Read stream from file.
    int nStreamSize;
    AUDIO_STREAM_S nStreamInfo;
    nStreamInfo.pStream = malloc(4096); // max size
    if(NULL == nStreamInfo.pStream)
    {
        aloge("malloc_4k_failed");
        goto _exit_2;
    }

    while (1)
    {
        // get stream from file
        nStreamSize = extractStreamPacket(&nStreamInfo, stContext.mFpAudioFile, &stContext.mConfigPara);
        if (0 == nStreamSize)
        {// if no available audio data, clean output buffer and set eof flag
            if (feof(stContext.mFpAudioFile))
            {
                alogd("read file finish!");
            }

            AW_MPI_ADEC_SetStreamEof(stContext.mADecChn, 1);
            
            // aloge("zjx_w_eof");
            cdx_sem_wait(&stContext.mSemEofCome);
            // aloge("zjx_wed_eof");
            
            AW_MPI_AO_SetStreamEof(stContext.mAIODev, stContext.mAOChn, 1, 1);

            while(!stContext.ao_eof_flag)
            {
                usleep(5000);
            }

            stContext.ao_eof_flag = 0; 
            break;
        }
        // send stream to adec
        int nWaitTimeMs = 5*1000;
        ret = AW_MPI_ADEC_SendStream(stContext.mADecChn, &nStreamInfo, nWaitTimeMs);
        if (ret != SUCCESS)
        {
            alogw("send StreamId[%d] with [%d]ms timeout fail?!", nStreamInfo.mId, nWaitTimeMs);
        }
    }

    free(nStreamInfo.pStream);


_exit_2:
    // Stop ADec channel.
    AW_MPI_ADEC_StopRecvStream(stContext.mADecChn);
    AW_MPI_ADEC_DestroyChn(stContext.mADecChn);

    AW_MPI_CLOCK_Stop(stContext.mClkChn);
    AW_MPI_CLOCK_DestroyChn(stContext.mClkChn);

    AW_MPI_AO_StopChn(stContext.mAIODev, stContext.mAOChn);
    AW_MPI_AO_DestroyChn(stContext.mAIODev, stContext.mAOChn);

    stContext.mADecChn = MM_INVALID_CHN;

_exit:
    destroySampleADecStreamManager(&stContext.mStreamManager);

    if (stContext.mFpAudioFile)
    {
        fclose(stContext.mFpAudioFile);
        stContext.mFpAudioFile = NULL;
    }
    destroySampleADecContext(&stContext);
    alogd("audio_play_aac result: %s", ((0 == result) ? "success" : "fail"));
    return result;
}
