/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DEMRasterImporter_dgndb.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
//#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\GeoCoords\Reprojection.h>
#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\Import\ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshPolicy.h>
#include <ScalableMesh\Memory\PacketAccess.h>
#include <ScalableMesh\Type\IScalableMeshPoint.h>

#include "PluginUtils.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_IMAGEPP
//using BENTLEY_NAMESPACE_NAME::GeoCoordinates::DgnGCS;

namespace { //BEGIN UNAMED NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DEMRasterSource : public SourceMixinBase<DEMRasterSource>
    {
public:
    HUTDEMRasterXYZPointsExtractor& GetPointExtractor      () { return *m_extractorP; }

private:
    friend class                    DEMRasterFileSourceCreator;
    //friend class                    DEMRasterElementSourceCreator;

    auto_ptr<HUTDEMRasterXYZPointsExtractor>
                                    m_extractorP;
    ContentDescriptor               m_descriptor;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DEMRasterSource        (HUTDEMRasterXYZPointsExtractor*     extractor,
                                                            const ContentDescriptor&            descriptor)
        :   m_extractorP(extractor),
            m_descriptor(descriptor)
        {

        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_extractorP.reset();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor  () const override
        {
        return m_descriptor;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType               () const override
        {
        return L"DEM Raster";
        }

    };



const double ANGULAR_TO_LINEAR_RATIO = GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree());

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const HFCPtr<HPMPool>&              GetPoolInstance                        ()
    {
    // NTERAY: Maybe this pool can share importer's memory manager?
    static const HFCPtr<HPMPool> POOL_INSTANCE_PTR(new HPMPool(65536, HPMPool::KeepLastBlock));
    return POOL_INSTANCE_PTR;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HUTDEMRasterXYZPointsExtractor*     CreateDEMExtractor                     (const WChar*              path)
    {
    const WString adaptedPath(WString(L"file://") + WString(path));

    try
        {
        return new HUTDEMRasterXYZPointsExtractor(adaptedPath, GetPoolInstance(), false);
        }
    catch (...) // TDORAY: Catch only IPP exceptions
        {
        throw FileIOException();
        }
    }

bool                                ExtractUnitFromGeoTiffKeys             (Unit&                 unit,
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
        if (BeStringUtilities::Wcsicmp(trimedUnitName.c_str(), name.c_str())==0)
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

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GCS                                 GetDEMFileGCS                          (const HUTDEMRasterXYZPointsExtractor&
                                                                                                            extractor)
    {
    GeoCoordinates::BaseGCSCP geoCodingP = extractor.GetDEMRasterCoordSysCP();
    
    if (0 == geoCodingP)
        return GCS::GetNull();
    
    const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr coordSysPtr(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(*geoCodingP));
    Unit gcsUnit((0 == coordSysPtr.get()) ? Unit::GetMeter() : GetUnitFor(*coordSysPtr));

    if (0 == coordSysPtr.get() && !ExtractUnitFromGeoTiffKeys(gcsUnit, geoCodingP))
        return GCS::GetNull();

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
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                IsDEMFileGeoreferenced                 (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor)
    {        
    return 0 != extractor.GetDEMRasterCoordSysCP();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d                            GetDEMFileRange                        (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor,
                                                                            const GCS&                  targetGCS)
    {
    DRange3d range = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

    extractor.Get2DCoordMinMaxValues(&range.low.x, &range.high.x, &range.low.y, &range.high.y);
    if (!extractor.GetZCoordMinMaxValues(&range.low.z, &range.high.z))
        {
        assert(0.0 == range.low.z && 0.0 == range.high.z);
        }

    // TDORAY: Take into account that units have been rectified to meter (which is the base of linear units) or
    // degree (which is the base of angular units). Convert range to target units.

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DEMRasterFileSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.img;*.dim;*.dt0;*.dt1;*.dt2;*.tif;*.tiff;*.itiff;*.itiff64;*.dem;*catd.ddf;*.adf;*.asc";
        }

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        try
            {
            const HFCPtr<HFCURL> urlPtr = new HFCURLFile(WString(L"file://") + pi_rSourceRef.GetPathCStr());
            const HRFRasterFileCreator* foundCreatorP = HRFRasterFileFactory::GetInstance()->FindCreator(urlPtr, HFC_READ_ONLY);
            assert(0 != foundCreatorP);
            return true;
            }
        catch (const HFCException&)
            {
            return false;
            }
        }



    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   sourceRef,
                                                                            Log&                        warningLog) const override
        {
        auto_ptr<HUTDEMRasterXYZPointsExtractor> pointExtractorP(CreateDEMExtractor(sourceRef.GetPathCStr()));

        ContentDescriptor descriptor(CreateDescriptor(*pointExtractorP));

        return new DEMRasterSource(pointExtractorP.release(), descriptor);
        }


    static ContentDescriptor        CreateDescriptor                       (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor)
        {
        const GCS gcs(GetDEMFileGCS(extractor));
        DRange3d range(GetDEMFileRange(extractor, gcs));
        ScalableMeshData data = ScalableMeshData::GetNull();
        data.SetIsGridData(true);
        data.AddExtent(range);

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
class DEMRasterElementSourceCreator : public DGNElementSourceCreatorBase
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

        auto_ptr<HUTDEMRasterXYZPointsExtractor> pointExtractorP(CreateDEMExtractor(sourceRef.GetLocalFileP()->GetPathCStr()));

        ContentDescriptor descriptor(CreateDescriptor(*pointExtractorP, dgnRasterP, sourceRef.GetModelRef()));




        return new DEMRasterSource(pointExtractorP.release(), descriptor);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  NTERAY: Incomplete implementation. Not used yet. See TR #336048
    *                       for more details.
    * @bsimethod                                                  Raymond.Gauthier   03/2012
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
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static TransfoMatrix            GetExtractorAPIToRasterPixelTransform  (const HUTDEMRasterXYZPointsExtractor&
                                                                                                        extractor)
        {
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
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
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
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BaseGCSCPtr              GetRasterBaseGCS                       (DgnRasterP                  dgnRasterP)
        {            
        if (0 == dgnRasterP->GetDgnGCSCP())
            throw CustomException(L"Error retrieving raster mstn gcs");
       
        return BaseGCS::CreateGCS(*((BaseGCS*)dgnRasterP->GetDgnGCSCP()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
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
    * @bsimethod                                                  Raymond.Gauthier   11/2011
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
            LayerDescriptor(L"",
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
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DEMRasterPointExtractor : public InputExtractorBase
    {
    // Dimension groups definition
    enum
        {
        DG_XYZ,
        DG_QTY,
        };

    auto_ptr<HUTDEMRasterXYZPointsIterator>
                                    m_pPtsIterator;

    PODPacketProxy<DPoint3d>        m_pointPacket;
    size_t                          m_maxPtQty;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     pi_rRawEntities) override
        {
        m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        const size_t PtQty = m_pPtsIterator->GetXYZPoints(m_pointPacket.Edit(), m_maxPtQty);
        m_pointPacket.SetSize(PtQty);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_pPtsIterator->NextBlock();
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DEMRasterPointExtractor              (HUTDEMRasterXYZPointsIterator*   pi_pPointIterator)
        :   m_pPtsIterator(pi_pPointIterator),
            m_maxPtQty(pi_pPointIterator->GetMaxXYZPointQty())
        {

        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DEMRasterPointExtractorCreator : public InputExtractorCreatorMixinBase<DEMRasterSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == PointTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (DEMRasterSource&                      sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection) const override
        {
        return RawCapacities (sourceBase.GetPointExtractor().GetMaxPointQtyForXYZPointsIterator()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (DEMRasterSource&                      sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection,
                                                                                    const ExtractionConfig&               config,
                                                                                    Log&                                  log) const override
        {
        const WString DestCoordSysKeyName = L"";
                
        double scaleFactor = 1;
        
        // TDORAY: CreateXYZPointsIterator should take a const WString as input but could not be changed due to 8.11.7 backward compatibility issues
        auto_ptr<HUTDEMRasterXYZPointsIterator>
            pIterator(sourceBase.GetPointExtractor().CreateXYZPointsIteratorWithNoDataValueRemoval(const_cast<WString&>(DestCoordSysKeyName), scaleFactor));
        if (0 == pIterator.get())
            return 0;

        return new DEMRasterPointExtractor(pIterator.release());
        }

    };

} //END UNAMED NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceRegistry::AutoRegister<DEMRasterFileSourceCreator> s_RegisterDEMRasterFileSource;
//const SourceRegistry::AutoRegister<DEMRasterElementSourceCreator> s_RegisterDEMRasterElementSource;
const ExtractorRegistry::AutoRegister<DEMRasterPointExtractorCreator> s_RegisterDEMRasterPointExtractor;

END_BENTLEY_SCALABLEMESH_NAMESPACE
