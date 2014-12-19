//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABitmapIterator.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapIterator
// ----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRABitmapIterator.h>
#include <Imagepp/all/h/HRABitmapBase.h>

//-----------------------------------------------------------------------------
// HRABitmapIterator::HRABitmapIterator - Constructor
//-----------------------------------------------------------------------------
HRABitmapIterator::HRABitmapIterator (const HFCPtr<HRABitmapBase>&  pi_pBitmap,
                                      const HRAIteratorOptions& pi_rOptions)
    : HRARasterIterator ( (HFCPtr<HRARaster>&) pi_pBitmap, pi_rOptions),
      m_Index (0L)
    {
    }

//-----------------------------------------------------------------------------
// HRABitmapIterator::~HRABitmapIterator - Destructor
//-----------------------------------------------------------------------------
HRABitmapIterator::~HRABitmapIterator()
    {
    }

//-----------------------------------------------------------------------------
// HRABitmapEditor::HRABitmapEditor - Copy constructor
//-----------------------------------------------------------------------------
HRABitmapIterator::HRABitmapIterator(const HRABitmapIterator& pi_rBitmapIterator)
    : HRARasterIterator (pi_rBitmapIterator),
      m_Index       (pi_rBitmapIterator.m_Index)
    {
    }

//-----------------------------------------------------------------------------
// HRABitmapEditor::operator= - Assignment operator
//-----------------------------------------------------------------------------
HRABitmapIterator& HRABitmapIterator::operator=(const HRABitmapIterator& pi_rBitmapIterator)
    {
    if(this != &pi_rBitmapIterator)
        {
        HRARasterIterator::operator=(pi_rBitmapIterator);

        m_Index     = pi_rBitmapIterator.m_Index;
        }

    return(*this);
    }



//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRABitmapIterator::PrintState (ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    // Call the parent
    HRARasterIterator::PrintState (po_rOutput);

    po_rOutput

            << "HRABitmapIterator"
            << endl;

#endif
    }
