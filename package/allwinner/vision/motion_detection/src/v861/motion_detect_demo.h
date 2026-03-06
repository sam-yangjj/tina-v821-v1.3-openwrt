/******************************************************************************
  Copyright (C), 2001-2025, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : motion_detect.h
  Version       : Initial Draft
  Author        : Allwinner
  Created       : 2025/02/27
  Last Modified :
  Description   : motion detect demo header file
  Function List :
  History       :
******************************************************************************/

#ifndef _MOTION_DETECT_DEMO_H_
#define _MOTION_DETECT_DEMO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <base_list.h>

#include "motion_detect.h"

#define IN_FRAME_BUF_NUM   (6)
#define MAX_FILE_PATH_LEN  (128)

typedef struct {
	unsigned char mCfgFilePath[MAX_FILE_PATH_LEN];
} SAMPLE_MD_CMD_LINE;

typedef struct {
    int mReadFrameNum;
    int mSrcBufNum;
    int mTotalResultEn;
    int mReadLocalFileEn;

    // for get picture from vi
    int mYuvWbEn;
    int mViDev;
    int mIspDev;
    int mViChn;
    int mFrameRate;
    int mDropFrameNum;
    int mGetFrameInterval;
    int mWdrEn;
    int mMirrorEn;
    int mFlipEn;
    int mPixFormat;
    int mColorSpace;

    // for get picture from yuv file
	int mFrontCutFrameNum;
	int mBackCutFrameNum;

	MOTION_DETECT_STATIC_CFG_S     mMdStaCfg;
	MOTION_DETECT_DYNAMIC_CFG_S    mMdDynCfg;

	FILE *mFdIn;
	FILE *mFdOut;
    FILE *mFdYuv;
	unsigned char mInputFile[MAX_FILE_PATH_LEN];
    unsigned char mOutputFile[MAX_FILE_PATH_LEN];
    unsigned char mYuvFile[MAX_FILE_PATH_LEN];
} SAMPLE_MD_CFG_S;

typedef struct {
	int mYSize;
	int mCSize;
	int mFrameSize;
	int mFrameNum;
	int mIsBackword;
	long long mFileLen;
	long long mSeekBgn;
	long long mSeekEnd;
	long long mCurPos;
} SAMPLE_MD_FILE_S;

typedef struct {
	void *pVirAddr;
	unsigned int  nPhyAddr;
	unsigned int  nIdx;
} MD_FRAME_BUF_S;

typedef struct {
	MD_FRAME_BUF_S     mFrame;
	struct list_head   mList;
} MD_FRAME_NODE_S;

typedef struct {
	int                mFrameNum;
	struct list_head   mIdleList;
	struct list_head   mReadyList;
	struct list_head   mUseList;
	pthread_mutex_t    mIdleListLock;
	pthread_mutex_t    mReadyListLock;
	pthread_mutex_t    mUseListLock;
} SAMPLE_MD_BUF_Q;

typedef struct {
	SAMPLE_MD_CFG_S            mCfg;
	SAMPLE_MD_CMD_LINE         mCmdLinePara;
	SAMPLE_MD_FILE_S           mFile;
	void                       *mHandle;
	MOTION_DETECT_RESULT_S     mResult;
	pthread_t                  mReadFrmThdId;
    pthread_t                  mGetFrmThdId;
	pthread_t                  mProcessFrmThdId;
	SAMPLE_MD_BUF_Q            mPicBufQ;
	int                        mRdFrmCnt;
	int                        mWrFrmCnt;
	long long                        mOneFrmTime;
	long long                        mTotalFrmTime;
} SAMPLE_MD_S;

#endif  /* _MOTION_DETECT_DEMO_H_ */
