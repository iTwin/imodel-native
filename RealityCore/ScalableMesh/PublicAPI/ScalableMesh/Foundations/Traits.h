/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  Type trait used to remove reference from a specified type.
*
* TDORAY:   Taken from <ImagePP/h/HStlStuff.h>. Find a way to place this tool in 
*           a place that both ImagePP and Memory can depend on (e.g.: Bentley headers).
*
* @bsiclass                                                  Raymond.Gauthier   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> struct RemoveReference     {
    typedef T type;
    };
template <typename T> struct RemoveReference<T&> {
    typedef T type;
    };


END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
