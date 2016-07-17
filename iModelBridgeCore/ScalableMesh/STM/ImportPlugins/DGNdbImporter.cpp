/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNdbImporter.cpp $
|    $RCSfile: DGNdbImporter.cpp,v $
|   $Revision: 1.0 $
|       $Date: 2016/06/29 12:15:00 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"

#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <ScalableMesh/Plugin/IScalableMeshPolicy.h>
#include <ScalableMesh/Plugin/IScalableMeshFileUtilities.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#undef static_assert
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <json/json.h>



USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Plugin;

namespace
    { //BEGIN UNNAMED NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbSource : public SourceMixinBase<DgnDbSource>
    {
public:
    int                             GetFeatureCount() { return m_featureCount; }
    int                             GetPointCount() { return m_pointCount; }
    DgnDb*                          GetDatabase() { return m_database.get(); }

private:
    friend class                    DgnDbSourceCreator;

    int                             m_featureCount = 500;
    int                             m_pointCount = 50000;
    DgnDbPtr                           m_database;
    
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DgnDbSource(const WString&   pi_rFilePath)
        {
        BeSQLite::DbResult status;
        DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::Readonly);
        BeFileName fileName(pi_rFilePath);
        m_database = DgnDb::OpenDgnDb(&status, fileName, openParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_database = nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor() const override
        {

        //GCSFactory::Status gcsCreateStatus;
       //     GCS gcs = GetGCSFactory().Create(wktStr.c_str(), gcsCreateStatus);
            ScalableMeshData data = ScalableMeshData::GetNull();
        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
            MeshType3d64fCreator().Create(),
            GCS::GetNull(),
            0,
            data)
            );
        }

        /*---------------------------------------------------------------------------------**//**
        * @description
        * @bsimethod                                                  Elenie.Godzaridis   04/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual const WChar*         _GetType() const override
            {
            return L"DgnDb file";
            }


    };

    struct NullContext : ViewContext
        {
        void _AllocateScanCriteria() override { ; }
        QvElem* _DrawCached(IStrokeForCache&) { return nullptr; }
        void _DrawSymbol(IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip) override {}
        void _DeleteSymbol(IDisplaySymbol*) override {}
        bool _FilterRangeIntersection(GeometrySourceCR element) override { return false; }
        void _CookDisplayParams(ElemDisplayParamsR, ElemMatSymbR) override {}
        void _SetupOutputs() override { SetIViewDraw(*m_IViewDraw); }
        NullContext(DgnDbR db) { m_dgnDb = &db; m_IViewDraw = nullptr; m_IDrawGeom = nullptr; m_ignoreViewRange = true; }
        };

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsiclass                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbMeshExtractor : public InputExtractorBase
        {
        private:
            friend class                            DgnDbMeshExtractorCreator;

            PODPacketProxy<DPoint3d>                m_pointPacket;
            PODPacketProxy<int32_t>                m_indexPacket;
            PODPacketProxy<uint8_t>               m_metadataPacket;


            bool m_hasNext;

            int                                     m_defaultPtCount;
            DgnDbPtr                                m_database;
            DgnModelPtr                             m_model;
            DgnModel::const_iterator                m_currentElement;
            Transform                              m_scaleUnits;

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis  06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            explicit                        DgnDbMeshExtractor(DgnDbSource& dgndbFile)
                {
                m_database = dgndbFile.GetDatabase();
                m_hasNext = false;
                m_model = m_database->Models().GetModel(m_database->Models().QueryFirstModelId());
                m_model->FillModel();
                m_currentElement = m_model->begin();
                m_hasNext = m_currentElement != m_model->end();
                DPoint3d scale = DPoint3d::From(1, 1, 1);
                m_database->Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
                m_scaleUnits.InverseOf(Transform::FromScaleFactors(scale.x, scale.y, scale.z));
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                 Elenie.Godzaridis   06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Assign(PacketGroup&     rawEntities) override
                {
                m_pointPacket.AssignTo(rawEntities[0]);
                m_indexPacket.AssignTo(rawEntities[1]);
                m_metadataPacket.AssignTo(rawEntities[2]);
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Read() override
                {
                    m_pointPacket.SetSize(0);
                    m_indexPacket.SetSize(0);
                    m_metadataPacket.SetSize(0);
                if (m_hasNext)
                    {
                    DgnElementCPtr elem = m_currentElement->second;
                    bool filterElem = true;//elem->GetElementClass()->GetId() == 341 || elem->GetElementClass()->GetId() == 344 || elem->GetElementClass()->GetId() == 388;
                    if (filterElem && nullptr != elem->ToGeometrySource3d())
                        {
                        Transform transformToWorld;
                        Placement3dCR placement = elem->ToGeometrySource3d()->GetPlacement();
                        transformToWorld = placement.GetTransform();
                        bvector<int32_t> indices;
                        bvector<DPoint3d> pts;
                        ElementGeometryCollection collection(*m_database, dynamic_cast<const GeometricElement3d*>(elem.get())->GetGeomStream());
                        size_t offset = 0;
                        for (ElementGeometryPtr geometry : collection)
                            {
                            geometry->TransformInPlace(transformToWorld);
                            geometry->TransformInPlace(m_scaleUnits);
                            PolyfaceHeaderPtr mesh = geometry->GetAsPolyfaceHeader();
                            if (!mesh.IsValid()) continue;
                            ElemDisplayParams params(collection.GetElemDisplayParams()); //find whether element is textured
                            NullContext ct(*m_database);
                            params.Resolve(ct);
                            DgnMaterialId id = params.GetMaterial();
                            RenderMaterialPtr renderMat = JsonRenderMaterial::Create(*m_database, id);
                            if (renderMat.IsValid())
                                {
                                RenderMaterialMapPtr      patternMap = renderMat->_GetMap(RENDER_MATERIAL_MAP_Pattern);
                                if (patternMap.IsValid())
                                    ImageBufferPtr data = patternMap->_GetImage(*m_database);
                                }
                            mesh->Triangulate();
                            pts.insert(pts.end(), mesh->GetPointCP(), mesh->GetPointCP() + mesh->GetPointCount());
                            PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*mesh);
                            bvector<int> &pointIndex = vis->ClientPointIndex();
                            for (vis->Reset(); vis->AdvanceToNextFace();)
                                {
                                indices.push_back(pointIndex[0] + 1 +(int)offset);
                                indices.push_back(pointIndex[1] + 1 + (int)offset);
                                indices.push_back(pointIndex[2] + 1 + (int)offset);
                                }
                            offset = pts.size();
                            }
                        if (!pts.empty() && !indices.empty())
                            {
                            memcpy(m_pointPacket.Edit(), &pts[0], pts.size()*sizeof(DPoint3d));
                            m_pointPacket.SetSize(pts.size());
                            memcpy(m_indexPacket.Edit(), &indices[0], indices.size() * sizeof(int32_t));
                            m_indexPacket.SetSize(indices.size());
                            Json::Value val(Json::objectValue);
                            val["elementId"] = elem->GetElementId().GetValue();
                            Utf8String jsonString(Json::FastWriter().write(val));
                            memcpy(m_metadataPacket.Edit(), jsonString.data(), jsonString.SizeInBytes());
                            m_metadataPacket.SetSize(jsonString.SizeInBytes());
                            }
                        }
                    m_currentElement++;
                    }
                m_hasNext = m_currentElement != m_model->end();
//                m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
//                m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());
                }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.dgndb";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        if(!DefaultSupports(pi_rSourceRef))
            return false;

        return true;
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                 pi_warningLog) const override
        {
        return new DgnDbSource(pi_rSourceRef.GetPath());
        }
    };

    const SourceRegistry::AutoRegister<DgnDbSourceCreator> s_RegisterDgnDbFile;


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbMeshExtractorCreator : public InputExtractorCreatorMixinBase<DgnDbSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == MeshTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities(DgnDbSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(1000000*sizeof(DPoint3d), 5000000*sizeof(int32_t), 100*sizeof(uint8_t));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create(DgnDbSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        return new DgnDbMeshExtractor(sourceBase);
        }
    };


    const ExtractorRegistry::AutoRegister<DgnDbMeshExtractorCreator> s_RegisterDgnDbMeshImporter;
    }//END UNNAMED NAMESPACE