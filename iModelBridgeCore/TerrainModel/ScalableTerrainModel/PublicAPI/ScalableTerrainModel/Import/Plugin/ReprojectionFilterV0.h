/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/ReprojectionFilterV0.h $
|    $RCSfile: ReprojectionFilterV0.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/09/01 14:07:35 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Plugin/FilterV0.h>

#include <ScalableTerrainModel/Import/Plugin/PacketBinderV0.h>
#include <ScalableTerrainModel/Import/Plugin/ReprojectionFilterRegistry.h>

#include <ScalableTerrainModel/GeoCoords/Reprojection.h>


BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct ReprojectionFilterCreator;
struct ReprojectionFilterFactory;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct ReprojectionFilterCreatorPlugin;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterBase : public FilterBase
    {
private:
    friend struct                           ReprojectionFilterCreatorBase;
    friend struct                           ReprojectionFilterCreator;

    struct                                  Impl;
    const void*                             m_implP; // Reserved for further use

protected:
    IMPORT_DLLE explicit                    ReprojectionFilterBase             ();

public:
    IMPORT_DLLE virtual                     ~ReprojectionFilterBase            () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterCreatorBase : private Uncopyable, public PacketGroupBinder
    {
private:
    friend struct                           ReprojectionFilterCreator;
    friend struct                           ReprojectionFilterCreatorPlugin;

    DataType                                m_type;

    virtual ReprojectionFilterBase*         _Create                            (const GeoCoords::Reprojection&  reprojection,
                                                                                const FilteringConfig&          config,
                                                                                Log&                            log) const = 0;

protected:
    IMPORT_DLLE explicit                    ReprojectionFilterCreatorBase      (const DataType&                 type);

    IMPORT_DLLE const DataType&             GetType                            () const;

public:
    typedef const ReprojectionFilterCreatorBase*    
                                            ID;

    IMPORT_DLLE virtual                     ~ReprojectionFilterCreatorBase     () = 0;
    };

END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE