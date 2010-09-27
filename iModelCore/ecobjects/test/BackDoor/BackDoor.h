/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/BackDoor/BackDoor.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjectsAPI.h>

#if defined (USE_PUBLISHED_HEADERS)

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
 @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct          ILeakDetector
{
virtual void    GetStats(Int32& currentLive, Int32& totalAllocs, Int32& totalFrees) const = 0;
virtual void    ResetStats() = 0;
virtual void    ReportStats (const wchar_t* prefix) const = 0;
virtual Int32   CheckForLeaks () const = 0;
};
END_BENTLEY_EC_NAMESPACE

#endif

/*---------------------------------------------------------------------------------**//**
* These methods are merely wrappers within a namespace.
* - They should have the same declaration unless they are wrapping class member functions.
*   In that case, the first parameter should be a pointer to that class.
* - You should try to use only published methods unless you must use non-published.
* - You should also question whether something should be published if you need to use it.
*                                                              KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
namespace BackDoor
{
    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace ECSchema
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod
        +-------+---------------+---------------+---------------+---------------+------*/
        EC::ILeakDetector&      Debug_GetLeakDetector ();

    }; // ECSchema

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace ECClass
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod
        +-------+---------------+---------------+---------------+---------------+------*/
        EC::ILeakDetector&      Debug_GetLeakDetector ();

    }; // ECClass

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace ECProperty
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod
        +-------+---------------+---------------+---------------+---------------+------*/
        EC::ILeakDetector&      Debug_GetLeakDetector ();

    }; // ECProperty

};
