//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMBufferedImageIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMBufferedImageIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HIMBufferedImageIterator.h>


//-----------------------------------------------------------------------------
// Shaped constructor.
//-----------------------------------------------------------------------------
HIMBufferedImageIterator::HIMBufferedImageIterator(const HFCPtr<HIMBufferedImage>& pi_pBufImg,
                                                   const HRAIteratorOptions&       pi_rOptions)
    : HRARasterIterator( (HFCPtr<HRARaster>&) pi_pBufImg, pi_rOptions)
    {
    m_pIDSet = pi_pBufImg->GetTileIDPoolFor(*pi_rOptions.CalculateClippedRegion(GetRaster()));

    Initialize();
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HIMBufferedImageIterator::HIMBufferedImageIterator(const HIMBufferedImageIterator& pi_rObj)
    : HRARasterIterator(pi_rObj),
      m_IDSetIterator(pi_rObj.m_IDSetIterator)
    {
    m_pIDSet = new HIMBufImgTileIDSet(*pi_rObj.m_pIDSet);

    m_CurrentID = pi_rObj.m_CurrentID;
    m_Valid = pi_rObj.m_Valid;

    m_pCurrentRaster = 0;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HIMBufferedImageIterator::~HIMBufferedImageIterator()
    {
    // Delete the ID set
    delete m_pIDSet;
    }


//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another iterator.
//-----------------------------------------------------------------------------
HIMBufferedImageIterator& HIMBufferedImageIterator::operator=(
    const HIMBufferedImageIterator& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Delete old Id set
        delete m_pIDSet;

        // Copy attributes

        m_pIDSet        = new HIMBufImgTileIDSet(*pi_rObj.m_pIDSet);
        m_IDSetIterator = pi_rObj.m_IDSetIterator;

        m_CurrentID      = pi_rObj.m_CurrentID;
        m_pCurrentRaster = 0;

        m_Valid = pi_rObj.m_Valid;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Reset to start state
//-----------------------------------------------------------------------------
void HIMBufferedImageIterator::Reset()
    {
    m_CurrentID      = 0;
    m_pCurrentRaster = 0;
    }


//-----------------------------------------------------------------------------
// Return current raster object
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HIMBufferedImageIterator::operator()()
    {
    if (m_Valid)
        {
        while (m_pCurrentRaster == 0 && m_Valid)
            {
            m_pCurrentRaster = ((const HFCPtr<HIMBufferedImage>&)GetRaster())->GetTile(m_CurrentID);

            if (m_pCurrentRaster == 0)
                FindNextUsefulID();
            }
        }
    else
        m_pCurrentRaster = 0;

    return m_pCurrentRaster;
    }


//-----------------------------------------------------------------------------
// Go to next raster object, and return it
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HIMBufferedImageIterator::Next()
    {
    FindNextUsefulID();

    m_pCurrentRaster = 0;

    return operator()();
    }


//-----------------------------------------------------------------------------
// Setup members
//-----------------------------------------------------------------------------
void HIMBufferedImageIterator::Initialize()
    {
    m_IDSetIterator = m_pIDSet->begin();
    if (m_IDSetIterator != m_pIDSet->end())
        {
        m_CurrentID = (*m_IDSetIterator);
        m_Valid = true;
        }
    else
        m_Valid = false;

    m_pCurrentRaster = 0;
    }


//-----------------------------------------------------------------------------
// Find the next useful tile ID
//-----------------------------------------------------------------------------
void HIMBufferedImageIterator::FindNextUsefulID()
    {
    if (++m_IDSetIterator != m_pIDSet->end())
        m_CurrentID = (*m_IDSetIterator);
    else
        m_Valid = false;
    }


//-----------------------------------------------------------------------------
// Print the object's state
//-----------------------------------------------------------------------------
#ifdef __HMR_PRINTSTATE
void HIMBufferedImageIterator::PrintState(ostream& po_rOutput) const
    {
    // Call ancestor
    HRARasterIterator::PrintState(po_rOutput);

    po_rOutput
            << "HIMBufferedImageIterator"
            << endl;
    }
#endif
