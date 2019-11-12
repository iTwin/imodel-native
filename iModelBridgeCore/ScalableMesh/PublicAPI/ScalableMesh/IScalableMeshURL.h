/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
// NTERAY: See if Bentley.h's forward declaration may suffice.
#include <Bentley/WString.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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
    BENTLEY_SM_EXPORT explicit                LocalFileURL                           (const WChar*          path);

    BENTLEY_SM_EXPORT                         ~LocalFileURL                          ();

    BENTLEY_SM_EXPORT                         LocalFileURL                           (const LocalFileURL&     rhs);
    BENTLEY_SM_EXPORT LocalFileURL&           operator=                              (const LocalFileURL&     rhs);


    BENTLEY_SM_EXPORT const WString&     GetPath                                () const;
    BENTLEY_SM_EXPORT const WChar*          GetPathCStr                            () const;

    // TDORAY: Return raw URL, extensions, filename etc..

    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
