#pragma once

#include <ScalableMesh/IScalableMeshMoniker.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
#if 0
class MyMSDocumentMoniker : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMoniker
{
private:

    BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocumentMoniker> m_mrdtmMonikerPtr;

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::LocalFileURL                _GetURL(StatusInt&                          status) const override
    {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));

        if (BSISUCCESS != status)
            fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return s_dgnFile name with configuration variable.

        return BENTLEY_NAMESPACE_NAME::ScalableMesh::LocalFileURL(fileName.c_str());
    }




    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceMonikerType        _GetType() const override
    {
        return BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
    }


    virtual bool                        _IsTargetReachable() const override
    {
        StatusInt status;
        m_mrdtmMonikerPtr->ResolveFileName(&status);
        return BSISUCCESS == status;
    }

    virtual StatusInt                   _Serialize(Import::SourceDataSQLite&                      sourceData,
        const BENTLEY_NAMESPACE_NAME::ScalableMesh::DocumentEnv&                  env) const override
    {
        // TDORAY: Recreate the moniker using new env prior to serializing it in order so
        // that relative path is correct on s_dgnFile moves...

        const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
        sourceData.SetMonikerString(monikerString);
        /*if (!WriteStringW(stream, monikerString.c_str()))
            return BSIERROR;*/

        return BSISUCCESS;
    }


    explicit                            MyMSDocumentMoniker(const DgnDocumentMonikerPtr&         monikerPtr)
        : m_mrdtmMonikerPtr(monikerPtr)
    {
        assert(0 != m_mrdtmMonikerPtr.get());
    }

public:

    static BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
    {
        return new MyMSDocumentMoniker(monikerPtr);
    }
};

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileMonikerCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerCreator
{
private:

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr         _Create(const DgnDocumentMonikerPtr&         msMoniker,
        StatusInt&                          status) const override
    {
        status = BSISUCCESS;
        return MyMSDocumentMoniker::Create(msMoniker);
    }

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr        _Create(const WChar*                      fullPath,
        StatusInt&                          status) const override
    {
        DgnFileStatus openStatus = DGNFILE_STATUS_Success;
        BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocument> docPtr = DgnDocument::CreateFromFileName(openStatus, fullPath, 0, 0, DgnDocument::FetchMode::InfoOnly);

        if (DGNFILE_STATUS_Success != openStatus || 0 == docPtr.get())
        {
            status = BSIERROR;
            return 0;
        }

        status = BSISUCCESS;
        return MyMSDocumentMoniker::Create(docPtr->GetMonikerPtr());
    }
};


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MonikerBinStreamCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerBinStreamCreator
{
private:
    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceMonikerType        _GetSupportedType() const override
    {
        return BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
    }

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerPtr                 _Create(Import::SourceDataSQLite&                      sourceData,
        const BENTLEY_NAMESPACE_NAME::ScalableMesh::DocumentEnv&                  env,
        StatusInt&                          status) const override
    {
        WString monikerString = sourceData.GetMonikerString();
        /*if (!ReadStringW(stream, monikerString))
        {
            status = BSIERROR;
            return 0;
        }*/

        const WChar* basePath = env.GetCurrentDirCStr();

        BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocumentMoniker> documentMonikerPtr
            (
                DgnDocumentMoniker::Create(monikerString.GetWCharCP(),
                    basePath,
                    false)
                );

        if (documentMonikerPtr == 0)
        {
            status = BSIERROR;
            return 0;
        }

        status = BSISUCCESS;

        return MyMSDocumentMoniker::Create(documentMonikerPtr.get());
    }

};


void InitScalableMeshMonikerFactories()
{
    //static const struct MonikerBinStreamCreator s_MonikerBinStreamCreator;
    static const struct LocalFileMonikerCreator s_LocalFileMonikerCreator;
    const BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerFactory::CreatorID localFileCreatorID
        = BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
    assert(localFileCreatorID == &s_LocalFileMonikerCreator);

   // const BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerFactory::BinStreamCreatorID binStreamCreator
   //     = BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
   // assert(binStreamCreator == &s_MonikerBinStreamCreator);
}


namespace {

    /*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    class GeoDtmMSDocumentMoniker : public ILocalFileMoniker
    {
    private:

        DgnDocumentMonikerPtr m_mrdtmMonikerPtr;

        virtual LocalFileURL                _GetURL(StatusInt&                          status) const override
        {
            WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));

            if (BSISUCCESS != status)
                fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return file name with configuration variable.

            return LocalFileURL(fileName.c_str());
        }




        virtual DTMSourceMonikerType        _GetType() const override
        {
            return DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


        virtual bool                        _IsTargetReachable() const override
        {
            StatusInt status;
            m_mrdtmMonikerPtr->ResolveFileName(&status);
            return BSISUCCESS == status;
        }

        virtual StatusInt                   _Serialize(Import::SourceDataSQLite&                      sourceData,
            const DocumentEnv&                  env) const override
        {
            // TDORAY: Recreate the moniker using new env prior to serializing it in order so
            // that relative path is correct on file moves...

            const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
            sourceData.SetMonikerString(monikerString);
            /*if (!WriteStringW(stream, monikerString.c_str()))
                return BSIERROR;*/

            return BSISUCCESS;
        }


        explicit                            GeoDtmMSDocumentMoniker(const DgnDocumentMonikerPtr&         monikerPtr)
            : m_mrdtmMonikerPtr(monikerPtr)
        {
            assert(0 != m_mrdtmMonikerPtr.get());
        }

    public:

        static ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
        {
            return new GeoDtmMSDocumentMoniker(monikerPtr);
        }
    };

    /*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    static const struct LocalFileMonikerCreator : public ILocalFileMonikerCreator
    {
    private:

        virtual ILocalFileMonikerPtr         _Create(const DgnDocumentMonikerPtr&         msMoniker,
            StatusInt&                          status) const override
        {
            status = BSISUCCESS;
            return GeoDtmMSDocumentMoniker::Create(msMoniker);
        }

        virtual ILocalFileMonikerPtr        _Create(const WChar*                      fullPath,
            StatusInt&                          status) const override
        {
            DgnFileStatus openStatus = DGNFILE_STATUS_Success;
            DgnDocumentPtr docPtr = DgnDocument::CreateFromFileName(openStatus, fullPath, 0, 0, DgnDocument::FetchMode::InfoOnly);

            if (DGNFILE_STATUS_Success != openStatus || 0 == docPtr.get())
            {
                status = BSIERROR;
                return 0;
            }

            status = BSISUCCESS;
            return GeoDtmMSDocumentMoniker::Create(docPtr->GetMonikerPtr());
        }
    } s_LocalFileMonikerCreator;


    /*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    static const struct MonikerBinStreamCreator : public IMonikerBinStreamCreator
    {
    private:
        virtual DTMSourceMonikerType        _GetSupportedType() const override
        {
            return DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


        virtual IMonikerPtr                 _Create(Import::SourceDataSQLite&                      sourceData,
            const DocumentEnv&                  env,
            StatusInt&                          status) const override
        {
            WString monikerString = sourceData.GetMonikerString();
            /*if (!ReadStringW(stream, monikerString))
            {
                status = BSIERROR;
                return 0;
            }*/

            const WChar* basePath = env.GetCurrentDirCStr();

            DgnDocumentMonikerPtr documentMonikerPtr
                (
                    DgnDocumentMoniker::Create(monikerString.GetWCharCP(),
                        basePath,
                        false)
                    );

            if (documentMonikerPtr == 0)
            {
                status = BSIERROR;
                return 0;
            }

            status = BSISUCCESS;

            return GeoDtmMSDocumentMoniker::Create(documentMonikerPtr.get());
        }

    } s_MonikerBinStreamCreator;


    void InitMonikerFactories()
    {
        const ILocalFileMonikerFactory::CreatorID localFileCreatorID
            = ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
        assert(localFileCreatorID == &s_LocalFileMonikerCreator);

        const IMonikerFactory::BinStreamCreatorID binStreamCreator
            = IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
        assert(binStreamCreator == &s_MonikerBinStreamCreator);
    }
}


BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocument> docPtr = nullptr;
BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnFile> file = nullptr;
#endif
END_BENTLEY_SCALABLEMESH_NAMESPACE