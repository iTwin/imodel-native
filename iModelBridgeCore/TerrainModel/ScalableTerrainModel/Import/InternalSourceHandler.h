/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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