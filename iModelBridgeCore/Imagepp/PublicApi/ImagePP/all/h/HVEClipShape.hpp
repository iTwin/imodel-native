//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEClipShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HVEShape
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
inline HVEClipShape::HVEClipShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    m_pCoordSys = pi_rpCoordSys;
    }

//-----------------------------------------------------------------------------
// Copy Constructor.
//-----------------------------------------------------------------------------
inline HVEClipShape::HVEClipShape(const HVEClipShape& pi_rClipShape)
    {
    ClipShapes::const_iterator clipIter(pi_rClipShape.m_clips.begin());
    ClipShapes::const_iterator clipIterEnd(pi_rClipShape.m_clips.end());

    while (clipIter != clipIterEnd)
        {
        m_clips.push_back(ClipDefinition(new HVEShape(*clipIter->m_pClipShape), clipIter->m_isClipMask));
        clipIter++;
        }

    m_pCoordSys = pi_rClipShape.m_pCoordSys;
    }

//-----------------------------------------------------------------------------
// GetCoordSys
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DCoordSys>& HVEClipShape::GetCoordSys () const
    {
    return m_pCoordSys;
    }

//-----------------------------------------------------------------------------
// Returns true if the object contains no clip.
//-----------------------------------------------------------------------------
inline bool HVEClipShape::IsEmpty() const
    {
    return m_clips.size() == 0;
    }

//-----------------------------------------------------------------------------
// Returns true if the object contains no clip.
//-----------------------------------------------------------------------------
inline void HVEClipShape::AddClip(const HFCPtr<HVEShape>& pi_rpClipShape, bool pi_isClipMask)
    {
    HPRECONDITION(pi_rpClipShape->GetCoordSys() == m_pCoordSys);

    ClipDefinition newClip(pi_rpClipShape, pi_isClipMask);

    m_clips.push_back(newClip);
    }

//-----------------------------------------------------------------------------
// Returns true if the point should be clipped, false otherwise.
//-----------------------------------------------------------------------------
inline bool HVEClipShape::IsPointClipped(const HGF2DLocation& pi_rPoint) const
    {
    //NTERAY: Bad idea, should not compare pointers but coord sys... This seems bad design to check for each point.
    HPRECONDITION(pi_rPoint.GetCoordSys() == m_pCoordSys);

    bool isClipped = false;

    ClipShapes::const_iterator clipIter(m_clips.begin());
    ClipShapes::const_iterator clipIterEnd(m_clips.end());

    while (clipIter != clipIterEnd)
        {
        if (clipIter->m_isClipMask == true)
            {
            if ((clipIter->m_pClipShape->IsPointIn(pi_rPoint) == true) ||
                (clipIter->m_pClipShape->IsPointOn(pi_rPoint) == true))
                {
                isClipped = true;
                break;
                }
            }
        else //Clip boundary
            {
            if ((clipIter->m_pClipShape->IsPointIn(pi_rPoint) == false) &&
                (clipIter->m_pClipShape->IsPointOn(pi_rPoint) == false))
                {
                isClipped = true;
                break;
                }
            }

        clipIter++;
        }

    return isClipped;
    }
END_IMAGEPP_NAMESPACE
