//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBinStream.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCBinStream
//---------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"
#include "HFCPtr.h"

class HFCURL;

class HNOVTABLEINIT HFCBinStream
    {
public:

    HDECLARE_BASECLASS_ID (1290);

    // Primary methods

    _HDLLu                        HFCBinStream();
    _HDLLu virtual                ~HFCBinStream();

    // This static method can be used instead of the constructor.
    // Use it to create a correctly-typed object for specified URL.
    // The parameter pi_NbRetryBeforeThrow is useful only with LocalBinStream presently:
    //   if the value < 0 then Do not throw an exception, call GetLastExceptionID
    //   othersise an exception will be thrown if error.

    _HDLLu static HFCBinStream*    Instanciate(HFCPtr<HFCURL> pi_pURL,
                                               HFCAccessMode  pi_AccessMode = HFC_NO_ACCESS,
                                               short pi_NbRetry=0);

    // Information methods

    virtual HFCPtr<HFCURL>  GetURL() const = 0;
    virtual uint64_t       GetSize() const = 0;
    virtual HFCAccessMode   GetAccessMode() const = 0;
    _HDLLu ExceptionID      GetLastExceptionID() const;
    _HDLLu bool             IsOpened() const;


    // Multiuser access

    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share) = 0;
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size) = 0;

    // File pointer management

    virtual void            Seek(int64_t pi_Delta) = 0;
    virtual void            SeekToPos(uint64_t pi_NewPos) = 0;
    virtual void            SeekToBegin() = 0;
    virtual void            SeekToEnd() = 0;
    virtual uint64_t       GetCurrentPos() = 0;
    virtual bool           EndOfFile() = 0;

    // Content access

    void                    SetNoException();
    virtual size_t          Read(void* po_pData, size_t pi_DataSize) = 0;
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize) = 0;
    virtual size_t          Read(WChar* po_pData, size_t pi_DataSize) = 0;
    virtual size_t          Write(const WChar* pi_pData, size_t pi_DataSize) = 0;
    virtual bool           SetEOF() = 0;
    virtual bool           Flush() = 0;

    // Type used to register a class in the stream-type list.

    struct Creator
        {
        virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL,
                                     HFCAccessMode  pi_AccessMode,
                                     short pi_NbRetryBeforeThrow=0) const = 0;
        };

protected:

    // The type of the stream-type list, based on URL scheme type.

    typedef map<WString, Creator*, CaseInsensitiveStringCompare, allocator<Creator*> >
    StreamTypeList;

    // Scheme list access

    static StreamTypeList&  GetStreamTypeList();

    // Internal use by derived classes.
    ExceptionID m_LastException;

    // true if the BinStream can be used
    bool       m_BinStreamOpen;

private:

    // Disabled methods

    HFCBinStream(const HFCBinStream& pi_rObj);
    HFCBinStream& operator=(const HFCBinStream& pi_rObj);

    friend struct BinStreamTypeListDestroyer;

    // The scheme list.

    static StreamTypeList*  s_pStreamTypeList;

    //

    };


