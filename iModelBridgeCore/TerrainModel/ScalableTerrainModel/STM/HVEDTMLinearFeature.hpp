//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HVEDTMLinearFeature.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::HVEDTMLinearFeature(size_t i_rCapacity)
    : HVE3DPolyLine(i_rCapacity)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::HVEDTMLinearFeature(IDTMFile::FeatureType featureType,
                                                size_t          i_rCapacity)
    : HVE3DPolyLine(i_rCapacity),
      m_featureType(featureType)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system only)
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::HVEDTMLinearFeature(IDTMFile::FeatureType                 featureType,
                                                const HFCPtr<HGF2DCoordSys>&    i_rpCoordSys,
                                                size_t                          i_rCapacity)
    : HVE3DPolyLine(i_rpCoordSys, i_rCapacity),
      m_featureType(featureType)
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system and initial list of points)
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::HVEDTMLinearFeature(IDTMFile::FeatureType featureType,
                                                const HGF3DPointCollection& i_rListOfPoints,
                                                const HFCPtr<HGF2DCoordSys>& i_rpCoordSys)
    : HVE3DPolyLine(i_rListOfPoints, i_rpCoordSys),
      m_featureType(featureType)
    {
    HINVARIANTS;
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVEDTMLinearFeature object.
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::HVEDTMLinearFeature(const HVEDTMLinearFeature& i_rObj)
    : HVE3DPolyLine(i_rObj),
      m_featureType(i_rObj.m_featureType)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature::~HVEDTMLinearFeature()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polysegment.
//-----------------------------------------------------------------------------
inline HVEDTMLinearFeature& HVEDTMLinearFeature::operator=(const HVEDTMLinearFeature& i_rObj)
    {
    // Check that given is not self
    if (&i_rObj != this)
        {
        HVE3DPolyLine::operator=(i_rObj);
        m_featureType = i_rObj.m_featureType;
        HINVARIANTS;
        }

    // Return reference to self
    return (*this);
    }



//-----------------------------------------------------------------------------
// AppendPoint
// Adds a point at the end of polyline
//-----------------------------------------------------------------------------
inline IDTMFile::FeatureType HVEDTMLinearFeature::GetFeatureType() const
    {
    HINVARIANTS;

    return m_featureType;
    }

