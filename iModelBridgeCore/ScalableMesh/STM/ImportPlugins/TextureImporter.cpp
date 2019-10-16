/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include <ScalableMesh/GeoCoords/DGNModelGeoref.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh/IScalableMeshPolicy.h>
#include <ScalableMesh/Memory/PacketAccess.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>

#include "..\InternalUtilityFunctions.h"

#include "PluginUtils.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_IMAGEPP


namespace { //BEGIN UNAMED NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
class TextureSource : public SourceMixinBase<TextureSource>
    {
public:
    
private:
    friend class                    TextureFileSourceCreator;
    friend class                    TextureElementSourceCreator;
    
    ContentDescriptor               m_descriptor;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        TextureSource(const ContentDescriptor&            descriptor)
        :   m_descriptor(descriptor)
        {

        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {        
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor  () const override
        {
        return m_descriptor;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType               () const override
        {
        return L"Texture";
        }

    };



const double ANGULAR_TO_LINEAR_RATIO = GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree());

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const HFCPtr<HPMPool>&              GetPoolInstance                        ()
    {
    // NTERAY: Maybe this pool can share importer's memory manager?
    static const HFCPtr<HPMPool> POOL_INSTANCE_PTR(new HPMPool(65536, HPMPool::KeepLastBlock));
    return POOL_INSTANCE_PTR;
    }

#ifdef VANCOUVER_API
bool                                ExtractUnitFromGeoTiffKeys             (Unit&                 unit,
                                                                            const IRasterBaseGcs& geoCoding)
    {    
    const unsigned int unitId = geoCoding.GetEPSGUnitCode();;

    if (0 == unitId)
        {
        unit = Unit::GetMeter();
        return false;
        }
        
    if (geoCoding.GetBaseGCS() == 0)
        {
        unit = Unit::GetMeter();
        return false;
        }

    WString unitName; 

    geoCoding.GetBaseGCS()->GetUnits(unitName);

    WString trimedUnitName(unitName.c_str());

    trimedUnitName.Trim();
       
    const double ratioToMeter = 1 / geoCoding.GetUnitsFromMeters();
    
    unit = Unit::CreateLinearFrom(trimedUnitName.c_str(), ratioToMeter);
    return true;
    }

#else

bool                                ExtractUnitFromGeoTiffKeys(Unit&                 unit,
                                                               const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& geoCoding)
    {
        const unsigned int unitId = geoCoding->GetEPSGUnitCode();

        if (0 == unitId)
        {
            unit = Unit::GetMeter();
            return false;
        }

        if (geoCoding == 0)
        {
            unit = Unit::GetMeter();
            return false;
        }

        WString unitName;

        geoCoding->GetUnits(unitName);

        WString trimedUnitName(unitName.c_str());

        trimedUnitName.Trim();

        double unitFromMeter = 1.0;
        T_WStringVector* pAllUnitName = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::GetUnitNames();
        for (T_WStringVector::const_iterator itr = pAllUnitName->begin(); itr != pAllUnitName->end(); ++itr)
        {
            WString name = *itr;
            name.Trim();
            if (BeStringUtilities::Wcsicmp(trimedUnitName.c_str(), name.c_str()) == 0)
            {
                BENTLEY_NAMESPACE_NAME::GeoCoordinates::UnitCP pUnit(BENTLEY_NAMESPACE_NAME::GeoCoordinates::Unit::FindUnit(itr->c_str()));
                unitFromMeter = 1.0 / pUnit->GetConversionFactor();
                break;
            }
        }

        const double ratioToMeter = 1 / unitFromMeter;

        unit = Unit::CreateLinearFrom(trimedUnitName.c_str(), ratioToMeter);
        return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS                                 GetTextureFileGCS                          (HFCPtr<HRFRasterFile>& pRasterFile)
    {
#ifdef VANCOUVER_API
    const IRasterBaseGcsCP geoCodingP = pRasterFile->GetPageDescriptor(0)->GetGeocodingCP();
        
    if (0 == geoCodingP)
        return GCS::GetNull();
    
    const BaseGCSCPtr coordSysPtr(Bentley::GeoCoordinates::BaseGCS::CreateGCS (*geoCodingP->GetBaseGCS()));
    Unit gcsUnit((0 == coordSysPtr.get()) ? Unit::GetMeter() : GetUnitFor(*coordSysPtr));

    if (0 == coordSysPtr.get() && !ExtractUnitFromGeoTiffKeys(gcsUnit, *geoCodingP))
        return GCS::GetNull();
#else
    GeoCoordinates::BaseGCSCP geoCodingP = pRasterFile->GetPageDescriptor(0)->GetGeocodingCP();

    if (0 == geoCodingP)
        return GCS::GetNull();

    const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr coordSysPtr(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(*geoCodingP));
    Unit gcsUnit((0 == coordSysPtr.get()) ? Unit::GetMeter() : GetUnitFor(*coordSysPtr));

    if (0 == coordSysPtr.get() && !ExtractUnitFromGeoTiffKeys(gcsUnit, geoCodingP))
        return GCS::GetNull();
#endif

    // Take into account that units have been rectified to meter (which is the base of linear units) or
    // degree (which is the base of angular units).
    const TransfoModel meterToRasterUnit(GetUnitRectificationTransfoModel(Unit::GetMeter(), gcsUnit, ANGULAR_TO_LINEAR_RATIO));

    return (0 != coordSysPtr.get()) ?
                GetGCSFactory().Create(coordSysPtr,
                                       LocalTransform::CreateFromToGlobal(meterToRasterUnit)) :
                GetGCSFactory().Create(gcsUnit,
                                       LocalTransform::CreateFromToGlobal(meterToRasterUnit));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//NEEDS_WORK_SM : Copied from Imagepp\all\gra\hut\src\HUTDEMRasterXYZPointsExtractor.cpp - Should create a utility function
#if 0 
void Get2DCoordMinMaxValues(HFCPtr<HRFRasterFile> pRasterFile,
                            double* po_pXMin, double* po_pXMax,
                            double* po_pYMin, double* po_pYMax)
{
    HPRECONDITION((po_pXMin != 0) && (po_pXMax != 0) && (po_pYMin != 0) && (po_pYMax != 0));

    HFCPtr<HRFPageDescriptor> pPageDescriptor(pRasterFile->GetPageDescriptor(0));

    HFCPtr<HGFHMRStdWorldCluster> worldCluster(new HGFHMRStdWorldCluster());    

    HFCPtr<HGF2DCoordSys>     pCoordSys(worldCluster->
        GetCoordSysReference(pRasterFile->
            GetWorldIdentificator()));

    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    if (pPageDescriptor->HasTransfoModel() != 0)
    {
        pTransfoModel = pPageDescriptor->GetTransfoModel();
    }
    else
    {
        pTransfoModel = new HGF2DIdentity();
    }

    pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*(pCoordSys->
        GetTransfoModelTo(m_pXYCoordSys)));

    *po_pXMin = DBL_MAX;
    *po_pXMax = (-DBL_MAX);
    *po_pYMin = DBL_MAX;
    *po_pYMax = (-DBL_MAX);


    HFCPtr<HGF2DCoordSys> pLogical;

    HFCPtr<HRARaster> pRaster;

    pLogical = worldCluster->GetCoordSysReference(pRasterFile->GetPageWorldIdentificator(0));
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(nullptr, pRasterFile, 0, pLogical);

    // Get the raster from the store
    pRaster = pStore->LoadRaster();
    HASSERT(pRaster != NULL);

    UInt64 heightInPixels;
    UInt64 widthInPixels;

    ((HFCPtr<HRAStoredRaster>&)pRaster)->GetRasterSize(&widthInPixels, &heightInPixels);
    
    double LogicCoordinateX;
    double LogicCoordinateY;

    for (UInt64 PhysicalCoordinateX = 0; PhysicalCoordinateX < widthInPixels;)
    {
        for (UInt64 PhysicalCoordinateY = 0; PhysicalCoordinateY < heightInPixels;)
        {
            pTransfoModel->ConvertDirect((double)PhysicalCoordinateX,
                (double)PhysicalCoordinateY,
                &LogicCoordinateX,
                &LogicCoordinateY);

            *po_pXMin = MIN(*po_pXMin, LogicCoordinateX);
            *po_pXMax = MAX(*po_pXMax, LogicCoordinateX);
            *po_pYMin = MIN(*po_pYMin, LogicCoordinateY);
            *po_pYMax = MAX(*po_pYMax, LogicCoordinateY);

            PhysicalCoordinateY += heightInPixels - 1;
        }

        PhysicalCoordinateX += widthInPixels - 1;
    }
}
#endif

DRange3d                            GetTextureFileRange                        (HFCPtr<HRFRasterFile> rasterFile,
                                                                                const GCS&            targetGCS)
    {
    DRange3d range = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
    
    //Get2DCoordMinMaxValues(pRasterFile, &range.low.x, &range.high.x, &range.low.y, &range.high.y);
                        
    return range;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConstructUrl(HFCPtr<HFCURL>& urlPtr, const LocalFileSourceRef&   pi_rSourceRef)
    {

#ifdef VANCOUVER_API
    if (!IsUrl(pi_rSourceRef.GetPathCStr()))
        urlPtr = HFCURL::Instanciate(WString(L"file://") + pi_rSourceRef.GetPathCStr());
    else
        urlPtr = HFCURL::Instanciate(pi_rSourceRef.GetPathCStr());
#else
    if (!IsUrl(pi_rSourceRef.GetPathCStr()))
        urlPtr = HFCURL::Instanciate(Utf8String(WString(L"file://") + pi_rSourceRef.GetPathCStr()));
    else
        urlPtr = HFCURL::Instanciate(Utf8String(pi_rSourceRef.GetPathCStr()));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class TextureFileSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.img;*.tif;*.tiff;*.itiff;*.itiff64";
        }

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        HFCPtr<HFCURL> urlPtr;

        try
            {            
            ConstructUrl(urlPtr, pi_rSourceRef);


            const HRFRasterFileCreator* foundCreatorP = HRFRasterFileFactory::GetInstance()->FindCreator(urlPtr, HFC_READ_ONLY);

            if (foundCreatorP == nullptr)
                return false;
            }
        catch (const HFCException&)
            {
            return false;
            }
             
        try
            {           
            HUTDEMRasterXYZPointsExtractor extractor(urlPtr->GetURL(), GetPoolInstance(), false);

            return false;
            }
        catch (const HFCException&)
            { //Not a DEM raster, thus a valid raster for texturing.
            return true;
            }
        }



    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   sourceRef,
                                                                            Log&                        warningLog) const override
        {
        HFCPtr<HFCURL> urlPtr;

        ConstructUrl(urlPtr, sourceRef);

        const HFCPtr<HRFRasterFile> rasterFile(HRFRasterFileFactory::GetInstance()->OpenFile(urlPtr, HFC_READ_ONLY | HFC_SHARE_READ_WRITE));
                        
        ContentDescriptor descriptor(CreateDescriptor(rasterFile));

        return new TextureSource(descriptor);
        }


    static ContentDescriptor        CreateDescriptor                       (HFCPtr<HRFRasterFile> pRasterFile)
        {        
        const GCS gcs(GetTextureFileGCS(pRasterFile));
        DRange3d range(GetTextureFileRange(pRasterFile, gcs));
        ScalableMeshData data = ScalableMeshData::GetNull();
        data.SetIsGridData(true);
        //data.AddExtent(range);

        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            PointType3d64fCreator().Create(),
                            gcs,
                            &range,
                            data)
            );
        }

    };

#if 0

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class TextureElementSourceCreator : public DGNElementSourceCreatorBase
    {
    virtual uint32_t                    _GetElementType                    () const override
        {
        return RASTER_FRAME_ELM;
        }

    virtual uint32_t                    _GetElementHandlerID               () const override
        {
        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   04/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const DGNElementSourceRef&  sourceRef) const override
        {
        DgnRasterP dgnRasterP = 0;
        const bool foundRasterHandle = (BSISUCCESS == mdlRaster_handleFromElementRefGet(&dgnRasterP, sourceRef.GetElementRef(), sourceRef.GetModelRef()));

        return 0 != sourceRef.GetLocalFileP() && foundRasterHandle && mdlRaster_HasDEMFilters(dgnRasterP);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   04/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const DGNElementSourceRef&  sourceRef,
                                                                            Log&                        log) const override
        {
        DgnRasterP dgnRasterP = 0;
        if (BSISUCCESS != mdlRaster_handleFromElementRefGet(&dgnRasterP, sourceRef.GetElementRef(), sourceRef.GetModelRef()))
            throw CustomException(L"Could not find raster handle");

        //auto_ptr<HUTDEMRasterXYZPointsExtractor> pointExtractorP(CreateDEMExtractor(sourceRef.GetLocalFileP()->GetPathCStr()));

        ContentDescriptor descriptor(CreateDescriptor(dgnRasterP, sourceRef.GetModelRef()));

        return new TextureSource(descriptor);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  NTERAY: Incomplete implementation. Not used yet. See TR #336048
    *                       for more details.
    * @bsimethod                                                  Mathieu.St-Pierre   03/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static HFCPtr<HVEClipShape>     GetClipShapeFor                        (DgnRasterP                  dgnRasterP)
        {
        long clipMaskCount = 0;
        if (BSISUCCESS != mdlRaster_clipMaskCount(&clipMaskCount, dgnRasterP))
            return 0; // TDORAY: Throw instead?

        ElemDescrSmartP clipBoundaryP;
        if (BSISUCCESS != mdlRaster_clipBoundaryGet(clipBoundaryP.GetDescrPtr(), PLAN_RASTER, dgnRasterP))
            return 0; // TDORAY: Throw instead?

        const bool hasClipMasks = (0 != clipMaskCount);
        const bool hasClipBoundary = (clipBoundaryP != NULL);

        if (!hasClipMasks && !hasClipBoundary)
            return 0;

        const HFCPtr<HGF2DCoordSys> coordSysPtr(new HGF2DCoordSys);

        HFCPtr<HVEClipShape> clipShapePtr(new HVEClipShape(coordSysPtr));

        HFCPtr<HVEShape> clipMaskShapePtr(new HVEShape(coordSysPtr));
        HFCPtr<HVEShape> boundaryShapePtr(new HVEShape(coordSysPtr));

        // TDORAY: Stroke boundary element and add it to boundaryShapePtr

        long maskIdx = 0;
        for (; maskIdx < clipMaskCount; ++maskIdx)
            {
            ElemDescrSmartP clipMaskP;
            if (BSISUCCESS != mdlRaster_clipMaskGet(clipMaskP.GetDescrPtr(), PLAN_RASTER, maskIdx, dgnRasterP))
                break;

            /* TDORAY: Stroke element and add it to clipMaskShapePtr
            DlmArrayAutoPtr strokedClipP;
            if (BSISUCCESS != mdlElmdscr_stroke(strokedClipP.getRefTo(), )


            double* dimP = ptsP.get();
            for (ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End() ; ++myPoint)
                {
                *dimP++ = (*myPoint).x;
                *dimP++ = (*myPoint).y;
                }
            HVE2DPolygonOfSegments polygon(dimP - ptsP.get(), ptsP, coordSysP);

            clipMaskShapePtr->Unify(polygon);

            clipShapePtr->AddClip(shapePtr, true);
            */
            }

        return maskIdx != clipMaskCount ? 0 : clipShapePtr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static TransfoMatrix            GetExtractorAPIToRasterPixelTransform  (/*const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor*/)
        {
#if 0 
        HFCPtr<HGF2DTransfoModel>
            apiToRasterPixelsTransfoModelPtr(extractor.GetXYCoordSysPtr()->GetTransfoModelTo(extractor.GetPhysicalLowerLeftPixelCoordSysPtr()));

        if (0 == apiToRasterPixelsTransfoModelPtr ||
            !apiToRasterPixelsTransfoModelPtr->CanBeRepresentedByAMatrix())
            throw CustomException(L"Impossible to access matrix/transform");

        HFCMatrix<3, 3> apiToRasterPixelsMatrix2d(apiToRasterPixelsTransfoModelPtr->GetMatrix());

        return TransfoMatrix
           (apiToRasterPixelsMatrix2d[0][0],    apiToRasterPixelsMatrix2d[0][1],    0.0,    apiToRasterPixelsMatrix2d[0][2],
            apiToRasterPixelsMatrix2d[1][0],    apiToRasterPixelsMatrix2d[1][1],    0.0,    apiToRasterPixelsMatrix2d[1][2],
            0.0,                                0.0,                                1.0,    0.0);
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static TransfoModel             GetExtractorAPIToModelGlobalTransform  (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor,
                                                                            DgnRasterP                  dgnRasterP,
                                                                            DgnModelRefP                dgnRasterModelRefP,
                                                                            const Unit&                 globalFrameUnit)
        {
        // This transform already includes global origin translation
        Transform rasterPixelsToUorTransform;
        if (BSISUCCESS != mdlRaster_getTransform(&rasterPixelsToUorTransform, dgnRasterP))
            throw CustomException(L"Could not read raster transform to uor!");

        TransfoMatrix extractorAPIToRasterPixel(GetExtractorAPIToRasterPixelTransform(extractor));
        TransfoMatrix modelDesignToGlobal (GetModelDesignToGlobalTransfoMatrix(dgnRasterModelRefP, globalFrameUnit));

        TransfoMatrix extractorAPIToModelGlobal(modelDesignToGlobal *
                                                FromBSITransform(rasterPixelsToUorTransform) *
                                                extractorAPIToRasterPixel);

        return TransfoModel::CreateFrom(extractorAPIToModelGlobal);
        }





    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BaseGCSCPtr              GetRasterBaseGCS                       (DgnRasterP                  dgnRasterP)
        {            
        if (0 == dgnRasterP->GetDgnGCSCP())
            throw CustomException(L"Error retrieving raster mstn gcs");
       
        return BaseGCS::CreateGCS(*((BaseGCS*)dgnRasterP->GetDgnGCSCP()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static GCS                      GetDgnRasterGCSAsSeenFromModel         (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor,
                                                                            DgnRasterP                  dgnRasterP,
                                                                            DgnModelRefP                dgnRasterModelRefP)
        {
        bool inheritModelGCS = false;
        if (BSISUCCESS != mdlRaster_GCSInheritedFromModelStateGet(&inheritModelGCS, dgnRasterP))
            { assert("Could not read GCS inheritance flag!"); }

        BaseGCSCPtr modelCoordSysPtr(DgnGCS::FromModel(dgnRasterModelRefP, true));
        BaseGCSCPtr rasterCoordSysPtr( inheritModelGCS ? modelCoordSysPtr : GetRasterBaseGCS(dgnRasterP));

        if (0 != modelCoordSysPtr.get() && inheritModelGCS)
            {
            Unit modelGCSUnit(GetUnitFor(*modelCoordSysPtr));
            TransfoModel extractorAPIToTargetUnits(GetExtractorAPIToModelGlobalTransform(extractor, dgnRasterP,
                                                                                         dgnRasterModelRefP, modelGCSUnit));
            return GetGCSFactory().Create(modelCoordSysPtr, LocalTransform::CreateFromToGlobal(extractorAPIToTargetUnits));
            }
        else if (0 != modelCoordSysPtr.get() && 0 != rasterCoordSysPtr.get())
            {
            const Unit rasterGCSUnit(GetUnitFor(*rasterCoordSysPtr));

            TransfoModel extractorAPIToTargetUnits(GetExtractorAPIToModelGlobalTransform(extractor, dgnRasterP,
                                                                                         dgnRasterModelRefP, rasterGCSUnit));

            GCS rasterGCS(GetGCSFactory().Create(rasterCoordSysPtr,
                                                 LocalTransform::CreateFromToGlobal(extractorAPIToTargetUnits)));

            GCS modelGCS(GetGCSFactory().Create(modelCoordSysPtr));


            Reprojection rasterToModelReprojection(GetReprojectionFactory().Create(rasterGCS, modelGCS, 0));

            TransfoModel rasterCSToModelCS(AsTransfoModel(rasterToModelReprojection));

            return GetGCSFactory().Create(modelGCS, LocalTransform::CreateFromToGlobal(rasterCSToModelCS));
            }
        else
            {
            Unit modelMasterUnit(GetModelMasterUnit(dgnRasterModelRefP));

            TransfoModel extractorAPIToTargetUnits(GetExtractorAPIToModelGlobalTransform(extractor, dgnRasterP,
                                                                                         dgnRasterModelRefP, modelMasterUnit));
            return GetGCSFactory().Create(modelMasterUnit, LocalTransform::CreateFromToGlobal(extractorAPIToTargetUnits));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ContentDescriptor        CreateDescriptor                       (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor,
                                                                            DgnRasterP                  dgnRasterP,
                                                                            DgnModelRefP                dgnRasterModelRefP)
        {
        const GCS dgnRasterGCSAsSeenFromModel(GetDgnRasterGCSAsSeenFromModel(extractor, dgnRasterP, dgnRasterModelRefP));
        const GCS gcs(ReinterpretModelGCSFromRootPerspective(dgnRasterGCSAsSeenFromModel, dgnRasterModelRefP));
        ScalableMeshData data = ScalableMeshData::GetNull();

        // TDORAY: Compute transformed range as seen from model

        return  ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            PointType3d64fCreator().Create(),
                            gcs,
                            0,
                            data)
            );        
        }
    };
#endif


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
class TexturePointExtractor : public InputExtractorBase
    {
    // Dimension groups definition
    enum
        {
        DG_XYZ,
        DG_QTY,
        };
/*
    auto_ptr<HUTDEMRasterXYZPointsIterator>
                                    m_pPtsIterator;
*/

    PODPacketProxy<DPoint3d>        m_pointPacket;
    size_t                          m_maxPtQty;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     pi_rRawEntities) override
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
        //m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
/*
        const size_t PtQty = m_pPtsIterator->GetXYZPoints(m_pointPacket.Edit(), m_maxPtQty);
        m_pointPacket.SetSize(PtQty);*/
        }

    virtual size_t              _GetPhysicalSize() override
        {
        return 0;
        }

    virtual size_t              _GetReadPosition() override
        {
        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
        return false;
        //return m_pPtsIterator->NextBlock();
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        TexturePointExtractor              ()        
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
class TexturePointExtractorCreator : public InputExtractorCreatorMixinBase<TextureSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == PointTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (TextureSource&                      sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection) const override
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
        return RawCapacities(0);
        //return RawCapacities (sourceBase.GetPointExtractor().GetMaxPointQtyForXYZPointsIterator()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Mathieu.St-Pierre   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (TextureSource&                      sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection,
                                                                                    const ExtractionConfig&               config,
                                                                                    Log&                                  log) const override
        {
        assert(!"Texturing use different pipeline than terrain data. Not expected to be called");
        return nullptr;

#if 0
        const WString DestCoordSysKeyName = L"";
                        
        double scaleFactor = 1;
        
        // TDORAY: CreateXYZPointsIterator should take a const WString as input but could not be changed due to 8.11.7 backward compatibility issues
        auto_ptr<HUTDEMRasterXYZPointsIterator>
            pIterator(sourceBase.GetPointExtractor().CreateXYZPointsIteratorWithNoDataValueRemoval(const_cast<WString&>(DestCoordSysKeyName), scaleFactor));
        if (0 == pIterator.get())
            return 0;

        return new TexturePointExtractor(pIterator.release());
#endif
        }

    };

} //END UNAMED NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceRegistry::AutoRegister<TextureFileSourceCreator> s_RegisterTextureFileSource;
//const SourceRegistry::AutoRegister<TextureElementSourceCreator> s_RegisterTextureElementSource;
const ExtractorRegistry::AutoRegister<TexturePointExtractorCreator> s_RegisterTexturePointExtractor;

END_BENTLEY_SCALABLEMESH_NAMESPACE
