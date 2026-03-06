#ifndef _MP4DEMUX_H_
#define _MP4DEMUX_H_
#include "mbuffer.h"
#include "mp4_tpwfile.h"
// TODO 评估写死的参数规格是否满足需求

#if defined(__cplusplus)
extern "C"
{
#endif

#define MP4DEMUX_MAX_AVC_SPS_LENGTH             256
#define MP4DEMUX_MAX_AVC_PPS_LENGTH             64
#define MP4DEMUX_H264_START_CODE_LENGTH         4

#define MP4DEMUX_MAX_TRACK                      2

#define MP4DEMUX_MAX_STCO                       10240   // temporary design, big enough to hold 3 minutes' frames
#define MP4DEMUX_MAX_STSZ                       10240
#define MP4DEMUX_MAX_STTS                       3 * 10240
#define MP4DEMUX_MAX_STSC                       3 * 10240
#define MP4DEMUX_MAX_CTTS                       10240
#define MP4DEMUX_MAX_STSS                       2048
#define MP4DEMUX_MAX_ELST                       10

#define MP4DEMUX_MAX_ADVANCE                    (4096*4)  // temporary design to 1024*4, see TEST_SYSBUFFER_SIZE and TEST_FILE_READ_SIZE

#define MP4DEMUX_MIN_SYSBUFFER_LENGTH           8
#define MP4DEMUX_MIN_INFOSYSBUFFER_LENGTH       4

#define MP4DEMUX_SIZE_INFOSYSBUFFER_PARSED      (10240*4)   // we parse 32 bytes in InfoSYSBuffer one time
#define MP4DEMUX_SIZE_INFOBUFFER                (MP4DEMUX_SIZE_INFOSYSBUFFER_PARSED / 4 / 2)

#define MP4DEMUX_SIZE_ENTRY_STSC                12      // size of a single entry in STTC
#define MP4DEMUX_SIZE_ENTRY_STTS                8
#define MP4DEMUX_SIZE_ENTRY_STSS                4
#define MP4DEMUX_SIZE_ENTRY_STSZ                4
#define MP4DEMUX_SIZE_ENTRY_STCO                4
#define MP4DEMUX_SIZE_ENTRY_CTTS                8

#define MP4DEMUX_DATASOURCE_FROM_BUFFER         1
#define MP4DEMUX_DATASOURCE_FROM_FILE           2

#define MP4DEMUX_DATATYPE_SWITCH_THRESHOLD      2       // time scale: second

// options
#define MP4DEMUX_OPTION_PARSE_ALL_SAMPLES       1
#define MP4DEMUX_OPTION_PARSE_FIRST_IDR         2
#define MP4DEMUX_OPTION_PARSE_HEADER            3

// error codes
#define MP4DEMUX_EC_OK                          0
#define MP4DEMUX_EC_FAILURE                     -1
#define MP4DEMUX_EC_BUFFER_FULL                 -2
#define MP4DEMUX_EC_NEED_COPY                   -3

#define MP4DEMUX_EC_GOT_VIDEO_FRAME             1
#define MP4DEMUX_EC_GOT_AUDIO_FRAME             2
#define MP4DEMUX_EC_FINISH                      3
#define MP4DEMUX_EC_NEED_SEEK                   4
#define MP4DEMUX_EC_FINISH_PARSE_HEAD           5

#define MP4DEMUX_STATUS_MOOV_FINDING             0
#define MP4DEMUX_STATUS_MOOV_ADVANCING           1
#define MP4DEMUX_STATUS_MOOV_COPYING_AND_PARSING 2
#define MP4DEMUX_STATUS_DATA_TOPARSE             3
#define MP4DEMUX_STATUS_DATA_PARSING             4
#define MP4DEMUX_STATUS_DATA_SEEKING             5
#define MP4DEMUX_STATUS_DATA_ADVANCING           6
#define MP4DEMUX_STATUS_DATA_COPING              7
#define MP4DEMUX_STATUS_FINISH                   8

#define MP4DEMUX_TIME_SCALE_MILLISECOND         1000

#define MP4DEMUX_ES_ALLOWANCE                   10

#define swapint(x) x = (((x >> 24) & 0xff) | ((x >> 8) & 0xff00) | \
    ((x << 8) & 0xff0000) | ((x << 24) & 0xff000000))
#define swapshort(x) x = (((x & 0xff) << 8) | ((x >> 8) & 0xff))

typedef struct {
    unsigned int uiFirstChunk;
    unsigned int uiSamplesPerChunk;
    unsigned int uiSampleDescription;
} MP4DEMUXSTSCENTRY;

typedef struct {
    // parameters to be set
    int iOptions; // MP4DEMUX_OPTION_PARSE_XXXX, parse first idr or all samples
    int iDataSource; // MP4DEMUX_DATASOURCE_FROM_XXXX, read data from sys buffer or local file
#if 1
    int bAddExtraInfo;  // if add sps and pps in the head of key sample(I frame)
#endif
    int bFinishParseHeaders; // if finish parsing the headers, which is the index of mp4

    TPWFile *pFile; // input local file, this should be set by calling MP4DemuxSetFile
    MBUFFERSYSBuffer *pSYSBuffer; // input sys buffer, this should be set by calling MP4DemuxSetSYSBuffer

    MBUFFERESBuffer *pVideoESBuffer; // output es buffer for video
    MBUFFERESBuffer *pAudioESBuffer; // output es buffer for audio

    // info from headers(index)
    unsigned char *pucInfoBuffer; // Buffer to save moov box data
    int iInfoBufferSize; // The size of pucInfoBuffer
    int iInfoBufferOffset; // Current read pos in pucInfoBuffer
    int iInfoBufferReadSize; // When iInfoBufferReadSize == iInfoBufferSize, means copying moov atom finished, only for sysbuffer mode
    int iInfoBufferOffsetInFile; // Offset of first data of buffer in file, used for file mode, always 0 when in sysbuffer mode

    int iMoovSize; // The size of moov box
    int iMoovReadSize; // The size of data in moov box being dealed with, when iMoovReadSize == iMoovSize, moov's parsing is over

    unsigned int uiCreationTime;
    unsigned int uiModificationTime;
    unsigned int uiTimeScale; // time scale for the entire mp4
    unsigned int uiDuration; // duration for the entire mp4
    unsigned int uiNextTrackID;

    unsigned short pusImageWidth[MP4DEMUX_MAX_TRACK];
    unsigned short pusImageHeight[MP4DEMUX_MAX_TRACK];
#if 0
    int iAVCSPSLength;
    unsigned char pucAVCSPS[MP4DEMUX_MAX_AVC_SPS_LENGTH];
    int piAVCPPSLength[3];
    unsigned char pucAVCPPS[3][MP4DEMUX_MAX_AVC_PPS_LENGTH];
#endif
    int iVideoTrack;
    int iAudioTrack;
    int iVideoStreamCodec;
    int iAudioStreamCodec;

    unsigned short usImageWidth; // video stream informations
    unsigned short usImageHeight;

    unsigned char ucObjectTypeIndication; // audio stream informations
    unsigned int uiSamplingFrequencyIndex;
    unsigned int uiChannelConfiguration;
    unsigned int uiAudioMaxBitrate;
    unsigned int uiAudioAvgBitrate;

    long long lStartTime;
    long long lEndTime;

    unsigned int puiTimeScale[MP4DEMUX_MAX_TRACK]; // time scale for tracks
    unsigned int puiDuration[MP4DEMUX_MAX_TRACK]; // media time scale, duration for tracks
    long long pllStartTime[MP4DEMUX_MAX_TRACK]; // media time scale, start time of tracks

    unsigned int uiFirstIDROffset;
    unsigned int uiFirstIDRSampleSize;

    unsigned int puiSTSCEntryCount[MP4DEMUX_MAX_TRACK];
    unsigned int puiSTCOEntryCount[MP4DEMUX_MAX_TRACK];
    unsigned int puiSTSZEntryCount[MP4DEMUX_MAX_TRACK];
    unsigned int puiSTTSEntryCount[MP4DEMUX_MAX_TRACK];
    unsigned int puiCTTSEntryCount[MP4DEMUX_MAX_TRACK];
    unsigned int uiSTSSEntryCount;

    unsigned short *puiSTSCBuffer[MP4DEMUX_MAX_TRACK]; // Chunk number for every sample
    unsigned int puiSTSCCntInBuffer[MP4DEMUX_MAX_TRACK];
    unsigned int *puiSTTSBuffer[MP4DEMUX_MAX_TRACK]; // Duration for every sample
    unsigned int puiSTTSCntInBuffer[MP4DEMUX_MAX_TRACK];
    unsigned int *puiSTSZBuffer[MP4DEMUX_MAX_TRACK]; // Sample size for every sample
    unsigned int puiSTSZCntInBuffer[MP4DEMUX_MAX_TRACK];
    unsigned int *puiSTCOBuffer[MP4DEMUX_MAX_TRACK]; // Chunk offset for every chunk
    unsigned int puiSTCOCntInBuffer[MP4DEMUX_MAX_TRACK];
    unsigned int *puiCTTSBuffer[MP4DEMUX_MAX_TRACK]; // Ctts for every sample: ct = dt + ctts
    unsigned int puiCTTSCntInBuffer[MP4DEMUX_MAX_TRACK];
    unsigned int *puiSTSSBuffer; // Index number for every sync sample
    unsigned int uiSTSSCntInBuffer;

    // for getting data from pucInfoBuffer to BufferSyncSample...
    unsigned int uiStartPos4STSS;            // start pos of STSS in pucInfoBuffer
    unsigned int puiStartPos4STTS[MP4DEMUX_MAX_TRACK]; // of STTS
    unsigned int puiStartPos4STSZ[MP4DEMUX_MAX_TRACK]; // of STSZ
    unsigned int puiStartPos4STSC[MP4DEMUX_MAX_TRACK]; // of STSC
    unsigned int puiStartPos4STCO[MP4DEMUX_MAX_TRACK]; // of STCO
    unsigned int puiStartPos4CTTS[MP4DEMUX_MAX_TRACK]; // of CTTS

    unsigned int uiCurOffset4STSS; // current read offset of STSS in pucInfoBuffer
    unsigned int puiCurOffset4STTS[MP4DEMUX_MAX_TRACK]; // of STTS
    unsigned int puiCurOffset4STSZ[MP4DEMUX_MAX_TRACK]; // of STSZ
    unsigned int puiCurOffset4STSC[MP4DEMUX_MAX_TRACK]; // of STSC
    unsigned int puiCurOffset4STCO[MP4DEMUX_MAX_TRACK]; // of STCO
    unsigned int puiCurOffset4CTTS[MP4DEMUX_MAX_TRACK]; // of CTTS

    // record info of MBUFFERSYSStartPos in sample table
    // for stts, we need to record total duration of already parsed data
    //          and current sample index in startpos of bufferSampleDuration
    // for stsc, we need to record the index of first sample in bufferSampleChunk,
    //          the index-in-chunk of it and the chunk index of last sample(sample before first sample)
    unsigned int puiFirstSampleIndexInBufferSTTS[MP4DEMUX_MAX_TRACK]; // stts
    unsigned int puiTotalDurationInBufferSTTS[MP4DEMUX_MAX_TRACK];
    unsigned int puiTotalDuration[MP4DEMUX_MAX_TRACK];
    unsigned int puiFirstSampleIndexInBufferSTSC[MP4DEMUX_MAX_TRACK]; // stsc
    unsigned int puiChunkIndexOfLastSampleInSYSBfSTSC[MP4DEMUX_MAX_TRACK]; // Index of last chunk dealed with
    unsigned int puiFirstSampleIndexInChunkInSYSBfSTSC[MP4DEMUX_MAX_TRACK]; // Index in chunk of first sample
    unsigned int puiFirstSampleIndexInBufferSTSZ[MP4DEMUX_MAX_TRACK]; // stsz
    unsigned int uiCurSyncSampleIndexInSTSS; // stss
    unsigned int uiNextSyncSampleIndexInSTSS;
    unsigned int uiFirstSampleIndexInBufferSTSS;
    unsigned int puiFirstSampleIndexInBufferCTTS[MP4DEMUX_MAX_TRACK]; // ctts
    unsigned int puiFirstChunkIndexInBufferSTCO[MP4DEMUX_MAX_TRACK]; // stco

    // info for parsing data
    unsigned int uiBytes2Advance; // when uiBytes2Advance > 0, we just advance data in pSYSBuffer or file
    unsigned int uiBytes2Copy;  // when uiBytes2Copy > 0, we just copy data from sysbuffer or file to esbuffer
    int bIfAfterCopy;   // if we just finish copying data from sysbuffer or file to esbuffer
    int bIfAddedCodecInfo2ESBuffer[MP4DEMUX_MAX_TRACK]; // if has already added codec info into the head of es buffer

    int iCurTrack; // current parsing track
    unsigned int puiNextSampleIndex[MP4DEMUX_MAX_TRACK]; // index of next sample
    unsigned int puiNextSampleOffset[MP4DEMUX_MAX_TRACK]; // offset of next sample

    long long pllCurrentPTS[MP4DEMUX_MAX_TRACK]; // movie time scale, pts of current sample

    long long llSeekOffset; // seek offset
    unsigned int uiOffsetOfSYSBufferInFile; // record the offset of first input data in SYSBuffer,
    // should be updated when seek,
    // current offset = uiOffsetOfSYSBufferInFile + MBUFFERSYSNumBytesOutput

    int iStatus;

    int bIfSeekSignalReceived;
    unsigned int uiSeekTime;

    unsigned char pucTempBuf[MP4DEMUX_SIZE_INFOSYSBUFFER_PARSED];
    unsigned char *pucData;
    unsigned int uiDataSize;
} MP4DEMUXCONTEXT;

/**
 *  MP4DemuxSetOutputBuffers
 *  @abstract       Defines the output buffers, need to set these buffers if parse all samples.
 *                  Need to set output buffers before parse headers if in MP4DEMUX_OPTION_PARSE_ALL_SAMPLES
 *
 *  @param[in]      pVideoESBuffer      Output video ESBuffer
 *  @param[in]      pAudioESBuffer      Output audio ESBuffer
 *  @param[in]      pMP4DemuxContext    Demux context
 *
 *  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxSetOutputBuffers(MBUFFERESBuffer *pVideoESBuffer, MBUFFERESBuffer *pAudioESBuffer,
                                    MP4DEMUXCONTEXT *pMP4DemuxContext);


extern int MP4DemuxGetOutputBuffers(MBUFFERESBuffer **ppVideoESBuffer, MBUFFERESBuffer **ppAudioESBuffer,
    MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxSetTimeRange
 *  @abstract       Defines the time range to demux.
 *                  Note: not implement relevant functionality yet.
 *
 *  @param[in]      lStartTime          start time
 *  @param[in]      lEndTime            end time
 *  @param[in]      pMP4DemuxContext    Demux context
 *
 *  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxSetTimeRange(long long lStartTime, long long lEndTime, MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxDeInit
 *  @abstract       DeInits a mp4 demux context
 *
 *  @param[in]      pMP4DemuxContext    Demux context
 *
 *  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxDeInit(MP4DEMUXCONTEXT *pMP4DemuxContext);


#if 0
/**
 *  MP4DemuxGetFirstIDR
 *  @abstract       Generates the first IDR frame and put it into the SYSBuffer.
 *                  Has to ensure that parsing headers succeeded.
 *
 *  @param[in,out]  pSYSBuffer          a large enough buffer to put the whole frame
 *  @param[in]      pMP4DemuxContext    Demux context
 *
 *  @return         MP4DEMUX_EC_FAILURE if failure(like SYSBuffer do not have enough space, 
 *                  fail to read data from the file, fail to seek to the frame position.)
 *                  MP4DEMUX_EC_OK if successfully generate the frame to the SYSBuffer
*/
extern int MP4DemuxGetFirstIDR(MBUFFERSYSBuffer *pSYSBuffer, MP4DEMUXCONTEXT *pMP4DemuxContext);
#endif

/**
 *  MP4DemuxGetFrameRate
 *  @abstract       Calculates the frame rate of the mp4 file, need to use it
 *                  after MP4DemuxParseHeaders().
 *
 *  @param[in]      pMP4DemuxContext
 * 
 *  @return         A float value indicate the fps
 *                  0 if data is not correct or haven't parsed the header.
*/
extern float MP4DemuxGetFrameRate(MP4DEMUXCONTEXT *pMP4DemuxContext);

#if 1
/**
 *  MP4DemuxSetIfAddExtraInfo
 *  @abstract       Defines whether add extra info before frame data when demux.
 *                  For H.264, extra info means SPS and PPS.
 *                  For AAC, extra info means ADTS header.
 *                  For now, we don't support stuffing extra info for other stream type.
 *
 *  @param[in]      bAddExtraInfo       1 means add while 0 means not
 *  @param[in]      pMP4DemuxContext
 *
 *  @return         MP4DEMUX_EC_OK
 */
extern int MP4DemuxSetIfAddExtraInfo(int bAddExtraInfo, MP4DEMUXCONTEXT *pMP4DemuxContext);
#endif

/**
 *  MP4DemuxGetTimescale
 *  @abstract       Get timescale for the MP4 file, only available after parse header.
 *
 *  @param[in]      pMP4DemuxContext
 *
 *  @return         unsigned int
 */
extern unsigned int MP4DemuxGetTimescale(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxGetDuration
 *  @abstract       Get duration for the MP4 file, only available after parse header.
 *
 *  @param[in]      pMP4DemuxContext
 *
 *  @return         duration in timescale from MP4DemuxGetTimescale()
 */
extern unsigned int MP4DemuxGetDuration(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxGetVideoCodec
 *  @abstract       Get video codec, if video track is not valid, the return value is negative
 *
 *  @param[in]      pMP4DemuxContext
 *
 *  @return         if video track is valid, return MP4_VIDEO_STREAM_XXX
 *                  else return MP4DEMUX_EC_FAILURE
 */
extern int MP4DemuxGetVideoCodec(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxGetAudioCodec
 *  @abstract       Get audio codec, if audio track is not valid, the return value is negative
 *
 *  @param[in]      pMP4DemuxContext
 *
 *  @return         if audio track is valid, return MP4_AUDIO_STREAM_XXX
 *                  else return MP4DEMUX_EC_FAILURE
 */
extern int MP4DemuxGetAudioCodec(MP4DEMUXCONTEXT *pMP4DemuxContext);

extern long long MP4DemuxGetSeekPosition(MP4DEMUXCONTEXT *pMP4DemuxContext);

//************************************
// MP4 demux full functionality
//************************************

/**
 *  MP4DemuxInit
 *  @abstract       Init a mp4 demux context.
 *
 *  @param[in]      pMP4DemuxContext
 *  @return         MP4DEMUX_EC_OK
 */
extern int MP4DemuxInit(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxSetOptions
*  @abstract       Defines the demux mode for the context. Default is to parse moov atom only.
*
*  @param[in]      iOptions            see MP4DEMUX_OPTION_XXXX
*  @param[in]      pMP4DemuxContext    Demux context
*
*  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxSetOptions(int iOptions, MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxSetSYSBuffer
*  @abstract       Specify the input SYS Buffer, which contains the input stream.
                   this func will also set iDataSource to be MP4DEMUX_DATASOURCE_FROM_BUFFER
*
*  @param[in]      pSYSBuffer
*  @param[in]      pMP4DemuxContext
*  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxSetSYSBuffer(MBUFFERSYSBuffer *pSYSBuffer, MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxSetFile
*  @abstract       Specify the input file, which contains the input stream.
                   this func will also set iDataSource to be MP4DEMUX_DATASOURCE_FROM_FILE
*
*  @param[in]      pFile
*  @param[in]      iBufferSize
*  @param[in]      pMP4DemuxContext
*  @return         MP4DEMUX_EC_OK
*/
extern int MP4DemuxSetFile(TPWFile *pFile, int iBufferSize, MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxGetFirstMediaOffset
*  @abstract       Get the offset in mp4 file of first media data.
*
*  @param[in]      pMP4DemuxContext
*  @return         offset if success
*                  MP4DEMUX_EC_FAILURE if failure
*/
extern int MP4DemuxGetFirstMediaOffset(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxParse
 *  @abstract       Parse the input stream,
 *
 *  @param[in]      pMP4DemuxContext
 *  @return         TODO
 */
extern int MP4DemuxParse(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxParseHeaders
*  @abstract       Starts to parse the moov, when to stop depends on the demux option.
*                  For default option, stop when finish fetcing enough information about first key frame.
*                  For all samples option, stop when finish parsing the whole moov atom.
*                  Note that the time scale of ES Buffers will be set after successfully parse headers in
*                  parsing all samples mode.
*
*  @param[in]      pMP4DemuxContext    Demux context
*
*  @return         MP4DEMUX_EC_OK if success
*                  MP4DEMUX_EC_FAILURE if failure
*/
extern int MP4DemuxParseHeaders(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxParseFileHeaders
*  @abstract       Parse the moov headers of mp4 file to get mp4 info or first key frame
*                  Only support local file, means MP4DemuxSetFile should be called first
*
*  @param[in]      pMP4DemuxContext    Demux context
*
*  @return         MP4DEMUX_EC_OK if success
*                  MP4DEMUX_EC_FAILURE if failure
*/
extern int MP4DemuxParseFileHeaders(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxParseAtom
*  @abstract       Parses a single atom.
*
*  @param[in]      pMP4DemuxContext    Demux context
*
*  @return         Return size of the atom if success
*                  MP4DEMUX_EC_OK if finish parsing the moov atom
*                  MP4DEMUX_EC_FAILURE if read/seek file failure or parse failure
*/
extern int MP4DemuxParseAtom(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
*  MP4DemuxParseData
*  @abstract       Read a sample from file and generate a frame into ES Buffer.
*                  Will switch data type base on PTS.
*
*  @param[in]      pMP4DemuxContext    Demux context
*
*  @return         MP4DEMUX_EC_OK
*                  MP4DEMUX_EC_GOT_VIDEO_FRAME
*                  MP4DEMUX_EC_GOT_AUDIO_FRAME
*                  MP4DEMUX_EC_FINISH          if finish demux the last sample for all the track
*                  MP4DEMUX_EC_FAILURE         if failure, like file operation failure
*                  MP4DEMUX_EC_BUFFER_FULL     if one of the output buffer is full
*/
extern int MP4DemuxParseData(MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxSeek
 *  @abstract       Set parse start time
 *
 *  @param[in]      uiStartTime         start time, beyond duration is permitted
 *  @param[in]      pMP4DemuxContext
 *  @return         MP4DEMUX_EC_OK
 */
extern int MP4DemuxSeek(unsigned int uiStartTime, MP4DEMUXCONTEXT *pMP4DemuxContext);

/**
 *  MP4DemuxGetSeekOffset
 *  @abstract       Get the seek offset
 *
 *  @param[in]      pMP4DemuxContext
 *  @return         offset to seek
 */
extern long long MP4DemuxGetSeekOffset(MP4DEMUXCONTEXT *pMP4DemuxContext);

// reset info buffer
extern int MP4DemuxResetBufferSampleChunk(int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext); // stsc
extern int MP4DemuxResetBufferSampleDuration(int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext); // stts
extern int MP4DemuxResetBufferCTTS(int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext); // ctts
extern int MP4DemuxResetBufferSyncSampleIndex(MP4DEMUXCONTEXT *pMP4DemuxContext);

// get
extern int MP4DemuxGetSampleOffsetBySampleIndex(int iIfSeek, unsigned int *puiSampleOffset, unsigned int uiSampleIndex,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetSampleIndexByOffset(unsigned int *puiSampleIndex, unsigned int uiSampleOffset,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);

extern int MP4DemuxGetChunkIndexBySampleIndex(int iIfSeek, unsigned int *puiChunkIndex, unsigned int *puiSampleIndexInChunk,
    unsigned int uiSampleIndex, int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetChunkOffsetByChunkIndex(unsigned int *puiChunkOffset, unsigned int uiChunkIndex,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetSampleSizeBySampleIndex(unsigned int *puiSampleSize, unsigned int uiSampleIndex,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetSampleIndexBySampleDuration(unsigned int *puiSampleIndex, unsigned int uiDuration,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetSampleDurationBySampleIndex(unsigned int *puiSampleDuration, unsigned int uiSampleIndex,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetClosedSyncSampleBySampleIndex(unsigned int *puiSyncSampleIndex, unsigned int uiSampleIndex,
    MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetCTTSBySampleIndex(unsigned int *puiSampleCTOffset, unsigned int uiSampleIndex,
    int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxIsKeyFrame(unsigned int uiSampleIndex, int iTrack, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetNextSampleTrackByOffset(int *piTrack, unsigned int uiOffset, MP4DEMUXCONTEXT *pMP4DemuxContext);
extern int MP4DemuxGetFirstAudioInfoAfterSeek(unsigned int *puiDuration, unsigned int uiOffset, MP4DEMUXCONTEXT *pMP4DemuxContext);

#if defined(__cplusplus)
}
#endif
#endif
