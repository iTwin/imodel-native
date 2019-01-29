#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite/BeSQLite.h>
#include <ScalableMesh/Import/DataSQLite.h>
#include "ScalableMeshDb.h"
#include "SMSQLiteFile.h"

USING_NAMESPACE_BENTLEY_SQLITE

USING_NAMESPACE_BENTLEY_SCALABLEMESH

class SMSQLiteDiffsetFile : public SMSQLiteFile
    {
    public:

             SMSQLiteDiffsetFile();

    virtual ~SMSQLiteDiffsetFile();

    virtual void GetDiffSet(int64_t diffsetID, bvector<uint8_t>& diffsetData, size_t& uncompressedSize);
    virtual void StoreDiffSet(int64_t& diffsetID, const bvector<uint8_t>& diffsetData, size_t uncompressedSize);
    virtual void DeleteDiffSet(int64_t diffsetID);

    static const BESQL_VERSION_STRUCT CURRENT_VERSION;

    protected:
        virtual BESQL_VERSION_STRUCT GetCurrentVersion() override
            {
            return SMSQLiteDiffsetFile::CURRENT_VERSION;
            }

    virtual DbResult CreateTables() override;

    virtual size_t GetNumberOfReleasedSchemas() override;
    virtual const BESQL_VERSION_STRUCT* GetListOfReleasedVersions() override;
    virtual double* GetExpectedTimesForUpdateFunctions() override;
    virtual std::function<void(BeSQLite::Db*)>* GetFunctionsForAutomaticUpdate() override;

    private:

        int  m_nbAutoCommitDone = 0;

    };