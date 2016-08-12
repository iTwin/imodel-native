/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/Reprojection.cpp $
|    $RCSfile: Reprojection.cpp,v $
|   $Revision: 1.32 $
|       $Date: 2011/11/23 21:47:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Foundations/Log.h>
#include <ScalableMesh/Foundations/Warning.h>
#include <ScalableMesh/Foundations/Error.h>

#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>

#include <ScalableMesh/GeoCoords/Transformation.h>
#include <ScalableMesh/GeoCoords/LocalTransform.h>

#include "TransformationUtils.h"
#include "TransformationInternal.h"
#include "TransfoModelMixinBase.h"


#define INCLUDED_FROM_TRANSFORMATION_CPP 1
#include "TransfoModelIdentity.hpp"
#include "TransfoModelUniformScaling.hpp"
#include "TransfoModelCombined.hpp"

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

using namespace Internal;

namespace { // BEGIN unnamed namespace


/*
 * Reprojection warnings
 */
struct ReprojectionWarning : public WarningMixinBase<ReprojectionWarning>
    {
    explicit    ReprojectionWarning                    (StatusInt               errorCode)   
        :   super_class(L"Reprojection warning!", LEVEL_0, errorCode) {}
    };

const ReprojectionWarning DEFAULT_REPROJECTION_WARNING(BSIERROR);


static_assert(SMStatus::S_QTY == SMStatus::S_QTY, "Reprojection status must correspond transfo model status");

inline SMStatus GetReprojectionStatusFor(SMStatus status)
    {
    return status;
    }
} // END unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionPolicy::ReprojectionPolicy ()
    :   m_flags(0),
        m_angularToLinearUnitRatio(1.0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionPolicy::~ReprojectionPolicy ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionPolicy::ReprojectionPolicy (const ReprojectionPolicy& rhs)
    :   m_flags(rhs.m_flags),
        m_angularToLinearUnitRatio(rhs.m_angularToLinearUnitRatio),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionPolicy& ReprojectionPolicy::operator= (const ReprojectionPolicy& rhs)
    {
    m_flags = rhs.m_flags;
    m_angularToLinearUnitRatio = rhs.m_angularToLinearUnitRatio;
    return *this;
    }



namespace {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelReprojecting : public TransfoModelMixinBase<TransfoModelReprojecting>
    {
private:
    const BaseGCSCPtr                       m_sourceGCSP;
    const BaseGCSCPtr                       m_targetGCSP;
    Log&                                    m_warningLog;

    // TDORAY: Maybe need  to store the target content extent here. In lat long??

    // TDORAY: May optimize be approximating reprojection with matrix stored here....

    enum CSmapConvStatus
        {
        CSMAPCONV_S_OK = 0,
        CSMAPCONV_S_USFL = 1, // This is a warning
        CSMAPCONV_S_DOMN = 2,
        CSMAPCONV_S_ERR = 4096,
        };

    SMStatus                    HandleError(StatusInt                       csMapStatus) const
        {

        switch (csMapStatus)
            {
        case CSMAPCONV_S_USFL: // TDORAY: Find meaning of that. Ask Alain Robert.
            m_warningLog.Add(DEFAULT_REPROJECTION_WARNING);
            return SMStatus::S_SUCCESS;
        case CSMAPCONV_S_DOMN:
            return SMStatus::S_ERROR_DOES_NOT_FIT_MATHEMATICAL_DOMAIN;
        default:
            return SMStatus::S_ERROR;
            }
        }

    StatusInt                               Reproject                  (const DPoint3d&                 pi_rSourcePt,
                                                                        DPoint3d&                       po_rTargetPt,
                                                                        GeoPoint&                       po_rSourceLatLong,
                                                                        GeoPoint&                       po_rTargetLatLong,
                                                                        StatusInt                       currentStatus) const
        {
        const StatusInt statCToLL = m_sourceGCSP->LatLongFromCartesian (po_rSourceLatLong, pi_rSourcePt);

        // NTERAY: This member has mutable states that should probably be made mutable. Maybe there is a 
        // valid reason why this member is not const. Ask Barry Bentley.
        const StatusInt statLLToLL = const_cast<BaseGCS&>(*m_sourceGCSP).LatLongFromLatLong (po_rTargetLatLong, 
                                                                                             po_rSourceLatLong, 
                                                                                             *m_targetGCSP);

        const StatusInt statLLToC = m_targetGCSP->CartesianFromLatLong (po_rTargetPt, po_rTargetLatLong);


        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL]
        if (CSMAPCONV_S_OK != statCToLL && ((CSMAPCONV_S_OK == currentStatus) || (CSMAPCONV_S_USFL == currentStatus)))
            {
            if (0 > statCToLL) // If stat2 is negative ... this is the one ...
                currentStatus = statCToLL;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                currentStatus = (statCToLL > currentStatus ? statCToLL : currentStatus);
            }
        if ((CSMAPCONV_S_OK != statLLToLL) && ((CSMAPCONV_S_OK == currentStatus) || (CSMAPCONV_S_USFL == currentStatus))) // If stat2 has error and status not already hard error
            {
            if (0 > statLLToLL) // If stat2 is negative ... this is the one ...
                currentStatus = statLLToLL;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                currentStatus = (statLLToLL > currentStatus ? statLLToLL : currentStatus);
            }
        if ((CSMAPCONV_S_OK != statLLToC) && ((CSMAPCONV_S_OK == currentStatus) || (CSMAPCONV_S_USFL == currentStatus))) // If stat3 has error and status not already hard error
            {
            if (0 > statLLToC) // If stat3 is negative ... this is the one ...
                currentStatus = statLLToC;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                currentStatus = (statLLToC > currentStatus ? statLLToC : currentStatus);
            }

        return currentStatus;
        }


    virtual bool                        _IsConvertibleToMatrix         () const override
        {
        // TDORAY: May eventually work on an approximation. If it is the case, it will have to be clear for 
        // users of this interface that what is returned is only an approximation an maybe provide by the 
        // way, what level (in order of magnitude) of error is to be expected.
        return false;
        }

    virtual TransfoMatrix               _ConvertToMatrix               () const override
        {
        assert(!"Unexpected!");
        return TransfoMatrix::GetIdentity();
        }


    virtual SMStatus        _Transform(const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt) const override
        {
        GeoPoint sourceLatLong, targetLatLong;
        const StatusInt baseStatus = Reproject(sourcePt, targetPt, sourceLatLong, targetLatLong, BSISUCCESS);
        return (BSISUCCESS != baseStatus) ? HandleError(baseStatus) : SMStatus::S_SUCCESS;
        }


    virtual SMStatus        _Transform(const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP) const override
        {
        StatusInt baseStatus = BSISUCCESS;
        GeoPoint sourceLatLong, targetLatLong;
    
        const DPoint3d* const pSourcePtsEnd = sourcePtP + sourcePtQty;
        
        while (sourcePtP != pSourcePtsEnd)
            {
            baseStatus = Reproject(*sourcePtP, *targetPtP, sourceLatLong, targetLatLong, baseStatus);
            ++sourcePtP;
            ++targetPtP;
            }
        
        return (BSISUCCESS != baseStatus) ? HandleError(baseStatus) : SMStatus::S_SUCCESS;
        }

    virtual TransfoModelBase*           _CreateInverse                 () const override
        {
        // TDORAY: This may be possible if an interface for this action is provided by Reprojection.
        return 0;
        }

    explicit                                TransfoModelReprojecting   (const GCS&                      sourceGCS,
                                                                        const GCS&                      targetGCS,
                                                                        Log&                            log)
        :   m_sourceGCSP(sourceGCS.GetGeoRef().GetBasePtr()),
            m_targetGCSP(targetGCS.GetGeoRef().GetBasePtr()),
            m_warningLog(log)
        {
        
        }

public:
    static TransfoModelReprojecting*            CreateFrom             (const GCS&                      sourceGCS,
                                                                        const GCS&                      targetGCS,
                                                                        Log&                            log)
        {
        return new TransfoModelReprojecting(sourceGCS, targetGCS, log);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelUnitConverting : public TransfoModelUniformScaling
    {
private:
    explicit                                TransfoModelUnitConverting (const Unit&                     sourceUnit,
                                                                        const Unit&                     targetUnit,
                                                                        double                          angularToLinearRatio)
        :   TransfoModelUniformScaling(GetUnitRectificationScaleFactor(sourceUnit, targetUnit, angularToLinearRatio))
        {
        }
public:
    static TransfoModelUnitConverting*      CreateFrom                 (const Unit&                     sourceUnit,
                                                                        const Unit&                     targetUnit,
                                                                        double                          angularToLinearRatio)
        {
        return new TransfoModelUnitConverting(sourceUnit, targetUnit, angularToLinearRatio);
        }

    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectFromLocalToSourceWithGCSImpl : public TransfoModelCombined
    {
public:
    static TransfoModelBaseCPtr             CreateFrom                 (const TransfoModel&             localToSource,
                                                                        const Reprojection&             reprojection)
        {
        return Handler::GetBasePtr(Combine(localToSource, AsTransfoModel(reprojection)));
        }
    };

}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    friend struct ReprojectionFactory;

    Log&                        m_warningLog;

    struct Policy
        {
        explicit                Policy                     (const ReprojectionPolicy&       policy)
            :   allowConversionBetweenUnitBases(policy.IsConversionBetweenUnitBasesAllowed()),
                allowGCSToLCS(policy.IsGCSToLCSAllowed()),
                allowLCSToGCS(policy.IsLCSToGCSAllowed()),
                angularToLinearUnitRatio(policy.GetAngularToLinearUnitRatio()),
                throwOnUnhandledErrors(true) // TDORAY: Extract from policy when one is available
            {
            }

        bool                    allowConversionBetweenUnitBases;
        bool                    allowGCSToLCS;
        bool                    allowLCSToGCS;
        double                  angularToLinearUnitRatio;
        bool                    throwOnUnhandledErrors;

        }                       m_policy;


    void                            OnUnhandledStatus(SMStatus                          status)
        {
        assert(BSISUCCESS != status);

        // TDORAY: Evaluate different statuses
        CustomError error(L"Error creating reprojection!");
        if (m_policy.throwOnUnhandledErrors)
            throw error;
        else
            m_warningLog.Add(error);
        }

    explicit                    Impl                       (Log&                     log) 
        :   m_warningLog(log),
            m_policy(ReprojectionPolicy())
        {
        }
    explicit                    Impl                       (const ReprojectionPolicy&       policy,
                                                            Log&                            log)
        :   m_warningLog(log),
            m_policy(policy)
        {
        }

    Reprojection                Create                     (const GCS&                      sourceGCS,
                                                            const GCS&                      targetGCS,
                                                            const DRange3d*                 sourceExtentP,
                                                            SMStatus&                         status) const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFactory::ReprojectionFactory (Log& log)
    :   m_implP(new Impl(log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFactory::ReprojectionFactory (const ReprojectionPolicy&         policy,
                                          Log&                              log)
    :   m_implP(new Impl(policy, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFactory::~ReprojectionFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFactory::ReprojectionFactory (const ReprojectionFactory& rhs)
    :   m_implP(rhs.m_implP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection ReprojectionFactory::Impl::Create (const GCS&      sourceGCS,
                                                const GCS&      targetGCS,
                                                const DRange3d* sourceExtentP,
                                                SMStatus&         status) const
    {
    status = S_SUCCESS;

    // TDORAY: Consider converting all possible cases to an enum

    // TDORAY: Make a case like with local transforms?

    if (sourceGCS.HasGeoRef() && targetGCS.HasGeoRef())
        {
        return Reprojection(TransfoModelReprojecting::CreateFrom(sourceGCS, targetGCS, m_warningLog));
        }
    else if (!sourceGCS.IsNull() && !targetGCS.IsNull())
        {
        // TDORAY: Raise a warning flag when one of these special cases happen.

        const bool baseCompatible = m_policy.allowConversionBetweenUnitBases ?
                                                            true :
                                                            HaveCompatibleUnits(sourceGCS, targetGCS);
   
        const bool fromSourceAuthorized = m_policy.allowGCSToLCS ? true : !sourceGCS.HasGeoRef();
        const bool toTargetAuthorized = m_policy.allowLCSToGCS ? true : !targetGCS.HasGeoRef();

        const bool unitConvertionAuthorized = baseCompatible && fromSourceAuthorized && toTargetAuthorized;

        // Only source may have a coordinate system. Otherwise a coordinate system should have been assigned to the source.
        // Source and target should have the same base.
        if (!unitConvertionAuthorized)
            {
            status = S_ERROR;
            return Reprojection::GetNull();
            }

        return Reprojection(GetUnitRectificationTransfoModel(sourceGCS.GetHorizontalUnit(),
                                                             sourceGCS.GetVerticalUnit(),
                                                             targetGCS.GetHorizontalUnit(), 
                                                             targetGCS.GetVerticalUnit(),
                                                             m_policy.angularToLinearUnitRatio));
        }
    else
        {
        // Both source and target be null
        if (!(sourceGCS.IsNull() && targetGCS.IsNull()))
            status = S_ERROR;

        // Source or target may still have a local transform
        return Reprojection::GetNull();
        }
    }

namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    TransfoModel GetLocalToBaseFor(const LocalTransform& sourceLTransform, SMStatus& status)
    {
    if (sourceLTransform.HasToGlobal())
        {
        status = SMStatus::S_SUCCESS;
        return sourceLTransform.GetToGlobal();
        }

    const TransfoModel localToBase(InverseOf(sourceLTransform.GetToLocal(), status));
    return localToBase;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetBaseToLocalFor(const LocalTransform& targetLTransform, SMStatus& status)
    {
    if (targetLTransform.HasToLocal())
        {
        status = SMStatus::S_SUCCESS;
        return targetLTransform.GetToLocal();
        }

    const TransfoModel baseToLocal(InverseOf(targetLTransform.GetToGlobal(), status));
    return baseToLocal;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel DecoratesWithSourceLTransform     (const LocalTransform&           sourceLTransform,
                                                const Reprojection&             reprojection,
                                                SMStatus&    status)
    {
    SMStatus localToBaseStatus(SMStatus::S_SUCCESS);
    TransfoModel sourceLocalToTargetLocal(GetLocalToBaseFor(sourceLTransform, localToBaseStatus));

    if (SMStatus::S_SUCCESS != localToBaseStatus)
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    if (reprojection.IsNull())
        return sourceLocalToTargetLocal;

    if (SMStatus::S_SUCCESS != sourceLocalToTargetLocal.Append(AsTransfoModel(reprojection)))
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    return sourceLocalToTargetLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel DecoratesWithTargetLTransform (const Reprojection&             reprojection,
                                            const LocalTransform&           targetLTransform,
                                            SMStatus&    status)
    {
    SMStatus baseToLocalStatus(SMStatus::S_SUCCESS);
    const TransfoModel baseToLocal(GetBaseToLocalFor(targetLTransform, baseToLocalStatus));

    if (SMStatus::S_SUCCESS != baseToLocalStatus)
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    if (reprojection.IsNull())
        return baseToLocal;

    SMStatus combineStatus(SMStatus::S_SUCCESS);
    TransfoModel sourceLocalToTargetLocal(Combine(AsTransfoModel(reprojection), baseToLocal, combineStatus));
    if (SMStatus::S_SUCCESS != combineStatus)
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    return sourceLocalToTargetLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel DecoratesWithSourceAndTargetLTransform    (const LocalTransform&           sourceLTransform,
                                                        const Reprojection&             reprojection,
                                                        const LocalTransform&           targetLTransform,
                                                        SMStatus&    status)
    {
    SMStatus sourceLocalToBaseStatus(SMStatus::S_SUCCESS);
    SMStatus targetBaseToLocalStatus(SMStatus::S_SUCCESS);

    const TransfoModel sourceLocalToBase(GetLocalToBaseFor(sourceLTransform, sourceLocalToBaseStatus));
    const TransfoModel targetBaseToLocal(GetBaseToLocalFor(targetLTransform, targetBaseToLocalStatus));

    if (SMStatus::S_SUCCESS != sourceLocalToBaseStatus ||
        SMStatus::S_SUCCESS != targetBaseToLocalStatus)
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    SMStatus append0Status(SMStatus::S_SUCCESS);
    SMStatus append1Status(SMStatus::S_SUCCESS);

    TransfoModel sourceLocalToTargetLocal(sourceLocalToBase);

    if (!reprojection.IsNull())
        append0Status = sourceLocalToTargetLocal.Append(AsTransfoModel(reprojection));

    append1Status = sourceLocalToTargetLocal.Append(targetBaseToLocal);

    if (SMStatus::S_SUCCESS != append0Status ||
        SMStatus::S_SUCCESS != append1Status)
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    return sourceLocalToTargetLocal;
    }
}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection ReprojectionFactory::Create   (const GCS&      sourceGCS,
                                            const GCS&      targetGCS,
                                            const DRange3d* sourceExtentP,
                                            SMStatus&         status) const
    {
    enum LocalTransformCase
        {
        LT_NONE,
        LT_SOURCE,
        LT_TARGET,
        LT_SOURCE_AND_TARGET,
        };

    const LocalTransformCase localTransformCase = static_cast<LocalTransformCase>(uint32_t(sourceGCS.HasLocalTransform()) | 
                                                                                  (uint32_t(targetGCS.HasLocalTransform()) << 1));

    const Reprojection reprojection(m_implP->Create(sourceGCS, targetGCS, sourceExtentP, status));

    if (SMStatus::S_SUCCESS != status)
        return reprojection;

    switch(localTransformCase)
        {
    case LT_NONE:
        return reprojection;
    case LT_SOURCE:
        return Reprojection(DecoratesWithSourceLTransform(sourceGCS.GetLocalTransform(), reprojection, status));
    case LT_TARGET:
        return Reprojection(DecoratesWithTargetLTransform(reprojection, targetGCS.GetLocalTransform(), status));
    case LT_SOURCE_AND_TARGET:
        return Reprojection(DecoratesWithSourceAndTargetLTransform(sourceGCS.GetLocalTransform(),
                                                                   reprojection,
                                                                   targetGCS.GetLocalTransform(),
                                                                   status));
        }

    assert(!"Unexpected!");
    status = S_ERROR;
    return Reprojection::GetNull();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection ReprojectionFactory::Create   (const GCS&      sourceGCS,
                                            const GCS&      targetGCS,
                                            const DRange3d* sourceExtentP) const
    {
    SMStatus status;
    Reprojection reprojection(Create(sourceGCS, targetGCS, sourceExtentP, status));
    if (S_SUCCESS != status) m_implP->OnUnhandledStatus(status);
    return reprojection;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Reprojection& Reprojection::GetNull ()
    {
    static const Reprojection NULL_REPROJECTION(TransfoModelIdentity::CreateFrom());
    return NULL_REPROJECTION;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection::Reprojection (Impl* implP)
    :   m_implP(implP),
        m_classID(BaseHandler::GetClassID(*implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection::Reprojection (const TransfoModel& transfoModel)
    :   m_implP(Handler::GetBasePtr(transfoModel)),
        m_classID(Handler::GetClassID(transfoModel))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection::~Reprojection  ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection::Reprojection (const Reprojection& rhs)
    :   m_implP(rhs.m_implP),
        m_classID(rhs.m_classID)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection& Reprojection::operator= (const Reprojection& rhs)
    {
    m_implP = rhs.m_implP;
    m_classID = rhs.m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Reprojection::IsNull () const
    {
    return m_classID == TransfoModelIdentity::s_GetClassID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus Reprojection::Reproject(const DPoint3d& sourcePt,
                                                DPoint3d&       targetPt) const
    {
    return GetReprojectionStatusFor(BaseHandler::Transform(*m_implP, sourcePt, targetPt));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus Reprojection::Reproject(const DPoint3d* sourcePtP,
                                                size_t          sourcePtQty,
                                                DPoint3d*       targetPtP) const
    {
    return GetReprojectionStatusFor(BaseHandler::Transform(*m_implP, sourcePtP, sourcePtQty, targetPtP));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel AsTransfoModel (const Reprojection& reprojection)
    {
    return Handler::CreateFrom(reprojection.m_implP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel AsTransfoModel (Reprojection& reprojection)
    {
    return Handler::CreateFrom(reprojection.m_implP);
    }


END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
