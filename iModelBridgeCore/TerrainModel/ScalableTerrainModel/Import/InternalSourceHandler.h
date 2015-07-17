/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/InternalSourceHandler.h $
|    $RCSfile: InternalSourceHandler.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 20:04:43 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct SourceBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct Source;
struct ContentDescriptor;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct InternalSourceHandler
    {
    typedef Plugin::V0::SourceBase              Base;
public:
    static Source*                              CreateFromBase                     (Base*                               baseP);

    static Base&                                GetOriginalBaseFor                 (Source&                             source);
    static const Base&                          GetOriginalBaseFor                 (const Source&                       source);

    static Base&                                GetBaseFor                         (Source&                             source);
    static const Base&                          GetBaseFor                         (const Source&                       source);

    static const ContentDescriptor&             GetDescriptorFor                   (const Source&                       source);
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE