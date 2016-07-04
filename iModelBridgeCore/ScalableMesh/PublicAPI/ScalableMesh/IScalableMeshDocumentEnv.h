/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshDocumentEnv.h $
|    $RCSfile: IScalableMeshDocumentEnv.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 15:00:33 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    explicit                            DocumentEnv                        (const WChar*          currentDir);

                                        ~DocumentEnv                       ();

                                        DocumentEnv                        (const DocumentEnv&      rhs);
    DocumentEnv&                        operator=                          (const DocumentEnv&      rhs);


    BENTLEY_SM_EXPORT const WString&     GetCurrentDir                      () const;
    BENTLEY_SM_EXPORT const WChar*          GetCurrentDirCStr                  () const;

    // TDORAY: Add a search paths accessors (storing stuff like MS_REFDIR)

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
