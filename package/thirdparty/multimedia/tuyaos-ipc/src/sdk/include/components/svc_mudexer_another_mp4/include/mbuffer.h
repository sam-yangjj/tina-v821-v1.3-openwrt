#ifndef _MBUFFER_H_
#define _MBUFFER_H_
#if defined(__cplusplus)
extern "C"
{
#endif
#include <stdio.h>
#include "mediainfo.h"

#define MBUFFER_SYS_MAX_PACKET_SIZE     376 // for 2 whole TS packets
#define MBUFFER_ES_MAX_FRAMES_IN_BUFFER 128
#define MBUFFER_BYTEARRAY_MAX_WHITESPACES 8

#define MBUFFER_EC_OK                0
#define MBUFFER_EC_FAIL              -1
#define MBUFFER_EC_NO_ENOUGH_DATA    -2
#define MBUFFER_EC_OUT_OF_BOUNDARY   -3

#define MBUFFER_EC_EXTRADATAM1   5      // VPS
#define MBUFFER_EC_EXTRADATA0    1
#define MBUFFER_EC_EXTRADATA1    2
#define MBUFFER_EC_SKIP          3
#define MBUFFER_EC_SEI           4

#define MBUFFER_SEVERAL_SECONDS      16  // used for TS wrap around calculation
#define MBUFFER_TS_WRAPAROUND_BIT    27  // lowest possible bit for TS wrap around

#define MBUFFER_ES_FRAME_TYPE_NORMALFRAME       0
#define MBUFFER_ES_FRAME_TYPE_EXTRAINFOFRAME    1
#define MBUFFER_ES_FRAME_TYPE_EMPTYFRAME        2 // for now, is to record discotinuity info

// Extracts iFlags of SideInfo to get whether is a key frame or not
#define MBUFFERESISKEYFRAME(iFlags) ((iFlags & 0x10000000) >> 28)
// Sets bIfKey to iFlags of SideInfo
#define MBUFFERESSETKEYFRAME(bIfKey, iFlags) (iFlags = ((iFlags) & 0xefffffff) | (bIfKey << 28))

// Extracts iFlags of Side to get frame type
#define MBUFFERESGETFRAMETYPE(iFlags) ((iFlags & 0xe0000000) >> 29)
// Sets iType to iFlags of SideInfo
#define MBUFFERESSETFRAMETYPE(iType, iFlags) (iFlags = ((iFlags) & 0x1fffffff) | (iType << 29))

// Extracts iFlags of SideInfo to get whether is a virtual I frame or not
#define MBUFFERESISVIRTUALIFRAME(iFlags) ((iFlags & 0x200000) >> 21)
// Sets bIfVirtualI to iFlags of SideInfo
#define MBUFFERESSETVIRTUALIFRAME(bIfVirtualI, iFlags) (iFlags = ((iFlags) & 0xffdfffff) | (bIfVirtualI << 21))

// Three buffers are used:
//  * buffer for data received from file or network
//  * buffer for video data
//  * buffer for audio data

// system buffer is the buffer to receive data from network/file;
// It's a circular buffer
typedef struct {
    unsigned char *pucBuffer;
    unsigned char pucBufferCache[MBUFFER_SYS_MAX_PACKET_SIZE];
        // in case the data wrapped around we copy a whole TS packet to this small cache buffer
        // at process the packet from it.
    unsigned char *pucCurStart;
    int iSize;
    int iHead;
    int iTail;

    // statistics: total num of bytes input/output
    int iNumBytesInput;
    int iNumBytesOutput;
} MBUFFERSYSBuffer;

// SideInfo stores information about a frame data in ES buffer
// the foramt of iFlags(in big endian):
// bits         meaning         range
// 3 bits       frame type              0:normal frame; 1:extra info frame; 2:empty frame
// 1 bit        is key frame            0:not key frame; 1: key frame
// 1 bit        if discontinuity begin  0:is not begin; 1: is begin discontinuity
// 1 bit        if discontinuity end    0:is not end; 1: is end discontinuity
// 1 bit        has UTC time            0:no; 1:yes, the value will stores in the llParameter
// 1 bit        if new codec            0:no; 1:yes, the codec value will stores in the llParameter
// 1 bit        if reset avsync         0:no; 1:yes, should reset avsync
// 1 bit        if drop frame happens   0:no; 1:yes, can drop the rest P frame within the GOP
// 22 bits      reserved                all 0
typedef struct {
    int iSize;
    int iFlags;
    long long PTS;
    long long DTS;
    long long llParameter; // to save extra info, usually the value is available when the corresponding flag is set
} MBUFFERESSideInfo;

// ES buffer is the buffer for elementary bitstream, audio or video
// It's a circular buffer but a decodable packet (a whold picture, etc.) is continuous in the buffer
typedef struct {
	int iIfDetached;

	// used if detached buffer
    unsigned char *pucDetachedBuffer[MBUFFER_ES_MAX_FRAMES_IN_BUFFER];
    int piBufferLength[MBUFFER_ES_MAX_FRAMES_IN_BUFFER];
    int iCurHead;

	// used if not detached
    unsigned char *pucBuffer;
    int piHead[MBUFFER_ES_MAX_FRAMES_IN_BUFFER];
    int piTail[MBUFFER_ES_MAX_FRAMES_IN_BUFFER];

    int iSize;
    MBUFFERESSideInfo pSideInfo[MBUFFER_ES_MAX_FRAMES_IN_BUFFER];
    int iTimeScale;             // example: 90000 means 1 is 1/90000s
    long long lLastTS;          //
    long long lLastAdjustedTS;  //
    long long lPrevLastTS;          //
    long long lPrevLastAdjustedTS;  //
    int iIndexHead; // point to the start position in piHead/piTail,which points to the first valid frame in buffer
    int iIndexTail; // point to the end position in piHead/piTail,which points to the first valid frame in buffer
	int iHeadRoom;	// for each frame sometimes we may want to put some space ahead of it there in order to write something (such as startcode) when needed
    /**
     *  For the situation that codec would not change, the iCodec represents the codec for all the frame in buffer
     *  For situation that codec would change, the iCodec only respresents the first several frames' codec,
     *  in this case, when codec change, an empty frame with new codec flag will appear, after encounter this
     *  empty frame, need to call MBUFFERESUpdateCodec() to update the new codec to iCodec.
     */
    int iCodec;
} MBUFFERESBuffer;

typedef struct {
    unsigned char *pucBuffer;
    int iCurPos;// if it's a buffer for reading, the position we read next time; if it's a buffer for writing, the position we write next time;
    int iEnd;   // if it's a buffer for reading, the end of valid data. ignored if it's a buffer for writing.
    int iSize;  // size of pucBuffer

    // used for substring search
    unsigned char pcSeparator[4];
    int iSeparatorLength;

	int iNumWhiteSpaces;
	char pcWhiteSpaces[MBUFFER_BYTEARRAY_MAX_WHITESPACES];
} MBUFFERByteArray;

// struct for audio extra info
typedef struct {
    int iSampleRate;
    int iNumChannels;
    int iNumBits;           // 16 or 8
} MBUFFERESAUDIOEXTRAINFO;


// public APIs
// macro definitions
#define MBUFFERESFIRSTFRAMESIDEINFO(pESBuffer) (&((pESBuffer)->pSideInfo[(pESBuffer)->iIndexHead]))

/**
 *  MBUFFERSYSBufferInit
 *  @abstract        Init MBUFFERSYSBuffer object pointed by pSYSBuffer.
 *
 *  @param[in]       pucBuffer   specifies the buffer where SYSBuffer will store data.
 *                             if NULL, a new buffer with size equal to iSize will be allocated dynamically.
 *  @param[in]       iHead       specifies the initial value for iHead variable in SYSBuffer.
 *  @param[in]       iTail       specifies the initial value for iTail variable in SYSBuffer.
 *  @param[in]       iSize       specifies the size of buffer.
 *  @param[in,out]   pSYSBuffer  specifies the SYSBuffer that will be initialized.
 *
 *  @return          MBUFFER_EC_OK.
 */
extern int MBUFFERSYSBufferInit(unsigned char *pucBuffer, int iHead, int iTail, int iSize, MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSBufferDeInit
 *  @abstract       Deinit MBUFFERSYSBuffer object. set iSize/iHead/iTail to 0,
 *                  and free the buffer if bIfFreeBuffer is 1;
 *
 *  @param[in]      bIfFreeBuffer   indicates whether need to free the buffer pointed by pucBuffer.
 *  @param[in,out]  pSYSBuffer      specifies the SYSBuffer that will be deinitialized.
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSBufferDeInit(int bIfFreeBuffer, MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSBufferSpaceAvailable
 *  @abstract        Get the size of available space in Buffer pointed by pucBuffer.
 *                   it should be called before writing data to SYSBuffer.
 *
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return          the size of available space
 */
extern int MBUFFERSYSBufferSpaceAvailable(MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSBufferSpaceAvailableToEnd
 *  @abstract        Get the size of available space from current tail index to the end of Buffer.
 *                   it should be called before writing data to SYSBuffer.
 *
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return          the size of available space
 */
extern int MBUFFERSYSBufferSpaceAvailableToEnd(MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSBufferLength
 *  @abstract        Get the size of data in Buffer pointed by pucBuffer
 *                   it should be called before reading data from SYSBuffer.
 *
 *  @param[in]       pSysBuffer  sys buffer
 *
 *  @return          the size of data
 */
extern int MBUFFERSYSBufferLength(MBUFFERSYSBuffer *pSysBuffer);


/**
 *  MBUFFERSYSBufferLengthToEnd
 *  @abstract        Get the size of data from current iHead index to the end of Buffer
 *                   it should be called before reading data from SYSBuffer.
 *
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return          the size of data
 */
extern int MBUFFERSYSBufferLengthToEnd(MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSValueAtPos
 *  @abstract        Get value of the byte that at the specified position in the Buffer.
 *
 *  @param[in]       iPos        specifies the position.
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return the value of the byte specified by iPos
 */
extern unsigned char MBUFFERSYSValueAtPos(int iPos, MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSAppendDataPos
 *  @abstract        Get a pointer to the first available byte starting from iTail.
 *                   it should be called before writing data to Buffer
 *
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return pointer to the first available byte
 */
extern unsigned char *MBUFFERSYSAppendDataPos(MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSAppendData
 *  @abstract        Append data to Buffer.
 *                   it should be called before writing data to Buffer
 *
 *  @param[in]       pucData     a pointer to the data that will be written to Buffer.
 *  @param[in]       iLength     specifies the size of data.
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return          MBUFFER_EC_OK if succeeds;
 *                   MBUFFER_EC_FAIL if fails
 */
extern int MBUFFERSYSAppendData(unsigned char *pucData, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSCopyByteArray2SYSBuffer
 *  @abstract       将pucData指定的数据拷贝到SYSBuffer中。
 *             拷贝时，如果SYSBuffer中可用空间大于等于iLength，则将iLength个字节的数据拷贝到SYSBuffer中；
 *             如果可用空间小于iLength,则拷贝的字节数等于可用空间大小
 *
 *  @param[in]      pucData     指向带拷贝的数据
 *  @param[in]      iLength     指定带拷贝数据的长度
 *  @param[in]      pSYSBuffer  指向SYSBuffer
 *
 *  @return         返回实际拷贝的字节数
 */
extern int MBUFFERSYSCopyByteArray2SYSBuffer(unsigned char *pucData, int iLength, MBUFFERSYSBuffer *    pSYSBuffer);

/**
 *  MBUFFERSYSAdvance
 *  @abstract       Increase iHead of SYSBuffer by iLength. After reading data from Buffer,
 *                  the function should be called to update the index of SYSBuffer
 *
 *  @param[in]      iLength     specifies the iLength that should be added to iHead
 *  @param[in,out]  pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK   if succeeds;
 *                  MBUFFER_EC_FAIL if fails cause buffer is not enough
 */
extern int MBUFFERSYSAdvance(int iLength, MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSExtend
 *  @abstract       Increase iTail of SYSBuffer by iLength. After writing data to Buffer,
 *                  the function should be called to update the index of SYSBuffer
 *
 *  @param[in]      iLength     specifies the iLength that should be added to iTail
 *  @param[in,out]  pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSExtend(int iLength, MBUFFERSYSBuffer *pSYSBuffer);

extern int MBUFFERSYSShrinkAtEnd(int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSStartPos
 *  @abstract       Get a pointer to the first byte of data.
 *
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         pointer to the first byte of data if Buffer has data;
 *                  NULL if Buffer is empty.
 */
extern unsigned char *MBUFFERSYSStartPos(MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSPosFromStart
 *  @abstract        Get a pointer to the byte that has an offset of @iOffset to iStart
 *
 *  @param[in]       iOffset     specifies the offset
 *  @param[in]       pSYSBuffer  sys buffer
 *
 *  @return          pointer of destinated position if succeeded
 *                  NULL if failed.
*/
extern unsigned char *MBUFFERSYSPosFromStart(int iOffset, MBUFFERSYSBuffer *pSYSBuffer);

extern unsigned char *MBUFFERSYSPosFromEnd(int iOffset, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSBufferLengthToEndByOffset
 *  @abstract       从距离数据块首字节偏移量为iOffset的位置处，计算到Buffer结束位置处的有效数据大小
 *                  该函数通常在读取Buffer中的数据之前调用。
 *  @param[in]      iOffset     指定偏移大小
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         数据大小
 */
extern int MBUFFERSYSBufferLengthToEndByOffset(int iOffset, MBUFFERSYSBuffer *pSYSBuffer);


/**
 *  MBUFFERSYSCopyToCache
 *  @abstract       将Buffer中指定长度的数据拷贝到Cache Buffer中
 *
 *  @param[in]      iLength     指定要拷贝的数据长度
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSCopyToCache(int iLength, MBUFFERSYSBuffer *pSYSBuffer);

extern int MBUFFERSYSCopyToExternalCacheByOffset(int iOffset, int iLength, char *pucCache, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSCopyToCacheByOffset
 *  @abstract       从距离数据块首字节偏移量为iOffset处的字节处，开始拷贝指定长度的数据到Cache Buffer中
 *
 *  @param[in]      iOffset     指定偏移量
 *  @param[in]      iLength     指定要拷贝的数据长度
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSCopyToCacheByOffset(int iOffset, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

extern int MBUFFERSYSCopyAtEnd(int iSrcOffset2End, int iLength, MBUFFERSYSBuffer *pDstSYSBuffer, MBUFFERSYSBuffer *pSrcSYSBuffer);

/**
 *  MBUFFERSYSGetContinousBuffer
 *  @abstract       获取一块连续内存的首地址，该内存保存了Buffer中离数据块首地址偏移量为iOffset的一段长度为iLength的数据。
 *
 *  @param[in]      iOffset     指定偏移量
 *  @param[in]      iLength     指定数据长度
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回数据块的首地址。
 */
extern unsigned char *MBUFFERSYSGetContinousBuffer(int iOffset, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSPeek2ByteArray
 *  @abstract       从Buffer中拷贝iLength个字节的数据到pucDest指向的内存中。
 *                  该函数执行完成之后，被拷贝的数据还保存在Buffer中，可重复使用。
 *
 *  @param[in]      pucDest     指向保存数据的内存地址
 *  @param[in]      iLength     指定要拷贝的数据长度
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSPeek2ByteArray(unsigned char *pucDest, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
*  MBUFFERSYSPeek2ByteArrayByOffset
*  @abstract       从Buffer中距离头部iOffset远处开始，拷贝iLength个字节的数据到pucDest指向的内存中。
*                  该函数执行完成之后，被拷贝的数据还保存在Buffer中，可重复使用。
*
*  @param[in]      pucDest     指向保存数据的内存地址
*  @param[in]      iLength     指定要拷贝的数据长度
*  @param[in]      iOffset     指定的偏移量
*  @param[in]      pSYSBuffer  sys buffer
*
*  @return         MBUFFER_EC_OK
*/
extern int MBUFFERSYSPeek2ByteArrayByOffset(unsigned char *pucDest, int iLength, int iOffset, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSCopy2ByteArray
 *  @abstract       从Buffer中拷贝iLength个字节的数据到pucDest指向的内存中。
 *                  该函数执行完成之后，会将拷贝的数据从Buffer中扔掉，不可重复使用。
 *
 *  @param[in]      pucDest     指向保存数据的内存地址
 *  @param[in]      iLength     指定要拷贝的数据长度
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERSYSCopy2ByteArray(unsigned char *pucDest, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSNumBytesInput
 *  @abstract       从MBUFFERSYSBufferInit以后，至今一共获得过的数据字节数
 *
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回字节数
 */
extern int MBUFFERSYSNumBytesInput(MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSNumBytesOutput
 *  @abstract       从MBUFFERSYSBufferInit以后，至今一共输出过的数据字节数
 *
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回字节数
 */
extern int MBUFFERSYSNumBytesOutput(MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSReadFromFile
 *  @abstract       从文件中读取指定长度的数据到Buffer中
 *
 *  @param[in]      fp_input    指向打开的文件
 *  @param[in]      iLength     指定要读取的字节数
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回成功读取的字节数
 */
extern int MBUFFERSYSReadFromFile(FILE *fp_input, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSWriteToFile
 *  @abstract       将Buffer中iLength个字节的数据写入到文件中， 并更新SYSBuffer中的iStart变量
 *
 *  @param[in]      fp_output   指向将要写入的文件
 *  @param[in]      iLength     指定要写入的字节数
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回成功写入的数据长度
 */
extern int MBUFFERSYSWriteToFile(FILE *fp_output, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERSYSWriteToFileNoAdvance
 *  @abstract       将Buffer中iLength个字节的数据写入到文件中， 但不更新SYSBuffer中的iStart变量
 *
 *  @param[in]      fp_output   指向将要写入的文件
 *  @param[in]      iLength     指定要写入的字节数
 *  @param[in]      pSYSBuffer  sys buffer
 *
 *  @return         返回成功写入的数据长度
 */
extern int MBUFFERSYSWriteToFileNoAdvance(FILE *fp_output, int iLength, MBUFFERSYSBuffer *pSYSBuffer);

/**
 *  MBUFFERESBufferInit
 *  @abstract       初始化MBUFFERESBuffer对象。
 *
 *  @param[in]      pucBuffer   如果不为NULL，则ESBuffer使用@pucBuffer指定的内存来保存数据；
 *                              如果为NULL，则在初始化时由函数动态创建内存空间
 *  @param[in]      iSize       如果@pucBuffer不为NULL，则表示@pucBuffer指定内存的大小；
 *                              如果为NULL，则指定需要创建的内存大小
 *  @param[in,out]  pESBuffer   指向需要初始化的ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESBufferInit(unsigned char *pucBuffer, int iSize, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferDeInit
 *  @abstract       重置或释放ESBuffer资源。
 *
 *  @param[in]      bIfFreeBuffer   如果为1，则释放ESBuffer的资源；如果为0；则重置ESBuffer。
 *  @param[in,out]  pESBuffer       指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESBufferDeInit(int bIfFreeBuffer, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferSetTimescale
 *  @abstract       Set timescale for ES buffer, the default value is 90000.
 *
 *  @param[in]      iTimescale  timescale
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESBufferSetTimescale(int iTimescale, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferGetTimescale
 *  @abstract       Get timescale for ES buffer
 *
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESBufferGetTimescale(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferSetCodec
 *  @abstract       设置视频的编码格式。
 *
 *  @param[in]      iCodec      指定编码格式，见mediainfo.h中的TP_AVCODEC_XXX
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESBufferSetCodec(int iCodec, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferUpdateCodec
 *  @abstract       Update the first frame's codec to the iCodec, only use when encounter a sideinfo
 *                  with new codec flag.
 *
 *  @param[in,out]  pESBuffer
 *
 *  @return         MBUFFER_EC_OK if the first frame has new codec flag and successfully update to iCodec
 *                  MBUFFER_EC_FAIL if the first frame do not have a new codec flag.
 */
extern int MBUFFERESBufferUpdateCodec(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferIsIndexFull
 *  @abstract       检查ESBuffer缓存是否已满。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         1：表示已满；0：表示未满
 */
extern int MBUFFERESBufferIsIndexFull(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESBufferSpaceAvailable
 *  @abstract       获取ESBuffer当前可使用的内存空间大小。
 *
 *  @param[in]      iIfStartFrame   表示是否是一帧的开始
 *  @param[in]      pESBuffer       指向ESBuffer对象
 *
 *  @return         可用空间大小
 */
extern int MBUFFERESBufferSpaceAvailable(int iIfStartFrame, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESAppendData
 *  @abstract       向ESBuffer中追加数据。
 *
 *  @param[in]      pucData         指向待追加的数据
 *  @param[in]      iLength         表示数据的大小
 *  @param[in]      iIfStartFrame   表示当前添加的数据是否是帧的最开始部分
 *  @param[in]      pESBuffer       指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESAppendData(unsigned char *pucData, int iLength, int iIfStartFrame, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESFirstFrameAdvance
 *  @abstract       将ESBuffer中当前第一帧的piHead向后移动iLength个字节。
 *
 *  @param[in]      iLength     表示向前移动的字节数
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESFirstFrameAdvance(int iLength, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastFrameAdvance
 *  @abstract       将ESBuffer中当前最后一帧的piHead向后移动iLength个字节。
 *
 *  @param[in]      iLength     表示向前移动的字节数
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESLastFrameAdvance(int iLength, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESAdvanceFrame
 *  @abstract       消费掉ESBuffer中的第一帧。
 *
 *  @param[in,out]  pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK   表示执行成功;
 *                  MBUFFER_EC_FAIL 执行失败，原因是ESBuffer无可使用的帧
 */
extern int MBUFFERESAdvanceFrame(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESFirstFramePos
 *  @abstract       获取ESBuffer中第一帧首字节的地址。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         成功则返回指向第一帧首字节的指针；失败返回NULL
 */
extern unsigned char *MBUFFERESFirstFramePos(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESFirstFrameLength
 *  @abstract       获取ESBuffer中第一帧的长度。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         如果ESBuffer中有可用帧，则返回第一帧的长度；
 *                  如果ESBuffer为空，则返回0
 */
extern int MBUFFERESFirstFrameLength(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESFirstFrameSideInfo
 *  @abstract       Gets the SideInfo of the first frame in ESBuffer.
 *                  Just return the pointer stored in ESBuffer, will not adjust the PTS and DTS.
 *
 *  @param[in]     pESBuffer   pointer to ESBuffer instance
 *
 *  @return         NULL if there is no frame in ESBuffer, otherwise return a pointer of MBUFFERESSideInfo instance.
*/
extern MBUFFERESSideInfo *MBUFFERESFirstFrameSideInfo(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastFrameSideInfo
 *  @abstract       Gets the SideInfo of the last frame in ESBuffer.
 *                  Just return the pointer stored in ESBuffer, will not adjust the PTS and DTS.
 *
 *  @param[in]     pESBuffer   pointer to ESBuffer instance
 *
 *  @return         NULL if there is no frame in ESBuffer, otherwise return a pointer of MBUFFERESSideInfo instance.
*/
extern MBUFFERESSideInfo *MBUFFERESLastFrameSideInfo(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESGetLastFrameSideInfo
 *  @abstract       Gets the SideInfo of last frame in ESBuffer with adjusted PTS and DTS.
 *
 *  @param[out]     pSideInfo   pointer to the memory that stores the SideInfo
 *  @param[in]      pESBuffer   pointer to ESBuffer instance
 *
 *  @return         MBUFFER_EC_FAIL if no frame in ESBuffer
                    MBUFFER_EC_OK   if success
*/
extern int MBUFFERESGetLastFrameSideInfo(MBUFFERESSideInfo *pSideInfo, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastTwoFrameSideInfo
 *  @abstract       Gets the SideInfo of the last but one frame in ESBuffer.
 *                  Just return the pointer stored in ESBuffer, will not adjust the PTS and DTS.
 *
 *  @param[in]     pESBuffer   pointer to ESBuffer instance
 *
 *  @return         NULL if there is not enough frame in ESBuffer, otherwise return a pointer of MBUFFERESSideInfo instance.
*/
extern MBUFFERESSideInfo *MBUFFERESLastTwoFrameSideInfo(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESGetLastTwoFrameSideInfo
 *  @abstract       Gets the SideInfo of the last but one frame in ESBuffer with adjusted PTS and DTS.
 *
 *  @param[out]     pSideInfo   pointer to the memory that stores the SideInfo
 *  @param[in]      pESBuffer   pointer to ESBuffer instance
 *
 *  @return         MBUFFER_EC_FAIL if not enough frame in ESBuffer
                    MBUFFER_EC_OK   if success
*/
extern int MBUFFERESGetLastTwoFrameSideInfo(MBUFFERESSideInfo *pSideInfo, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastFrameNo
 *  @abstract       返回ESBuffer中最后一帧的编号。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         最后一帧的编号
 */
extern int MBUFFERESLastFrameNo(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastFramePos
 *  @abstract       获取ESBuffer中最后一帧首字节的地址。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         成功则返回指向最后一帧首字节的指针；失败返回NULL
 */
extern unsigned char *MBUFFERESLastFramePos(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESLastFrameLength
 *  @abstract       获取ESBuffer中最后一帧的长度。
 *
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         如果ESBuffer中有可用帧，则返回最后一帧的长度；
 *                  如果ESBuffer为空，则返回0
*/
extern int MBUFFERESLastFrameLength(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESSetSideInfo
 *  @abstract       设置ESBuffer中最后一帧的SideInfo信息。
 *
 *  @param[in]      pSideInfo   指向SideInof对象
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSetSideInfo(MBUFFERESSideInfo *pSideInfo, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESGetSideInfo
 *  @abstract       Gets the SideInfo of first frame in ESBuffer with adjusted PTS and DTS.
 *
 *  @param[out]     pSideInfo   pointer to the memory that stores the SideInfo
 *  @param[in]      pESBuffer   pointer to ESBuffer instance
 *
 *  @return         MBUFFER_EC_FAIL if there is no frame in ESBuffer
                    MBUFFER_EC_OK   if success
*/
extern int MBUFFERESGetSideInfo(MBUFFERESSideInfo *pSideInfo, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESSetKeyFrame
 *  @abstract       设置ESBuffer中最后一帧的关键帧标识。
 *
 *  @param[in]      bIfKeyFrame 如果为1，则将最后一帧设置为关键帧；为0，则设置为非关键帧
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSetKeyFrame(int bIfKeyFrame, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESSplitFrame
 *  @abstract       从追加进ESBuffer的数据中分割出一帧
 *
 *  @param[in]      iPos        指定分割的位置
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSplitFrame(int iPos, MBUFFERESBuffer *pESBuffer);

extern int MBUFFERESFinishLastFrame(MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESThrowLastFrame
 *  @abstract       扔掉ESBuffer中最后一帧的数据。
 *
 *  @param[in]      bIfRecede   如果为0，函数仍保存最后一帧的元信息；为1则不保存
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK：执行成功
 *                  MBUFFER_EC_FAIL：执行失败
 */
extern int MBUFFERESThrowLastFrame(int bIfRecede, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESNumFrames
 *  @abstract       获取ESBuffer中当前帧的数量。
 *
 *  @param[in]      pESBuffer   指向ESBUffer对象
 *
 *  @return         当前帧的个数
 */
extern int MBUFFERESNumFrames(MBUFFERESBuffer *pESBuffer);
extern unsigned char *MBUFFERESPosForData(int iLength, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESReadFromFile
 *  @abstract       从文件中读取指定长度的数据到ESBuffer中
 *
 *  @param[in]      fp_input    指向打开的文件
 *  @param[in]      iLength     指定读取数据的长度
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK：读取成功
 *                  MBUFFER_EC_FAIL：读取失败（ESBuffer可用空间不够或者已满）
*/
extern int MBUFFERESReadFromFile(FILE *fp_input, int iLength, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESWriteFirstFrameToFile
 *  @abstract       将ESBuffer中第一帧数据写入到文件中
 *
 *  @param[in]      fp_output   指向打开的文件
 *  @param[in]      pESBuffer   指向ESBuffer对象
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESWriteFirstFrameToFile(FILE *fp_output, MBUFFERESBuffer *pESBuffer);
extern int MBUFFERESWriteFirstFrameToFd(int iFd, MBUFFERESBuffer *pESBuffer);

extern int MBUFFERESWriteSliceOfFirstFrameToFile(int iNumBytes, FILE *fp_output, MBUFFERESBuffer *pESBuffer);
extern int MBUFFERESWriteSliceOfFirstFrameToFd(int iNumBytes, int iFd, MBUFFERESBuffer *pESBuffer);

/**
*  MBUFFERESCheckValid
*  @abstract       判断根据一个指针取得数据是否有效；使用场景：消费者取出一帧数据，advance之前记录数据在esbuffer中的位置，
*                  为了反复利用该数据又不申请额外的空间存储取出的数据，在advance后还想获取该数据时应该查询有效性（原数据是否被覆盖）
*
*  @param[in]      pucData   查询者拥有的数据指针
*  @param[in]      pESBuffer   指向ESBuffer对象
*
*  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESCheckValid(unsigned char *pucData, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESAddExtraInfo
 *  @abstract       Generates a frame data of extra information and appends to the ESBuffer.
 *
 *  @param[in]      pucExtraInfo        pointer to extra information
 *  @param[in]      iExtraInfoLength    length of extrainformation
 *  @param[in,out]  pESBuffer           pointer to the ESBuffer instance
 *
 *  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESAddExtraInfo(unsigned char *pucExtraInfo, int iExtraInfoLength, MBUFFERESBuffer *pESBuffer);

/**
 *  MBUFFERESSideInfoSetIfKeyFrame
 *  @abstract       Sets the bIfKeyFrame to iFlags of pSideInfo.
 *
 *  @param[in]      bIfKeyFrame  1 means is key while 0 means no
 *  @param[in,out]  pSideInfo    the side info need to set key frame property
 *
 *  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESSideInfoSetIfKeyFrame(int bIfKeyFrame, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetIfKeyFrame
 *  @abstract       Extracts the bIfKeyFrame from iFlags of pSideInfo.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         1 if the pSideInfo is a key frame, 0 if the other side.
*/
extern int MBUFFERESSideInfoGetIfKeyFrame(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetFrameType
 *  @abstract       Sets the frame type to iFlags of pSideInfo.
 *
 *  @param[in]      iFramType   frame type, see MBUFFER_ES_FRAME_TYPE_XXXX
 *  @param[in,out]  pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESSideInfoSetFrameType(int iFramType, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetFrameType
 *  @abstract       Extracts the frame type from iFlags of pSideInfo.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         see MBUFFER_ES_FRAME_TYPE_XXXX
*/
extern int MBUFFERESSideInfoGetFrameType(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetIfDiscontinuityBegin
 *  @abstract       Set the side info to indicate whether will begin discontinuity or not.
 *
 *  @param[in]      bIfBegin
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetIfDiscontinuityBegin(int bIfBegin, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetIfDiscontinuityBegin
 *  @abstract       Extract if discontinuity begin value fraom side info.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         0: is not begin; 1: is begin
 */
extern int MBUFFERESSideInfoGetIfDiscontinuityBegin(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetIfDiscontinuityEnd
 *  @abstract       Set the side info to indicate whether will end discontinuity or not.
 *
 *  @param[in]      bIfEnd
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetIfDiscontinuityEnd(int bIfEnd, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetIfDiscontinuityEnd
 *  @abstract       Extract if discontinuity end value from side info.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         0: is not end; 1: is end
 */
extern int MBUFFERESSideInfoGetIfDiscontinuityEnd(MBUFFERESSideInfo *pSideInfo);

/**
*  MBUFFERESSideInfoGetIfNewCodec
*  @abstract
*
*  @param[in]      pSideInfo   the pointer of SideInfo
*
*  @return         0: no; 1: yes
*/
extern int MBUFFERESSideInfoGetIfNewCodec(MBUFFERESSideInfo *pSideInfo);

/**
*  MBUFFERESSideInfoSetIfNewCodec
*  @abstract       Set the side info to indicate whether is new codec for the frame
*
*  @param[in]      bIfNewCodec
*  @param[in]      pSideInfo   the pointer of SideInfo
*
*  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESSideInfoSetIfNewCodec(int bIfNewCodec, MBUFFERESSideInfo *pSideInfo);

/**
*  MBUFFERESSideInfoGetIfResetAVSync
*  @abstract
*
*  @param[in]      pSideInfo   the pointer of SideInfo
*
*  @return         0: no; 1: yes
*/
extern int MBUFFERESSideInfoGetIfResetAVSync(MBUFFERESSideInfo *pSideInfo);

/**
*  MBUFFERESSideInfoSetIfResetAVSync
*  @abstract       Set the side info to indicate whether is reset avsync for the frame
*
*  @param[in]      bIfResetAVSync
*  @param[in]      pSideInfo   the pointer of SideInfo
*
*  @return         MBUFFER_EC_OK
*/
extern int MBUFFERESSideInfoSetIfResetAVSync(int bIfResetAVSync, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetIfHasUTCTime
 *  @abstract       Set the side info to indicate whether has UTC time or not.
 *
 *  @param[in]      bHasUTCTime
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetIfHasUTCTime(int bHasUTCTime, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetIfHasUTCTime
 *  @abstract       Extract if has UTC time value from side info.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         0: no; 1: yes
 */
extern int MBUFFERESSideInfoGetIfHasUTCTime(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetUTCTime
 *  @abstract       Get the UTC time from side info when the has UTC time flag is set.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_FAIL is the has UTC time flag is not set
 *                  otherwise, return UTC time of type long long
 */
extern long long MBUFFERESSideInfoGetUTCTime(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetUTCTime
 *  @abstract       Set the UTC time to side info
 *
 *  @param[in]      llUTCTime   UTC time
 *  @param[in,out]  pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetUTCTime(long long llUTCTime, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetCodec
 *  @abstract       Get the codec info from side info when the if new codec flag is set.
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_FAIL is the new codec flag is not set
 *                  otherwise, return TP_AVCODEC_XXX
 */
extern int MBUFFERESSideInfoGetCodec(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetCodec
 *  @abstract       Set the codec to side info to
 *
 *  @param[in]      iCodec      codec, see TP_AVCODEC_XXX
 *  @param[in,out]  pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetCodec(int iCodec, MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoGetIfDropFrame
 *  @abstract       Get whether drop frame happens
 *
 *  @param[in]      pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoGetIfDropFrame(MBUFFERESSideInfo *pSideInfo);

/**
 *  MBUFFERESSideInfoSetIfDropFrame
 *  @abstract       Set whether drop frame happens
 *
 *  @param[in]      bIfDrop
 *  @param[in,out]  pSideInfo   the pointer of SideInfo
 *
 *  @return         MBUFFER_EC_OK
 */
extern int MBUFFERESSideInfoSetIfDropFrame(int bIfDrop, MBUFFERESSideInfo *pSideInfo);

/* Byte Array
    Byte array is used either for output or for input, but never for both at the same time (unless an output one is converted to input by MBUFFERByteConvert2ReadBuff()).
    iSize is the size of buffer, for either input or output
    In case of output, iCurPos is the position for output new data, and iEnd is not used.
    In case of input, iCurPos is the position for reading new data, and iEnd is the end of valid data.
*/
/**
*  MBUFFERByteArrayInit
*
*  @param[in]      pucBuffer    the buffer for the byte array. Can be created outside and set to ByteArray. If NULL, this function will
*                               allcate memory of size iSize
*  @param[in]      iHead        the start position of buffer (for read or write, basically will be added to pucBuffer and set to this->pucBuffer)
*  @param[in]      iTail        the end position of buffer (for read, basically will be set to iEnd after subtracting iHead)
*  @param[in]      iSize        the size of buffer (for both read/write, basically will be set to iiSize)
*  @param[in,out]  pByteArray   the byearray that inited by this function
*
*  @return         0: success; -1: fail
*/
extern int MBUFFERByteArrayInit(unsigned char *pucBuffer, int iHead, int iTail, int iSize, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArraySpaceAvailable(MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayLength(MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayCurPos(MBUFFERByteArray *pByteArray);
extern unsigned char * MBUFFERByteArrayCurBufferPos(MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayCopy(unsigned char *pucDest, int iLength, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayAdvance(int iLength, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayExtend(int iLength, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArraySeek(int iOffset, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayMove(int iDestPos, int iSrcPos, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayMove2Head(MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayReadFromFile(FILE *fp_input, int iLength, MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayFlush(MBUFFERByteArray *pByteArray);
extern int MBUFFERByteArrayDeInit(int bIfFreeBuffer, MBUFFERByteArray *pByteArray);


#if defined(__cplusplus)
}
#endif
#endif
