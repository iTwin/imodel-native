/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNLevelImporter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"                         
#include <ScalableMesh\ScalableMeshLib.h>
#include <ScalableMesh\ScalableMeshAdmin.h>
#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\Import\Attachment.h>
#include <ScalableMesh\Import\ContentDescriptor.h>
#include <ScalableMesh\Import\DataType.h>
#include <ScalableMesh\Import\SourceReference.h>
#include <ScalableMesh\Import\SourceReferenceVisitor.h>
#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceReferenceV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\Memory\PacketAccess.h>
#include <ScalableMesh\Memory\Packet.h>
#include <ScalableMesh\Type\IScalableMeshLinear.h>
#include <ScalableMesh\Type\IScalableMeshPoint.h>
#include <ScalableMesh\Type\IScalableMeshMesh.h>




#include "DGNModelUtilities.h"      
#include "PluginUtils.h"
#include "ElemSourceRef.h"
#include "ElementType.h"


#define PointCloudMinorId_Handler 1

//USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)

USING_NAMESPACE_BENTLEY_SCALABLEMESH



namespace { //BEGIN UNAMED NAMESPACE




struct DTMFeaturesStruct
{
    DTMFeatureType m_featureType;
    IDTMFeatureArray<DPoint3d>& m_featureArray;

    explicit DTMFeaturesStruct(DTMFeatureType& featureType, IDTMFeatureArray<DPoint3d>& featureArray)
        : m_featureArray(featureArray),
        m_featureType(featureType)
    {}
};

struct ElementStats
    {
    ElementPointStats   m_point;
    ElementLinearStats  m_linear;
    ElementMeshStats    m_mesh;

    explicit            ElementStats           ()
        {
        }
    };

struct RefHolder
    {
    DGNModelRefHolder   m_modelRef;
    list<SourceRef>&    m_srcRefArray;

    explicit            RefHolder              (const DGNModelRefHolder& modelRefPtr, list<SourceRef>& srcRefArray)
        : m_modelRef(modelRefPtr),
          m_srcRefArray(srcRefArray)
        {
        }
    };

//Copied from MstnPlatform\PublicAPI\Mstn\MdlApi\scanner.h
#define ELEINVISIBLE 0x0080

struct ElementIterator
    {
    ScanCriteria* m_scanCriteria;

    explicit            ElementIterator        (DgnModelRefP modelRefPtr, LevelId levelID)
        {
        m_scanCriteria = new ScanCriteria(); 
        m_scanCriteria->AddSingleLevelTest (levelID);
        m_scanCriteria->SetPropertiesTest (0, ELEINVISIBLE);
        m_scanCriteria->SetModelRef (modelRefPtr);
        m_scanCriteria->SetReturnType (MSSCANCRIT_ITERATE_ELMREF, false, false);        
        }

                        ~ElementIterator       ()
        {
        delete m_scanCriteria;        
        }

    int                 AddSingleElementTypeTest           (const MSElementTypes& type)
        {
        m_scanCriteria->AddSingleElementTypeTest (type);
        return SUCCESS;
        }

    int                 Scan                   (PFScanElemRefCallback callbackFunc, void* userArgP)
        {
        m_scanCriteria->SetElemRefCallback (callbackFunc, userArgP);                        
        return m_scanCriteria->Scan (NULL, NULL, NULL, NULL);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int ComputeCountsCallback(ElementRefP elmRef, void* userArgP, ScanCriteria* scanCritP)
    {
    ElementStats& stats = *((ElementStats*)userArgP);
    
    EditElementHandle handle(elmRef, scanCritP->GetModelRef());
    MSElementDescrP edP = handle.GetElementDescrP();

    // Extract element type and compute count
    ElementPointExtractor::GetFor(edP).ComputeStats(edP, stats.m_point);
    ElementLinearExtractor::GetFor(edP).ComputeStats(edP, stats.m_linear);
    ElementMeshExtractor::GetFor(edP).ComputeStats(edP, stats.m_mesh);

    return SUCCESS;
    }
 
/*---------------------------------------------------------------------------------**//**
* @description
* NTERAY: This whole method should be redesigned in order to implement a kind of
*         registration mechanism that would facilitate introduction of new element
*         types instead of hard to maintain logic.
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int CreateSourceRefListCallback(ElementRefP elmRef, void* userArgP, ScanCriteria* scanCritP)
    {
    RefHolder& ref = *((RefHolder*)userArgP);

    EditElementHandle elHandle(elmRef, ref.m_modelRef.GetP());

    bool supported = false;

    // Raster element
    MSElementDescrP edP = elHandle.GetElementDescrP();
    
    if(RASTER_FRAME_ELM == edP->el.ehdr.type)
        {
        DgnRasterP dgnRasterP = 0;
        if (BSISUCCESS == mdlRaster_handleFromElementRefGet(&dgnRasterP, elHandle.GetElementRef(), elHandle.GetModelRef()) &&
            mdlRaster_HasDEMFilters(dgnRasterP))
            {
            SourceRef srcRef = RasterElemSourceRef::CreateFrom(elmRef, ref.m_modelRef);
            ref.m_srcRefArray.push_back(srcRef);
            supported = true;
            }
        }
    else if (EXTENDED_ELM == edP->el.ehdr.type)
        {
        // DTM element        
        if (ElementHandlerManager::GetHandlerId (elHandle) == MrDTMDefaultElementHandler::GetElemHandlerId())
            {           
            // STM element
            SourceRef srcRef = STMElemSourceRef::CreateFrom(elmRef, ref.m_modelRef);
            ref.m_srcRefArray.push_back(srcRef);
            supported = true;
            }
        else
        if (ElementHandlerManager::GetHandlerId (elHandle) == DTMElementHandler ::GetElemHandlerId()
            || ElementHandlerManager::GetHandlerId(elHandle) == ElementHandlerId(XATTRIBUTEID_CIF, ELEMENTHANDLER_DTMOVERRIDESYMBOLOGY))
            {
            // Civil element
            SourceRef srcRef = CivilElemSourceRef::CreateFrom(elmRef, ref.m_modelRef);
            ref.m_srcRefArray.push_back(srcRef);
            supported = true;            
            }                    
        // POD element        
        else if(ElementHandlerManager::GetHandlerId(elHandle) == ElementHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Handler))
            {
            SourceRef srcRef = PODElemSourceRef::CreateFrom(elmRef, ref.m_modelRef);
            ref.m_srcRefArray.push_back(srcRef);
            supported = true;
            }
        }

    // Enable for debug purpose
    assert(supported);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int ScanPointsCallback(ElementRefP elmRef, void* userArgP, ScanCriteria* scanCritP)
    {
    HPU::Array<DPoint3d>& pointArray = *((HPU::Array<DPoint3d>*)userArgP);

    EditElementHandle handle(elmRef, scanCritP->GetModelRef());
    MSElementDescrP edP = handle.GetElementDescrP();

    // Extract element type and import points
    return ElementPointExtractor::GetFor(edP).Scan(edP, pointArray);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int ScanFeaturesCallback(ElementRefP elmRef, void* userArgP, ScanCriteria* scanCritP)
    {
        DTMFeaturesStruct featuresStruct = *(DTMFeaturesStruct*)userArgP;

    EditElementHandle handle(elmRef, scanCritP->GetModelRef());
    MSElementDescrP edP = handle.GetElementDescrP();



    return ElementLinearExtractor::GetFor(edP).Scan(edP, featuresStruct.m_featureArray, featuresStruct.m_featureType);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ScanMeshesCallback(ElementRefP elmRef, void* userArgP, ScanCriteria* scanCritP)
    {
    IDTMFeatureArray<DPoint3d>& featureArray = *((IDTMFeatureArray<DPoint3d>*)userArgP);

    EditElementHandle handle(elmRef, scanCritP->GetModelRef());
    MSElementDescrP edP = handle.GetElementDescrP();

    // Extract element type and import features
    return ElementMeshExtractor::GetFor(edP).Scan(edP, featureArray);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelSource : public SourceMixinBase<DGNLevelSource>
    {
public:

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementStats GetStats() const
        {
        return m_stats;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    LevelId GetLevelID() const
        {
        return m_levelID;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnModelRefP GetDGNModelRefP() const
        {
        return m_refHolder.m_modelRef.GetP();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ComputeCounts(void* userArgP)
        {
        ElementIterator elementIt(m_refHolder.m_modelRef.GetP(), m_levelID);
        elementIt.AddSingleElementTypeTest(LINE_ELM);
        elementIt.AddSingleElementTypeTest(LINE_STRING_ELM);
        elementIt.AddSingleElementTypeTest(CMPLX_STRING_ELM);
        elementIt.AddSingleElementTypeTest(SHAPE_ELM);
        elementIt.AddSingleElementTypeTest(CMPLX_SHAPE_ELM);
        elementIt.AddSingleElementTypeTest(MESH_HEADER_ELM);
        elementIt.Scan(ComputeCountsCallback, userArgP);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ImportSourceRef(void* userArgP)
        {
        ElementIterator elementIt(m_refHolder.m_modelRef.GetP(), m_levelID);
        elementIt.AddSingleElementTypeTest(RASTER_FRAME_ELM);
        elementIt.AddSingleElementTypeTest(EXTENDED_ELM);
        elementIt.Scan(CreateSourceRefListCallback, userArgP);
        }

private:
    friend class            DGNLevelSourceCreator;

    LevelId                 m_levelID;
    ElementStats            m_stats;

    list<SourceRef>         m_srcRefList;
    RefHolder               m_refHolder;


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DGNLevelSource         (const DGNModelRefHolder&    modelRef,
                                                            LevelId                     levelID)
        : m_levelID(levelID),
          m_refHolder(modelRef, m_srcRefList)
        {
        ComputeCounts(&m_stats);
        ImportSourceRef(&m_refHolder);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_refHolder.m_modelRef.Reset();
        m_refHolder.m_srcRefArray.clear();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor        _CreateDescriptor  () const override
        {
        ContentDescriptor contentDesc(L"");

        // DataType
        DataTypeSet dataTypes;
        if (m_stats.m_point.HasAny())
            dataTypes.push_back(PointType3d64fCreator().Create());
        if (m_stats.m_linear.HasAny())
            dataTypes.push_back(LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create());
        if (m_stats.m_mesh.HasAny())
            dataTypes.push_back(MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create());
        ScalableMeshData data = ScalableMeshData::GetNull();

        auto layerDesc = ILayerDescriptor::CreateLayerDescriptor(L"",
                                  dataTypes,
                                  GetBSIElementGCSFromRootPerspective(m_refHolder.m_modelRef.GetP()),
                                  0,
                                  data);

        list<SourceRef>::const_iterator srcRefIter = m_srcRefList.begin();
        for(; srcRefIter != m_srcRefList.end(); srcRefIter++)
            {
            auto record = layerDesc->GetAttachmentRecord();
            record.push_back(AttachmentEntry(*srcRefIter));
            layerDesc->SetAttachmentRecord(record);
            }
        contentDesc.Add(layerDesc);

        return contentDesc;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType               () const override
        {
        return L"DGN Level";
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNSourceRefVisitor// : SourceRefVisitor
    {
    DGNModelRefHolder                   m_modelRef;
    LevelId                             m_levelID;

    explicit                            DGNSourceRefVisitor     ()
        :   m_levelID(0)
        {
        }


    static bool                         IsActiveDgnFile        (const WChar*                          dgnFilePath)
        {
        DgnFileP activeDgnFile(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetDgnFileP());
                            
        return 0 != activeDgnFile && 0 == wcscmp(dgnFilePath, activeDgnFile->GetFileName().c_str());
        }

    static bool                         IsActiveModel          (uint32_t                                  modelID)
        {              
        if (INVALID_MODELREF == ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef())
            return false;
        assert(0 == ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetParentModelRefP()); // Active model should also be a root model

        const ModelId activeModelID = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelId(); 

        return activeModelID == modelID;
        }


    static bool                         IsActiveModel          (const WChar*                          modelName)
        {               
        if (INVALID_MODELREF == ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef())
            return false;
        assert(0 == ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetParentModelRefP()); // Active model should also be a root model
     
        return 0 == wcscmp(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelNameCP(), modelName);
        }

    // TDORAY: Add dgn model source ref?

    static DGNFileHolder                OpenFile               (const WChar*                          filePath)
        {
        StatusInt openFileStatus = BSISUCCESS;
        DGNFileHolder dgnFile(OpenDGNFile(filePath, openFileStatus));

        if (BSISUCCESS != openFileStatus)
            throw FileIOException();

        return dgnFile;
        }

    static DGNFileHolder                GetActiveDgnFile       ()
        {               
        return DGNFileHolder::CreateFromActive(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetDgnFileP());
        }

    static DGNModelRefHolder            GetActiveModel         ()
        {       
        return DGNModelRefHolder::CreateFromActive(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef());
        }


    static DGNModelRefHolder            OpenRootModel          (const WChar*                          filePath,
                                                                uint32_t                                  modelID)
        {
        const bool isActiveDgnFile = IsActiveDgnFile(filePath);

        if (isActiveDgnFile && IsActiveModel(modelID))
            return GetActiveModel();

        DGNFileHolder dgnFile(isActiveDgnFile ? GetActiveDgnFile() : OpenFile(filePath));

        StatusInt modelOpenStatus = BSISUCCESS;
        DGNModelRefHolder modelRef = FindDGNModel(dgnFile, modelID, modelOpenStatus);

        if (BSISUCCESS != modelOpenStatus)
            throw FileIOException(); // TDORAY: Find code for this case

        return modelRef;
        }

    static DGNModelRefHolder            OpenRootModel          (const WChar*                          filePath,
                                                                const WChar*                          modelName)
        {
        const bool isActiveDgnFile = IsActiveDgnFile(filePath);

        if (isActiveDgnFile && IsActiveModel(modelName))
            return GetActiveModel();

        DGNFileHolder dgnFile(isActiveDgnFile ? GetActiveDgnFile() : OpenFile(filePath));

        StatusInt modelOpenStatus = BSISUCCESS;
        DGNModelRefHolder modelRef = FindDGNModel(dgnFile, modelName, modelOpenStatus);

        if (BSISUCCESS != modelOpenStatus)
            throw FileIOException(); // TDORAY: Find code for this case

        return modelRef;
        }

    static DGNModelRefHolder            OpenReferenceFromRoot  (const DGNModelRefHolder&                rootModel,
                                                                const WChar*                             persistantPath)
        {
        StatusInt referenceOpenStatus = BSISUCCESS;
        DGNModelRefHolder referenceModel(FindDGNReferenceFromRootModel(rootModel, persistantPath, referenceOpenStatus));

        if (BSISUCCESS != referenceOpenStatus)
            throw FileIOException(); // TDORAY: Find code for this case

        return referenceModel;
        }


    static LevelId                      FindLevel              (const DGNModelRefHolder&                model,
                                                                LevelId                                 levelID)
        {            
        if (!model.GetP()->GetLevelCache().GetLevel (levelID, true).IsValid())
            throw FileIOException(); // TDORAY: Find code for this case

        return levelID;
        }

    static LevelId                      FindLevel              (const DGNModelRefHolder&              model,
                                                                const WChar*                          levelName)
        {
        LevelId levelID = 0;
        if (!FindDGNLevelIDFromName(model.GetP(), levelName, levelID))
            throw FileIOException(); // TDORAY: Find code for this case

        return levelID;
        }


     void                        Visit                 (const DGNLevelByIDSourceRef&            sourceRef) 
        {
        const DGNModelRefHolder modelRef(OpenRootModel(sourceRef.GetDGNPathCStr(), sourceRef.GetModelID()));
        LevelId levelID = FindLevel(modelRef, sourceRef.GetLevelID());

        m_modelRef = modelRef;
        m_levelID = levelID;
        }

     void                        Visit                 (const DGNReferenceLevelByIDSourceRef&   sourceRef) 
        {
        const DGNModelRefHolder rootModel(OpenRootModel(sourceRef.GetDGNPathCStr(), sourceRef.GetRootModelID()));
        const DGNModelRefHolder referenceModel(OpenReferenceFromRoot(rootModel, sourceRef.GetRootToRefPersistentPathCStr()));
        LevelId levelID = FindLevel(referenceModel, sourceRef.GetLevelID());

        m_modelRef = referenceModel;
        m_levelID = levelID;
        }


     void                        Visit                 (const DGNLevelByNameSourceRef&          sourceRef) 
        {
        const DGNModelRefHolder modelRef(OpenRootModel(sourceRef.GetDGNPathCStr(), sourceRef.GetModelNameCStr()));
        LevelId levelID = FindLevel(modelRef, sourceRef.GetLevelNameCStr());

        m_modelRef = modelRef;
        m_levelID = levelID;
        }

     void                        Visit                 (const DGNReferenceLevelByNameSourceRef& sourceRef) 
        {
        const DGNModelRefHolder rootModel(OpenRootModel(sourceRef.GetDGNPathCStr(), sourceRef.GetRootModelNameCStr()));
        const DGNModelRefHolder referenceModel(OpenReferenceFromRoot(rootModel, sourceRef.GetRootToRefPersistentPathCStr()));
        LevelId levelID = FindLevel(referenceModel, sourceRef.GetLevelNameCStr());

        m_modelRef = referenceModel;
        m_levelID = levelID;
        }
        
        void Visit(const SourceRefBase& sourceRef)
        {
        if(dynamic_cast<const DGNReferenceLevelByNameSourceRef*>(&sourceRef) != nullptr)
           Visit(*dynamic_cast<const DGNReferenceLevelByNameSourceRef*>(&sourceRef));
        else if(dynamic_cast<const DGNLevelByNameSourceRef*>(&sourceRef) != nullptr)
           Visit(*dynamic_cast<const DGNLevelByNameSourceRef*>(&sourceRef));
        else if(dynamic_cast<const DGNReferenceLevelByIDSourceRef*>(&sourceRef) != nullptr)
           Visit(*dynamic_cast<const DGNReferenceLevelByIDSourceRef*>(&sourceRef));
        else if(dynamic_cast<const DGNLevelByIDSourceRef*>(&sourceRef) != nullptr)
           Visit(*dynamic_cast<const DGNLevelByIDSourceRef*>(&sourceRef));
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelSourceCreator : public SourceCreatorBase
    {
    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const SourceRef&                sourceRef) const override
        {
        return  DGNLevelByIDSourceRef::s_GetClassID() == sourceRef.GetClassID() ||
                DGNReferenceLevelByIDSourceRef::s_GetClassID() == sourceRef.GetClassID() ||
                DGNLevelByNameSourceRef::s_GetClassID() == sourceRef.GetClassID() ||
                DGNReferenceLevelByNameSourceRef::s_GetClassID() == sourceRef.GetClassID();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    *
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const SourceRef&                sourceRef,
                                                                            Log&                            log) const override
        {
        DGNSourceRefVisitor visitor;
        visitor.Visit(*sourceRef.m_basePtr);
        //sourceRef.Accept(visitor);

        assert(0 != visitor.m_modelRef.GetP()); // Should already have thrown on errors


        return new DGNLevelSource(visitor.m_modelRef, visitor.m_levelID);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelPointExtractor : public InputExtractorBase
    {
private:
    friend class DGNLevelPointExtractorFactory;

    // Dimension groups definition
    enum
    {
        DG_XYZ,
        DG_QTY,
    };


    DGNLevelSource&                 m_dgnLevel;
    bool                            m_hasNext;
    ElementIterator                 m_elementIt;


    PODPacketProxy<DPoint3d>        m_pointPacket;
    HPU::Array<DPoint3d>            m_pointArray;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DGNLevelPointExtractor          (DGNLevelSource& dgnLevel)
        :   m_dgnLevel(dgnLevel),
            m_hasNext(dgnLevel.GetStats().m_point.HasAny()),
            m_elementIt(dgnLevel.GetDGNModelRefP(), dgnLevel.GetLevelID())
        {
        m_elementIt.AddSingleElementTypeTest(LINE_ELM);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     rawEntities) override
        {
        m_pointPacket.AssignTo(rawEntities[0]);
        m_pointArray.WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (!m_hasNext)
            m_pointPacket.SetSize(0);

        m_pointArray.Clear();

        m_hasNext = (BUFF_FULL == m_elementIt.Scan(ScanPointsCallback, &m_pointArray));

        m_pointPacket.SetSize(m_pointArray.GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelPointExtractorFactory : public InputExtractorCreatorMixinBase<DGNLevelSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == PointTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(sourceBase.GetStats().m_point.GetPointCapacity() * sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        return new DGNLevelPointExtractor(sourceBase);
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelLinearExtractor : public InputExtractorBase
    {
private:
    friend class                            DGNLevelLinearExtractorCreator;

    // Dimension groups definition
    enum
    {
        DG_Header,
        DG_XYZ,
        DG_QTY,
    };

    DGNLevelSource&                         m_dgnLevel;
    bool                                    m_hasNext;
    ElementIterator                         m_elementIt;

    PODPacketProxy<IDTMFile::FeatureHeader> m_headerPacket;
    PODPacketProxy<DPoint3d>                m_pointPacket;
    IDTMFeatureArray<DPoint3d>              m_featureArray;

    DTMFeatureType                          m_linearFeatureType;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DGNLevelLinearExtractor        (DGNLevelSource& dgnLevel, const DTMFeatureType linearType = DTMFeatureType::Breakline)
        :   m_dgnLevel(dgnLevel),
            m_hasNext(dgnLevel.GetStats().m_linear.HasAny()),
            m_elementIt(dgnLevel.GetDGNModelRefP(), dgnLevel.GetLevelID())
        {
        m_elementIt.AddSingleElementTypeTest(LINE_ELM);
        m_elementIt.AddSingleElementTypeTest(LINE_STRING_ELM);
        m_elementIt.AddSingleElementTypeTest(CMPLX_STRING_ELM);
        m_elementIt.AddSingleElementTypeTest(SHAPE_ELM);
        m_elementIt.AddSingleElementTypeTest(CMPLX_SHAPE_ELM);

        m_linearFeatureType = linearType;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     rawEntities) override
        {
        m_headerPacket.AssignTo(rawEntities[DG_Header]);
        m_pointPacket.AssignTo(rawEntities[DG_XYZ]);
        m_featureArray.EditHeaders().WrapEditable(m_headerPacket.Edit(), 0, m_headerPacket.GetCapacity());
        m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (!m_hasNext)
            {
            m_headerPacket.SetSize(0);
            m_pointPacket.SetSize(0);
            }

        m_featureArray.Clear();

        DTMFeaturesStruct featureStruct(m_linearFeatureType, m_featureArray);

        m_hasNext = (BUFF_FULL == m_elementIt.Scan(ScanFeaturesCallback, &featureStruct));

        m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
        m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelLinearExtractorCreator : public InputExtractorCreatorMixinBase<DGNLevelSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == LinearTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(sourceBase.GetStats().m_linear.m_featureCount * sizeof(IDTMFile::FeatureHeader),
                             sourceBase.GetStats().m_linear.GetPointCapacity() * sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
            SourceImportConfig* sourceImportConf = source.GetSourceImportConfigC();
            ScalableMeshData data = sourceImportConf->GetReplacementSMData();
        return new DGNLevelLinearExtractor(sourceBase, data.GetLinearFeatureType());
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelMeshExtractor : public InputExtractorBase
    {
private:
    friend class                            DGNLevelMeshExtractorCreator;

    // Dimension groups definition
    enum
    {
        DG_Header,
        DG_XYZ,
        DG_QTY,
    };

    DGNLevelSource&                         m_dgnLevel;
    bool                                    m_hasNext;
    ElementIterator                         m_elementIt;

    PODPacketProxy<IDTMFile::FeatureHeader> m_headerPacket;
    PODPacketProxy<DPoint3d>                m_pointPacket;
    IDTMFeatureArray<DPoint3d>              m_featureArray;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DGNLevelMeshExtractor        (DGNLevelSource& dgnLevel)
        :   m_dgnLevel(dgnLevel),
            m_hasNext(dgnLevel.GetStats().m_mesh.HasAny()),
            m_elementIt(dgnLevel.GetDGNModelRefP(), dgnLevel.GetLevelID())
        {
        m_elementIt.AddSingleElementTypeTest(MESH_HEADER_ELM);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     rawEntities) override
        {
        m_headerPacket.AssignTo(rawEntities[DG_Header]);
        m_pointPacket.AssignTo(rawEntities[DG_XYZ]);
        m_featureArray.EditHeaders().WrapEditable(m_headerPacket.Edit(), 0, m_headerPacket.GetCapacity());
        m_featureArray.EditPoints().WrapEditable(m_pointPacket.Edit(), 0, m_pointPacket.GetCapacity());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (!m_hasNext)
            {
            m_headerPacket.SetSize(0);
            m_pointPacket.SetSize(0);
            }

        m_featureArray.Clear();

        m_hasNext = (BUFF_FULL == m_elementIt.Scan(ScanMeshesCallback, &m_featureArray));

        m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
        m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* NTERAY: It would be a good idea to isolate mesh element into an individual source
*         with its own module.
* @bsiclass                                                 Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DGNLevelMeshExtractorCreator : public InputExtractorCreatorMixinBase<DGNLevelSource>
    {
    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == MeshTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(sourceBase.GetStats().m_mesh.m_featureCount * sizeof(IDTMFile::FeatureHeader),
                             sourceBase.GetStats().m_mesh.GetPointCapacity() * sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (DGNLevelSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        return new DGNLevelMeshExtractor(sourceBase);
        }
    };

} //END UNAMED NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceRegistry::AutoRegister<DGNLevelSourceCreator> s_RegisterDGNLevelSourceFile;
const ExtractorRegistry::AutoRegister<DGNLevelPointExtractorFactory> s_RegisterDGNLevelPointExtractor;
const ExtractorRegistry::AutoRegister<DGNLevelLinearExtractorCreator> s_RegisterDGNLevelLinearExtractor;
const ExtractorRegistry::AutoRegister<DGNLevelMeshExtractorCreator> s_RegisterDGNLevelMeshExtractor;


END_BENTLEY_SCALABLEMESH_NAMESPACE
