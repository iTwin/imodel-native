#include <ScalableMeshPCH.h>
#include "SMSQLiteDiffsetFile.h"
#include "Stores/SMStoreUtils.h"


#define WSTRING_FROM_CSTR(cstr) WString(cstr, BentleyCharEncoding::Utf8)
#define MAKE_COPY_NO Statement::MakeCopy::No
#define MAKE_COPY_YES Statement::MakeCopy::Yes
#define GET_VALUE_STR(stmt, id) stmt->GetValueText(id)
#define BIND_VALUE_STR(stmt, id, utf8str, copyval) stmt->BindText(id, utf8str, copyval)
#define READONLY Db::OpenMode::Readonly
#define READWRITE Db::OpenMode::ReadWrite


#define AUTO_COMMIT_FREQUENCY 20

const BESQL_VERSION_STRUCT SMSQLiteDiffsetFile::CURRENT_VERSION = BESQL_VERSION_STRUCT(1, 1, 0, 2);

const BESQL_VERSION_STRUCT s_listOfReleasedSchemasDiffset[3] = { BESQL_VERSION_STRUCT(1, 1, 0, 0), BESQL_VERSION_STRUCT(1, 1, 0, 1), BESQL_VERSION_STRUCT(1, 1, 0, 2) };
const size_t s_numberOfReleasedSchemasDiffset = 3;
double s_expectedTimeUpdateDiffset[2] = { 1.2*1e-5, 1.2*1e-5 };
std::function<void(BeSQLite::Db*)> s_databaseUpdateFunctionsDiffset[2] = {
    [](BeSQLite::Db* database)
        {
        database->DropTable("SMNodeHeader");
        database->DropTable("SMTexture");
        database->DropTable("SMPoint");
        database->DropTable("SMUVs");
        database->DropTable("SMFeatures");
        database->DropTable("SMMasterHeader");
        database->DropTable("SMGraph");
        database->DropTable("SMSkirts");
        database->DropTable("SMClipDefinitions");

        },
	[](BeSQLite::Db* database)
		{
			const int maxSize = 2000;

			int offset = 0;
			bool rowsToProcess = true;
			while (rowsToProcess)
			{
				bvector<bvector<uint8_t>> toEnterResults;
				bvector<int64_t> toEnterIds;
				CachedStatementPtr stmt;
				database->GetCachedStatement(stmt, "SELECT Data, length(Data), Size, DiffsetId FROM SMDiffSets LIMIT ? OFFSET ?");
				stmt->BindInt(1, maxSize);
				stmt->BindInt(2, offset);
				DbResult status = stmt->Step();
				int64_t diffsetID;
				bvector<uint8_t> diffsetData;
				size_t uncompressedSize;
				while (status == BE_SQLITE_ROW)
				    {
					++offset;
					diffsetData.resize(stmt->GetValueInt64(1));
					uncompressedSize = stmt->GetValueInt64(2);
					diffsetID = stmt->GetValueInt64(3);
					memcpy(&diffsetData[0], stmt->GetValueBlob(0), diffsetData.size());

					HCDPacket pi_uncompressedPacket, pi_compressedPacket;

					pi_uncompressedPacket.SetBuffer(diffsetData.data(), diffsetData.size() * sizeof(uint8_t));
					pi_uncompressedPacket.SetDataSize(uncompressedSize);
					WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);

					bvector<uint8_t> data;
					data.resize(pi_compressedPacket.GetDataSize());
					memcpy(&data[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
					toEnterResults.push_back(data);
					toEnterIds.push_back(diffsetID);

					status = stmt->Step();
				    }
				if (offset < maxSize) rowsToProcess = false;

				for (size_t i = 0; i < toEnterIds.size(); ++i)
				   {
					CachedStatementPtr stmt2;
					database->GetCachedStatement(stmt2, "UPDATE SMDiffSets SET Data=? WHERE DiffsetId=?");
					stmt2->BindBlob(1, &toEnterResults[i][0], (int)toEnterResults[i].size(), MAKE_COPY_NO);
					stmt2->BindInt64(2, toEnterIds[i]);
					DbResult statusInsert = stmt2->Step();
					assert(statusInsert == BE_SQLITE_DONE);
				   }
			}
		}
    };


SMSQLiteDiffsetFile::SMSQLiteDiffsetFile()
    {

    }

SMSQLiteDiffsetFile::~SMSQLiteDiffsetFile()
    {
    

    }



size_t SMSQLiteDiffsetFile::GetNumberOfReleasedSchemas() { return s_numberOfReleasedSchemasDiffset; }
const BESQL_VERSION_STRUCT* SMSQLiteDiffsetFile::GetListOfReleasedVersions() { return s_listOfReleasedSchemasDiffset; }
double* SMSQLiteDiffsetFile::GetExpectedTimesForUpdateFunctions() { return s_expectedTimeUpdateDiffset; }
std::function<void(BeSQLite::Db*)>* SMSQLiteDiffsetFile::GetFunctionsForAutomaticUpdate() { return s_databaseUpdateFunctionsDiffset; }

DbResult SMSQLiteDiffsetFile::CreateTables()
    {
    DbResult result;


    result = m_database->CreateTable("SMDiffSets", "DiffsetId INTEGER PRIMARY KEY,"
                                     "Data BLOB,"
                                     "Size INTEGER");

    assert(result == BE_SQLITE_OK);
    return result;
    }

void SMSQLiteDiffsetFile::StoreDiffSet(int64_t& diffsetID, const bvector<uint8_t>& diffsetData, size_t uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    size_t nRows = 0;
    if (diffsetID != SQLiteNodeHeader::NO_NODEID)
        {
        CachedStatementPtr stmt3;
        m_database->GetCachedStatement(stmt3, "SELECT COUNT(DiffsetId) FROM SMDiffSets WHERE DiffsetId=?");
        stmt3->BindInt64(1, diffsetID);
        stmt3->Step();
        nRows = stmt3->GetValueInt64(0);
        }
    if (diffsetID == SQLiteNodeHeader::NO_NODEID)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMDiffSets (Data,Size) VALUES(?,?)");
        stmt->BindBlob(1, &diffsetData[0], (int)diffsetData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();
        CachedStatementPtr stmt2;
        m_database->GetCachedStatement(stmt2, "SELECT last_insert_rowid()");
        status = stmt2->Step();
        diffsetID = stmt2->GetValueInt64(0);
        if (m_autocommit) 
            { 
            m_nbAutoCommitDone++;

            if (m_nbAutoCommitDone == AUTO_COMMIT_FREQUENCY)            
                { 
                m_database->SaveChanges();
                m_nbAutoCommitDone = 0;
                }
            }
        }
    else if (nRows == 0)
        {
        Savepoint insertTransaction(*m_database, "insert");
        m_database->GetCachedStatement(stmt, "INSERT INTO SMDiffSets (DiffsetId, Data,Size) VALUES(?, ?,?)");
        stmt->BindInt64(1, diffsetID);
        stmt->BindBlob(2, &diffsetData[0], (int)diffsetData.size(), MAKE_COPY_NO);
        stmt->BindInt64(3, uncompressedSize);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();

        if (m_autocommit)
            {
            m_nbAutoCommitDone++;

            if (m_nbAutoCommitDone == AUTO_COMMIT_FREQUENCY)
                {
                m_database->SaveChanges();
                m_nbAutoCommitDone = 0;
                }
            }        
        }
    else
        {
        m_database->GetCachedStatement(stmt, "UPDATE SMDiffSets SET Data=?, Size=? WHERE DiffsetId=?");
        stmt->BindBlob(1, &diffsetData[0], (int)diffsetData.size(), MAKE_COPY_NO);
        stmt->BindInt64(2, uncompressedSize);
        stmt->BindInt64(3, diffsetID);
        DbResult status = stmt->Step();
        assert(status == BE_SQLITE_DONE);
        stmt->ClearBindings();

        if (m_autocommit)
            {
            m_nbAutoCommitDone++;

            if (m_nbAutoCommitDone == AUTO_COMMIT_FREQUENCY)
                {
                m_database->SaveChanges();
                m_nbAutoCommitDone = 0;
                }
            }    
        }
    }

void SMSQLiteDiffsetFile::GetDiffSet(int64_t diffsetID, bvector<uint8_t>& diffsetData, size_t& uncompressedSize)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "SELECT Data, length(Data), Size FROM SMDiffSets WHERE DiffsetId=?");
    stmt->BindInt64(1, diffsetID);
    DbResult status = stmt->Step();
    // assert(status == BE_SQLITE_ROW);
    if (status == BE_SQLITE_DONE)
        {
        uncompressedSize = 0;
        return;
        }
    diffsetData.resize(stmt->GetValueInt64(1));
    uncompressedSize = stmt->GetValueInt64(2);
    memcpy(&diffsetData[0], stmt->GetValueBlob(0), diffsetData.size());
    }

void SMSQLiteDiffsetFile::DeleteDiffSet(int64_t diffsetID)
    {
    std::lock_guard<std::mutex> lock(dbLock);
    CachedStatementPtr stmt;
    m_database->GetCachedStatement(stmt, "DELETE FROM SMDiffSets WHERE DiffsetId=?");
    stmt->BindInt64(1, diffsetID);
    stmt->Step();
    }

