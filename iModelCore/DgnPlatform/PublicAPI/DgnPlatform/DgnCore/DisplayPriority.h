/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayPriority.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/Bentley.h>
#include "BSISerializable.h"

#define QV_RESERVED_DISPLAYPRIORITY     (32)
#define MAX_HW_DISPLAYPRIORITY          ((1<<23)-QV_RESERVED_DISPLAYPRIORITY)
#define RESERVED_DISPLAYPRIORITY        (1<<19)
#define MAX_MSTN_DISPLAYPRIORITY        (MAX_HW_DISPLAYPRIORITY - RESERVED_DISPLAYPRIORITY)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
//! Holds all the values to be persisted for display priority algorithms.
//! @bsiclass                                                     RichardTrefz    10/02
//=======================================================================================
struct      DisplayPrioritySettings : public BsiSerializable
{
private:
    Int32       m_active;           // current 'Active' element display priority used when new elements are created

protected:
    StatusInt SerWriteFields (DataExternalizer&);
    StatusInt SerReadFields (DataInternalizer&);
    void SerInitFields ();

public:
    DisplayPrioritySettings ();
    virtual ~DisplayPrioritySettings     () {}

    void Init () {SerInitFields ();}
    Int32 GetActive () const {return m_active;}
    void SetActive (Int32 active) {m_active = active;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
