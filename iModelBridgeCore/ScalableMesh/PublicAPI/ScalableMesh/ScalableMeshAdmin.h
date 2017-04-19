/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatform.h>
#include <ScalableMesh\IScalableMeshTextureGenerator.h>
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
    private :         

    public : 

        virtual int                     _GetVersion() const {return 1;} // Do not override!
        virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    
        virtual bool _CanImportPODfile() const
            {
            return false;
            }

        virtual IScalableMeshTextureGeneratorPtr _GetTextureGenerator () 
            {
            IScalableMeshTextureGeneratorPtr textureGeneratorPtr;

            return textureGeneratorPtr;
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
    WsgTokenAdmin() {}
    WsgTokenAdmin(std::function<Utf8String(void)> tokenGetter)
        : m_getToken(tokenGetter)
        {}
    Utf8String GetToken()
        {
        return m_getToken();
        }
    };

struct SASTokenAdmin
    {
    private:
        typedef std::function<Utf8String(const Utf8String& guid)> TokenFromGUID;
        TokenFromGUID m_getToken;

    public:
        SASTokenAdmin() {}
        SASTokenAdmin(TokenFromGUID tokenGetter)
            : m_getToken(tokenGetter)
            {}
        Utf8String GetToken(const Utf8String& guid)
            {
            return m_getToken(guid);
            }
    };

struct SSLCertificateAdmin
    {
    private:
        std::function<Utf8String(void)> m_getSSLCertificatePath;

    public:
        SSLCertificateAdmin() {}
        SSLCertificateAdmin(std::function<Utf8String(void)> SSLCertificatePathGetter)
            : m_getSSLCertificatePath(SSLCertificatePathGetter)
            {}
        Utf8String GetSSLCertificatePath()
            {
            return m_getSSLCertificatePath();
            }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE

