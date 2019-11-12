/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/GlobalHandleContainer.h>
#include <Bentley/Desktop/FileSystem.h> // *** NEEDS WORK: Why are we using desktop-only functions?
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

static int s_parasolidInitialized = 0;
static bool s_usingExternalFrustrum = false;

struct FrustrumOutput
{
IParasolidWireOutput*   m_wireOutput;
IParasolidHLineOutput*  m_hlineOutput;
};

static FrustrumOutput s_frustrumOutput;

#define PARTITION_ROLLBACK_REQUIRED     // Using parasolid from multiple threads (as in tilepublishing) requires partitioned rollback for error handling.
#if defined (PARTITION_ROLLBACK_REQUIRED)

#define PPI_DELTA_DATA_BLOCK_SIZE 10240

#define DELTA_IS_CLOSED             0
#define DELTA_IS_OPEN               1

#define DELTA_IS_NOT_FOR_READ_OR_WRITE  0
#define DELTA_IS_FOR_READ               1
#define DELTA_IS_FOR_WRITE              2

#define IS_DELTA_CLOSED(pDelta)         (DELTA_IS_CLOSED == pDelta->openFlag && DELTA_IS_NOT_FOR_READ_OR_WRITE == pDelta->readwriteFlag)
#define IS_DELTA_OPEN(pDelta)           (DELTA_IS_OPEN == pDelta->openFlag && DELTA_IS_NOT_FOR_READ_OR_WRITE != pDelta->readwriteFlag)
#define IS_DELTA_OPEN_FOR_READ(pDelta)  (DELTA_IS_OPEN == pDelta->openFlag && DELTA_IS_FOR_READ == pDelta->readwriteFlag)
#define IS_DELTA_OPEN_FOR_WRITE(pDelta) (DELTA_IS_OPEN == pDelta->openFlag && DELTA_IS_FOR_WRITE == pDelta->readwriteFlag)

struct ppiDeltaStruct
    {
    PK_PMARK_t                  partitionMarkTag;       // partition mark to which this delta corresponds.
    PK_DELTA_t                  deltaTag;               // handle that identifies this delta to parasolid.
    int                         openFlag;               // Is this delta opened ? 1 if open, 0 if close
    int                         readwriteFlag;          // Is this delta opened for read? 1 if for read, 2 if for write
    long                        startFileOffset;        // offset in delta file where this data starts
    long                        endFileOffset;          // offset in delta file where this delta ends
    unsigned int                numByte;                // total number of bytes of stored for this delta
    char                        *pDataBuffer;           // the actual data buffer
    unsigned int                numDataBufferByte;      // number of useful bytes in data buffer
    unsigned int                numByteToReadFromFile;  // number of bytes left to read from file
    unsigned int                nextReadByte;           // next read index from data buffer
    long                        nextReadOffset;         // next read offset in delta file to get the next data buffer
#ifdef DEBUG_PARTITION_FRUSTRUM
    FILE                        *pDebugFile;            // File for debugging purposes
#endif
    struct ppiDeltaStruct       *pPrev;                 // pointer to previous delta
    };

typedef struct ppiDeltaStruct           PpiDelta;

struct ppiDataBufferStruct
    {
    int                          inUseFlag;          // Is this buffer being used?
    char                         *pDataBuffer;       // the actual data buffer
    struct ppiDataBufferStruct   *pPrev;             // pointer to next data buffer
    };

typedef struct ppiDataBufferStruct      PpiDataBuffer;

FILE                    *s_pDeltaFile = 0;              // FILE pointer to Delta File.
long                    s_endDeltaFileOffset = 0;       // Offset to begin write from

static int              s_deltaSystemActiveFlag = 0;    // Is the mark system activated?

static PpiDelta         *s_pDeltaList = 0;              // list of deltas
static PpiDataBuffer    *s_pDataBufferList = 0;         // list of data buffers

static BeFileNameCP     s_assetDir = nullptr;
static BeFileNameCP     s_tempDirBaseName = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidKernelManager::Initialize(BeFileNameCR assetDirW, BeFileNameCR tempDirBaseNameW) {
    s_assetDir = new BeFileName(assetDirW.c_str());
    s_tempDirBaseName = new BeFileName(tempDirBaseNameW.c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_DELTA_t allocateDeltaHandle (PpiDelta* deltaPtr)
    {
    return GlobalHandleContainer::AllocateHandle (deltaPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static PpiDelta* getPpiDeltaFromHandle (PK_DELTA_t deltaTag)
    {
    PpiDelta* deltaPtr = (PpiDelta*) GlobalHandleContainer::GetPointer (deltaTag);

    if (!deltaPtr || deltaPtr->deltaTag != deltaTag)
        {
        BeAssert (false);

        return NULL;
        }

    return deltaPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void releaseDeltaHandle (PK_DELTA_t deltaTag)
    {
    return GlobalHandleContainer::ReleaseHandle (deltaTag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static FILE*    openTmpFile
(
WCharCP         fileNameInP    // => path/prefix to use or NULL
)
    {
    WString     prefix;
    BeFileName  path;

    if (!WString::IsNullOrEmpty(fileNameInP))
        {
        WString dev, dir;
        BeFileName::ParseName(&dev, &dir, &prefix, NULL, fileNameInP);

        path.BuildName(dev.c_str(), dir.c_str(), NULL, NULL);
        }
    else
        {
//        path = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
        BeAssert (nullptr != s_tempDirBaseName && "PSolidKernelManager::Initialize wasn't called with GetLocalTempDirectoryBaseName.");
        if (nullptr != s_tempDirBaseName)
            path = *s_tempDirBaseName;
        prefix = L"fmp";
        }

    BeFileName tempFileName;

    if (BeFileNameStatus::Success != Desktop::FileSystem::BeGetTempFileName(tempFileName, path, prefix.c_str())) // <- NEEDSWORK: This method is currently only implemented for desktop systems...
        return nullptr;

#if defined(BENTLEYCONFIG_OS_WINDOWS)
    return _wfsopen (tempFileName.GetName(), L"w+bD", _SH_DENYNO);
#elif defined(BENTLEYCONFIG_OS_UNIX)
    // Is this good enough?
    // I can't find a better alternative to the "D" mode on Unix.
    return tmpfile();
#else
    #error Unsupported OS.
#endif
    }

/*---------------------------------------------------------------------------------**//**
* This function is called by the application to initialise the partitioned rollback system. It is not called from the Parasolid kernel.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      partitionRollbackSystemInit
(
WCharCP         pFileNameIn,            // => input name of file in which to store delta data
int             startFlagIn             // => input 1 = start, 0 = end
)
    {
    if (startFlagIn)
        {
        if (NULL == (s_pDeltaFile = openTmpFile (pFileNameIn)))
            return PK_ERROR_frustrum_failure;

        s_endDeltaFileOffset = ftell (s_pDeltaFile);

        s_deltaSystemActiveFlag = 1;    // Partitioned rollback system is activated
        }
    else if (s_deltaSystemActiveFlag)
        {
        PpiDelta        *pDelta = s_pDeltaList;

        while (pDelta)
            {
            PpiDelta *pPrevDelta = pDelta->pPrev;

            releaseDeltaHandle (pDelta->deltaTag);
            free (pDelta);

            pDelta = pPrevDelta;
            }

        fclose (s_pDeltaFile);

        PpiDataBuffer   *pDataBufferStruct = s_pDataBufferList;

        while (pDataBufferStruct)
            {
            PpiDataBuffer *pPrevDataBufferStruct = pDataBufferStruct->pPrev;

            if (pDataBufferStruct->pDataBuffer)
                free (pDataBufferStruct->pDataBuffer);

            free (pDataBufferStruct);

            pDataBufferStruct = pPrevDataBufferStruct;
            }

        s_pDeltaFile            = NULL;
        s_pDeltaList            = NULL;
        s_pDataBufferList       = NULL;

        s_endDeltaFileOffset    = 0;
        s_deltaSystemActiveFlag = 0;    // partitioned rollback system is deactivated
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Read the next delta buffer data from the delta file
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      readDataBufferFromFile
(
PpiDelta*       pDeltaIn      // => input delta whose buffer is to be read into from file
)
    {
    unsigned int numBufferByteToRead = MIN(PPI_DELTA_DATA_BLOCK_SIZE, pDeltaIn->numByteToReadFromFile);
    size_t numBytesRead = 0;

    if (numBufferByteToRead > 0)
        {
        if (0 != fseek(s_pDeltaFile, pDeltaIn->nextReadOffset, SEEK_SET))
            {
            PK_SESSION_comment("readDataBufferFromFile returns PK_ERROR_frustrum_failure - seek");
            return PK_ERROR_frustrum_failure;
            }

        if (numBufferByteToRead != (numBytesRead = fread(pDeltaIn->pDataBuffer, sizeof(char), numBufferByteToRead, s_pDeltaFile)))
            {
            PK_SESSION_comment("readDataBufferFromFile returns PK_ERROR_frustrum_failure - read");
            return PK_ERROR_frustrum_failure;
            }

#ifdef DEBUG_PARTITION_FRUSTRUM
        if (numBufferByteToRead != fwrite(pDeltaIn->pDataBuffer, sizeof(char), numBufferByteToRead, pDeltaIn->pDebugFile))
            PK_SESSION_comment("pki_partitionFrustrumDeltaRead: Error in writing to debug File");
#endif
        pDeltaIn->nextReadOffset = ftell(s_pDeltaFile);
        }
    else
        {
        pDeltaIn->nextReadOffset = 0;
        }

    pDeltaIn->numDataBufferByte = numBufferByteToRead;
    pDeltaIn->numByteToReadFromFile -= numBufferByteToRead;
    pDeltaIn->nextReadByte = 0;

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Write input delta buffer data to the delta file
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      writeDataBufferToFile
(
PpiDelta*       pDeltaIn      // => input delta whose buffer is to be written to file
)
    {
    if (pDeltaIn->numDataBufferByte > 0)
        {
        if (0 != fseek(s_pDeltaFile, pDeltaIn->endFileOffset, SEEK_SET))
            {
            PK_SESSION_comment("writeDataBufferFromFile returns PK_ERROR_frustrum_failure - seek");
            return PK_ERROR_frustrum_failure;
            }

        unsigned int numWriteByte = pDeltaIn->numDataBufferByte;
        size_t numBytesWritten = 0;

        if (numWriteByte != (numBytesWritten = fwrite(pDeltaIn->pDataBuffer, sizeof(char), numWriteByte, s_pDeltaFile)))
            {
            PK_SESSION_comment("writeDataBufferFromFile returns PK_ERROR_frustrum_failure - write");
            return PK_ERROR_frustrum_failure;
            }

#ifdef DEBUG_PARTITION_FRUSTRUM

        printf("Wrote %d bytes in Delta %d at position %d\n", numWriteByte, pDeltaIn->partitionMarkTag, pDeltaIn->endFileOffset);

        if (numWriteByte != fwrite(pDeltaIn->pDataBuffer, sizeof(char), numWriteByte, pDeltaIn->pDebugFile))
            PK_SESSION_comment("pki_partitionFrustrumDeltaWrite: Error in writing to debug File");
#endif

        pDeltaIn->endFileOffset = s_endDeltaFileOffset = ftell(s_pDeltaFile);

#ifdef DEBUG_PARTITION_FRUSTRUM

        if (((pDeltaIn->endFileOffset - pDeltaIn->startFileOffset) % PPI_DELTA_DATA_BLOCK_SIZE) !=
                                                    (long) (numBytesWritten % PPI_DELTA_DATA_BLOCK_SIZE))
            {
            printf("Write mismatch in Delta %d: actualBytes %d, FileBytes %d\n", pDeltaIn->partitionMarkTag, pDeltaIn->endFileOffset - pDeltaIn->startFileOffset);
            }
#endif
        pDeltaIn->numDataBufferByte = 0;
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Return one data buffer of size PPI_DELTA_DATA_BLOCK_SIZE. Will initially look for a free data buffer in s_pDataBufferList. If all the data
* buffers in s_pDataBufferList are in use, then a fresh data buffer will be allocated & returned.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static char*    getDataBuffer ()
    {
    // Look for an unused data buffer in s_pDataBufferList
    PpiDataBuffer *pDataBufferStruct = NULL;

    for (pDataBufferStruct = s_pDataBufferList;
                NULL != pDataBufferStruct && pDataBufferStruct->inUseFlag;
                pDataBufferStruct = pDataBufferStruct->pPrev)
        {
        }

    // If none found, then allocate a fresh new PpiDataBuffer & add
    // into s_pDataBufferList
    if (NULL == pDataBufferStruct)
        {
        pDataBufferStruct = (PpiDataBuffer *) malloc(sizeof(PpiDataBuffer));

        if (NULL == pDataBufferStruct)
            {
            PK_SESSION_comment ("getDataBuffer returns PK_ERROR_memory_full");

            return NULL;
            }

        pDataBufferStruct->pDataBuffer = (char *) malloc(PPI_DELTA_DATA_BLOCK_SIZE);

        if (NULL == pDataBufferStruct->pDataBuffer)
            {
            PK_SESSION_comment ("getDataBuffer returns PK_ERROR_memory_full");

            return NULL;
            }

        pDataBufferStruct->pPrev = s_pDataBufferList;
        s_pDataBufferList = pDataBufferStruct;
        }

    pDataBufferStruct->inUseFlag = 1;

    return pDataBufferStruct->pDataBuffer;
    }

/*---------------------------------------------------------------------------------**//**
* Release data buffer - back for reuse
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      releaseDataBuffer
(
char*           pDataBufferIn          // => input data buffer to release
)
    {
    PpiDataBuffer *pDataBufferStruct = s_pDataBufferList;

    for (pDataBufferStruct = s_pDataBufferList;
                NULL != pDataBufferStruct && pDataBufferStruct->pDataBuffer != pDataBufferIn;
                pDataBufferStruct = pDataBufferStruct->pPrev)
        {
        }

    if (NULL == pDataBufferStruct)
        {
        PK_SESSION_comment ("releaseDataBuffer: Cannot find data buffer struct to release");

        return PK_ERROR_frustrum_failure;
        }

    pDataBufferStruct->inUseFlag = 0;

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Opens a new delta associated with a partition mark for writing.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaOpenForWrite
(
PK_PMARK_t      markIn,         // => input partition mark associated with delta
PK_DELTA_t*     pDeltaOut       // <= ouput delta-id associated with this mark.
)
    {
    // Create a new mark structure.
    PpiDelta *pNewDelta = (PpiDelta *) malloc(sizeof(PpiDelta));

    if (NULL == pNewDelta)
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaOpenForWrite returns PK_ERROR_memory_full");

        return PK_ERROR_memory_full;
        }

    pNewDelta->pDataBuffer = getDataBuffer();

    pNewDelta->partitionMarkTag = markIn;
    pNewDelta->openFlag = DELTA_IS_OPEN;
    pNewDelta->readwriteFlag = DELTA_IS_FOR_WRITE;
    pNewDelta->startFileOffset = s_endDeltaFileOffset;
    pNewDelta->endFileOffset = s_endDeltaFileOffset;
    pNewDelta->numByte = 0;

    pNewDelta->numDataBufferByte = 0;
    pNewDelta->nextReadByte = 0;
    pNewDelta->nextReadOffset = 0;

    pNewDelta->pPrev = s_pDeltaList;

    s_pDeltaList = pNewDelta;

    // Return output delta-id as handle to memory pointer of pNewDelta.
    pNewDelta->deltaTag = allocateDeltaHandle (pNewDelta);

    *pDeltaOut = pNewDelta->deltaTag;

#ifdef DEBUG_PARTITION_FRUSTRUM
    char    debugFileName[50];
    sprintf(debugFileName, "c:\\tmp\\d%dw.dta", *pDeltaOut);
    pNewDelta->pDebugFile = fopen(debugFileName, "wb");
#endif

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Opens a delta for reading. The delta should already exist.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaOpenForRead
(
PK_DELTA_t      deltaIn     // => input delta to open
)
    {
    PpiDelta*   pDelta = getPpiDeltaFromHandle (deltaIn);

    if (!pDelta)
        return PK_ERROR_frustrum_failure;

    pDelta->pDataBuffer = getDataBuffer();

    if (IS_DELTA_OPEN(pDelta))
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaOpenForRead returns PK_ERROR_bad_key");

        return PK_ERROR_bad_key;
        }

    pDelta->openFlag = DELTA_IS_OPEN;
    pDelta->readwriteFlag = DELTA_IS_FOR_READ;

    pDelta->nextReadOffset = pDelta->startFileOffset;
    pDelta->numByteToReadFromFile = pDelta->numByte;

#ifdef DEBUG_PARTITION_FRUSTRUM
    char    debugFileName[50];
    sprintf(debugFileName, "c:\\tmp\\d%dr.dta", deltaIn);
    pDelta->pDebugFile = fopen(debugFileName, "wb");
#endif

    return readDataBufferFromFile(pDelta);
    }

/*---------------------------------------------------------------------------------**//**
* Closes a delta file.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaClose
(
PK_DELTA_t      deltaIn
)
    {
    int         status = PK_ERROR_ok;
    PpiDelta*   pDelta = getPpiDeltaFromHandle (deltaIn);

    if (!pDelta)
        return PK_ERROR_frustrum_failure;

    // Check if delta is opened.
    if (IS_DELTA_CLOSED(pDelta))
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaClose returns PK_ERROR_frustrum_failure - already closed");

        return PK_ERROR_frustrum_failure;
        }

    if (IS_DELTA_OPEN_FOR_WRITE(pDelta))
        {
        if (PK_ERROR_ok != (status = writeDataBufferToFile(pDelta)))
            return status;

#ifdef DEBUG_PARTITION_FRUSTRUM
        fclose(pDelta->pDebugFile);
#endif
        }
    else if (IS_DELTA_OPEN_FOR_READ(pDelta))
        {
#ifdef DEBUG_PARTITION_FRUSTRUM
        fclose(pDelta->pDebugFile);
#endif
        }
    else
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaClose returns PK_ERROR_frustrum_failure - unknown open mode");

        return PK_ERROR_frustrum_failure;
        }

    pDelta->openFlag = DELTA_IS_CLOSED;
    pDelta->readwriteFlag = DELTA_IS_NOT_FOR_READ_OR_WRITE;

    releaseDataBuffer(pDelta->pDataBuffer);
    pDelta->pDataBuffer = 0;

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Writes the specified number of bytes to a (already open) delta file.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaWrite
(
PK_DELTA_t      deltaIn,                // => input delta to write into
unsigned int    numByteIn,              // => input number of bytes to write
const char*     pByteArrayIn            // => input data to write
)
    {
    int         status = PK_ERROR_ok;
    PpiDelta*   pDelta = getPpiDeltaFromHandle (deltaIn);

    if (!pDelta)
        return PK_ERROR_frustrum_failure;

    if (!IS_DELTA_OPEN_FOR_WRITE(pDelta))
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaWrite returns PK_ERROR_frustrum_failure - not open for write");

        return PK_ERROR_frustrum_failure;
        }

    if (numByteIn > 0)
        {
        char *pDataBuffer = pDelta->pDataBuffer;

        unsigned int numByteAvailable = PPI_DELTA_DATA_BLOCK_SIZE - pDelta->numDataBufferByte;

        if (numByteIn <= numByteAvailable)
            {
            memcpy(pDataBuffer + pDelta->numDataBufferByte, pByteArrayIn, numByteIn);
            pDelta->numDataBufferByte += numByteIn;
            }
        else
            {
            if (numByteAvailable > 0)
                memcpy(pDataBuffer + pDelta->numDataBufferByte, pByteArrayIn, numByteAvailable);

            pDelta->numDataBufferByte = PPI_DELTA_DATA_BLOCK_SIZE;

            if (PK_ERROR_ok != (status = writeDataBufferToFile(pDelta)))
                return status;

            int numByteToWrite = numByteIn - numByteAvailable;
            const char *pInputByteArray = pByteArrayIn + numByteAvailable;

            while (numByteToWrite > 0)
                {
                if (numByteToWrite <= PPI_DELTA_DATA_BLOCK_SIZE)
                    {
                    memcpy(pDataBuffer, pInputByteArray, numByteToWrite);

                    pDelta->numDataBufferByte = numByteToWrite;
                    numByteToWrite = 0;
                    }
                else
                    {
                    memcpy(pDataBuffer, pInputByteArray, PPI_DELTA_DATA_BLOCK_SIZE);

                    pDelta->numDataBufferByte = PPI_DELTA_DATA_BLOCK_SIZE;
                    pInputByteArray += PPI_DELTA_DATA_BLOCK_SIZE;
                    numByteToWrite -= PPI_DELTA_DATA_BLOCK_SIZE;

                    if (PK_ERROR_ok != (status = writeDataBufferToFile(pDelta)))
                        return status;
                    }
                }
            }

        pDelta->numByte += numByteIn;
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Reads the specified number of bytes from an open delta file. `pByteArrayOut' may be NULL, in which case a skip is performed.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaRead
(
PK_DELTA_t      deltaIn,        // => input mark to read from
unsigned int    numByteIn,      // => input num bytes to read
char*           pByteArrayOut   // <= output data read
)
    {
    int         status = PK_ERROR_ok;
    PpiDelta*   pDelta = getPpiDeltaFromHandle (deltaIn);

    if (!pDelta)
        return PK_ERROR_frustrum_failure;

    if (!IS_DELTA_OPEN_FOR_READ(pDelta))
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaRead returns PK_ERROR_frustrum_failure - not open for read");

        return PK_ERROR_frustrum_failure;
        }

    if (numByteIn > 0)
        {
        char            *pDataBuffer = pDelta->pDataBuffer;
        unsigned int    numByteAvailable = pDelta->numDataBufferByte - pDelta->nextReadByte;

        if (numByteIn <= numByteAvailable)
            {
            if (pByteArrayOut)
                memcpy(pByteArrayOut, pDataBuffer + pDelta->nextReadByte, numByteIn);

            pDelta->nextReadByte += numByteIn;
            }
        else
            {
            if (numByteAvailable > 0 && pByteArrayOut)
                memcpy(pByteArrayOut, pDataBuffer + pDelta->nextReadByte, numByteAvailable);

            if (PK_ERROR_ok != (status = readDataBufferFromFile(pDelta)))
                return status;

            unsigned int numByteToRead = numByteIn - numByteAvailable;
            const char *pOutputByteArray = pByteArrayOut ? pByteArrayOut + numByteAvailable : 0;

            while (numByteToRead > 0)
                {
                if (numByteToRead <= pDelta->numDataBufferByte)
                    {
                    if (pOutputByteArray)
                        memcpy((void *) pOutputByteArray, pDataBuffer, numByteToRead);

                    pDelta->nextReadByte = numByteToRead;
                    numByteToRead = 0;
                    }
                else
                    {
                    unsigned int numByteToCopy = pDelta->numDataBufferByte;

                    if (pOutputByteArray)
                        {
                        memcpy((void *) pOutputByteArray, pDataBuffer, numByteToCopy);
                        pOutputByteArray += numByteToCopy;
                        }
                    numByteToRead -= numByteToCopy;

                    if (PK_ERROR_ok != (status = readDataBufferFromFile(pDelta)))
                        return status;
                    }
                }
            }
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes the specified mark file.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_partitionFrustrumDeltaDelete
(
PK_DELTA_t      deltaIn      // => input delta to delete
)
    {
    PpiDelta*   pDeltaToDelete = getPpiDeltaFromHandle (deltaIn);

    if (!pDeltaToDelete)
        return PK_ERROR_frustrum_failure;

    PpiDelta *pDelta = s_pDeltaList;
    PpiDelta *pNextDelta = NULL;

    while (pDelta && pDelta != pDeltaToDelete)
        {
        pNextDelta = pDelta;
        pDelta = pDelta->pPrev;
        }

    if (!pDelta)
        {
        PK_SESSION_comment ("pki_partitionFrustrumDeltaDelete returns PK_ERROR_frustrum_failure - not found");

        releaseDeltaHandle (deltaIn);
        BeAssert (false);

        return PK_ERROR_frustrum_failure;
        }

    PpiDelta *pPrevDelta = pDelta->pPrev;

    if (pNextDelta)
        {
        BeAssert (pDelta != s_pDeltaList);

        pNextDelta->pPrev = pPrevDelta;
        }
    else
        {
        BeAssert (pDelta == s_pDeltaList);

        s_pDeltaList = pPrevDelta;
        }

    releaseDeltaHandle (deltaIn);
    free (pDelta);

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Starts partitioned rollback.
* @bsimethod                                                    Deepak.Malkan   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_partitionRollbackStart
(
WCharCP         pDeltaFilenameIn               // => input name of file in which to store delta data
)
    {
    int errorCode = 0;

    errorCode = partitionRollbackSystemInit (pDeltaFilenameIn, 1);      // intialize partition rollback frustrum data

    if (errorCode)
        return errorCode;

    // Register the Partitioned Rollback frustrum
    PK_DELTA_frustrum_t partitionFrustrum;

    partitionFrustrum.open_for_write_fn = pki_partitionFrustrumDeltaOpenForWrite;
    partitionFrustrum.open_for_read_fn  = pki_partitionFrustrumDeltaOpenForRead;
    partitionFrustrum.close_fn          = pki_partitionFrustrumDeltaClose;
    partitionFrustrum.write_fn          = pki_partitionFrustrumDeltaWrite;
    partitionFrustrum.read_fn           = pki_partitionFrustrumDeltaRead;
    partitionFrustrum.delete_fn         = pki_partitionFrustrumDeltaDelete;

    errorCode = PK_DELTA_register_callbacks (partitionFrustrum);

    return errorCode;
    }

/*---------------------------------------------------------------------------------**//**
* Stops session specific rollback.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_partitionRollbackStop ()
    {
    int errorCode = 0;

    partitionRollbackSystemInit (L"", 0);        // re-set partition rollback frustrum data

    return errorCode;
    }
#endif

#define PKI_MARK_DATA_BLOCK_SIZE 1024

struct pkiMarkDataStruct
    {
    int                         numBytesUsed;
    struct pkiMarkDataStruct    *pNext;                 /* Next data block. */
    char                        data[PKI_MARK_DATA_BLOCK_SIZE];
    };

typedef struct pkiMarkDataStruct        PkiMarkData;

typedef struct pkiMarkStruct
    {
    PK_MARK_t                           markNumber;
    PkiMarkData                         *pFirstMarkData;
    PkiMarkData                         *pLastMarkData;
    struct pkiMarkStruct                *pNext;
    } PkiMark;

static int          s_markSystemActiveFlag = 0;     /* Is the mark system activated? */

static PkiMark      *s_pMarkList = NULL;            /* list of marks & their stored data */
static PkiMark      *s_pCurrentMark;                /* current mark in use */
static PkiMarkData  *s_pCurrentMarkData;            /* current mark data block in use */
static int          s_markDataReadOffset;           /* read offset in current mark data block */

/*---------------------------------------------------------------------------------**//**
* Deletes a data block chain.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void    delete_mark_data_block
(
PkiMarkData     *pDataBlockIn
)
    {
    PkiMarkData *pDataBlock = pDataBlockIn;

    while (pDataBlock)
        {
        PkiMarkData *pNextDataBlock = pDataBlock->pNext;

        free(pDataBlock);

        pDataBlock = pNextDataBlock;
        }
    }

/*---------------------------------------------------------------------------------**//**
* This function is called by the application to initialise the rollback system. It is not called from the Parasolid kernel.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mark_system_init
(
int             startIn /* => input 1 = start, 0 = end */
)
    {
    if (startIn)
        {       /* Start */
        s_pCurrentMark = 0;             /* No mark is open as yet */
        s_markSystemActiveFlag = 1;     /* Mark system is activated  */
        }
    else
        {       /* End */
        PkiMark *pMark = s_pMarkList;

        while (pMark)
            {
            PkiMark *pNextMark = pMark->pNext;

            delete_mark_data_block(pMark->pFirstMarkData);
            free(pMark);

            pMark = pNextMark;
            }

        s_markSystemActiveFlag = 0;     /* Mark system is deactivated */
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* Opens a mark for reading or writing. The mark should or should not already exist, respectively.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t  pki_frustrum_mark_open
(
PK_MARK_t       markIn, /* => input mark to open */
PK_LOGICAL_t    writeIn /* => input whether in read-mode or write-mode ? */
)
    {
    /* Check if any mark is open. If so, then return error. */
    if (s_pCurrentMark)
        return PK_ERROR_frustrum_failure;

    if (PK_LOGICAL_true == writeIn)
        {
        /* Create a new mark structure. */
        PkiMark *pNewMark = (PkiMark *) malloc(sizeof(PkiMark));

        pNewMark->markNumber     = markIn;
        pNewMark->pFirstMarkData = NULL;
        pNewMark->pLastMarkData  = NULL;
        pNewMark->pNext          = s_pMarkList;

        s_pMarkList        = pNewMark;
        s_pCurrentMark     = pNewMark;
        s_pCurrentMarkData = s_pCurrentMark->pLastMarkData;
        }
    else        /* Mark is opened for read purposes. It should already be existing. */
        {
        PkiMark *pMark = s_pMarkList;

        while (pMark && pMark->markNumber != markIn)
            pMark = pMark->pNext;

        /* Mark is not existing. Return Error. */
        if (!pMark)
            return PK_ERROR_frustrum_failure;

        s_pCurrentMark       = pMark;
        s_markDataReadOffset = 0;
        s_pCurrentMarkData   = s_pCurrentMark->pFirstMarkData;
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Closes a mark file. Is called after an open, before another open, as a courtesy.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_frustrum_mark_close
(
PK_MARK_t       markIn
)
    {
    /* Check if some mark is opened. If no mark is open, then return error */
    if (!s_pCurrentMark)
        return PK_ERROR_frustrum_failure;

    s_pCurrentMark     = NULL;  /* Set nil current open mark. */
    s_pCurrentMarkData = NULL;  /* Set nil current mark data block */

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Writes the specified number of bytes to the (already open) mark file.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_frustrum_mark_write
(
PK_MARK_t       markIn,         /* => input mark to write into */
int             numBytesIn,     /* => input number of bytes to write */
const char      *pBytesIn       /* => input data to write */
)
    {
    int         offset, numWriteBytes = 0;

    /* MarkIn should be same as the current opened mark. If not then return error. */
    if (NULL == s_pCurrentMark || markIn != s_pCurrentMark->markNumber)
        return PK_ERROR_frustrum_failure;

    /* If current opened mark has no data pointer, then allocate one. */
    if (NULL == s_pCurrentMarkData)
        {
        s_pCurrentMarkData = (PkiMarkData *) malloc(sizeof(PkiMarkData));

        if (NULL == s_pCurrentMarkData)
            return PK_ERROR_frustrum_failure;

        s_pCurrentMarkData->numBytesUsed = 0;
        s_pCurrentMarkData->pNext = NULL;

        s_pCurrentMark->pFirstMarkData = s_pCurrentMarkData;
        s_pCurrentMark->pLastMarkData = s_pCurrentMarkData;
        }

    for (offset = 0; offset < numBytesIn; offset += numWriteBytes)
        {
        /* If current mark data block is filled up, then allocate a new one. */
        if (PKI_MARK_DATA_BLOCK_SIZE == s_pCurrentMarkData->numBytesUsed)
            {
            s_pCurrentMarkData->pNext = (PkiMarkData *) malloc(sizeof(PkiMarkData));

            if (NULL == s_pCurrentMarkData->pNext)
                return PK_ERROR_frustrum_failure;

            s_pCurrentMarkData = s_pCurrentMarkData->pNext;
            s_pCurrentMarkData->numBytesUsed = 0;
            s_pCurrentMarkData->pNext = NULL;

            s_pCurrentMark->pLastMarkData = s_pCurrentMarkData;
            }

        /* Calculate number of bytes to copy */
        numWriteBytes = MIN(PKI_MARK_DATA_BLOCK_SIZE - s_pCurrentMarkData->numBytesUsed, numBytesIn - offset);

        memcpy (s_pCurrentMarkData->data + s_pCurrentMarkData->numBytesUsed, pBytesIn + offset, numWriteBytes);

        s_pCurrentMarkData->numBytesUsed += numWriteBytes;
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Reads the specified number of bytes from an open mark file. `pBytesIn' may be NULL, in which case a skip is performed.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_frustrum_mark_read
(
PK_MARK_t       markIn, /* => input mark to read from */
int             numBytesIn,     /* => input num bytes to read */
char            *pBytesIn       /* <= output data read */
)
    {
    int         offset, numReadBytes = 0;

    /* markIn should be same as the current opened mark. If not then return error. */
    if (NULL == s_pCurrentMark || markIn != s_pCurrentMark->markNumber)
        return PK_ERROR_frustrum_failure;

    /* Read numBytes in from s_markDataReadOffset position in s_pCurrentMarkData. */
    for (offset = 0; offset < numBytesIn; offset += numReadBytes)
        {
        if (s_markDataReadOffset == PKI_MARK_DATA_BLOCK_SIZE)
            {
            s_pCurrentMarkData = s_pCurrentMarkData->pNext;
            s_markDataReadOffset = 0;
            }

        if (NULL == s_pCurrentMarkData)
            return PK_ERROR_frustrum_failure;

        numReadBytes = MIN(PKI_MARK_DATA_BLOCK_SIZE - s_markDataReadOffset, numBytesIn - offset);
        if (pBytesIn)
            memcpy(pBytesIn + offset, s_pCurrentMarkData->data + s_markDataReadOffset, numReadBytes);

        s_markDataReadOffset += numReadBytes;
        }

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if the given mark is available.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_frustrum_mark_check
(
PK_MARK_t       markIn, /* => input mark to check for */
PK_LOGICAL_t    *pOkOut /* <= output is it ok or not ? */
)
    {
    PkiMark     *pMark = s_pMarkList;

    *pOkOut = PK_LOGICAL_true;

    while (pMark && pMark->markNumber != markIn)
        pMark = pMark->pNext;

    /* Mark is not existing. Return false. */
    if (!pMark)
        *pOkOut = PK_LOGICAL_false;

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes the specified mark file.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ERROR_code_t pki_frustrum_mark_delete
(
PK_MARK_t       markIn  /* => input mark to delete */
)
    {
    PkiMark *pMark = s_pMarkList;
    PkiMark *pPrevMark = NULL;
    PkiMark *pNextMark = NULL;

    while (pMark && pMark->markNumber != markIn)
        {
        pPrevMark = pMark;
        pMark = pMark->pNext;
        }

    if (!pMark)
        return PK_ERROR_frustrum_failure;

    pNextMark = pMark->pNext;

    if (pPrevMark)
        pPrevMark->pNext = pNextMark;
    else
        s_pMarkList = pNextMark;

    if (pMark == s_pCurrentMark)
        {
        s_pCurrentMark = NULL;
        s_pCurrentMarkData = NULL;
        }

    delete_mark_data_block(pMark->pFirstMarkData);
    free(pMark);

    return PK_ERROR_ok;
    }

/*---------------------------------------------------------------------------------**//**
* Starts session specific rollback.
* @bsimethod                                                    Deepak.Malkan   12/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_mark_start
(
void
)
    {
    PK_MARK_frustrum_t  rollbackFrustrum;
    PK_MARK_start_o_t   rollbackOption;

    mark_system_init (1);       /* intialize rollback frustrum data */

    /* Setup PK rollback */
    rollbackFrustrum.open_fn    = pki_frustrum_mark_open;
    rollbackFrustrum.close_fn   = pki_frustrum_mark_close;
    rollbackFrustrum.write_fn   = pki_frustrum_mark_write;
    rollbackFrustrum.read_fn    = pki_frustrum_mark_read;
    rollbackFrustrum.check_fn   = pki_frustrum_mark_check;
    rollbackFrustrum.delete_fn  = pki_frustrum_mark_delete;

    rollbackOption.forward = PK_LOGICAL_false;  /* Switch off rollforward. */

    return PK_MARK_start (rollbackFrustrum, &rollbackOption);
    }

#define FR_NOT_STARTED          FR_unspecified
#define FR_INTERNAL_ERROR       FR_unspecified

#define FR_MAX_OPEN_FILES       32

#define FR_NULL_STRID           (-1)
#define FR_READ_ACCESS          1
#define FR_WRITE_ACCESS         2
#define FR_READ_WRITE_ACCESS    3

#define FR_MAX_NAMELEN          256 // DGNPLATFORM_RESOURCE_MAXFILELENGTH
#define FR_MAX_HEADER_LINE      (FR_MAX_NAMELEN + 32)

#define END_OF_STRING_C         '\0'
#define END_OF_STRING_S         "\0"
#define NEW_LINE_C              '\n'
#define NEW_LINE_S              "\n"

// One structure per open file containing info such as filename and the C stream id.
// The structures are chained together, accessed via the "s_pFrOpenFiles" variable.
typedef struct file_s *PFrustrumFile;

typedef struct file_s
    {
    PFrustrumFile   next;
    PFrustrumFile   prev;
    int             strid;
    int             guise;
    int             format;
    int             access;
    wchar_t         name[FR_MAX_NAMELEN + 1];
    wchar_t         key[FR_MAX_NAMELEN + 1];
    FILE*           stream;
    } FrustrumFile;

static int s_frStarted = 0;
static int s_frStreamIdArray[FR_MAX_OPEN_FILES];

static PFrustrumFile s_pFrOpenFiles = NULL;
static int s_frFileCount = 0;

// the following are for writing and checking file headers
static char s_frPreamble1[FR_MAX_HEADER_LINE] = END_OF_STRING_S;
static char s_frPreamble2[FR_MAX_HEADER_LINE] = END_OF_STRING_S;
static char s_frPrefix1[FR_MAX_HEADER_LINE] = "**PART1;\n";
static char s_frPrefix2[FR_MAX_HEADER_LINE] = "**PART2;\n";
static char s_frPrefix3[FR_MAX_HEADER_LINE] = "**PART3;\n";
static char s_frTrailerStart[FR_MAX_HEADER_LINE] = "**END_OF_HEADER";
static char s_frTrailer[FR_MAX_HEADER_LINE] = END_OF_STRING_S;
static char s_frUnknownValue[] = "unknown";

// this buffer used for input-output of file headers and text files
static char *s_pFrIOBuffer = NULL;
static int s_frIOBufferLength = 0;

/*---------------------------------------------------------------------------------**//**
* purpose returns a string declaring file format (text/binary)
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static char*    getFileFormatString (int format)
    {
    static char ffbnry[] = "binary";
    static char fftext[] = "text";

    switch (format)
        {
        case FFBNRY: return ffbnry;
        case FFTEXT: return fftext;
        }

    return s_frUnknownValue;
    }

/*---------------------------------------------------------------------------------**//**
* purpose returns a string declaring file guise (rollback,journal,..)
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static char*    getFileGuiseString (int guise)
    {
    static char ffcrol[] = "rollback";
    static char ffcsnp[] = "snapshot";
    static char ffcjnl[] = "journal";
    static char ffcxmt[] = "transmit";
    // unused - static char ffcxmo[] = "old_transmit";
    static char ffcsch[] = "schema";
    static char ffclnc[] = "licence";
    static char ffcdbg[] = "debug_report";

    switch (guise)
        {
        case FFCROL: return ffcrol;
        case FFCSNP: return ffcsnp;
        case FFCJNL: return ffcjnl;
        case FFCXMT: return ffcxmt;
        case FFCSCH: return ffcsch;
        case FFCLNC: return ffclnc;
        case FFCDBG: return ffcdbg;
        }

    return s_frUnknownValue;
    }

/*---------------------------------------------------------------------------------**//**
* purpose Initialize global standard headers written to files
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setupHeaderStrings ()
    {
    strcpy (s_frPreamble1, "**");                           // two asterisks
    strcat (s_frPreamble1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");   // upper case letters
    strcat (s_frPreamble1, "abcdefghijklmnopqrstuvwxyz");   // lower case letters
    strcat (s_frPreamble1, "**************************");   // twenty six asterisks
    strcat (s_frPreamble1, NEW_LINE_S);

    strcpy (s_frPreamble2, "**");                           // two asterisks
    strcat (s_frPreamble2, "PARASOLID");                    // PARASOLID (upper case)
    strcat (s_frPreamble2, " !");                           // space and exclamation
    strcat (s_frPreamble2, "\"");                           // a double quote char
    strcat (s_frPreamble2, "#$%&'()*+,-./:;<=>?@[");        // some special chars
    strcat (s_frPreamble2, "\\");                           // a backslash char
    strcat (s_frPreamble2, "]^_`{|}~");                     // more special chars
    strcat (s_frPreamble2, "0123456789");                   // digits
    strcat (s_frPreamble2, "**************************");   // twenty six asterisks
    strcat (s_frPreamble2, NEW_LINE_S);

    strcpy (s_frTrailer, s_frTrailerStart);
    strcat (s_frTrailer, "*****************************************************************");
    strcat (s_frTrailer, NEW_LINE_S);
    }

/*---------------------------------------------------------------------------------**//**
* purpose return a file extension string depending on guise & format
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static wchar_t const* getFileExtension (int guise, int format)
    {
    static wchar_t ffcsnp[] = L".snp";      // snapshot file
    static wchar_t ffcjnl[] = L".jnl";      // journal file
    static wchar_t ffcxmt[] = L".x_t";      // ascii parasolid transmit file
    static wchar_t ffcxmb[] = L".x_b";      // binary parasolid transmit file
    static wchar_t ffcsch[] = L".sch_txt";  // schema file
    static wchar_t ffclnc[] = L".lnc";      // licence file
    static wchar_t ffcxmo[] = L".xmt";      // romulus transmit file
    static wchar_t ffcxmp[] = L".xmp";      // partition transmit file
    static wchar_t ffcxmd[] = L".xmd";      // delta transmit file
    static wchar_t ffcdbg[] = L".xml";      // delta transmit file

    switch (guise)
        {
        case FFCSNP: return ffcsnp;
        case FFCJNL: return ffcjnl;
        case FFCXMT: return (FFTEXT == format ? ffcxmt : ffcxmb);
        case FFCSCH: return ffcsch;
        case FFCLNC: return ffclnc;
        case FFCXMO: return ffcxmo;
        case FFCXMP: return ffcxmp;
        case FFCXMD: return ffcxmd;
        case FFCDBG: return ffcdbg;
        }

    return L"\0";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getPSolidSchemasDir()
    {
//    BeFileName fullName = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    BeAssert (nullptr != s_assetDir && "PSolidKernelManager::Initialize wasn't called with GetDgnPlatformAssetsDirectory.");
    BeFileName fullName;
    if (nullptr != s_assetDir)
        fullName = *s_assetDir;
    fullName.AppendToPath (L"ParasolidSchema");
    return fullName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   05/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void     prependSchemaExtension (wchar_t* filename, int guise)
    {
    if (FFCSCH != guise)
        return;

    BeFileName  fullName = getPSolidSchemasDir();

    fullName.AppendToPath (filename);

    wcscpy (filename, fullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     appendExtension (wchar_t* filename, wchar_t const* extension)
    {
    WString     ext;
    BeFileName::ParseName (NULL, NULL, NULL, &ext, filename);

    if (ext.empty())
        wcscat (filename, extension);
    }

/*---------------------------------------------------------------------------------**//**
* purpose registers an opened file in FrustrumFile structure
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      registerOpenFile
(
FILE*           pFileIn,        // => input file pointer
int             guiseIn,        // => input class of file
int             formatIn,       // => input binary or text
int             accessIn,       // => input read/write access ?
wchar_t const*  pFilenameIn,    // => input filename
wchar_t const*  pKeynameIn,     // => input keyname
PFrustrumFile*  pFileOut        // <= output fr file structure
)
    {
    // allocate and add file structure into list of open files
    PFrustrumFile   pCurrFile = (PFrustrumFile) malloc (sizeof (FrustrumFile));

    if (NULL == pCurrFile)
        {
        fclose (pFileIn);

        return FR_open_fail;
        }

    PFrustrumFile   pTempFile = 0;

    if (NULL == s_pFrOpenFiles)
        {
        s_pFrOpenFiles = pCurrFile;
        }
    else
        {
        for (pTempFile = s_pFrOpenFiles; pTempFile->next; pTempFile = pTempFile->next);

        pTempFile->next = pCurrFile;
        }

    // initialise file structure
    pCurrFile->next = NULL;

    if (s_pFrOpenFiles == pCurrFile)
        pCurrFile->prev = NULL;
    else
        pCurrFile->prev = pTempFile;

    int     streamInd;

    for (streamInd = 0; s_frStreamIdArray[streamInd]; streamInd++);

    s_frStreamIdArray[streamInd] = streamInd + 1;

    pCurrFile->strid  = streamInd + 1;
    pCurrFile->guise  = guiseIn;
    pCurrFile->format = formatIn;
    pCurrFile->access = accessIn;
    pCurrFile->stream = pFileIn;

    wcscpy (pCurrFile->name, pFilenameIn);
    wcscpy (pCurrFile->key, pKeynameIn);

    s_frFileCount++;

    *pFileOut = pCurrFile;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* purpose reads required amount of data from open file into buffer
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      readFromFile
(
PFrustrumFile   pFileIn,            // => input file pointer
char*           pBufferOut,         // <= output buffer
int             headerFlagIn,       // => reading header ?
int             maxBufferLenIn,     // => input max buffer size
int*            pBufferLenOut       // <= no. of chars read
)
    {
    if (headerFlagIn || FFTEXT == pFileIn->format)
        {
        if (1 == maxBufferLenIn)
            {
            int     value = fgetc (pFileIn->stream);

            if (EOF == value)
                {
                return (feof (pFileIn->stream) ? FR_end_of_file : FR_read_fail);
                }
            else
                {
                pBufferOut[0] = static_cast<char>(value);
                *pBufferLenOut = 1;
                }

            return FR_no_errors;
            }

        int     required = (maxBufferLenIn + 1) * sizeof (char);

        // check whether current global input-output buffer long enough
        if (s_frIOBufferLength < required)
            {
            if (NULL != s_pFrIOBuffer)
                free (s_pFrIOBuffer);

            s_frIOBufferLength = 0;
            s_pFrIOBuffer = (char*) malloc (required);

            if (NULL == s_pFrIOBuffer)
                return FR_unspecified;

            s_frIOBufferLength = required;
            }

        // note that the second argument to fgets is the maximim number
        // of characters which can ever be written (including the null)
        // which is why the second argument to fgets = maxBufferLenIn+1
        if (NULL == fgets (s_pFrIOBuffer, maxBufferLenIn + 1, pFileIn->stream))
            return (feof (pFileIn->stream) ? FR_end_of_file : FR_read_fail);

        // copy input buffer back to calling function without terminator
        *pBufferLenOut = (int) strlen (s_pFrIOBuffer);
        strncpy (pBufferOut, s_pFrIOBuffer, *pBufferLenOut);

        return FR_no_errors;
        }

    int     chars = (int) fread (pBufferOut, (unsigned) (sizeof (char)), maxBufferLenIn, pFileIn->stream);

    if (0 == chars)
        return (feof (pFileIn->stream) ? FR_end_of_file : FR_read_fail);

    *pBufferLenOut = chars;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* purpose writes given buffer to open file
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6386) // static analysis thinks we exceed the bounds of s_pFrIOBuffer... I don't see how.
static int      writeToFile
(
PFrustrumFile   pFileIn,            // => input file pointer
char const*     pBufferIn,          // => input buffer
int             headerFlagIn,       // => reading header ?
size_t          bufferLenIn         // => input buffer size
)
    {
    if (headerFlagIn || pFileIn->format == FFTEXT)
        {
        if (1 == bufferLenIn)
            {
            if (EOF == fputc (pBufferIn[0], pFileIn->stream))
                return FR_write_fail;

            return FR_no_errors;
            }

        int     required = (int) ((bufferLenIn + 1) * sizeof (char));

        // check whether the global input-output buffer is long enough
        if (s_frIOBufferLength < required)
            {
            if (NULL != s_pFrIOBuffer)
                free (s_pFrIOBuffer);

            s_frIOBufferLength = 0;
            s_pFrIOBuffer = (char*) malloc (required);

            if (NULL == s_pFrIOBuffer)
                return FR_unspecified;
            else
                s_frIOBufferLength = required;
            }

        // copy the buffer and add a null-terminating character
        for (size_t count = 0; count < bufferLenIn; count ++)
            s_pFrIOBuffer[count] = pBufferIn[count];

        s_pFrIOBuffer[bufferLenIn] = END_OF_STRING_C;

        // the string will already contain any necessary formatting characters
        // (added by the header routines or Parasolid); fputs does not add any
        if (EOF == fputs (s_pFrIOBuffer, pFileIn->stream))
            return FR_write_fail;

        return FR_no_errors;
        }

    int     written = (int) fwrite (pBufferIn, (unsigned) (sizeof (char)), bufferLenIn, pFileIn->stream);

    // check binary file write...
    if (written != bufferLenIn)
        return FR_write_fail;

    return FR_no_errors;
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* purpose skip header information on opening a file to
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      skipFileHeader (PFrustrumFile pFileIn)
    {
    char        buffer[FR_MAX_HEADER_LINE];
    int         chars_read = 0;
    int         end_header = 0;
    int         first_line = 1;

    while (!end_header)
        {
        int     failureCode = readFromFile (pFileIn, buffer, 1, FR_MAX_HEADER_LINE, &chars_read);

        if (FR_no_errors != failureCode)
            return failureCode;

        if (0 == strncmp (buffer, s_frTrailerStart, strlen (s_frTrailerStart)))
            {
            // this is the end of the header
            end_header = 1;
            }
        else if (first_line && 0 != strncmp (buffer, s_frPreamble1, strlen (s_frPreamble1)))
            {
            // rewind the file to the beginning as the header is not there
            // (this must be a Parasolid version 1 or Romulus version 6 file)
            rewind (pFileIn->stream);
            end_header = 1;
            }
        else
            {
            // line skipped
            }

        first_line = 0;
        }

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* purpose skip standard header information to file
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      writeFileHeader (PFrustrumFile pFileIn, const char pr2hdr[])
    {
    if (FR_no_errors != writeToFile (pFileIn, s_frPreamble1, 1, strlen (s_frPreamble1)))
        return FR_write_fail;

    if (FR_no_errors != writeToFile (pFileIn, s_frPreamble2, 1, strlen (s_frPreamble2)))
        return FR_write_fail;

    if (FR_no_errors != writeToFile (pFileIn, s_frPrefix1, 1, strlen (s_frPrefix1)))
        return FR_write_fail;

    char        buffer[FR_MAX_HEADER_LINE];

    // machine specific - the fr should write the machine name
    BeStringUtilities::Strncpy(buffer, "MC=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - the fr should write the machine model number
    BeStringUtilities::Strncpy (buffer, "MC_MODEL=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - the fr should write the machine identifier
    BeStringUtilities::Strncpy (buffer, "MC_ID=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - the fr should write the operating system name
    BeStringUtilities::Strncpy (buffer, "OS=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - the fr should write the operating system versn
    BeStringUtilities::Strncpy (buffer, "OS_RELEASE=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - this should be replaced by your company name
    BeStringUtilities::Strncpy (buffer, "FRU=Bentley_Systems;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - this should be replaced by your product's name
    BeStringUtilities::Strncpy (buffer, "APPL=Microstation_modeler;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - this should be replaced by your company's location
    BeStringUtilities::Strncpy (buffer, "SITE=Exton;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - this should be replaced by runtime user's login id
    BeStringUtilities::Strncpy (buffer, "USER=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    BeStringUtilities::Strncpy (buffer, "FORMAT=");
    strcat (buffer, getFileFormatString (pFileIn->format));
    strcat (buffer, ";\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    BeStringUtilities::Strncpy (buffer, "GUISE=");
    strcat (buffer, getFileGuiseString (pFileIn->guise));
    strcat (buffer, ";\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    char    fileStr[FR_MAX_NAMELEN + 1];

    BeStringUtilities::Strncpy (buffer, "KEY=");
    strcat (buffer, BeStringUtilities::WCharToCurrentLocaleChar (fileStr, pFileIn->key, sizeof (fileStr)));
    strcat (buffer, ";\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    BeStringUtilities::Strncpy (buffer, "FILE=");
    strcat (buffer, BeStringUtilities::WCharToCurrentLocaleChar (fileStr, pFileIn->name, sizeof (fileStr)));
    strcat (buffer, ";\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    // machine specific - this should be replaced by the runtime date
    BeStringUtilities::Strncpy (buffer, "DATE=unknown;\n");
    if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
        return FR_write_fail;

    if (FR_no_errors != writeToFile (pFileIn, s_frPrefix2, 1, strlen (s_frPrefix2)))
        return FR_write_fail;

    int     buffer_count = 0;
    size_t  pr2hdrLen = strlen (pr2hdr);

    for (size_t pd2_count = 0; pd2_count < pr2hdrLen; pd2_count++)
        {
        char    c = buffer[buffer_count] = pr2hdr[pd2_count];

        if (c == ';')
            {
            buffer[buffer_count + 1 ] = NEW_LINE_C;
            buffer[buffer_count + 2 ] = END_OF_STRING_C;

            if (FR_no_errors != writeToFile (pFileIn, buffer, 1, strlen (buffer)))
                return FR_write_fail;

            buffer_count = 0;
            }
        else
            {
            buffer_count++;
            }
        }

    if (FR_no_errors != writeToFile (pFileIn, s_frPrefix3, 1, strlen (s_frPrefix3)))
        return FR_write_fail;

    if (FR_no_errors != writeToFile (pFileIn, s_frTrailer, 1, strlen (s_frTrailer)))
        return FR_write_fail;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int      writeXMLFileHeader (PFrustrumFile pFileIn, const char pr2hdr[])
    {
    char        buffer[FR_MAX_HEADER_LINE];

    BeStringUtilities::Strncpy(buffer, "<?xml version=\"1.0\" ?>" ); // <?xml version

    return writeToFile (pFileIn, buffer, 1, strlen (buffer));
    }

/*---------------------------------------------------------------------------------**//**
* purpose delete given file
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int      deleteFile (wchar_t* filename)
    {
    if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(filename))
        return FR_close_fail;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* purpose Start fr; set up file structures if not already done
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FSTART (int *pFailureCodeOut)
    {
    int         streamInd;

    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted == 0)
        {
        for (streamInd = 0; streamInd < FR_MAX_OPEN_FILES; streamInd++)
            s_frStreamIdArray[streamInd] = 0;

        // set up fr file header variables
        setupHeaderStrings ();
        }

    s_frStarted++;

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Stop fr
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FSTOP (int *pFailureCodeOut)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    s_frStarted--;

    if (s_pFrIOBuffer != NULL)
        {
        s_frIOBufferLength = 0;
        free (s_pFrIOBuffer);
        s_pFrIOBuffer = NULL;
        }

    if (s_frStarted == 0)
        {
        PFrustrumFile   pCurrOpenFile = s_pFrOpenFiles;

        // while there are still files open - close them down
        while (pCurrOpenFile)
            {
            fclose (pCurrOpenFile->stream);

            if (pCurrOpenFile->next == NULL)
                {
                // free the space used in the file pointer
                free (pCurrOpenFile);
                pCurrOpenFile = NULL;
                }
            else
                {
                pCurrOpenFile = pCurrOpenFile->next;
                free (pCurrOpenFile->prev);
                }
            }

        // reset variables and return values
        s_frFileCount = 0;
        s_pFrOpenFiles = NULL;
        }

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Allocate memory as requested
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FMALLO
(
int*            pNumBytesIn,        // => Input total bytes requested
char**          ppMemoryOut,        // <= Output memory pointer
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *ppMemoryOut = 0;
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    *ppMemoryOut = (char *) bentleyAllocator_malloc(*pNumBytesIn);

    if (*ppMemoryOut == NULL)
        {
        *pFailureCodeOut = FR_memory_full;
        return;
        }

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Frees memory
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FMFREE
(
int*            pNumBytesIn,        // => Input total bytes requested
char**          ppMemoryIn,         // => Input memory pointer
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    bentleyAllocator_free(*ppMemoryIn);

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_UCOPRD
(
const int           guise,      // => input class of file
const int           format,     // => input binary or text
const PK_UCHAR_t    name[],     // => input key which identifies file (null-terminated Unicode)
const PK_LOGICAL_t  skiphd,     // => input skip header or not
int*                strid       // <= output stream-id on which file is opened
)
    {
    if (s_frStarted <= 0)
        return FR_NOT_STARTED;

    // check that limit hasn't been reached
    if (s_frFileCount == FR_MAX_OPEN_FILES)
        return FR_open_fail;

    wchar_t     filename[FR_MAX_NAMELEN + 1];

    wcscpy (filename, (wchar_t const*) name);

#if defined(BENTLEYCONFIG_OS_UNIX)
    WString lower(filename);
    lower.ToLower();
    wcscpy(filename, lower.c_str());
#endif
    // prepend the schema path
    prependSchemaExtension (filename, guise);

    // add the file extension
    appendExtension (filename, getFileExtension (guise, format));

    FILE*   fp;

    // open file for reading
    if (FFTEXT == format)
#if defined(BENTLEYCONFIG_OS_WINDOWS)
        fp = _wfopen (filename, L"r");
#elif defined(BENTLEYCONFIG_OS_UNIX)
        fp = fopen(Utf8String(filename).c_str(), "r");
#else
        #error Unsupported OS.
#endif
    else
#if defined(BENTLEYCONFIG_OS_WINDOWS)
        fp = _wfopen (filename, L"rb");
#elif defined(BENTLEYCONFIG_OS_UNIX)
        fp = fopen(Utf8String(filename).c_str(), "rb");
#else
        #error Unsupported OS.
#endif

    if (0 == fp)
        return FR_not_found;

    PFrustrumFile   pFr;

    if (FR_no_errors != registerOpenFile (fp, guise, format, FR_READ_ACCESS, filename, (wchar_t const*) name, &pFr))
        return FR_open_fail;

    if (skiphd == FFSKHD)
        {
        if (FR_no_errors != skipFileHeader (pFr))
            return FR_bad_header;
        }

    *strid = pFr->strid;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_UCOPWR
(
const int           guise,      // => input class of file
const int           format,     // => input binary or text
const PK_UCHAR_t    name[],     // => input key which identifies file (null-terminated Unicode)
const char          pr2hdr[],   // => input part 2 header data (null-terminated)
int*                strid       // <= output stream-id on which file is opened
)
    {
    if (s_frStarted <= 0)
        return FR_NOT_STARTED;

    if (s_frFileCount == FR_MAX_OPEN_FILES)
        return FR_open_fail;

    wchar_t     filename[FR_MAX_NAMELEN + 1];

    wcscpy (filename, (wchar_t const*) name);

    // add the file extension
    appendExtension (filename, getFileExtension (guise, format));

    FILE*   fp;

    // open file for writing
    if (FFTEXT == format)
#if defined(BENTLEYCONFIG_OS_WINDOWS)
        fp = _wfopen (filename, L"w");
#elif defined(BENTLEYCONFIG_OS_UNIX)
        fp = fopen(Utf8String(filename).c_str(), "w");
#else
        #error Unsupported OS.
#endif
    else
#if defined(BENTLEYCONFIG_OS_WINDOWS)
        fp = _wfopen (filename, L"wb");
#elif defined(BENTLEYCONFIG_OS_UNIX)
        fp = fopen(Utf8String(filename).c_str(), "wb");
#else
        #error Unsupported OS.
#endif

    if (0 == fp)
        return FR_already_exists;

    PFrustrumFile   pFr;

    if (FR_no_errors != registerOpenFile (fp, guise, format, FR_WRITE_ACCESS, filename, (wchar_t const*) name, &pFr))
        return FR_open_fail;

    if (FFCDBG == guise)
        {
        if (FR_no_errors != writeXMLFileHeader (pFr, pr2hdr))
            return FR_bad_header;
        }
    else
        {
        if (FR_no_errors != writeFileHeader (pFr, pr2hdr))
            return FR_bad_header;
        }

    *strid = pFr->strid;

    return FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Open temporary rollback file for read/write
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFOPRB
(
const int*      pGuiseIn,           // => input class of file
const int*      pMinSizeIn,         // => input minimum file size (bytes)
const int*      pMaxSizeIn,         // => input maximum file size (bytes)
int*            pSizeOut,           // <= ouput actual file size (bytes)
int*            pStreamIdOut,       // <= output stream-id on which file is opened
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    PFrustrumFile   pFr;

    *pFailureCodeOut = FR_unspecified;
    *pStreamIdOut = FR_NULL_STRID;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    if (s_frFileCount == FR_MAX_OPEN_FILES)
        {
        *pFailureCodeOut = FR_open_fail;
        return;
        }

    if (*pGuiseIn != FFCROL)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    wchar_t filename[FR_MAX_NAMELEN + 1];
    wchar_t keyname[FR_MAX_NAMELEN + 1];

    BeStringUtilities::Wcsncpy(filename, L"rollback.001");
    BeStringUtilities::Wcsncpy(keyname, L"rollback");

#if defined(BENTLEYCONFIG_OS_WINDOWS)
    FILE*   fp = _wfopen (filename, L"a");
#elif defined(BENTLEYCONFIG_OS_UNIX)
    FILE*   fp = fopen(Utf8String(filename).c_str(), "a");
#else
        #error Unsupported OS.
#endif

    if (0 == fp)
        {
        *pFailureCodeOut = FR_open_fail;
        return;
        }

    int     fileFormat = FFBNRY;

    *pFailureCodeOut = registerOpenFile (fp, *pGuiseIn, fileFormat, FR_READ_WRITE_ACCESS, filename, keyname, &pFr);

    if (*pFailureCodeOut != FR_no_errors)
        return;

    *pSizeOut = *pMaxSizeIn;
    *pStreamIdOut = pFr->strid;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Open a file for read
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFOPRD
(
const int*      pGuiseIn,           // => input class of file
const int*      pFormatIn,          // => input binary or text
const char      fileKeyIn[],        // => input key which identifies file
const int*      pKeyLengthIn,       // => input fileKeyIn length
const int*      pSkipHeaderIn,      // => input skip header or not ?
int*            pStreamIdOut,       // <= output stream-id on which file is opened
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    // NOTE: This is still called for schema files...
    PK_UCHAR_t  fileKeyW[FR_MAX_NAMELEN + 1];

    BeStringUtilities::CurrentLocaleCharToWChar ((WCharP) fileKeyW, fileKeyIn, _countof (fileKeyW));

    *pFailureCodeOut = pki_UCOPRD (*pGuiseIn, *pFormatIn, fileKeyW, (*pSkipHeaderIn == 0 ? PK_LOGICAL_false : PK_LOGICAL_true), pStreamIdOut);
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Open a file for write, also writes the standard header
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFOPWR
(
const int*      pGuiseIn,               // => input class of file
const int*      pFormatIn,              // => input binary or text
const char      fileKeyIn[],            // => input key which identifies file
const int*      pKeyLengthIn,           // => input fileKeyIn length
const char      part2HeaderIn[],        // => input part 2 header data
const int*      pPart2HeaderLenIn,      // => input part 2 header data length
int*            pStreamIdOut,           // <= output stream-id on which file is opened
int*            pFailureCodeOut         // <= outputs failure code if error, else 0
)
    {
    // NOTE: This is still called for schema files...
    PK_UCHAR_t  fileKeyW[FR_MAX_NAMELEN + 1];

    BeStringUtilities::CurrentLocaleCharToWChar ((WCharP) fileKeyW, fileKeyIn, _countof (fileKeyW));

    char*       pr2hdrP = (pPart2HeaderLenIn ? (char*) malloc (((*pPart2HeaderLenIn)+1) * sizeof (char)) : NULL);

    if (pr2hdrP)
        {
        strncpy (pr2hdrP, part2HeaderIn, *pPart2HeaderLenIn);
        pr2hdrP[*pPart2HeaderLenIn] = END_OF_STRING_C;
        }

    *pFailureCodeOut = pki_UCOPWR (*pGuiseIn, *pFormatIn, fileKeyW, pr2hdrP, pStreamIdOut);
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Read buffer from open file
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFREAD
(
const int*      pGuiseIn,           // => input class of file
const int*      pStreamIdIn,        // => input stream-id on which file is opened
const int*      maxCharIn,          // => input max. chars. to read
char            bufferOut[],        // <= output buffer containing read data
int*            numCharOut,         // <= output actual no. of chars. read
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;
    *numCharOut = 0;

    // check that the frustrum has been started
    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    PFrustrumFile   pCurrOpenFile;

    // find the correct file pointer
    for (pCurrOpenFile = s_pFrOpenFiles; pCurrOpenFile != NULL; pCurrOpenFile = pCurrOpenFile->next)
        {
        if (pCurrOpenFile->strid == *pStreamIdIn)
            break;
        }

    if (pCurrOpenFile == NULL)
        {
        *pFailureCodeOut = FR_INTERNAL_ERROR;
        return;
        }

    // check file guise
    if (*pGuiseIn != pCurrOpenFile->guise)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    // check access
    if (pCurrOpenFile->access != FR_READ_ACCESS && pCurrOpenFile->access != FR_READ_WRITE_ACCESS)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    int     chars_read = 0;

    // read the information from the file
    *pFailureCodeOut = readFromFile (pCurrOpenFile, bufferOut, 0, *maxCharIn, &chars_read);

    if (FR_no_errors != *pFailureCodeOut)
        return;

    *numCharOut = chars_read;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Change position in rollback file
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFSEEK
(
const int*      pGuiseIn,           // => input class of file
const int*      pStreamIdIn,        // => input stream-id on which file is opened
const int*      pPositionIn,        // => input required seek position
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    if (*pGuiseIn != FFCROL)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    PFrustrumFile   pCurrOpenFile;

    // check file is open
    for (pCurrOpenFile = s_pFrOpenFiles; pCurrOpenFile != NULL; pCurrOpenFile = pCurrOpenFile->next)
        {
        if (pCurrOpenFile->strid == *pStreamIdIn)
            break;
        }

    if (pCurrOpenFile == NULL)
        {
        *pFailureCodeOut = FR_INTERNAL_ERROR;
        return;
        }

    // check file guise
    if (*pGuiseIn != pCurrOpenFile->guise)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    // reset file pointer
    if (fseek (pCurrOpenFile->stream, (long) (*pPositionIn), 0) != 0)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Indicate position in rollback file
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFTELL
(
const int*      pGuiseIn,           // => input class of file
const int*      pStreamIdIn,        // => input stream-id on which file is opened
int*            pPositionOut,       // <= output required seek position
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    if (*pGuiseIn != FFCROL)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    PFrustrumFile   pCurrOpenFile;

    // check file is open
    for (pCurrOpenFile = s_pFrOpenFiles; pCurrOpenFile != NULL; pCurrOpenFile = pCurrOpenFile->next)
        {
        if (pCurrOpenFile->strid == *pStreamIdIn)
            break;
        }

    if (pCurrOpenFile == NULL)
        {
        *pFailureCodeOut = FR_INTERNAL_ERROR;
        return;
        }

    // check file guise
    if (*pGuiseIn != pCurrOpenFile->guise)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    // note file pointer
    *pPositionOut = ftell (pCurrOpenFile->stream);

    *pFailureCodeOut = FR_no_errors;
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Write buffer to open file
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFWRIT
(
const int*      pGuiseIn,           // => input class of file
const int*      pStreamIdIn,        // => input stream-id on which file is opened
const int*      pNumCharsIn,        // => input no. of chars. to write
const char      bufferIn[],         // <= output buffer containing read data
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    PFrustrumFile   pCurrOpenFile;

    // find the file info for this stream-id
    for (pCurrOpenFile = s_pFrOpenFiles; pCurrOpenFile != NULL; pCurrOpenFile = pCurrOpenFile->next)
        {
        if (pCurrOpenFile->strid == *pStreamIdIn)
            break;
        }

    if (pCurrOpenFile == NULL)
        {
        *pFailureCodeOut = FR_INTERNAL_ERROR;
        return;
        }

    // check file guise
    if (*pGuiseIn != pCurrOpenFile->guise)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    // check access
    if (pCurrOpenFile->access != FR_WRITE_ACCESS && pCurrOpenFile->access != FR_READ_WRITE_ACCESS)
        {
        *pFailureCodeOut = FR_unspecified;
        return;
        }

    *pFailureCodeOut = writeToFile (pCurrOpenFile, bufferIn, 0, *pNumCharsIn);
    }

/*---------------------------------------------------------------------------------**//**
* author DeepakMalkan
* purpose Close speficied file. If a rollback file, or abort then delete the file.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_FFCLOS
(
const int*      pGuiseIn,           // => input class of file
const int*      pStreamIdIn,        // => input stream-id on which file is opened
const int*      pActionIn,          // => input closure type
int*            pFailureCodeOut     // <= outputs failure code if error, else 0
)
    {
    *pFailureCodeOut = FR_unspecified;

    if (s_frStarted <= 0)
        {
        *pFailureCodeOut = FR_NOT_STARTED;
        return;
        }

    PFrustrumFile   pCurrOpenFile;

    // find the file info for this stream-id
    for (pCurrOpenFile = s_pFrOpenFiles; pCurrOpenFile; pCurrOpenFile = pCurrOpenFile->next)
        {
        if (pCurrOpenFile->strid == *pStreamIdIn)
            break;
        }

    if (pCurrOpenFile == NULL)
        {
        *pFailureCodeOut = FR_close_fail;
        return;
        }

    bool        doDeleteFile = false;
    wchar_t     filename[FR_MAX_NAMELEN + 1];

    if (pCurrOpenFile->access == FR_READ_WRITE_ACCESS || (pCurrOpenFile->access == FR_WRITE_ACCESS && *pActionIn == FFABOR))
        {
        doDeleteFile = true;
        wcscpy (filename, pCurrOpenFile->name);
        }

    // close file
    s_frStreamIdArray[pCurrOpenFile->strid - 1] = 0;

    if (EOF == fclose (pCurrOpenFile->stream))
        {
        *pFailureCodeOut = FR_close_fail;
        return;
        }

    if (pCurrOpenFile == s_pFrOpenFiles)
        s_pFrOpenFiles = s_pFrOpenFiles->next;
    else
        pCurrOpenFile->prev->next = pCurrOpenFile->next;

    if (pCurrOpenFile->next != NULL)
        pCurrOpenFile->next->prev = pCurrOpenFile->prev;

    free (pCurrOpenFile);
    s_frFileCount--;

    *pFailureCodeOut = FR_no_errors;

    if (!doDeleteFile)
        return;

    *pFailureCodeOut = deleteFile (filename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/94
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr curveFromParams
(
double const*   geomArrayP,
bool            rational,
int             nPoles,
int             order
)
    {
    if (nPoles < order)
        return nullptr;

    if (2 == order)
        {
        bvector<DPoint3d> points;

        for (int i = 0; i < nPoles; i++)
            {
            DPoint3d pole;

            pole.x = *geomArrayP++;
            pole.y = *geomArrayP++;
            pole.z = *geomArrayP++;

            if (rational)
                geomArrayP++;

            points.push_back(pole);
            }

        return ICurvePrimitive::CreateLineString(points);
        }

    MSBsplineCurve  bcurve;

    bcurve.Zero();
    bcurve.rational = rational;
    bcurve.params.order = order;
    bcurve.params.numPoles = nPoles;
    bcurve.params.numKnots = nPoles - order;
    bcurve.display.polygonDisplay = false;
    bcurve.display.curveDisplay = true;
    bcurve.Allocate();

    int         i;
    double*     weightP;
    DPoint3d*   poleP;

    for (i = 0, poleP = bcurve.poles, weightP = bcurve.weights; i < nPoles; i++, poleP++)
        {
        poleP->x = *geomArrayP++;
        poleP->y = *geomArrayP++;
        poleP->z = *geomArrayP++;

        if (rational)
            poleP->Scale(*poleP, *weightP++ = *geomArrayP++);
        }

    memcpy(bcurve.knots, geomArrayP, (bcurve.params.numPoles + bcurve.params.order) * sizeof(double));
    bspknot_normalizeKnotVector(bcurve.knots, bcurve.params.numPoles, bcurve.params.order, bcurve.params.closed);

    return ICurvePrimitive::CreateBsplineCurveSwapFromSource(bcurve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/94
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr curveFromPoints
(
DPoint3dCP  pointsP,
int         nPoints
)
    {
    if (nPoints < 2)
        return nullptr;

    return ICurvePrimitive::CreateLineString(pointsP, nPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/98
+---------------+---------------+---------------+---------------+---------------+------*/
static double ellipseAngle
(
DPoint3dCP      centerP,
RotMatrixCP     rMatrixP,
double          major,
double          minor,
DPoint3dCP      pointP
)
    {
    DPoint3d delta;

    delta.DifferenceOf(*pointP, *centerP);
    rMatrixP->MultiplyTranspose(delta);

    if (delta.x == 0.0 && delta.y == 0.0)
        return 0.0;
    else
        return atan2(major * delta.y, minor * delta.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void legacyMath_RMatrix_FromNormalVector (RotMatrixP rotMatrixP, DPoint3dCP normalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    zNormal = *(DVec3dCP)normalP;
    zNormal.Normalize ();

    if ((fabs (zNormal.x) < 0.01) && (fabs (zNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    xNormal.CrossProduct (world, zNormal);
    xNormal.Normalize ();
    yNormal.CrossProduct (zNormal, xNormal);
    yNormal.Normalize ();
    rotMatrixP->InitFromRowVectors (xNormal, yNormal, zNormal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/98
+---------------+---------------+---------------+---------------+---------------+------*/
static DEllipse3d ellipseFromParams
(
DPoint3dCP      centerP,
DVec3dCP        normalP,
DVec3dCP        majorDirectionP,
DVec3dCP        minorDirectionP,
double          major,
double          minor,
DPoint3dCP      startP,
DPoint3dCP      endP
)
    {
    DPoint3d    center = *centerP;
    double      rX = major;
    double      rY = minor;
    RotMatrix   rMatrix;

    if (NULL != normalP && NULL == majorDirectionP)
        {
        legacyMath_RMatrix_FromNormalVector(&rMatrix, normalP);
        rMatrix.InverseOf(rMatrix);
        }
    else if (NULL != normalP && NULL != majorDirectionP)
        {
        DVec3d minorDirection;

        minorDirection.CrossProduct(*normalP, *majorDirectionP);
        rMatrix.InitFromColumnVectors(*majorDirectionP, minorDirection, *normalP);
        }
    else
        {
        DVec3d normal;

        normal.CrossProduct(*majorDirectionP, *minorDirectionP);
        rMatrix.InitFromColumnVectors(*majorDirectionP, *minorDirectionP, normal);
        }

    double start = (NULL == startP ? 0.0 : ellipseAngle(&center, &rMatrix, major, minor, startP));
    double sweep = (NULL == endP ? msGeomConst_2pi : ellipseAngle(&center, &rMatrix, major, minor, endP) - start);

    if (sweep < 0.0)
        sweep += msGeomConst_2pi;

    DVec3d primary, secondary;

    rMatrix.GetColumn(primary, 0);
    rMatrix.GetColumn(secondary, 1);

    DEllipse3d ellipse;

    ellipse.InitFromDGNFields3d(center, primary, secondary, rX, rY, start, sweep);

    return ellipse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/98
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr pki_extractGoOutput
(
int             lineType,
int             nGeoms,
double const*   geomArrayP,
int             nLineType,
int const*      lineTypeP
)
    {
    switch (lineType)
        {
        case L3TPSL:
            return curveFromPoints((DPoint3dCP) geomArrayP, 2);

        case L3TPCC:
            return ICurvePrimitive::CreateArc(ellipseFromParams((DPoint3dCP) &geomArrayP[0], (DVec3dCP) &geomArrayP[3], nullptr, nullptr, geomArrayP[6], geomArrayP[6], nullptr, nullptr));

        case L3TPCI:
            return ICurvePrimitive::CreateArc(ellipseFromParams((DPoint3dCP) &geomArrayP[0], (DVec3dCP) &geomArrayP[3], nullptr, nullptr, geomArrayP[6], geomArrayP[6], (DPoint3dCP) &geomArrayP[7], (DPoint3dCP) &geomArrayP[10]));

        case L3TPCE:
            return ICurvePrimitive::CreateArc(ellipseFromParams((DPoint3dCP) &geomArrayP[0], nullptr, (DVec3dCP) &geomArrayP[3], (DVec3dCP) &geomArrayP[6], geomArrayP[9], geomArrayP[10], nullptr, nullptr));

        case L3TPEL:
            return ICurvePrimitive::CreateArc(ellipseFromParams((DPoint3dCP) &geomArrayP[0], nullptr, (DVec3dCP) &geomArrayP[3], (DVec3dCP) &geomArrayP[6], geomArrayP[9], geomArrayP[10], (DPoint3dCP) &geomArrayP[11], (DPoint3dCP) &geomArrayP[14]));

        case L3TPPY:
            return curveFromPoints((DPoint3dCP) geomArrayP, nGeoms);

        case L3TPNC:
        case L3TPRN:
            return curveFromParams(geomArrayP, L3TPRN == lineType, lineTypeP[9], lineTypeP[8] + 1);

        default:
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void pki_GOSGMT
(
const int      *pSegTypeIn,         /* => input segment type  */
const int      *pNumTagIn,          /* => input size of tag array */
const int      *pTagArrayIn,        /* => input segment tag array */
const int      *pNumGeomIn,         /* => input size of geom array */
const double   *pGeomArrayIn,       /* => input segment geom array */
const int      *pNumLineTypeIn,     /* => input size of line type array */
const int      *pLineTypeArrayIn,   /* => input line type array */
int            *pFailureCodeOut     /* <= output failure code: CONTIN or ABORT */
)
    {
    *pFailureCodeOut = CONTIN;

    switch (*pSegTypeIn)
        {
        case SGTPED:
        case SGTPPL:
        case SGTPSI:
        case SGTPRH:
            {
            if (nullptr != s_frustrumOutput.m_wireOutput)
                {
                ICurvePrimitivePtr curve = pki_extractGoOutput(pLineTypeArrayIn[1], *pNumGeomIn, pGeomArrayIn, *pNumLineTypeIn, pLineTypeArrayIn);

                if (!curve.IsValid())
                    break;

                if (SUCCESS != s_frustrumOutput.m_wireOutput->_ProcessGoOutput(*curve, pTagArrayIn[0]))
                    *pFailureCodeOut = ABORT;
                }
            else if (nullptr != s_frustrumOutput.m_hlineOutput)
                {
                int     visibilityToken = pLineTypeArrayIn[3];
                bool    isSmooth = (CODSMO == pLineTypeArrayIn[4] && CODNIN != pLineTypeArrayIn[5]); /* Not sure about this condition for smoothness -JS 2/98 */

                if ((visibilityToken == CODVIS || visibilityToken == CODUNV))
                    {
                    ICurvePrimitivePtr curve = pki_extractGoOutput(pLineTypeArrayIn[1], *pNumGeomIn, pGeomArrayIn, *pNumLineTypeIn, pLineTypeArrayIn);

                    if (!curve.IsValid())
                        break;

                    if (SUCCESS != s_frustrumOutput.m_hlineOutput->_ProcessGoOutput(*curve, pTagArrayIn[0], false, isSmooth, *pSegTypeIn))
                        *pFailureCodeOut = ABORT;
                    }
                else if (s_frustrumOutput.m_hlineOutput->_ReturnHidden() && (visibilityToken == CODDRV || (s_frustrumOutput.m_hlineOutput->_IncludeTraceEdges() && visibilityToken == CODINV)))
                    {
                    ICurvePrimitivePtr curve = pki_extractGoOutput(pLineTypeArrayIn[1], *pNumGeomIn, pGeomArrayIn, *pNumLineTypeIn, pLineTypeArrayIn);

                    if (!curve.IsValid())
                        break;

                    if (SUCCESS != s_frustrumOutput.m_hlineOutput->_ProcessGoOutput(*curve, pTagArrayIn[0], true, isSmooth, *pSegTypeIn))
                        *pFailureCodeOut = ABORT;
                    }
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_GOOPPX
(
const int      *pNumRealIn,        /* => input number of reals in real array  */
const double   *pRealArrayIn,        /* => input real array  */
const int      *pNumIntIn,        /* => input number of integers in int array  */
const int      *pIntArrayIn,        /* => input integer array  */
int            *pFailureCodeOut        /* <= output failure code: CONTIN or ABORT  */
)
    {
    *pFailureCodeOut = CONTIN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_GOCLPX
(
const int      *pNumRealIn,        /* => input number of reals in real array  */
const double   *pRealArrayIn,        /* => input real array  */
const int      *pNumIntIn,        /* => input number of integers in int array  */
const int      *pIntArrayIn,        /* => input integer array  */
int            *pFailureCodeOut        /* <= output failure code: CONTIN or ABORT  */
)
    {
    *pFailureCodeOut = CONTIN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_GOPIXL
(
const int      *pNumPixelIn,        /* => input number of pixels to output  */
const double   *pPixelArrayIn,        /* => input real array of pixel intensities  */
const int      *pNumIntIn,        /* => input number of integers in int array  */
const int      *pIntArrayIn,        /* => input integer array  */
int            *pFailureCodeOut        /* <= output failure code: CONTIN or ABORT  */
)
    {
    *pFailureCodeOut = CONTIN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_GOOPSG
(
const int      *pSegTypeIn,        /* => input segment type   */
const int      *pNumTagIn,        /* => input size of tag array  */
const int      *pTagArrayIn,        /* => input segment tag array  */
const int      *pNumGeomIn,        /* => input size of geom array  */
const double   *pGeomArrayIn,        /* => input segment geom array  */
const int      *pNumLineTypeIn,        /* => input size of line type array  */
const int      *pLineTypeArrayIn,        /* => input line type array  */
int            *pFailureCodeOut        /* <= output failure code: CONTIN or ABORT  */
)
    {
    *pFailureCodeOut = CONTIN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pki_GOCLSG
(
const int      *pSegTypeIn,        /* => input segment type   */
const int      *pNumTagIn,        /* => input size of tag array  */
const int      *pTagArrayIn,        /* => input segment tag array  */
const int      *pNumGeomIn,        /* => input size of geom array  */
const double   *pGeomArrayIn,        /* => input segment geom array  */
const int      *pNumLineTypeIn,        /* => input size of line type array  */
const int      *pLineTypeArrayIn,        /* => input line type array  */
int            *pFailureCodeOut        /* <= output failure code: CONTIN or ABORT  */
)
    {
    *pFailureCodeOut = CONTIN;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Brien.Bastings                 11/2017
//---------------------------------------------------------------------------------------
static void computeViewTransform
(
PK_TRANSF_t&    viewTransformTag,   // <= view transform tag
DPoint3dCP      eyePoint,           // => eye point (nullptr if parallel)
DVec3dCR        direction           // => toward the view (positive Z)
)
    {
    PK_TRANSF_create_view_o_t options;

    PK_TRANSF_create_view_o_m(options);

    if (nullptr != eyePoint)
        {
        options.have_eye_position = PK_LOGICAL_true;
        options.eye_position.coord[0] = eyePoint->x;
        options.eye_position.coord[1] = eyePoint->y;
        options.eye_position.coord[2] = eyePoint->z;
        }

    DVec3d tmpDir = DVec3d::From(direction);
    PK_VECTOR1_t vector;

    tmpDir.Normalize();
    vector.coord[0] = -tmpDir.x;
    vector.coord[1] = -tmpDir.y;
    vector.coord[2] = -tmpDir.z;

    PK_TRANSF_create_view(vector, &options, &viewTransformTag);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Brien.Bastings                 11/2017
//---------------------------------------------------------------------------------------
void PSolidGoOutput::ProcessSilhouettes(IParasolidWireOutput& output, DPoint3dCP eyePoint, DVec3dCR direction, PK_ENTITY_t entityTag, double tolerance)
    {
    PK_TRANSF_t viewTransformTag = PK_ENTITY_null;

    computeViewTransform(viewTransformTag, eyePoint, direction);

    PK_TOPOL_render_line_o_t options;

    PK_TOPOL_render_line_o_m(options);
    options.edge = PK_render_edge_no_c;
    options.silhouette = PK_render_silhouette_arcs_c;
    options.visibility = PK_render_vis_no_c;

    if (tolerance > 0.0)
        {
        options.is_curve_chord_tol = true;
        options.curve_chord_tol = tolerance;
        }

    s_frustrumOutput.m_wireOutput = &output; // Setup static global callback function...
    /* unused - PK_ERROR_code_t failureCode = */
    PK_TOPOL_render_line(1, &entityTag, nullptr, viewTransformTag, &options);
    s_frustrumOutput.m_wireOutput = nullptr; // Clear static global callback function...

    PK_ENTITY_delete(1, &viewTransformTag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getAdjustedRadialAngles
(
PK_FACE_t       faceTag,
double&         radialAround,
double&         radialAbout,
bool            swapUV
)
    {
    PK_UVBOX_t  uvBox;

    if (SUCCESS != PK_FACE_find_uvbox (faceTag, &uvBox))
        return false;

    if (swapUV)
        {
        if (radialAround && (uvBox.param[3] - uvBox.param[1]) < radialAround + 5.0 * (msGeomConst_pi / 180.0))
            radialAround = 0.0;

        if (radialAbout && (uvBox.param[2] - uvBox.param[0]) < radialAbout + 5.0 * (msGeomConst_pi / 180.0))
            radialAbout = 0.0;
        }
    else
        {
        if (radialAround && (uvBox.param[2] - uvBox.param[0]) < radialAround + 5.0 * (msGeomConst_pi / 180.0))
            radialAround = 0.0;

        if (radialAbout && (uvBox.param[3] - uvBox.param[1]) < radialAbout + 5.0 * (msGeomConst_pi / 180.0))
            radialAbout = 0.0;
        }

    return (radialAround > 0.0 || radialAbout > 0.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Brien.Bastings                 11/2017
//---------------------------------------------------------------------------------------
static bool setupHatchOptionsForFace(PK_TOPOL_render_line_o_t& options, PK_FACE_t faceTag, int divisor, double tolerance)
    {
    PK_SURF_t   surfaceTag = PK_ENTITY_null;

    if (PK_ERROR_no_errors != PK_FACE_ask_surf(faceTag, &surfaceTag))
        return false;

    PK_CLASS_t  surfaceClass = 0;

    PK_ENTITY_ask_class(surfaceTag, &surfaceClass);

    switch (surfaceClass)
        {
        case PK_CLASS_plane:
            return false; // Not hatched...

        case PK_CLASS_cone:
        case PK_CLASS_cyl:
            {
            options.radial = PK_render_radial_yes_c;
            options.radial_around = (msGeomConst_2pi / (double) (2 * divisor));
            options.radial_around_start = msGeomConst_pi;

            return getAdjustedRadialAngles(faceTag, options.radial_around, options.radial_about, false);
            }

        case PK_CLASS_torus:
            {
            options.radial = PK_render_radial_yes_c;
            options.radial_around = (msGeomConst_2pi / 4.0); // Don't think other arcs are very useful snap locations...avoid clutter and just output every 90 degrees...
            options.radial_about = (msGeomConst_2pi / (double) (2 * divisor));
            options.radial_around_start = msGeomConst_pi;

            return getAdjustedRadialAngles(faceTag, options.radial_around, options.radial_about, true);
            }

        case PK_CLASS_sphere:
            {
            options.radial = PK_render_radial_yes_c;
            options.radial_around = (msGeomConst_2pi / (double) (2 * divisor));
            options.radial_about = (msGeomConst_2pi / 4.0); // Don't think other arcs are very useful snap locations...avoid clutter and just output equator...
            options.radial_around_start = msGeomConst_pi;
            options.radial_about_start = msGeomConst_pi;

            return getAdjustedRadialAngles(faceTag, options.radial_around, options.radial_about, false);
            }

        default:
            {
            double uDivisor = (double) divisor;
            double vDivisor = uDivisor;

            PK_PARAM_periodic_t periodicU;
            PK_PARAM_periodic_t periodicV;

            if (PK_ERROR_no_errors != PK_FACE_is_periodic(faceTag, &periodicU, &periodicV))
                return false;

            if (PK_PARAM_periodic_no_c != periodicU)
                uDivisor *= 2.0;

            if (PK_PARAM_periodic_no_c != periodicV)
                vDivisor *= 2.0;

            PK_UVBOX_t uvBox;

            if (SUCCESS != PK_FACE_find_uvbox(faceTag, &uvBox))
                return false;

            options.param = PK_render_param_yes_c;
            options.param_u = (uvBox.param[2] - uvBox.param[0]) / uDivisor;
            options.param_v = (uvBox.param[3] - uvBox.param[1]) / vDivisor;

            if (tolerance > 0.0)
                {
                options.is_curve_chord_tol = PK_LOGICAL_true;
                options.curve_chord_tol = tolerance;
                }

            return true;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Brien.Bastings                 11/2017
//---------------------------------------------------------------------------------------
void PSolidGoOutput::ProcessFaceHatching(IParasolidWireOutput& output, int divisor, PK_FACE_t entityTag, double tolerance)
    {
    if (0 == divisor)
        return;

    PK_TOPOL_render_line_o_t options;

    PK_TOPOL_render_line_o_m(options);

    options.edge   = PK_render_edge_no_c;
    options.bcurve = PK_render_bcurve_nurbs_c; // bcurves in nurbs format...

    if (!setupHatchOptionsForFace(options, entityTag, divisor, tolerance))
        return;

    s_frustrumOutput.m_wireOutput = &output; // Setup static global callback function...
    /* unused - PK_ERROR_code_t failureCode = */
    PK_TOPOL_render_line(1, &entityTag, nullptr, PK_ENTITY_null, &options);
    s_frustrumOutput.m_wireOutput = nullptr; // Clear static global callback function...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   06/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int pki_start_modeler()
    {
    if (s_parasolidInitialized)
        return SUCCESS;

    PK_SESSION_frustrum_t sessionFrustrum;

    PK_SESSION_frustrum_o_m (sessionFrustrum);

    sessionFrustrum.fstart = pki_FSTART;
    sessionFrustrum.fstop  = pki_FSTOP;
    sessionFrustrum.fmallo = pki_FMALLO;
    sessionFrustrum.fmfree = pki_FMFREE;
    sessionFrustrum.ffoprd = pki_FFOPRD; // Don't use, doesn't support unicode filenames...see ucoprd.
    sessionFrustrum.ffopwr = pki_FFOPWR; // Don't use, doesn't support unicode filenames...see ucopwr.
    sessionFrustrum.ucoprd = pki_UCOPRD;
    sessionFrustrum.ucopwr = pki_UCOPWR;
    sessionFrustrum.ffclos = pki_FFCLOS;
    sessionFrustrum.ffread = pki_FFREAD;
    sessionFrustrum.ffwrit = pki_FFWRIT;
    sessionFrustrum.ffoprb = pki_FFOPRB;
    sessionFrustrum.ffseek = pki_FFSEEK;
    sessionFrustrum.fftell = pki_FFTELL;
    sessionFrustrum.gosgmt = pki_GOSGMT; // Used to produce face-iso/hidden line/silhouettes...yucky and not thread safe...
    sessionFrustrum.goopsg = pki_GOOPSG;
    sessionFrustrum.goclsg = pki_GOCLSG;
    sessionFrustrum.gopixl = pki_GOPIXL;
    sessionFrustrum.gooppx = pki_GOOPPX;
    sessionFrustrum.goclpx = pki_GOCLPX;

    PK_SESSION_register_frustrum (&sessionFrustrum);

#if defined (PARTITION_ROLLBACK_REQUIRED)
    pki_partitionRollbackStart (NULL); // Start partitioned rollback using a temporary file
#endif

    int failureCode;
    PK_SESSION_start_o_t sessionOptions;

    PK_SESSION_start_o_m (sessionOptions);

    if (SUCCESS != (failureCode = PK_SESSION_start (&sessionOptions)))
        return failureCode;

    PK_SESSION_set_unicode (PK_LOGICAL_true); // Required for R20 parasolid/acis interop...
    PK_SESSION_set_general_topology (PK_LOGICAL_false); // Setup General Body (Off)
    PK_SESSION_set_check_self_int (PK_LOGICAL_false); // Setup Geometry Self-Intersection Checks (Off)

    pki_mark_start ();

    PK_MEMORY_frustrum_t memoryFunctions;

    memoryFunctions.alloc_fn = malloc;
    memoryFunctions.free_fn = free;

    if (SUCCESS != (failureCode = PK_MEMORY_register_callbacks (memoryFunctions))) // register memory alloc/free functions
        return failureCode;

    PSolidAttrib::CreateEntityIdAttributeDef(true);
    PSolidAttrib::CreateUserDataAttributeDef(true);
    PSolidAttrib::CreateHiddenEntityAttributeDef(true);
    PSolidAttrib::CreateFaceMaterialIndexAttributeDef(true);

    s_parasolidInitialized = 1;

    return failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   06/96
+---------------+---------------+---------------+---------------+---------------+------*/
static int pki_end_modeler()
    {
    s_parasolidInitialized = 0;

#if defined (PARTITION_ROLLBACK_REQUIRED)
    pki_partitionRollbackStop();
#endif

    return PK_SESSION_stop();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidKernelManager::StartSession()
    {
    if (s_usingExternalFrustrum)
        return;

    pki_start_modeler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidKernelManager::StopSession()
    {
    if (s_usingExternalFrustrum)
        return;

    pki_end_modeler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidKernelManager::IsSessionStarted()
    {
    return 0 != s_parasolidInitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidKernelManager::SetExternalFrustrum(bool isActive)
    {
    bool wasActive = s_usingExternalFrustrum;

    s_usingExternalFrustrum = isActive;

    return wasActive;
    }
