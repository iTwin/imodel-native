/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshStorage.h $
|    $RCSfile: ScalableMeshStorage.h,v $
|   $Revision: 1.16 $
|       $Date: 2011/10/26 17:55:17 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../Import/Sink.h"
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include "SMPointIndex.h"
#include "SMMeshIndex.h"
#include <ScalableMesh/Memory/PacketAccess.h>
#include "ScalableMesh.h"
#include "Plugins/ScalableMeshIDTMFileTraits.h"
#include <ScalableMesh/IScalableMeshSourceImporter.h>
#include "IDTMFeatureArray.h"

#ifdef VANCOUVER_API
typedef IDTMFile::FeatureHeader headerType;
#else
#include "Stores/SMStoreUtils.h"
typedef ISMStore::FeatureHeader headerType;
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IStorage : public Import::Sink
    {

    };

// typedef DPoint3d                          PointType;
// typedef DRange3d PointIndexExtentType;
// typedef SMMeshIndex <PointType, PointIndexExtentType> MeshIndexType;

template <typename PtType>
class ScalableMeshStorage;
template <typename PtType> class ScalableMeshNonDestructiveEditStorage;

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class ScalableMeshPointStorageEditor : public Import::BackInserter
    {

    friend class                            ScalableMeshStorage<PtType>;
    protected:
    Memory::ConstPacketProxy<PtType>        m_pointPacket;

    typedef SMPointIndex<PtType, Extent3dType>
                                            MeshIndexType;    
    MeshIndexType&                         m_rIndex;

    size_t m_totalSources;
    size_t m_totalBytesToImport;
    size_t m_nSourcesImported;


    explicit                                ScalableMeshPointStorageEditor    (MeshIndexType&                 pi_rIndex, size_t nSources)
        : m_rIndex(pi_rIndex), m_totalSources(nSources), m_totalBytesToImport(0), m_nSourcesImported(0) {}

    virtual void                            _Assign                    (const Memory::PacketGroup&      pi_rSrc) override
        {
        HPRECONDITION(1 <= pi_rSrc.GetSize());
        m_pointPacket.AssignTo(pi_rSrc[0]);
        }

    virtual void                            _NotifyImportProgress(size_t currentlyImportedBytes) 
        {
        if (m_rIndex.m_progress != nullptr && m_totalBytesToImport > 0 && m_totalSources> 0)
            m_rIndex.m_progress->Progress() = (float)(m_nSourcesImported + (float)currentlyImportedBytes / m_totalBytesToImport) / m_totalSources;
        };

    virtual void                            _Write                     () override
        {

        const bool Success = m_rIndex.AddArray(m_pointPacket.Get(), m_pointPacket.GetSize(), m_is3dData, m_isGridData);

        // TDORAY: Throw on failures?
        assert(Success);
        }

    virtual bool                           _IsAcceptingData() { return m_rIndex.m_progress == nullptr || !m_rIndex.m_progress->IsCanceled(); }

    virtual void                            _NotifyImportSourceStarted(size_t totalBytes) 
        {
        m_totalBytesToImport = totalBytes;
        };

    virtual void                            _NotifySourceImported() override
        {
        m_nSourcesImported++;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Elenie.Godzaridis   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class ScalableMeshPointNonDestructiveStorageEditor : public ScalableMeshPointStorageEditor<PtType>
    {

    friend class                            ScalableMeshNonDestructiveEditStorage<PtType>;
    typedef ScalableMeshPointStorageEditor<PtType> super_class;

    explicit                                ScalableMeshPointNonDestructiveStorageEditor(typename super_class::MeshIndexType&                 pi_rIndex)
        : super_class(pi_rIndex, 0)
        {}

    virtual void                            _Write                     () override
        {
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Mathieu.St-Pierre   03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PtType>
class GenericStorage;

template<typename PtType = DPoint3d>
class GenericPointStorageEditor : public Import::BackInserter
    {

    friend class                            GenericStorage<PtType>;

    Memory::ConstPacketProxy<PtType>        m_pointPacket;
    
    WritePointsCallbackFP                   m_writePointsCallback;

    explicit                                GenericPointStorageEditor    (WritePointsCallbackFP writePointsCallback)
        :   m_writePointsCallback(writePointsCallback) {}

    virtual void                            _Assign                    (const Memory::PacketGroup&      pi_rSrc) override
        {
        HPRECONDITION(1 <= pi_rSrc.GetSize());
        m_pointPacket.AssignTo(pi_rSrc[0]);
        }

    virtual void                            _Write                     () override
        {               
        const bool success = (*m_writePointsCallback)(m_pointPacket.Get(), m_pointPacket.GetSize(), m_is3dData);

        assert(success);        
        }
    };



/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Mathieu.St-Pierre   04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType = DPoint3d>
class GenericLinearStorageEditor : public Import::BackInserter
    {

    friend class                            GenericStorage<PtType>;

    Memory::ConstPacketProxy<PtType>      m_pointPacket;
    Memory::ConstPacketProxy<ISMStore::FeatureHeader> 
                                          m_headerPacket;

    typedef IDTMFeatureArray<PtType> ArrayType;
    ArrayType                              m_Features;
        
    WriteFeatureCallbackFP                 m_featureCallback;


    explicit                                GenericLinearStorageEditor(WriteFeatureCallbackFP pi_rFeatureCallback)
        :   m_featureCallback(pi_rFeatureCallback) {}


    virtual void                            _Assign                    (const Memory::PacketGroup&  pi_rSrc) override
        {
        m_headerPacket.AssignTo(pi_rSrc[0]);
        m_pointPacket.AssignTo(pi_rSrc[1]);
        }

    virtual void                            _Write                     () override
        {
        m_Features.EditHeaders().Wrap((const headerType *)m_headerPacket.Get(), m_headerPacket.GetSize());
        m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());
        
        for (ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End() ; myFeature++)
            {            
            const bool success = (*m_featureCallback)(myFeature->Begin(), myFeature->GetSize(), (DTMFeatureType)myFeature->GetType(), m_is3dData);

            assert(success);                       
            }
        }
    };


/*---------------------------------------------------------------------------------**//**
 * @description
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
-+---------------+---------------+---------------+---------------+---------------+------*/
 template<typename PtType>
 class ScalableMeshLinearStorageEditor : public Import::BackInserter

     {
     friend class                            ScalableMeshStorage < PtType > ;

     protected:
     Memory::ConstPacketProxy<PtType>      m_pointPacket;
     Memory::ConstPacketProxy < ISMStore::FeatureHeader >
         m_headerPacket;

     typedef IDTMFeatureArray<PtType> ArrayType;
     ArrayType                               m_Features;


     typedef SMMeshIndex<PtType, Extent3dType>
         MeshIndexType;

     MeshIndexType&                              m_rIndex;
     size_t     m_importedFeatures;
     size_t m_totalSources;

     explicit                                ScalableMeshLinearStorageEditor
         (MeshIndexType&                  pi_rFeatureIndex, size_t nSources)
         : m_rIndex(pi_rFeatureIndex), m_importedFeatures(0), m_totalSources(nSources)
         {}

     virtual void                            _Assign(const Memory::PacketGroup&  pi_rSrc) override
         {
         m_headerPacket.AssignTo(pi_rSrc[0]);
         m_pointPacket.AssignTo(pi_rSrc[1]);
         }

     virtual void                            _Write() override
         {
         m_Features.EditHeaders().Wrap((const headerType *)m_headerPacket.Get(), m_headerPacket.GetSize());
         m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());

         for (ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End(); myFeature++)
             {
             bvector<DPoint3d> newFeature;
             DRange3d featureExtent;
             for (ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End(); myPoint++)
                 {
                 newFeature.push_back(DPoint3d::From(PointOp<PtType>::GetX(*myPoint),
                     PointOp<PtType>::GetY(*myPoint),
                     PointOp<PtType>::GetZ(*myPoint)));
                 if (newFeature.size() == 1) featureExtent.InitFrom(newFeature.back());
                 else featureExtent.Extend(newFeature.back());
                 }

             // Append to index
			 ISMStore::FeatureType type = myFeature->GetType();
			 if ((DTMFeatureType)type == DTMFeatureType::TinHull)
				 type = (ISMStore::FeatureType)DTMFeatureType::Hull;
             m_rIndex.AddFeatureDefinition(myFeature->GetType(),newFeature, featureExtent);
             ++m_importedFeatures;
             }

         // TDORAY: Throw on failures?
         }
     virtual bool                           _IsAcceptingData() { return m_rIndex.m_progress == nullptr || !m_rIndex.m_progress->IsCanceled(); }
     };

 /*---------------------------------------------------------------------------------**//**
 * @description
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
-+---------------+---------------+---------------+---------------+---------------+------*/
 template<typename PtType>
 class ScalableMeshMeshStorageEditor : public Import::BackInserter

     {
     friend class                            ScalableMeshStorage < PtType > ;
     protected:
     Memory::ConstPacketProxy<PtType>      m_pointPacket;
     Memory::ConstPacketProxy <int32_t>
         m_indexPacket;
     Memory::ConstPacketProxy <uint8_t>
         m_metadataPacket;
     Memory::ConstPacketProxy <uint8_t>
         m_texPacket;
     Memory::ConstPacketProxy <DPoint2d>
         m_uvPacket;

     size_t m_totalSources;

     typedef SMMeshIndex<PtType, Extent3dType>
         MeshIndexType;

     MeshIndexType&                              m_rIndex;

     explicit                                ScalableMeshMeshStorageEditor
         (MeshIndexType&                  pi_rFeatureIndex,
         size_t nSources)
         : m_rIndex(pi_rFeatureIndex), m_totalSources(nSources)
         {}

     virtual void                            _Assign(const Memory::PacketGroup&  pi_rSrc) override
         {
         m_pointPacket.AssignTo(pi_rSrc[0]);
         m_indexPacket.AssignTo(pi_rSrc[1]);
         m_metadataPacket.AssignTo(pi_rSrc[2]);
         m_texPacket.AssignTo(pi_rSrc[3]);
         m_uvPacket.AssignTo(pi_rSrc[4]);
         }

     virtual void                            _Write() override
         {
         if ((int)m_pointPacket.GetSize() == 0) return;
//#ifdef WIP_MESH_IMPORT
         Utf8String str;
         str.Sprintf("%s", m_metadataPacket.Get());
         
         DRange3d meshExtent = DRange3d::From(m_pointPacket.Get(), (int)m_pointPacket.GetSize());

         size_t remainingTexBytes = 0;
         bvector<uint8_t> tex;
         for(size_t i =0; i < m_texPacket.GetSize();)
             {
             if (remainingTexBytes == 0)
                 {
                 memcpy(&remainingTexBytes,m_texPacket.Get()+i, sizeof(size_t));
                 i+=sizeof(size_t);
                 if(!tex.empty())
                     {
                     int width, height, nChannels;
                     memcpy(&width, &tex[0],sizeof(int));
                     memcpy(&height, &tex[0]+sizeof(int),sizeof(int));
                     memcpy(&nChannels, &tex[0]+2*sizeof(int),sizeof(int));
                     m_rIndex.AddTexture(width, height, nChannels, &tex[0]+3*sizeof(int), tex.size()-3*sizeof(int));
                     }
                 tex.clear();
                 }
             else{
                 tex.push_back(m_texPacket.Get()[i]);
                 remainingTexBytes--;
                 i++;
                 }
             }
         if(!tex.empty())
             {
             int width, height, nChannels;
             memcpy(&width, &tex[0], sizeof(int));
             memcpy(&height, &tex[0] + sizeof(int), sizeof(int));
             memcpy(&nChannels, &tex[0] + 2 * sizeof(int), sizeof(int));
             m_rIndex.AddTexture(width, height, nChannels, &tex[0] + 3 * sizeof(int), tex.size() - 3 * sizeof(int));
             }
             // Append to index
         m_rIndex.AddMeshDefinition(m_pointPacket.Get(), m_pointPacket.GetSize(), m_indexPacket.Get(), m_indexPacket.GetSize(), meshExtent, str.c_str(), 0,0, m_uvPacket.Get(), m_is3dData);
//#endif
         // TDORAY: Throw on failures?
         }
     virtual bool                           _IsAcceptingData() { return m_rIndex.m_progress == nullptr || !m_rIndex.m_progress->IsCanceled(); }
     };


 /*---------------------------------------------------------------------------------**//**
 * @description
*
* @bsiclass                                                  Elenie.Godzaridis   10/2015
-+---------------+---------------+---------------+---------------+---------------+------*/
 template<typename PtType>
 class ScalableMeshNonDestructiveLinearStorageEditor : public ScalableMeshLinearStorageEditor<PtType>

     {
     friend class                            ScalableMeshNonDestructiveEditStorage < PtType > ;
     typedef ScalableMeshLinearStorageEditor<PtType> super_class;

     explicit                                ScalableMeshNonDestructiveLinearStorageEditor
         (typename super_class::MeshIndexType&                  pi_rFeatureIndex)
         : super_class(pi_rFeatureIndex,0)
         {}


     virtual void                            _Write() override
         {
         this->m_Features.EditHeaders().Wrap((const headerType *) this->m_headerPacket.Get(), this->m_headerPacket.GetSize());
         this->m_Features.EditPoints().Wrap(this->m_pointPacket.Get(), this->m_pointPacket.GetSize());

         for (super_class::ArrayType::const_iterator myFeature = this->m_Features.Begin(); myFeature != this->m_Features.End(); myFeature++)
             {
             bvector<DPoint3d> newFeature;
             DRange3d featureExtent;
             for (super_class::ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End(); myPoint++)
                 {
                 newFeature.push_back(DPoint3d::From(PointOp<PtType>::GetX(*myPoint),
                     PointOp<PtType>::GetY(*myPoint),
                     PointOp<PtType>::GetZ(*myPoint)));
                 if (newFeature.size() == 1) featureExtent.InitFrom(newFeature.back());
                 else featureExtent.Extend(newFeature.back());
                 }

             // Append to index
             this->m_rIndex.AddClipDefinition(newFeature, featureExtent);
             }

         // TDORAY: Throw on failures?
         }

     virtual void                            _NotifySourceImported() override
         {
         this->m_rIndex.RefreshMergedClips();
         }
     };


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class ScalableMeshStorage : public IStorage
    {
    friend class ScalableMeshNonDestructiveEditStorage<PtType>;

    protected:
    typedef typename PointTypeCreatorTrait<PtType>::type 
                                            PointTypeFactory;
    typedef typename LinearTypeCreatorTrait<PtType>::type 
                                            LinearTypeFactory;
    typedef typename TINAsLinearTypeCreatorTrait<DPoint3d>::type
                                            TINTypeFactory;
    typedef typename MeshAsLinearTypeCreatorTrait<DPoint3d>::type
                                            MeshAsLinearTypeFactory;
    typedef typename MeshTypeCreatorTrait<DPoint3d>::type
        MeshTypeFactory;

    typedef SMMeshIndex<PtType, Extent3dType>
                                            MeshIndexType;    

    MeshIndexType*                         m_pPointIndex;

    GeoCoords::GCS                          m_geoCoordSys;

    virtual Import::ContentDescriptor       _CreateDescriptor             () const override
        {
        using Import::ContentDescriptor;
        using Import::LayerDescriptor;

        // TDORAY: Return more accurate data..
        return ContentDescriptor
            (
            L"STM",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            Import::DataTypeSet
                                (
                                PointTypeFactory().Create(), 
                                LinearTypeFactory().Create(),
                                MeshTypeFactory().Create()
                                ),
                            m_geoCoordSys,
                            0,
                            ScalableMeshData::GetNull())
            );
        }

    virtual Import::BackInserter*          _CreateBackInserterFor      (uint32_t                        layerID,
                                                                        const Import::DataType&     type,
                                                                        Import::Log&         log) const override
        {
        assert(0 == layerID);

        //TDORAY: Ensure that type can be found in layer before returning

        if (PointTypeFactory().Create() == type)
            return (0 != m_pPointIndex) ? new ScalableMeshPointStorageEditor<PtType>(*m_pPointIndex, GetTotalNumberOfExpectedSources()) : 0;
        if (LinearTypeFactory().Create() == type)
            return (0 != m_pPointIndex) ? new ScalableMeshLinearStorageEditor<PtType>(*m_pPointIndex, GetTotalNumberOfExpectedSources()) : 0;
        if (MeshTypeFactory().Create() == type)
            return (0 != m_pPointIndex) ? new ScalableMeshMeshStorageEditor<PtType>(*m_pPointIndex, GetTotalNumberOfExpectedSources()) : 0;
        return 0;
        }

public:

    explicit                                ScalableMeshStorage               (MeshIndexType&                 pi_rIndex,
                                                                        const GeoCoords::GCS&           pi_rGeoCoordSys)
        :   m_pPointIndex(&pi_rIndex),
            m_geoCoordSys(pi_rGeoCoordSys)
            
        {
        
        }
};


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class ScalableMeshNonDestructiveEditStorage : public ScalableMeshStorage<PtType>
    {
    typedef ScalableMeshStorage<PtType> super_class;

    virtual Import::BackInserter*          _CreateBackInserterFor      (uint32_t                        layerID,
                                                                        const Import::DataType&     type,
                                                                        Import::Log&         log) const override
        {
        assert(0 == layerID);

        //TDORAY: Ensure that type can be found in layer before returning

        if (super_class::PointTypeFactory().Create() == type)
            return (0 != this->m_pPointIndex) ? new ScalableMeshPointNonDestructiveStorageEditor<PtType>(*this->m_pPointIndex) : 0;
        if (super_class::LinearTypeFactory().Create() == type)
            return (0 != this->m_pPointIndex) ? new ScalableMeshNonDestructiveLinearStorageEditor<PtType>(*this->m_pPointIndex) : 0;

        return 0;
        }

public:

    explicit                                ScalableMeshNonDestructiveEditStorage(typename super_class::MeshIndexType&                 pi_rIndex,
                                                                        const GeoCoords::GCS&           pi_rGeoCoordSys)
                                                                        : super_class(pi_rIndex, pi_rGeoCoordSys)
        {
        
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Mathieu.St-Pierre   03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class GenericStorage : public IStorage
    {
    typedef typename PointTypeCreatorTrait<PtType>::type 
                                            PointTypeFactory;
    typedef typename LinearTypeCreatorTrait<PtType>::type 
                                            LinearTypeFactory;
    
    WritePointsCallbackFP                   m_writePointsCallback;
    WriteFeatureCallbackFP                  m_writeFeatureCallback;
   
    GeoCoords::GCS                          m_geoCoordSys;

    virtual Import::ContentDescriptor       _CreateDescriptor             () const override
        {
        using Import::ContentDescriptor;
        using Import::LayerDescriptor;

        // TDORAY: Return more accurate data..
        return ContentDescriptor
            (
            L"STM",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            Import::DataTypeSet
                                (
                                PointTypeFactory().Create(), 
                                LinearTypeFactory().Create()
                                ),
                            m_geoCoordSys,
                            0,
                            ScalableMeshData::GetNull())
            );
        }

    virtual Import::BackInserter*          _CreateBackInserterFor      (uint32_t                        layerID,
                                                                        const Import::DataType&     type,
                                                                        Import::Log&         log) const override
        {
        assert(0 == layerID);
        
        if (PointTypeFactory().Create() == type)
            return (0 != m_writePointsCallback) ? new GenericPointStorageEditor<PtType>(m_writePointsCallback) : 0;        
        if (LinearTypeFactory().Create() == type)
            return (0 != m_writeFeatureCallback) ? new GenericLinearStorageEditor<PtType>(m_writeFeatureCallback) : 0;            

        return 0;
        }

public:
   
    explicit                                GenericStorage               (WritePointsCallbackFP  pi_writePointsCallback,
                                                                          WriteFeatureCallbackFP pi_writeFeatureCallback,
                                                                          const GeoCoords::GCS& pi_rGeoCoordSys)
        :   m_writePointsCallback(pi_writePointsCallback),
            m_writeFeatureCallback(pi_writeFeatureCallback),           
            m_geoCoordSys(pi_rGeoCoordSys)
        {        
        }
    };



class ClipShapeStorage;
typedef RefCountedPtr<ClipShapeStorage> ClipShapeStoragePtr;      

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Jean-Francois.Cote   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ClipShapeLinearFeatureStorageEditor : public Import::BackInserter
    {
    friend class ClipShapeStorage;

    Import::ConstPacketProxy<DPoint3d>                m_pointPacket;
    Import::ConstPacketProxy<ISMStore::FeatureHeader> m_headerPacket;

    typedef IDTMFeatureArray<DPoint3d>  ArrayType;

    struct
        {
        bool operator() (const ArrayType::value_type& lhs, const ArrayType::value_type& rhs) const { return lhs.GetSize() < rhs.GetSize(); }
        } s_LessThan;

    ArrayType                           m_Features;
    HFCPtr<HVEClipShape>                m_resultingClipShapePtr;
    bool                                m_isMaskSource;


    explicit ClipShapeLinearFeatureStorageEditor(const HFCPtr<HVEClipShape>&         resultingClipShapePtr, 
                                                 bool                                isMask,
                                                 Import::Log&                 log)
        : m_resultingClipShapePtr(resultingClipShapePtr),
          m_isMaskSource(isMask)
        {
        
        }

    virtual void _Assign(const Import::PacketGroup&  pi_rSrc) override
        {
        m_headerPacket.AssignTo(pi_rSrc[0]);
        m_pointPacket.AssignTo(pi_rSrc[1]);
        }

    virtual void _Write() override
        {
        m_resultingClipShapePtr->AddClip(CreateClipShape(), m_isMaskSource);
        }
  
    HFCPtr<HVEShape> CreateClipShape()
        {
        m_Features.EditHeaders().Wrap((const headerType *)m_headerPacket.Get(), m_headerPacket.GetSize());
        m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());
     
        const HFCPtr<HGF2DCoordSys> coordSysP = m_resultingClipShapePtr->GetCoordSys();
        
        HFCPtr<HVEShape> shapeP(new HVEShape(coordSysP));

        const size_t maxPointQty = m_Features.GetHeaders().IsEmpty() ? 0 : (*max_element(m_Features.Begin(), m_Features.End(), s_LessThan)).GetSize(); 
        HArrayAutoPtr<double> ptsP(new double[maxPointQty * 2]);

        for(ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End() ; ++myFeature)
            {
            double* dimP = ptsP.get();
            // WIP_NEEDS_WORK_2017
            for (ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End() ; ++myPoint)
                {               
                *dimP++ = (*myPoint).x;
                *dimP++ = (*myPoint).y;    
                }  
            HVE2DPolygonOfSegments polygon(dimP - ptsP.get(), ptsP, coordSysP);

            shapeP->Unify(HVEShape(polygon));
            }                                                                                                                 

        return shapeP;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Jean-Francois.Cote   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ClipShapeStorage : public IStorage
    {
    typedef LinearTypeCreatorTrait<DPoint3d>::type LinearFeatureTypeFactory;  

    HFCPtr<HVEClipShape>                    m_resultingClipShapePtr;
    GeoCoords::GCS                          m_targetGCS;
    bool                                    m_isMaskSource;

    
    virtual Import::ContentDescriptor _CreateDescriptor() const override
        {
        using Import::ContentDescriptor;
        using Import::LayerDescriptor;

        // TDORAY: Return more accurate data..
        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
                            LinearFeatureTypeFactory().Create(),
                            m_targetGCS,
                            0,
                            Import::ScalableMeshData::GetNull())
            );
        }

    virtual Import::BackInserter* _CreateBackInserterFor(uint32_t                    layerID,
                                                         const Import::DataType& dataType,
                                                         Import::Log&     log) const override
        {
        assert(0 == layerID);

        //TDORAY: Ensure that type can be found in layer before returning

        if(LinearFeatureTypeFactory().Create() == dataType)
            return new ClipShapeLinearFeatureStorageEditor(m_resultingClipShapePtr, m_isMaskSource, log);

        return 0;
        }     

public:
    explicit ClipShapeStorage(const HFCPtr<HVEClipShape>& totalClipShapePtr, const GeoCoords::GCS& targetGCS)
        : m_resultingClipShapePtr(totalClipShapePtr),
          m_targetGCS(targetGCS),
          m_isMaskSource(false)
        {
        
        }

    HFCPtr<HVEClipShape> GetResultingClipShape()
        {
        return m_resultingClipShapePtr;
        }

    void SetIsMaskSource(bool isMask)
        {
        m_isMaskSource = isMask; 
        }
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE