#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite/BeSQLite.h>
#include <ScalableMesh/Import/DataSQLite.h>
#include "ScalableMeshDb.h"
#include "SMSQLiteFile.h"

USING_NAMESPACE_BENTLEY_SQLITE

USING_NAMESPACE_BENTLEY_SCALABLEMESH


class SMSQLiteClipDefinitionsFile : public SMSQLiteFile
    {
    public:
    virtual void StoreClipPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize, SMClipGeometryType geom = SMClipGeometryType::Polygon, SMNonDestructiveClipType type = SMNonDestructiveClipType::Mask, bool isActive = true);
    virtual void SetClipPolygonMetadata(uint64_t& clipID, double importance, int nDimensions);
    virtual void GetClipPolygonMetadata(uint64_t clipID, double& importance, int& nDimensions);
    virtual void StoreSkirtPolygon(int64_t& clipID, const bvector<uint8_t>& clipData, size_t uncompressedSize);

    virtual void DeleteClipPolygon(int64_t clipID);
    virtual void DeleteSkirtPolygon(int64_t clipID);

    virtual void GetClipPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive);
    virtual void GetSkirtPolygon(int64_t clipID, bvector<uint8_t>& clipData, size_t& uncompressedSize);
    
    virtual void SetClipOnOrOff(uint64_t id, bool isActive);
    virtual void GetIsClipActive(uint64_t id,  bool& isActive);

    virtual void GetClipType(uint64_t id, SMNonDestructiveClipType& type);

    virtual void GetAllClipIDs(bvector<uint64_t>& allIds) override;

    virtual void GetAllCoverageIDs(bvector<uint64_t>& ids) override;

    virtual size_t GetClipPolygonByteCount(int64_t clipID);
    virtual size_t GetSkirtPolygonByteCount(int64_t skirtID);

    virtual void  GetCoverageName(int64_t coverageID, Utf8String* name, size_t& uncompressedSize);
    virtual size_t GetCoverageNameByteCount(int64_t coverageID);
    virtual void GetCoveragePolygon(int64_t coverageID, bvector<uint8_t>& coverageData, size_t& uncompressedSize);
    virtual void StoreCoveragePolygon(int64_t& coverageID, const bvector<uint8_t>& coverageData, size_t uncompressedSize);
    virtual void StoreCoverageName(int64_t& coverageID, Utf8String& coverageName, size_t uncompressedSize);
    virtual size_t GetCoveragePolygonByteCount(int64_t coverageID);
    virtual void DeleteCoveragePolygon(int64_t coverageID);

    virtual void GetAllPolys(bvector<bvector<uint8_t>>& polys, bvector<size_t>& sizes);


    static const BESQL_VERSION_STRUCT CURRENT_VERSION;

    protected:
        virtual BESQL_VERSION_STRUCT GetCurrentVersion() override
            {
            return SMSQLiteClipDefinitionsFile::CURRENT_VERSION;
            }
    virtual DbResult CreateTables() override;

    virtual size_t GetNumberOfReleasedSchemas() override;
    virtual const BESQL_VERSION_STRUCT* GetListOfReleasedVersions() override;
    virtual double* GetExpectedTimesForUpdateFunctions() override;
    virtual std::function<void(BeSQLite::Db*)>* GetFunctionsForAutomaticUpdate() override;

    private:
    };