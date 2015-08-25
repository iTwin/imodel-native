/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/TypeConversionFilterV0.h $
|    $RCSfile: TypeConversionFilterV0.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/09/01 14:07:38 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Plugin/FilterV0.h>

#include <ScalableMesh/Import/Plugin/PacketBinderV0.h>
#include <ScalableMesh/Import/Plugin/TypeConversionFilterRegistry.h>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct TypeConversionFilterCreator;
struct TypeConversionFilterFactory;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE
struct TypeConversionFilterCreatorPlugin;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterBase : public FilterBase
    {
private:
    friend struct                           TypeConversionFilterCreatorBase;
    friend struct                           TypeConversionFilterCreator;

    struct                                  Impl;
    const void*                             m_implP; // Reserved for further use

protected:
    IMPORT_DLLE explicit                    TypeConversionFilterBase           ();

public:
    IMPORT_DLLE virtual                     ~TypeConversionFilterBase          () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterCreatorBase : private Uncopyable, public PacketGroupBinder
    {
private:
    friend struct                           TypeConversionFilterCreatorPlugin;
    friend struct                           TypeConversionFilterCreator;

    DataType                                m_sourceType;
    DataType                                m_targetType;

    virtual TypeConversionFilterBase*       _Create                            (const FilteringConfig&  config,
                                                                                Log&                    log) const = 0;

public:
    typedef const TypeConversionFilterCreatorBase*    
                                            ID;

    IMPORT_DLLE explicit                    TypeConversionFilterCreatorBase    (const DataType&         source,
                                                                                const DataType&         target);

    IMPORT_DLLE virtual                     ~TypeConversionFilterCreatorBase   () = 0;

    IMPORT_DLLE const DataType&             GetSourceType                      () const;
    IMPORT_DLLE const DataType&             GetTargetType                      () const;
    };

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
