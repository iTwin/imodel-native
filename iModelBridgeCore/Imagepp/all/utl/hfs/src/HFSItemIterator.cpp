//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSItemIterator.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> Methods for class HFSItemIterator
//:>---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFSItemIterator.h>


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor of this class.

 @param pi_rpItem The item to iterate. Cannot be 0
-----------------------------------------------------------------------------*/
HFSItemIterator::HFSItemIterator(const HFCPtr<HFSItem>& pi_rpItem)
    : HFCNodeIterator((const HFCPtr<HFCNode>&)pi_rpItem)
    {
    HPRECONDITION(pi_rpItem != 0);

    if (pi_rpItem->IsFolder())
        pi_rpItem->Expand();
    }

/**----------------------------------------------------------------------------
 Copy constructor.

 @param pi_rObj
-----------------------------------------------------------------------------*/
HFSItemIterator::HFSItemIterator(const HFSItemIterator& pi_rObj)
    : HFCNodeIterator(pi_rObj)
    {
    }

