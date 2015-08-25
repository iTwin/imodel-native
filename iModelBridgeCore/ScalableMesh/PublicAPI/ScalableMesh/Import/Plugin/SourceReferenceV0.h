/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/SourceReferenceV0.h $
|    $RCSfile: SourceReferenceV0.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/08/10 17:03:32 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_NAMESPACE
struct ElementRefBase;

namespace Ustn {
struct DgnModelRef;
}
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct DGNElementSourceRef;
struct LocalFileSourceRef;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElementSourceRefBase : private Unassignable
    {
private:
    friend struct                       DGNElementSourceRef;

    virtual DGNElementSourceRefBase*    _Clone                                 () const = 0;

    virtual UInt                        _GetElementType                        () const = 0;
    virtual UInt                        _GetElementHandlerID                   () const = 0;

    virtual ElementReferenceP           _GetElementRef                         () const = 0;

    virtual DgnModelReferenceP          _GetModelRef                           () const = 0;

    virtual const LocalFileSourceRef*   _GetLocalFileP                         () const = 0;

protected:
    typedef LocalFileSourceRef          LocalFileSourceRef;
public:
    virtual                             ~DGNElementSourceRefBase               () = 0 {}
    };

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
