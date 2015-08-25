/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshTypeConversionFilterPlugins.cpp $
|    $RCSfile: ScalableMeshTypeConversionFilterPlugins.cpp,v $
|   $Revision: 1.15 $
|       $Date: 2011/09/01 14:07:11 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Plugin/TypeConversionFilterV0.h>

#include <ScalableMesh/Import/DataTypeDescription.h>
#include "../Import/DimensionIterator.h"

#include "ScalableMeshDimensionTypeConversionFilter.h"

#include "ScalableMeshIDTMFileTraits.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)

using namespace IDTMFile;


namespace { // BEGIN UNAMED NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ConverterT, typename SrcTypeCreator, typename DstTypeCreator>
class RegisterConverter
    {
    struct Creator : public TypeConversionFilterCreatorBase
        {
        typename ConverterT::Binder             m_binder;

        explicit                                Creator                    ()
            :   TypeConversionFilterCreatorBase(SrcTypeCreator().Create(), DstTypeCreator().Create())
            {
            }

        virtual TypeConversionFilterBase*       _Create                    (const FilteringConfig&      config,
                                                                            Log&                        log) const override
            {
            return new ConverterT(GetSourceType(), GetTargetType(), config, log);
            }

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { m_binder.Bind(pi_Src, po_Dst); }
        };

    TypeConversionFilterRegistry::AutoRegister<Creator>
                                                m_autoRegister;
    };





/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT>
struct SameToSameConvertPoint
    {
    SrcT operator () (const SrcT& pi_srcPt) const
        { return pi_srcPt; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT>
struct AnyToXYZConvertPoint
    {
    Point3d64f operator () (const SrcT& pi_pt) const
        { 
        return PointTrait<Point3d64f>::Create(PointTrait<SrcT>::GetX(pi_pt), 
                                              PointTrait<SrcT>::GetY(pi_pt), 
                                              PointTrait<SrcT>::GetZ(pi_pt)); 
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT, typename DstT>
struct AnyToAnyConvertPoint
    {
    DstT operator () (const SrcT& pi_pt) const
        { 
        DstT pt(PointTrait<DstT>::Create(PointTrait<SrcT>::GetX(pi_pt), 
                                         PointTrait<SrcT>::GetY(pi_pt), 
                                         PointTrait<SrcT>::GetZ(pi_pt)));

        PointGroupIdTrait<DstT>::Set(pt, PointGroupIdTrait<SrcT>::Get(pi_pt));
        PointSignificanceTrait<DstT>::Set(pt, PointSignificanceTrait<SrcT>::Get(pi_pt));

        return pt; 
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT, typename DstT>
struct PointConverterTrait                                                  { typedef AnyToAnyConvertPoint<SrcT, DstT> type; };
template <typename SrcT> struct  PointConverterTrait<SrcT, Point3d64f>      { typedef AnyToXYZConvertPoint<SrcT> type;};
template <> struct PointConverterTrait<Point3d64f, Point3d64f>              { typedef SameToSameConvertPoint<Point3d64f> type; };
template <> struct PointConverterTrait<Point3d64fM64f, Point3d64fM64f>      { typedef SameToSameConvertPoint<Point3d64fM64f> type; };
template <> struct PointConverterTrait<Point3d64fG32, Point3d64fG32>        { typedef SameToSameConvertPoint<Point3d64fG32> type; };
template <> struct PointConverterTrait<Point3d64fM64fG32, Point3d64fM64fG32>{ typedef SameToSameConvertPoint<Point3d64fM64fG32> type; };
template <> struct PointConverterTrait<Point3d64fR8G8B8I8, Point3d64fR8G8B8I8>
                                                                            { typedef SameToSameConvertPoint<Point3d64fR8G8B8I8> type; };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcType, typename DstType>
class IDTMPointDimConverter : public DimensionFilter
    {
    typedef typename PointConverterTrait<SrcType, DstType>::type
                                                PerPointConverter;

    static const size_t     SrcStructSize = sizeof(SrcType);
    static const size_t     DstStructSize = sizeof(DstType);

    ConstPacketProxy<SrcType>                   m_srcPacket;
    PODPacketProxy<DstType>                     m_dstPacket;

    

    PerPointConverter                           m_perPointConverter;

    virtual void                                _Assign                (const Packet&               pi_Src,
                                                                        Packet&                     po_Dst) override
        {
        m_srcPacket.AssignTo(pi_Src);
        m_dstPacket.AssignTo(po_Dst);
        }

    virtual void                                _Run                   () override
        {
        assert(m_dstPacket.GetCapacity() >= m_srcPacket.GetSize());
        std::transform(m_srcPacket.begin(), m_srcPacket.end(), m_dstPacket.begin(), m_perPointConverter);
        m_dstPacket.SetSize(m_srcPacket.GetSize()); 
        }
public:
    class Binder : public PacketBinder
        {
        //TDORAY: Bind differently if dst point is smaller than src
        virtual void                            _Bind                  (const Packet&               pi_Src,
                                                                        Packet&                     po_Dst) const override
            { po_Dst.BindCapacityTo(pi_Src, DstStructSize, SrcStructSize); } 

        };

    explicit                                    IDTMPointDimConverter  (const DimensionOrg&,
                                                                        const DimensionOrg&,
                                                                        const FilteringConfig&,
                                                                        Log&) {}
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointDimConverterT>
class IDTMPointConverter : public TypeConversionFilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
        void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef PointDimConverterT                  PointDimConverter;

    enum 
        {
        POINT_DIM
        };

    PointDimConverter                           m_pointDimConv;

    virtual void                                _Assign                (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        m_pointDimConv.Assign(pi_Src[POINT_DIM], po_Dst[POINT_DIM]);
        }

    virtual void                                _Run                   () override
        {
        m_pointDimConv.Run();
        }

public:
    class Binder : public PacketGroupBinder
        {
        typename PointDimConverter::Binder      m_pointBinder;

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { 
            m_pointBinder.Bind(pi_Src[POINT_DIM], po_Dst[POINT_DIM]); 
            }
        };

    explicit                                    IDTMPointConverter         (const DataType&             src,
                                                                            const DataType&             dst,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        warnLog)
        :   m_pointDimConv(src.GetOrgGroup()[POINT_DIM], 
                           dst.GetOrgGroup()[POINT_DIM], 
                           config,
                           warnLog) 
        {}

    };





/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointDimConverterT>
class IDTMFeatureConverter : public TypeConversionFilterBase
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef DimTypeConvSame__To__Same           HeaderDimConverter;
    typedef PointDimConverterT                  PointDimConverter;


    enum 
        {
        HEADER_DIM,
        POINT_DIM,
        };  

    HeaderDimConverter                          m_headerDimConv;
    PointDimConverter                           m_pointDimConv;

    virtual void                                _Assign                (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        assert(po_Dst.GetSize() >= pi_Src.GetSize());
        m_headerDimConv.Assign(pi_Src[HEADER_DIM], po_Dst[HEADER_DIM]);
        m_pointDimConv.Assign(pi_Src[POINT_DIM], po_Dst[POINT_DIM]);
        }

    virtual void                                _Run                   () override
        {
        m_headerDimConv.Run();
        m_pointDimConv.Run();
        }
public:
    class Binder : public PacketGroupBinder
        {
        typename HeaderDimConverter::Binder     m_headerBinder;
        typename PointDimConverter::Binder      m_pointBinder;

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { 
            m_headerBinder.Bind(pi_Src[HEADER_DIM], po_Dst[HEADER_DIM]); 
            m_pointBinder.Bind(pi_Src[POINT_DIM], po_Dst[POINT_DIM]); 
            }
        };

    explicit                                    IDTMFeatureConverter       (const DataType&             src,
                                                                            const DataType&             dst,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        warnLog)
        :   m_headerDimConv(src.GetOrgGroup()[HEADER_DIM], 
                            dst.GetOrgGroup()[HEADER_DIM], 
                            config,
                            warnLog),
            m_pointDimConv(src.GetOrgGroup()[POINT_DIM], 
                           dst.GetOrgGroup()[POINT_DIM], 
                           config,
                           warnLog) 
        {}

    };





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Jean-Francois.Cote   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointDimConverterT>
class IDTMFeatureToPointConverter : public TypeConversionFilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef PointDimConverterT                  PointDimConverter;

    enum 
        {
        SRC_HEADER_DIM,
        SRC_POINT_DIM,
        };

    enum 
        {
        DST_POINT_DIM
        };

    PointDimConverter                           m_pointDimConv;


    virtual void                                _Assign                (const PacketGroup&              pi_Src,
                                                                        PacketGroup&                    po_Dst) override
        {
        m_pointDimConv.Assign(pi_Src[SRC_POINT_DIM], po_Dst[DST_POINT_DIM]);
        }

    virtual void                                _Run                   () override
        {
        m_pointDimConv.Run();
        }

public:
    class Binder : public PacketGroupBinder
        {
        typename PointDimConverter::Binder      m_pointBinder;

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { 
            m_pointBinder.Bind(pi_Src[SRC_POINT_DIM], po_Dst[DST_POINT_DIM]); 
            }
        };

    explicit                                    IDTMFeatureToPointConverter(const DataType&             src,
                                                                            const DataType&             dst,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        warnLog)
        :   m_pointDimConv(src.GetOrgGroup()[SRC_POINT_DIM], 
                           dst.GetOrgGroup()[DST_POINT_DIM], 
                           config,
                           warnLog) 
        {}

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier    08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class TINAsIDTMLinearToPointConverter : public TypeConversionFilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
        void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef IDTMFeatureArray<Point3d64f, FeatureHeader>
                                                LinearArray;
    typedef LinearArray::value_type             LinearType;

    enum 
        {
        SRC_HEADER_DIM,
        SRC_POINT_DIM,
        };

    enum 
        {
        DST_POINT_DIM
        };
    ConstPacketProxy<FeatureHeader>             m_srcHeaderPacket;
    ConstPacketProxy<Point3d64f>                m_srcPtPacket;
    PODPacketProxy<Point3d64f>                  m_dstPtPacket;

    LinearArray                                 m_srcLinearArray;

    virtual void                                _Assign                (const PacketGroup&              src,
                                                                        PacketGroup&                    dst) override
        {
        m_srcHeaderPacket.AssignTo(src[SRC_HEADER_DIM]);
        m_srcPtPacket.AssignTo(src[SRC_POINT_DIM]);
        m_dstPtPacket.AssignTo(dst[DST_POINT_DIM]);
        }

    struct CopyLinearPtsTo
        {
        PODPacketProxy<Point3d64f>&             m_dstPacket;

        explicit                                CopyLinearPtsTo        (PODPacketProxy<Point3d64f>&     dstPacket)
            : m_dstPacket(dstPacket) {}

        void                                    operator()             (const LinearType&               linear)
            {
            if (DTMFeatureType::TinPoint == (DTMFeatureType)linear.GetType())
                m_dstPacket.SetEnd(std::copy(linear.Begin(), linear.End(), m_dstPacket.begin() + m_dstPacket.GetSize()));
            }
        };

    virtual void                                _Run                   () override
        {
        m_srcLinearArray.EditHeaders().Wrap(m_srcHeaderPacket.Get(), m_srcHeaderPacket.GetSize());
        m_srcLinearArray.EditPoints().Wrap(m_srcPtPacket.Get(), m_srcPtPacket.GetSize());

        m_dstPtPacket.Clear();
        std::for_each(m_srcLinearArray.Begin(), m_srcLinearArray.End(), CopyLinearPtsTo(m_dstPtPacket));
        }

public:
    class Binder : public PacketGroupBinder
        {
        virtual void                            _Bind                      (const PacketGroup&          src,
                                                                            PacketGroup&                dst) const override
            { 
            dst[DST_POINT_DIM].BindUseSameAs(src[SRC_POINT_DIM]);
            }
        };

    explicit                                    TINAsIDTMLinearToPointConverter
                                                                           (const DataType&,
                                                                            const DataType&,
                                                                            const FilteringConfig&,
                                                                            Log&)
        {}

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier    08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class TINAsIDTMLinearToIDTMLinearConverter : public TypeConversionFilterBase
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    enum 
        {
        SRC_HEADER_DIM,
        SRC_POINT_DIM,
        };

    enum 
        {
        DST_HEADER_DIM,
        DST_POINT_DIM,
        };

    typedef IDTMFeatureArray<Point3d64f, FeatureHeader>
                                                LinearArray;
    typedef LinearArray::value_type             LinearType;

    ConstPacketProxy<FeatureHeader>             m_srcHeaderPacket;
    PODPacketProxy<FeatureHeader>               m_dstHeaderPacket;

    ConstPacketProxy<Point3d64f>                m_srcPointPacket;
    PODPacketProxy<Point3d64f>                  m_dstPointPacket;

    LinearArray                                 m_srcLinFeatArray;
    LinearArray                                 m_dstLinFeatArray;

    virtual void                                _Assign                (const PacketGroup&              src,
                                                                        PacketGroup&                    dst) override
        {
        m_srcHeaderPacket.AssignTo(src[SRC_HEADER_DIM]);
        m_dstHeaderPacket.AssignTo(dst[DST_HEADER_DIM]);

        m_srcPointPacket.AssignTo(src[SRC_POINT_DIM]);
        m_dstPointPacket.AssignTo(dst[DST_POINT_DIM]);
        }

    struct ChangeLinearType
        {
        void                                    operator()             (FeatureHeader&                  linearHeader) const
            {
            if (DTMFeatureType::TinLine == (DTMFeatureType)linearHeader.type)
                linearHeader.type = (IDTMFile::FeatureType)DTMFeatureType::Breakline;
            }
        };

    struct SelectionPredicate : public std::unary_function<LinearType, bool>
        {
        bool                                    operator()             (const LinearType&               linear) const
            {
            return DTMFeatureType::TinLine == (DTMFeatureType)linear.GetType ();
            }
        };

    virtual void                                _Run                   () override
        {
        m_srcLinFeatArray.EditHeaders().Wrap(m_srcHeaderPacket.Get(), m_srcHeaderPacket.GetSize());
        m_srcLinFeatArray.EditPoints().Wrap(m_srcPointPacket.Get(), m_srcPointPacket.GetSize());

        m_dstLinFeatArray.EditHeaders().WrapEditable(m_dstHeaderPacket.Edit(), 0, m_dstHeaderPacket.GetCapacity());
        m_dstLinFeatArray.EditPoints().WrapEditable(m_dstPointPacket.Edit(), 0, m_dstPointPacket.GetCapacity());

        std::remove_copy_if(m_srcLinFeatArray.Begin(), m_srcLinFeatArray.End(), 
                            back_inserter(m_dstLinFeatArray), 
                            not1(SelectionPredicate()));

        m_dstHeaderPacket.SetSize(m_dstLinFeatArray.GetHeaders().GetSize());
        m_dstPointPacket.SetSize(m_dstLinFeatArray.GetPoints().GetSize());

        std::for_each(m_dstHeaderPacket.begin(), m_dstHeaderPacket.end(), ChangeLinearType());
        }

public:
    class Binder : public PacketGroupBinder
        {
        virtual void                            _Bind                      (const PacketGroup&          src,
                                                                            PacketGroup&                dst) const override
            { 
            dst[DST_HEADER_DIM].BindUseSameAs(src[SRC_HEADER_DIM]);
            dst[DST_POINT_DIM].BindUseSameAs(src[SRC_POINT_DIM]);
            }
        };


    explicit                                    TINAsIDTMLinearToIDTMLinearConverter
                                                                           (const DataType&,
                                                                            const DataType&,
                                                                            const FilteringConfig&,
                                                                            Log&)
        {}

    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
struct PointDimConverterTrait
    {
    template <bool IsSameType>
    struct Impl         { typedef IDTMPointDimConverter<SrcPointT, DstPointT> type; };
    template <>
    struct Impl<true>   { typedef DimTypeConvSame__To__Same type; };

    typedef typename Impl<PointTypeIDTrait<SrcPointT>::value == PointTypeIDTrait<DstPointT>::value>::type type;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
class RegisterIDTMPointConverter
    {
    RegisterConverter<IDTMPointConverter< typename PointDimConverterTrait<SrcPointT, DstPointT>::type >, 
                      typename PointTypeCreatorTrait<SrcPointT>::type, 
                      typename PointTypeCreatorTrait<DstPointT>::type>
                                                m_autoRegister;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
class RegisterIDTMLinearConverter
    {
    RegisterConverter<IDTMFeatureConverter< typename PointDimConverterTrait<SrcPointT, DstPointT>::type >, 
                      typename LinearTypeCreatorTrait<SrcPointT>::type, 
                      typename LinearTypeCreatorTrait<DstPointT>::type>
                                                m_autoRegister;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                Jean-Francois.Cote   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
class RegisterIDTMLinearToPointConverter
    {
    RegisterConverter<IDTMFeatureToPointConverter< typename PointDimConverterTrait<SrcPointT, DstPointT>::type >, 
                      typename LinearTypeCreatorTrait<SrcPointT>::type, 
                      typename PointTypeCreatorTrait<DstPointT>::type>
                                                m_autoRegister;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
class RegisterMeshAsIDTMLinearConverter
    {
    RegisterConverter<IDTMFeatureConverter< typename PointDimConverterTrait<SrcPointT, DstPointT>::type >, 
                      typename MeshAsLinearTypeCreatorTrait<SrcPointT>::type, 
                      typename MeshAsLinearTypeCreatorTrait<DstPointT>::type>
                                                m_autoRegister;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                Jean-Francois.Cote   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class RegisterMeshAsIDTMLinearToPointConverter
    {
    RegisterConverter<TINAsIDTMLinearToPointConverter, 
                      typename MeshAsLinearTypeCreatorTrait<Point3d64f>::type, 
                      typename PointTypeCreatorTrait<Point3d64f>::type>
                                                m_autoRegister;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class RegisterMeshAsIDTMLinearToIDTMLinearConverter
    {
    RegisterConverter<TINAsIDTMLinearToIDTMLinearConverter, 
                      typename MeshAsLinearTypeCreatorTrait<Point3d64f>::type, 
                      typename LinearTypeCreatorTrait<Point3d64f>::type>
                                                m_autoRegister;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT, typename DstPointT>
class RegisterTINAsIDTMLinearConverter
    {
    RegisterConverter<IDTMFeatureConverter< typename PointDimConverterTrait<SrcPointT, DstPointT>::type >, 
                      typename TINAsLinearTypeCreatorTrait<SrcPointT>::type, 
                      typename TINAsLinearTypeCreatorTrait<DstPointT>::type>
                                                m_autoRegister;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                Jean-Francois.Cote   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class RegisterTINAsIDTMLinearToPointConverter
    {
    RegisterConverter<TINAsIDTMLinearToPointConverter, 
                      typename TINAsLinearTypeCreatorTrait<Point3d64f>::type, 
                      typename PointTypeCreatorTrait<Point3d64f>::type>
                                                m_autoRegister;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class RegisterTINAsIDTMLinearToIDTMLinearConverter
    {
    RegisterConverter<TINAsIDTMLinearToIDTMLinearConverter, 
                      typename TINAsLinearTypeCreatorTrait<Point3d64f>::type, 
                      typename LinearTypeCreatorTrait<Point3d64f>::type>
                                                m_autoRegister;
    };





// Register point converters:
const RegisterIDTMPointConverter<Point3d64f, Point3d64f>                        s_ptTypeConv0; 
const RegisterIDTMPointConverter<Point3d64f, Point3d64fM64f>                    s_ptTypeConv1; 
const RegisterIDTMPointConverter<Point3d64f, Point3d64fG32>                     s_ptTypeConv2; 
const RegisterIDTMPointConverter<Point3d64f, Point3d64fM64fG32>                 s_ptTypeConv3; 
const RegisterIDTMPointConverter<Point3d64fM64f, Point3d64f>                    s_ptTypeConv4; 
const RegisterIDTMPointConverter<Point3d64fG32, Point3d64f>                     s_ptTypeConv5; 
const RegisterIDTMPointConverter<Point3d64fM64fG32, Point3d64f>                 s_ptTypeConv6; 


const RegisterConverter<IDTMPointConverter< IDTMPointDimConverter<Point3d64f, Point3d64f> >, 
                        PointType3d64f_R16G16B16_I16Creator, 
                        PointType3d64fCreator>                                  s_ptTypeConvSp0; 


// Register linear converters
const RegisterIDTMLinearConverter<Point3d64f, Point3d64f>                       s_linTypeConv0;
const RegisterIDTMLinearConverter<Point3d64f, Point3d64fM64f>                   s_linTypeConv1; 
const RegisterIDTMLinearConverter<Point3d64fM64f, Point3d64f>                   s_linTypeConv2; 

// Register linear to point converters
const RegisterIDTMLinearToPointConverter<Point3d64f, Point3d64f>                s_linToPtTypeConv0;



// Register mesh converters
const RegisterMeshAsIDTMLinearConverter<Point3d64f, Point3d64f>                 s_meshTypeConv0;
// Register mesh to points converters
const RegisterMeshAsIDTMLinearToPointConverter                                  s_meshToPtTypeConv0;
// Register mesh to linear converters
const RegisterMeshAsIDTMLinearToIDTMLinearConverter                             s_meshToLinTypeConv0;


// Register TIN converters
const RegisterTINAsIDTMLinearConverter<Point3d64f, Point3d64f>                  s_tinTypeConv0;
// Register TIN to points converters
const RegisterTINAsIDTMLinearToPointConverter                                   s_tinToPtTypeConv0;
// Register TIN to linear converters
const RegisterTINAsIDTMLinearToIDTMLinearConverter                              s_tinToLinTypeConv0;


} // END UNAMED NAMESPACE