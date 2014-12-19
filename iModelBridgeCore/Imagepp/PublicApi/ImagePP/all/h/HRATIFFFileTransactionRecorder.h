//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATIFFFileTransactionRecorder.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HRATIFFFileTransactionRecorder
//:>---------------------------------------------------------------------------
#pragma once

#include "HRATransactionRecorder.h"
#include "HFCAccessMode.h"


class HTIFFFileRecorder;
class HRATransaction;
class HFCURL;
class HFCURLFile;


//--------------------------------------------------------------------------------------
// Class : HRATIFFFileTransactionRecorder
//--------------------------------------------------------------------------------------
class HRATIFFFileTransactionRecorder : public HRATransactionRecorder
    {
public:

    _HDLLg static HRATIFFFileTransactionRecorder* CreateFor(const HFCPtr<HFCURL>& pi_rpURL,
                                                            uint32_t              pi_Page,
                                                            bool                 pi_NewFile = false);


    _HDLLg HRATIFFFileTransactionRecorder    (const HFCPtr<HFCURLFile>& pi_rpURL,
                                              bool                     pi_NewFile = false);
    _HDLLg virtual ~HRATIFFFileTransactionRecorder   ();

    virtual HRATransaction*     CreateNewTransaction(TransactionType    pi_TransactionType) override;
    virtual HRATransaction*     CreateTransaction   (TransactionType    pi_TransactionType,
                                                     uint32_t          pi_TransactionID) override;


    virtual bool               HasStack            (TransactionType    pi_TransactionType);
    virtual void                GetTransactionStack (TransactionType    pi_TransactionType,
                                                     void**             po_ppStack,
                                                     size_t*            po_pStackSize) override;

    virtual void                PutTransactionStack (TransactionType    pi_TransactionType,
                                                     const void*        pi_pStack,
                                                     size_t             pi_StackSize) override;

    _HDLLg virtual void         ClearAllRecordedData() override;



protected:

private:

    // members
    HAutoPtr<HTIFFFileRecorder>     m_pUndoRecorder;
    HAutoPtr<HTIFFFileRecorder>     m_pRedoRecorder;
    WString                         m_FileName;
    bool                           m_NewFile;

    void            AcquiredRecorders();

    // disabled methods
    HRATIFFFileTransactionRecorder(const HRATIFFFileTransactionRecorder&);
    HRATIFFFileTransactionRecorder& operator=(const HRATIFFFileTransactionRecorder&);
    };


