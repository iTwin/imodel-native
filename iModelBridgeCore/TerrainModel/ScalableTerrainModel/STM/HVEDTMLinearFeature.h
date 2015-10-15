//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HVEDTMLinearFeature.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEDTMLinearFeature
//-----------------------------------------------------------------------------
// This class implements chained multi-segments basic linear
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HGF2DCoordSys;
class HVE2DPolySegment;
END_IMAGEPP_NAMESPACE

#include <ImagePP/all/h/HVE2DPolySegment.h>
#include <ImagePP/all/h/HGF3DPoint.h>
#include <ImagePP/all/h/HGF3DExtent.h>
#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HGF2DExtent.h>
#include <ImagePP/all/h/HVE3DPolyLine.h>
#include <STMInternal/Storage/IDTMFileDirectories/FeatureHeaderTypes.h>





/** -----------------------------------------------------------------------------

    This class implements a 3D polyline which is a simple chain of 3D points.
    The present implementation is not intended as a full 3D implementation
    but does provide a rich set of 2D operations.
    -----------------------------------------------------------------------------
*/
class HVEDTMLinearFeature : public HVE3DPolyLine
    {


public:


    // Primary methods
    HVEDTMLinearFeature(size_t i_rCapacity = 0);
    HVEDTMLinearFeature(IDTMFile::FeatureType featureType,
                        size_t i_rCapacity = 0);
#if (0)
    HVEDTMLinearFeature(const HGF3DPoint& i_rStartPoint,
                        const HGF3DPoint& i_rEndPoint);
    HVEDTMLinearFeature(const HGF3DPointCollection& i_rListOfPoints);
#endif
    HVEDTMLinearFeature(IDTMFile::FeatureType featureType,
                        const HGF3DPointCollection& i_rListOfPoints,
                        const HFCPtr<HGF2DCoordSys>& i_rpCoordSys);
    HVEDTMLinearFeature(IDTMFile::FeatureType featureType,
                        const HFCPtr<HGF2DCoordSys>& i_rpCoordSys,
                        size_t i_rCapacity = 0);
    HVEDTMLinearFeature(const HVEDTMLinearFeature&       i_rObject);
    virtual            ~HVEDTMLinearFeature();

    HVEDTMLinearFeature&  operator=(const HVEDTMLinearFeature& i_rObj);

    // New interface
    IDTMFile::FeatureType    GetFeatureType() const;
    // Override of HVE3DPolyLine
    HVE3DPolyLine*     Clone() const;
protected:


private:

    void               ValidateInvariants() const
        {
        HVE3DPolyLine::ValidateInvariants();
        }

    IDTMFile::FeatureType m_featureType;
    };


#include "HVEDTMLinearFeature.hpp"

