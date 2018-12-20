/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMURL.h $
|    $RCSfile: IMrDTMURL.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 15:00:10 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileURL // TDORAY: Make this one part of a URL hierarchy?
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                 m_implP;
public:
    BENTLEYSTM_EXPORT explicit                LocalFileURL                           (const WChar*          path);

    BENTLEYSTM_EXPORT                         ~LocalFileURL                          ();

    BENTLEYSTM_EXPORT                         LocalFileURL                           (const LocalFileURL&     rhs);
    BENTLEYSTM_EXPORT LocalFileURL&           operator=                              (const LocalFileURL&     rhs);


    BENTLEYSTM_EXPORT const WString&     GetPath                                () const;
    BENTLEYSTM_EXPORT const WChar*          GetPathCStr                            () const;

    // TDORAY: Return raw URL, extensions, filename etc..

    };



END_BENTLEY_MRDTM_NAMESPACE