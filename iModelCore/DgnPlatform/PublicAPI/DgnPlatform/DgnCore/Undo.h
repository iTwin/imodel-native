/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Undo.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ITxnManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
//! @bsiclass                                                     Keith.Bentley   02/05
//=======================================================================================
struct UndoableTxn : ITxn
{
    DEFINE_T_SUPER(ITxn)
    UndoableTxn() : ITxn() {m_opts.m_clearReversedTxns=true;}
    DGNPLATFORM_EXPORT virtual StatusInt _CheckDgnModelForWrite (DgnModelP) override;
 };

//=======================================================================================
//! @bsiclass                                                     Keith.Bentley   03/07
//=======================================================================================
struct  IllegalTxn : ITxn
{
    IllegalTxn() : ITxn() {m_opts.m_writesIllegal = true;}
    virtual StatusInt _CheckDgnModelForWrite  (DgnModelP) override {BeAssert (0); return ERROR;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
