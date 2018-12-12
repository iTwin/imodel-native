/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportConfig.cpp $
|    $RCSfile: ImportConfig.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/07/20 20:22:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/Import/ImportConfig.h>
#include "InternalImporterConfig.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig::ImportConfig ()
    //:   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfig::~ImportConfig ()
    {
    }


RefCountedPtr<ImportConfig> ImportConfig::Create()
    {
    return new Internal::Config();
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
