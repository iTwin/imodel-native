#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite\BeSQLite.h>
#include <ScalableMesh/Import/DataSQLite.h>
#include "ScalableMeshDb.h"
#include "SMSQLiteFile.h"

USING_NAMESPACE_BENTLEY_SQLITE

USING_NAMESPACE_BENTLEY_SCALABLEMESH

class SMSQLiteFeatureFile : public SMSQLiteFile
    {
    public:
        virtual void GetGraph(int64_t nodeID, bvector<uint8_t>& graph, size_t& uncompressedSize);
        virtual void GetFeature(int64_t featureID, bvector<uint8_t>& featureData, size_t& uncompressedSize);

        virtual void StoreGraph(int64_t& nodeID, const bvector<uint8_t>& graph, size_t uncompressedSize);
        virtual void StoreFeature(int64_t& featureID, const bvector<uint8_t>& featureData, size_t uncompressedSize);

        virtual size_t GetNumberOfFeaturePoints(int64_t featureID);

        static const BESQL_VERSION_STRUCT CURRENT_VERSION;

    protected:
        virtual BESQL_VERSION_STRUCT GetCurrentVersion() override
            {
            return SMSQLiteFeatureFile::CURRENT_VERSION;
            }

        virtual DbResult CreateTables() override;

        virtual size_t GetNumberOfReleasedSchemas() override;
        virtual const BESQL_VERSION_STRUCT* GetListOfReleasedVersions() override;
        virtual double* GetExpectedTimesForUpdateFunctions() override;
        virtual std::function<void(BeSQLite::Db*)>* GetFunctionsForAutomaticUpdate() override;

    private:

    };
