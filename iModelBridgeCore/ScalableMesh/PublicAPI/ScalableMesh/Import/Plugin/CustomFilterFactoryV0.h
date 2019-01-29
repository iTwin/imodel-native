/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/CustomFilterFactoryV0.h $
|    $RCSfile: CustomFilterFactoryV0.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/01 14:07:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Foundations/Warning.h>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct DataType;
struct CustomFilterFactory;
struct FilterCreator;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

struct FilterCreatorBase;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilterFactoryBase : private Uncopyable, public ShareableObjectTypeTrait<CustomFilterFactoryBase>::type
    {
private:
    friend struct                           BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::CustomFilterFactory;

    const void*                             m_implP; // Reserve some space for further use

    virtual const FilterCreatorBase*        _FindCreatorFor                        (const DataType&                     sourceType,
                                                                                    Log&                                log) const = 0;

protected:
    typedef FilterCreatorBase               FilterCreatorBase;
    typedef DataType                        DataType;
    typedef Log                             Log;

    IMPORT_DLLE static CustomFilterFactory  CreateFromBase                         (CustomFilterFactoryBase*            filterP);


    IMPORT_DLLE explicit                    CustomFilterFactoryBase                ();
    IMPORT_DLLE virtual                     ~CustomFilterFactoryBase               () = 0;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
