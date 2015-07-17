/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/CustomFilterFactoryV0.h $
|    $RCSfile: CustomFilterFactoryV0.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/01 14:07:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/DataType.h>
#include <ScalableTerrainModel/Foundations/Warning.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct DataType;
struct CustomFilterFactory;
struct FilterCreator;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

struct FilterCreatorBase;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilterFactoryBase : private Uncopyable, public ShareableObjectTypeTrait<CustomFilterFactoryBase>::type
    {
private:
    friend struct                           CustomFilterFactory;

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


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE