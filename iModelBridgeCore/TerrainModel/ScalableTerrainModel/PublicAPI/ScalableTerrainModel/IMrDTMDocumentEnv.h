/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>


BEGIN_BENTLEY_MRDTM_NAMESPACE


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
    explicit                            DocumentEnv                        (const WChar*          currentDir);

                                        ~DocumentEnv                       ();

                                        DocumentEnv                        (const DocumentEnv&      rhs);
    DocumentEnv&                        operator=                          (const DocumentEnv&      rhs);


    BENTLEYSTM_EXPORT const WString&     GetCurrentDir                      () const;
    BENTLEYSTM_EXPORT const WChar*          GetCurrentDirCStr                  () const;

    // TDORAY: Add a search paths accessors (storing stuff like MS_REFDIR)

    };

END_BENTLEY_MRDTM_NAMESPACE