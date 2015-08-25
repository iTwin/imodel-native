/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshAdmin.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatform.h>
/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ScalableMeshAdmin : DgnHost::IHostObject
{
    virtual int                     _GetVersion() const {return 1;} // Do not override!
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    
    virtual bool _CanImportPODfile() const
        {
        return false;
        }

    virtual DgnModelRefP _GetActiveModelRef() const
        {
        return 0;
        }

    virtual StatusInt _ResolveMrDtmFileName(Bentley::WString& fileName, const Bentley::DgnPlatform::EditElementHandle& elHandle) const
        {
        return ERROR;
        }          
};

END_BENTLEY_SCALABLEMESH_NAMESPACE

