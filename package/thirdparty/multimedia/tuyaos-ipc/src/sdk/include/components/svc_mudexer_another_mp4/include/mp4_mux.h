#ifndef _MP4MUX_H_
#define _MP4MUX_H_
#pragma  once
#if defined(__cplusplus)
extern "C"
{
#endif
#include "mediainfo.h"
#include "mbuffer.h"
#include <time.h>
    // mp4mux is not only used for generating mp4 files, but can also genarate data in memory
    // to be sent out via network.

    // size: include the 4 bytes used to indicate atom size
    // important atoms :
    //      moov
    //          mvhd
    //          trak
    //              tkhd
    //              stbl
    //                  stts: timestamp
    //                  stss: keyframe
    //                  stsz: sampe size
    //                  stco: chunk offset
    //      mdat (sample)
    //
    // Usually we put samples right after header, and collect PTS/size of samples
    // to build stbl, which is put at the end of file with movie atom
    //
    // A video stream can be very long so sometimes it's impractical to allocate memory to
    // hold all the data used by stbl. We may have multiple mdat atoms and moov atoms.

#define MP4MUX_LLONG_MAX 9223372036854775807;        //maximum signed long long int value

// #define MP4MUX_SHARED_FRAMEINFO

#define MP4MUX_SHARED_MAX_NUM_FRAME  10800

#define MP4MUX_MAX_SLICE_IN_FRAME      136

#define MP4MUX_MIN_COMPRESS_SIZE  (1024*10)             //最小压缩大小，用于IN-PLACE 模式，可压缩空间小于此值则不压缩

#define MP4MUX_MAX_STRING_LENGTH            128

#define MP4MUX_VIDEO_ES_INDEX               0
#define MP4MUX_AUDIO_ES_INDEX               1

#define MP4MUX_INFO_BUFFER_SIZE             40000000
#define MP4MUX_MAX_NUM_FRAME                800000      // 1GB文件最对可保存的帧数：(1 * 1024 * 1024)K / 32(K/s) * 20fps = 655360
#define MP4MUX_TS_TIME_SCALE                90000       // 90kHz
#define MP4MUX_MOVIE_TIME_SCALE             1000        // time scale for the MP4 file
#define MP4MUX_DURATION_THRESHOLD           180000      // 2秒，基于单位换算
#define MP4MUX_DEFAULT_DURATION             3600        // 40毫秒，参考25fps得到，基于单位换算

#define MP4MUX_ATOM_SIZE_LENGTH             4

#define MP4MUX_ADTS_HEADER_LENGTH           (7)

#define MP4MUX_TRACK_INFO_UNDEFINED         -1 // if we do not known the track info when we init a mux context
#define MP4MUX_TRACK_INFO_VIDEO_AUDIO       0
#define MP4MUX_TRACK_INFO_VIDEO_ONLY        1
#define MP4MUX_TRACK_INFO_AUDIO_ONLY        2

#define MP4MUX_DATATYPE_SWITCH_THRESHOLD    6000

#define MP4MUX_OUTPUT_OPTION_DEFAULT        1   //默认模式，写完源数据再写索引数据
#define MP4MUX_OUTPUT_OPTION_IN_PLACE       2   //固定索引位置模式。以一定频率写入索引数据，需预设置文件长度。
#define MP4MUX_OUTPUT_OPTION_TWO_PASS       3   //双程模式。以一定频率在临时文件写入索引数据，写完后需恢复。
#define MP4MUX_OUTPUT_OPTION_REALTIME       4   // 实时模式，每处理一定数量的帧后写入一次索引数据，无需对所有帧保存数据结构

#define MP4MUX_OUTPUT_PERIOD                50  //单位 帧

#define MP4MUX_LENGTH_PER_MIN               3000000 //预估每分钟数据长度

#define MP4MUX_MAX_FRAME_PER_SEGMENT  10000    //每个MP4片段的最大帧数
#define MP4MUX_DATA_INIT_OFFSET 500

    // error codes
#define MP4MUX_EC_OK                        0
#define MP4MUX_EC_GOT_EXTRAINFO             2
#define MP4MUX_EC_FAILURE                   -1
#define MP4MUX_EC_OUT_OF_RANGE              -3
#define MP4MUX_EC_INVALID_CONFIG            -4
#define MP4MUX_EC_MISSING_FRAME_DATA        -5
#define MP4MUX_EC_FRAME_NOT_ENOUGH          -6
#define MP4MUX_EC_SKIP_FRAME                -7
#define MP4MUX_EC_NEED_UPDATE_BOX           -8

#define MP4MUX_DATASOURCE_ESBUFFER    0
#define MP4MUX_DATASOURCE_FRAMEINFOQUEUE  1

#define MP4MUX_FRAMEINFOQUEUE_DEFAULT_FRAMETYPE -1

#define MP4MUX_FIND_PTS_RANGE 8

#define MPTMUX_VIDEO_SAMPLE_PER_CHUNK 1
#define MPTMUX_AUDIO_SAMPLE_PER_CHUNK 20

/* Box size */
#define MP4MUX_BOX_VIDEO_STXX_BYTES 32
#define MP4MUX_BOX_AUDIO_STXX_BYTES 28
#define MP4MUX_BOX_OTHER_SIZE 1500
#define MP4MUX_BOX_FIRST_TRAK_OTHER_SIZE 800
/* Size + tag */
#define MP4MUX_BOX_HEAD_MIN_SIZE 8
/* Size + tag + version&flag + entry count */
#define MP4MUX_BOX_HEAD_MAX_SIZE 16
#define MP4MUX_BOX_MVHD_SIZE 108
#define MP4MUX_BOX_TKHD_SIZE 92
#define MP4MUX_BOX_ELST_MIN_SIZE 28
#define MP4MUX_BOX_ELST_MAX_SIZE 40
#define MP4MUX_BOX_MDHD_SIZE 32
#define MP4MUX_BOX_HDLR_SIZE 45
#define MP4MUX_BOX_VMHD_SIZE 20
#define MP4MUX_BOX_SMHD_SIZE 16
#define MP4MUX_BOX_URL_SIZE 12

#define swapint(x) x = (((x >> 24) & 0xff) | ((x >> 8) & 0xff00) | \
    ((x << 8) & 0xff0000) | ((x << 24) & 0xff000000))
#define swapshort(x) x = (((x & 0xff) << 8) | ((x >> 8) & 0xff))

    typedef struct {
        int i;
    } MP4HEADER;

    // Time-to-Sample stts
    typedef struct
    {
        unsigned int uiSampleCount:14;
        unsigned int uiSampleDuration:18; // in the scale of media's timescale
    } MP4STTSENTRY;

    // Sample-to-Chunk sttc
    typedef struct
    {
        unsigned short uiFirstChunk;
        unsigned short uiSamplesPerChunk;
        unsigned short uiSampleDescIndex;
    } MP4STSCENTRY;

    //chunck offset stco
    typedef struct
    {
        unsigned int uiSamplesPerChunk;
        unsigned int uiChunkOffset;
    } MP4STCOENTRY;
    // Composition Time to Sample
    // Uses when there is B frame and the PTS is not ascending.
    typedef struct
    {
        unsigned int uiSampleCount:10;
        unsigned int uiSampleOffset:22; // PTS - DTS
    } MP4CTTSENTRY;

    typedef struct MP4MUXFRAMEINFO {
        long long llPTS;
#ifdef MP4MUX_ENABLE_DTS
        long long llDTS;
#endif
        int iOffsetInFile;
        unsigned int uiSampleSize:20;  // stsz, video: 4 byte(file length) + nalu length(without start code)
        unsigned int isKeyFrame:1;

#ifdef MP4MUX_SHARED_FRAMEINFO
        int iFileNameIndex;
#endif
    } MP4MUXFRAMEINFO;

    typedef struct MP4MUX_FRAMEINFO_CONTEXT {
        char pcFileName[3][64];
        // int iCurrentFileIndex;
        int iHead[2];
        //int iTail[2];
        int iSize[2];
        int iTimeScale;
        MP4MUXFRAMEINFO *pFrameInfo[2];
    }MP4MUXFRAMEINFOCONTEXT;

    typedef struct MP4MUX_MOVIE_ATOM_OFFSETS_AND_SIZES
    {
        int iMoovSize;
        // int iMvhdOffset;

        /* Video */
        // int iVideoTrakOffset;
        int iVideoTrakSize;
        // int iVideoTkhdOffset;
        // int iVideoEdtsOffset;
        // int iVideoElstOffset;
        // int iVideoMdiaOffset;
        int iVideoMdiaSize;
        // int iVideoMdhdOffset;
        // int iVideoHdlrOffset;
        // int iVideoMinfOffset;
        int iVideoMinfSize;
        // int iVideoVmhdOffset;
        // int iVideoDinfOffset;
        // int iVideoDrefOffset;
        // int iVideoUrlOffset;
        // int iVideoStblOffset;
        int iVideoStblSize;
        // int iVideoStsdOffset;
        // int iVideoDescOffset;
        int iVideoSttsOffset;
        int iVideoSttsEntryOffset;
        int iVideoStssOffset;
        int iVideoStssEntryOffset;
        int iVideoStscOffset;
        int iVideoStscEntryOffset;
        int iVideoStszOffset;
        int iVideoStszEntryOffset;
        int iVideoStcoOffset;
        int iVideoStcoEntryOffset;

        /* Audio */
        int iAudioTrakOffset;
        int iAudioTrakSize;
        // int iAudioTkhdOffset;
        // int iAudioEdtsOffset;
        // int iAudioElstOffset;
        // int iAudioMdiaOffset;
        int iAudioMdiaSize;
        // int iAudioMdhdOffset;
        // int iAudioHdlrOffset;
        // int iAudioMinfOffset;
        int iAudioMinfSize;
        // int iAudioSmhdOffset;
        // int iAudioDinfOffset;
        // int iAudioDrefOffset;
        // int iAudioUrlOffset;
        // int iAudioStblOffset;
        int iAudioStblSize;
        // int iAudioStsdOffset;
        // int iAudioDescOffset;
        int iAudioSttsOffset;
        int iAudioSttsEntryOffset;
        int iAudioStscOffset;
        int iAudioStscEntryOffset;
        int iAudioStszOffset;
        int iAudioStszEntryOffset;
        int iAudioStcoOffset;
        int iAudioStcoEntryOffset;
    } MP4MUXMOVIEATOMOFFSETSANDSIZES;

    // to build a mp4 file which has header, data and index, we allocate:
    //  * a small buffer (no wraparound needed) for header,
    //  * a "larger" buffer (no wraparound needed) for index,
    //  * sys buffer for data (so we can accumulate a big chunk of data and write it once)
    typedef struct {
        //// info. we may want to keep when calling MP4MuxReset()
        /// input data info.
        int iNumTracks;
        int iTrackInfo; // MP4MUX_TRACK_INFO_VIDEO_AUDIO/MP4MUX_TRACK_INFO_VIDEO_ONLY/MP4MUX_TRACK_INFO_AUDIO_ONLY
        int iMaxNumFrame; // we use this value to decide how many memory to allocate
        int iFrameInterval; // Indicate the interval of frames between two of updating box
        // codec
        int iVideoCodec;
        int iAudioCodec;

        // for H.264/H.265
        unsigned int uiImageWidth;
        unsigned int uiImageHeight;
        int iVPSLength; //vps buffer length without start code, for H.265
        unsigned char pucVPS[MP4MUX_MAX_STRING_LENGTH]; //vps buffer without start code, for H.265
        int iSPSLength; //sps buffer length without start code
        unsigned char pucSPS[MP4MUX_MAX_STRING_LENGTH]; //sps buffer without start code
        int iPPSLength; // pps buffer length without start code
        unsigned char pucPPS[MP4MUX_MAX_STRING_LENGTH]; //pps buffer without start code

        // for AAC ADTS
        int iADTSHeaderParsed;
        unsigned int uiChannelConfiguration;    // number of channels
        unsigned int uiSamplingFrequencyIndex;  // sample rate
        unsigned int uiMaxBitrate;
        unsigned int uiAvgBitrate;

        //// info. we may want to reset (clear) when calling MP4MuxReset()
        /// input data
        MBUFFERESBuffer *pESBuffer[2];

        /// output file and temp file used for recovery
        unsigned int uiMP4FileSize; // The size of MP4 file, only be used for realtime mode
        int iMP4Fd;
        int iRecoveryInfoFd;

        /// settings by high level
        int uiOutputOption;    //MP4MUX_OUTPUT_OPTION_DEFAULT/MP4MUX_OUTPUT_OPTION_IN_PLACE/MP4MUX_OUTPUT_OPTION_TWO_PASS

        /// data structure to hold index info.
        int uiNumSampleDesc;
        unsigned int *puiSampleDesc[2];

        unsigned int uiNumCTTSEntry;
        MP4CTTSENTRY *pCTTSEntry;  // ctts
        int bNeedCTTS;

        int puiNumSamples[2]; // 总sample数
        int piSampleInterval[2]; // 两次更新box信息之间处理的sample数，仅对实时模式有效

        int uiNumKeyFrames;
        unsigned short *puiKeyFrame; // stss

        int puiNumSTTSEntry[2];
        MP4STTSENTRY *pSTTSEntry[2]; // stts
        long long llSTTSEntryLastSamplePTS[2];

        int puiNumSTSCEntry[2];
        MP4STSCENTRY *pSTSCEntry[2]; // stsc

        int puiNumSTCOEntry[2];
        MP4STCOENTRY *pSTCOEntry[2]; // stco

        /// buffer to hold header/index to be written to file
        MBUFFERByteArray * pInfoByteArray;

        // running status
        int uiCurrentOffset; // the current offset of the mp4 file
        int iLastSampleESIndex; // -1:init value 0:video es 1:audio es
        int bFlush;             // value set by other module. If set the last audio/video frame in ESBuffer is complete and so we can output it to file 
        unsigned int puiDurationInMovieTimescale[2];

        // used to skip non-IDR video frames (together with accompanying audio frames in time) so we have IDR as the first video frame in MP4 file
        int bHasGotIDRFrame;
        long long lFirstIDRPTS;

        int iCheckMultiSliceInFrame;

        int uiIndexOffset;     //索引数据在文件中的偏移量
        MP4MUXMOVIEATOMOFFSETSANDSIZES MovieAtomOffsetsAndSizes; // 各box在文件中的偏移量

        int uiLastReorderPTSIndex;

        MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext;
        int piFrameInfoHead[2];
        int piFrameInfoIndex[2];
        int iDataSource;
        int iIsInChargeOfFrameInfoContext;

        int iMinSpatialSegmentationIdc;
        int iChromaFormatIdc;
        int iBitDepthLumaMinus8;
        int iBitDepthChromaMinus8;


        unsigned long ulCurrentTime; // seconds since 1904-1-1 00:00:00
        unsigned int puiTimeScale[2]; // timescale for each media, usually different from movie's timescale
        unsigned int puiDuration[2]; // in the scale of the media's timescale
        unsigned int puiStartTime[2]; // in the scale of the movie's timescale

        unsigned int uiMdatSizePos;
        unsigned char pucMdatSizeField[MP4MUX_ATOM_SIZE_LENGTH];

        long long llPreTimeStampMilliSec;

        long long pllPrePTS[2]; // previous PTS for each track
        long long pllPTSInterval[2]; // PTS adjustment value, initial value is 0
        int pbIfDiscontinuityEnd[2]; // flag about whether encounter a discontinuity end flag

        int iIfNotFirstlyUpdateMovieAtom;
    } MP4MUXCONTEXT;


    typedef struct MP4MUX_RECOVER_DATA{
        int iTrackInfo;
        unsigned long ulCurrentTime;
        unsigned int uiImageWidth;
        unsigned int uiImageHeight;
        int iADTSHeaderParsed;
        unsigned int uiChannelConfiguration;    // number of channels
        unsigned int uiSamplingFrequencyIndex;  // sample rate
        int iSPSLength; //sps buffer length without start code
        unsigned char pucSPS[MP4MUX_MAX_STRING_LENGTH]; //sps buffer without start code
        int iPPSLength; // pps buffer length without start code
        unsigned char pucPPS[MP4MUX_MAX_STRING_LENGTH]; //pps buffer without start code
        int puiNumSamples[2];
        int uiCurrentOffset;
        unsigned int uiMdatSizePos;
    }MP4MUXRECOVERDATA;

    /*==========================================================================================
     *
     *                                 Public Method
     *
     *==========================================================================================
     */
    /**
     *  @method         MP4MuxInit
     *  @abstract       Init a MP4MUXCONTEXT. Only allocate resources if track info is defined.
     *
     *  @param (in)     iFrameMaxCount  approximate the max count of frame to be muxed. If set to zero, a default value will be used.
     *  @param (in)     iFrameInterval  approximate the count of frame between two boxes updating, only used for realtime mode.
     *  @param (in)     iTrackInfo      see MP4MUX_TRACK_INFO_XXX, to determine how many tracks the mp4 will have.
     *  @parma (in)     iOutputOption   mux option macro.
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxInit(int iFrameMaxCount, int iFrameInterval, int iTrackInfo, int iOutputOption,
        MP4MUXFRAMEINFOCONTEXT *pFrameInfoContext, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxSetTrackInfo
    *  @abstract       Only allocate resources if track info is defined.
    *
    *  @param[in]      iTrackInfo      see MP4MUX_TRACK_INFO_XXX, to determine how many tracks the mp4 will have.
    *  @param[in]      iCount          use for buffer size
    *  @param[in,out]  pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetTrackInfo(int iTrackInfo, int iCount, MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxDeInit
     *  @abstract       Releases any resources that the context owns.
     *
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxDeInit(MP4MUXCONTEXT *pMP4MuxContext);

    extern int MP4MuxSetOutputFile(int iFd, unsigned int uiSize, MP4MUXCONTEXT *pMP4MuxContext);

    extern int MP4MuxSetRecoveryInfoFile(int iRecoveryInfo, MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxReset
     *  @abstract       Saving video and audio parameters, re-init MP4MUXCONTEXT, then set parameters back to it
     *
     *  @param (in-out) pMP4MuxContext  the context instance
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxReset(int iDataSize, int iTrackInfo, MP4MUXFRAMEINFOCONTEXT *pFrameInfoContext,
        MP4MUXCONTEXT *pMP4MuxContext);


    /**
    *  @method         MP4MuxSetVideoCodec
    *  @abstract       set video codec
    *
    *  @param (in-out) pMP4MuxContext  the context instance
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetVideoCodec(int iCodec, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxSetAudioCodec
    *  @abstract       set audio codec
    *
    *  @param (in-out) pMP4MuxContext  the context instance
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetAudioCodec(int iCodec, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxSetCheckMultiSliceInFrame
    *  @abstract       set check slice flag
    *
    *  @param (in-out) pMP4MuxContext  the context instance
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetCheckMultiSliceInFrame(int iCheck, MP4MUXCONTEXT *pMP4MuxContext);


    /**
     *  @method         MP4MuxSetInputBuffers
     *  @abstract       Defines the input buffers of the mp4 mux context.
     *                  Here we just store the pointers of these buffers, you are
     *                  responsible for keeping them validly through out the context.
     *
     *  @param (in)     pVideoESBuffer  pointer of video ESBuffer, could be NULL
     *  @param (in)     pAudioESBuffer  pointer of audio ESBuffer, could be NULL
     *  @param (in-out) pMP4MuxContext  pointer of mp4 mux context
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxSetInputBuffers(MBUFFERESBuffer *pVideoESBuffer, MBUFFERESBuffer *pAudioESBuffer, MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxSetImageSize
     *  @abstract       Set up the image width and height of the mp4 file.
     *                  This method is deprecated, we now extract the image width and height from sps.
     *
     *  @param (in)     uiImageWidth    the width of the video track
     *  @param (in)     uiImageHeight   the height of the video track
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxSetImageSize(unsigned int uiImageWidth, unsigned int uiImageHeight, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxSetDataSource
    *  @abstract         set media data source
    *
    *  @param          iDataSource：datasource
    (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetDataSource(int iDataSource, MP4MUXCONTEXT *pMP4MuxContext);
    /**
    *  @method         MP4MuxSetCurrentTime
    *  @abstract       Specify the creation time of the mp4 file.
    *
    *  @param (in)     CurrentTime     seconds since 1904-1-1 00:00:00.
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetCurrentTime(time_t CurrentTime, MP4MUXCONTEXT *pMP4MuxContext);

#if 0
    /**
    *  @method         MP4MuxSetOutputOption
    *  @abstract       Set output option.Three option can be choosed.
    *
    *  @param (in)     option          option macro.
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetOutputOption(int option, MP4MUXCONTEXT *pMP4MuxContext);
#endif

    /**
    *  @method         MP4MuxSetTotalDuration
    *  @abstract       PreSet diration of MP4 file
    *
    *  @param (in)     minutes         duration.
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetTotalDuration(int minutes, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxSetMoovAndTrakInfo
    *  @abstract       Pre-set moov offset and size and trak size of MP4 file, only for realtime mode.
    *
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    */
    extern int MP4MuxSetMoovAndTrakInfo(MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxCheckDataFull
    *  @abstract       Check whether media data is full, only for in-place mode.
    *
    *  @param (in)     iNumBytes       byte count of new media data.
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_EC_OK
    *                  MP4MUX_EC_OUT_OF_RANGE
    */
    extern int MP4MuxCheckDataFull(unsigned int iNumBytes, MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxResetByteInfoArray
     *  @abstract       Resets the pucInfoBuffer, which means the buffer is now empty.
     *
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */

    extern int MP4MuxResetByteInfoArray(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxPutHeader
     *  @abstract       Generates header atoms and put it into context's pucInfoBuffer.
     *                  The next step is to put this buffer to the target mp4 file.
     *
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxPutHeader(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxPutmdat
     *  @abstract       Generates a mdat atom and put it into context's pucInfoBuffer.
     *                  The next step is to put this buffer to the target mp4 file.
     *
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxPutmdat(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxPutData
     *  @abstract       Parses the data from ES buffer, remove the useless parts and
     *                  record the information of the data for future usage.
     *                  If the process is successful, the next step is to put the data
     *                  in the ES buffer to the target mp4 file according to the return value.
     *
     *  @param (in)     iESIndex        The ESBuffer index, see MP4MUX_XXXX_ES_INDEX, specifies the data type to process.
     *                                  Or pass -1 and it will automatically choose one data type.
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_FAILURE           If the ES buffer has no complete frame data
     *                  MP4MUX_VIDEO_ES_INDEX       If successfully parse a video frame buffer
     *                  MP4MUX_AUDIO_ES_INDEX       If successfully parse a audio frame buffer
     *                  MP4MUX_EC_GOT_EXTRAINFO     If successfully extract a extra info frame
     *                  MP4MUX_EC_INVALID_CONFIG    If failed to extract a extra info frame
     */
    extern int MP4MuxPutData(int iESIndex, MP4MUXCONTEXT *pMP4MuxContext);


    /**
     *  @method         MP4MuxPutMovieAtom
     *  @abstract       Generates a movie atom and put it into context's pucInfoBuffer.
     *
     *  @param (in-out) pMP4MuxContext  the context instance.
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxPutMovieAtom(MP4MUXCONTEXT *pMP4MuxContext);


    /**
    *  @method         MP4MuxNextDataType
    *  @abstract       Create next data type.
    *
    *  @param (in-out) pMP4MuxContext  the context instance.
    *
    *  @return         MP4MUX_AUDIO_ES_INDEX
    *                  MP4MUX_VIDEO_ES_INDEX
    */
    extern int MP4MuxNextDataType(MP4MUXCONTEXT *pMP4MuxContext);


    extern int MP4MuxWriteInfoByteArray(MP4MUXCONTEXT *pMP4MuxContext);
    /**
     *  @method         MP4MuxWriteIndex
     *  @abstract       write index to file
     *
     *  @param (in-out) pMP4MuxContext  the context instance
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxWriteIndex(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxRecover
     *  @abstract       Recover from last mux
     *
     *  @param (in-out) pMP4MuxContext  the context instance
     *
     *  @return         MP4MUX_EC_OK
     */
    extern int MP4MuxRecover(MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method       MP4MuxCompact
    *  @abstract     Compress file when finish record.Only use in IN-PLACE mode.
    *  @param        MP4MUXCONTEXT
    *  @return       MP4MUX_EC_OK
    */
    extern int MP4MuxCompact(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxGetCurrentPTS
     *  @abstract
     *
     *  @param          type: MP4MUX_VIDEO_ES_INDEX/MP4MUX_AUDIO_ES_INDEX
     *
     *  @return         PTS
     */
    extern long long MP4MuxGetCurrentPTS(int type, MP4MUXCONTEXT *pMP4MuxContext);

    /**
    *  @method         MP4MuxGetCurTimeStampInMilliSecond
    *  @abstract       get time stamp of the frame just muxed, in millisecond.
    *
    *  @param
    *
    *  @return         time stamp in millisecond.
    */
    extern long long MP4MuxGetCurTimeStampInMilliSecond(MP4MUXCONTEXT *pMP4MuxContext);

    /**
     *  @method         MP4MuxGetCurrentFrameInfo
     *  @abstract
     *
     *  @param
     *
     *  @return         Frame info
     */
    extern MP4MUXFRAMEINFO * MP4MuxGetCurrentFrameInfo(int type, MP4MUXCONTEXT *pMP4MuxContext);


    /*========================================================================================
     *
     *                               Private Method
     *
     *========================================================================================
     */

    /*=========================== mp4 mux operation ==========================================*/
    int MP4MuxPutBE(unsigned int uiValue, unsigned int uiNumBytes, unsigned char *pucPos);
    extern int MP4MuxIfCheckMultiSliceInFrame(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxAdvanceOffset(unsigned int iNumBytes, MP4MUXCONTEXT *pMP4MuxContext);
    extern unsigned int MP4MuxGetDuration(int iTrackIndex, int iSampleIndex, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxCalculateAudioBitrate(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxIfCheckMultiSliceInFrame(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxParseADTSHeader(unsigned char *pucHeaderPos, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxFrameInfoNextDataType(int videoIndex, int audioIndex, MP4MUXFRAMEINFOCONTEXT *pMP4MuxFrameInfoContext);
    extern int MP4MuxCalculateMdatSize(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateCTTS(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTCO(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTTSForVideoAndAudio(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTTS(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTSS(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTSZ(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxGenerateSTSC(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxSetTimeScale(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxCalculateDuration(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxParseSPS(MP4MUXCONTEXT *pMP4MuxContext);
    /*=========================== mp4 mux atom  ==========================================*/
    extern int MP4MuxPutMVHD(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutTKHD(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutELST(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutEDTS(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutMDHD(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutHDLR(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutVMHD(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSMHD(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutURL(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutDREF(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutDINF(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutESDS(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutAVCC(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutHVCC(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutAVC1(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutCTTS(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTSD(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTTS(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTSS(MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTSC(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTSZ(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTCO(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutSTBL(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutMINF(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutMDIA(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxPutTRAK(int iTrak, MP4MUXCONTEXT *pMP4MuxContext);

    /*===================Recover Mode Relative=====================*/


    extern int MP4MuxWriteFrameInfo(int iType, MP4MUXCONTEXT *pMP4MuxContext);
    extern int MP4MuxWriteRecoverData(MP4MUXCONTEXT *pMP4MuxContext);

    /*=========================== FrameInfo Method ==========================================*/
    /**
    *  @method       MP4MuxFrameInfoContextInit
    *  @abstract     Init Frame info context which is used to managed frame info
    *  @param        size: Size of  Frame info queue
    *
    *  @return       MP4MUX_EC_OK
    */
    extern int MP4MuxFrameInfoContextInit(int size, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextDeInit
    *  @abstract     DeInit Frame info context which is used to managed frame info.Release memory of queue.
    *
    *  @return       MP4MUX_EC_OK
    */
    extern int MP4MuxFrameInfoContextDeInit(MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextAdvance
    *  @abstract     Advance frame info queue
    *  @param        type: is video or audio
    *
    *  @return       MP4MUX_EC_OK
    */
    extern int MP4MuxFrameInfoContextAdvance(int type, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextAppend (Not in use at present )
    *  @abstract     Append frame info queue
    *  @param        type: is video or audio
    *
    *  @return       MP4MUX_EC_OK
    */
    extern int MP4MuxFrameInfoContextAppend(int type, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextSpaceAvailable (Not in use at present )
    *  @abstract     Get space of frame info queue
    *  @param        type: is video or audio
    *
    *  @return       space of queue
    */
    extern int MP4MuxFrameInfoContextSpaceAvailable(int type, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextLength (Not in use at present )
    *  @abstract     Get length of frame info in queue
    *  @param        type: is video or audio
    *
    *  @return       length of  frame info which is available
    */
    extern int MP4MuxFrameInfoContextLength(int type, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContextGetFrameInfo
    *  @abstract     Get the frame info by index in queue
    *  @param        type: is video or audio
    *                index: index of taget frame info in queue
    *  @return       Frame info struct
    */
    extern MP4MUXFRAMEINFO* MP4MuxFrameInfoContextGetFrameInfo(int type, int index, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContexGetPreviousFramInfoByPTS
    *  @abstract     Get the frame info by PTS in queue
    *  @param        type: is video or audio
    *                PTS: PTS of taget frame info in queue
    *  @return       Frame info struct
    */
    extern MP4MUXFRAMEINFO* MP4MuxFrameInfoContexGetPreviousFramInfoByPTS(int type, long long  PTS, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);
    /**
    *  @method       MP4MuxFrameInfoContexGetPreviousKeyFrameInfo （Deprecate）
    *  @abstract     Get the previous keyframe info
    *  @param        type: is video or audio
    *                second: forward time in seconds
    *  @return       Frame info struct
    */
    extern MP4MUXFRAMEINFO*  MP4MuxFrameInfoContexGetPreviousKeyFrameInfo(int type, int second, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);
    /**
    *  @method       MP4MuxFrameInfoContexGetPreviousKeyFrameIndex （Deprecate）
    *  @abstract     Get the previous keyframe index
    *  @param        type: is video or audio
    *                second: forward time in seconds
    *  @return       target keyframe index in queue
    */
    extern int  MP4MuxFrameInfoContexGetPreviousKeyFrameIndex(int type, int second, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    /**
    *  @method       MP4MuxFrameInfoContexGetPreviousFrameIndexByPTS
    *  @abstract     Get the previous frame index
    *  @param        type: is video or audio
    *                PTS: PTS of taget frame info in queue
    *  @return          target keyframe index in queue
    */
    extern int  MP4MuxFrameInfoContexGetPreviousFrameIndexByPTS(int type, long long  PTS, MP4MUXFRAMEINFOCONTEXT * pFrameInfoContext);

    extern int MP4MuxCheckFrameCount(unsigned int uiFrameCnt, MP4MUXCONTEXT * pMP4MuxContext);

#if defined(__cplusplus)
}
#endif
#endif /* _MP4MUX_H_ */

