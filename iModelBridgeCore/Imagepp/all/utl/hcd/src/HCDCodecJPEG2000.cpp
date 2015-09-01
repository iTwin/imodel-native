//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecJPEG2000.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIJG
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecJPEG2000.h>

#define HCD_CODEC_NAME     L"JPEG2000"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::HCDCodecJPEG2000()
    : HCDCodecErMapperSupported(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::HCDCodecJPEG2000(const HCDCodecJPEG2000& pi_rObj)
    : HCDCodecErMapperSupported(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::~HCDCodecJPEG2000()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecJPEG2000::Clone() const
    {
    return new HCDCodecJPEG2000(*this);
    }




#if 0 //MST DONT DELETE - Developped for the JPEG 2000 codec prototype
//which purposes was to verify the feasibility of supporting
//JPEG 2000 in iTiff. This code would likely be used eventually
//to replace the file creation code from HUTExportToFile.cpp

#include <Imagepp/all/h/HFCBinStream.h>

#define NCSECW_STATIC_LIBS

#include <Imagepp/all/h/NCSFile.h>

static bool IsPrototype2 = true;

class JPEG2000Codec : public CNCSFile
    {
public :

    JPEG2000Codec()
        {
        }
    // WriteReadLine, WriteStatus, and WriteCancel are inherited from the base CNCSFile class,
    // and overriden to perform the compression.

    // WriteReadLine() - called by SDK once for each line read.
    // The parameters passed are the next line number being read from the input,
    // and a pointer to a buffer into which that line's data should be loaded by
    // your code.
    //
    // The cell type of the buffer is the same as that passed into SetFileInfo() in the
    // NCSFileViewFileInfoEx struct.
    virtual CNCSError WriteReadLine(uint32_t nNextLine, void** ppInputArray);

    // Status update function, called for each line output during compression, and therefore
    // useful for updating progress indicators in the interface of an application.
    virtual void WriteStatus(uint32_t nCurrentLine);

    // A cancel function called by the SDK which will cancel the compression if it returns
    // true.  We override it here without actually introducing cancel functionality.
    virtual bool WriteCancel();

    const Byte*        m_pInData;
    size_t              m_InDataSize;
    uint32_t            m_BitsPerPixel;
    uint32_t            m_Width;

    };

class CMyIOStream2: public CNCSJPCIOStream {
public:

    CMyIOStream2() {
        m_pOutBuffer         = 0;
        m_OutBufferSize      = 0;
        m_BufferOffset       = 0;
        m_CompressedDataSize = 0;

        }

    CMyIOStream2(Byte*  pi_pOutBuffer,
                 uint64_t pi_OutBufferSize) {
        m_pOutBuffer         = pi_pOutBuffer;
        m_OutBufferSize      = pi_OutBufferSize;
        m_BufferOffset       = 0;
        m_CompressedDataSize = 0;

        }
    virtual ~CMyIOStream2() {
        }

    void SetOutputBuffer(Byte*  pi_pOutBuffer,
                         uint64_t pi_OutBufferSize)
        {
        m_pOutBuffer         = pi_pOutBuffer;
        m_OutBufferSize      = pi_OutBufferSize;
        m_BufferOffset       = 0;
        m_CompressedDataSize = 0;
        }
    /**
    * Open the stream on the specified file.
    * @param            pName           Full Name of JP2 stream being parsed
    * @param            bWrite          Open for writing flag.
    * @return      CNCSError    NCS_SUCCESS, or error code on failure.
    */
    virtual CNCSError Open(char* pName, bool bWrite = false) {
        /*    if(!bWrite) {
                m_pFile = fopen(pName, "rb");
            } else {
                m_pFile = fopen(pName, "w+b");
            }

            if(m_pFile) {
                return(CNCSJPCIOStream::Open(pName, bWrite));
            }
            return(NCS_FILE_OPEN_FAILED);
            return NCS_SUCCESS;
        */
        return(CNCSJPCIOStream::Open(pName, bWrite));
        };
    /**
    * Close the stream.
    * @return      CNCSError    NCS_SUCCESS, or error code on failure.
    */
    virtual CNCSError Close() {
        /*if(m_pFile) {
            fclose(m_pFile);
            m_pFile = NULL;
        }*/
        return(CNCSJPCIOStream::Close());
        }

    /**
    * Seek on the file to the specified location.
    * @param            offset          Signed 64bit offset to seek by
    * @param            origin          Origin to calculate new position from.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Seek(INT64 offset, Origin origin = CURRENT) {
        switch(origin) {
            case CNCSJPCIOStream::START:
                m_BufferOffset = offset;
                break;
            case CNCSJPCIOStream::CURRENT:
                m_BufferOffset += offset;
                break;
            case CNCSJPCIOStream::END:
                m_BufferOffset = m_CompressedDataSize;
                break;
            }
        return(true);
        }
    /**
    * Is stream seekable.
    * @return      bool         true if seekable, or false if not.
    */
    virtual bool NCS_FASTCALL Seek() {
        return(true);
        }
    /**
    * Get the current file pointer position.
    * @return      INT64                Current file pointer position, or -1 on error.
    */
    virtual INT64 NCS_FASTCALL Tell() {
        return m_BufferOffset;
        }
    /**
    * Get the total current size of the file, in bytes.
    * @return      INT64                Size of the file, or -1 on error.
    */
    virtual INT64 NCS_FASTCALL Size() {
        return (INT64)m_CompressedDataSize;
        /*
                struct stat buf;
        #ifdef WIN32
                if(fstat(_fileno(m_pFile), &buf) == 0) {
        #else
                if(fstat(fileno(m_pFile), &buf) == 0) {
        #endif
                    return(buf.st_size);
                }
                return(-1);*/
        }

    /**
    * Read some data from the stream into the supplied buffer.
    * @param            buffer          Buffer to read the data into
    * @param            count           How many bytes of data to read.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Read(void* buffer, uint32_t count) {

        HPRECONDITION(m_BufferOffset + count <= m_OutBufferSize);

        if (count == 0) return true;
        errno_t ErrCode = BeStringUtilities::Memcpy(buffer, count, (m_pOutBuffer + m_BufferOffset), count);

        //No error occurred
        if (ErrCode == 0)
            {
            m_BufferOffset += count;
            return true;
            }

        return false;
        }
    /**
    * Write some data to the stream.
    * @param            buffer          Buffer of data to write to the stream
    * @param            count           How many bytes of data to write to the stream.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Write(void* buffer, uint32_t count) {

        HPRECONDITION(m_BufferOffset + count <= m_OutBufferSize);

        if (count == 0) return true;

        errno_t ErrCode = BeStringUtilities::Memcpy(m_pOutBuffer + m_BufferOffset,
                                   count,
                                   buffer,
                                   count);
        //No error occurred
        if (ErrCode == 0)
            {
            m_BufferOffset += count;

            if (m_BufferOffset > m_CompressedDataSize)
                {
                m_CompressedDataSize = m_BufferOffset;
                }
            return true;
            }

        return false;
        }

    uint64_t GetCompressedDataSize()
        {
        return m_CompressedDataSize;
        }


    void SetCompressedDataSize(uint64_t pi_CompressedDataSize)
        {
        m_CompressedDataSize = pi_CompressedDataSize;
        }


private:
    Byte*  m_pOutBuffer;
    uint64_t m_BufferOffset;
    uint64_t m_CompressedDataSize;
    uint64_t m_OutBufferSize;
    };



class CMyIOFileStream: public CNCSJPCIOStream {
public:

    CMyIOFileStream(HFCBinStream* pi_pTiffStream,
                    uint64_t     pi_Jp2StreamOffset,
                    uint64_t     pi_CompressedDataSize) {

        m_pTiffStream        = pi_pTiffStream;
        m_Jp2StreamOffset    = pi_Jp2StreamOffset;
        m_CompressedDataSize = pi_CompressedDataSize;
        }
    virtual ~CMyIOFileStream() {
        }

    /**
    * Open the stream on the specified file.
    * @param            pName           Full Name of JP2 stream being parsed
    * @param            bWrite          Open for writing flag.
    * @return      CNCSError    NCS_SUCCESS, or error code on failure.
    */
    virtual CNCSError Open(char* pName, bool bWrite = false) {

        /*    if(!bWrite) {
        m_pFile = fopen(pName, "rb");
        } else {
        m_pFile = fopen(pName, "w+b");
        }

        if(m_pFile) {
        return(CNCSJPCIOStream::Open(pName, bWrite));
        }
        return(NCS_FILE_OPEN_FAILED);
        return NCS_SUCCESS;
        */
        return(CNCSJPCIOStream::Open(pName, bWrite));
        };
    /**
    * Close the stream.
    * @return      CNCSError    NCS_SUCCESS, or error code on failure.
    */
    virtual CNCSError Close() {
        /*if(m_pFile) {
        fclose(m_pFile);
        m_pFile = NULL;
        }*/
        return(CNCSJPCIOStream::Close());
        }

    /**
    * Seek on the file to the specified location.
    * @param            offset          Signed 64bit offset to seek by
    * @param            origin          Origin to calculate new position from.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Seek(INT64 offset, Origin origin = CURRENT) {
        switch(origin) {
            case CNCSJPCIOStream::START:
                m_pTiffStream->SeekToPos(m_Jp2StreamOffset + offset);
                break;
            case CNCSJPCIOStream::CURRENT:
                m_pTiffStream->Seek(offset);
                break;
            case CNCSJPCIOStream::END:
                m_pTiffStream->SeekToEnd();
                break;
            }
        return(true);
        }
    /**
    * Is stream seekable.
    * @return      bool         true if seekable, or false if not.
    */
    virtual bool NCS_FASTCALL Seek() {
        return(true);
        }
    /**
    * Is stream seekable.
    * @return      bool         true if seekable, or false if not.
    */
    virtual void NCS_FASTCALL SeekToJP2Stream() {
        m_pTiffStream->SeekToPos(m_Jp2StreamOffset);
        }
    /**
    * Get the current file pointer position.
    * @return      INT64                Current file pointer position, or -1 on error.
    */
    virtual INT64 NCS_FASTCALL Tell() {
        return m_pTiffStream->GetCurrentPos() - m_Jp2StreamOffset;
        }
    /**
    * Get the total current size of the file, in bytes.
    * @return      INT64                Size of the file, or -1 on error.
    */
    virtual INT64 NCS_FASTCALL Size() {
        return (INT64)m_CompressedDataSize;
        /*
        struct stat buf;
        #ifdef WIN32
        if(fstat(_fileno(m_pFile), &buf) == 0) {
        #else
        if(fstat(fileno(m_pFile), &buf) == 0) {
        #endif
        return(buf.st_size);
        }
        return(-1);*/
        }

    /**
    * Read some data from the stream into the supplied buffer.
    * @param            buffer          Buffer to read the data into
    * @param            count           How many bytes of data to read.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Read(void* buffer, uint32_t count) {

        HPRECONDITION(m_Jp2StreamOffset + count <= m_CompressedDataSize);

        if (count == 0) return true;

        size_t NbBytesRead = m_pTiffStream->Read(buffer, count);

        //No error occurred
        if (NbBytesRead == count)
            {
            return true;
            }

        return false;
        }
    /**
    * Write some data to the stream.
    * @param            buffer          Buffer of data to write to the stream
    * @param            count           How many bytes of data to write to the stream.
    * @return      bool         true, or false on failure.  Instance inherits CNCSError object containing error value.
    */
    virtual bool NCS_FASTCALL Write(void* buffer, uint32_t count) {
        HASSERT(0);
        return false;
        }

private:
    HFCBinStream*  m_pTiffStream;
    uint64_t      m_Jp2StreamOffset;
    uint64_t      m_CompressedDataSize;
    };

HPM_REGISTER_CLASS(HCDCodecJPEG2000, HCDCodecErMapperSupported)

#define HCD_CODEC_NAME L"JPEG2000"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::HCDCodecJPEG2000()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    try
        {
        InitObject();

        static bool IsInit = false;

        if (!IsInit)
            {
            NCSecwInit();
            IsInit = true;
            }
        }
    catch(...)
        {
        // DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::HCDCodecJPEG2000(uint32_t pi_Width,
                                   uint32_t pi_Height,
                                   uint32_t pi_BitsPerPixel)
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    try
        {
        InitObject();
        SetDimensions(pi_Width, pi_Height);
        SetBitsPerPixel(pi_BitsPerPixel);
        SetSubset(pi_Width, pi_Height);
        }
    catch(...)
        {
        //     DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::HCDCodecJPEG2000(const HCDCodecJPEG2000& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    try
        {
        m_pStream = pi_rObj.m_pStream;
        m_pCompressedStream = pi_rObj.m_pCompressedStream;
        m_pCodec = pi_rObj.m_pCodec;
        m_CellType = pi_rObj.m_CellType;
        m_NbBands = pi_rObj.m_NbBands;
        m_pJP2Stream = pi_rObj.m_pJP2Stream;
        m_JP2StreamOffset = pi_rObj.m_JP2StreamOffset;
        m_StripInd = pi_rObj.m_StripInd;
        m_JP2StreamSize = pi_rObj.m_JP2StreamSize;
        m_StripHeight = pi_rObj.m_StripHeight;

        //   DeepCopy(pi_rObj);
        }
    catch(...)
        {
        //   DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::~HCDCodecJPEG2000()
    {
//   DeepDelete();
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HCDCodecJPEG2000::Clone() const
    {
    return new HCDCodecJPEG2000(*this);
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecJPEG2000::IsBitsPerPixelSupported(uint32_t pi_Bits) const
    {
    //if((pi_Bits % 8) == 0)
    return pi_Bits == 8;
    }

//-----------------------------------------------------------------------------
// public
// OpenJP2Stream
//-----------------------------------------------------------------------------
void HCDCodecJPEG2000::OpenJP2Stream()
    {
    m_pCodec = new JPEG2000Codec;

    NCSFileViewFileInfoEx Info;
    //NCSFileViewFileInfoEx
    memset(&Info, 0, sizeof(NCSFileViewFileInfoEx));

    Info.fOriginX = 0;
    Info.fOriginY = 0;
    Info.fCellIncrementX = 1;
    Info.fCellIncrementY = 1;
    Info.nSizeX = GetWidth();
    Info.nSizeY = m_ResHeight;
    Info.szDatum      = "RAW";
    Info.szProjection = "RAW";

    static UINT16 CompressionRate = 10;
    Info.nCompressionRate = CompressionRate;

    ColorModes ColorMode = GetColorMode();

    switch (GetColorMode())
        {
        case YCC :
            Info. eColorSpace = NCSCS_YCbCr;
            Info.nBands = 3;
            Info.pBands = new NCSFileBandInfo[Info.nBands];
            Info.pBands->bSigned = false;
            Info.pBands->nBits = 8;
            break;
        case RGB :
            Info.eColorSpace = NCSCS_sRGB;
            Info.nBands = 3;
            Info.pBands = new NCSFileBandInfo[Info.nBands];
            for (INT32 nBand = 0; nBand < Info.nBands; nBand++)
                {
                Info.pBands[nBand].bSigned = false;
                Info.pBands[nBand].nBits = 8;
                }

            Info.pBands[0].szDesc = "Red";
            Info.pBands[1].szDesc = "Green";
            Info.pBands[2].szDesc = "Blue";
            break;
        case GRAYSCALE :
            Info.eColorSpace = NCSCS_GREYSCALE;
            Info.nBands = 1;
            Info.pBands = new NCSFileBandInfo[Info.nBands];
            Info.pBands->bSigned = false;
            Info.pBands->nBits = 8;
            Info.pBands->szDesc = "Gray";
            break;
        default :
            HASSERT(0);
        }

    unsigned short BitsPerPixelPerBand = (unsigned short)GetBitsPerPixel() / Info.nBands;

    switch (BitsPerPixelPerBand)
        {
        case 8 :
            Info.eCellType = NCSCT_UINT8;
            break;
        case 16 :
            Info.eCellType = NCSCT_UINT16;
            break;
        case 32 :
            Info.eCellType = NCSCT_UINT32;
            break;
        case 64 :
            Info.eCellType = NCSCT_UINT64;
            break;
        }

    m_CellType = Info.eCellType;
    m_NbBands = Info.nBands;

    Info.eCellSizeUnits = ECW_CELL_UNITS_METERS;

    m_pCodec->SetFileInfo(Info);

    //Open the output file.  The second parameter is whether we are opening
    //in progressive read mode (no) and the third is whether we want to write
    //a new JPEG 2000 or ECW file (yes).
    CNCSError Error = ((CNCSJP2FileView*)(m_pCodec.get()))->Open(m_pStream.get());

    if (Error != NCS_SUCCESS)
        {
        //Create the output.

        }
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJPEG2000::CompressSubset(const void* pi_pInData,
                                        size_t      pi_InDataSize,
                                        void*       po_pOutBuffer,
                                        size_t      po_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(po_OutBufferSize < INT_MAX);

    if (IsPrototype2)
        {
        if (m_pStream == 0)
            {
            m_pStream = new CMyIOStream2((Byte*)po_pOutBuffer,
                                         po_OutBufferSize);
            CNCSError Error;

            Error = m_pStream->Open("out.jpc", true);
            HASSERT(Error == NCS_SUCCESS);
            }
        else
            {
            m_pStream->SetOutputBuffer((Byte*)po_pOutBuffer,
                                       po_OutBufferSize);
            }

        if (m_pCodec == 0)
            {
            OpenJP2Stream();
            }

        uint32_t BytesPerLine = (uint32_t)ceil(GetBitsPerPixel() / 8.0) * GetSubsetWidth();
        void* pInData = pi_pInData;
        CNCSError Error;

        HASSERT(BytesPerLine != 0);

        uint32_t StripHeight = (uint32_t)pi_InDataSize / BytesPerLine;


        if (m_ColorMode == GRAYSCALE)
            {
            for (unsigned int LineInd = 0; LineInd < StripHeight; LineInd++)
                {
                Error = m_pCodec->WriteLineBIL((NCSEcwCellType)m_CellType,
                                               m_NbBands,
                                               &pInData);

                pInData = ((Byte*)pInData + BytesPerLine);
                }
            }
        else
            {
            Byte*          pInputBufferPtr;
            HAutoPtr<Byte> pInputBuffer;
            const Byte*    pDataBuffer = 0;
            uint64_t       LineOffset = 0;

            pInputBuffer = new Byte[BytesPerLine];

            pInputBufferPtr = pInputBuffer.get();

            for (unsigned int LineInd = 0; LineInd < StripHeight; LineInd++)
                {
                LineOffset = (BytesPerLine * LineInd);

                for (int BandInd = 0; BandInd < 3; BandInd++)
                    {
                    pDataBuffer = ((Byte*)pi_pInData) + LineOffset + BandInd;
                    for (unsigned int PixelIndex = 0; PixelIndex < GetWidth(); PixelIndex++)
                        {
                        *pInputBufferPtr = *pDataBuffer;
                        pInputBufferPtr++;
                        pDataBuffer += 3;
                        }
                    }

                pInputBufferPtr = pInputBuffer.get();

                Error = m_pCodec->WriteLineBIL((NCSEcwCellType)m_CellType,
                                               m_NbBands,
                                               (void**)&pInputBufferPtr);

                HASSERT(Error == NCS_SUCCESS);
                }
            }

        return (size_t)m_pStream->GetCompressedDataSize();
        }
    else
        {
        JPEG2000Codec Codec;

        Codec.m_InDataSize = pi_InDataSize;
        Codec.m_pInData = (const Byte*)pi_pInData;
        Codec.m_Width = GetSubsetWidth();
        Codec.m_BitsPerPixel = GetBitsPerPixel();
        /*
            m_pInData = (const Byte*)pi_pInData;
            m_InDataSize = pi_InDataSize;
        */
        NCSFileViewFileInfoEx Info;
        //NCSFileViewFileInfoEx
        memset(&Info, 0, sizeof(NCSFileViewFileInfoEx));

        Info.fOriginX = 0;
        Info.fOriginY = 0;
        Info.fCellIncrementX = 1;
        Info.fCellIncrementY = 1;
        Info.nSizeX = GetSubsetWidth();
        Info.nSizeY = GetSubsetHeight();
        Info.szDatum      = "RAW";
        Info.szProjection = "RAW";

        static UINT16 CompressionRate = 10;
        Info.nCompressionRate = CompressionRate;

        ColorModes ColorMode = GetColorMode();

        switch (GetColorMode())
            {
            case YCC :
                Info. eColorSpace = NCSCS_YCbCr;
                Info.nBands = 3;
                Info.pBands = new NCSFileBandInfo[Info.nBands];
                Info.pBands->bSigned = false;
                Info.pBands->nBits = 8;
                break;
            case RGB :
                Info.eColorSpace = NCSCS_sRGB;
                Info.nBands = 3;
                Info.pBands = new NCSFileBandInfo[Info.nBands];
                for (INT32 nBand = 0; nBand < Info.nBands; nBand++)
                    {
                    Info.pBands[nBand].bSigned = false;
                    Info.pBands[nBand].nBits = 8;
                    }

                Info.pBands[0].szDesc = "Red";
                Info.pBands[1].szDesc = "Green";
                Info.pBands[2].szDesc = "Blue";
                break;
            case GRAYSCALE :
                Info.eColorSpace = NCSCS_GREYSCALE;
                Info.nBands = 1;
                Info.pBands = new NCSFileBandInfo[Info.nBands];
                Info.pBands->bSigned = false;
                Info.pBands->nBits = 8;
                Info.pBands->szDesc = "Gray";
                break;
            default :
                HASSERT(0);
            }

        unsigned short BitsPerPixelPerBand = (unsigned short)GetBitsPerPixel() / Info.nBands;

        switch (BitsPerPixelPerBand)
            {
            case 8 :
                Info.eCellType = NCSCT_UINT8;
                break;
            case 16 :
                Info.eCellType = NCSCT_UINT16;
                break;
            case 32 :
                Info.eCellType = NCSCT_UINT32;
                break;
            case 64 :
                Info.eCellType = NCSCT_UINT64;
                break;
            }

        Info.eCellSizeUnits = ECW_CELL_UNITS_METERS;

        CMyIOStream2 IOStream((Byte*)po_pOutBuffer, po_OutBufferSize);
        CNCSError    Error;

        Error = IOStream.Open("out.jpc", true);
        HASSERT(Error == NCS_SUCCESS);

        Codec.SetFileInfo(Info);

        //Open the output file.  The second parameter is whether we are opening
        //in progressive read mode (no) and the third is whether we want to write
        //a new JPEG 2000 or ECW file (yes).
        Error = ((CNCSJP2FileView*)(&Codec))->Open(&IOStream);

        if (Error == NCS_SUCCESS)
            {
            //Create the output.  Will return NCS_USER_CANCELLED_COMPRESSION if WriteCancel()
            //returns true, another error code if something goes wrong, or NCS_SUCCESS.

            static bool Test = false;

            if (Test == true)
                {
                Error = Codec.Write();
                }
            else
                {
                uint32_t BytesPerLine = (uint32_t)ceil(GetBitsPerPixel() / 8.0) * GetSubsetWidth();
                void* pInData = pi_pInData;

                for (unsigned int LineInd = 0; LineInd < GetSubsetHeight(); LineInd++)
                    {
                    Error = Codec.WriteLineBIL(Info.eCellType,
                                               Info.nBands,
                                               &pInData);

                    pInData = ((Byte*)pInData + BytesPerLine);
                    }
                }

            if (Error == NCS_SUCCESS)
                fprintf(stdout,"Finished compression\n");
            else if (Error == NCS_USER_CANCELLED_COMPRESSION)
                fprintf(stdout,"Compression cancelled\n");
            else fprintf(stdout,"Error during compression: %s\n",Error.GetErrorMessage());
            fflush(stdout);
            Error = Codec.Close(true);
            }
        return (size_t)IOStream.GetCompressedDataSize();
        }

    }

// This is called once for each output line.
// In this example, we can simply call ReadLineBIL on m_Src.
//
    CNCSError JPEG2000Codec::WriteReadLine(uint32_t nNextLine, void** ppInputArray)
    {
    static bool RGB = true;

    if (RGB == true)
        {
        Byte* pInputBuffer = 0;
        const Byte* pDataBuffer = 0;
        uint32_t BytesPerLine = (uint32_t)ceil(m_BitsPerPixel / 8.0) * m_Width;
        uint64_t LineIndex = (BytesPerLine * nNextLine);

        for (int BandInd = 0; BandInd < 3; BandInd++)
            {
            pInputBuffer = (Byte*)ppInputArray[BandInd];
            pDataBuffer = m_pInData + LineIndex + BandInd;
            for (unsigned int PixelIndex = 0; PixelIndex < m_Width; PixelIndex++)
                {
                *pInputBuffer = *pDataBuffer;
                pInputBuffer++;
                pDataBuffer += 3;
                }
            }
        }
    else
        {
        uint32_t BytesPerLine = (uint32_t)ceil(m_BitsPerPixel / 8.0) * m_Width;
        memcpy(*ppInputArray, m_pInData + (BytesPerLine * nNextLine), BytesPerLine);
        }
    return NCS_SUCCESS;
    }

// Status update.  In this example, we write the percentage complete to the standard
// output at each 1% increment.
//
// A GUI app would run a progress/status bar from this, but would normally only
// update the GUI every now and then since this could be called _many_ times on a large output.
// The correct approach would be to schedule a fixed number of updates and call them at even
// intervals based on the output file size.
//
    void JPEG2000Codec::WriteStatus(uint32_t nCurrentLine)
    {
    /*
    NCSFileViewFileInfoEx *pInfo = GetFileInfo();

    uint32_t nPercentComplete = (uint32_t)((nCurrentLine * 100)/(pInfo->nSizeY));
    if (nPercentComplete > m_nPercentComplete)
    {
        m_nPercentComplete = nPercentComplete;
        fprintf(stdout, "Completed %d%%\n", m_nPercentComplete);
        fflush(stdout);
    }*/
    }

//Cancel check.  We always return false, but an application with a graphical user interface
//could have a "Cancel" button that when pressed caused this to return true and halt the
//compression process.
bool JPEG2000Codec::WriteCancel(void)
    {
    // Return true to cancel compression - eg from a GUI cancel button.
    return false;

    }

//-----------------------------------------------------------------------------
// public
// SetJP2Stream
//-----------------------------------------------------------------------------
void HCDCodecJPEG2000::SetJP2Stream(HFCBinStream* pi_pTiffFile,
                                    uint32_t      pi_JP2StreamOffset,
                                    uint64_t&      pi_rStreamSize)
    {
    m_pJP2Stream      = pi_pTiffFile;
    m_JP2StreamOffset = pi_JP2StreamOffset;
    m_JP2StreamSize   = pi_rStreamSize;
    }

//-----------------------------------------------------------------------------
// public
// SetStrip
//-----------------------------------------------------------------------------
void HCDCodecJPEG2000::SetStripInd(uint32_t pi_StripInd)
    {
    m_StripInd = pi_StripInd;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJPEG2000::DecompressSubset(const void*  pi_pInData,
                                          size_t pi_InDataSize,
                                          void*  po_pOutBuffer,
                                          size_t pi_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(pi_OutBufferSize < INT_MAX);

    if (IsPrototype2)
        {
        CNCSError Error;

        if (m_pCompressedStream == 0)
            {
            HASSERT(m_pJP2Stream != 0);
            HASSERT(m_JP2StreamSize != 0);

            m_pCompressedStream = new CMyIOFileStream(m_pJP2Stream,
                                                      m_JP2StreamOffset,
                                                      m_JP2StreamSize);

            Error = m_pCompressedStream->Open("out.jpc");
            HASSERT(Error == NCS_SUCCESS);
            }

        if (m_pCodec == 0)
            {
            m_pCodec = new JPEG2000Codec;

            //Open the output file.  The second parameter is whether we are opening
            //in progressive read mode (no) and the third is whether we want to write
            //a new JPEG 2000 or ECW file (yes).
            m_pCompressedStream->SeekToJP2Stream();

            Error = ((CNCSJP2FileView*)(m_pCodec.get()))->Open(m_pCompressedStream.get());
            HASSERT(Error == NCS_SUCCESS);

            }

        size_t NbBytesWritten       = 0;

        if (m_pCodec != 0)
            {
            m_pCompressedStream->SeekToJP2Stream();

            uint32_t BytesPerLine = (uint32_t)ceil(GetBitsPerPixel() / 8.0) * GetWidth();


            NCSFileViewFileInfoEx* pInfo = m_pCodec->GetFileInfo();

            INT32* Bands = new INT32[pInfo->nBands];
            for (INT32 nBand = 0; nBand < pInfo->nBands; nBand++)
                Bands[nBand] = nBand;

            uint32_t nWidth, nHeight;
            IEEE8 fStartX, fStartY, fEndX, fEndY;

            HASSERT((pInfo->fCellIncrementX == 1) && (pInfo->fCellIncrementY == 1));

            nWidth = pInfo->nSizeX;
            nHeight = GetHeight();
            fStartX = pInfo->fOriginX;
            fStartY = m_StripInd * m_StripHeight;
            fEndX = fStartX + nWidth;
            fEndY = fStartY + nHeight;

            Error = m_pCodec->SetView(pInfo->nBands, Bands,
                                      nWidth, nHeight,
                                      fStartX,
                                      fStartY,
                                      fEndX,
                                      fEndY);

            NCSEcwReadStatus ReadStatus;
            UINT8* pOutBufferPointer = (UINT8*)po_pOutBuffer;

            for (uint32_t LineInd = 0; LineInd < GetSubsetHeight(); LineInd++)
                {
                if (pInfo->eColorSpace == NCSCS_GREYSCALE)
                    {
                    ReadStatus = m_pCodec->ReadLineBIL((UINT8**)&pOutBufferPointer);
                    }
                else
                    {
                    if (true)
                        {
                        ReadStatus = m_pCodec->ReadLineRGB(pOutBufferPointer);
                        //break;
                        }
                    else
                        {
                        Byte** pOutBuffer = new Byte*[pInfo->nBands];
                        uint32_t  LineSteps = pInfo->nBands;

                        pOutBuffer[0] = (UINT8*)pOutBufferPointer;
                        pOutBuffer[1] = ((UINT8*)pOutBufferPointer + 1);
                        pOutBuffer[2] = ((UINT8*)pOutBufferPointer + 2);

                        ReadStatus = m_pCodec->ReadLineBIL(pInfo->eCellType,
                                                           pInfo->nBands,
                                                           (void**)pOutBuffer,
                                                           &LineSteps);
                        break;
                        }
                    }

                if (ReadStatus != NCSECW_READ_OK)
                    {
                    break;
                    }

                pOutBufferPointer += BytesPerLine;
                NbBytesWritten += BytesPerLine;
                }
            }

        return NbBytesWritten;
        }
    else
        {
        CMyIOStream2 IOStream((Byte*)pi_pInData, pi_InDataSize);
        IOStream.SetCompressedDataSize(pi_InDataSize);


        CNCSError   Error = NCS_SUCCESS;
        uint32_t    BytesPerLine = (uint32_t)ceil(GetBitsPerPixel() / 8.0) * GetSubsetWidth();

        JPEG2000Codec Codec;


        static bool tttt = true;

        Codec.m_InDataSize = pi_InDataSize;
        Codec.m_pInData = (const Byte*)pi_pInData;
        Codec.m_Width = GetSubsetWidth();

        Error = IOStream.Open("out.jpc");
        HASSERT(Error == NCS_SUCCESS);

        //Open the output file.  The second parameter is whether we are opening
        //in progressive read mode (no) and the third is whether we want to write
        //a new JPEG 2000 or ECW file (yes).
        Error = ((CNCSJP2FileView*)(&Codec))->Open(&IOStream);

        size_t NbBytesWritten       = 0;

        if (Error == NCS_SUCCESS)
            {
            NCSFileViewFileInfoEx* pInfo = Codec.GetFileInfo();

            INT32* Bands = new INT32[pInfo->nBands];
            for (INT32 nBand = 0; nBand < pInfo->nBands; nBand++)
                Bands[nBand] = nBand;

            uint32_t nWidth, nHeight;
            IEEE8 fStartX, fStartY, fEndX, fEndY;

            nWidth = pInfo->nSizeX;
            nHeight = pInfo->nSizeY;
            fStartX = pInfo->fOriginX;
            fStartY = pInfo->fOriginY;
            fEndX = fStartX + pInfo->fCellIncrementX * nWidth;
            fEndY = fStartY + pInfo->fCellIncrementY * nHeight;

            Error = Codec.SetView(pInfo->nBands, Bands,
                                  nWidth, nHeight,
                                  fStartX,
                                  fStartY,
                                  fEndX,
                                  fEndY);

            NCSEcwReadStatus ReadStatus;
            UINT8* pOutBufferPointer = (UINT8*)po_pOutBuffer;

            for (uint32_t LineInd = 0; LineInd < GetSubsetHeight(); LineInd++)
                {
                if (pInfo->eColorSpace == NCSCS_GREYSCALE)
                    {
                    ReadStatus = Codec.ReadLineBIL((UINT8**)&pOutBufferPointer);
                    }
                else
                    {
                    if (tttt)
                        {
                        ReadStatus = Codec.ReadLineRGB(pOutBufferPointer);
                        //break;
                        }
                    else
                        {
                        Byte** pOutBuffer = new Byte*[pInfo->nBands];
                        uint32_t  LineSteps = pInfo->nBands;

                        pOutBuffer[0] = (UINT8*)pOutBufferPointer;
                        pOutBuffer[1] = ((UINT8*)pOutBufferPointer + 1);
                        pOutBuffer[2] = ((UINT8*)pOutBufferPointer + 2);

                        ReadStatus = Codec.ReadLineBIL(pInfo->eCellType,
                                                       pInfo->nBands,
                                                       (void**)pOutBuffer,
                                                       &LineSteps);
                        break;
                        }
                    }

                if (ReadStatus != NCSECW_READ_OK)
                    {
                    break;
                    }

                pOutBufferPointer += BytesPerLine;
                NbBytesWritten += BytesPerLine;
                }
            }
        return NbBytesWritten;
        }

    return 0;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecJPEG2000::HasLineAccess() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t HCDCodecJPEG2000::GetMinimumSubsetSize() const
    {
    size_t ImageWidthInBytes = (GetWidth() * GetBitsPerPixel() + GetLinePaddingBits() + 7) / 8;
    size_t MaxLineCount = (1024*1024) / ImageWidthInBytes;  // 1 meg is big enough

    return (GetHeight() < MaxLineCount)
           ? (ImageWidthInBytes * GetHeight())
           : (ImageWidthInBytes * MaxLineCount);
    }


//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void HCDCodecJPEG2000::InitObject()
    {
    /*
    m_ExternalQuantizationTablesUse = false;

    // Verify if we can use ijl.
    // --> Can't load ijl.dll for example...
    if (s_UseIJL && ijl_IsLoaded())
        m_IJLInMemory   = true;
    else
    {
        s_UseIJL        = false;
        m_IJLInMemory   = false;
    }


    /*
    ////////////////////////////////////////
    // Set a custom error handler
    ////////////////////////////////////////

    // Set custom error handlers in the compression/decompression
    // structure of the m_Jpeg member.  This is done to avoid using
    // the default handlers which use exit() when an error occurs

    // IJL-specific init

    if (m_IJLInMemory)
    {
        m_pIJLInfoEnc = new JPEG_CORE_PROPERTIES;
        m_pIJLInfoDec = new JPEG_CORE_PROPERTIES;
        ijlInit(m_pIJLInfoEnc);
        ijlInit(m_pIJLInfoDec);
        m_pIJLInfoEnc->DIBWidth = GetWidth();
        m_pIJLInfoEnc->DIBHeight = GetHeight();
        m_pIJLInfoEnc->JPGWidth = GetWidth();
        m_pIJLInfoEnc->JPGHeight = GetHeight();
    }

    // set the jpeg error manager to a standard
    cinfodec->err  = jpeg_std_error(m_ErrorManager.pub);
    cinfocomp->err = jpeg_std_error(m_ErrorManager.pub);

    m_ErrorManager.pub->error_exit = HCDJpegErrorExit;
        jpeg_create_compress(cinfocomp);

        jpeg_mem_dest(  cinfocomp,
                    (Byte*)0,
                    0);

        jpeg_create_decompress(cinfodec);

    jpeg_mem_src(   cinfodec,
                    (Byte*)0,
                    0);

    // Default color mode for stored pixels.
    m_StoredColorMode = YCC;

    if(GetBitsPerPixel() == 8)
    {
        SetColorMode(GRAYSCALE);
        SetSubsamplingMode(SNONE);
    }
    else if (GetBitsPerPixel() == 32)
    {
        SetColorMode(RGBA);
        SetSubsamplingMode(S411);
    }
    else
    {
        SetColorMode(RGB);
        SetSubsamplingMode(S411);
    } */
    }


//-----------------------------------------------------------------------------
// public
// SetColorMode
//-----------------------------------------------------------------------------
void HCDCodecJPEG2000::SetColorMode(ColorModes pi_Mode)
    {
    m_ColorMode = pi_Mode;
    /*
    if(m_ColorMode == YCC)
    {
    m_UseIJLRead = false;
    m_UseIJLWrite = false;

    cinfocomp->in_color_space = JCS_YCbCr;
    cinfodec->out_color_space = JCS_YCbCr;
    }
    else if(m_ColorMode == RGB)
    {
    if (m_IJLInMemory)
    {
    m_pIJLInfoEnc->DIBColor = IJL_RGB;
    m_pIJLInfoDec->DIBColor = IJL_RGB;
    m_pIJLInfoEnc->DIBChannels = 3;
    m_pIJLInfoDec->DIBChannels = 3;
    m_pIJLInfoEnc->JPGColor = IJL_YCBCR;
    m_pIJLInfoEnc->JPGChannels = 3;
    }

    cinfocomp->in_color_space = JCS_RGB;
    cinfodec->out_color_space = JCS_RGB;
    }
    else if(m_ColorMode == BGR)
    {
    if (m_IJLInMemory)
    {
    m_pIJLInfoEnc->DIBColor = IJL_BGR;
    m_pIJLInfoDec->DIBColor = IJL_BGR;
    m_pIJLInfoEnc->DIBChannels = 3;
    m_pIJLInfoDec->DIBChannels = 3;
    m_pIJLInfoEnc->JPGColor = IJL_YCBCR;
    m_pIJLInfoEnc->JPGChannels = 3;
    }

    cinfocomp->in_color_space = JCS_UNKNOWN;
    cinfodec->out_color_space = JCS_UNKNOWN;
    }
    else if(m_ColorMode == GRAYSCALE)
    {
    if (m_IJLInMemory)
    {
    m_pIJLInfoEnc->DIBColor = IJL_G;
    m_pIJLInfoDec->DIBColor = IJL_G;
    m_pIJLInfoEnc->DIBChannels = 1;
    m_pIJLInfoDec->DIBChannels = 1;
    m_pIJLInfoEnc->JPGColor = IJL_G;
    m_pIJLInfoEnc->JPGChannels = 1;
    }
    cinfocomp->in_color_space = JCS_GRAYSCALE;
    cinfodec->out_color_space = JCS_GRAYSCALE;
    }
    else if(m_ColorMode == CMYK)
    {
    m_UseIJLRead = false;
    m_UseIJLWrite = false;

    cinfocomp->in_color_space = JCS_CMYK;
    cinfodec->out_color_space = JCS_CMYK;
    }
    else if(m_ColorMode == RGBA)
    {
    if (m_IJLInMemory)
    {
    m_pIJLInfoEnc->DIBColor = IJL_RGBA_FPX;
    m_pIJLInfoDec->DIBColor = IJL_RGBA_FPX;
    m_pIJLInfoEnc->DIBChannels = 4;
    m_pIJLInfoDec->DIBChannels = 4;
    m_pIJLInfoEnc->JPGColor = IJL_YCBCRA_FPX;
    m_pIJLInfoEnc->JPGChannels = 4;
    }

    cinfocomp->in_color_space = JCS_UNKNOWN;
    cinfodec->out_color_space = JCS_UNKNOWN;
    }
    else if(m_ColorMode == UNKNOWN)
    {
    m_UseIJLRead = false;
    m_UseIJLWrite = false;

    cinfocomp->in_color_space = JCS_UNKNOWN;
    cinfodec->out_color_space = JCS_UNKNOWN;
    }

    cinfocomp->input_components = GetBitsPerPixel() / 8;
    cinfodec->output_components = GetBitsPerPixel() / 8;

    jpeg_set_defaults(cinfocomp);

    // !!!! HChkSebG !!!!
    // Must take care if jpeg data source has been stored as RGB (should'nt but happen..) or YCbCr
    // wich is standard.
    if (m_StoredColorMode == RGB)
    jpeg_set_colorspace(cinfocomp, JCS_RGB);

    if (m_ColorMode == GRAYSCALE)
    SetSubsamplingMode(SNONE);*/
    }

//-----------------------------------------------------------------------------
// public
// GetColorMode
//-----------------------------------------------------------------------------
HCDCodecJPEG2000::ColorModes HCDCodecJPEG2000::GetColorMode() const
    {
    return (ColorModes)m_ColorMode;
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HCDCodecIJG::DeepCopy(const HCDCodecIJG& pi_rObj)
    {
    InitObject();

    m_StoredColorMode = pi_rObj.GetSourceColorMode();

    SetAbbreviateMode(pi_rObj.m_AbbreviateMode);
    SetQuality(pi_rObj.m_Quality);
    SetProgressiveMode(pi_rObj.m_ProgressiveMode);
    SetColorMode((ColorModes)(pi_rObj.m_ColorMode));
    SetOptimizeCoding(pi_rObj.m_OptimizeCoding);

    // copy the decompressor info
    if(pi_rObj.m_HeaderSize != 0)
        {
        ReadHeader(pi_rObj.m_pHeader, pi_rObj.m_HeaderSize);
        CopyTablesFromDecoderToEncoder();
        }

    // copy the compressor info
    m_ExternalQuantizationTablesUse = pi_rObj.m_ExternalQuantizationTablesUse;
    if(m_ExternalQuantizationTablesUse)
        {
        int tblno;

        m_QuantizationTables.resize(NUM_QUANT_TBLS , HUINTVector());

        /* Copy the source's quantization tables. */
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if(pi_rObj.m_QuantizationTables[tblno].size() != 0)
                SetQuantizationTable(tblno, (const unsigned int*)&pi_rObj.m_QuantizationTables[tblno][0], false);
            else
                m_QuantizationTables[tblno].clear();
            }
        }
    SetSubsamplingMode(pi_rObj.m_SubsamplingMode);
    }


//-----------------------------------------------------------------------------
// public
// UpdateInternalState
//-----------------------------------------------------------------------------
void HCDCodecIJG::UpdateInternalState()
    {
    SetAbbreviateMode(m_AbbreviateMode);
    SetQuality(m_Quality);
    SetOptimizeCoding(m_OptimizeCoding);
    SetProgressiveMode(m_ProgressiveMode);
    SetColorMode((ColorModes)(m_ColorMode));

    // copy the decompressor info
    if(m_HeaderSize != 0)
        ReadHeader(m_pHeader, m_HeaderSize);

    // copy the compressor info
    if(m_ExternalQuantizationTablesUse)
        {
        int tblno;

        /* Copy the source's quantization tables. */
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if(m_QuantizationTables[tblno].size() != 0)
                SetQuantizationTable(tblno, (const unsigned int*)&m_QuantizationTables[tblno][0], false);
            }
        }
    SetSubsamplingMode(m_SubsamplingMode);
    }

//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HCDCodecIJG::DeepDelete()
    {
    if (cinfocomp)
        jpeg_destroy_compress(cinfocomp);
    if (cinfodec)
        jpeg_destroy_decompress(cinfodec);

    delete cinfocomp;
    delete cinfodec;

    delete m_ErrorManager.pub;

    if (m_IJLInMemory)
        {
        ijlFree(m_pIJLInfoEnc);
        ijlFree(m_pIJLInfoDec);
        delete m_pIJLInfoEnc;
        delete m_pIJLInfoDec;
        }
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecIJG::Reset()
    {
    if (!m_UseIJLWrite && (GetCurrentState() == STATE_COMPRESS))
        jpeg_abort_compress(cinfocomp);
    else if (!m_UseIJLRead && (GetCurrentState() == STATE_DECOMPRESS))
        jpeg_abort_decompress(cinfodec);

    HCDCodecJPEG::Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetOptimizeCoding
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetOptimizeCoding(bool pi_Enable)
    {
    HASSERT(cinfocomp != 0);

    if (m_UseIJLWrite)
        m_UseIJLWrite = !pi_Enable;

    m_OptimizeCoding = pi_Enable;
    cinfocomp->optimize_coding = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// SetQuality
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetQuality(Byte pi_Percentage)
    {
    HPRECONDITION(pi_Percentage <= 100);
    HASSERT(cinfocomp != 0);

    m_Quality = pi_Percentage;

    if (m_IJLInMemory)
        m_pIJLInfoEnc->jquality = m_Quality;

    if (m_ExternalQuantizationTablesUse)
        {
        for (int tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if(m_QuantizationTables[tblno].size() != 0)
                {
                jpeg_add_quant_table (cinfocomp,
                                      tblno,
                                      &m_QuantizationTables[tblno][0],
                                      jpeg_quality_scaling(m_Quality),  // set to 50% to preserve table values
                                      true);
                }
            }
        }
    else
        jpeg_set_quality(cinfocomp, pi_Percentage, true);
    }

//-----------------------------------------------------------------------------
// public
// GetOptimizeCoding
//-----------------------------------------------------------------------------
bool HCDCodecIJG::GetOptimizeCoding() const
    {
    return(m_OptimizeCoding);
    }

//-----------------------------------------------------------------------------
// public
// GetQuality
//-----------------------------------------------------------------------------
Byte HCDCodecIJG::GetQuality() const
    {
    return m_Quality;
    }


//-----------------------------------------------------------------------------
// public
// SetProgressiveMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetProgressiveMode(bool pi_Enable)
    {
    if(pi_Enable)
        {
        jpeg_simple_progression(cinfocomp);
        }
    else
        {
        if (!m_UseIJLRead)
            {
            if(cinfocomp->scan_info != NULL)
                {
                free((cinfocomp->scan_info));
                cinfocomp->scan_info = NULL;
                cinfocomp->num_scans = 0;
                }
            }
        }

    // IJL, don't support Read in progressive mode.
    if (m_UseIJLRead && pi_Enable)
        m_UseIJLRead = false;

    m_ProgressiveMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsProgressive
//-----------------------------------------------------------------------------
bool HCDCodecIJG::IsProgressive() const
    {
    return m_ProgressiveMode;
    }

//-----------------------------------------------------------------------------
// public
// SetBitsPerPixel
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetBitsPerPixel(uint32_t pi_BitsPerPixel)
    {
    HCDCodecJPEG::SetBitsPerPixel(pi_BitsPerPixel);

    if(pi_BitsPerPixel == 24)
        SetColorMode(RGB);
    else if (pi_BitsPerPixel == 32)
        SetColorMode(RGBA);
    else
        SetColorMode(GRAYSCALE);
    }

//-----------------------------------------------------------------------------
// public
// ReadHeader
//-----------------------------------------------------------------------------
void HCDCodecIJG::ReadHeader(const void* pi_pInData, size_t pi_InDataSize)
    {
    HPRECONDITION(pi_pInData != 0);
    HPRECONDITION(pi_InDataSize > 0);
    HPRECONDITION(pi_InDataSize < INT_MAX);

    if (m_UseIJLRead || m_UseIJLWrite)
        {
        m_HeaderSize = pi_InDataSize;
        m_pHeader = new Byte[m_HeaderSize];
        memcpy(m_pHeader, pi_pInData, m_HeaderSize);

        m_pIJLInfoDec->JPGBytes = m_pHeader;
        m_pIJLInfoDec->JPGSizeBytes = (int)m_HeaderSize;
        m_pIJLInfoDec->jprops.use_external_qtables = 1;
        m_pIJLInfoDec->jprops.use_external_htables = 1;

        for (int i=0; i<4; ++i)
            m_pIJLInfoDec->jprops.rawquanttables[i].quantizer = &m_QuantTable[i][0];

        for (int i=0; i<8; ++i)
            {
            m_pIJLInfoDec->jprops.rawhufftables[i].bits = &m_HuffBits[i][0];
            m_pIJLInfoDec->jprops.rawhufftables[i].vals = &m_HuffVals[i][0];
            }

        IJLERR IJLErrorCode = ijlRead(m_pIJLInfoDec, IJL_JBUFF_READHEADER);

        if (IJLErrorCode < IJL_OK)
            throw HCDIJLErrorException((short)IJLErrorCode);

        // Resetting parameters cleared by read header operation
        //SetQuality(m_Quality);
        //SetColorMode(m_ColorMode);
        //SetSubsamplingMode(m_SubsamplingMode);
        }
    if ((!m_UseIJLRead) || (!m_UseIJLWrite))  // yep this may cause the header to be read for both IJL and IJG
        {
        ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer = pi_InDataSize;
        ((my_source_mgr*)(cinfodec->src))->pub.next_input_byte = (Byte*)pi_pInData;
        ((my_source_mgr*)(cinfodec->src))->m_DataSize = pi_InDataSize;
        ((my_source_mgr*)(cinfodec->src))->m_pData = (Byte*)pi_pInData;

        jpeg_read_header(cinfodec, false);

        m_HeaderSize = (uint32_t)(pi_InDataSize - ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer);
        m_pHeader = new Byte[m_HeaderSize];
        memcpy(m_pHeader, pi_pInData, m_HeaderSize);
        }
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetDimensions(uint32_t pi_Width, uint32_t pi_Height)
    {
    cinfocomp->image_width = pi_Width;
    cinfocomp->image_height = pi_Height;
    cinfodec->image_width = pi_Width;
    cinfodec->image_height = pi_Height;

    if (m_IJLInMemory)
        {
        m_pIJLInfoEnc->DIBWidth = pi_Width;
        m_pIJLInfoEnc->DIBHeight = pi_Height;
        m_pIJLInfoEnc->JPGWidth = pi_Width;
        m_pIJLInfoEnc->JPGHeight = pi_Height;
        m_pIJLInfoEnc->DIBPadBytes = 0;
        }

    HCDCodecJPEG::SetDimensions(pi_Width, pi_Height);
    }

//-----------------------------------------------------------------------------
// public
// SetAbbreviateMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetAbbreviateMode(bool pi_Enable)
    {
    m_AbbreviateMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// GetAbbreviateMode
//-----------------------------------------------------------------------------
bool HCDCodecIJG::GetAbbreviateMode() const
    {
    return m_AbbreviateMode;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsamplingMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetSubsamplingMode(SubsamplingModes pi_Mode)
    {
    m_SubsamplingMode = pi_Mode;

    switch (m_SubsamplingMode)
        {
        case S422:
            if (m_IJLInMemory)
                m_pIJLInfoEnc->JPGSubsampling = (m_pIJLInfoEnc->JPGChannels == 4) ? IJL_4224 : IJL_422;

            cinfocomp->comp_info[0].h_samp_factor = 2;
            cinfocomp->comp_info[0].v_samp_factor = 2;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 2;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 2;
            break;
        case S411:
            if (m_IJLInMemory)
                m_pIJLInfoEnc->JPGSubsampling = (m_pIJLInfoEnc->JPGChannels == 4) ? IJL_4114 : IJL_411;

            cinfocomp->comp_info[0].h_samp_factor = 2;
            cinfocomp->comp_info[0].v_samp_factor = 2;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 1;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 1;
            break;
        case SNONE:
            if (m_IJLInMemory)
                m_pIJLInfoEnc->JPGSubsampling = (IJL_JPGSUBSAMPLING)IJL_NONE;

            cinfocomp->comp_info[0].h_samp_factor = 1;
            cinfocomp->comp_info[0].v_samp_factor = 1;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 1;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 1;
            break;
        }
    }

//-----------------------------------------------------------------------------
// public
// SetQuantizationTable
// PLEASE CALL FOR SLOT 0 BEFORE SLOT 1
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable, bool pi_UnZigZag)
    {
    if (m_UseIJLWrite)
        {
        m_pIJLInfoEnc->jprops.use_external_qtables = 1;
        if(!m_ExternalQuantizationTablesUse)
            {
            m_QuantizationTables.resize(4, HUINTVector());
            m_ExternalQuantizationTablesUse = true;
            }
        m_QuantizationTables[pi_Slot].resize(64);

        uint32_t* pNatural = &m_QuantizationTables[pi_Slot][0];

        if (pi_pTable != pNatural)
            {
            if (pi_UnZigZag)
                for (register int r=0; r<64; pNatural[s_NaturalOrderIndexes[r++]] = pi_pTable[r]);
            else
                memcpy(pNatural, pi_pTable, 64 * sizeof(uint32_t));
            }

        if (pi_Slot > m_pIJLInfoEnc->jprops.maxquantindex)
            {
            m_pIJLInfoEnc->jprops.maxquantindex = pi_Slot;
            m_pIJLInfoEnc->jprops.nqtables = pi_Slot;
            }
        Byte ByteArray[64];
        for (register int r=0; r<64; ByteArray[r++] = pNatural[r]); // Converts from int[] to uchar[]
        m_pIJLInfoEnc->jprops.rawquanttables[pi_Slot].quantizer = &ByteArray[0];
        m_pIJLInfoEnc->jprops.rawquanttables[pi_Slot].ident = pi_Slot;
        for (int j = pi_Slot; j < m_pIJLInfoEnc->JPGChannels; j++)
            m_pIJLInfoEnc->jprops.jframe.comps[j].quant_sel = pi_Slot;
        }
    else
        {
        if(!m_ExternalQuantizationTablesUse)
            {
            m_QuantizationTables.resize(NUM_QUANT_TBLS, HUINTVector());
            m_ExternalQuantizationTablesUse = true;
            }
        jpeg_add_quant_table (cinfocomp,
                              pi_Slot,
                              pi_pTable,
                              jpeg_quality_scaling(m_Quality),  // set to 50% to preserve table values
                              true);

        // stock it
        m_QuantizationTables[pi_Slot].resize(64);
        memcpy(&m_QuantizationTables[pi_Slot][0], pi_pTable, 64 * sizeof(uint32_t));
        }
    }

//-----------------------------------------------------------------------------
// public
// GetSubsamplingMode
//-----------------------------------------------------------------------------

HCDCodecIJG::SubsamplingModes HCDCodecIJG::GetSubsamplingMode() const
    {
    return m_SubsamplingMode;
    }

//-----------------------------------------------------------------------------
// public
// SetSourceColorMode
//-----------------------------------------------------------------------------

void HCDCodecIJG::SetSourceColorMode(ColorModes pi_Mode)
    {
    m_StoredColorMode = pi_Mode;
    }

//-----------------------------------------------------------------------------
// public
// GetSourceColorMode
//-----------------------------------------------------------------------------

HCDCodecIJG::ColorModes HCDCodecIJG::GetSourceColorMode() const
    {
    return m_StoredColorMode;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetSubsetMaxCompressedSize() const
    {
    HPRECONDITION(cinfocomp->num_components > 0 && cinfocomp->num_components < 5);

    return (GetSubsetWidth() * cinfocomp->num_components) * GetSubsetHeight() * 2; // SAFETY
    }

//-----------------------------------------------------------------------------
// public
// CopyTablesFromDecoderToEncoder
//-----------------------------------------------------------------------------
void HCDCodecIJG::CopyTablesFromDecoderToEncoder()
    {
    if (m_UseIJLWrite)
        {
        ijlFree(m_pIJLInfoEnc);
        ijlInit(m_pIJLInfoEnc);
        m_pIJLInfoEnc->DIBWidth = GetWidth();
        m_pIJLInfoEnc->DIBHeight = GetHeight();
        m_pIJLInfoEnc->JPGWidth = GetWidth();
        m_pIJLInfoEnc->JPGHeight = GetHeight();
        SetAbbreviateMode(m_AbbreviateMode);
        SetQuality(50);   // to preserve table values
        SetProgressiveMode(m_ProgressiveMode);
        SetColorMode((ColorModes)(m_ColorMode));
        SetOptimizeCoding(m_OptimizeCoding);
        SetSubsamplingMode(m_SubsamplingMode);

        m_pIJLInfoEnc->jprops.use_external_qtables = 1;
        m_pIJLInfoEnc->jprops.maxquantindex = m_pIJLInfoDec->jprops.maxquantindex;
        m_pIJLInfoEnc->jprops.nqtables = m_pIJLInfoDec->jprops.nqtables;
        for (int i = 0; i < 4; i++)
            {
            m_pIJLInfoEnc->jprops.rawquanttables[i].quantizer = m_pIJLInfoDec->jprops.rawquanttables[i].quantizer;
            m_pIJLInfoEnc->jprops.rawquanttables[i].ident = m_pIJLInfoDec->jprops.rawquanttables[i].ident;
            }

        m_pIJLInfoEnc->jprops.use_external_htables = 1;
        m_pIJLInfoEnc->jprops.nhuffActables = m_pIJLInfoDec->jprops.nhuffActables;
        m_pIJLInfoEnc->jprops.nhuffDctables = m_pIJLInfoDec->jprops.nhuffDctables;
        m_pIJLInfoEnc->jprops.maxhuffindex = m_pIJLInfoDec->jprops.maxhuffindex;
        for (int i=0; i < 8; i++)
            {
            m_pIJLInfoEnc->jprops.rawhufftables[i].vals = m_pIJLInfoDec->jprops.rawhufftables[i].vals;
            m_pIJLInfoEnc->jprops.rawhufftables[i].hclass = m_pIJLInfoDec->jprops.rawhufftables[i].hclass;
            m_pIJLInfoEnc->jprops.rawhufftables[i].ident = m_pIJLInfoDec->jprops.rawhufftables[i].ident;
            }
        }
    else
        {
        //    jpeg_copy_critical_parameters(cinfodec, cinfocomp);
        /* jpeg_set_defaults may choose wrong colorspace, eg YCbCr if input is RGB.
         * Fix it to get the right header markers for the image colorspace.
         */
        JQUANT_TBL** qtblptr;
        int tblno;

        /*jpeg_set_colorspace(cinfocomp, cinfodec->jpeg_color_space);
        cinfocomp->data_precision = cinfodec->data_precision;
        cinfocomp->CCIR601_sampling = cinfodec->CCIR601_sampling;
        */ /* Copy the source's quantization tables. */
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if (cinfodec->quant_tbl_ptrs[tblno] != NULL)
                {
                qtblptr = & cinfocomp->quant_tbl_ptrs[tblno];
                if (*qtblptr == NULL)
                    *qtblptr = jpeg_alloc_quant_table((j_common_ptr) cinfocomp);
                MEMCOPY((*qtblptr)->quantval,
                        cinfodec->quant_tbl_ptrs[tblno]->quantval,
                        SIZEOF((*qtblptr)->quantval));
                (*qtblptr)->sent_table = false;
                }
            }
        /* Copy the source's per-component info.
         * Note we assume jpeg_set_defaults has allocated the dest comp_info array.
         */
        /*cinfocomp->num_components = cinfodec->num_components;
        if (cinfocomp->num_components < 1 || cinfocomp->num_components > MAX_COMPONENTS)
          ERREXIT2(cinfocomp, JERR_COMPONENT_COUNT, cinfocomp->num_components,
                   MAX_COMPONENTS);
        for (ci = 0, incomp = cinfodec->comp_info, outcomp = cinfocomp->comp_info;
             ci < cinfocomp->num_components; ci++, incomp++, outcomp++)
        {
          outcomp->component_id = incomp->component_id;
          outcomp->h_samp_factor = incomp->h_samp_factor;
          outcomp->v_samp_factor = incomp->v_samp_factor;
          outcomp->quant_tbl_no = incomp->quant_tbl_no;
          /* Make sure saved quantization table for component matches the qtable
           * slot.  If not, the input file re-used this qtable slot.
           * IJG encoder currently cannot duplicate this.
           *//*
tblno = outcomp->quant_tbl_no;
if (tblno < 0 || tblno >= NUM_QUANT_TBLS ||
cinfodec->quant_tbl_ptrs[tblno] == NULL)
ERREXIT1(cinfocomp, JERR_NO_QUANT_TABLE, tblno);
slot_quant = cinfodec->quant_tbl_ptrs[tblno];
c_quant = incomp->quant_table;
if (c_quant != NULL)
{
for (coefi = 0; coefi < DCTSIZE2; coefi++)
{
if (c_quant->quantval[coefi] != slot_quant->quantval[coefi])
ERREXIT1(cinfocomp, JERR_MISMATCHED_QUANT_TABLE, tblno);
}
}*/
        /* Note: we do not copy the source's Huffman table assignments;
         * instead we rely on jpeg_set_colorspace to have made a suitable choice.
         */
        // }
        }
    }

#endif