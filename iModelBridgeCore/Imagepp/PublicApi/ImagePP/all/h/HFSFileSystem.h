//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSFileSystem.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFSFileSystem
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFSItem.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe the base class for a file system.

    @inheritance This is a pure virtual class, cannot be instanciate.

    @see HFSItem
    -----------------------------------------------------------------------------
*/
class HFSFileSystem : public HFCShareableObject<HFSFileSystem>
    {
public:
    HDECLARE_BASECLASS_ID(5012);

    //:> Primary method
    _HDLLu virtual                 ~HFSFileSystem();

    virtual HFCPtr<HFCURL>  GetCurrentURLPath() const = 0;
    virtual WString         GetCurrentPath() const = 0;
    virtual const HFCPtr<HFSItem>&
    ChangeFolder(const WString& pi_rPath) = 0;
    virtual const HFCPtr<HFSItem>&
    GetCurrentFolder() const = 0;

    virtual const HFCPtr<HFSItem>&
    GetItem(const WString& pi_rPath) const = 0;

protected:

    _HDLLu                         HFSFileSystem();

private:

    //:> Disabled methods
    HFSFileSystem(const HFSFileSystem&);
    HFSFileSystem& operator=(const HFSFileSystem&);
    };
