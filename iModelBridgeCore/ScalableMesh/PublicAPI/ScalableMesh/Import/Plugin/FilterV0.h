/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/FilterV0.h $
|    $RCSfile: FilterV0.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/22 16:26:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
 
/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Plugin/PacketBinderV0.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Memory/Packet.h>
#include <ScalableMesh/Memory/PacketAccess.h>
#include <ScalableMesh/Import/Warnings.h>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct FilterCreator;
struct FilteringConfig;

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)


struct FilterBase;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterCreatorBase : private Uncopyable, protected PacketGroupBinder
    {
private:
    friend struct                           FilterCreator;
    friend struct                           CustomFilterFactoryBase;
    struct                                  Impl;
    Impl*                                   m_implP; // Reserve some space for future uses

    virtual FilterBase*                     _Create                            (const PacketGroup&          src,
                                                                                PacketGroup&                dst,
                                                                                const FilteringConfig&      config,
                                                                                Log&                        log) const = 0;

    virtual const DataType&                 _GetSourceType                     () const = 0;
    virtual const DataType&                 _GetTargetType                     () const = 0;

    IMPORT_DLLE virtual bool                _ProducesEffects                   () const;

protected:
    typedef FilterBase                      FilterBase;
    struct                                  FilterHandler;

    IMPORT_DLLE explicit                    FilterCreatorBase                  ();
    IMPORT_DLLE static FilterCreator*       CreateFromBase                     (FilterCreatorBase*          baseP);

public:
    IMPORT_DLLE virtual                     ~FilterCreatorBase                 () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterBase : private Uncopyable, public ShareableObjectTypeTrait<FilterBase>::type
    {
private:
    friend struct                   Filter;
    friend struct                   FilterCreator;
    friend struct                   FilterCreatorBase;

    struct                          Impl;
    void*                           m_implP; // Reserve some space for further use

    virtual void                    _Assign                            (const PacketGroup&              src,
                                                                        PacketGroup&                    dst) = 0;

    virtual void                    _Run                               () = 0;

protected:
    struct                          Handler;

    IMPORT_DLLE explicit            FilterBase                         ();

public:
    IMPORT_DLLE virtual             ~FilterBase                        () = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterBase::Handler
    {
    static void                     Assign                             (FilterBase&                     instance,
                                                                        const PacketGroup&              src,
                                                                        PacketGroup&                    dst);

    static void                     Run                                (FilterBase&                     instance);
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterCreatorBase::FilterHandler : public FilterBase::Handler
    {
    };

// TDORAY: Move to hpp?
inline void FilterBase::Handler::Assign    (FilterBase&         instance,
                                            const PacketGroup&  src,
                                            PacketGroup&        dst)
    { instance._Assign(src, dst); }

inline void FilterBase::Handler::Run   (FilterBase& instance)
    { instance._Run(); }

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
