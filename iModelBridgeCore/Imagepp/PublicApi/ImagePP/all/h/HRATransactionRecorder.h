//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATransactionRecorder.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HRATransactionRecorder
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"


BEGIN_IMAGEPP_NAMESPACE
class HRATransaction;

class HRATransactionRecorder : public HFCShareableObject<HRATransactionRecorder>
    {
public:

    typedef enum
        {
        UNDO,
        REDO
        } TransactionType;

    virtual ~HRATransactionRecorder();

    virtual HRATransaction* CreateNewTransaction(TransactionType    pi_TransactionType) = 0;
    virtual HRATransaction* CreateTransaction   (TransactionType    pi_TransactionType,
                                                 uint32_t          pi_TransactionID) = 0;


    virtual void            GetTransactionStack (TransactionType   pi_TransactionType,
                                                 void**            po_ppStack,
                                                 size_t*           po_pStackSize) = 0;

    virtual void            PutTransactionStack (TransactionType   pi_TransactionType,
                                                 const void*       pi_pHeader,
                                                 size_t            pi_HeaderSize) = 0;

    virtual void                    ClearAllRecordedData() = 0;

protected:

    HRATransactionRecorder();

private:

    // disabled methods
    HRATransactionRecorder(const HRATransactionRecorder&);
    HRATransactionRecorder& operator=(const HRATransactionRecorder&);
    };
END_IMAGEPP_NAMESPACE
