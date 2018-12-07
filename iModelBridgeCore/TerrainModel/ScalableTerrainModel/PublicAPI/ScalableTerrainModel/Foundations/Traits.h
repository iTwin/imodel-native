/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Traits.h $
|    $RCSfile: Algorithm.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/12/20 16:23:40 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE


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


END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE