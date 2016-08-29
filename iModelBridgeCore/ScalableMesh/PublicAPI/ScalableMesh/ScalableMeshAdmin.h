/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshAdmin.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#ifdef VANCOUVER_API
    virtual DgnModelRefP _GetActiveModelRef() const
        {
        return 0;
        }

    virtual StatusInt _ResolveMrDtmFileName(BENTLEY_NAMESPACE_NAME::WString& fileName, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle) const
        {
        return ERROR;
        }       
#endif
};

struct WsgTokenAdmin
    {
    private:
        std::function<Utf8String(void)> m_getToken;

    public:
    WsgTokenAdmin()
        {
        }
    WsgTokenAdmin(std::function<Utf8String(void)> tokenGetter)
        : m_getToken(tokenGetter)
        {
        }
    Utf8String GetToken()
        {
        return m_getToken();
        }
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE

