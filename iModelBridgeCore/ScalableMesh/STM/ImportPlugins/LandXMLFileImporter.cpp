/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/LandXMLFileImporter.cpp $
|    $RCSfile: LandXMLFileImporter.cpp,v $
|   $Revision: 1.28 $
|       $Date: 2011/08/26 18:45:58 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <ScalableMesh/Plugin/IScalableMeshPolicy.h>

#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>

#include <TerrainModel/Formats/LandXMLImporter.h>

#include <STMInternal/Foundations/PrivateStringTools.h>

#include "CivilDTMHelpers.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH

using namespace Bentley::ScalableMesh::Plugin;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin { namespace V0 {

// Borrow some functionalities from civil dtm import module
InputExtractorBase* CreateCivilDTMPointExtractor (const PointHandler& handler);
InputExtractorBase* CreateCivilDTMLinearExtractor (const LinearHandler& handler);

}} // END namespace Plugin::V0
END_BENTLEY_SCALABLEMESH_NAMESPACE

namespace { //BEGIN UNAMED NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Surface
    {
private:
    struct                          Impl;
    typedef Bentley::RefCountedPtr<Impl>      
                                    ImplPtr;

    ImplPtr                         m_implP;
    WString                      m_name;

public:
    explicit                        Surface                        (BcDTMR   dtm,
                                                                    const WChar*         name);

    explicit                        Surface                        (const WChar*         name);

    // Use default copy behavior

    bool                            IsInitialized                  () const { return 0 != m_implP.get(); }

    const WString&               GetName                        () const { return m_name; }

    BcDTM&                         GetDTM                         () const;

    void                            Close                          ();

    void                            Assign                         (BcDTMR   dtmP);

    bool                            HasTIN                         () const;

    PointHandler&                   GetPointHandler                ();
    LinearHandler&                  GetLinearHandler               ();
    TINAsLinearHandler&             GetTINLinearHandler            ();
    };

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                 Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Surface::Impl : public Bentley::RefCountedBase
    {
  public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }


    CivilDTMWrapper             m_dtm;
    PointHandler                m_pointHandler;
    LinearHandler               m_linearHandler;
    TINAsLinearHandler          m_tinLinearHandler;

    explicit                    Impl                           (BcDTMR dtm)
        :   m_dtm(dtm),
            m_pointHandler(m_dtm.Get(), CivilImportedTypes::GetInstance().points),
            m_linearHandler(m_dtm.Get(), CivilImportedTypes::GetInstance().linears),
            m_tinLinearHandler(m_dtm.Get(), CivilImportedTypes::GetInstance().tinLinears)
        {
        }
    };


Surface::Surface(BcDTMR       dtm,
                        const WChar*             name)
    :   m_implP(new Impl(dtm)),
        m_name(name)
    {
    }

Surface::Surface (const WChar* name)
    :   m_name(name)
    {
    }

BcDTM& Surface::GetDTM  () const 
    {
    assert(0 != m_implP.get());
    return m_implP->m_dtm.Get();
    }

void Surface::Close ()
    {
    m_implP = 0;
    }

void Surface::Assign (BcDTMR  dtm)
    {
    assert(0 == m_implP.get());
    m_implP = new Impl(dtm);
    }

bool Surface::HasTIN () const { return m_implP->m_dtm.HasTIN(); }

PointHandler& Surface::GetPointHandler () { return m_implP->m_pointHandler; }
LinearHandler& Surface::GetLinearHandler () { return m_implP->m_linearHandler; }
TINAsLinearHandler& Surface::GetTINLinearHandler () { return m_implP->m_tinLinearHandler; }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                 Jean-Francois.Cote   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LandXMLSource : public SourceMixinBase<LandXMLSource>
    {
private:
    typedef vector<Surface>             SurfaceList;
    friend class LandXMLFileCreator;

    Bentley::TerrainModel::LandXMLImporterPtr m_landImporterP;
    SurfaceList                             m_surfaces; 

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                Jean-Francois.Cote   02/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit        LandXMLSource  (Bentley::TerrainModel::LandXMLImporter& landImporter) 
        :   m_landImporterP(&landImporter)
        { 
        TerrainInfoList const& landSurfaceList = m_landImporterP->GetTerrains();

        for(TerrainInfoList::const_iterator itr(landSurfaceList.begin()); itr != landSurfaceList.end(); ++itr)
            {
            m_surfaces.push_back(Surface(itr->GetName().c_str()));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                Jean-Francois.Cote   02/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                        _Close                                         () override
        {
        m_landImporterP = NULL;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description   
    * @bsimethod                                                Jean-Francois.Cote   02/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor           _CreateDescriptor                              () const override       
        {
        ContentDescriptor contentDesc(L"");

        struct SurfaceToLayerDesc
            {
            LayerDescriptor operator() (const Surface& surface) const
                {
                return LayerDescriptor(surface.GetName().c_str(),
                                       DataTypeSet
                                            (
                                            LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create(), 
                                            PointType3d64fCreator().Create(),
                                            TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create()
                                            ), 
                                        GetGCSFactory().Create(Unit::GetMeter()), 
                                        0,
                                        ScalableMeshData::GetNull());
                }
            };

        std::transform(m_surfaces.begin(), m_surfaces.end(), back_inserter(contentDesc), SurfaceToLayerDesc());

        return contentDesc;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*             _GetType                                       () const override
        {
        return L"Land XML";
        }


public:
    uint32_t                                GetSurfaceCount                                () const 
        {
        return (uint32_t)m_surfaces.size();
        }

    Surface*                            FindSurfaceFor                                    (UInt                    layer)
        {
        if (m_surfaces.size() < layer)
            return 0;
              
        if (m_surfaces[layer].IsInitialized())
            return &m_surfaces[layer];

        // Load stuff      
        ImportedTerrain importedSurface = m_landImporterP->ImportTerrain(m_surfaces[layer].GetName().GetWCharCP());
        if(importedSurface.GetTerrain() == NULL)
            return NULL;

        m_surfaces[layer].Assign(*importedSurface.GetTerrain());
        return &m_surfaces[layer];
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description      
* @bsiclass                                                 Jean-Francois.Cote   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LandXMLFileCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter             _GetExtensions                                 () const override
        {
        return L"*.xml";
        }    

    /*---------------------------------------------------------------------------------**//**
    * @description      
    * @bsimethod                                                 Jean-Francois.Cote   02/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                        _Supports                                      (const LocalFileSourceRef&       sourceRef) const override
        {
        if (!DefaultSupports(sourceRef))
            return false;

        // Look for the element by the name LandXML and at the same moment, check if this file contains at least one surface
        Bentley::TerrainModel::LandXMLImporterPtr landImporterP(Bentley::TerrainModel::LandXMLImporter::Create(sourceRef.GetPath().c_str()));

        return landImporterP->GetTerrains().size() > 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description     
    * @bsimethod                                                Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual SourceBase*                _Create                                         (const LocalFileSourceRef&       sourceRef,
                                                                                        Log&                     log) const override
        {  
        Bentley::TerrainModel::LandXMLImporterPtr landImporterP(Bentley::TerrainModel::LandXMLImporter::Create(sourceRef.GetPath().c_str()));
        return new LandXMLSource(*landImporterP);
        }
    };

const SourceRegistry::AutoRegister<LandXMLFileCreator> s_RegisterLandXMLFile;


/*---------------------------------------------------------------------------------**//**
* @description      
* @bsiclass                                                 Jean-Francois.Cote   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LandXMLPointExtractorCreator : public InputExtractorCreatorMixinBase<LandXMLSource>
    {
    virtual bool                                _Supports                          (const DataType&         pi_rType) const override
        {
        return pi_rType.GetFamily() == PointTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&         selection) const override
        {
        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP || !surfaceP->GetPointHandler().ComputeCounts())
            return RawCapacities(0);

        return RawCapacities (surfaceP->GetPointHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        using namespace Bentley::ScalableMesh::Plugin::V0;

        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP || !surfaceP->GetPointHandler().ComputeCounts())
            return 0;

        // Borrow extractor from the civil dtm import module as it does exactly what we need
        return CreateCivilDTMPointExtractor(surfaceP->GetPointHandler());
        }
    };

const ExtractorRegistry::AutoRegister<LandXMLPointExtractorCreator> s_RegisterLandXMLPointImporter;


/*---------------------------------------------------------------------------------**//**
* @description      
* @bsiclass                                                 Jean-Francois.Cote   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LandXMLLinearExtractorCreator : public InputExtractorCreatorMixinBase<LandXMLSource>
    {
    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == LinearTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP || !surfaceP->GetLinearHandler().ComputeCounts())
            return RawCapacities(0, 0);

        return RawCapacities (surfaceP->GetLinearHandler().GetMaxLinearCount()*sizeof(IDTMFile::FeatureHeader),
                              surfaceP->GetLinearHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        using namespace Bentley::ScalableMesh::Plugin::V0;

        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP || !surfaceP->GetLinearHandler().ComputeCounts())
            return 0;

        // Borrow extractor from the civil dtm import module as it does exactly what we need
        return CreateCivilDTMLinearExtractor(surfaceP->GetLinearHandler());
        }
    };

const ExtractorRegistry::AutoRegister<LandXMLLinearExtractorCreator> s_RegisterLandXMLFeatureImporter;




/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class EmptyLandXMLLinearExtractor : public InputExtractorBase
    {
private:
    enum 
        {
        DG_Header,
        DG_XYZ,
        };

    PODPacketProxy<byte>            m_headerPacket;
    PODPacketProxy<byte>            m_pointPacket;

    virtual void                    _Assign                            (PacketGroup&     dst) override
        {
        m_headerPacket.AssignTo(dst[DG_Header]);
        m_pointPacket.AssignTo(dst[DG_XYZ]);
        }

    virtual void                    _Read                              () override
        {
        m_headerPacket.SetSize(0);
        m_pointPacket.SetSize(0);
        }

    virtual bool                    _Next                              () override
        {
        return false;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description      
* @bsiclass                                                 Jean-Francois.Cote   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LandXMLTINExtractorCreator : public InputExtractorCreatorMixinBase<LandXMLSource>
    {
    virtual bool                                _Supports                          (const DataType&                 pi_rType) const override
        {
        return pi_rType.GetFamily() == TINTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP || !surfaceP->HasTIN() || !surfaceP->GetTINLinearHandler().ComputeCounts())
            return RawCapacities(0, 0);

        return RawCapacities (surfaceP->GetTINLinearHandler().GetMaxLinearCount()*sizeof(IDTMFile::FeatureHeader),
                              surfaceP->GetTINLinearHandler().GetMaxPointCount()*sizeof(DPoint3d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (LandXMLSource&                  sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        using namespace Bentley::ScalableMesh::Plugin::V0;

        Surface* surfaceP = sourceBase.FindSurfaceFor(selection.GetLayer());
        if (0 == surfaceP)
            return 0;

        if (!surfaceP->HasTIN())
            return new EmptyLandXMLLinearExtractor;

        if (!surfaceP->GetTINLinearHandler().ComputeCounts())
            return 0;

        // Borrow extractor from the civil dtm import module as it does exactly what we need
        return CreateCivilDTMLinearExtractor(surfaceP->GetTINLinearHandler());
        }
    };

const ExtractorRegistry::AutoRegister<LandXMLTINExtractorCreator> s_RegisterLandXMLTINLinearImporter;

} //END UNAMED NAMESPACE
