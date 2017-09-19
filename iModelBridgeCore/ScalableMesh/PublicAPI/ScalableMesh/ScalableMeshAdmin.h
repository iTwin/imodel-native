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

        IScalableMeshTextureGeneratorPtr m_textureGeneratorPtr;

    public : 

        virtual int                     _GetVersion() const {return 1;} // Do not override!
        virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    
        virtual bool _CanImportPODfile() const
            {
            return false;
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

