#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <glogCWrapper/log.h>
#include "motion_detect_demo.h"

#define U_ALIGN(y, x) (((x) + ((y)-1)) & ~((y)-1))
#define D_ALIGN(y, x) ((x) & ~((y)-1))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FREE(buf) if((buf)!=NULL){free(buf);(buf)=NULL;}
#define FCLOSE(fp) if((fp)!=NULL){fclose(fp);(fp)=NULL;}

static long long mdGetNowUs(void)
{
    long long curr;
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    curr = ((long long)(t.tv_sec)*1000000000LL + t.tv_nsec)/1000LL;
    return curr;
}

static int mdReadYuvFromFile(unsigned char *pAddr, int nStride, int nWidth, int nHeight, FILE *fpIn)
{
    int nRdLen = 0, nTotalLen = 0;
    int nYSize = nWidth * nHeight;
    int nCSize = nYSize >> 1;

    if (nStride == nWidth)
    {
        nRdLen = fread(pAddr, 1, nYSize, fpIn);
        nTotalLen = nRdLen;
    }
    else
    {
        int h = 0;
        unsigned char *pRd = pAddr;
        for (h = 0; h < nHeight; h++)
        {
            nRdLen = fread(pRd, 1, nWidth, fpIn);
            pRd += nStride;
            nTotalLen += nRdLen;
            if (nRdLen != nWidth)
                break;;
        }
    }

    return nTotalLen;
}

static int mdGetOneFrameResult(SAMPLE_MD_S *pSample)
{
    SAMPLE_MD_CFG_S *pCfg =                &pSample->mCfg;
    MOTION_DETECT_DYNAMIC_CFG_S *pDynCfg = &pCfg->mMdDynCfg;
    MOTION_DETECT_RESULT_S *pResult =      &pSample->mResult;
    MOTION_DETECT_REGION_S *pRegion =      NULL;
    int nLastHorRegNum =                   pResult->nHorRegionNum;
    int nLastVerRegNum =                   pResult->nVerRegionNum;
    int nLastTotalRegNum =                 nLastHorRegNum * nLastVerRegNum;
    int nCurrTotalRegNum =                 pDynCfg->nHorRegionNum * pDynCfg->nVerRegionNum;
    int x = 0, y = 0, idx = 0;

    if (nLastTotalRegNum < nCurrTotalRegNum || !pResult->mRegion)
    {
        FREE(pResult->mRegion);
        pResult->mRegion = (MOTION_DETECT_REGION_S *)malloc(nCurrTotalRegNum * sizeof(MOTION_DETECT_REGION_S));
        if (!pResult->mRegion)
        {
            ALOGE("pResult->mRegion malloc failed!\n");
            return -1;
        }
    }

    MotionDetectGetResult(pSample->mHandle, pResult);

    fprintf(pCfg->mFdOut, "F%d Motion:%d, cost %lldus\n",
        pSample->mWrFrmCnt, pResult->nMotionRegionNum, pSample->mOneFrmTime);
    pRegion = pResult->mRegion;
    if (pCfg->mTotalResultEn)
    {
        for (idx = 0, y = 0; y < pResult->nVerRegionNum; y++)
        {
            fprintf(pCfg->mFdOut, "Region[%02d][]:", y);
            for (x = 0; x < pResult->nHorRegionNum; x++)
            {
                fprintf(pCfg->mFdOut, " (%04d,%04d)(%04d,%04d)%04d,%04d,%02d%%,%d",
                    pRegion[idx].nPixXBgn, pRegion[idx].nPixYBgn,
                    pRegion[idx].nPixXEnd, pRegion[idx].nPixYEnd,
                    pRegion[idx].nLargeMadNum, pRegion[idx].nTotalNum,
                    pRegion[idx].nLargeMadNum * 100 / pRegion[idx].nTotalNum,
                    pRegion[idx].bIsMotion);
                idx++;
            }
            fprintf(pCfg->mFdOut, "\n");
        }
    }
    else
    {
        for (idx = 0, y = 0; y < pResult->nVerRegionNum; y++)
        {
            fprintf(pCfg->mFdOut, "Region[%02d][]:", y);
            for (x = 0; x < pResult->nHorRegionNum; x++)
            {
                fprintf(pCfg->mFdOut, " %d", pRegion[idx].bIsMotion);
                idx++;
            }
            fprintf(pCfg->mFdOut, "\n");
        }
    }
    fprintf(pCfg->mFdOut, "\n");

    if (pSample->mWrFrmCnt % 20 == 0)
    {
        ALOGW("Finish process F%d.", pSample->mWrFrmCnt);
    }

    return 0;
}

static void *mdReadFrameThread(void *pThreadData)
{
    SAMPLE_MD_S *pSample = (SAMPLE_MD_S *)pThreadData;
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    SAMPLE_MD_FILE_S *pFile = &pSample->mFile;
    SAMPLE_MD_BUF_Q *pBufList = &pSample->mPicBufQ;
    MOTION_DETECT_STATIC_CFG_S *pStaCfg = &pCfg->mMdStaCfg;
    int nFlushSize = pStaCfg->nPicStride * pStaCfg->nPicHeight;
    long long nForward = pFile->mCSize;
    long long nBackward = pFile->mFrameSize + pFile->mYSize;
    long long nPosOffset = 2 * pFile->mFrameSize;

    while (pSample->mRdFrmCnt < pCfg->mReadFrameNum)
    {
        pthread_mutex_lock(&pBufList->mIdleListLock);
        if (list_empty(&pBufList->mIdleList))
        {
            ALOGI("F%d-%d mIdleList is empty!\n", pSample->mRdFrmCnt, pSample->mWrFrmCnt);
        }
        else
        {
            MD_FRAME_NODE_S *pIdleFrame = list_first_entry(&pBufList->mIdleList, MD_FRAME_NODE_S, mList);
            ALOGI("F%d-%d VFrame[%d]:%p, mpVirAddr[0]:%p\n", pSample->mRdFrmCnt, pSample->mWrFrmCnt,
                pIdleFrame->mFrame.nIdx, pIdleFrame, pIdleFrame->mFrame.pVirAddr);
            mdReadYuvFromFile((unsigned char *)pIdleFrame->mFrame.pVirAddr, pStaCfg->nPicStride,
                pStaCfg->nPicWidth, pStaCfg->nPicHeight, pCfg->mFdIn);

            pFile->mCurPos += pFile->mFrameSize;
            if (pFile->mCurPos == pFile->mSeekEnd)
            {
                pFile->mIsBackword = 1;
            }
            if (pFile->mIsBackword)
            {
                fseek(pCfg->mFdIn, -nBackward, SEEK_CUR);
                pFile->mCurPos -= nPosOffset;
            }
            else
            {
                fseek(pCfg->mFdIn, nForward, SEEK_CUR);
            }
            if (pFile->mCurPos == pFile->mSeekBgn)
            {
                pFile->mIsBackword = 0;
            }

            pSample->mRdFrmCnt++;
            pthread_mutex_lock(&pBufList->mReadyListLock);
            list_move_tail(&pIdleFrame->mList, &pBufList->mReadyList);
            pthread_mutex_unlock(&pBufList->mReadyListLock);
        }
        pthread_mutex_unlock(&pBufList->mIdleListLock);

        if (pSample->mRdFrmCnt > 0)
        {
            pthread_mutex_lock(&pBufList->mReadyListLock);
            while (1)
            {
                if (list_empty(&pBufList->mReadyList))
                {
                    ALOGI("F%d-%d mReadyList is empty!\n", pSample->mRdFrmCnt, pSample->mWrFrmCnt);
                    break;
                }
                MD_FRAME_NODE_S *pReadyFrame = list_first_entry(&pBufList->mReadyList, MD_FRAME_NODE_S, mList);
                pthread_mutex_lock(&pBufList->mUseListLock);
                list_move_tail(&pReadyFrame->mList, &pBufList->mUseList);
                pthread_mutex_unlock(&pBufList->mUseListLock);
            }
            pthread_mutex_unlock(&pBufList->mReadyListLock);
        }
    }

    return NULL;
}

static void *mdProcessFrameThread(void *pThreadData)
{
    SAMPLE_MD_S *pSample = (SAMPLE_MD_S *)pThreadData;
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    SAMPLE_MD_BUF_Q *pBufList = &pSample->mPicBufQ;
    int ret = 0;

    while (pSample->mWrFrmCnt < pCfg->mReadFrameNum)
    {
        pthread_mutex_lock(&pBufList->mUseListLock);
        if (list_empty(&pBufList->mUseList))
        {
            ALOGI("F%d-%d mUseList is empty\n", pSample->mRdFrmCnt, pSample->mWrFrmCnt);
        }
        else
        {
            MD_FRAME_NODE_S *pUseFrame = list_first_entry(&pBufList->mUseList, MD_FRAME_NODE_S, mList);
            if (!pUseFrame->mFrame.pVirAddr)
            {
                ALOGI("F%d-%d VFrame[%d], pVirAddr is NULL!\n",
                    pSample->mRdFrmCnt, pSample->mWrFrmCnt, pUseFrame->mFrame.nIdx);
            }
            else
            {
                long long nFrmBgnTime, nFrmEndTime;
                MotionDetectSetParam(pSample->mHandle, &pCfg->mMdDynCfg);
                nFrmBgnTime = mdGetNowUs();
                MotionDetectProcessOneFrame(pSample->mHandle, pUseFrame->mFrame.pVirAddr);
                nFrmEndTime = mdGetNowUs();
                pSample->mOneFrmTime = nFrmEndTime - nFrmBgnTime;
                pthread_mutex_lock(&pBufList->mIdleListLock);
                list_move_tail(&pUseFrame->mList, &pBufList->mIdleList);
                pthread_mutex_unlock(&pBufList->mIdleListLock);

                mdGetOneFrameResult(pSample);
                pSample->mWrFrmCnt++;
            }
        }
        pthread_mutex_unlock(&pBufList->mUseListLock);
    }

    return NULL;
}

static void mdCheckYuvFile(SAMPLE_MD_S *pSample)
{
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    SAMPLE_MD_FILE_S *pFile = &pSample->mFile;

    pFile->mYSize =            pCfg->mMdStaCfg.nPicWidth * pCfg->mMdStaCfg.nPicHeight;
    pFile->mCSize =            pFile->mYSize >> 1;
    pFile->mFrameSize =        pFile->mYSize + pFile->mCSize;
    fseek(pCfg->mFdIn, 0, SEEK_END);
    pFile->mFileLen =          ftell(pCfg->mFdIn);
    pFile->mFrameNum =         pFile->mFileLen / pFile->mFrameSize;
    pFile->mFrameNum =         pFile->mFrameNum - pCfg->mFrontCutFrameNum - pCfg->mBackCutFrameNum;
    pFile->mSeekBgn =          pCfg->mFrontCutFrameNum * pFile->mFrameSize;
    pFile->mSeekEnd =          pFile->mSeekBgn + pFile->mFrameSize * (pFile->mFrameNum - 1);
    pFile->mCurPos =           pFile->mSeekBgn;
    fseek(pCfg->mFdIn, pFile->mSeekBgn, SEEK_SET);
    ALOGW("mYSize:%d, mFrameSize:%d, mFrameNum:%d, mSeekBgn:%lld, mSeekEnd:%lld\n",
        pFile->mYSize, pFile->mFrameSize, pFile->mFrameNum, pFile->mSeekBgn, pFile->mSeekEnd);
}

static int mdLoadConfigPara(SAMPLE_MD_S *pPsample)
{
    int ret = 0;
    unsigned char *ptr = NULL;
    SAMPLE_MD_CFG_S *pCfg = &pPsample->mCfg;

    // for sample common paramter
    pCfg->mReadFrameNum = 240;
    pCfg->mSrcBufNum = 3;
    pCfg->mTotalResultEn = 0;
    pCfg->mReadLocalFileEn = 1;

    strncpy(pCfg->mOutputFile, "/mnt/extsd/motion_detect/md_result.txt", MAX_FILE_PATH_LEN-1);

    // for get picture from yuv file
    strncpy(pCfg->mInputFile, "/mnt/extsd/motion_detect/480x272/ms_480x272_3m0_240f.yuv", MAX_FILE_PATH_LEN-1);
    pCfg->mFrontCutFrameNum = 0;
    pCfg->mBackCutFrameNum = 0;

    // for motion detect static config
    pCfg->mMdStaCfg.nPicStride = 480;
    pCfg->mMdStaCfg.nPicWidth = 480;
    pCfg->mMdStaCfg.nPicStride = MAX(pCfg->mMdStaCfg.nPicWidth, pCfg->mMdStaCfg.nPicStride);
    pCfg->mMdStaCfg.nPicStride = U_ALIGN(MOTION_DETECT_ALIGN, pCfg->mMdStaCfg.nPicStride);
    pCfg->mMdStaCfg.nPicHeight = 272;
    pCfg->mMdStaCfg.bCropEn = 0;
    pCfg->mMdStaCfg.nCropLeft = 128;
    pCfg->mMdStaCfg.nCropTop = 96;
    pCfg->mMdStaCfg.nCropWidth = 256;
    pCfg->mMdStaCfg.nCropHeight = 160;
    ALOGW("mMdStaCfg: {%d, %d, %d, %d, %d, %d, %d, %d}\n",
        pCfg->mMdStaCfg.nPicStride, pCfg->mMdStaCfg.nPicWidth, pCfg->mMdStaCfg.nPicHeight, pCfg->mMdStaCfg.bCropEn,
        pCfg->mMdStaCfg.nCropLeft, pCfg->mMdStaCfg.nCropTop, pCfg->mMdStaCfg.nCropWidth, pCfg->mMdStaCfg.nCropHeight);

    // for motion detect dynamic config
    pCfg->mMdDynCfg.bDefaultCfgDis = 1;
    pCfg->mMdDynCfg.nHorRegionNum = 30;
    pCfg->mMdDynCfg.nVerRegionNum = 17;
    pCfg->mMdDynCfg.bMorphologEn = 0;
    pCfg->mMdDynCfg.nLargeMadTh = 10;
    pCfg->mMdDynCfg.nBackgroundWeight = 0;
    pCfg->mMdDynCfg.fLargeMadRatioTh = 10.0f;
    ALOGW("mMdDynCfg: {%d, %d, %d, %d, %d, %d, %d}\n",
        pCfg->mMdDynCfg.bDefaultCfgDis, pCfg->mMdDynCfg.nHorRegionNum, pCfg->mMdDynCfg.nVerRegionNum,
        pCfg->mMdDynCfg.bMorphologEn, pCfg->mMdDynCfg.nLargeMadTh, pCfg->mMdDynCfg.nBackgroundWeight,
        (int)pCfg->mMdDynCfg.fLargeMadRatioTh);

    return 0;
}

static int mdBufMgrCreate(SAMPLE_MD_S *pSample)
{
    int k = 0, ret = 0;
    SAMPLE_MD_BUF_Q *pBufList = &pSample->mPicBufQ;
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    MOTION_DETECT_STATIC_CFG_S *pStaCfg = &pCfg->mMdStaCfg;
    unsigned int nWidth = pStaCfg->nPicStride;
    unsigned int nHeight = U_ALIGN(MOTION_DETECT_ALIGN, pStaCfg->nPicHeight); /* Should use 16align to malloc input buffer */
    unsigned int nSize = nWidth * nHeight;

    INIT_LIST_HEAD(&pBufList->mIdleList);
    INIT_LIST_HEAD(&pBufList->mReadyList);
    INIT_LIST_HEAD(&pBufList->mUseList);

    pthread_mutex_init(&pBufList->mIdleListLock, NULL);
    pthread_mutex_init(&pBufList->mReadyListLock, NULL);
    pthread_mutex_init(&pBufList->mUseListLock, NULL);

    for (k = 0; k < pCfg->mSrcBufNum; k++)
    {
        MD_FRAME_BUF_S *pFrame = NULL;
        MD_FRAME_NODE_S *pFrameNode = (MD_FRAME_NODE_S *)malloc(sizeof(MD_FRAME_NODE_S));
        if (!pFrameNode)
        {
            ALOGE("alloc MD_FRAME_NODE_S[%d] failed!\n", k);
            ret = -1;
            break;
        }
        memset(pFrameNode, 0, sizeof(MD_FRAME_NODE_S));
        pFrame = &pFrameNode->mFrame;

        pFrame->pVirAddr = (void *)malloc(nSize);
        if (!pFrame->pVirAddr)
        {
            ALOGE("alloc pFrameNode[%d].VirAddr failed!\n", k);
            FREE(pFrameNode);
            ret = -1;
            break;
        }

        ALOGW("pFrameNode[%d]:%p, mList:%p, pVirAddr:%p, nSize:%d\n",
            k, pFrameNode, &pFrameNode->mList, pFrame->pVirAddr, nSize);

        pFrameNode->mFrame.nIdx = k;
        list_add_tail(&pFrameNode->mList, &pBufList->mIdleList);
        pBufList->mFrameNum++;
    }

    return ret;
}


static int mdBufMgrDestroy(SAMPLE_MD_S *pSample)
{
    SAMPLE_MD_BUF_Q *pBufList = &pSample->mPicBufQ;
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    MD_FRAME_NODE_S *pEntry = NULL, *pTmp = NULL;
    struct list_head *pList[3] = {&pBufList->mIdleList, &pBufList->mReadyList, &pBufList->mUseList};
    pthread_mutex_t *pLock[3] = {&pBufList->mIdleListLock, &pBufList->mReadyListLock, &pBufList->mUseListLock};
    int nBufNum = 0, nIdx;

    if (!pBufList)
    {
        ALOGW("pBufList is NULL!\n");
        return -1;
    }

    for (nIdx = 0; nIdx < 3; nIdx++)
    {
        pthread_mutex_lock(pLock[nIdx]);
        if (!list_empty(pList[nIdx]))
        {
            list_for_each_entry_safe(pEntry, pTmp, pList[nIdx], mList)
            {
                list_del(&pEntry->mList);
                FREE(pEntry->mFrame.pVirAddr);
                FREE(pEntry);
                nBufNum++;
            }
        }
        pthread_mutex_unlock(pLock[nIdx]);
    }

    if (nBufNum != pBufList->mFrameNum)
        ALOGW("Frame node number is not match[%d][%d]\n", nBufNum, pBufList->mFrameNum);

    for (nIdx = 0; nIdx < 3; nIdx++)
    {
        pthread_mutex_destroy(pLock[nIdx]);
    }

    return 0;
}

static int mdMainReadFromLocalFile(SAMPLE_MD_S *pSample)
{
    int ret = 0;
    long long nBgnTime = 0, nEndTime = 0;
    SAMPLE_MD_CFG_S *pCfg = &pSample->mCfg;
    MOTION_DETECT_STATIC_CFG_S *pStaCfg = &pCfg->mMdStaCfg;

    pCfg->mFdIn = fopen(pCfg->mInputFile, "rb");
    if (!pCfg->mFdIn)
    {
        ALOGE("mFdIn open failed!\n");
        ret = -8;
        goto MAIN_FILE_FINISH;
    }

    mdCheckYuvFile(pSample);

    nBgnTime = mdGetNowUs();

    ret = pthread_create(&pSample->mReadFrmThdId, NULL, mdReadFrameThread, pSample);
    if (ret || !pSample->mReadFrmThdId)
    {
        ALOGE("mdReadFrameThread create failed!\n");
        ret = -9;
        goto MAIN_FILE_FINISH;
    }

    ret = pthread_create(&pSample->mProcessFrmThdId, NULL, mdProcessFrameThread, pSample);
    if (ret || !pSample->mProcessFrmThdId)
    {
        ALOGE("mdProcessFrameThread create failed!\n");
        ret = -10;
        goto MAIN_FILE_FINISH;
    }

MAIN_FILE_FINISH:

    pthread_join(pSample->mProcessFrmThdId, (void *)&ret);
    pthread_join(pSample->mReadFrmThdId, (void *)&ret);

    nEndTime = mdGetNowUs();
    pSample->mTotalFrmTime = nEndTime - nBgnTime;

    FCLOSE(pCfg->mFdIn);

    return ret;
}


int main(int argc, char **argv)
{
	int ret = 0;
    int nTotalRegionNum = 0;
    SAMPLE_MD_S *pSample = NULL;
    SAMPLE_MD_CFG_S *pCfg = NULL;
    MOTION_DETECT_STATIC_CFG_S *pStaCfg = NULL;
    MOTION_DETECT_DYNAMIC_CFG_S *pDynCfg = NULL;

    GLogConfig stGLogConfig =
    {
        .FLAGS_logtostderr = 1,
        .FLAGS_colorlogtostderr = 1,
        .FLAGS_stderrthreshold = _GLOG_WARN,
        .FLAGS_minloglevel = _GLOG_WARN,
        .FLAGS_logbuflevel = -1,
        .FLAGS_logbufsecs = 0,
        .FLAGS_max_log_size = 1,
        .FLAGS_stop_logging_if_full_disk = 1,
    };
    strcpy(stGLogConfig.LogDir, "/tmp/log");
    strcpy(stGLogConfig.InfoLogFileNameBase, "LOG-");
    strcpy(stGLogConfig.LogFileNameExtension, "MD-");
    log_init(argv[0], &stGLogConfig);

    pSample = (SAMPLE_MD_S *)malloc(sizeof(SAMPLE_MD_S));
    if (!pSample)
    {
        ALOGE("pSample malloc failed!\n");
        ret = -1;
        goto MAIN_FINISH;
    }
    memset(pSample, 0, sizeof(SAMPLE_MD_S));

    pCfg = &pSample->mCfg;
    pStaCfg = &pCfg->mMdStaCfg;
    pDynCfg = &pCfg->mMdDynCfg;

    if (mdLoadConfigPara(pSample))
    {
        ALOGE("mdLoadConfigPara failed!\n");
        ret = -3;
        goto MAIN_FINISH;
    }

	pCfg->mFdOut = fopen(pCfg->mOutputFile, "w");
    if (!pCfg->mFdOut)
    {
        ALOGE("mFdOut open failed!\n");
        ret = -4;
        goto MAIN_FINISH;
    }

    pSample->mHandle = MotionDetectCreat(pStaCfg);
    if (!pSample->mHandle)
    {
        ALOGE("mHandle creat failed!\n");
        ret = -5;
        goto MAIN_FINISH;
    }
    MotionDetectInit(pSample->mHandle, pDynCfg);

    if (mdBufMgrCreate(pSample))
    {
        ALOGE("mdBufMgrCreate failed!\n");
        ret = -7;
        goto MAIN_FINISH;
    }

    if (pCfg->mReadLocalFileEn)
    {
        ret = mdMainReadFromLocalFile(pSample);
    }

    ALOGW("Finish tatal %d frames, cost %lldms.\n", pSample->mWrFrmCnt, pSample->mTotalFrmTime / 1000);

MAIN_FINISH:

    if (pSample)
    {
        FREE(pSample->mResult.mRegion);
        FCLOSE(pCfg->mFdOut);
        if (pSample->mHandle)
        {
            MotionDetectDestroy(pSample->mHandle);
        }
        mdBufMgrDestroy(pSample);
    }
    FREE(pSample);

    log_quit();

	return ret;
}