/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/STMSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
        
#include <ScalableMesh\IScalableMeshPolicy.h>
#include <ScalableMesh\GeoCoords\DGNModelGeoref.h>
#include <ScalableMesh\GeoCoords\GCS.h>
#include <ScalableMesh\GeoCoords\Reprojection.h>
#include <ScalableMesh\Import\ContentConfig.h>
#include <ScalableMesh\Import\Config\Content\GCS.h>
#include <ScalableMesh\Import\Config\Content\Layer.h>
#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\Plugin\IScalableMeshSTMSource.h>

#include "PluginUtils.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)

//USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

namespace {

const double ANGULAR_TO_LINEAR_RATIO = GetAngularToLinearRatio(Unit::GetMeter(), Unit::GetDegree());

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class STMElementSourceDecorator : public SourceBase
    {  
    friend class                    STMElementSourceCreator;

    auto_ptr<SourceBase>            m_decoratedP;
    ElementHandle                   m_elHandle;

    explicit                        STMElementSourceDecorator              (SourceBase*                 decoratedP,
                                                                            const DGNElementSourceRef&  sourceRef)
        :   m_decoratedP(decoratedP),
            m_elHandle(sourceRef.GetElementRef(), sourceRef.GetModelRef())
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

        ContentConfig contentConfig;
        for (ContentDescriptor::const_iterator layerIt = descriptor.LayersBegin(), layersEnd = descriptor.LayersEnd();
             layerIt != layersEnd;
             ++layerIt)
            {
            LayerConfig layerConfig(DecorateLayer(descriptor.GetLayerIDFor(layerIt), *layerIt, m_elHandle));
            contentConfig.push_back(layerConfig);
            }

        if (ContentDescriptor::S_SUCCESS != descriptor.Configure(contentConfig, GetLog()))
            throw CustomError(L"Error configuring descriptor!");

        return descriptor;
        }

    virtual void                    _ExtendDescription                     (ContentDescriptor&          description) const
        { return Handler::ExtendDescription(*m_decoratedP, description); }

    virtual const WChar*         _GetType                               () const
        { return Handler::GetType(*m_decoratedP); }



    static LayerConfig              DecorateLayer                          (UInt                        layerID,
                                                                            const LayerDescriptor&      descriptor,
                                                                            const ElementHandle&           elHandle)
        {
        GCSConfig gcsConfig(AdaptGCS(descriptor.GetGCS(), elHandle));

        LayerConfig layerConfig(layerID);
        layerConfig.push_back(gcsConfig);


        return layerConfig;
        }

    static GCSConfig                AdaptGCS                               (const GCS&                  storageGCS,
                                                                            const ElementHandle&           elHandle)
        {
        GCS stmInModelGCS(GetModelMasterGCS(elHandle.GetModelRef()));

        if (storageGCS.HasGeoRef() && stmInModelGCS.HasGeoRef())
            {
            Reprojection toModelReprojection(GetReprojectionFactory().Create(storageGCS, stmInModelGCS, 0));

            TransfoModel localToGlobal(Combine(AsTransfoModel(toModelReprojection),
                                               GetModelLocalToGlobalTransfoModel(elHandle.GetModelRef(), stmInModelGCS.GetUnit(), stmInModelGCS.GetUnit())));


            if (GCS::S_SUCCESS != stmInModelGCS.AppendLocalTransform(LocalTransform::CreateFromToGlobal(localToGlobal)))
                throw CustomError(L"Error appending tranform");
            }
        else
            {
            const TransfoModel localToGlobal(GetModelLocalToGlobalTransfoModel(elHandle.GetModelRef(), storageGCS.GetUnit(), stmInModelGCS.GetUnit()));

            if (GCS::S_SUCCESS != stmInModelGCS.AppendLocalTransform(LocalTransform::CreateFromToGlobal(localToGlobal)))
                throw CustomError(L"Error appending tranform");
            }

        return GCSConfig(ReinterpretModelGCSFromRootPerspective(stmInModelGCS, elHandle.GetModelRef()));
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class STMElementSourceCreator : public DGNElementSourceCreatorBase
    {
    virtual UInt                    _GetElementType                    () const override
        {
        return EXTENDED_ELM;
        }

    virtual UInt                    _GetElementHandlerID               () const override
        {        
        return MrDTMDefaultElementHandler::GetElemHandlerId().GetId();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Supports                              (const DGNElementSourceRef&  sourceRef) const override
        {
        return 0 != sourceRef.GetLocalFileP();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                               Jean-Francois.Cote   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*             _Create                                (const DGNElementSourceRef&  sourceRef,
                                                                            Log&                        log) const override
        {
        using namespace Bentley::ScalableMesh::Plugin::V0;
        SourceBase* decoratedP = CreateSTMSource(sourceRef.GetLocalFileP()->GetPathCStr(), log);
        if (0 == decoratedP)
            return 0;

        return new STMElementSourceDecorator(decoratedP, sourceRef);
        }
    };

} // END unnamed namespace


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceRegistry::AutoRegister<STMElementSourceCreator> s_RegisterSTMElementSource;

END_BENTLEY_SCALABLEMESH_NAMESPACE
