/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Filter.h $
|    $RCSfile: Filter.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/01 14:07:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>


BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct FilterCreatorBase;
struct FilterBase;
struct CustomFilterFactoryBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE
struct PacketGroup;
END_BENTLEY_MRDTM_MEMORY_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Forward declarations
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataType;

struct Filter;
struct FilterCreator;

struct FilteringConfig;

typedef SharedPtrTypeTrait<const FilterCreator>::type
                                            FilterCreatorCPtr; 

typedef SharedPtrTypeTrait<Filter>::type    FilterPtr;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterCreator : private Uncopyable, public ShareableObjectTypeTrait<FilterCreator>::type
    {
private:
    typedef Plugin::V0::FilterCreatorBase   Base;
    friend                                  Base;

    std::auto_ptr<const Base>                    m_baseP;

public:
    IMPORT_DLLE explicit                    FilterCreator                      (const Base*                 baseP);
    IMPORT_DLLE                             ~FilterCreator                     ();

    IMPORT_DLLE FilterPtr                   Create                             (const PacketGroup&          src,
                                                                                PacketGroup&                dst,
                                                                                Log&                        log) const;  

    IMPORT_DLLE FilterPtr                   Create                             (PacketGroup&                src,
                                                                                PacketGroup&                dst,
                                                                                Log&                        log) const; 

    IMPORT_DLLE FilterPtr                   Create                             (const PacketGroup&          src,
                                                                                PacketGroup&                dst,
                                                                                const FilteringConfig&      config,
                                                                                Log&                        log) const;  

    IMPORT_DLLE FilterPtr                   Create                             (PacketGroup&                src,
                                                                                PacketGroup&                dst,
                                                                                const FilteringConfig&      config,
                                                                                Log&                        log) const; 

    IMPORT_DLLE const DataType&             GetSourceType                      () const;
    IMPORT_DLLE const DataType&             GetTargetType                      () const;


    IMPORT_DLLE bool                        ProducesEffects                    () const;
    };





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Filter : private Uncopyable, public ShareableObjectTypeTrait<Filter>::type
    {
private:
    typedef Plugin::V0::FilterBase          Base;

    friend struct                           FilterCreator;

    std::auto_ptr<Base>                     m_baseP;
    explicit                                Filter                             (Base*                       baseP);
public:
    IMPORT_DLLE                             ~Filter                            ();

    IMPORT_DLLE void                        Run                                ();
    };

END_BENTLEY_MRDTM_IMPORT_NAMESPACE