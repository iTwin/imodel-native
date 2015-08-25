/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshReprojectionFilterPlugins.cpp $
|    $RCSfile: ScalableMeshReprojectionFilterPlugins.cpp,v $
|   $Revision: 1.15 $
|       $Date: 2011/09/01 14:07:09 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Plugin/ReprojectionFilterV0.h>

#include "ScalableMeshDimensionTypeConversionFilter.h"
#include "ScalableMeshIDTMFileTraits.h"

#include <ScalableMesh/Import/Exceptions.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)

using namespace IDTMFile;

namespace { // BEGIN UNAMED NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ReprojectorT, typename SrcTypeCreator>
class RegisterReprojector
    {
    struct Creator : public ReprojectionFilterCreatorBase
        {
        typename ReprojectorT::Binder           m_binder;

        explicit                                Creator                    ()
            :   ReprojectionFilterCreatorBase(SrcTypeCreator().Create())
            {
            }

        virtual ReprojectionFilterBase*       _Create                      (const Reprojection&         reprojection,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        log) const override
            {
            return new ReprojectorT(reprojection, GetType(), config, log);
            }

        virtual void                            _Bind                      (const PacketGroup&          src,
                                                                            PacketGroup&                dst) const override
            { m_binder.Bind(src, dst); }

        };

    ReprojectionFilterRegistry::AutoRegister<Creator>
                                                m_autoRegister;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT>
struct AnyReprojectPoint
    {
private:
    const Reprojection&         m_rReprojection;
public:
    explicit                AnyReprojectPoint      (const Reprojection&         pi_rReprojection)
        :   m_rReprojection(pi_rReprojection) 
        {
        
        }

    SrcT operator () (const SrcT& pi_pt) const
        { 
        DPoint3d pt(PointTrait<DPoint3d>::Create(PointTrait<SrcT>::GetX(pi_pt), 
                                                 PointTrait<SrcT>::GetY(pi_pt), 
                                                 PointTrait<SrcT>::GetZ(pi_pt)));

        const Reprojection::Status status = m_rReprojection.Reproject(pt, pt);
        if (Reprojection::S_SUCCESS != status)
                throw ReprojectionException(status);

        return PointTrait<SrcT>::Create(PointTrait<DPoint3d>::GetX(pt), 
                                        PointTrait<DPoint3d>::GetY(pt), 
                                        PointTrait<DPoint3d>::GetZ(pt)); 
        }


    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT>
struct PointReprojectorTrait                                               { typedef AnyReprojectPoint<SrcT> type; };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Type>
class IDTMPointDimReprojector : public DimensionFilter
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef typename PointReprojectorTrait<Type>::type
                                                PerPointReprojector;

    Reprojection                                m_reprojection;

    ConstPacketProxy<Type>                      m_srcPacket;
    PODPacketProxy<Type>                        m_dstPacket;


    virtual void                                _Assign                    (const Packet&               pi_Src,
                                                                            Packet&                     po_Dst) override
        {
        m_srcPacket.AssignTo(pi_Src);
        m_dstPacket.AssignTo(po_Dst);
        }

    virtual void                                _Run                       () override
        {
        assert(m_dstPacket.GetCapacity() >= m_srcPacket.GetSize());

        m_dstPacket.SetEnd(std::transform(m_srcPacket.begin(), m_srcPacket.end(), 
                                          m_dstPacket.begin(), 
                                          PerPointReprojector(m_reprojection)));
        }
public:
    explicit                                    IDTMPointDimReprojector    (const Reprojection&         reprojection,
                                                                            const DimensionOrg&,
                                                                            const FilteringConfig&,
                                                                            Log&)
        :   m_reprojection(reprojection)
        {
        }

    class Binder : public PacketBinder
        {
        virtual void                            _Bind                      (const Packet&               pi_Src,
                                                                            Packet&                     po_Dst) const override
            { po_Dst.BindUseSameAs(pi_Src); }

        };
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class IDTMPoint3d64fDimReprojector : public DimensionFilter
    {
    Reprojection                                m_reprojection;

    ConstPacketProxy<DPoint3d>                  m_srcPacket;
    PODPacketProxy<DPoint3d>                    m_dstPacket;

    virtual void                                _Assign                    (const Packet&               pi_Src,
                                                                            Packet&                     po_Dst) override
        {
        m_srcPacket.AssignTo(pi_Src);
        m_dstPacket.AssignTo(po_Dst);
        }

    virtual void                                _Run                       () override
        {
        m_dstPacket.Reserve(m_srcPacket.GetCapacity());
        assert(m_dstPacket.GetCapacity() >= m_srcPacket.GetSize());


        const DPoint3d* const pSourcePts = m_srcPacket.Get();
        const size_t sourcePtCount = m_srcPacket.GetSize();

        DPoint3d* const pTargetPts = m_dstPacket.Edit();


        Reprojection::Status status = m_reprojection.Reproject(pSourcePts, sourcePtCount, pTargetPts);
        if (Reprojection::S_SUCCESS != status)
            throw ReprojectionException(status);

        m_dstPacket.SetSize(sourcePtCount);
        }
public:
    explicit                                    IDTMPoint3d64fDimReprojector
                                                                           (const Reprojection&         pi_rReprojection,
                                                                            const DimensionOrg&,
                                                                            const FilteringConfig&,
                                                                            Log&)
        :   m_reprojection(pi_rReprojection)
        {
        }

    class Binder : public PacketBinder
        {
        virtual void                            _Bind                      (const Packet&               pi_Src,
                                                                            Packet&                     po_Dst) const override
            { po_Dst.BindUseSameAs(pi_Src); }

        };
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcT>
struct IDTMPointDimReprojectorTrait                                         { typedef IDTMPointDimReprojector<SrcT> type; };

template <> struct IDTMPointDimReprojectorTrait<DPoint3d>                   { typedef IDTMPoint3d64fDimReprojector  type; };
template <> struct IDTMPointDimReprojectorTrait<IDTMFile::Point3d64f>       { typedef IDTMPoint3d64fDimReprojector  type; };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointDimReprojectorT>
class IDTMPointReprojector : public ReprojectionFilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef PointDimReprojectorT                PointDimReprojector;

    enum {POINT_DIM};

    PointDimReprojector                         m_pointDimReprojector;

    virtual void                                _Assign                    (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) override
        {
        m_pointDimReprojector.Assign(pi_Src[POINT_DIM], po_Dst[POINT_DIM]);
        }

    virtual void                                _Run                       () override
        {
        m_pointDimReprojector.Run();
        }

public:
    explicit                                    IDTMPointReprojector       (const Reprojection&         reprojection,
                                                                            const DataType&             type,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        warnLog)
        :   m_pointDimReprojector(reprojection, type.GetOrgGroup()[POINT_DIM], config, warnLog)
        {
        }

    class Binder : public PacketGroupBinder
        {
        typename PointDimReprojector::Binder    m_pointBinder;

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { 
            m_pointBinder.Bind(pi_Src[POINT_DIM], po_Dst[POINT_DIM]); 
            }
        };
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointDimReprojectorT>
class IDTMFeatureReprojector : public ReprojectionFilterBase
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    typedef PointDimReprojectorT                PointDimReprojector;

    enum 
        {
        HEADER_DIM,
        POINT_DIM,
        };  

    DimTypeConvSame__To__Same                   m_headerDimReprojector;
    PointDimReprojector                         m_pointDimReprojector;

    virtual void                                _Assign                    (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) override
        {
        m_headerDimReprojector.Assign(pi_Src[HEADER_DIM], po_Dst[HEADER_DIM]);
        m_pointDimReprojector.Assign(pi_Src[POINT_DIM], po_Dst[POINT_DIM]);
        }

    virtual void                                _Run                       () override
        {
        m_headerDimReprojector.Run();
        m_pointDimReprojector.Run();
        }

public:
    explicit                                    IDTMFeatureReprojector     (const Reprojection&         reprojection,
                                                                            const DataType&             type,
                                                                            const FilteringConfig&      config,
                                                                            Log&                        warnLog)
        :   m_headerDimReprojector(type.GetOrgGroup()[HEADER_DIM], 
                                   type.GetOrgGroup()[HEADER_DIM], 
                                   config,
                                   warnLog),
            m_pointDimReprojector(reprojection, 
                                  type.GetOrgGroup()[POINT_DIM], 
                                  config,
                                  warnLog)
        {
        }

    class Binder : public PacketGroupBinder
        {
        typename DimTypeConvSame__To__Same::Binder  
                                                m_headerBinder;
        typename PointDimReprojector::Binder    m_pointBinder;

        virtual void                            _Bind                      (const PacketGroup&          pi_Src,
                                                                            PacketGroup&                po_Dst) const override
            { 
            m_headerBinder.Bind(pi_Src[HEADER_DIM], po_Dst[HEADER_DIM]); 
            m_pointBinder.Bind(pi_Src[POINT_DIM], po_Dst[POINT_DIM]); 
            }
        };
    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT>
class RegisterIDTMPointReprojector
    {
    RegisterReprojector<IDTMPointReprojector< typename IDTMPointDimReprojectorTrait<SrcPointT>::type >, 
                    typename PointTypeCreatorTrait<SrcPointT>::type>
                                            m_autoRegister;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT>
class RegisterIDTMLinearReprojector
    {
    RegisterReprojector<IDTMFeatureReprojector<typename IDTMPointDimReprojectorTrait<SrcPointT>::type >, 
                    typename LinearTypeCreatorTrait<SrcPointT>::type>
                                            m_autoRegister;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SrcPointT>
class RegisterMeshAsIDTMLinearReprojector
    {
    RegisterReprojector<IDTMFeatureReprojector<typename IDTMPointDimReprojectorTrait<SrcPointT>::type >, 
                    typename MeshAsLinearTypeCreatorTrait<SrcPointT>::type>
                                            m_autoRegister;
    };



// Register point converters:
const RegisterIDTMPointReprojector<Point3d64f> s_ptReproj0; 
const RegisterIDTMPointReprojector<Point3d64fM64f> s_ptReproj1; 
const RegisterIDTMPointReprojector<Point3d64fG32> s_ptReproj2;
const RegisterIDTMPointReprojector<Point3d64fM64fG32> s_ptReproj3;


// Register linear feature converters
const RegisterIDTMLinearReprojector<Point3d64f> s_ftReproj0; 
const RegisterIDTMLinearReprojector<Point3d64fM64f> s_ftReproj1; 



} // END UNAMED NAMESPACE