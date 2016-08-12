/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/PODImporter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include <PointoolsVortexAPI_DLL/vortexLicense.c>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.h>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_ResultCodes.h>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.cpp>
#include <ScalableMesh\ScalableMeshLib.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh\Import\Plugin\TypeConversionFilterV0.h>
//#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\Type\IScalableMeshPoint.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
//#include <ScalableMesh\AutomaticGroundDetection\GroundDetectionManager.h>

#include <BePointCloud/BePointCloudApi.h>
#include <BePointCloud/PointCloudTypes.h>
#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/PointCloudColorDef.h>
#include <BePointCloud/PointCloudHandle.h>
#include <BePointCloud/PointCloudScene.h>
#include <BePointCloud/PointCloudChannelHandler.h>
#include <BePointCloud/PointCloudDataQuery.h>
#include <BePointCloud/PointCloudQueryBuffer.h>
#include <BePointCloud/PointCloudVortex.h>

#include "PluginUtils.h"

#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\IScalableMeshPolicy.h>


#define PointCloudMinorId_Handler 1

#define GROUND_CHANNEL_NUMBER (2)

using namespace std;

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD


namespace { //BEGIN UNAMED NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PODSource : public SourceMixinBase<PODSource>
    {
private:
    friend class                    PODFileSourceCreator;
    friend class                    PODElementSourceCreator;



    bool                            m_isSingleElementOwner;

    PointCloudScenePtr              m_pointCloudScenePtr;
    bool                            m_hasInternalClassification;

    bool                            m_WantClassification;
    

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        PODSource          (bool                                    isSingleElementOwner,
                                                        const PointCloudScenePtr&          pointCloudScenePtr
                                                        )
        :   m_isSingleElementOwner(isSingleElementOwner),
            m_pointCloudScenePtr(pointCloudScenePtr),
            m_hasInternalClassification(m_pointCloudScenePtr->HasClassificationChannel())
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    static PODSource*               CreateFrom                                 (WString fileName)
        {
        PointCloudScenePtr pointCloudScenePtr = PointCloudScene::Create(fileName.c_str());

        return new PODSource(true,
                             pointCloudScenePtr);
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                                     () override
        {
        //m_elementHandleP.reset(); // Dispose of element
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    GCS                             GetFileGCS                                 () const
        {
        WString WKT;
        bool    gcsError = false;

        GCS gcs(GCS::GetNull());

        if (SUCCESS != m_pointCloudScenePtr->GetMetaTag(L"Survey.GeoReference", WKT) && 0 != WKT.size())
            {
            gcsError = true;
            }
        else
            {
            if (WKT.empty())
                {
                return GetGCSFactory().Create(Unit::GetMeter());
                }

            try
                {
                gcs = GetGCSFactory().Create(WKT.c_str());
                }
            catch (CustomError&)
                {
                gcsError = true;
                }

            if (gcsError == false)
                {
                static const double ANGULAR_TO_LINEAR_RATIO = GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree());

                /*
                 * TR #346465:  as point cloud's query interface invariably returns points in meter (when transform to UOR is disabled)
                 *              rather than in a unit matching what is described in Survey.GeoReference WKT, we need to fix this
                 *              ourselves.
                 */
                const TransfoModel localToGlobal(GetUnitRectificationTransfoModel(Unit::GetMeter(), gcs.GetUnit(), ANGULAR_TO_LINEAR_RATIO));
                if (SMStatus::S_SUCCESS != gcs.AppendLocalTransform(LocalTransform::CreateFromToGlobal(localToGlobal)))
                    {
                    gcsError = true;
                    }
                }
            }

        if (gcsError == true)
            {
            //assert(!"GCS error");
            gcs = GetGCSFactory().Create(Unit::GetMeter());
            }

        return gcs;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    /*GCS                             GetElementGCS                              () const
        {
        return GetBSIElementGCSFromRootPerspective(m_elementHandleP->GetModelRef());
        }*/

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool                            GetFileRange                               (DRange3d&           range) const
        {
        WString name;
        uint32_t num_clouds;
        uint64_t num_points;

        if (SUCCESS != m_pointCloudScenePtr->GetMetaData (name, num_clouds, num_points, &range.low, &range.high))
            throw CustomException(L"Error fetching metadata");
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   11/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool                            GetElementRange                            (DRange3d&           range) const
        {
        // TDORAY: Need to apply transform to range as it is in meter.

        return false;
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor        _CreateDescriptor  () const override
        {
        const DataTypeSet storedType(HasClassification() ? PointType3d64f_R16G16B16_I16_C8Creator().Create() :
                                                           PointType3d64f_R16G16B16_I16Creator().Create());

        const GCS gcs(GetFileGCS());

        DRange3d range = { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0} };
        bool hasRange(IsFromFile() ? GetFileRange(range) : GetElementRange(range));
        ScalableMeshData data = ScalableMeshData::GetNull();
        data.AddExtent(range);
        data.SetIsGroundDetection(true);

        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                                                    storedType,
                                                    gcs,
                                                    (hasRange) ? &range : 0,
                                                    data),
            true
            );

        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType               () const override
        {
        return L"POD Point Cloud";
        }

public:

    const PointCloudScenePtr     GetFileQuery                           () const { return m_pointCloudScenePtr; }

    void                            SetFileQuery                           (PointCloudScenePtr pointCloudScenePtr) { m_pointCloudScenePtr = pointCloudScenePtr;}

    bool                            HasInternalClassification              () const { return m_hasInternalClassification; }    

    bool                            HasClassification                      () const { return HasInternalClassification(); }

    void                            SetClassification                      (bool hasInternalClassification) { m_hasInternalClassification = hasInternalClassification;}
    
    bool                            IsTransient                            () const { return 0/* == m_elementHandleP->GetElementRef()*/; }

    bool                            IsFromFile                             () const { return IsTransient(); }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * NOTE: Resolution of TR #335455 required this kind of knowledge in order to avoid
    *       the performance penalty of calling a method for every point. There should
    *       be an accessor exported from PointCloudHandler in order to make it simpler
    *       for the user to access this information. Even better: user shouldn't even
    *       have to be aware of this if clipping of points was handled under the hood
    *       of PointCloudDataQuery.
    * @bsimethod                                                  Raymond.Gauthier   03/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool                            IsClipped                              () const
        {
        // When source is drawn from an existing element it may be clipped. However, when
        // drawn from a file, we assumed it couldn't.
        /*if (IsFromFile())
            return false;
        

        PointCloudPropertiesPtr pointCloudPropertiesP(m_handler.GetPointCloudProperties(*m_elementHandleP));
        PointCloudClipPropertiesPtr pointCloudClipPropertiesP(m_handler.GetPointCloudClipProperties(*m_elementHandleP));
       
        // As soon as at least one of these call return successfully, we know that POD element
        // may be or is clipped.
        OrientedBox clipBoundaryAsOB;
         
        if (BSISUCCESS == pointCloudClipPropertiesP->GetClipBoundary(*pointCloudPropertiesP, clipBoundaryAsOB))
            return true;

        bvector<DPoint3d> clipBoundaryAsVec;
        if (BSISUCCESS == pointCloudClipPropertiesP->GetClipBoundaryPolygon(*pointCloudPropertiesP, clipBoundaryAsVec))
            return true;

        OrientedBoxList clipMaskAsOBList;
        if (BSISUCCESS == pointCloudClipPropertiesP->GetClipMaskList(*pointCloudPropertiesP, clipMaskAsOBList))
            return true;

        bvector<bvector<DPoint3d>> clipMaskAsListVec;
        if (BSISUCCESS == pointCloudClipPropertiesP->GetClipMaskList(*pointCloudPropertiesP, clipMaskAsListVec))
            return true;

        PointCloudClipReferencePtr pClipRef = m_handler.GetClipReference(*m_elementHandleP);
        if (pClipRef.IsValid())
            return true;

        // We found no clips or masks*/
        return false;
        }

    };



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PODFileSourceCreator : public LocalFileSourceCreatorBase
    {    
    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.pod";
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        // TDORAY: Bad... do a real is kind of here...
        return DefaultSupports(pi_rSourceRef);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   sourceRef,
                                                                            Log&                        log) const override
        {      

        return PODSource::CreateFrom(sourceRef.GetPath());
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsiclass                                                Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PODElementSourceCreator : public DGNElementSourceCreatorBase
    {
    virtual uint32_t                    _GetElementType                    () const override
        {
        //return EXTENDED_ELM;
        return 0;
        }

    virtual uint32_t                    _GetElementHandlerID               () const override
        {
        //return ElementHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Handler).GetId();
        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const DGNElementSourceRef&  sourceRef) const override
        {
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const DGNElementSourceRef&  sourceRef,
                                                                            Log&                        log) const override
        {

        return PODSource::CreateFrom(sourceRef.GetLocalFileP()->GetPath());
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
#define PointCloudChannels_Is_Point_Visible(bValue)     ( (bValue &  0x07) != 0)  //if one of the first 3 bits On

struct RgbColorDef
    {
    Byte    red;
    Byte    green;
    Byte    blue;
    };

class PODPointExtractor : public InputExtractorBase
    {
private:
    friend class                    PODPointExtractorCreator;

    static const uint32_t               MAX_PT_QTY = 100000;

    PointCloudScenePtr              m_pointCloudScenePtr;
    PointCloudChannelVector         m_queryChannels;
    
    PODPacketProxy<DPoint3d>        m_packetXYZ;
    PODPacketProxy<RgbColorDef>     m_packetRGB;
    PODPacketProxy<short>           m_packetIntensity;
    
    bool                            m_reachedEof;
    PointCloudQueryBuffersPtr       m_pointCloudQueryBufferPtr;
    bool                            m_isClip;


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        PODPointExtractor  (const PointCloudScenePtr& pointCloudScenePtr,
                                                        bool                           isClip)
        :   m_pointCloudScenePtr(pointCloudScenePtr),
            m_reachedEof(false), 
            m_isClip(isClip)
        {
        
        m_pointCloudQueryBufferPtr = PointCloudQueryBuffers::Create(MAX_PT_QTY, (uint32_t)PointCloudChannelId::Rgb |
                                                                   (uint32_t)PointCloudChannelId::Xyz |
                                                                   (uint32_t)PointCloudChannelId::Intensity |
                                                                   (uint32_t)PointCloudChannelId::Filter);

        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     pi_rRawEntities) override
        {
        m_packetXYZ.AssignTo(pi_rRawEntities[0]);
        m_packetRGB.AssignTo(pi_rRawEntities[1]);
        m_packetIntensity.AssignTo(pi_rRawEntities[2]);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        PThandle query = m_pointCloudScenePtr->GetVisiblePointsQueryHandle()->GetHandle();
        PointCloudVortex::SetQueryScope(query, m_pointCloudScenePtr->GetSceneHandle());
        PointCloudVortex::SetQueryRGBMode(query, QUERY_RGB_MODE_ACTUAL);
        PointCloudVortex::SetQueryDensity(query, QUERY_DENSITY_FULL, 1.0);

        uint32_t pointsReadQty = m_pointCloudQueryBufferPtr->GetPoints(query);

        unsigned char* filterBufferP(m_pointCloudQueryBufferPtr->GetFilterBuffer());

        size_t packetInd = 0;

        if (m_isClip)
            {
            for (size_t pointInd = 0; pointInd < pointsReadQty; pointInd++)
                {
                if (PointCloudChannels_Is_Point_Visible(filterBufferP[pointInd]))
                    {
                    m_packetXYZ.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetXyzBuffer()[pointInd];
                    RgbColorDef tmpColor;
                    tmpColor.blue = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetBlue();
                    tmpColor.red = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetRed();
                    tmpColor.green = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetGreen();
                    m_packetRGB.Edit()[packetInd] = tmpColor;
                    m_packetIntensity.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetIntensityBuffer()[pointInd];
                    packetInd++;
                    }            
                }
            }
        else
            {
            memcpy(m_packetXYZ.Edit(), m_pointCloudQueryBufferPtr->GetXyzBuffer(), sizeof(DPoint3d) * pointsReadQty);
            memcpy(m_packetRGB.Edit(), m_pointCloudQueryBufferPtr->GetRgbBuffer(), sizeof(RgbColorDef) * pointsReadQty);
            packetInd = pointsReadQty;
            }
        
        m_packetXYZ.SetSize(packetInd);
        m_packetRGB.SetSize(packetInd);

        m_reachedEof = (0 == pointsReadQty);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return !m_reachedEof;   
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PODPointExtractorWithInternalClassif : public InputExtractorBase
    {
private:
    friend class                    PODPointExtractorCreator;

    static const uint32_t               MAX_PT_QTY = 100000;



    PointCloudScenePtr              m_pointCloudScenePtr;
    PointCloudQueryBuffersPtr       m_pointCloudQueryBufferPtr;
    PointCloudChannelVector         m_queryChannels;

    PODPacketProxy<DPoint3d>        m_packetXYZ;
    PODPacketProxy<RgbColorDef>     m_packetRGB;
    PODPacketProxy<short>           m_packetIntensity;
    PODPacketProxy<unsigned char>   m_classification;

    bool                            m_reachedEof;
    bool                            m_isClip;
    bool                            m_isGroundDetection;


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        PODPointExtractorWithInternalClassif
                                                                   (PointCloudScenePtr pointCloudScenePtr,
                                                                    bool isClip,
                                                                    bool isGroundDetection)
        :   m_pointCloudScenePtr(pointCloudScenePtr),
            m_reachedEof(false), 
            m_isClip(isClip),
            m_isGroundDetection(isGroundDetection)
        {
        //m_elHandle.AddToModel();

        if (m_isGroundDetection)
        {
#ifdef SCALABLE_MESH_ATP

            clock_t startClock = clock();    
#endif
            //NEEDS_WORK_SM_IMPORTER_GROUND
            //NEEDS_WORK_SM_IMPORTER : Should backup the current classif file if any and restore it after
//            GroundDetectionParametersPtr pParam(GroundDetectionParameters::Create());
//            GroundDetectionManager::DoGroundDetection(m_elHandle, *pParam);

#ifdef SCALABLE_MESH_ATP
            double t = ((double)clock() - startClock) / CLOCKS_PER_SEC / 60.0;
            AddGroundDetectionDuration(t);
#endif

        }
        
        ////NEEDS_WORK_SM_IMPORTER_GROUND
//        m_queryChannels.push_back(GroundDetectionManager::GetChannelFromPODElement(m_elHandle));

        uint32_t channelFlags = ((uint32_t)PointCloudChannelId::Rgb | (uint32_t)PointCloudChannelId::Xyz | (uint32_t)PointCloudChannelId::Intensity | (uint32_t)PointCloudChannelId::Classification| (uint32_t)PointCloudChannelId::Filter);

        m_pointCloudQueryBufferPtr = PointCloudQueryBuffers::Create(MAX_PT_QTY, channelFlags, m_queryChannels);
        //m_pointCloudQueryBufferPtr = m_dataQueryPtr->CreateBuffers(MAX_PT_QTY, channelFlags, m_queryChannels);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     dst) override
        {
        m_packetXYZ.AssignTo(dst[0]);
        m_packetRGB.AssignTo(dst[1]);
        m_packetIntensity.AssignTo(dst[2]);
        m_classification.AssignTo(dst[3]);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {

        PThandle query = m_pointCloudScenePtr->GetVisiblePointsQueryHandle()->GetHandle();
        PointCloudVortex::SetQueryScope(query, m_pointCloudScenePtr->GetSceneHandle());
        PointCloudVortex::SetQueryRGBMode(query, QUERY_RGB_MODE_ACTUAL);
        PointCloudVortex::SetQueryDensity(query, QUERY_DENSITY_FULL, 1.0);

        uint32_t pointsReadQty = m_pointCloudQueryBufferPtr->GetPoints(query);
                
        size_t packetInd = 0;

        if (m_isClip || m_isGroundDetection)
            { 
            unsigned char* filterBufferP(m_pointCloudQueryBufferPtr->GetFilterBuffer());

            for (size_t pointInd = 0; pointInd < pointsReadQty; pointInd++)
                {
                if (PointCloudChannels_Is_Point_Visible(filterBufferP[pointInd]))
                    {
                    if(m_isGroundDetection)
                        {
                        unsigned char* classificationBuffer = (unsigned char*)m_pointCloudQueryBufferPtr->GetChannelBuffer((IPointCloudChannelP)(m_queryChannels[0]));
                        if(classificationBuffer[pointInd] == GROUND_CHANNEL_NUMBER) // if ground
                            {
                            m_packetXYZ.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetXyzBuffer()[pointInd];
                            RgbColorDef tmpColor;
                            tmpColor.blue = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetBlue();
                            tmpColor.red = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetRed();
                            tmpColor.green = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetGreen();
                            m_packetRGB.Edit()[packetInd] = tmpColor;
                            m_packetIntensity.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetIntensityBuffer()[pointInd];
                            m_classification.Edit()[packetInd] = classificationBuffer[pointInd];
                            packetInd++;
                            }
                        }
                    else
                        {
                        m_packetXYZ.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetXyzBuffer()[pointInd];
                        RgbColorDef tmpColor;
                        tmpColor.blue = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetBlue();
                        tmpColor.red = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetRed();
                        tmpColor.green = m_pointCloudQueryBufferPtr->GetRgbBuffer()[pointInd].GetGreen();
                        m_packetRGB.Edit()[packetInd] = tmpColor;
                        m_packetIntensity.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetIntensityBuffer()[pointInd];
                        m_classification.Edit()[packetInd] = m_pointCloudQueryBufferPtr->GetClassificationBuffer()[pointInd];
                        packetInd++;
                        }
                    }            
                }
            }
        else
            {
            memcpy(m_packetXYZ.Edit(), m_pointCloudQueryBufferPtr->GetXyzBuffer(), sizeof(DPoint3d) * pointsReadQty);
            memcpy(m_packetRGB.Edit(), m_pointCloudQueryBufferPtr->GetRgbBuffer(), sizeof(RgbColorDef) * pointsReadQty);
            memcpy(m_packetIntensity.Edit(), m_pointCloudQueryBufferPtr->GetIntensityBuffer(), sizeof(int64_t) * pointsReadQty);
            memcpy(m_classification.Edit(), m_pointCloudQueryBufferPtr->GetClassificationBuffer(), sizeof(unsigned char) * pointsReadQty);

            packetInd = pointsReadQty;
            }

        m_packetXYZ.SetSize(packetInd);
        m_packetRGB.SetSize(packetInd);
        m_packetIntensity.SetSize(packetInd);
        m_classification.SetSize(packetInd);

        m_reachedEof = (0 == pointsReadQty);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return !m_reachedEof;
        }

    ~PODPointExtractorWithInternalClassif()
        {
//            m_elHandle.DeleteFromModel();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PODPointExtractorCreator : public InputExtractorCreatorMixinBase<PODSource>
    {

    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == PointTypeFamilyCreator().Create();
        }



    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (PODSource&                              sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&   source,
                                                                                    const ExtractionQuery&                  selection) const override
        {
        SourceImportConfig* sourceImportConf = source.GetSourceImportConfigC();
        ScalableMeshData data = sourceImportConf->GetReplacementSMData();
        if(data.IsGroundDetection())
            return RawCapacities (PODPointExtractor::MAX_PT_QTY * sizeof(DPoint3d),
            PODPointExtractor::MAX_PT_QTY * sizeof(RgbColorDef),
            PODPointExtractor::MAX_PT_QTY * sizeof(short),
            PODPointExtractor::MAX_PT_QTY * sizeof(unsigned char));
        else
            return (sourceBase.HasClassification()) ?
                        RawCapacities (PODPointExtractor::MAX_PT_QTY * sizeof(DPoint3d),
                                       PODPointExtractor::MAX_PT_QTY * sizeof(RgbColorDef),
                                       PODPointExtractor::MAX_PT_QTY * sizeof(short),
                                       PODPointExtractor::MAX_PT_QTY * sizeof(unsigned char))
                        :
                        RawCapacities (PODPointExtractor::MAX_PT_QTY * sizeof(DPoint3d),
                                       PODPointExtractor::MAX_PT_QTY * sizeof(RgbColorDef),
                                       PODPointExtractor::MAX_PT_QTY * sizeof(short));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static InputExtractorBase*                  CreateExtractor                    (PODSource&                     sourceBase,
                                                                                    const PointCloudScenePtr& pointCloudScenePtr,
                                                                                    bool                           isClipped,
                                                                                    bool                           isGroundDetection)
        {
        // NEEDS_WORK_SM : internal classification => classification ?
       /* if (sourceBase.HasInternalClassification() || isGroundDetection)
            return new PODPointExtractorWithInternalClassif(dataQueryPtr, sourceBase.GetElementHandle(), isClipped, isGroundDetection);*/

        return new PODPointExtractor(pointCloudScenePtr, isClipped);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (PODSource&                              sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&  source,
                                                                                    const ExtractionQuery&                  selection,
                                                                                    const ExtractionConfig&                 config,
                                                                                    Log&                                    log) const override
        {
        // Initialize our data query
        static const DPoint3d BottomExtentCorner = {-(numeric_limits<double>::max)(),
                                                    -(numeric_limits<double>::max)(),
                                                    -(numeric_limits<double>::max)()};
        static const DPoint3d TopExtentCorner = {(numeric_limits<double>::max)(),
                                                 (numeric_limits<double>::max)(),
                                                 (numeric_limits<double>::max)()};


        /*ElementHandle& rElementHandle = sourceBase.GetElementHandle();

        IPointCloudDataQueryPtr pDataQuery = IPointCloudDataQuery::CreateBoundingBoxQuery(rElementHandle, BottomExtentCorner, TopExtentCorner);

        if (0 == pDataQuery.get())
            return 0;

        Transform transform;

        pDataQuery->GetUORToNativeTransform(transform);

        /*if (sourceBase.IsFromFile())
            pDataQuery->SetIgnoreTransform(true);*/

        //pDataQuery->SetDensity (IPointCloudDataQuery::QUERY_DENSITY_FULL, 1);
        //IPointCloudChannelVector queryChannels;
        //IPointCloudDataQueryPtr pDataQuery;
        /*IPointCloudDataQueryPtr pDataQuery = IPointCloudDataQuery::CreateBuffers(100000, (uint32_t)PointCloudChannelId::Rgb |
                                                                                 (uint32_t)PointCloudChannelId::Xyz |
                                                                                 (uint32_t)PointCloudChannelId::Intensity |
                                                                                 (uint32_t)PointCloudChannelId::Filter,
                                                                                 queryChannels);*/

        PointCloudScenePtr pointCloudScene = sourceBase.GetFileQuery();

        const bool isClipped = sourceBase.IsClipped();
        
        SourceImportConfig* sourceImportConf = source.GetSourceImportConfigC();
        ScalableMeshData data = sourceImportConf->GetReplacementSMData();

        return CreateExtractor(sourceBase, pointCloudScene, isClipped, data.IsGroundDetection());
        }
    };




/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier    08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class Point3d64f_R16G16B16_I16_C8ToPoint3d64fConverter : public TypeConversionFilterBase
    {
    enum
        {
        SRC_POINT_DIM,
        SRC_CLASSIF_DIM = 3,
        };

    enum
        {
        DST_POINT_DIM
        };


    ConstPacketProxy<DPoint3d>                  m_srcPtPacket;
    ConstPacketProxy<unsigned char>                     m_srcPtClassifPacket;

    PODPacketProxy<DPoint3d>                    m_dstPtPacket;



    /*---------------------------------------------------------------------------------**//**
    * @description  Remove all points that do not respect specified Predicate's selection
    *               criteria.
    *
    *               Predicate -> A predicate of the following form
    *                            "bool Predicate(const DPoint3d& pi_rPt,
    *                                            UChar pi_rPtClass)"
    *                            that return true when the point as specified by Predicate's
    *                            parameters should be removed.

    * @return       Kept point quantity
    * @bsimethod                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename Predicate>
    static size_t                               RemovePointsIf         (const DPoint3d*                 srcPtP,
                                                                        const unsigned char*                    srcPtClassP,
                                                                        size_t                          srcPtQty,
                                                                        DPoint3d*                       dstPtP,
                                                                        Predicate                       shouldPtBeRemoved)
        {
        const DPoint3d* const   srcPtEnd = srcPtP + srcPtQty;
        DPoint3d* const         dstPtBegin = dstPtP;

        while (srcPtP != srcPtEnd)
            {
           // if (!shouldPtBeRemoved(*srcPtP, *srcPtClassP))
                {
                *dstPtP++ = *srcPtP;
                }

            ++srcPtP;
            ++srcPtClassP;
            }

        return distance(dstPtBegin, dstPtP);
        }


    virtual void                                _Assign                (const PacketGroup&              src,
                                                                        PacketGroup&                    dst) override
        {
        m_srcPtPacket.AssignTo(src[SRC_POINT_DIM]);
        //m_srcPtClassifPacket.AssignTo(src[SRC_CLASSIF_DIM]);
        m_dstPtPacket.AssignTo(dst[DST_POINT_DIM]);
        }

    virtual void                                _Run                   () override
        {
        // NEEDS_WORK_SM: comment for now GROUND_CLASS because we work with Scalable Mesh and not Terrain Model.
        // Maybe later we want only a specific class (not only GROUND, but maybe building => add user choice for this)
        //static const UChar GROUND_CLASS_ID = 2;
        static const struct
            {
            bool operator () (const DPoint3d& pi_rPt, unsigned char pi_rPtClass) const
                {
                return false;//GROUND_CLASS_ID != pi_rPtClass;
                }
            } SHOULD_POINT_BE_REMOVED_PREDICATE;

        m_dstPtPacket.SetSize(RemovePointsIf(m_srcPtPacket.Get(),
                                             //m_srcPtClassifPacket.Get(),
                                             nullptr,
                                             m_srcPtPacket.GetSize(),
                                             m_dstPtPacket.Edit(),
                                             SHOULD_POINT_BE_REMOVED_PREDICATE));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier    08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3d64f_R16G16B16_I16_C8ToPoint3d64fConverterCreator : public TypeConversionFilterCreatorBase
    {
    enum
        {
        SRC_POINT_DIM,
        };

    enum
        {
        DST_POINT_DIM
        };

    explicit                                Point3d64f_R16G16B16_I16_C8ToPoint3d64fConverterCreator
                                                                       ()
        :   TypeConversionFilterCreatorBase(PointType3d64f_R16G16B16_I16_C8Creator().Create(), PointType3d64fCreator().Create())
        {
        }

    virtual TypeConversionFilterBase*       _Create                    (const FilteringConfig&      config,
                                                                        Log&                        log) const override
        {
        return new Point3d64f_R16G16B16_I16_C8ToPoint3d64fConverter;
        }

    virtual void                            _Bind                      (const PacketGroup&          src,
                                                                        PacketGroup&                dst) const
        {
        dst[DST_POINT_DIM].BindUseSameAs(src[SRC_POINT_DIM]);
        }
    virtual void                            _Bind                      (PacketGroup&                src,
                                                                        PacketGroup&                dst) const
        {
        dst[DST_POINT_DIM].BindUseSameAs(src[SRC_POINT_DIM]);
        }
    };

} //END UNAMED NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/

void RegisterPODImportPlugin()
    {
    if (!ScalableMeshLib::GetHost().GetScalableMeshAdmin()._CanImportPODfile())
        return;
    else
        {

        static bool s_loaded = false;
        if (!s_loaded)
            {
            s_loaded = LoadPointoolsDLL("PointoolsVortexAPI.dll");
            }

        if (!ptIsInitialized())
            {
            ptInitialize(vortexLicCode);
            }

        BePointCloudApi::Initialize();
        static const SourceRegistry::AutoRegister<PODFileSourceCreator> s_RegisterPODFile;
        static const SourceRegistry::AutoRegister<PODElementSourceCreator> s_RegisterPODElement;
        static const ExtractorRegistry::AutoRegister<PODPointExtractorCreator> s_RegisterPODExtractor;
        static const TypeConversionFilterRegistry::AutoRegister<Point3d64f_R16G16B16_I16_C8ToPoint3d64fConverterCreator> s_RegisterConverter;
        }       
    } 

END_BENTLEY_SCALABLEMESH_NAMESPACE
