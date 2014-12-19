//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCStruturedStorage.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCLocalBinStream
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCBinStream.h>

#ifndef _WIN32
#error HFCStructuredStorage.h is windows dependent.
#endif

enum StorageCreateMode
    {
    // An action to take on storage that exist and do not exist.
    StorageCreateMode_Never,       // Never create. Open will fail if storage doesn't exist.
    StorageCreateMode_New,         // Create a new storage if storage doesn't exist.
    StorageCreateMode_Always,      // Always create a new storage.
    };


class HFCStorageBinStream : public HFCBinStream
    {
public:

    HDECLARE_CLASS_ID (1287, HFCBinStream);

    HFCStorageBinStream(WString const&      pi_RootFilename,
                        IStorage*           pi_pStorage,
                        WString const&      pi_Name,
                        bool               pi_ReadOnly,
                        StorageCreateMode   pi_CreateMode);

    virtual                 ~HFCStorageBinStream();

    // Information methods
    virtual HFCPtr<HFCURL>  GetURL() const override;
    virtual uint64_t        GetSize() const override;
    virtual HFCAccessMode   GetAccessMode() const override;

    // Multiuser access
    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share) override;
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size) override;

    // File pointer management
    virtual void            Seek(int64_t pi_Delta) override;
    virtual void            SeekToPos(uint64_t pi_NewPos) override;
    virtual void            SeekToBegin() override;
    virtual void            SeekToEnd() override;
    virtual uint64_t        GetCurrentPos() override;
    virtual bool            EndOfFile() override;

    // Content access
    virtual size_t          Read(void* po_pData, size_t pi_DataSize) override;
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize) override;
    virtual size_t          Read(WChar* po_pData, size_t pi_CharCount) override;
    virtual size_t          Write(const WChar* pi_pData, size_t pi_CharCount) override;
    virtual bool           SetEOF() override;
    virtual bool           Flush() override;

private:

    // Disabled methods
    HFCStorageBinStream(const HFCStorageBinStream& pi_rObj);
    HFCStorageBinStream& operator=(const HFCStorageBinStream& pi_rObj);

    IStream*        m_pIStream;
    bool            m_SharedLock;
    WString         m_RootFilename;
    HFCAccessMode   m_accessMode;
    };

class HFCStruturedStorage
    {
public:

    HDECLARE_SEALEDCLASS_ID (1258);

    HFCStruturedStorage(WString const&      pi_Filename,
                        bool               pi_ReadOnly,
                        StorageCreateMode   pi_CreateMode);

    HFCStruturedStorage(WString const&      pi_RootFilename,
                        IStorage*           pi_pRootStorage,
                        WString const&      pi_SubStorageName,
                        bool               pi_ReadOnly,
                        StorageCreateMode   pi_CreateMode);

    ~HFCStruturedStorage(void);

    HFCStruturedStorage*    OpenSubStorage(WString const& pi_Name, bool pi_ReadOnly, StorageCreateMode pi_CreateMode);

    HFCStorageBinStream*   	OpenStream(WString const& pi_Name, bool pi_ReadOnly, StorageCreateMode pi_CreateMode);

    ExceptionID             GetLastExceptionID() const;

    IStorage*               GetStorage() const {return m_pIStorage;};

    //void Dump();

private:

    // Disabled methods
    HFCStruturedStorage(const HFCStruturedStorage& pi_rObj);
    HFCStruturedStorage& operator=(const HFCStruturedStorage& pi_rObj);

    IStorage*       m_pIStorage;
    ExceptionID     m_LastExceptionID;
    WString         m_RootFilename;
    };
