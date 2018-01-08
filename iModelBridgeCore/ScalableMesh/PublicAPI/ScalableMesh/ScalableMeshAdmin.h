/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/ScalableMeshAdmin.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatform.h>
#include <ScalableMesh\IScalableMeshTextureGenerator.h>

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ScalableMeshAdmin : DgnHost::IHostObject
{
    private :         

        IScalableMeshTextureGeneratorPtr m_textureGeneratorPtr;

    public : 
       
        struct ProxyInfo
            {
            Utf8String m_user;
            Utf8String m_password;
            Utf8String m_serverUrl;
            };

        virtual int                     _GetVersion() const {return 1;} // Do not override!
        virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    
        virtual bool _CanImportPODfile() const
            {
            return false;
            }

        virtual bool _ProvideImageppAuthentication() const
            {
            return true;
            }

        virtual IScalableMeshTextureGeneratorPtr _GetTextureGenerator () 
            {
            return m_textureGeneratorPtr;            
            }        

        virtual void _SetTextureGenerator(IScalableMeshTextureGeneratorPtr& textureGenerator)
            { 
            m_textureGeneratorPtr = textureGenerator;        
            }

        virtual Utf8String _GetProjectID() const { return Utf8String(); }

        virtual ProxyInfo _GetProxyInfo() const { return ProxyInfo(); }

        virtual uint64_t  _GetProductId() const { return UINT64_MAX; }

    #ifdef VANCOUVER_API
        virtual DgnModelRefP _GetActiveModelRef() const
            {
            return 0;
            }

		//virtual StatusInt _ResolveMrDtmFileName(BENTLEY_NAMESPACE_NAME::WString& fileName, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle) const;
    #endif
};

#ifdef VANCOUVER_API
struct STMAdmin
{
private:
	std::function<StatusInt(BENTLEY_NAMESPACE_NAME::WString&, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle&)> m_resolveFileName;

public:
	STMAdmin() 
	    {
		m_resolveFileName = [](BENTLEY_NAMESPACE_NAME::WString&, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle&)
		    {
			return ERROR;
		};
	    }
	STMAdmin(std::function<StatusInt(BENTLEY_NAMESPACE_NAME::WString&, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle&)> NameResolver)
		: m_resolveFileName(NameResolver)
	{}

	STMAdmin(const STMAdmin& myAdmin)
	    {
		m_resolveFileName = myAdmin.m_resolveFileName;
	    }

	StatusInt _ResolveMrDtmFileName(BENTLEY_NAMESPACE_NAME::WString& fileName, const BENTLEY_NAMESPACE_NAME::DgnPlatform::EditElementHandle& elHandle)
	{
		return m_resolveFileName(fileName, elHandle);
	}
};

#endif

END_BENTLEY_SCALABLEMESH_NAMESPACE

