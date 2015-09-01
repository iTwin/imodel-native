//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRATIFFFileTransactionRecorder.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HRATIFFFileTransactionRecorder
//:>---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRATIFFFileTransactionRecorder.h>

#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCURLEmbedFile.h>
#include <Imagepp/all/h/HCDPacket.h>

//---------------------------------------------------------------------------
// class HTIFFFileRecorder
//---------------------------------------------------------------------------

class ImagePP::HTIFFFileRecorder
    {
public:
    HTIFFFileRecorder       (const WString& pi_rFileName,
                             bool          pi_NewFile);
    virtual ~HTIFFFileRecorder      ();

    void    SetTransactionIndex     (uint32_t       pi_Index);
    uint32_t GetTransactionIndex     () const;
    uint32_t NewTransaction          ();
    bool   IsCurrentTransactionEmpty() const;

    bool   HasTransaction          (uint32_t                   pi_Index) const;

    size_t  Read                    (void*                      po_pData,
                                     size_t                     pi_DataSize);
    size_t  Write                   (const void*                pi_pData,
                                     size_t                     pi_DataSize);

    bool   Read                    (HFCPtr<HCDPacket>&         pi_rpPacket,
                                     uint32_t                   pi_Index);
    bool   Write                   (const HFCPtr<HCDPacket>&   pi_rpPacket,
                                     uint32_t                   pi_Index);

    void    Clear                   ();

private:
    // members
    HAutoPtr<HTIFFFile>         m_pFile;

    HFCPtr<HCDPacket>           m_pPacket;

    size_t                      m_BufferSize;
    size_t                      m_DataToRead;

    uint32_t                    m_CurrentIndex;
    uint32_t                    m_NewTransactionIndex;
    };


//--------------------------------------------------------------------------------------
// Class HRATIFFFileTransaction
//--------------------------------------------------------------------------------------
class HRATIFFFileTransaction : public HRATransaction
    {
public:
    HRATIFFFileTransaction
    (HTIFFFileRecorder* pi_pRecorder,
     uint32_t          pi_Index = -1);
    virtual ~HRATIFFFileTransaction();

    void    PushEntry       (uint64_t           pi_PosX,
                             uint64_t           pi_PosY,
                             uint32_t           pi_Width,
                             uint32_t           pi_Height,
                             size_t             pi_DataSize,
                             const void*        pi_pData) override;

    bool   PopEntry        (uint64_t*           po_pPosX,
                             uint64_t*          po_pPosY,
                             uint32_t*          po_pWidth,
                             uint32_t*          po_pHeight,
                             size_t*            po_pDataSize,
                             bool               pi_FirstCall = false) override;


    size_t  ReadEntryData   (size_t             pi_DataSize,
                             void*              po_pData) override;

    void    Commit          () override;
    void    Rollback        () override;

    void    Clear           () override;

    bool   IsEmpty         () const override;

    uint32_t GetID           () const override;

protected:

private:

    typedef struct
        {
        uint64_t PosX;
        uint64_t PosY;
        uint32_t Width;
        uint32_t Height;
        uint64_t DataSize;
        } EntryHeader;

    // members
    HTIFFFileRecorder*  m_pRecorder;
    uint32_t           m_Index;
    };








//--------------------------------------------------------------------------------------
// Class HRATIFFFileTransactionRecorder
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// static section - HRATIFFFileTransactionRecorder
//--------------------------------------------------------------------------------------
HRATIFFFileTransactionRecorder* HRATIFFFileTransactionRecorder::CreateFor(const HFCPtr<HFCURL>& pi_rpURL,
                                                                          uint32_t              pi_Page,
                                                                          bool                 pi_NewFile)
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID) || pi_rpURL->IsCompatibleWith(HFCURLEmbedFile::CLASS_ID));

    WString ComposedName(pi_rpURL->GetURL());

    // Compose the file name for the specified URL
    WString  Seps(L"\\/:*?\"<>|");
    size_t   Pos = 0;
    while ((Pos = ComposedName.find_first_of (Seps, Pos)) != WString::npos)
        {
        ComposedName[Pos] = L'_';
        Pos++;
        }

    // Add the path to the Booster url
    BeFileName pathFilename;
    ImageppLib::GetHost().GetImageppLibAdmin()._GetFileTransactionRecorderDirPath(pathFilename);
    HFCPtr<HFCURL> pPath = HFCURL::CreateFrom(pathFilename);

    WString RecorderPath(pPath->GetURL());

    // Add the / if necessary
    if ((RecorderPath[RecorderPath.size() - 1] != L'/') && (RecorderPath[RecorderPath.size() - 1] != L'\\'))
        RecorderPath += L"\\";

    RecorderPath += ComposedName;

    if(pi_Page != 0)
        {
        WChar TempBuffer[10];
        BeStringUtilities::Itow(TempBuffer, pi_Page, sizeof(TempBuffer) / sizeof(WChar), 10);

        RecorderPath += L"_page_";
        RecorderPath += TempBuffer;
        }

    HFCPtr<HFCURLFile> pRecorderFile(new HFCURLFile(RecorderPath));

    return new HRATIFFFileTransactionRecorder(pRecorderFile, pi_NewFile);
    }





//--------------------------------------------------------------------------------------
// public section - HRATIFFFileTransactionRecorder
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// public
// constructor
//--------------------------------------------------------------------------------------
HRATIFFFileTransactionRecorder::HRATIFFFileTransactionRecorder(const HFCPtr<HFCURLFile>&    pi_rpURL,
                                                               bool                        pi_NewFile)
    {
    HPRECONDITION(pi_rpURL != 0);

    // keep file information and create files only if a transaction is started
    m_FileName = pi_rpURL->GetHost()  + L"\\" + pi_rpURL->GetPath();
    m_NewFile = pi_NewFile;

    }

//--------------------------------------------------------------------------------------
// public
// destructor
//--------------------------------------------------------------------------------------
HRATIFFFileTransactionRecorder::~HRATIFFFileTransactionRecorder()
    {
    // close files
    m_pUndoRecorder = 0;
    m_pRedoRecorder = 0;
    }

//--------------------------------------------------------------------------------------
// public
// CreateNewTransaction
//--------------------------------------------------------------------------------------
HRATransaction* HRATIFFFileTransactionRecorder::CreateNewTransaction(TransactionType    pi_TransactionType)
    {
    AcquiredRecorders();
    return new HRATIFFFileTransaction(pi_TransactionType == UNDO ? m_pUndoRecorder : m_pRedoRecorder);
    }


//--------------------------------------------------------------------------------------
// public
// CreateTransaction
//--------------------------------------------------------------------------------------
HRATransaction* HRATIFFFileTransactionRecorder::CreateTransaction(TransactionType    pi_TransactionType,
                                                                  uint32_t          pi_TransactionID)
    {
    AcquiredRecorders();
    return new HRATIFFFileTransaction(pi_TransactionType == UNDO ? m_pUndoRecorder : m_pRedoRecorder,
                                      pi_TransactionID);
    }


//--------------------------------------------------------------------------------------
// public
// HasStack
//--------------------------------------------------------------------------------------
bool HRATIFFFileTransactionRecorder::HasStack(TransactionType pi_TransactionType)
    {
    AcquiredRecorders();
    if (pi_TransactionType == UNDO)
        return m_pUndoRecorder->HasTransaction(0);
    else
        return m_pRedoRecorder->HasTransaction(0);
    }


//--------------------------------------------------------------------------------------
// public
// GetTransactionStack
//--------------------------------------------------------------------------------------
void HRATIFFFileTransactionRecorder::GetTransactionStack(TransactionType  pi_TransactionType,
                                                         void**           po_ppStack,
                                                         size_t*          po_pStackSize)
    {
    HPRECONDITION(po_ppStack != 0);
    HPRECONDITION(po_pStackSize != 0);

    bool ReadSucceed = false;
    HFCPtr<HCDPacket> pPacket;

    if (!m_NewFile || m_pUndoRecorder != 0)
        {
        AcquiredRecorders();

        pPacket = new HCDPacket();
        pPacket->SetBufferOwnership(true);

        if (pi_TransactionType == UNDO)
            ReadSucceed = m_pUndoRecorder->Read(pPacket, 0);
        else
            ReadSucceed = m_pRedoRecorder->Read(pPacket, 0);
        }

    if (!ReadSucceed)
        {
        *po_ppStack = 0;
        *po_pStackSize = 0;
        }
    else
        {
        *po_ppStack = pPacket->GetBufferAddress();
        *po_pStackSize = pPacket->GetDataSize();
        pPacket->SetBufferOwnership(false);
        }
    }

//--------------------------------------------------------------------------------------
// public
// SetHeader
//--------------------------------------------------------------------------------------
void HRATIFFFileTransactionRecorder::PutTransactionStack(TransactionType  pi_TransactionType,
                                                         const void*      pi_pStack,
                                                         size_t           pi_StackSize)
    {
    HFCPtr<HCDPacket> pStack(new HCDPacket());
    pStack->SetBuffer(const_cast<void*>(pi_pStack), pi_StackSize);
    pStack->SetDataSize(pi_StackSize);

    AcquiredRecorders();
    if (pi_TransactionType == UNDO)
        m_pUndoRecorder->Write(pStack, 0);
    else
        m_pRedoRecorder->Write(pStack, 0);
    }


//--------------------------------------------------------------------------------------
// public
// SetHeader
//--------------------------------------------------------------------------------------
void HRATIFFFileTransactionRecorder::ClearAllRecordedData()
    {
    if (m_pUndoRecorder != 0)
        {
        m_pUndoRecorder = 0;
        BeFileName::BeDeleteFile((m_FileName + L".UNDO").c_str());
        }

    if (m_pRedoRecorder != 0)
        {
        m_pRedoRecorder = 0;
        BeFileName::BeDeleteFile((m_FileName + L".REDO").c_str());
        }
    }


//--------------------------------------------------------------------------------------
// private section - HRATIFFFileTransactionRecorder
//--------------------------------------------------------------------------------------
void HRATIFFFileTransactionRecorder::AcquiredRecorders()
    {
    HPRECONDITION((m_pUndoRecorder == 0 && m_pRedoRecorder == 0) ||
                  (m_pUndoRecorder != 0 && m_pRedoRecorder != 0));

    if (m_pUndoRecorder == 0 && m_pRedoRecorder == 0)
        {
        m_pUndoRecorder = new HTIFFFileRecorder(m_FileName + L".UNDO", m_NewFile);
        m_pRedoRecorder = new HTIFFFileRecorder(m_FileName + L".REDO", m_NewFile);
        }

    HPOSTCONDITION(m_pUndoRecorder != 0 && m_pRedoRecorder != 0);
    }




//--------------------------------------------------------------------------------------
// Class HRATIFFFileTransaction
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// public section - HRATIFFFileTransaction
//--------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
// public
// constructor
//---------------------------------------------------------------------------
HRATIFFFileTransaction::HRATIFFFileTransaction(HTIFFFileRecorder*   pi_pRecorder,
                                               uint32_t             pi_Index)
    : HRATransaction(),
      m_pRecorder(pi_pRecorder)
    {
    HPRECONDITION(pi_pRecorder != 0);

    m_Index = (pi_Index == -1 ? m_pRecorder->NewTransaction() : pi_Index);
    }


//---------------------------------------------------------------------------
// public
// destructor
//---------------------------------------------------------------------------
HRATIFFFileTransaction::~HRATIFFFileTransaction ()
    {

    }


//---------------------------------------------------------------------------
// public
// PushEntry
//---------------------------------------------------------------------------
void HRATIFFFileTransaction::PushEntry(uint64_t     pi_PosX,
                                       uint64_t     pi_PosY,
                                       uint32_t     pi_Width,
                                       uint32_t     pi_Height,
                                       size_t        pi_DataSize,
                                       const void*   pi_pData)
    {
    EntryHeader Header;
    Header.PosX     = pi_PosX;
    Header.PosY     = pi_PosY;
    Header.Width    = pi_Width;
    Header.Height   = pi_Height;
    Header.DataSize = pi_DataSize;

    m_pRecorder->Write(pi_pData, pi_DataSize);
    m_pRecorder->Write(&Header, sizeof(Header));
    }


//---------------------------------------------------------------------------
// public
// PopEntry
//
// Note : ReadEntryData must be called after each PopFirst() and
//        PopNext()
//---------------------------------------------------------------------------
bool HRATIFFFileTransaction::PopEntry(uint64_t*  po_pPosX,
                                       uint64_t*  po_pPosY,
                                       uint32_t*   po_pWidth,
                                       uint32_t*   po_pHeight,
                                       size_t*   po_pDataSize,
                                       bool     pi_FirstCall)
    {
    if (pi_FirstCall)
        m_pRecorder->SetTransactionIndex(m_Index);

    EntryHeader Header;
    if (m_pRecorder->Read(&Header, sizeof(Header)) == sizeof(Header))
        {
        *po_pPosX = Header.PosX;
        *po_pPosY = Header.PosY;
        *po_pWidth = Header.Width;
        *po_pHeight = Header.Height;
        *po_pDataSize = (size_t)Header.DataSize;

        return true;
        }
    return false;
    }


//---------------------------------------------------------------------------
// public
// ReadEntryData
//
// Note : po_pBuffer must be enough to read the entire data, pi_DataSize
//        must be the size of the data, See PopEntry
//---------------------------------------------------------------------------
size_t HRATIFFFileTransaction::ReadEntryData(size_t  pi_DataSize,
                                             void*   po_pBuffer)
    {
    return m_pRecorder->Read(po_pBuffer, pi_DataSize);
    }


//---------------------------------------------------------------------------
// public
// Commit
//---------------------------------------------------------------------------
void HRATIFFFileTransaction::Commit()
    {
    // write the current block
    m_Index = m_pRecorder->GetTransactionIndex();
    m_pRecorder->Write(0, 0);   // flush block
    }


//---------------------------------------------------------------------------
// public
// Rollback
//---------------------------------------------------------------------------
void HRATIFFFileTransaction::Rollback()
    {
    HASSERT(0);
    }

//---------------------------------------------------------------------------
// public
// Clear
//---------------------------------------------------------------------------
void HRATIFFFileTransaction::Clear()
    {
//    HASSERT(0);
    }

//---------------------------------------------------------------------------
// public
// IsEmpty
//---------------------------------------------------------------------------
bool HRATIFFFileTransaction::IsEmpty() const
    {
    return m_pRecorder->IsCurrentTransactionEmpty();
    }

//---------------------------------------------------------------------------
// public
// GetID
//---------------------------------------------------------------------------
uint32_t HRATIFFFileTransaction::GetID() const
    {
    return m_Index;
    }



//--------------------------------------------------------------------------------------
// Class HTIFFFileRecorder
//--------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
// public section - HTIFFFileRecorder
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// public
// constructor
//---------------------------------------------------------------------------
HTIFFFileRecorder::HTIFFFileRecorder(const WString& pi_rFileName,
                                     bool          pi_NewFile)
    : m_CurrentIndex(-1),
      m_NewTransactionIndex(1)
    {

    HFCAccessMode AccessMode;
    if (pi_NewFile)
        AccessMode = HFC_READ_WRITE_CREATE;
    else
        AccessMode = HFC_READ_WRITE | HFC_OPEN_ALWAYS;

    m_pFile = HTIFFFile::UndoRedoFile(pi_rFileName, AccessMode);

    m_BufferSize = 16 * 1024;

    m_pPacket = new HCDPacket(new Byte[m_BufferSize], m_BufferSize);
    m_pPacket->SetBufferOwnership(true);
    m_DataToRead = 0;
    }


//---------------------------------------------------------------------------
// public
// destructor
//---------------------------------------------------------------------------
HTIFFFileRecorder::~HTIFFFileRecorder()
    {
    }

//---------------------------------------------------------------------------
// public
// SetCurrentIndex
//---------------------------------------------------------------------------
void HTIFFFileRecorder::SetTransactionIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index > 0);    // the first strip is reserved

    bool RetValue = true;

    if (m_CurrentIndex != pi_Index)
        {
        RetValue = (m_pFile->StripRead(m_pPacket, pi_Index) == H_SUCCESS);
        m_CurrentIndex = pi_Index;
        }

    if (RetValue)
        m_DataToRead = m_pPacket->GetDataSize() - sizeof(bool);    // the buffer start with a bool value
    else
        m_DataToRead = 0;
    }

//---------------------------------------------------------------------------
// public
// GetCurrentIndex
//---------------------------------------------------------------------------
uint32_t HTIFFFileRecorder::GetTransactionIndex() const
    {
    return m_CurrentIndex;
    }

//---------------------------------------------------------------------------
// public
// NewTransaction
//---------------------------------------------------------------------------
uint32_t HTIFFFileRecorder::NewTransaction()
    {
    m_CurrentIndex = m_NewTransactionIndex;

    // reset the packet
    m_pPacket->SetDataSize(0);
    return m_CurrentIndex;
    }


//---------------------------------------------------------------------------
// public
// IsCurrentTransactionEmpty
//---------------------------------------------------------------------------
bool HTIFFFileRecorder::IsCurrentTransactionEmpty() const
    {
    return (m_pPacket->GetDataSize() == 0 && m_CurrentIndex == m_NewTransactionIndex);
    }


//---------------------------------------------------------------------------
// public
// HasTransaction
//---------------------------------------------------------------------------
bool HTIFFFileRecorder::HasTransaction(uint32_t pi_Index) const
    {
    return (m_pFile->NumberOfStrips() > pi_Index);
    }


//---------------------------------------------------------------------------
// public
// Read
//---------------------------------------------------------------------------
size_t HTIFFFileRecorder::Read(void*          po_pData,
                               size_t         pi_DataSize)
    {
    if (pi_DataSize <= m_DataToRead)
        {
        // the buffer contain enough data
        m_DataToRead -= pi_DataSize;
        memcpy(po_pData, &(m_pPacket->GetBufferAddress()[sizeof(bool) + m_DataToRead]), pi_DataSize);

        return pi_DataSize;
        }
    else if (*((bool*)m_pPacket->GetBufferAddress()))
        {
        // we have another block...
        memcpy(&((Byte*)po_pData)[pi_DataSize - m_DataToRead], &(m_pPacket->GetBufferAddress()[sizeof(bool)]), m_DataToRead);

        size_t DataRead = m_DataToRead;

        m_pFile->StripRead(m_pPacket, --m_CurrentIndex);

        m_DataToRead = m_pPacket->GetDataSize() - sizeof(bool);
        return DataRead + Read(po_pData, pi_DataSize - DataRead);
        }
    else
        return 0;   // no more blocks
    }

//---------------------------------------------------------------------------
// public
// Write
//---------------------------------------------------------------------------
size_t HTIFFFileRecorder::Write(const void*    pi_pData,
                                size_t         pi_DataSize)
    {
    if (pi_DataSize == 0)
        {
        if (m_pPacket->GetDataSize() != 0)
            {
            // write current block
            m_pFile->StripWriteCompress(m_pPacket->GetBufferAddress(),
                                        (uint32_t)m_pPacket->GetDataSize(),
                                        m_CurrentIndex);
            m_NewTransactionIndex = m_CurrentIndex + 1;
            }

        return 0;
        }

    if (m_pPacket->GetDataSize() == 0)
        {
        bool LinkedToPreviousBlock = false;
        memcpy(m_pPacket->GetBufferAddress(), &LinkedToPreviousBlock, sizeof(bool));
        m_pPacket->SetDataSize(sizeof(bool));
        }

    if (m_pPacket->GetDataSize() + pi_DataSize < m_pPacket->GetBufferSize())
        {
        // the buffer has enough space to copy data
        memcpy(&(m_pPacket->GetBufferAddress()[m_pPacket->GetDataSize()]), pi_pData, pi_DataSize);
        m_pPacket->SetDataSize(m_pPacket->GetDataSize() + pi_DataSize);
        return pi_DataSize;
        }
    else
        {
        // fill the buffer
        // compute the available size
        size_t WrittenDataSize = m_pPacket->GetBufferSize() - m_pPacket->GetDataSize();
        memcpy(&(m_pPacket->GetBufferAddress()[m_pPacket->GetDataSize()]), pi_pData, WrittenDataSize);

        // write the buffer into the file
        m_pFile->StripWriteCompress(m_pPacket->GetBufferAddress(), (uint32_t)m_pPacket->GetBufferSize(), m_CurrentIndex++);

        // link the new block to the previous
        bool LinkedToPreviousBlock = true;
        memcpy(m_pPacket->GetBufferAddress(), &LinkedToPreviousBlock, sizeof(bool));
        m_pPacket->SetDataSize(sizeof(bool));

        return WrittenDataSize + Write(&((Byte*)pi_pData)[WrittenDataSize], pi_DataSize - WrittenDataSize);
        }
    }



//---------------------------------------------------------------------------
// public
// Read
//---------------------------------------------------------------------------
bool HTIFFFileRecorder::Read(HFCPtr<HCDPacket>&    po_pData,
                              uint32_t              pi_Index)
    {
    return (m_pFile->StripRead(po_pData, pi_Index) == H_SUCCESS);
    }

//---------------------------------------------------------------------------
// public
// Write
//---------------------------------------------------------------------------
bool HTIFFFileRecorder::Write(const HFCPtr<HCDPacket>& pi_rpData,
                               uint32_t                 pi_Index)
    {
    return (m_pFile->StripWriteCompress(pi_rpData->GetBufferAddress(), (uint32_t)pi_rpData->GetDataSize(), pi_Index) == H_SUCCESS);
    }


