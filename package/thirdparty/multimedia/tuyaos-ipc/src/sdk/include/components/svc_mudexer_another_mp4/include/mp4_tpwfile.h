/*
 * TPWFile is designed to handle all file operation for multi-platform.
 * We use two sets of APIs to doing so. One with buffered operation, like fopen/fseek;
 * One with no buffered operation, like open/read/write/lseek.
 * For now, we support Linux, Unix, Android, iOS and Win32.
 * For read/write operation, maximun size is 2GB.
 * For seek operation, no limit for the size theoretically(64bits).
 * To support seek with offset larger than 2GB, add compiler option "-D_FILE_OFFSET_BITS=64".
 * Unfortunately, Android platform do not support this marco. So we have to use unbuffered file
 * operation to process file larger than 2GB.
 * For platform except WIN32, add compiler option "-D_FILE_OFFSET_BITS=64" to support fetching
 * TPWFileStat for file larger than 2GB.
 */

#ifndef _TPWFILE_H_
#define _TPWFILE_H_
#if defined(__cplusplus)
extern "C"
{
#endif

#include <stdio.h>
#include <sys/types.h>

#define TPWFILE_EC_OK           0
#define TPWFILE_EC_FAILURE      -1
#define TPWFILE_EC_EOF          -2

#define TPWFILE_MAX_FILENAME_LENGTH     255 /*在window系统中文件名最大长度260字节(io.h)，linux中是255，这里取最小值*/

#define TPWFILE_ATTRIB_NORMAL       0x00    /* Normal file*/
#define TPWFILE_ATTRIB_SUBDIR       0x10    /* Subdirectory */

    // general properties for file
    typedef struct TPWFileStat
    {
        unsigned iAttrib;
        char pcName[TPWFILE_MAX_FILENAME_LENGTH];
        long long llFileLength; // file length in bytes
        time_t CreateTime;
    } TPWFileStat;

    typedef struct TPWFile
    {
        int bBuffered;
        long long llOffset;
        union {
            FILE *fp;
            int fd;
        };
    } TPWFile;

    /**
     *  TPWFileOpen
     *  @abstract       Opens a file with a specific file path. If use buffered option, then APIs like
     fread/fwrite will be used; if not, APIs like read/write will be used.
     *
     *  @param[in]      pcFileName  absolute file path
     *  @param[in]      pcMode      use "r" for read only, use "w" for write only, "rw" is not supported right now
     *  @param[in]      bBuffered   use buffered file operation or not
     *  @param[out]     pFile       TPWFile instance
     *
     *  @return         TPWFILE_EC_OK       if successfully open a file at specific path
     *                  TPWFILE_EC_FAILURE  if failed to open
     */
    extern int TPWFileOpen(char *pcFileName, char *pcMode, int bBuffered, TPWFile *pFile);

    /**
     *  TPWFileClose
     *  @abstract       Closes a opend file.
     *
     *  @param[in]      pFile       TPWFile instance
     *
     *  @return         TPWFILE_EC_OK       if successfully close
     *                  TPWFILE_EC_FAILURE  if failed to close
     */
    extern int TPWFileClose(TPWFile *pFile);

    /**
     *  TPWFileSeek
     *  @abstract       Seeks to a specific position.
     *
     *  @param[in]      lOffset     offset from origin position
     *  @param[in]      iOrigin     SEEK_CUR for current position; SEEK_SET for start position, SEEK_END for end position
     *  @param[in]      pFile       TPWFile instance
     *
     *  @return         Current offset if success
     *                  TPWFILE_EC_FAILURE if failed, for Android platform with buffered option,
     *                  seek beyond 2GB is also a failure
     */
    extern long long TPWFileSeek(long long lOffset, int iOrigin, TPWFile *pFile);

    /**
     *  TPWFileRead
     *  @abstract       Reads number of bytes to buffer. 2GB is maximun size for reading once.
     *
     *  @param[in,out]  pBuffer     pointer to write the read bytes
     *  @param[in]      iNumBytes   size to read
     *  @param[in]      pFile       TPWFile instance
     *
     *  @return         Number of bytes read if success
     *                  TPWFILE_EC_EOF if read after encouter a EOF
     *                  TPWFILE_EC_FAILURE for other failure
     */
    extern int TPWFileRead(void *pBuffer, int iNumBytes, TPWFile *pFile);

    /**
     *  TPWFileWrite
     *  @abstract       Writes the content of the buffer to file. 2GB is maximu
     *
     *  @param[in]      pBuffer     pointer of content for writing to file
     *  @param[in]      iNumBytes   size to write
     *  @param[in]      pFile       TPWFile instance
     *
     *  @return         Number of bytes write if success
     *                  TPWFILE_EC_FAILURE is failure
     */
    extern int TPWFileWrite(void *pBuffer, int iNumBytes, TPWFile *pFile);

    /**
     *  TPWFileGetStat
     *  @abstract       Get file stat
     *
     *  @param[out]     pFileStat   TPWFileStat to write the result
     *  @param[in]      pFile       TPWFile instance
     *
     *  @return         TPWFILE_EC_OK if success
     *                  TPWFILE_EC_FAILURE if failure
     */
    extern int TPWFileGetStat(TPWFileStat *pFileStat, TPWFile *pFile);

    /**
    *  TPWFileGetStatByPath
    *  @abstract       Get file stat
    *
    *  @param[out]     pFileStat   TPWFileStat to write the result
    *  @param[in]      pcFilePath  File path
    *
    *  @return         TPWFILE_EC_OK if success
    *                  TPWFILE_EC_FAILURE if failure
    */
    extern int TPWFileGetStatByPath(TPWFileStat *pFileStat, const char *pcFilePath);


    /***
    * TPWDirectoryCreate
    *  创建多级目录
    *  @param[in]      pcDir  Directory path， such as "C:\\Output\\src\\db"
    *  @return         TPWFILE_EC_OK if success
    *                  TPWFILE_EC_FAILURE if failure
    */
    extern int TPWDirectoryCreate(const char *pcDir);

    /**
     *  TPWGetFileNameFromPath
     *  @abstract       get file name from file path
     *
     *  @param[in]      pcPath       path
     *  @param[out]     pcResult     name from path
     *
     *  @return         TPWFILE_EC_OK       get name succeed
     *                  TPWFILE_EC_FAILURE  wrong path or NULL paramater
     */
    extern int TPWFileGetFileNameFromPath(const char *pcPath, char *pcResult);

#if defined(__cplusplus)
}
#endif
#endif
