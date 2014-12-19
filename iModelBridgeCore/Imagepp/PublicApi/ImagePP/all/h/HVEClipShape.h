//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEClipShape.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEShape
//-----------------------------------------------------------------------------
// Description of a shape in two-dimensional coordinate system.
//-----------------------------------------------------------------------------
#pragma once

#include <HFCPtr.h>
#include "HGF2DCoordSys.h"
#include "HGF2DLocation.h"
#include "HVEShape.h"

// Class declaration
// This class has been designed for clipping data according to a stack of
// masking or boundary clips when the range of the data that
// need to be clipped is not known.
class HVEClipShape : public HFCShareableObject<HVEClipShape>
    {
public :

    HVEClipShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    HVEClipShape(const HVEClipShape& pi_rClipShape);

    const HFCPtr<HGF2DCoordSys>& GetCoordSys () const;

    bool IsEmpty() const;

    void AddClip(const HFCPtr<HVEShape>& pi_rpClipShape, bool pi_isClipMask);

    bool IsPointClipped(const HGF2DLocation& pi_rPoint) const;

private :

    struct ClipDefinition
        {
        ClipDefinition(const HFCPtr<HVEShape>& pi_rpClipShape, bool pi_isClipMask)
            {
            m_pClipShape = pi_rpClipShape;
            m_isClipMask = pi_isClipMask;
            }

        HFCPtr<HVEShape>             m_pClipShape;
        bool                         m_isClipMask;
        };

    typedef list<ClipDefinition> ClipShapes;
    ClipShapes                   m_clips;
    HFCPtr<HGF2DCoordSys>        m_pCoordSys;
    };

#include "HVEClipShape.hpp"





