//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSPersistentFileInfoList.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCSPersistentFileInfoList
//-----------------------------------------------------------------------------

#pragma once

#include "HFCFileInfo.h"
#include "HPMPersistentObject.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author ???

    ?????

    DOES NOT APPEAR USED ... MOVE TO PROTO
    -----------------------------------------------------------------------------
*/
class HCSPersistentFileInfoList : public HPMPersistentObject,
    public HPMShareableObject<HCSPersistentFileInfoList>
    {
    HPM_DECLARE_CLASS_DLL(_HDLLn, 1150)

public:

    // Constructor - destructor
    HCSPersistentFileInfoList();
    virtual ~HCSPersistentFileInfoList();

    // List operations
    const HFCFileInfo::HFCFileInfoList& GetList();
    void Add(HFCFileInfo* pi_FileInfo);
    void Remove(HFCFileInfo* pi_FileInfo);

protected:

private:

    // Not implemented
    HCSPersistentFileInfoList(const HCSPersistentFileInfoList&);
    HCSPersistentFileInfoList& operator=(const HCSPersistentFileInfoList&);

    // Attribute
    HFCFileInfo::HFCFileInfoList m_FileList;
    };

