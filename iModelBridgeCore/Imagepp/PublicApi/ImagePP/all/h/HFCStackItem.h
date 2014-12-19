//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStackItem.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFCStackItem
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCStackItem.h"


class HFCStack;

class HFCStackItem : public HFCShareableObject<HFCStackItem>
    {
public:
    HDECLARE_BASECLASS_ID (1370);

    virtual ~HFCStackItem() {};

protected:
    HFCStackItem() {};

private:


    // disabled methods
    HFCStackItem(const HFCStackItem&);
    HFCStackItem& operator=(const HFCStackItem&);
    };


