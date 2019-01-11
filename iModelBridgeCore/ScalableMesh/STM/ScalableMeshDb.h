#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite/BeSQLite.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/Import/DataSQLite.h>


enum class SQLDatabaseType
    {
    SM_MAIN_DB_FILE,
    SM_CLIP_DEF_FILE,
    SM_DIFFSETS_FILE,
    SM_GENERATION_FILE
    };



class SMSQLiteFile;

USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#ifdef VANCOUVER_API
//=======================================================================================
//! A 4-digit number that specifies the version of the "schema" of a Db
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct SchemaVersion
{
private:
    UInt16 m_major;
    UInt16 m_minor;
    UInt16 m_sub1;
    UInt16 m_sub2;

public:
    enum Mask ENUM_UNDERLYING_TYPE (UInt64)
        {
        VERSION_Major      = (((UInt64) 0xffff) << 48),
        VERSION_Minor      = (((UInt64) 0xffff) << 32),
        VERSION_Sub1       = (((UInt64) 0xffff) << 16),
        VERSION_Sub2       = ((UInt64) 0xffff),
        VERSION_MajorMinor = (VERSION_Major | VERSION_Minor),
        VERSION_All        = (VERSION_Major | VERSION_Minor | VERSION_Sub1 | VERSION_Sub2),
        };

    SchemaVersion (UInt16 major, UInt16 minor, UInt16 sub1, UInt16 sub2) {m_major=major; m_minor=minor; m_sub1=sub1; m_sub2=sub2;}
    explicit SchemaVersion(Utf8CP json) {FromJson(json);}

    //! Get the Major version. Typically indicates a software "generation".
    //! When this number changes, older versions will no longer open the database.
    UInt16 GetMajor() const {return m_major;}

    //! Get the Minor version. Typically indicates a software release cycle.
    //! When this number changes, older versions will no longer open the database.
    UInt16 GetMinor() const {return m_minor;}

    //! Get the Sub1 version. Typically means small backwards compatible changes to the schema within a release cycle.
    //! When this number changes, older versions will open the database readonly.
    UInt16 GetSub1() const {return m_sub1;}

    //! Get the Sub2 version. Changes to this number mean new features were added, but older versions may continue to edit
    UInt16 GetSub2() const {return m_sub2;}

    UInt64 GetInt64(Mask mask) const {return mask & ((((UInt64) m_major)<<48) | (((UInt64)m_minor)<<32) | ((UInt64)m_sub1<<16) | (UInt64)m_sub2);}
    int CompareTo(SchemaVersion other, Mask mask) const {return (GetInt64(mask)==other.GetInt64(mask)) ? 0 : (GetInt64(mask) > other.GetInt64(mask) ? 1 : -1);}

    Utf8String ToJson() const;
    void FromJson(Utf8CP);
};
#endif

//#ifdef VANCOUVER_API
//    #define BESQL_VERSION_STRUCT SchemaVersion
//#else
    #define BESQL_VERSION_STRUCT ProfileVersion    
//#endif


class InfiniteRetries : public BeSQLite::BusyRetry
{
	virtual int _OnBusy(int count) const;
};

class ScalableMeshDb : public BeSQLite::Db
    {
    private:
        SQLDatabaseType m_type;
        SMSQLiteFile*   m_smFile; 
        BESQL_VERSION_STRUCT GetCurrentVersion() const;
        Savepoint* m_currentSavepoint;

#ifndef VANCOUVER_API
        BENTLEY_NAMESPACE_NAME::Utf8String m_path;
#endif

        static bool s_enableSharedDatabase;

    protected:
#ifndef VANCOUVER_API    
       ProfileState _CheckProfileVersion() const override;

       virtual DbResult _UpgradeProfile() override;
#endif

    virtual DbResult _OnDbCreated(CreateParams const& params) override;

    public:

        ScalableMeshDb(SQLDatabaseType type, SMSQLiteFile* smFile) : m_type(type), m_smFile(smFile), m_currentSavepoint(nullptr){}

#ifndef VANCOUVER_API   
        //Offer functions to more explicitly deal with shared db's; open/close after specific transactions, allow implicit transactions, busy-retry

        DbResult OpenShared(BENTLEY_NAMESPACE_NAME::Utf8CP path, bool readonly, bool allowBusyRetry);
        BENTLEY_SM_EXPORT bool ReOpenShared(bool readonly, bool allowBusyRetry);
        BENTLEY_SM_EXPORT bool StartTransaction();
        BENTLEY_SM_EXPORT bool CommitTransaction();
        BENTLEY_SM_EXPORT void CloseShared(bool& wasTransactionAbandoned);

        void GetSharedDbFileName(BENTLEY_NAMESPACE_NAME::Utf8String& path);        
#endif

#ifdef VANCOUVER_API
        DbResult IsProfileVersionUpToDate(OpenParams const& params);
#endif

        static const BESQL_VERSION_STRUCT CURRENT_VERSION;

        BENTLEY_SM_EXPORT static bool GetEnableSharedDatabase();

        BENTLEY_SM_EXPORT static void  SetEnableSharedDatabase(bool isShared);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE