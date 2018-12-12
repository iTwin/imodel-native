/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Error/Source.h $
|    $RCSfile: Source.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/26 18:47:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Exceptions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/* Status */


/* Exceptions */
struct FileIOException : public ExceptionMixinBase<FileIOException>
    {
    explicit                        FileIOException                    ()                 
        :   super_class(L"FileIO error!") 
        {
        }
    explicit                        FileIOException                    (StatusInt code)   
        :   super_class(L"FileIO error!", code) 
        {
        }
    };

struct SourceNotFoundException : public ExceptionMixinBase<SourceNotFoundException>
    {
    explicit                        SourceNotFoundException            ()                 
        :   super_class(L"Source not found error!") 
        {
        }
    explicit                        SourceNotFoundException            (StatusInt code)   
        :   super_class(L"Source not found error!", code) 
        {
        }
    };

struct PluginNotFoundException : public ExceptionMixinBase<PluginNotFoundException>
    {
    explicit                        PluginNotFoundException            ()                 
        :   super_class(L"Source not supported error!") 
        {
        }
    explicit                        PluginNotFoundException            (StatusInt code)   
        :   super_class(L"Source not supported error!", code) 
        {
        }
    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
