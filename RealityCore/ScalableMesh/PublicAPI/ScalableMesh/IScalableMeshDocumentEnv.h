/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DocumentEnv
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    BENTLEY_SM_EXPORT  explicit                            DocumentEnv                        (const WChar*          currentDir);

    BENTLEY_SM_EXPORT                                    ~DocumentEnv                       ();

    BENTLEY_SM_EXPORT                                    DocumentEnv                        (const DocumentEnv&      rhs);
    BENTLEY_SM_EXPORT DocumentEnv&                        operator=                          (const DocumentEnv&      rhs);


    BENTLEY_SM_EXPORT const WString&     GetCurrentDir                      () const;
    BENTLEY_SM_EXPORT const WChar*          GetCurrentDirCStr                  () const;

    // TDORAY: Add a search paths accessors (storing stuff like MS_REFDIR)

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
