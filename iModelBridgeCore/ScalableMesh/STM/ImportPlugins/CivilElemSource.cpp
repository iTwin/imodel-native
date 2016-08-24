/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/CivilElemSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <GeoCoord\BaseGeoCoord.h>
#include <DgnPlatform\DgnPlatformAPI.h>
#include <DgnView\IViewManager.h>
#include <DgnView\IRedraw.h>
#include <DgnView\AccuSnap.h>
#include <DgnPlatform\DgnECProviderBase.h>
#include <DgnPlatform\ElementProperties.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>



#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>
#include <TerrainModel\ElementHandler\TMHandlersResources.h>
#include <TerrainModel\ElementHandler\TMHandlersResources.r.h>
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <TerrainModel\Core\bcDTMElement.h>
#include <TerrainModel\TerrainModel.h>                        
#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\Import\ContentConfig.h>
#include <ScalableMesh\Import\Config\Content\GCS.h>
#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\IScalableMeshPolicy.h>
#include <ScalableMesh\Plugin\IScalableMeshCivilDTMSource.h>
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>   

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)

//USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH


namespace {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilElementSourceDecorator : public SourceBase
    {
    friend class                    CivilElementSourceCreator;

    std::auto_ptr<SourceBase>            m_decoratedP;
    ElementHandle                      m_elHandle;

    explicit                        CivilElementSourceDecorator            (SourceBase*                 decoratedP,
                                                                            const ElementHandle&           elHandle)
        :   m_decoratedP(decoratedP),
            m_elHandle(elHandle)
        {
        }

    virtual ClassID                 _GetClassID                            () const
        { return Handler::GetClassID(*m_decoratedP); }

    virtual SourceBase&             _ResolveOriginal                       ()
        { return Handler::ResolveOriginal(*m_decoratedP); }

    virtual void                    _Close                                 ()
        { return Handler::Close(*m_decoratedP); }

    virtual ContentDescriptor       _CreateDescriptor                      () const
        {
        ContentDescriptor descriptor(Handler::CreateDescriptor(*m_decoratedP));

        GCSConfig storageGCSConfig(GetStorageGCS(m_elHandle));


        ContentConfig contentConfig;
        contentConfig.SetGCSConfig(storageGCSConfig);

        if (SMStatus::S_SUCCESS != descriptor.Configure(contentConfig, GetLog()))
            throw CustomError(L"Error configuring descriptor!");

        return descriptor;
        }

    virtual void                    _ExtendDescription                     (ContentDescriptor&          description) const
        { return Handler::ExtendDescription(*m_decoratedP, description); }

    virtual const WChar*         _GetType                               () const
        { return Handler::GetType(*m_decoratedP); }



    static GCS                      GetStorageGCS                          (const ElementHandle&           elHandle)
        {
        Transform storageToUorTransform;
        DTMElementHandlerManager::GetStorageToUORMatrix(storageToUorTransform, elHandle);


        GCS storageToRoot(GetBSIElementGCSFromRootPerspective(elHandle.GetModelRef()));

        TransfoModel storageToUorTransfoModel(TransfoModel::CreateFrom(FromBSITransform(storageToUorTransform)));

        if (SMStatus::S_SUCCESS != storageToRoot.AppendLocalTransform(LocalTransform::CreateFromToGlobal(storageToUorTransfoModel)))
            throw CustomError(L"Error appending storage to uor transform!");

        return storageToRoot;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsiclass                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CivilElementSourceCreator : public DGNElementSourceCreatorBase
    {
    virtual uint32_t                    _GetElementType                        () const override
        {
        return EXTENDED_ELM;
        }

    virtual uint32_t                    _GetElementHandlerID                   () const override
        {
        //DTMElementHandlerManager::InitializeDgnPlatform();
        return DTMElementHandler::GetElemHandlerId().GetId();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   04/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const DGNElementSourceRef&  sourceRef) const override
        {
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   04/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const DGNElementSourceRef&  sourceRef,
                                                                            Log&                        log) const override
        {
        using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Plugin::V0;


        //Retrieve BcDTM
        EditElementHandle elHandle(sourceRef.GetElementRef(), sourceRef.GetModelRef());

        RefCountedPtr<DTMDataRef> ref;
        DTMElementHandlerManager::GetDTMDataRef(ref, elHandle);

        if (0 == ref.get())
            return 0;

        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtm;
        ref->GetDTMReferenceStorage(dtm);
        if (0 == dtm.get())
            return 0;

        BcDTM* fullDTM = dtm->GetBcDTM();

        SourceBase* decoratedSourceP = CreateCivilDTMSource(*fullDTM, log);
        if (0 == decoratedSourceP)
            return 0;

        return new CivilElementSourceDecorator(decoratedSourceP, elHandle);
        }
    };

} // END unnamed namespace


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
/*void RegisterCivilImportPlugin (GeoDTMCoreInitRegistry& registry)
    {
    registry.Register(new RegisterPluginInit<SourceRegistry, CivilElementSourceCreator>);
    }*/

const SourceRegistry::AutoRegister<CivilElementSourceCreator> s_RegisterCivilElementSource;
END_BENTLEY_SCALABLEMESH_NAMESPACE
