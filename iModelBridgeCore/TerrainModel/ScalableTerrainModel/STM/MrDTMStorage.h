/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMStorage.h $
|    $RCSfile: MrDTMStorage.h,v $
|   $Revision: 1.16 $
|       $Date: 2011/10/26 17:55:17 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../Import/Sink.h"
#include <ScalableTerrainModel/GeoCoords/Reprojection.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <ImagePP/all/h/HGFPointIndex.h>
#include <ScalableTerrainModel/Memory/PacketAccess.h>
#include "MrDTM.h"
#include "Plugins/MrDTMIDTMFileTraits.h"

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IStorage : public Import::Sink
    {

    };


template <typename PtType>
class MrDTMStorage;

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class MrDTMPointStorageEditor : public Import::BackInserter
    {

    friend class                            MrDTMStorage<PtType>;

    Memory::ConstPacketProxy<PtType>        m_pointPacket;

    typedef HGFPointIndex<PtType, YProtPtExtentType>
                                            PointIndexType;    
    PointIndexType&                         m_rIndex;

    explicit                                MrDTMPointStorageEditor    (PointIndexType&                 pi_rIndex)
        :   m_rIndex(pi_rIndex) {}

    virtual void                            _Assign                    (const Memory::PacketGroup&      pi_rSrc) override
        {
        HPRECONDITION(1 <= pi_rSrc.GetSize());
        m_pointPacket.AssignTo(pi_rSrc[0]);
        }

    virtual void                            _Write                     () override
        {
        const bool Success = m_rIndex.AddArray(m_pointPacket.Get(), m_pointPacket.GetSize());

        // TDORAY: Throw on failures?
        assert(Success);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class MrDTMLinearStorageEditor : public Import::BackInserter
    {

    friend class                            MrDTMStorage<PtType>;

    Memory::ConstPacketProxy<PtType>      m_pointPacket;
    Memory::ConstPacketProxy<IDTMFile::FeatureHeader> 
                                            m_headerPacket;

    typedef IDTMFeatureArray<PtType> ArrayType;
    ArrayType                               m_Features;

    typedef HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, HGF3DPoint, YProtFeatureExtentType>
                                            IndexType;        

    IndexType&                              m_rIndex;


    explicit                                MrDTMLinearStorageEditor  
                                                                       (IndexType&                  pi_rFeatureIndex)
        :   m_rIndex(pi_rFeatureIndex) {}


    virtual void                            _Assign                    (const Memory::PacketGroup&  pi_rSrc) override
        {
        m_headerPacket.AssignTo(pi_rSrc[0]);
        m_pointPacket.AssignTo(pi_rSrc[1]);
        }

    virtual void                            _Write                     () override
        {
        m_Features.EditHeaders().Wrap(m_headerPacket.Get(), m_headerPacket.GetSize());
        m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());

        //TDORAY: Insert code here that feed the feature index.
        for (ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End() ; myFeature++)
            {
            HFCPtr<HVEDTMLinearFeature> newFeature = new HVEDTMLinearFeature(myFeature->GetType(), myFeature->GetSize());
            newFeature->SetAutoToleranceActive(false);
            for (ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End() ; myPoint++)
                {
                newFeature->AppendPoint(PointOp<HGF3DPoint>::Create(PointOp<PtType>::GetX(*myPoint),
                                                                    PointOp<PtType>::GetY(*myPoint),
                                                                    PointOp<PtType>::GetZ(*myPoint)));
                }

            // Append to index
            m_rIndex.Add (newFeature);
            }

        // TDORAY: Throw on failures?
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MrDTMTINAsLinearStorageEditor : public Import::BackInserter
    {

    friend class                            MrDTMStorage<IDTMFile::Point3d64f>;

    typedef IDTMFile::Point3d64f            PointType;

    Memory::ConstPacketProxy<PointType>     m_pointPacket;
    Memory::ConstPacketProxy<IDTMFile::FeatureHeader> 
                                            m_headerPacket;

    typedef IDTMFeatureArray<PointType>     ArrayType;
    ArrayType                               m_Features;

    typedef HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, HGF3DPoint, YProtFeatureExtentType>
                                            IndexType;        

    IndexType&                              m_rIndex;


    explicit                                MrDTMTINAsLinearStorageEditor  
                                                                       (IndexType&                  featureIndex)
        :   m_rIndex(featureIndex) {}


    IDTMFile::FeatureType                   ConvertLinearType          (IDTMFile::FeatureType       sourceType)
        {
        switch(sourceType)
            {
            case DTMFeatureType::TinLine: 
                return (IDTMFile::FeatureType)DTMFeatureType::Breakline;
            default:
                assert(!"Unsupported");
                return sourceType;
            }
        }

    virtual void                            _Assign                    (const Memory::PacketGroup&  src) override
        {
        m_headerPacket.AssignTo(src[0]);
        m_pointPacket.AssignTo(src[1]);
        }

    virtual void                            _Write                     () override
        {
        m_Features.EditHeaders().Wrap(m_headerPacket.Get(), m_headerPacket.GetSize());
        m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());

        

        //TDORAY: Insert code here that feed the feature index.
        for (ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End() ; myFeature++)
            {
            HFCPtr<HVEDTMLinearFeature> newFeature = new HVEDTMLinearFeature(ConvertLinearType(myFeature->GetType()), myFeature->GetSize());
            newFeature->SetAutoToleranceActive(false);
            for (ArrayType::value_type::const_iterator myPoint = myFeature->Begin(); myPoint != myFeature->End() ; myPoint++)
                {
                newFeature->AppendPoint(PointOp<HGF3DPoint>::Create(PointOp<PointType>::GetX(*myPoint),
                                                                    PointOp<PointType>::GetY(*myPoint),
                                                                    PointOp<PointType>::GetZ(*myPoint)));
                }

            // Append to index
            m_rIndex.Add (newFeature);
            }

        // TDORAY: Throw on failures?
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename PtType>
class MrDTMStorage : public IStorage
    {
    typedef typename PointTypeCreatorTrait<PtType>::type 
                                            PointTypeFactory;
    typedef typename LinearTypeCreatorTrait<PtType>::type 
                                            LinearTypeFactory;
    typedef typename TINAsLinearTypeCreatorTrait<IDTMFile::Point3d64f>::type 
                                            TINTypeFactory;
    typedef typename MeshAsLinearTypeCreatorTrait<IDTMFile::Point3d64f>::type 
                                            MeshTypeFactory;

    typedef HGFPointIndex<PtType, YProtPtExtentType>
                                            PointIndexType;    
    typedef HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, HGF3DPoint, YProtFeatureExtentType>
                                            FeatureIndexType;     

    PointIndexType*                         m_pPointIndex;
    FeatureIndexType*                       m_pFeatureIndex;

    GeoCoords::GCS                          m_geoCoordSys;

    virtual Import::ContentDescriptor       _CreateDescriptor             () const override
        {
        using Import::ContentDescriptor;
        using Import::LayerDescriptor;

        // TDORAY: Return more accurate data..
        return ContentDescriptor
            (
            L"STM",
            LayerDescriptor(L"",
                            Import::DataTypeSet
                                (
                                PointTypeFactory().Create(), 
                                LinearTypeFactory().Create()
                                ),
                            m_geoCoordSys,
                            0)
            );
        }

    virtual Import::BackInserter*          _CreateBackInserterFor      (UInt                        layerID,
                                                                        const Import::DataType&     type,
                                                                        Import::Log&         log) const override
        {
        assert(0 == layerID);

        //TDORAY: Ensure that type can be found in layer before returning

        if (PointTypeFactory().Create() == type)
            return (0 != m_pPointIndex) ? new MrDTMPointStorageEditor<PtType>(*m_pPointIndex) : 0;
        if (LinearTypeFactory().Create() == type)
            return (0 != m_pFeatureIndex) ? new MrDTMLinearStorageEditor<PtType>(*m_pFeatureIndex) : 0;

        return 0;
        }

public:
    explicit                                MrDTMStorage               (PointIndexType&                 pi_rPointIndex,
                                                                        FeatureIndexType&               pi_rFeatureIndex,
                                                                        const GeoCoords::GCS&           pi_rGeoCoordSys)
        :   m_pPointIndex(&pi_rPointIndex),
            m_pFeatureIndex(&pi_rFeatureIndex),
            m_geoCoordSys(pi_rGeoCoordSys)
        {
        
        }

    explicit                                MrDTMStorage               (PointIndexType&                 pi_rIndex,
                                                                        const GeoCoords::GCS&           pi_rGeoCoordSys)
        :   m_pPointIndex(&pi_rIndex),
            m_pFeatureIndex(0),
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
    Import::ConstPacketProxy<IDTMFile::FeatureHeader> m_headerPacket;

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
        m_Features.EditHeaders().Wrap(m_headerPacket.Get(), m_headerPacket.GetSize());
        m_Features.EditPoints().Wrap(m_pointPacket.Get(), m_pointPacket.GetSize());
     
        const HFCPtr<HGF2DCoordSys> coordSysP = m_resultingClipShapePtr->GetCoordSys();
        
        HFCPtr<HVEShape> shapeP(new HVEShape(coordSysP));

        const size_t maxPointQty = m_Features.GetHeaders().IsEmpty() ? 0 : (*max_element(m_Features.Begin(), m_Features.End(), s_LessThan)).GetSize(); 
        HArrayAutoPtr<double> ptsP(new double[maxPointQty * 2]);

        for(ArrayType::const_iterator myFeature = m_Features.Begin(); myFeature != m_Features.End() ; ++myFeature)
            {
            double* dimP = ptsP.get();
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
            LayerDescriptor(L"",
                            LinearFeatureTypeFactory().Create(),
                            m_targetGCS,
                            0)
            );
        }

    virtual Import::BackInserter* _CreateBackInserterFor(UInt                    layerID,
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


END_BENTLEY_MRDTM_NAMESPACE