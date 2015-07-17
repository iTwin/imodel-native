/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/GeoCoords/GCS.cpp $
|    $RCSfile: GCS.cpp,v $
|   $Revision: 1.30 $
|       $Date: 2011/12/01 18:51:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <ScalableTerrainModel/GeoCoords/LocalTransform.h>

#include <ScalableTerrainModel/Foundations/Log.h>
#include <ScalableTerrainModel/Foundations/Exception.h>

#include <STMInternal/Foundations/PrivateStringTools.h>

#include "WktUtils.h"
#include "GCSWktParsing.h"


BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE


inline GeospatialReference::GeospatialReference (const BaseGCSCPtr& basePtr)
    :   m_implP(0),
        m_basePtr(basePtr)
    {
    
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GeospatialReference& GeospatialReference::GetNull ()
    {
    static const GeospatialReference NULL_INSTANCE(0);
    return NULL_INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GeospatialReference::~GeospatialReference ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GeospatialReference::GeospatialReference (const GeospatialReference& rhs)
    :   m_implP(0),
        m_basePtr(rhs.m_basePtr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GeospatialReference& GeospatialReference::operator= (const GeospatialReference& rhs)
    {
    m_implP = 0;
    m_basePtr = rhs.m_basePtr;
    return *this;
    }

const Unit& Unit::GetMeter ()           { static const Unit METER = CreateLinearFrom(L"meter", 1.0);        return METER; }
const Unit& Unit::GetRadian ()          { static const Unit RADIAN = CreateAngularFrom(L"radian", 1.0);        return RADIAN; }
const Unit& Unit::GetDegree ()          { static const Unit DEGREE = CreateFromDegreeBased(L"degree", 1.0);     return DEGREE; }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::CreateLinearFrom    (const WChar*  name,
                                double          ratioToMeter)
    {
    return Unit(name, BASEID_METER, ratioToMeter);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::CreateAngularFrom   (const WChar*     name,
                                double          ratioToRadian)
    {
    return Unit(name, BASEID_RADIAN, ratioToRadian);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::CreateFromDegreeBased   (const WChar* name,
                                    double      ratioToDegree)
    {
    const double ratioToRadian(ratioToDegree*msGeomConst_radiansPerDegree);
    return Unit(name, BASEID_RADIAN, ratioToRadian);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit (const WChar*  name,
            BaseID          baseID,
            double          ratioToBase)
    :   m_nameP(new WString(name)),
        m_baseID(baseID),
        m_ratioToBase(ratioToBase)

    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::~Unit ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit (const Unit& rhs)
    :   m_nameP(new WString(*rhs.m_nameP)),
        m_baseID(rhs.m_baseID),
        m_ratioToBase(rhs.m_ratioToBase)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit& Unit::operator= (const Unit& rhs)
    {
    *m_nameP = *rhs.m_nameP;
    m_baseID = rhs.m_baseID;
    m_ratioToBase = rhs.m_ratioToBase;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* Unit::GetNameCStr () const
    {
    return m_nameP->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& Unit::GetName () const
    {
    return *m_nameP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Unit& Unit::GetBase () const
    {
    switch (m_baseID)
        {
    case BASEID_METER:
        return GetMeter();
    case BASEID_RADIAN:
        return GetDegree();
    default:
        assert(!"Unexpected!");
        return GetMeter();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsLinear () const
    {
    return BASEID_METER == m_baseID; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsAngular () const
    {
    return BASEID_RADIAN == m_baseID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsEquivalent (const Unit& rhs) const
    {
    return m_baseID == rhs.m_baseID &&
           EqEps(m_ratioToBase, rhs.m_ratioToBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit GetUnitFor (const BaseGCS& baseGCS)
    {
    // User must provide a valid base GCS.
    assert(baseGCS.IsValid());

    typedef Bentley::GeoCoordinates::Unit GeoCoordUnit;
    WString units;
    const GeoCoordUnit* geoCoordUnitP = GeoCoordUnit::FindUnit(baseGCS.GetUnits(units));

    if (0 == geoCoordUnitP)
        {
        assert(!"Unexpected!");
        return Unit::GetMeter();
        }

    switch (geoCoordUnitP->GetBase())
        {
    case Bentley::GeoCoordinates::GeoUnitBase::Meter:
        return Unit::CreateLinearFrom(geoCoordUnitP->GetName(), geoCoordUnitP->GetConversionFactor());
    case Bentley::GeoCoordinates::GeoUnitBase::Degree:
        return Unit::CreateFromDegreeBased(geoCoordUnitP->GetName(), geoCoordUnitP->GetConversionFactor());
    default:
        assert(!"Unexpected!");
        return Unit::GetMeter();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HaveCompatibleUnits   (const GCS&  lhs,
                            const GCS&  rhs)
    {
    return lhs.GetHorizontalUnit().GetBaseID() == rhs.GetHorizontalUnit().GetBaseID() &&
           lhs.GetVerticalUnit().GetBaseID() == rhs.GetVerticalUnit().GetBaseID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HaveEquivalentUnits   (const GCS&  lhs,
                            const GCS&  rhs)
    {
    return lhs.GetHorizontalUnit().IsEquivalent(rhs.GetHorizontalUnit()) &&
           lhs.GetVerticalUnit().IsEquivalent(rhs.GetVerticalUnit());
    }


namespace {

inline double CreateUnitRectificationScale (const Unit& sourceUnit,
                                            const Unit& targetUnit,
                                            double      angularToLinearRatio)
    {
    // Bring both side to linear equivalent
    const double targetRatioToBase = targetUnit.IsAngular() ? 
                                        targetUnit.GetRatioToBase() * angularToLinearRatio : 
                                        targetUnit.GetRatioToBase();
    const double sourceRatioToBase = sourceUnit.IsAngular() ? 
                                        sourceUnit.GetRatioToBase() * angularToLinearRatio : 
                                        sourceUnit.GetRatioToBase();

    return sourceRatioToBase / targetRatioToBase;
    }

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double GetUnitRectificationScaleFactor  (const Unit& sourceUnit,
                                            const Unit& targetUnit,
                                            double      angularToLinearRatio)
    {
    return CreateUnitRectificationScale(sourceUnit, targetUnit, angularToLinearRatio);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetUnitRectificationTransfoModel  (const Unit& sourceUnit,
                                                const Unit& targetUnit,
                                                double      angularToLinearRatio)
    {
    const double sourceToTargetRatio = CreateUnitRectificationScale(sourceUnit, targetUnit, angularToLinearRatio);

    if (EqOneEps(sourceToTargetRatio))
        return TransfoModel::GetIdentity();

    return TransfoModel::CreateScalingFrom(sourceToTargetRatio);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetUnitRectificationTransfoModel      (const Unit& sourceHorizontalUnit,
                                                    const Unit& sourceVerticalUnit,
                                                    const Unit& targetHorizontalUnit,
                                                    const Unit& targetVerticalUnit,
                                                    double      angularToLinearRatio)
    {
    const double horizontalSourceToTargetRatio = CreateUnitRectificationScale(sourceHorizontalUnit, targetHorizontalUnit, angularToLinearRatio);
    const double verticalSourceToTargetRatio = CreateUnitRectificationScale(sourceVerticalUnit, targetVerticalUnit, angularToLinearRatio);

    if (EqOneEps(horizontalSourceToTargetRatio) && EqOneEps(verticalSourceToTargetRatio))
        return TransfoModel::GetIdentity();

    return TransfoModel::CreateScalingFrom(horizontalSourceToTargetRatio, 
                                           horizontalSourceToTargetRatio,
                                           verticalSourceToTargetRatio);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetUnitRectificationTransfoMatrix    (const Unit&     sourceUnit,
                                                    const Unit&     targetUnit,
                                                    double          angularToLinearRatio)
    {
    const double sourceToTargetRatio = CreateUnitRectificationScale(sourceUnit, targetUnit, angularToLinearRatio);

    if (EqOneEps(sourceToTargetRatio))
        return TransfoMatrix::GetIdentity();

    return TransfoMatrix   (sourceToTargetRatio,   0.0,                0.0,                0.0,
                            0.0,                sourceToTargetRatio,   0.0,                0.0,
                            0.0,                0.0,                sourceToTargetRatio,   0.0);

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetUnitRectificationTransfoMatrix    (const Unit&     sourceHorizontalUnit,
                                                    const Unit&     sourceVerticalUnit,
                                                    const Unit&     targetHorizontalUnit,
                                                    const Unit&     targetVerticalUnit,
                                                    double          angularToLinearRatio)
    {
    const double horizontalSourceToTargetRatio = CreateUnitRectificationScale(sourceHorizontalUnit, targetHorizontalUnit, angularToLinearRatio);
    const double verticalSourceToTargetRatio = CreateUnitRectificationScale(sourceVerticalUnit, targetVerticalUnit, angularToLinearRatio);

    if (EqOneEps(horizontalSourceToTargetRatio) && EqOneEps(verticalSourceToTargetRatio))
        return TransfoMatrix::GetIdentity();

    return TransfoMatrix   (horizontalSourceToTargetRatio, 0.0,                        0.0,                        0.0,
                            0.0,                        horizontalSourceToTargetRatio, 0.0,                        0.0,
                            0.0,                        0.0,                        verticalSourceToTargetRatio,   0.0);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCS::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    GeospatialReference                     m_geoRef;

    bool                                    m_hasUnit;
    Unit                                    m_horizontalUnit;

    bool                                    m_hasLocalTransform;
    LocalTransform                          m_localTransform;
    
    explicit                                Impl                           (const GeospatialReference&          geoRef,
                                                                            const Unit*                         unitP,
                                                                            const LocalTransform*               localTransformP) 
        :   m_geoRef(geoRef),
            m_hasUnit(0 != unitP),
            m_horizontalUnit(m_hasUnit ? *unitP : Unit::GetMeter()) ,
            m_hasLocalTransform(0 != localTransformP),
            m_localTransform(m_hasLocalTransform ? *localTransformP : LocalTransform::GetIdentity())
        {
        }

    explicit                                Impl                           (const BaseGCSCPtr&                  baseGCSPtr,
                                                                            const Unit*                         unitP,
                                                                            const LocalTransform*               localTransformP) 
        :   m_geoRef(baseGCSPtr),
            m_hasUnit(0 != unitP),
            m_horizontalUnit(m_hasUnit ? *unitP : Unit::GetMeter()) ,
            m_hasLocalTransform(0 != localTransformP),
            m_localTransform(m_hasLocalTransform ? *localTransformP : LocalTransform::GetIdentity())
        {
        }


    static void                             UpdateForEdit                  (ImplPtr&                            instancePtr)
        {
        // Copy on write when config is shared
        if (instancePtr->IsShared())
            instancePtr = new Impl(*instancePtr);
        }

    WKTSupport                              ComputeWKTSupport              () const;



    Status                                  GetNullCSWKT                   (WString&                             wkt) const;
    Status                                  GetBaseCSWKT                   (WString&                             wkt) const;
    Status                                  GetLocalCSWKT                  (WString&                             wkt) const;
    Status                                  GetFittedCSWKT                 (WString&                             wkt) const;

    Status                                  GetWKT                         (WString&                             wkt) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GCS& GCS::GetNull ()
    {
    static const GCS NULL_GCS(new Impl(0, 0, 0));
    return NULL_GCS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::GCS (Impl* pi_pImpl)
    :   m_implP(pi_pImpl)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::~GCS ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::GCS (const GCS& pi_rRight)
    :   m_implP(pi_rRight.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GCS& GCS::operator= (const GCS& pi_rRight)
    {
    m_implP = pi_rRight.m_implP;
    return *this;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline GCS::Status GCS::Impl::GetNullCSWKT (WString& wkt) const
    {
    wkt.clear();
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::Status GCS::Impl::GetBaseCSWKT (WString& wkt) const
    {            
    if (BSISUCCESS != m_geoRef.GetBase().GetWellKnownText(wkt, BaseGCS::wktFlavorAutodesk))
        return S_ERROR;  
     
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::Status GCS::Impl::GetLocalCSWKT (WString& wkt) const
    {
    assert(m_hasUnit);

    if (m_horizontalUnit.IsLinear())
        {
        GetLocalCsWkt(wkt, m_horizontalUnit);
        }
    else // Angular units
        {
        // Vertical units always meters for the moment as there is no way to retrieve this kind of information from base CS.
        GetLocalCsWkt(wkt, m_horizontalUnit, Unit::GetMeter());
        }

    return S_SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::Status GCS::Impl::GetFittedCSWKT (WString& wkt) const
    {
    assert(m_hasLocalTransform);

    WString baseCSWkt;
    const Status baseCSStatus = (!m_geoRef.IsNull()) ? 
                                        GetBaseCSWKT(baseCSWkt) : 
                                        GetLocalCSWKT(baseCSWkt);
    if (S_SUCCESS != baseCSStatus)
        return baseCSStatus;

    TransfoMatrix transform;
    bool transformIsToBase = true;

    if (m_localTransform.HasToGlobal() && m_localTransform.GetToGlobal().IsConvertibleToMatrix())
        {
        transform = m_localTransform.GetToGlobal().ConvertToMatrix();
        }
    else if (m_localTransform.HasToLocal() && m_localTransform.GetToLocal().IsConvertibleToMatrix())
        {
        assert(m_localTransform.HasToLocal());
        transform = m_localTransform.GetToLocal().ConvertToMatrix();
        transformIsToBase = false;
        }
    else
        {
        return S_ERROR; 
        }

    GetFittedCsWkt(wkt, transform, transformIsToBase, baseCSWkt);
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::Status GCS::Impl::GetWKT (WString& wkt) const
    {
    if (m_hasLocalTransform)
        return GetFittedCSWKT(wkt);

    if (m_geoRef.IsNull())
        {
        return (m_hasUnit) ? 
                    GetLocalCSWKT(wkt) : 
                    GetNullCSWKT(wkt);
        }

    return GetBaseCSWKT(wkt);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT GCS::GetWKT () const
    {
    WString wktString;
    m_implP->GetWKT(wktString);
    return WKT(wktString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT GCS::GetWKT (Status& status) const
    {
    WString wktString;
    status = m_implP->GetWKT(wktString);
    return WKT(wktString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::IsNull () const
    {
    // When no units can be found, system is null
    return !m_implP->m_hasUnit; 
    }





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::HasGeospatialReference () const
    {
    return !m_implP->m_geoRef.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::HasGeoRef () const
    {
    return !m_implP->m_geoRef.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GeospatialReference& GCS::GetGeospatialReference () const
    {
    assert(!m_implP->m_geoRef.IsNull());
    return m_implP->m_geoRef;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const GeoRef& GCS::GetGeoRef () const
    {
    return m_implP->m_geoRef;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::HasUniformUnits () const
    {
    return !m_implP->m_horizontalUnit.IsAngular();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Unit& GCS::GetUnit () const 
    { 
    assert(m_implP->m_hasUnit); 
    return m_implP->m_horizontalUnit; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Unit& GCS::GetHorizontalUnit () const
    {
    assert(m_implP->m_hasUnit); 
    return m_implP->m_horizontalUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Unit& GCS::GetVerticalUnit () const
    {
    assert(m_implP->m_hasUnit); 
    return m_implP->m_horizontalUnit.IsLinear() ? m_implP->m_horizontalUnit : Unit::GetMeter();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::HasLocalTransform () const
    {
    return m_implP->m_hasLocalTransform;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalTransform& GCS::GetLocalTransform () const
    { 
    return m_implP->m_localTransform; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void GCS::SetLocalTransform (const LocalTransform& localTransform)
    {
    assert(!IsNull());

    Impl::UpdateForEdit(m_implP);
    m_implP->m_localTransform = localTransform;
    m_implP->m_hasLocalTransform = !m_implP->m_localTransform.IsIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS::Status GCS::AppendLocalTransform (const LocalTransform& localTransform)
    {
    assert(!IsNull());

    LocalTransform::Status status;
    LocalTransform newLocalTransform(Combine(m_implP->m_localTransform, localTransform, status));

    if (LocalTransform::S_SUCCESS != status)
        return S_ERROR;

    Impl::UpdateForEdit(m_implP);
    swap(m_implP->m_localTransform, newLocalTransform);
    m_implP->m_hasLocalTransform = !m_implP->m_localTransform.IsIdentity();

    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCS::IsEquivalent (const GCS& rhs) const
    {
    if (HasGeoRef())
        {
        bool areEquivalent = rhs.HasGeoRef() && 
                             GetGeoRef().GetBase().IsEquivalent(rhs.GetGeoRef().GetBase());

        if (HasLocalTransform() || rhs.HasLocalTransform())
            areEquivalent &= m_implP->m_localTransform.IsEquivalent(rhs.m_implP->m_localTransform);

        return areEquivalent;
        }
    else if (!IsNull())
        {
        bool areEquivalent = !rhs.IsNull() && 
                             GetUnit().IsEquivalent(rhs.GetUnit());

        if (HasLocalTransform() || rhs.HasLocalTransform())
            areEquivalent &= m_implP->m_localTransform.IsEquivalent(rhs.m_implP->m_localTransform);

        return areEquivalent;
        }
    else // IsNull
        {
        return rhs.IsNull();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void swap  (GCS&    lhs,
            GCS&    rhs)
    {
    using std::swap;
    swap(lhs.m_implP, rhs.m_implP);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCSFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    Log&                            m_warningLog;
    bool                            m_throwOnUnhandledErrors;


    explicit                        Impl                               (Log& log)
        :   m_warningLog(log),
            m_throwOnUnhandledErrors(true) // TDORAY: Extract from policy when one is available
        {
        }

    void                            OnUnhandledStatus                  (Status                          status)
        {
        assert(BSISUCCESS != status);

        // TDORAY: Evaluate different statuses
        CustomError error(L"Error creating gcs!");
        if (m_throwOnUnhandledErrors)
            throw error;
        else
            m_warningLog.Add(error);
        }

    template <typename Arg1T, typename Arg2T>
    GCS                             CreateWithUnhandledStatus          (const GCSFactory&               instance,
                                                                        Arg1T                           arg1,
                                                                        Arg2T                           arg2)
        {
        Status status(S_SUCCESS);
        GCS gcs(instance.Create(arg1, arg2, status));
        if (S_SUCCESS != status) OnUnhandledStatus(status);
        return gcs;
        }

    template <typename Arg1T>
    GCS                             CreateWithUnhandledStatus          (const GCSFactory&               instance,
                                                                        Arg1T                           arg1)
        {
        Status status(S_SUCCESS);
        GCS gcs(instance.Create(arg1, status));
        if (S_SUCCESS != status) OnUnhandledStatus(status);
        return gcs;
        }


    GCS                             CreateFromBaseCS                   (const WChar*                     wkt,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;    

    GCS                             CreateFromLocalCS                  (const WChar*                     wktBegin,
                                                                        const WChar*                     wktEnd,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;

    GCS                             CreateFromFittedCS                 (const WChar*                     wktBegin,
                                                                        const WChar*                     wktEnd,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;


    GCS                             CreateFromComposedCS               (const WChar*                     wktBegin,
                                                                        const WChar*                     wktEnd,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;


    GCS                             Create                             (const WChar*                     wktBegin,
                                                                        const WChar*                     wktEnd,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;

    GCS                             Create                             (const WChar*                     wkt,
                                                                        BaseGCS::WktFlavor              wktFlavor,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;

    GCS                             Create                             (const BaseGCSCPtr&              baseGCSPtr,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;

    GCS                             Create                             (const GeospatialReference&      geoRef,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;

    GCS                             Create                             (const Unit&                     unit,
                                                                        const LocalTransform*           localTransformP,
                                                                        Status&                         status) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSFactory::GCSFactory (Log& log)
    :   m_implP(new Impl(log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSFactory::~GCSFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCSFactory::GCSFactory (const GCSFactory& rhs)
    :   m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::CreateFromBaseCS (const WChar*             wkt,                                        
                                        BaseGCS::WktFlavor      wktFlavor,
                                        const LocalTransform*   localTransformP,
                                        Status&                 status) const
    {
    BaseGCSPtr gcsPtr(BaseGCS::CreateGCS());     
    WString w_wkt(wkt);
    StatusInt initWarningCode = BSISUCCESS;
    const StatusInt initStatus = gcsPtr->InitFromWellKnownText(&initWarningCode, 0, wktFlavor, w_wkt.GetWCharCP());
    
    if (BSISUCCESS != initStatus)
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    return Create(gcsPtr, localTransformP, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::CreateFromLocalCS(const WChar*             wktBegin,
                                        const WChar*             wktEnd,
                                        BaseGCS::WktFlavor      wktFlavor,
                                        const LocalTransform*   localTransformP,
                                        Status&                 status) const
    {
    WKTRoot root(wktBegin, wktEnd);
    if (WKTRoot::S_SUCCESS != root.Parse() || root.IsEmpty())
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    if (!HasBentleyAsAuthority(root.GetSection()))
        {
        assert(!"Possibly invalid Wkt flavor!");
        // This is not ours. Try extracting GCS using BaseCS facilities.
        return CreateFromBaseCS(wktBegin, wktFlavor, localTransformP, status); 
        }

    Unit unit(Unit::GetMeter());

    if (!ExtractLocalCS(root.GetSection(), unit))
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    return Create(unit, localTransformP, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::CreateFromComposedCS (const WChar*             wktBegin,
                                            const WChar*             wktEnd,
                                            BaseGCS::WktFlavor      wktFlavor,
                                            const LocalTransform*   localTransformP,
                                            Status&                 status) const
    {
    WKTRoot root(wktBegin, wktEnd);
    if (WKTRoot::S_SUCCESS != root.Parse() || root.IsEmpty())
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    const WKTSection& composedCSSection = root.GetSection();

    if (!HasBentleyAsAuthority(composedCSSection))
        {
        assert(!"Possibly invalid Wkt flavor!");
        // This is not ours. Try extracting GCS using BaseCS facilities.
        return CreateFromBaseCS(wktBegin, wktFlavor, localTransformP, status); 
        }


    Unit horizontalUnit(Unit::GetDegree());
    Unit verticalUnit(Unit::GetMeter());

    if (!ExtractLocalComposedCS(root.GetSection(), horizontalUnit, verticalUnit))
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    if (!verticalUnit.IsEquivalent(Unit::GetMeter()))
        {
        assert(!"It may be time that we really supports vertical units other then meter!");
        status = S_ERROR;
        return GCS::GetNull ();
        }

    // TDORAY: Use a creator that supports specifying vertical units so that we can supports
    // units other than meters.
    return Create(horizontalUnit, localTransformP, status);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::CreateFromFittedCS   (const WChar*             wktBegin,
                                            const WChar*             wktEnd,
                                            BaseGCS::WktFlavor      wktFlavor,
                                            const LocalTransform*   localTransformP,
                                            Status&                 status) const
    {
    WKTRoot root(wktBegin, wktEnd);
    if (WKTRoot::S_SUCCESS != root.Parse() || root.IsEmpty())
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    const WKTSection& fittedCSSection = root.GetSection();

    if (!HasBentleyAsAuthority(fittedCSSection))
        {
        assert(!"Possibly invalid Wkt flavor!");
        // This is not ours. Try extracting GCS using BaseCS facilities.
        return CreateFromBaseCS(wktBegin, wktFlavor, localTransformP, status); 
        }

    TransfoMatrix       transform;
    bool                transformIsToBase = false;
    const WKTParameter* baseCsWktParameter = 0;


    if (!ExtractFittedCS(fittedCSSection, transform, transformIsToBase, baseCsWktParameter))
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }


    TransfoModel::Status transfoModelCreateStatus;
    TransfoModel transfoModel(TransfoModel::CreateFrom(transform, transfoModelCreateStatus));

    if (TransfoModel::S_SUCCESS != transfoModelCreateStatus)
        {
        status = S_ERROR;
        return GCS::GetNull ();
        }

    LocalTransform localTransform(transformIsToBase ? 
                                    LocalTransform::CreateFromToGlobal(transfoModel) :
                                    LocalTransform::CreateFromToLocal(transfoModel));

    assert(0 != baseCsWktParameter);
    return Create(baseCsWktParameter->strBegin(), baseCsWktParameter->strEnd(), wktFlavor, &localTransform, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::Create   (const WChar*             wktBegin,
                                const WChar*             wktEnd,
                                BaseGCS::WktFlavor      wktFlavor,
                                const LocalTransform*   localTransformP,
                                Status&                 status) const
    {
    const WChar* firstKeywordBegin = FindWKTSectionKeyword(wktBegin, wktEnd);
    if (wktEnd == firstKeywordBegin) 
        {
        // Empty wkt.
        status = S_SUCCESS;
        return GCS::GetNull ();
        }

    const WKTKeyword& firstKeyword = GetWKTKeyword(firstKeywordBegin);

    switch (firstKeyword.type)
        {
    case WKTKeyword::TYPE_LOCAL_CS:         
        return CreateFromLocalCS(wktBegin, wktEnd, wktFlavor, localTransformP, status);
    case WKTKeyword::TYPE_FITTED_CS:
        return CreateFromFittedCS(wktBegin, wktEnd, wktFlavor, localTransformP, status);
    case WKTKeyword::TYPE_COMPD_CS:
        return CreateFromComposedCS(wktBegin, wktEnd, wktFlavor, localTransformP, status);
    case WKTKeyword::TYPE_NULL:
        status = S_SUCCESS;
        return GCS::GetNull ();
        }

    assert(WKTKeyword::TYPE_UNKNOWN == firstKeyword.type);
    return CreateFromBaseCS(wktBegin, wktFlavor, localTransformP, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline GCS GCSFactory::Impl::Create    (const WChar*             wkt,
                                        BaseGCS::WktFlavor      wktFlavor,
                                        const LocalTransform*   localTransformP,
                                        Status&                 status) const
    {
    const WChar* const wktEnd = wkt + wcslen(wkt);
    return Create(wkt, wktEnd, wktFlavor, localTransformP, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::Create   (const BaseGCSCPtr&      baseGCSPtr,
                                const LocalTransform*   localTransformP,
                                Status&                 status) const
    {
    if (baseGCSPtr == 0 || !baseGCSPtr->IsValid())
        {
        status = (baseGCSPtr == 0) ? S_SUCCESS : S_ERROR; // TDORAY: Need better diagnosis error
        return GCS::GetNull();
        }

    status = S_SUCCESS;
    Unit baseUnit = GetUnitFor (*baseGCSPtr);
    return GCS(new GCS::Impl(baseGCSPtr, &baseUnit, localTransformP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Impl::Create   (const GeospatialReference&      geoRef,
                                const LocalTransform*           localTransformP,
                                Status&                         status) const
    {
    assert(!geoRef.IsNull());

    status = S_SUCCESS;
    Unit baseUnit = GetUnitFor (geoRef.GetBase());
    return GCS(new GCS::Impl(geoRef, &baseUnit, localTransformP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline GCS GCSFactory::Impl::Create    (const Unit&             unit,
                                        const LocalTransform*   localTransformP,
                                        Status&                 status) const
    {
    status = S_SUCCESS;
    return GCS(new GCS::Impl(0, &unit, localTransformP));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const WChar*  wkt,
                        Status&         status) const
    {
    BaseGCS::WktFlavor wktFlavor = BaseGCS::wktFlavorOracle9; 

    return m_implP->Create(wkt, wktFlavor, 0, status);
    }    

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const wchar_t*              wkt,
                        BaseGCS::WktFlavor          wktFlavor,
                        Status&                     status) const
    {
    return m_implP->Create(wkt, wktFlavor, 0, status);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const WKT&  wkt,
                        Status&     status) const
    {
    BaseGCS::WktFlavor wktFlavor = BaseGCS::wktFlavorOracle9; 

    return m_implP->Create(wkt.GetCStr(), wktFlavor, 0, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const BaseGCSCPtr&  baseGCSPtr,
                        Status&             status) const
    {
    return m_implP->Create(baseGCSPtr, 0, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GeospatialReference&  geoRef,
                        Status&                     status) const
    {
    return m_implP->Create(geoRef, 0, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const Unit& unit,
                        Status&     status) const
    {
    return m_implP->Create(unit, 0, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GCS&              gcs,
                        const LocalTransform&   appendedLocalTransform,
                        Status&                 status) const
    {
    LocalTransform::Status createTransfromStatus = LocalTransform::S_SUCCESS;

    const LocalTransform localTransform(gcs.HasLocalTransform() ? 
                                            Combine(gcs.GetLocalTransform(), appendedLocalTransform, createTransfromStatus) : 
                                            appendedLocalTransform);

    if (LocalTransform::S_SUCCESS != createTransfromStatus)
        {
        status = S_ERROR;
        return GCS::GetNull();
        }

    // TDORAY: If this becomes false, we will need to creates local cs another way...
    assert(gcs.GetHorizontalUnit().IsLinear() || gcs.GetVerticalUnit().IsEquivalent(Unit::GetMeter()));

    return (gcs.HasGeoRef()) ? 
                Create(gcs.GetGeoRef(), localTransform, status) :
                Create(gcs.GetUnit(), localTransform, status);
    }

namespace {

inline const LocalTransform* GetValidLocalTransformP (const LocalTransform&   localTransform)
    {
    return (localTransform.IsIdentity() ? 0 : &localTransform);
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const BaseGCSCPtr&      baseGCSPtr,
                        const LocalTransform&   localTransform,
                        Status&                 status) const
    {
    return m_implP->Create(baseGCSPtr, 
                           GetValidLocalTransformP(localTransform), 
                           status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GeospatialReference&  geoRef,
                        const LocalTransform&       localTransform,
                        Status&                     status) const
    {
    return m_implP->Create(geoRef, 
                           GetValidLocalTransformP(localTransform), 
                           status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const Unit&             unit,
                        const LocalTransform&   localTransform,
                        Status&                 status) const
    {
    return m_implP->Create(unit, 
                           GetValidLocalTransformP(localTransform), 
                           status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const WChar* wkt) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, wkt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const WKT& wkt) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, wkt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const BaseGCSCPtr& baseGCSPtr) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, baseGCSPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GeospatialReference& geoRef) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, geoRef);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const Unit& unit) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, unit);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const BaseGCSCPtr&      baseGCSPtr,
                        const LocalTransform&   localTransform) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, baseGCSPtr, localTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GeospatialReference&  geoRef,
                        const LocalTransform&       localTransform) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, geoRef, localTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const Unit&             unit,
                        const LocalTransform&   localTransform) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, unit, localTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GCSFactory::Create (const GCS&              gcs,
                        const LocalTransform&   appendedLocalTransform) const
    {
    return m_implP->CreateWithUnhandledStatus(*this, gcs, appendedLocalTransform);
    }



namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKTSupport ComputeWKTSupport (const GCS&         gcs, 
                              BaseGCS::WktFlavor wktFlavor)
    {
    GCS::Status toStatus;

    WKT wkt(gcs.GetWKT(toStatus));

    if (GCS::S_SUCCESS != toStatus)
        return WKTSUPPORT_NONE; // TDORAY: As we may have been created from WKT, we may return that we are FROM compliant.


    // TDORAY: This step could be skipped if we were created from WKT
    static const GCSFactory DEFAULT_FACTORY;


    GCSFactory::Status fromStatus;
    DEFAULT_FACTORY.Create(wkt.GetCStr(), wktFlavor, fromStatus);

    return (GCSFactory::S_SUCCESS == fromStatus) ? WKTSUPPORT_FULL : WKTSUPPORT_ONLY_TO;
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKTSupport GetWKTSupportFor (const GCS& gcs)
    {            
    //Use Autodesk.
    BaseGCS::WktFlavor wktFlavor = (BaseGCS::WktFlavor)8;     

    return GetWKTSupportFor (gcs, wktFlavor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WKTSupport GetWKTSupportFor (const GCS&                  gcs, 
                             BaseGCS::WktFlavor          wktFlavor)
    {
    return ComputeWKTSupport(gcs, wktFlavor);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT::WKT ()
    :   m_wktP(new WString)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT::WKT (const WChar* wkt)
    :   m_wktP(new WString(wkt))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT::~WKT ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT::WKT (const WKT& rhs)
    :   m_wktP(new WString(*rhs.m_wktP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT& WKT::operator= (const WKT& rhs)
    {
    *m_wktP = *rhs.m_wktP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKT& WKT::operator= (const WChar* wkt)
    {
    *m_wktP = wkt;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool WKT::IsEmpty () const
    {
    return m_wktP->empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* WKT::GetCStr () const
    {
    return m_wktP->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& WKT::Get () const
    {
    return *m_wktP;
    }

END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE