/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/DGNModelGeoref.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh\Foundations\Exception.h>
#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\GeoCoords\GCS.h>
#include <ScalableMesh\IScalableMeshPolicy.h>
#include <ScalableMesh\GeoCoords\Reprojection.h>

               
USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES

USING_NAMESPACE_BENTLEY_SCALABLEMESH

using BENTLEY_NAMESPACE_NAME::GeoCoordinates::DgnGCS;
using BENTLEY_NAMESPACE_NAME::GeoCoordinates::DgnGCSPtr;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
    
BEGIN_UNNAMED_NAMESPACE

const double ANGULAR_TO_LINEAR_RATIO = GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree());


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToMasterTransfoMatrix (DgnModelRefP modelRef)
    {
    // Get [Model uor -> Model master] scale
    const double masterUnitPerUor = 1.0 / ModelInfo::GetUorPerMaster (modelRef->GetModelInfoCP());

    // Get Uor global origin
    DPoint3d modelMasterGlobalOrigin;
    
    if (BSISUCCESS != ModelInfo::GetGlobalOrigin(modelRef->GetModelInfoCP(), &modelMasterGlobalOrigin))
        throw CustomException(L"Could not access global origin!");

    // Transform global origin so that it is expressed in master units
    modelMasterGlobalOrigin.x *= -masterUnitPerUor;
    modelMasterGlobalOrigin.y *= -masterUnitPerUor;
    modelMasterGlobalOrigin.z *= -masterUnitPerUor;

    // Get matrix representation that includes both transformations
    return TransfoMatrix   (masterUnitPerUor,   0.0,                0.0,                modelMasterGlobalOrigin.x,
                            0.0,                masterUnitPerUor,   0.0,                modelMasterGlobalOrigin.y,
                            0.0,                0.0,                masterUnitPerUor,   modelMasterGlobalOrigin.z);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelMasterToDesignTransfoMatrix (DgnModelRefP modelRef)
    {
    // Get [Model master to Model uor] scale

    const double modelUorPerMasterUnit = ModelInfo::GetUorPerMaster (modelRef->GetModelInfoCP());

    // Get Uor global origin
    DPoint3d modelUORGlobalOrigin;
    if (BSISUCCESS != ModelInfo::GetGlobalOrigin(modelRef->GetModelInfoCP(), &modelUORGlobalOrigin))
        throw CustomException(L"Could not access global origin!");

    return TransfoMatrix   (modelUorPerMasterUnit,  0.0,                    0.0,                    modelUORGlobalOrigin.x,
                            0.0,                    modelUorPerMasterUnit,  0.0,                    modelUORGlobalOrigin.y,
                            0.0,                    0.0,                    modelUorPerMasterUnit,  modelUORGlobalOrigin.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToParentDesignTransfoMatrix    (DgnModelRefP modelRef,
                                                            DgnModelRefP parentModelRef)
    {
    // TDORAY: Assert that parent model ref is really parent to model ref

    // Get [Model uor to Parent Model uor] scale
    Transform modelUorToParentModelUorTransform;
    mdlRefFile_getTransformToParent(&modelUorToParentModelUorTransform,
                                    modelRef->AsDgnAttachmentCP(),
                                    (NULL == parentModelRef ? NULL : parentModelRef->AsDgnAttachmentCP()));

    return FromBSITransform(modelUorToParentModelUorTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToRootDesignTransfoMatrix   (DgnModelRefP modelRef)
    {
    // Get [Model uor to root Model uor] scale
    Transform modelUorToRootModelUorTransform;
    mdlRefFile_getTransformToParent(&modelUorToRootModelUorTransform,
                                    modelRef->AsDgnAttachmentCP(),
                                    NULL);

    return FromBSITransform(modelUorToRootModelUorTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToParentMasterTransfoMatrix    (DgnModelRefP modelRef,
                                                            DgnModelRefP parentModelRef)
    {

    // Get [Parent Model uor -> Parent Model master] scale
    TransfoMatrix rootUorToRootMasterTransform(GetModelDesignToMasterTransfoMatrix(parentModelRef));

    // Get [Model uor -> Parent Model uor] transform
    TransfoMatrix refUorToRootMasterTransform(GetModelDesignToParentDesignTransfoMatrix(modelRef, parentModelRef));

    // Produce [Model uor -> Parent Model master] transform by appending [Parent Model uor -> Parent Model master] scaling
    // to [Model uor -> Parent Model uor] transform
    return rootUorToRootMasterTransform * refUorToRootMasterTransform;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToRootMasterTransfoMatrix (DgnModelRefP modelRef)
    {
    // Get [ROOT uor -> ROOT master] scale
    TransfoMatrix rootUorToRootMasterTransform(GetModelDesignToMasterTransfoMatrix(modelRef->GetRoot()));

    // Get [Model uor -> ROOT uor] transform
    TransfoMatrix refUorToRootMasterTransform(GetModelDesignToRootDesignTransfoMatrix(modelRef));


    // Produce [Model uor -> ROOT master] transform by appending [ROOT uor -> ROOT master] scaling
    // to [Model uor -> ROOT uor] transform
    return rootUorToRootMasterTransform * refUorToRootMasterTransform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelMasterToRootMasterTransfoMatrix (DgnModelRefP modelRef)
    {
    // Get [Model master to Model uor] scale
    TransfoMatrix modelMasterToModelUOR(GetModelMasterToDesignTransfoMatrix(modelRef));

    TransfoMatrix modelMasterToRootMaster(GetModelDesignToRootMasterTransfoMatrix(modelRef));

    // Produce [Model master -> ROOT master] transform by prepending [Model Master to Model uor] scaling
    // to [Model uor -> ROOT master] transform
    return modelMasterToRootMaster * modelMasterToModelUOR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetModelDesignToRootMasterTransfoModel (DgnModelRefP modelRef)
    {
    return TransfoModel::CreateAffineFrom(GetModelDesignToRootMasterTransfoMatrix(modelRef));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetModelDesignToParentMasterTransfoModel  (DgnModelRefP modelRef,
                                                        DgnModelRefP parentModelRef)
    {
    return TransfoModel::CreateAffineFrom(GetModelDesignToParentMasterTransfoMatrix(modelRef, parentModelRef));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetModelMasterToRootMasterTransfoModel (DgnModelRefP modelRef)
    {
    return TransfoModel::CreateAffineFrom(GetModelMasterToRootMasterTransfoMatrix(modelRef));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetModelDesignToMasterTransfoModel (DgnModelRefP modelRef)
    {
    return TransfoModel::CreateAffineFrom(GetModelDesignToMasterTransfoMatrix(modelRef));
    }

END_UNNAMED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsModelAttachedReprojected (DgnModelRefP  modelRef)
    {        
    DgnAttachmentP refP(modelRef->AsDgnAttachmentP());

    assert(refP != NULL);

    uint32_t attachMethod = refP->GetAttachMethod();

    return ATTACHMETHOD_GeographicProjected == attachMethod ||
           ATTACHMETHOD_GeographicTransformed == attachMethod;
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsModelGeoreferenced (DgnModelRefP  modelRef)
    {
    // TDORAY: Optimize
    const DgnGCSPtr modelGCSPtr(DgnGCS::FromModel(modelRef, true));
    return 0 != modelGCSPtr.get();
    }






/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit GetModelUOR (DgnModelRefP modelRef)
    {
    UnitInfo masterUnit;
    if(BSISUCCESS != ModelInfo::GetMasterUnit(modelRef->GetModelInfoCP(), &masterUnit))
        {
        assert(!"Unexpected");
        return Unit::GetMeter();
        }

    const double masterPerUor = 1.0/ModelInfo::GetUorPerMaster (modelRef->GetModelInfoCP());
    const double basePerMaster = (masterUnit.denominator / masterUnit.numerator);
    const double basePerUor = masterPerUor * basePerMaster;    

    DgnPlatform::UnitBase unitBase = static_cast<UnitBase>(masterUnit.flags.base);

    if(UnitBase::Degree == unitBase)
        return Unit::CreateFromDegreeBased(L"uor", basePerUor);

    assert(UnitBase::Meter == unitBase);
    return Unit::CreateLinearFrom(L"uor", basePerUor);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit GetModelMasterUnit (DgnModelRefP modelRef)
    {
    UnitInfo masterUnit;
    if(BSISUCCESS != ModelInfo::GetMasterUnit(modelRef->GetModelInfoCP(), &masterUnit))
        {
        assert(!"Unexpected");
        return Unit::GetMeter();
        }

    const DgnPlatform::UnitBase unitBase = static_cast<UnitBase>(masterUnit.flags.base); 

    if(UnitBase::Degree == unitBase)
        return Unit::CreateFromDegreeBased(masterUnit.label, (masterUnit.denominator / masterUnit.numerator));


    assert(UnitBase::Meter == unitBase);
    return Unit::CreateLinearFrom(masterUnit.label, (masterUnit.denominator / masterUnit.numerator));
    }

/*---------------------------------------------------------------------------------**//**
* @description  Differ from GetModelMasterUnit in that it will return the model's GeoCS
*               units when a GeoCS is found for this model. Otherwise, will return
*               the model master unit which we considered as global.
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Unit GetModelGlobalUnit (DgnModelRefP modelRef)
    {
    // TDORAY:  Optimize for the not geo-referenced case if we find a accessor returning
    //          directly this information without creating a DgnGCS.
    //          See IsModelGeoreferenced.
    const DgnGCSPtr modelGCSPtr(DgnGCS::FromModel(modelRef, true));
    if (0 == &*modelGCSPtr)
        return GetModelMasterUnit(modelRef);

    return GetUnitFor(*modelGCSPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GetModelMasterGCS (DgnModelRefP modelRef)
    {
    const DgnGCSPtr modelGCSPtr(DgnGCS::FromModel(modelRef, true));
    if(0 != modelGCSPtr.get())
        {
        return GetGCSFactory().Create(modelGCSPtr.get());
        }

    return GetGCSFactory().Create(GetModelMasterUnit(modelRef));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GetModelActiveGCS (DgnModelRefP modelRef)
    {
    const DgnGCSPtr modelGCSPtr(DgnGCS::FromModel(modelRef, true));
    if(0 != modelGCSPtr.get())
        {
        return GetGCSFactory().Create(modelGCSPtr.get());
        }

    return GetGCSFactory().Create(Unit::GetMeter());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel GetModelLocalToGlobalTransfoModel (DgnModelRefP    modelRefP,
                                                const Unit&     localFrameUnit,
                                                const Unit&     globalFrameUnit)
    {
    const Unit modelUor(GetModelUOR(modelRefP));

    DPoint3d translation;
    if (BSISUCCESS != ModelInfo::GetGlobalOrigin(modelRefP->GetModelInfoCP(), &translation))
        throw CustomException(L"Could not access global origin!");

    const double uorToGlobalScale = GetUnitRectificationScaleFactor(modelUor, globalFrameUnit, ANGULAR_TO_LINEAR_RATIO);
    const double lobalToGlobalScale = GetUnitRectificationScaleFactor(localFrameUnit, globalFrameUnit, ANGULAR_TO_LINEAR_RATIO);

    translation.x *= -uorToGlobalScale;
    translation.y *= -uorToGlobalScale;
    translation.z *= -uorToGlobalScale;

    return TransfoModel::CreateScalingTranslatingFrom(lobalToGlobalScale, translation.x, translation.y, translation.z);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix GetModelDesignToGlobalTransfoMatrix  (DgnModelRefP    modelRefP,
                                                    const Unit&     globalFrameUnit)
    {
    const Unit modelUor(GetModelUOR(modelRefP));

    DPoint3d translation;
    if (BSISUCCESS != ModelInfo::GetGlobalOrigin(modelRefP->GetModelInfoCP(), &translation))
        throw CustomException(L"Could not access global origin!");

    const double uorToGlobalScale = GetUnitRectificationScaleFactor(modelUor, globalFrameUnit, ANGULAR_TO_LINEAR_RATIO);

    translation.x *= -uorToGlobalScale;
    translation.y *= -uorToGlobalScale;
    translation.z *= -uorToGlobalScale;

    return TransfoMatrix   (uorToGlobalScale,   0.0,                0.0,                translation.x,
                            0.0,                uorToGlobalScale,   0.0,                translation.y,
                            0.0,                0.0,                uorToGlobalScale,   translation.z);  ;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS GetBSIElementGCSFromRootPerspective (DgnModelRefP elementModelRef)
    {
    GCS gcs(GetModelMasterGCS(elementModelRef->GetRoot()));
    assert(!gcs.HasLocalTransform());

    TransfoModel uorToGCSUnits(Combine(GetModelDesignToRootMasterTransfoModel(elementModelRef),
                                       GetUnitRectificationTransfoModel(GetModelMasterUnit(elementModelRef->GetRoot()),
                                                                        gcs.GetUnit(),
                                                                        ANGULAR_TO_LINEAR_RATIO)));

    gcs.SetLocalTransform(LocalTransform::CreateFromToGlobal(uorToGCSUnits));
    return gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS ReinterpretModelGCSFromRootPerspective (const GCS&      modelGlobalGCS,
                                            DgnModelRefP    modelRefP)
    {
    if (!modelRefP->IsDgnAttachment())
        return modelGlobalGCS;

    if (IsModelAttachedReprojected(modelRefP))
        {
        const GCS rootModelGCS(GetModelMasterGCS(modelRefP->GetRoot()));

        Reprojection elementToRootReprojection(GetReprojectionFactory().Create(modelGlobalGCS, rootModelGCS, 0));

        return GetGCSFactory().Create(rootModelGCS,
                                      LocalTransform::CreateFromToGlobal(AsTransfoModel(elementToRootReprojection)));
        }


    TransfoModel elementToRoot(modelGlobalGCS.GetLocalTransform().GetToGlobal());

    elementToRoot.Append(GetUnitRectificationTransfoModel(modelGlobalGCS.GetUnit(),
                                                          GetModelMasterUnit(modelRefP),
                                                          ANGULAR_TO_LINEAR_RATIO));

    elementToRoot.Append(GetModelMasterToRootMasterTransfoModel(modelRefP));


    const GCS rootModelGCS(GetModelMasterGCS(modelRefP->GetRoot()));
    elementToRoot.Append(GetUnitRectificationTransfoModel(GetModelMasterUnit(modelRefP->GetRoot()),
                                                          rootModelGCS.GetUnit(),
                                                          ANGULAR_TO_LINEAR_RATIO));

    return GetGCSFactory().Create (rootModelGCS,
                                   LocalTransform::CreateFromToGlobal(elementToRoot));
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
