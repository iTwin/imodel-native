/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_NAMESPACE
struct ElementRefBase;

namespace Ustn {
struct DgnModelRef;
}
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct DGNElementSourceRef;
struct LocalFileSourceRef;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

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

END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE