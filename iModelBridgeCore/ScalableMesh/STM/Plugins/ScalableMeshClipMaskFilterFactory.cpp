/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshClipMaskFilterFactory.cpp $
|    $RCSfile: ScalableMeshClipMaskFilterFactory.cpp,v $
|   $Revision: 1.7 $
|       $Date: 2011/09/01 14:07:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include "ScalableMeshClipMaskFilterFactory.h"
#include <ScalableMesh/Import/Plugin/FilterV0.h>
#include "../IDTMFeatureArray.h"
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include "..\Stores\SMStoreUtils.h"
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
using namespace ISMStore;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

template <typename PointT>
struct IsPointClippedPred
    {
    const HVEClipShape& m_rShape;
    HGF2DLocation& m_rTmpPt; // NTERAY: this is bad if a threaded remove_copy_if algorithm is used
    explicit IsPointClippedPred (const HVEClipShape& pi_rShape, 
                            HGF2DLocation& po_rTmpPt) : m_rShape(pi_rShape), m_rTmpPt(po_rTmpPt) {}

    bool operator () (const PointT& rhs) const
        {
        m_rTmpPt.SetX(rhs.x);
        m_rTmpPt.SetY(rhs.y);
        return m_rShape.IsPointClipped(m_rTmpPt);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT>
class PointClipMaskFilter : public FilterBase
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    const HVEClipShape&             m_rShape;
    HGF2DLocation                   m_tempPt;
    IsPointClippedPred<PointT>           m_isClippedPred;
    

    ConstPacketProxy<PointT>        m_srcPacket;
    PODPacketProxy<PointT>          m_dstPacket;

    virtual void                    _Assign                            (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        m_srcPacket.AssignTo(pi_Src[0]);
        m_dstPacket.AssignTo(po_Dst[0]);
        }

    virtual void                    _Run                               () override
        {
        m_dstPacket.SetEnd(std::remove_copy_if(m_srcPacket.begin(), m_srcPacket.end(), 
                                               m_dstPacket.begin(), 
                                               m_isClippedPred));
        }

public:
    explicit                        PointClipMaskFilter                (const HVEClipShape&             pi_rShape)
        :   m_rShape(pi_rShape),
            m_tempPt(pi_rShape.GetCoordSys()),
            m_isClippedPred(m_rShape, m_tempPt)
            
        {
        
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT>
class PointClipMaskFilterCreator : public FilterCreatorBase
    {
    DataType                        m_type;
    HFCPtr<HVEClipShape>            m_pShape;

    virtual void                    _Bind                              (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) const override
        {
        po_Dst[0].BindUseSameAs(pi_Src[0]);
        }


    virtual FilterBase*             _Create                            (const PacketGroup&,
                                                                        PacketGroup&,
                                                                        const FilteringConfig&,
                                                                        Log&) const override
        {
        return new PointClipMaskFilter<PointT>(*m_pShape);
        }

    virtual const DataType&         _GetSourceType                     () const override
        {
        return m_type;
        }

    virtual const DataType&         _GetTargetType                     () const override
        {
        return m_type;
        }

public:
    explicit                        PointClipMaskFilterCreator         (const DataType&                 pi_rType,
                                                                        const HFCPtr<HVEClipShape>&     pi_pShape)
        :   m_type(pi_rType),
            m_pShape(pi_pShape)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename LinFeatureT>
struct IsLinearFeatureClippedPred
    {
    typedef typename LinFeatureT::value_type PointType;
    IsPointClippedPred<PointType> m_pointClippedPred;

    explicit IsLinearFeatureClippedPred    (const HVEClipShape& pi_rShape, 
                                            HGF2DLocation& po_rTmpPt) 
        : m_pointClippedPred(pi_rShape, po_rTmpPt) {}

    bool operator () (const LinFeatureT& rhs) const
        {
        // A feature is clipped when one of its point is found to be clipped
        return rhs.End() != find_if(rhs.Begin(), rhs.End(), m_pointClippedPred);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT>
class LinearFeatureClipMaskFilter : public FilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef IDTMFeatureArray<PointT, FeatureHeader>
                                    LinearFeatureArray;
    typedef typename LinearFeatureArray::value_type
                                    LinearFeatureType;

    const HVEClipShape&             m_rShape;
    HGF2DLocation                   m_tempPt;
    IsLinearFeatureClippedPred<LinearFeatureType>      
                                    m_isClippedPred;
    
    ConstPacketProxy<FeatureHeader> m_srcHeaderPacket;
    PODPacketProxy<FeatureHeader>   m_dstHeaderPacket;

    ConstPacketProxy<PointT>        m_srcPointPacket;
    PODPacketProxy<PointT>          m_dstPointPacket;


    LinearFeatureArray              m_srcLinFeatArray;
    LinearFeatureArray              m_dstLinFeatArray;

    virtual void                    _Assign                            (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        m_srcHeaderPacket.AssignTo(pi_Src[0]);
        m_dstHeaderPacket.AssignTo(po_Dst[0]);

        m_srcPointPacket.AssignTo(pi_Src[1]);
        m_dstPointPacket.AssignTo(po_Dst[1]);
        }

    virtual void                    _Run                               () override
        {
        m_srcLinFeatArray.EditHeaders().Wrap(m_srcHeaderPacket.Get(), m_srcHeaderPacket.GetSize());
        m_srcLinFeatArray.EditPoints().Wrap(m_srcPointPacket.Get(), m_srcPointPacket.GetSize());

        m_dstLinFeatArray.EditHeaders().WrapEditable(m_dstHeaderPacket.Edit(), 0, m_dstHeaderPacket.GetCapacity());
        m_dstLinFeatArray.EditPoints().WrapEditable(m_dstPointPacket.Edit(), 0, m_dstPointPacket.GetCapacity());

        std::remove_copy_if(m_srcLinFeatArray.Begin(), m_srcLinFeatArray.End(), 
                            back_inserter(m_dstLinFeatArray), 
                            m_isClippedPred);
        
        m_dstHeaderPacket.SetSize(m_dstLinFeatArray.GetHeaders().GetSize());
        m_dstPointPacket.SetSize(m_dstLinFeatArray.GetPoints().GetSize());
        }

public:
    explicit                        LinearFeatureClipMaskFilter        (const HVEClipShape&             pi_rShape)
        :   m_rShape(pi_rShape),
            m_tempPt(pi_rShape.GetCoordSys()),
            m_isClippedPred(m_rShape, m_tempPt)
            
        {
        
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT>
class LinearFeatureClipMaskFilterCreator : public FilterCreatorBase
    {
    DataType                        m_type;
    HFCPtr<HVEClipShape>            m_pShape;

    virtual void                    _Bind                              (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) const override
        {
        po_Dst[0].BindUseSameAs(pi_Src[0]);
        po_Dst[1].BindUseSameAs(pi_Src[1]);
        }

    virtual FilterBase*             _Create                            (const PacketGroup&,
                                                                        PacketGroup&,
                                                                        const FilteringConfig&,
                                                                        Log&) const override
        {
        return new LinearFeatureClipMaskFilter<PointT>(*m_pShape);
        }

    virtual const DataType&         _GetSourceType                     () const override
        {
        return m_type;
        }

    virtual const DataType&         _GetTargetType                     () const override
        {
        return m_type;
        }

public:
    explicit                        LinearFeatureClipMaskFilterCreator (const DataType&                 pi_rType,
                                                                        const HFCPtr<HVEClipShape>&     pi_pShape)
        :   m_type(pi_rType),
            m_pShape(pi_pShape)
        {
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClipMaskFilterFactory::Impl
    {
    explicit                        Impl                               (const HFCPtr<HVEClipShape>&     pi_pShape)
        :   m_pShape(pi_pShape)
        {

        }

    HFCPtr<HVEClipShape>            m_pShape;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory ClipMaskFilterFactory::CreateFrom (const HFCPtr<HVEClipShape>& pi_pShape)
    {
    auto_ptr<Impl> pImpl(new Impl(pi_pShape));
    return CreateFromBase(new ClipMaskFilterFactory(pImpl.release()));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClipMaskFilterFactory::ClipMaskFilterFactory (Impl* pi_pImpl)
    :   m_pImpl(pi_pImpl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClipMaskFilterFactory::~ClipMaskFilterFactory ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FilterCreatorBase* ClipMaskFilterFactory::_FindCreatorFor    (const DataType& pi_rSourceType,
                                                                    Log&            po_warningLog) const
    {
    if (pi_rSourceType == PointType3d64fCreator().Create())
        {
        return new PointClipMaskFilterCreator<DPoint3d>(pi_rSourceType, m_pImpl->m_pShape);
        }
    else if (pi_rSourceType == LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create())
        {
        return new LinearFeatureClipMaskFilterCreator<DPoint3d>(pi_rSourceType, m_pImpl->m_pShape);
        }
    else
        {
        assert(!"Implement better behaviour!");
        return 0;
        }
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE
