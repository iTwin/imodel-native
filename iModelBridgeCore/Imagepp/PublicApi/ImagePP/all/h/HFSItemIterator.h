//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSItemIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Class : HFSItemIterator
//:>---------------------------------------------------------------------------

#pragma once

#include "HFCNodeIterator.h"
#include "HFSItem.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class describe a special iterator for an HFS - File System.

    @see HFSItem
    @see HFCNodeIterator
    -----------------------------------------------------------------------------
*/
class HFSItemIterator : public HFCNodeIterator
    {
public:
    HDECLARE_CLASS_ID(HFSItemId_Iterator, HFCNodeIterator);

    HFSItemIterator(const HFCPtr<HFSItem>& pi_rpItem);
    HFSItemIterator(const HFSItemIterator& pi_rObj);
    ~HFSItemIterator();

    HFSItemIterator& operator=(const HFSItemIterator& pi_rObj);

    const HFCPtr<HFSItem>& GetFirstItem() const;
    const HFCPtr<HFSItem>& GetNextItem() const;
    const HFCPtr<HFSItem>& GetItem() const;

private:

    // disabled method
    HFSItemIterator();
    };

END_IMAGEPP_NAMESPACE
#include "HFSItemIterator.hpp"

