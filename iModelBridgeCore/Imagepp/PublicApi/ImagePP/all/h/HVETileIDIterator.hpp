//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVETileIDIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVETileIDIterator::HVETileIDIterator()
    {
    m_pDescriptor = 0;

    m_Index = 0;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVETileIDIterator::HVETileIDIterator(HGFTileIDDescriptor*    pi_pDescriptor,
                                            const HFCPtr<HVEShape>& pi_rpRegion)
    : m_pRegion(pi_rpRegion)
    {
    m_pDescriptor = pi_pDescriptor;
    m_Index = 0;
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
inline HVETileIDIterator::HVETileIDIterator (const HVETileIDIterator& pi_rObj)
    {
    m_Index       = pi_rObj.m_Index;
    m_pDescriptor = pi_rObj.m_pDescriptor;
    m_pRegion     = pi_rObj.m_pRegion;
    }


//-----------------------------------------------------------------------------
// Assignment
//-----------------------------------------------------------------------------
inline HVETileIDIterator& HVETileIDIterator::operator=(const HVETileIDIterator& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Index       = pi_rObj.m_Index;
        m_pDescriptor = pi_rObj.m_pDescriptor;
        m_pRegion     = pi_rObj.m_pRegion;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HVETileIDIterator::~HVETileIDIterator()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline void HVETileIDIterator::SetParameters(HGFTileIDDescriptor*    pi_pDescriptor,
                                             const HFCPtr<HVEShape>& pi_rpRegion)
    {
    HASSERT(m_pDescriptor == 0);

    m_pRegion     = pi_rpRegion;
    m_pDescriptor = pi_pDescriptor;
    }


//-----------------------------------------------------------------------------
// Only keep indexes of tiles that are within the region
//-----------------------------------------------------------------------------
inline void HVETileIDIterator::FindNextValidIndex()
    {
    HASSERT(m_pDescriptor != 0);

    if (!m_pRegion->IsRectangle())
        {
        uint64_t PosX;
        uint64_t PosY;
        while (m_Index < m_pDescriptor->GetTileCount())
            {
            m_pDescriptor->GetPositionFromIndex(m_Index, &PosX, &PosY);

            // Create a shape for this tile
            HVE2DRectangle TileShape((double)PosX,
                                     (double)PosY,
                                     (double)(PosX + m_pDescriptor->GetTileWidth()),
                                     (double)(PosY + m_pDescriptor->GetTileHeight()),
                                     m_pRegion->GetCoordSys());

            // If TileShape touches or is inside the region, the tile is useful.
            // If tile is out, check if it contains the region. If so, it is also useful.
            if (TileShape.CalculateSpatialPositionOf(*m_pRegion->GetShapePtr()) != HVE2DShape::S_OUT ||
                m_pRegion->GetShapePtr()->CalculateSpatialPositionOf(TileShape) != HVE2DShape::S_OUT)
                {
                break;
                }

            m_Index = m_pDescriptor->GetNextTileIndex();
            }
        }
    }


//-----------------------------------------------------------------------------
// Retrieve the first index
//-----------------------------------------------------------------------------
inline uint64_t HVETileIDIterator::GetFirstTileIndex()
    {
    HASSERT(m_pDescriptor != 0);

    m_Index = m_pDescriptor->GetFirstTileIndex(m_pRegion->GetExtent());

    FindNextValidIndex();

    return m_Index;
    }


//-----------------------------------------------------------------------------
// Advance to next index
//-----------------------------------------------------------------------------
inline uint64_t HVETileIDIterator::GetNextTileIndex()
    {
    HASSERT(m_pDescriptor != 0);

    m_Index = m_pDescriptor->GetNextTileIndex();

    FindNextValidIndex();

    return m_Index;
    }


//-----------------------------------------------------------------------------
// Get the current index
//-----------------------------------------------------------------------------
inline uint64_t HVETileIDIterator::GetCurrentTileIndex() const
    {
    HASSERT(m_pDescriptor != 0);

    return m_Index;
    }


//-----------------------------------------------------------------------------
// Get the descriptor
//-----------------------------------------------------------------------------
inline HGFTileIDDescriptor* HVETileIDIterator::GetDescriptor() const
    {
    return m_pDescriptor;
    }


//-----------------------------------------------------------------------------
// Get the region
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape> HVETileIDIterator::GetRegion() const
    {
    return m_pRegion;
    }
END_IMAGEPP_NAMESPACE
