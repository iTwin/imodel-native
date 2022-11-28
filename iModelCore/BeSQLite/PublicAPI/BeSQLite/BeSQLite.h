/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <cstddef>
#include <Bentley/Bentley.h>
#include <Bentley/bset.h>
#include <Bentley/BeId.h>
#include <Bentley/BeEvent.h>
#include <json/BeJsValue.h>
#include <list>
#include <type_traits>
#include <functional>
#include <chrono>
#include <Bentley/Logging.h>

/** @namespace BentleyApi::BeSQLite Classes used to access a SQLite database. */

/****
@page BeSQLiteOverview BeSQLite Overview

BeSQLite is a Bentley-authored layer to leverage and extend the unique features of SQLite
(http://www.sqlite.org/index.html). SQLite is a full-featured SQL database engine that can be embedded inside an
application. Unlike enterprise SQL database systems, SQLite has no client/server architecture, and a "SQLte Database" is
a disk file. This makes SQLite a wonderful solution for fast and flexible access to locally stored data, but
intrinsically limits its relevance for multi-user scenarios. In fact, there are very few scenarios where SQLite and
enterprise-level databases are interchangeable. Despite the fact that SQLite supports multiple connections to the same
database file from separate processes, BeSQLite is explicitly designed for use cases where the file is stored on a local
disk and is only accessed by a single process from a single connection.

It is important to recognize that while BeSQLite extends the capabilities of SQLite, <em>it is a strict superset of
SQLite</em> and a BeSQLite database file <b>is a</b> SQLite file. Therefore, all SQLite-based utilities and tools will
work on a BeSQLite file. Further, the SQL grammar, rules, and idiosyncrasies of SQLite apply equally to BeSQLite. To
find the answer to specific SQL-related questions, please consult the SQLite website at http://sqlite.org/lang.html. All
SQLite documentation is considered "incorporated by reference" in the BeSQLite documentation and is not repeated here.
Exceptions are clearly noted.

@section OVRBeSQLiteTransactions 1. Transactions in BeSQLite

BeSQLite's transactions are based on SQLite's transactions, and inherit the same guarantees of ACID compliance (see
http://sqlite.org/transactional.html). All changes to persistent data are made in the context of a SQLite transaction
and are not saved to the database unless and until committed. However, to help coordinate the synchronization of in-memory
application data with related persistent data, BeSQLite wraps the SQL- based transaction system with the SavePoint api.
Further, SQLite supports a concept called "implicit transactions" that is almost never desirable for a BeSQLite
application. Therefore, BeSQLite will start a "default transaction" unless explicitly directed otherwise. When
the default transaction is committed, it is automatically restarted.

<b>For this reason, it is important that you do not use any of the SQLite Transaction SQL statements BEGIN, END,
COMMIT, or ROLLBACK (http://www.sqlite.org/lang_transaction.html) directly in your code.</b> Doing so will cause
unpredictable results. Instead, use the methods #BeSQlite::Db::SaveChanges and #BeSQlite::Db::AbandonChanges.

Generally, the methods in BeSQLite do not do transaction management themselves, leaving the control of transaction to
the application. So, if you call method that changes persistent data, it may potentially make partial changes to the
data before encountering an error. When this happens, the active transaction can potentially hold inconsistent data
from the perspective of your application. When this happens, you may choose to abandon the entire transaction as appropriate
(note: changes are never permanently saved to disk until you call SaveChanges.)

@section OVRBeSQLiteRepositories 2. Repositories in BeSQLite

Many layers that use BeSQLite are designed to work with a single process with a single (exclusive) connection to a
database  file. That means that in a multi-user scenario, every user must have their own @i private copy of the
database. We refer to each user's copy of the database as a "briefcase" (following the convention of Mercurial and Git
for Distributed Revision Control, see http://en.wikipedia.org/wiki/Distributed_revision_control.) Each briefcase stores
its own 4-byte identifier, referred to as its @c BeBriefcaseId, that distinguishes it from all other repositories.
Changes made in each briefcase can then be merged together in an orderly fashion by coordinating with a team server.
Team servers are also responsible for assigning and tracking BeBriefcaseIds.

@section OVRBeSQLiteIds 3. Ids in BeSQLite

Since each user has a local copy of the briefcase, Ids in BeSQLite must be carefully managed. There are several
approaches:

    -# Use Globally Unique Identifiers (GUID, See #BeGuid). A globally unique identifier is a 16 byte randomly assigned
value. 16 bytes hold enough data that there is virtually no chance of collision. Unfortunately, 16 bytes will not fit
into registers of current generation computers, making them relatively expensive to handle programmatically. For large
tables, GUIDs can add significant overhead for lookups and are much more expensive than 8-byte values. This usually
makes them a <i>poor choice for primary keys.</i> However, as the name implies, when you use a GUID, you never need to
worry about uniqueness.

    -# Use #BeBriefcaseBasedIds. A BeBriefcaseBasedId is an 8-byte number that is designed to be unique by combining the
4-byte BeBriefcaseId with a unique-within-the-briefcase 4-byte value. This limits the number of potential values
within a BeBriefcase to 2^32 (4 billion). Of course it permits 4 billion different BeRepositories. It is often the best
compromise of performance and flexibility for tables with lots of activity.

    -# Use #BeServerIssuedId. A BeServerIssuedId is an 8-byte number that is unique because it is issued by a
synchronized id-administration server, enforced outside of the scope of BeSQLite. BeServerIssuedIds therefore cannot be
created locally and require a connection to the (one, globally administered) server to obtain a new value. Generally
they are useful for "rarely changed but often used" ids such as styles and fonts, etc.

@section OVRBeSQLiteProperties 4. Properties

Every BeSQlite database has a table named "be_Prop" that can be used to save name-id-value triplets, referred to as
Properties. Properties are often used for storing ad-hoc values that don't warrant the creation of a specific
class/table. The "name" for a Property is defined by a #PropertySpec that includes a two-part namespace/name to uniquely
identify the property. The id for a Property is also a two-part majorId/subId pair to permit arrays.

@section OVRBeSQLiteProperties 5. Briefcase Local Values

Every BeSQLite database has a table named "be_local" that can be used to save name-value pairs that are specific to the
BeBriefcase and not merged with the team server. They are used to keep track of the state of the local file and are not
considered part of the "real" database.

@section OVRBeSQLiteEmbeddedFiles 6. Embedded Files

Every BeSQLite database has a table named "be_EmbedFile" that holds copies of files that are "embedded in" the database.
These files are stored as blobs, and are not directly accessible by external applications. Instead, BeSQLite provides
methods to extract them into temporary locations.

@section OVRBeSQLiteLanguageSupport 7. Support for language-specific collation and case-folding

By default, BeSQLite does not support language-specific collation, and performs case-folding only for the ASCII
character set. However, applications can extend BeSQLite by implementing the #BeSQLiteLib::ILanguageSupport interface.

*/

#ifdef __BE_SQLITE_HOST_DLL__
    #define BE_SQLITE_EXPORT EXPORT_ATTRIBUTE
#else
#define BE_SQLITE_EXPORT IMPORT_ATTRIBUTE
#endif


#define TEMP_TABLE_Prefix "temp."
#define TEMP_TABLE(name) TEMP_TABLE_Prefix name

// the "unique temporary table" macros can be used for temporary tables that shadow a real table, but use a unique name.
#define TEMP_TABLE_UniquePrefix TEMP_TABLE_Prefix "t_"
#define TEMP_TABLE_UNIQUE(name) TEMP_TABLE_UniquePrefix name

#define BEDB_TABLE_Local        "be_Local"
#define BEDB_TABLE_Property     "be_Prop"
#define BEDB_TABLE_EmbeddedFile "be_EmbedFile"
#define BEDB_MemoryDb           ":memory:"

#define BEGIN_BENTLEY_SQLITE_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace BeSQLite {
#define END_BENTLEY_SQLITE_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SQLITE using namespace BentleyApi::BeSQLite;

#include <Bentley/BeFile.h>
#include <Bentley/BeThread.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/RefCounted.h>
#include <Bentley/DateTime.h>
#include <Bentley/BeVersion.h>

#include <Bentley/BeAssert.h>
#include <zlib/zlib.h>

// this is used to quiet compiler warnings for variables only used in asserts
#define UNUSED_VARIABLE(x) (void)(x)

#define BESQLITE_TYPEDEFS(_name_) BEGIN_BENTLEY_SQLITE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_SQLITE_NAMESPACE

BESQLITE_TYPEDEFS(BeGuid);
BESQLITE_TYPEDEFS(Db);
BESQLITE_TYPEDEFS(DbFile);
BESQLITE_TYPEDEFS(Statement);
BESQLITE_TYPEDEFS(NamedParams);

#if !defined (DOCUMENTATION_GENERATOR)
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    uint32_t Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif

typedef struct sqlite3_blob* SqlDbBlobP;
typedef struct sqlite3* SqlDbP;
typedef struct sqlite3& SqlDbR;
typedef struct sqlite3_stmt* SqlStatementP;
typedef struct sqlite3_session* SqlSessionP;
typedef struct sqlite3_changeset_iter* SqlChangesetIterP;
typedef struct sqlite3_value* SqlValueP;

#endif // DOCUMENTATION_GENERATOR

BEGIN_BENTLEY_SQLITE_NAMESPACE

namespace MemorySize {
    enum : uint64_t {
        K = 1024,
        MEG = K * K,
        GIG = K * MEG,
        T = K * GIG,
    };
};

typedef struct CloudContainer* CloudContainerP;

//=======================================================================================
//! A 16-byte Globally Unique Id. A value of all zeros means "Invalid Id".
// @bsiclass
//=======================================================================================
struct BeGuid
{
    union{struct _GUID g; uint64_t u[2]; uint32_t i[4]; uint8_t b[16];} m_guid;

    //! Construct a new BeGuid. Optionally, initialize to a new unique value.
    //! @param[in] createValue if true, the BeGuid will hold a new globally unique value, otherwise the value is invalid (all zeros)
    //! @note The default constructor initializes the BeGuid to be invalid.
    explicit BeGuid(bool createValue=false) {if (createValue) Create(); else Invalidate();}

    //! Construct and initialize a BeGuid from two 64 bit values. Caller must ensure these values constitute a valid BeGuid.
    BeGuid(uint64_t u0, uint64_t u1) {m_guid.u[0]=u0; m_guid.u[1]=u1;}

    //! Construct and initialize a BeGuid from four 32 bit values. Caller must ensure these values constitute a valid BeGuid.
    BeGuid(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) {m_guid.i[0]=i0; m_guid.i[1]=i1; m_guid.i[2]=i2; m_guid.i[3]=i3;}

    //! Compare two BeGuids for equality
    bool operator==(BeGuid const& rhs) const {return rhs.m_guid.u[0]==m_guid.u[0] && rhs.m_guid.u[1]==m_guid.u[1];}

    //! Compare two BeGuids for inequality
    bool operator!=(BeGuid const& rhs) const {return !(*this==rhs);}

    //! Compare two BeGuids
    bool operator<(BeGuid const& rhs) const {return m_guid.u[0] < rhs.m_guid.u[0] || (m_guid.u[0] == rhs.m_guid.u[0] && m_guid.u[1] < rhs.m_guid.u[1]);}

    //! Set this BeGuid to the invalid id value (all zeros).
    void Invalidate() {m_guid.u[0] = m_guid.u[1] = 0;}

    //! Test to see whether this BeGuid is non-zero
    bool IsValid() const {return 0!=m_guid.u[0] && 0!=m_guid.u[1];}

    //! Assign a new random (Version 4, see http://en.wikipedia.org/wiki/Universally_unique_identifier) id for this BeGuid. Old value is overwritten.
    //! @note This method requires that the BeSQLiteLib::Initialize be called prior to use.
    BE_SQLITE_EXPORT void Create();

    //! Convert this BeGuid to a string
    //! @return The formatted value in 8-4-4-4-12 format
    BE_SQLITE_EXPORT Utf8String ToString() const;

    //! Initialize this BuGuid from a previously saved string.
    //! @param[in] str The string holding the value. Must be in the following format: 8-4-4-4-12
    //! @return non-zero error status if \a str is not a valid BeGuid string.
    BE_SQLITE_EXPORT BentleyStatus FromString(Utf8CP str);
};

//=======================================================================================
//! A unique Id for a BeBriefcase (a particular copy of a BeSQLite::Db is referred to as a BeBriefcase.)
//! The Id is assigned by iModelHub or is one of the special values that identify the special kinds of iModels that are not synchronized with iModelHub.
//! Whenever more than one BeBriefcase of a the same Db exists, each of them must have a unique identifier to facilitate
//! change merging via BeBriefcaseBasedId's.
//! <p> This strategy relies on of uniqueness of BeBriefcaseId's, but that must be enforced by infrastructure outside of BeSQLite.
// @bsiclass
//=======================================================================================
struct BeBriefcaseId
{
protected:
    uint32_t m_id;

public:
    //! BeBriefcaseIds must be less than this value
    static uint32_t const MaxRepo() {return 1L<<24;}
    //! A standalone Db. These files are not mergeable, and cannot generate changesets.
    static uint32_t const Standalone() {return 0;} // was previously called "master"
    static uint32_t const DeprecatedStandalone() {return 1;} // Note: this value was previously used for "standalone",
    //! the first valid briefcaseId
    static uint32_t const FirstValidBriefcaseId() {return 2;}
    //! the last valid briefcaseId
    // NOTE: The 10 largest valid BeBriefcaseIds will not be assigned by iModelHub, so are available to identify special kinds of iModels.
    static uint32_t const LastValidBriefcaseId() {return MaxRepo() - 10;}
    //! An illegal value
    static uint32_t const Illegal() {return (uint32_t)0xffffffff;}

    BeBriefcaseId GetNextBriefcaseId() const {return BeBriefcaseId(m_id+1);}
    BeBriefcaseId() {Invalidate();}             //!< Construct an invalid BeBriefcaseId.
    //! Construct a BeBriefcaseId from a 32 bit value. Sets to Invalid if value is greater than MaxRepo.
    explicit BeBriefcaseId(uint32_t u) {m_id = (u >= MaxRepo()) ? Illegal() : u;}
    void Invalidate() {m_id = Illegal();}  //!< Set this BeBriefcaseId to the invalid id value
    bool IsValid() const {return Illegal() != m_id;}  //!< Test to see whether this BriefcaseId is valid.
    bool IsBriefcase() const {return m_id >= FirstValidBriefcaseId() && m_id <=LastValidBriefcaseId();}
    bool IsStandalone() const {return m_id == Standalone() || m_id == DeprecatedStandalone();} //!< Determine whether this is a standalone iModel.
    uint32_t GetValue() const {BeAssert(IsValid()); BeAssert(m_id<MaxRepo()); return m_id;} //!< Get the briefcase id as a uint32_t
    bool operator==(BeBriefcaseId const& rhs) const {return rhs.m_id==m_id;}
    bool operator!=(BeBriefcaseId const& rhs) const {return !(*this == rhs);}
};

//=======================================================================================
//! A 8-byte value that is locally unique within a BeBriefcase. Since BeBriefcaseId's are forced to be unique externally, a BeBriefcaseBasedId
//! can be assumed to be unique for a given DgnDb. This provides a more efficient strategy for id values than using true 128-bit GUIDs.
// @bsiclass
//=======================================================================================
struct BeBriefcaseBasedId : BeInt64Id
{
    BEINT64_ID_DECLARE_MEMBERS(BeBriefcaseBasedId,BeInt64Id)

public:
    //!Top 24 bits are BeBriefcaseId.
    //!Lower 40 bits are local id
    static uint64_t const MaxLocal() {return 1LL<<40;}
    static uint64_t const LocalMask() {return ~-(int64_t)MaxLocal();}

    //! CONSTRUCT a BeInt64Id from a BriefcaseId value and an id.
    BeBriefcaseBasedId(BeBriefcaseId briefcaseId, uint64_t id) {BeAssert(id<MaxLocal()); m_id = ((briefcaseId.GetValue() * MaxLocal()) + id);}

    BeBriefcaseId GetBriefcaseId() const {return BeBriefcaseId((uint32_t) (m_id / MaxLocal()));} //!< Get the BeBriefcaseId of this BeBriefcaseBasedId
    uint64_t GetLocalId() const {return m_id & LocalMask();} //!< Get the local id

    //! Increment this BeBriefcaseBasedId
    //! @note If this BeBriefcaseBasedId is not valid, this method does nothing.
    BE_SQLITE_EXPORT void UseNext(Db&);

    //! Construct a BeBriefcaseBasedId with the value of the next available (unused) value for the supplied Table/Column.
    //! @param[in] db the Db for this BeBriefcaseBasedId
    //! @param[in] tableName the name of the table for this BeBriefcaseBasedId
    //! @param[in] columnName the name of the column for this BeBriefcaseBasedId
    //! @note if the highest value of BeBriefcaseBasedId is already used (i.e. the id column is "full" for this BeBriefcaseId),
    //! this value will be invalid on return.
    BE_SQLITE_EXPORT BeBriefcaseBasedId(Db& db, Utf8CP tableName, Utf8CP columnName);

    static BeBriefcaseBasedId CreateFromJson(BeJsConst val) {return BeBriefcaseBasedId(val.asUInt64());}
    void FromJson(BeJsConst val) {m_id = val.asUInt64();}
};

#define BEBRIEFCASEBASED_ID_SUBCLASS(classname,superclass) struct classname : superclass { \
    classname(BeSQLite::BeBriefcaseId briefcaseId, uint64_t id) : superclass(briefcaseId,id){} \
    classname(BeSQLite::Db& db, Utf8CP tableName, Utf8CP columnName) : superclass(db,tableName,columnName){} \
    BEINT64_ID_DECLARE_MEMBERS(classname,superclass) };

#define BEBRIEFCASEBASED_ID_CLASS(classname) BEBRIEFCASEBASED_ID_SUBCLASS(classname,BeSQLite::BeBriefcaseBasedId)

//=======================================================================================
//! An 8-byte Id value that must be requested from an external authority that enforces uniqueness.
// @bsiclass
//=======================================================================================
struct BeServerIssuedId : BeInt64Id
{
    BEINT64_ID_DECLARE_MEMBERS(BeServerIssuedId,BeInt64Id)
};

#define BESERVER_ISSUED_ID_SUBCLASS(classname,superclass) struct classname : superclass {BEINT64_ID_DECLARE_MEMBERS(classname,superclass)};
#define BESERVER_ISSUED_ID_CLASS(classname) BESERVER_ISSUED_ID_SUBCLASS(classname,BeSQLite::BeServerIssuedId)

//=======================================================================================
// @bsiclass
//=======================================================================================
enum DbConstants
{
    DbUserVersion           = 10,  //!< the "user" version of SQLite databases created by this version of the BeSQLite library
    NoCompressionLevel      = 0,   //!< do not compress data
    FastCompressionLevel    = 1,   //!< use the fastest compression level (see Zlib documentation for explanation).
    DefaultCompressionLevel = 3,   //!< use the default compression level. This is pretty good tradeoff for size vs. speed.
    MaxCompressionLevel     = 9,   //!< the maximum compression level. Can be very slow but results in smallest size.
};

//=======================================================================================
// The profile version for BeSQLite database files created by this version of the BeSQLite library
// @bsiclass
//=======================================================================================
enum DbProfileValues
    {
    BEDB_CURRENT_VERSION_Major = 3,
    BEDB_CURRENT_VERSION_Minor = 1,
    BEDB_CURRENT_VERSION_Sub1  = 0,
    BEDB_CURRENT_VERSION_Sub2  = 2,

    BEDB_SUPPORTED_VERSION_Major = BEDB_CURRENT_VERSION_Major,  // oldest version of the db schema supported by current api
    BEDB_SUPPORTED_VERSION_Minor = 1,
    BEDB_SUPPORTED_VERSION_Sub1  = 0,
    BEDB_SUPPORTED_VERSION_Sub2  = 0,
    };

//=======================================================================================
// These constants define the current transaction state of a database file.
// For more information see https://www.sqlite.org/c3ref/c_txn_none.html
// @bsiclass
//=======================================================================================
enum DbTxnState {
    BE_SQLITE_TXN_NONE = 0,
    BE_SQLITE_TXN_READ = 1,
    BE_SQLITE_TXN_WRITE = 2,
};

//=======================================================================================
//! A 4-digit number that specifies the version of the "profile" (schema) of a Db
// @bsiclass
//=======================================================================================
struct ProfileVersion : BeVersion
{
public:
    ProfileVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : BeVersion(major, minor, sub1, sub2) {}
    explicit ProfileVersion(Utf8CP json) {FromJson(json);}
    bool operator==(BeVersionCR rhs) const { return CompareTo(rhs) == 0; }
    bool operator!=(BeVersionCR rhs) const { return CompareTo(rhs) != 0; }
    bool operator<(BeVersionCR rhs) const { return CompareTo(rhs) < 0; }
    bool operator<=(BeVersionCR rhs) const { return CompareTo(rhs) <= 0; }
    bool operator>(BeVersionCR rhs) const { return CompareTo(rhs) > 0; }
    bool operator>=(BeVersionCR rhs) const { return CompareTo(rhs) >= 0; }
    BE_SQLITE_EXPORT Utf8String ToJson() const;
    BE_SQLITE_EXPORT void FromJson(Utf8CP);
};
//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DbTrace
{
    None = 0,
    Stmt = 0x01,
    Profile = 0x02,
    Row = 0x04,
    Close= 0x08,
};
ENUM_IS_FLAGS(DbTrace)

enum class DbDeserializeOptions {
    None = 0,
    FreeOnClose = 1,
    Resizable = 2,
    Readonly = 3
};
ENUM_IS_FLAGS(DbDeserializeOptions)

//=======================================================================================
// @bsiclass
//=======================================================================================
enum DbResult
{
    BE_SQLITE_OK          = 0,    //!< Successful result
    BE_SQLITE_ERROR       = 1,    //!< SQL error or missing database
    BE_SQLITE_INTERNAL    = 2,    //!< Internal logic error
    BE_SQLITE_PERM        = 3,    //!< Access permission denied
    BE_SQLITE_ABORT       = 4,    //!< Callback routine requested an abort
    BE_SQLITE_BUSY        = 5,    //!< The database file is locked
    BE_SQLITE_LOCKED      = 6,    //!< A table in the database is locked
    BE_SQLITE_NOMEM       = 7,    //!< A malloc() failed
    BE_SQLITE_READONLY    = 8,    //!< Attempt to write a readonly database
    BE_SQLITE_INTERRUPT   = 9,    //!< Operation terminated by interrupt
    BE_SQLITE_IOERR       = 10,   //!< Some kind of disk I/O error occurred
    BE_SQLITE_CORRUPT     = 11,   //!< The database disk image is malformed
    BE_SQLITE_NOTFOUND    = 12,   //!< NOT USED. Table or record not found
    BE_SQLITE_FULL        = 13,   //!< Insertion failed because database is full or write operation failed because disk is full
    BE_SQLITE_CANTOPEN    = 14,   //!< Unable to open the database file
    BE_SQLITE_PROTOCOL    = 15,   //!< Database lock protocol error
    BE_SQLITE_EMPTY       = 16,   //!< Database is empty
    BE_SQLITE_SCHEMA      = 17,   //!< The database schema changed
    BE_SQLITE_TOOBIG      = 18,   //!< String or BLOB exceeds size limit
    BE_SQLITE_CONSTRAINT  = 19,   //!< Abort due to constraint violation. See extended error values.
    BE_SQLITE_MISMATCH    = 20,   //!< Data type mismatch
    BE_SQLITE_MISUSE      = 21,   //!< Library used incorrectly
    BE_SQLITE_NOLFS       = 22,   //!< Uses OS features not supported on host
    BE_SQLITE_AUTH        = 23,   //!< Authorization denied
    BE_SQLITE_FORMAT      = 24,   //!< Auxiliary database format error
    BE_SQLITE_RANGE       = 25,   //!< 2nd parameter to Bind out of range
    BE_SQLITE_NOTADB      = 26,   //!< File opened that is not a database file
    BE_SQLITE_NOTICE      = 27,    //!< Notifications from sqlite3_log()
    BE_SQLITE_WARNING     = 28,    //!< Warnings from sqlite3_log()
    BE_SQLITE_ROW         = 100,  //!< Step() has another row ready
    BE_SQLITE_DONE        = 101,  //!< Step() has finished executing

    BE_SQLITE_IOERR_READ              = (BE_SQLITE_IOERR | (1<<8)),
    BE_SQLITE_IOERR_SHORT_READ        = (BE_SQLITE_IOERR | (2<<8)),
    BE_SQLITE_IOERR_WRITE             = (BE_SQLITE_IOERR | (3<<8)),
    BE_SQLITE_IOERR_FSYNC             = (BE_SQLITE_IOERR | (4<<8)),
    BE_SQLITE_IOERR_DIR_FSYNC         = (BE_SQLITE_IOERR | (5<<8)),
    BE_SQLITE_IOERR_TRUNCATE          = (BE_SQLITE_IOERR | (6<<8)),
    BE_SQLITE_IOERR_FSTAT             = (BE_SQLITE_IOERR | (7<<8)),
    BE_SQLITE_IOERR_UNLOCK            = (BE_SQLITE_IOERR | (8<<8)),
    BE_SQLITE_IOERR_RDLOCK            = (BE_SQLITE_IOERR | (9<<8)),
    BE_SQLITE_IOERR_DELETE            = (BE_SQLITE_IOERR | (10<<8)),
    BE_SQLITE_IOERR_BLOCKED           = (BE_SQLITE_IOERR | (11<<8)),
    BE_SQLITE_IOERR_NOMEM             = (BE_SQLITE_IOERR | (12<<8)),
    BE_SQLITE_IOERR_ACCESS            = (BE_SQLITE_IOERR | (13<<8)),
    BE_SQLITE_IOERR_CHECKRESERVEDLOCK = (BE_SQLITE_IOERR | (14<<8)),
    BE_SQLITE_IOERR_LOCK              = (BE_SQLITE_IOERR | (15<<8)),
    BE_SQLITE_IOERR_CLOSE             = (BE_SQLITE_IOERR | (16<<8)),
    BE_SQLITE_IOERR_DIR_CLOSE         = (BE_SQLITE_IOERR | (17<<8)),
    BE_SQLITE_IOERR_SHMOPEN           = (BE_SQLITE_IOERR | (18<<8)),
    BE_SQLITE_IOERR_SHMSIZE           = (BE_SQLITE_IOERR | (19<<8)),
    BE_SQLITE_IOERR_SHMLOCK           = (BE_SQLITE_IOERR | (20<<8)),
    BE_SQLITE_IOERR_SHMMAP            = (BE_SQLITE_IOERR | (21<<8)),
    BE_SQLITE_IOERR_SEEK              = (BE_SQLITE_IOERR | (22<<8)),
    BE_SQLITE_IOERR_DELETE_NOENT      = (BE_SQLITE_IOERR | (23<<8)),
    BE_SQLITE_IOERR_MMAP              = (BE_SQLITE_IOERR | (24<<8)),
    BE_SQLITE_IOERR_GETTEMPPATH       = (BE_SQLITE_IOERR | (25<<8)),
    BE_SQLITE_IOERR_CONVPATH          = (BE_SQLITE_IOERR | (26<<8)),
    BE_SQLITE_IOERR_VNODE             = (BE_SQLITE_IOERR | (27<<8)),
    BE_SQLITE_IOERR_AUTH              = (BE_SQLITE_IOERR | (28<<8)),
    BE_SQLITE_IOERR_BEGIN_ATOMIC      = (BE_SQLITE_IOERR | (29<<8)),
    BE_SQLITE_IOERR_COMMIT_ATOMIC     = (BE_SQLITE_IOERR | (30<<8)),
    BE_SQLITE_IOERR_ROLLBACK_ATOMIC   = (BE_SQLITE_IOERR | (31<<8)),
    BE_SQLITE_IOERR_DATA              = (BE_SQLITE_IOERR | (32<<8)),
    BE_SQLITE_IOERR_CORRUPTFS         = (BE_SQLITE_IOERR | (33<<8)),

    BE_SQLITE_ERROR_MISSING_COLLSEQ   = (BE_SQLITE_ERROR | (1<<8)),
    BE_SQLITE_ERROR_RETRY             = (BE_SQLITE_ERROR | (2<<8)),
    BE_SQLITE_ERROR_SNAPSHOT          = (BE_SQLITE_ERROR | (3<<8)),

    BE_SQLITE_ERROR_FileExists        = (BE_SQLITE_IOERR | (1<<24)),  //!< attempt to create a new file when a file by that name already exists
    BE_SQLITE_ERROR_AlreadyOpen       = (BE_SQLITE_IOERR | (2<<24)),  //!< attempt to open a BeSQLite::Db that is already in use somewhere.
    BE_SQLITE_ERROR_NoPropertyTable   = (BE_SQLITE_IOERR | (3<<24)),  //!< attempt to open a BeSQLite::Db that doesn't have a property table.
    BE_SQLITE_ERROR_FileNotFound      = (BE_SQLITE_IOERR | (4<<24)),  //!< the database name is not a file.
    BE_SQLITE_ERROR_NoTxnActive       = (BE_SQLITE_IOERR | (5<<24)),  //!< there is no transaction active and the database was opened with AllowImplicitTransactions=false
    BE_SQLITE_ERROR_BadDbProfile      = (BE_SQLITE_IOERR | (6 << 24)), //!< wrong BeSQLite profile version
    BE_SQLITE_ERROR_InvalidProfileVersion = (BE_SQLITE_IOERR | (7<<24)),  //!< Profile of file could not be determined.
    BE_SQLITE_ERROR_ProfileUpgradeFailed = (BE_SQLITE_IOERR | (8<<24)),  //!< Upgrade of profile of file failed.
    BE_SQLITE_ERROR_ProfileTooOldForReadWrite = (BE_SQLITE_IOERR | (9 << 24)),  //!< Profile of file is too old for read-write access. Therefore file can only be opened read-only.
    BE_SQLITE_ERROR_ProfileTooOld     = (BE_SQLITE_IOERR | (10<<24)),  //!< Profile of file is too old. Therefore file cannot be opened.
    BE_SQLITE_ERROR_ProfileTooNewForReadWrite = (BE_SQLITE_IOERR | (11<<24)),  //!< Profile of file is too new for read-write access. Therefore file can only be opened read-only.
    BE_SQLITE_ERROR_ProfileTooNew     = (BE_SQLITE_IOERR | (12<<24)),  //!< Profile of file is too new. Therefore file cannot be opened.
    BE_SQLITE_ERROR_ChangeTrackError  = (BE_SQLITE_IOERR | (13<<24)),  //!< attempt to commit with active changetrack
    BE_SQLITE_ERROR_InvalidRevisionVersion = (BE_SQLITE_IOERR | (14 << 24)), //!< invalid version of the revision file is being imported

    BE_SQLITE_ERROR_SchemaUpgradeRequired   = (BE_SQLITE_IOERR | 15 << 24), //!< The schemas found in the database need to be upgraded.
    BE_SQLITE_ERROR_SchemaTooNew            = (BE_SQLITE_IOERR | 16 << 24), //!< The schemas found in the database are too new, and the application needs to be upgraded.
    BE_SQLITE_ERROR_SchemaTooOld            = (BE_SQLITE_IOERR | 17 << 24), //!< The schemas found in the database are too old, and the DgnDb needs to be recreated after extensive data transformations ("teleported").
    BE_SQLITE_ERROR_SchemaLockFailed        = (BE_SQLITE_IOERR | 18 << 24), //!< Error acquiring a lock on the schemas before upgrade.
    BE_SQLITE_ERROR_SchemaUpgradeFailed     = (BE_SQLITE_IOERR | 19 << 24), //!< Error upgrading the schemas in the database.
    BE_SQLITE_ERROR_SchemaImportFailed      = (BE_SQLITE_IOERR | 20 << 24), //!< Error importing the schemas into the database.
    BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes = (BE_SQLITE_IOERR | 21 << 24), //!< Error acquiring locks or codes
    BE_SQLITE_ERROR_SchemaUpgradeRecommended = (BE_SQLITE_IOERR | 22 << 24), //!< Recommended that the schemas found in the database be upgraded
    BE_SQLITE_ERROR_NOTOPEN                 = (BE_SQLITE_IOERR | (23 << 24)),

    BE_SQLITE_LOCKED_SHAREDCACHE      = (BE_SQLITE_LOCKED   | (1<<8)),

    BE_SQLITE_CANTOPEN_NOTEMPDIR      = (BE_SQLITE_CANTOPEN | (1<<8)),
    BE_SQLITE_CANTOPEN_ISDIR          = (BE_SQLITE_CANTOPEN | (2<<8)),
    BE_SQLITE_CANTOPEN_FULLPATH       = (BE_SQLITE_CANTOPEN | (3<<8)),
    BE_SQLITE_CANTOPEN_CONVPATH       = (BE_SQLITE_CANTOPEN | (4<<8)),
    BE_SQLITE_CANTOPEN_DIRTYWAL       = (BE_SQLITE_CANTOPEN | (5<<8)),
    BE_SQLITE_CANTOPEN_SYMLINK        = (BE_SQLITE_CANTOPEN | (6<<8)),
    BE_SQLITE_CORRUPT_VTAB            = (BE_SQLITE_CORRUPT  | (1<<8)),

    BE_SQLITE_READONLY_RECOVERY       = (BE_SQLITE_READONLY | (1<<8)),
    BE_SQLITE_READONLY_CANTLOCK       = (BE_SQLITE_READONLY | (2<<8)),
    BE_SQLITE_READONLY_ROLLBACK       = (BE_SQLITE_READONLY | (3<<8)),
    BE_SQLITE_READONLY_DBMOVED        = (BE_SQLITE_READONLY | (4<<8)),
    BE_SQLITE_READONLY_CANTINIT       = (BE_SQLITE_READONLY | (5<<8)),
    BE_SQLITE_READONLY_DIRECTORY      = (BE_SQLITE_READONLY | (6<<8)),
    BE_SQLITE_ABORT_ROLLBACK          = (BE_SQLITE_ABORT    | (2<<8)),

    BE_SQLITE_CONSTRAINT_CHECK        = (BE_SQLITE_CONSTRAINT | (1<<8)),
    BE_SQLITE_CONSTRAINT_COMMITHOOK   = (BE_SQLITE_CONSTRAINT | (2<<8)),
    BE_SQLITE_CONSTRAINT_FOREIGNKEY   = (BE_SQLITE_CONSTRAINT | (3<<8)),
    BE_SQLITE_CONSTRAINT_FUNCTION     = (BE_SQLITE_CONSTRAINT | (4<<8)),
    BE_SQLITE_CONSTRAINT_NOTNULL      = (BE_SQLITE_CONSTRAINT | (5<<8)),
    BE_SQLITE_CONSTRAINT_PRIMARYKEY   = (BE_SQLITE_CONSTRAINT | (6<<8)),
    BE_SQLITE_CONSTRAINT_TRIGGER      = (BE_SQLITE_CONSTRAINT | (7<<8)),
    BE_SQLITE_CONSTRAINT_UNIQUE       = (BE_SQLITE_CONSTRAINT | (8<<8)),
    BE_SQLITE_CONSTRAINT_VTAB         = (BE_SQLITE_CONSTRAINT | (9<<8)),
    BE_SQLITE_CONSTRAINT_ROWID        = (BE_SQLITE_CONSTRAINT |(10<<8)),
    BE_SQLITE_CONSTRAINT_PINNED       = (BE_SQLITE_CONSTRAINT |(11<<8)),

    BE_SQLITE_NOTICE_RECOVER_WAL      = (BE_SQLITE_NOTICE | (1<<8)),
    BE_SQLITE_NOTICE_RECOVER_ROLLBACK = (BE_SQLITE_NOTICE | (2<<8)),

    BE_SQLITE_BUSY_RECOVERY           = (BE_SQLITE_BUSY   |  (1<<8)),
    BE_SQLITE_BUSY_SNAPSHOT           = (BE_SQLITE_BUSY   |  (2<<8)),
    BE_SQLITE_BUSY_TIMEOUT            = (BE_SQLITE_BUSY   |  (3<<8)),
    BE_SQLITE_WARNING_AUTOINDEX       = (BE_SQLITE_WARNING | (1<<8)),
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DbLimits : int
{
    Length=0,
    SqlLength=1,
    Column=2,
    ExprDepth=3,
    CompoundSelect=4,
    VdbeOp=5,
    FunctionArg=6,
    Attached=7,
    LikePatternLength=8,
    VariableNumber=9,
    TriggerDepth=10,
    WorkerThreads=11,
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DbOpcode : int
{
    Delete  = 9,
    Insert  = 18,
    Update  = 23,
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum class DbValueType : int
{
    IntegerVal  = 1,
    FloatVal    = 2,
    TextVal     = 3,
    BlobVal     = 4,
    NullVal     = 5,
};

//=======================================================================================
// Utility class to initialize the BeSQLite library and directly use a few SQLite APIs.
// @bsiclass
//=======================================================================================
struct BeSQLiteLib
{
public:
    //=======================================================================================
    //! This is an interface class that allows applications to provide custom language processing for SQL case and collation operations.
    //! While a single static instance of this class is registered, collations are registered on a per-database basis. They are <i>not</i> expected to vary per database.
    // @bsiclass
    //=======================================================================================
    struct ILanguageSupport
    {
        //! Signature of the callback method used to free collator objects provided by _InitCollation. Objects will be freed as each database is closed (since they are created for each database).
        typedef void(*CollationUserDataFreeFunc)(void*);

        //! Describes a custom collator to register.
        //! @see _InitCollation.
        struct CollationEntry
        {
            AString m_name;     //!< Name that query strings will use to use this collation.
            void* m_collator;   //!< User data object provided in the collation callback. @see _Collate. @see CollationUserDataFreeFunc.
        };

        //! Converts source to lower-case into result according to localeName. result cannot be reallocated, and is typically over-allocated based on source.
        //! This is called when the SQL scalar function LOWER is processed.
        virtual void _Lower(Utf16CP source, int sourceLen, Utf16P result, int resultLen) = 0;

        //! Converts source to upper-case into result according to localeName. result cannot be reallocated, and is typically over-allocated based on source.
        //! This is called when the SQL scalar function UPPER is processed.
        virtual void _Upper(Utf16CP source, int sourceLen, Utf16P result, int resultLen) = 0;

        //! Registers a collection of collations with the database.
        //! This is called when every database is opened, and collatorFreeFunc is called when the database is closed for each collator provided.
        virtual void _InitCollation(bvector<CollationEntry>& collations, CollationUserDataFreeFunc& collatorFreeFunc) = 0;

        //! Compares two strings for sorting purposes. collator is the m_collator object provided in the corresponding CollationEntry.
        //! This is called when a custom collation is processed in a SQL query (e.g. in an ORDER BY clause).
        virtual int _Collate(Utf16CP lhs, int lhsLen, Utf16CP rhs, int rhsLen, void* collator) = 0;

        //! Maps the given UTF-32 character to its case folding equivalent (i.e. a normalized form used for comparison). This is primarily used in the LIKE operator.
        //! If the character has no case folding equivalent, the character itself is returned.
        virtual uint32_t _FoldCase(uint32_t) = 0;
    };

    enum class LogErrors : bool {Yes=1, No=0};

    //! This method MUST be called once per process before any other SQLite methods are called, and should never be called again.
    //! @param[in] tempDir The path for BeSQLite to use to store temporary files. Must be an existing directory.
    //! @param[in] wantLogging If Yes, then SQLite error messages are logged. Note that some SQLite errors are expected and are handled, so they do not necessarily indicate a problem.
    //! Turn this option on only for limited debugging purposes.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise, e.g. if @p tempDir does not exist
    //! @see sqlite3_initialize
    BE_SQLITE_EXPORT static DbResult Initialize(BeFileNameCR tempDir, LogErrors wantLogging=LogErrors::No);

    //! Generate a sequence of pseudo-random bytes
    //! @param[in] numBytes number of bytes of randomness to generate.
    //! @param[out] random buffer to hold randomness.
    //! @see sqlite3_randomness
    BE_SQLITE_EXPORT static void Randomness(int numBytes, void* random);

    //! Allocate a buffer using SQLite's memory manager.
    static void* MallocMem(int sz);

    //! Re-allocate a buffer returned from MallocMem.
    static void* ReallocMem(void* p, int sz);

    //! Free memory allocated by MallocMem.
    static void FreeMem(void* p);

    BE_SQLITE_EXPORT static int CloseSqlDb(void* p);

    //! Sets the static ILanguageSupport object for handling custom language processing.
    //! This should be called once per session before opening any databases and applies to all future opened databases.
    BE_SQLITE_EXPORT static void SetLanguageSupport(ILanguageSupport*);

    //! Gets the current ILanguageSupport. Can return nullptr.
    BE_SQLITE_EXPORT static ILanguageSupport* GetLanguageSupport();

    //! Get memory used by SQLite for current process
    BE_SQLITE_EXPORT static DbResult GetMemoryUsed(int64_t& current, int64_t& high, bool reset = false);

    //! Enabled shared cache for the process.
    BE_SQLITE_EXPORT static DbResult EnableSharedCache(bool enabled);

    //! Convert sqlite return code to string.
    BE_SQLITE_EXPORT static Utf8CP GetErrorString(DbResult rc);

    //! Return result code as static string e.g. SQLITE_ROW -> "SQLITE_ROW"
    BE_SQLITE_EXPORT static Utf8CP GetErrorName(DbResult rc);

    //! Return a log message (ErrorString, GetErrorName) for a DbResult
    BE_SQLITE_EXPORT static Utf8String GetLogError(DbResult rc);

    static int GetBaseDbResult(DbResult val) {return 0xff & val;}
    static bool TestBaseDbResult(DbResult val1, DbResult val2) {return GetBaseDbResult(val1) == GetBaseDbResult(val2);}
    static bool IsConstraintDbResult(DbResult val1) {return GetBaseDbResult(val1) == BE_SQLITE_CONSTRAINT;}
    BE_SQLITE_EXPORT static bool s_throwExceptionOnUnexpectedAutoCommit;

};

//=======================================================================================
//! Trace context provided during trace
// @bsiclass
//=======================================================================================
struct TraceContext : NonCopyableClass
{
private:
    SqlStatementP m_stmt;
public:
    explicit TraceContext(SqlStatementP stmt) {m_stmt=stmt;}

    //! Get a saved copy of the original SQL text used to prepare this Statement
    //! @see sqlite3_sql
    BE_SQLITE_EXPORT Utf8CP GetSql() const;

    //! Returns a UTF-8 string containing the SQL text of prepared statement with bound parameters expanded.
    //! @see sqlite3_expanded_sql
    BE_SQLITE_EXPORT Utf8String GetExpandedSql() const;

    //! Returns a pointer to a UTF-8 string containing the normalized SQL text of prepared statement.
    //! @see sqlite3_normalized_sql
    BE_SQLITE_EXPORT Utf8CP GetNormalizedSql() const;

    SqlStatementP GetSqlStatementP() const {return m_stmt;}  // for direct use of sqlite3 api
    operator SqlStatementP(){return m_stmt;}

    BE_SQLITE_EXPORT bool IsReadonly() const;
};


using TraceStmtEvent = BeEvent<TraceContext const&, Utf8CP>;
using TraceProfileEvent = BeEvent<TraceContext const &, int64_t>;
using TraceRowEvent = BeEvent<TraceContext const &>;
using TraceCloseEvent = BeEvent<SqlDbP,Utf8CP>;

//=======================================================================================
//! A wrapper for a SQLite Prepared Statement. Every BeSQLite::Statement object associated with a BeSQLite::Db must be deleted
//! before the database is closed.
// @bsiclass
//=======================================================================================
struct Statement : NonCopyableClass
{
private:
    SqlStatementP m_stmt;

    DbResult DoPrepare(DbFileCR, Utf8CP sql);

public:
    enum class MakeCopy : bool {No=0, Yes=1};
    explicit Statement(SqlStatementP stmt) {m_stmt=stmt;}

    //! construct a new blank Statement.
    Statement() {m_stmt=nullptr;}
    Statement(DbCR db, Utf8CP sql) {m_stmt=nullptr; Prepare(db, sql);}
    ~Statement() {Finalize();}

    SqlStatementP& GetStmtR() {return m_stmt;} //! @private internal use only
    DbResult Prepare(DbFileCR, Utf8CP sql, bool suppressDiagnostics = false); //! @private internal use only
    DbResult TryPrepare(DbFileCR, Utf8CP sql);

    //! Determine whether this Statement has already been prepared.
    bool IsPrepared() const {return nullptr != m_stmt;}

    //! Destroy this Statement. If the Statement was Prepared, releases all memory associated it.
    BE_SQLITE_EXPORT void Finalize();

    //! Prepare this Statement
    //! @param[in] db The database to use
    //! @param[in] sql The SQL string to prepare.
    //! @see sqlite3_prepare
    BE_SQLITE_EXPORT DbResult Prepare(DbCR db, Utf8CP sql);

    //! Prepare this Statement. Identical to Prepare, except that it does not automatically log errors
    //! @param[in] db The database to use
    //! @param[in] sql The SQL string to prepare.
    //! @see sqlite3_prepare
    BE_SQLITE_EXPORT DbResult TryPrepare(DbCR db, Utf8CP sql);

    //! Indicates whether the prepared statement makes no @b direct changes to the content of the db or not.
    //! @remarks
    //! @return true if the prepared statement makes no direct changes to the content of the db, false otherwise
    //! @see sqlite3_stmt_readonly, https://www.sqlite.org/c3ref/stmt_readonly.html
    BE_SQLITE_EXPORT bool IsReadonly() const;

    //! Perform a single step on this (previously prepared) Statement
    //! @see sqlite3_step
    BE_SQLITE_EXPORT DbResult Step();

    //! Reset this Statement
    //! @see sqlite3_reset
    BE_SQLITE_EXPORT DbResult Reset();

    //! Clear the bindings of this Statement
    //! @see sqlite3_clear_bindings
    BE_SQLITE_EXPORT DbResult ClearBindings();

    //! Bind an integer value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int
    BE_SQLITE_EXPORT DbResult BindInt(int paramNum, int value);

    //! Bind an Int64 value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int64
    BE_SQLITE_EXPORT DbResult BindInt64(int paramNum, int64_t value);

    //! Bind a UInt64 value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int64
    DbResult BindUInt64(int paramNum, uint64_t value) {return BindInt64(paramNum, (int64_t) value);}

    //! Bind a Boolean value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    DbResult BindBoolean(int paramNum, bool value) {return BindInt(paramNum, value ? 1 : 0);}

    //! Bind a BeBriefcaseBasedId value to a parameter of this (previously prepared) Statement (1-based). Binds NULL if the id is not valid.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    DbResult BindId(int paramNum, BeInt64Id value) {return value.IsValid() ? BindUInt64(paramNum,value.GetValue()) : BindNull(paramNum);}

    //! Bind a double value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_double
    BE_SQLITE_EXPORT DbResult BindDouble(int paramNum, double value);

    //! Bind the value of a Utf8String to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] stringValue the value to bind.
    //! @param[in] makeCopy Make a private copy of the string in the Statement. Only pass Statement::MakeCopy::No if stringValue will remain valid until the Statement's bindings are cleared.
    //! @see sqlite3_bind_text
    DbResult BindText(int paramNum, Utf8StringCR stringValue, MakeCopy makeCopy) {return BindText(paramNum, stringValue.c_str(), makeCopy, (int)stringValue.size());}

    //! Bind the value of a Utf8CP to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] stringValue the value to bind.
    //! @param[in] makeCopy Make a private copy of the string in the Statement. Only pass Statement::MakeCopy::No if stringValue will remain valid until the Statement's bindings are cleared.
    //! @param[in] nBytes The number of bytes (not characters) in @p stringValue. If negative, it will be calculated from stringValue. Passing this value is only an optimization.
    //! @see sqlite3_bind_text
    BE_SQLITE_EXPORT DbResult BindText(int paramNum, Utf8CP stringValue, MakeCopy makeCopy, int nBytes=-1);

    //! Bind a BeGuid to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @note BeGuids are saved as a 16-byte blob in the database.
    BE_SQLITE_EXPORT DbResult BindGuid(int paramNum, BeGuidCR value);

    //! Bind a zero-blob of the specified size to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] size The number of bytes for the blob.
    //! @see sqlite3_bind_zeroblob
    BE_SQLITE_EXPORT DbResult BindZeroBlob(int paramNum, int size);

    //! Bind a blob to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] blobValue the value to bind.
    //! @param[in] size The number of bytes in blobValue
    //! @param[in] makeCopy Make a private copy of the blob in the Statement. Only pass Statement::MakeCopy::No if blobValue will remain valid until the Statement's bindings are cleared.
    //! @see sqlite3_bind_blob
    BE_SQLITE_EXPORT DbResult BindBlob(int paramNum, void const* blobValue, int size, MakeCopy makeCopy);

    //! Bind a null value to a parameter of this (previously prepared) Statement (1-based)
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @see sqlite3_bind_null
    BE_SQLITE_EXPORT DbResult BindNull(int paramNum);

    //! Bind a VirtualSet. Must be the first parameter of the "InVirtualSet" BeSQLite function.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] vSet the VirtualSet to bind.
    //! @see BeSQLite::VirtualSet
    BE_SQLITE_EXPORT DbResult BindVirtualSet(int paramNum, struct VirtualSet const& vSet);

    //! @private internal use only
    //! Bind a DbValue from a BeSQLite function (1-based)
    BE_SQLITE_EXPORT DbResult BindDbValue(int paramNum, struct DbValue const& dbVal);

    //! Get the number of columns resulting from Step on this Statement (0-based)
    //! @see sqlite3_column_count
    BE_SQLITE_EXPORT int GetColumnCount();

    //! Get the type for a column of the result of Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_type
    BE_SQLITE_EXPORT DbValueType GetColumnType(int col);

    //!Get the table name from which a column of the result of Step originates (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_table_name
    BE_SQLITE_EXPORT Utf8CP GetColumnTableName(int col);

    //! Get the declared type for a column of the result of Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_decltype
    BE_SQLITE_EXPORT Utf8CP GetColumnDeclaredType(int col);

    //! Determine whether the column value is NULL. (0-based)
    bool IsColumnNull(int col) {return DbValueType::NullVal == GetColumnType(col);}

    //! Get the name of a column of the result of Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_name
    BE_SQLITE_EXPORT Utf8CP GetColumnName(int col);

    //! Get the number of bytes in a column of the result of Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_bytes
    BE_SQLITE_EXPORT int GetColumnBytes(int col);

    //! Get the number of bytes in a column as a utf16 string. This is only valid after a call to GetValueUtf16 (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_bytes16
    BE_SQLITE_EXPORT int GetColumnBytes16(int col);

    //! Get the value of a column in the result of Step as a blob (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_blob
    BE_SQLITE_EXPORT void const* GetValueBlob(int col);

    //! Get the value of a column in the result of Step as a UTF-8 string (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_text
    BE_SQLITE_EXPORT Utf8CP GetValueText(int col);

    //! Get an integer value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int
    BE_SQLITE_EXPORT int GetValueInt(int col);

    //! Get an Int64 value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int64
    BE_SQLITE_EXPORT int64_t GetValueInt64(int col);

    //! Get a UInt64 value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int64
    uint64_t GetValueUInt64(int col) {return (uint64_t) GetValueInt64(col);}

    //! Get a Boolean value from a column returned from Step (0-based)
    //! @remark As SQLite per-se doesn't have a Boolean type, this
    //! method returns true if the value is not 0 and false if it is 0.
    //! This is consistent with Statement::BindBoolean.
    //! @param[in] col The column of interest
    bool GetValueBoolean(int col) {return GetValueInt(col) != 0;}

    //! Get a double value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_double
    BE_SQLITE_EXPORT double GetValueDouble(int col);

    //! Get a BeBriefcaseBasedId value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    template <class T_Id> T_Id GetValueId(int col) {if (!IsColumnNull(col)) {return T_Id(GetValueUInt64(col));} return T_Id();}

    //! Get a BeGuid value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_blob
    BE_SQLITE_EXPORT BeGuid GetValueGuid(int col);

    //! @private internal use only
    //! Get an invariant sqlite value from a column returned from Step (0-based)
    //! @param[in] col The column of interest
    //! @see sqlite3_column_value, sqlite3_value_dup
    BE_SQLITE_EXPORT struct DbDupValue GetDbValue(int col);

    //! Get the index of a bound parameter by name.
    //! @param[in] name the name of the bound parameter
    //! @see sqlite3_bind_parameter_index
    BE_SQLITE_EXPORT int GetParameterIndex(Utf8CP name);

    //! Get the number of parameters in the statement
    //! @note This method actually returns the index of the largest (rightmost) parameter.
    //! For all forms except ?NNN, this will correspond to the number of unique parameters.
    //! If parameters of the ?NNN form are used, there may be gaps in the list
    //! @see sqlite3_bind_parameter_count
    BE_SQLITE_EXPORT int GetParameterCount();

    //! Get a saved copy of the original SQL text used to prepare this Statement
    //! @see sqlite3_sql
    BE_SQLITE_EXPORT Utf8CP GetSql() const;

    //! Returns a UTF-8 string containing the SQL text of prepared statement with bound parameters expanded.
    //! @see sqlite3_expanded_sql
    BE_SQLITE_EXPORT Utf8String GetExpandedSql() const;

    //! Returns a pointer to a UTF-8 string containing the normalized SQL text of prepared statement.
    //! @see sqlite3_normalized_sql
    BE_SQLITE_EXPORT Utf8CP GetNormalizedSql() const;

    //! Dump query results to stdout, for debugging purposes
    BE_SQLITE_EXPORT void DumpResults();

    SqlStatementP GetSqlStatementP() const {return m_stmt;}  // for direct use of sqlite3 api
    operator SqlStatementP(){return m_stmt;}                 // for direct use of sqlite3 api
};

#define DIAGNOSTICS_PREPARE_LOGGER_NAME "Diagnostics.BeSQLite.Prepare"
#define DIAGNOSTICS_QUERYPLAN_LOGGER_NAME "Diagnostics.BeSQLite.QueryPlan"
#define DIAGNOSTICS_QUERYPLANWITHTABLESCANS_LOGGER_NAME "Diagnostics.BeSQLite.QueryPlanWithTableScans"

#ifdef NDEBUG
#define STATEMENT_DIAGNOSTICS_ON
#define STATEMENT_DIAGNOSTICS_OFF
#define STATEMENT_DIAGNOSTICS_LOGCOMMENT(comment)
#else
#define STATEMENT_DIAGNOSTICS_ON StatementDiagnostics::SetIsEnabled(true);
#define STATEMENT_DIAGNOSTICS_OFF StatementDiagnostics::SetIsEnabled(false);
#define STATEMENT_DIAGNOSTICS_LOGCOMMENT(comment) StatementDiagnostics::LogComment(comment)

//=======================================================================================
//! Class to turn on/off diagnostics for BeSQLite::Statement.
//!
//! @note The diagnostics are only available in debug builds.
//!
//! ###Available diagnostics
//! - Minimize SQL preparation cost(Logger name: @b Diagnostics.BeSQLite.Prepare)
//! Preparing an @ref BentleyApi::BeSQLite::Statement "Statement" can be expensive.So statements should be reused
//! where applicable.In order to help analyze which statements to reuse and which not, turn on preparation diagnostics
//! by using this logger name.This will log all SQL statements being prepared by BeSQLite.
//! - Examine the SQL query plan(Logger name: @b Diagnostics.BeSQLite.QueryPlan or @b Diagnostics.BeSQLite.QueryPlanWithTableScans)
//! Examining the SQL query plan can, for example, be used to identify missing indexes.They can slow down queries significantly.
//! Turn on the query plan diagnostics by using one of the two logger names.
//! With @b Diagnostics.BeSQLite.QueryPlan the whole query plan is logged along with the SQL string.
//! With @b Diagnostics.BeSQLite.QueryPlanWithTableScans only query plans are logged if they contain <c>SCAN TABLE</c> directives,
//! which can be (but must not be) indications for missing indexes.
//!
//! Output format: SQL|1st item of query plan;2nd item of query plan;3rd item of query plan...
//!
//! Further notes:
//!  - BentleyApi::BeSQLite::StatementDiagnostics::SetIsEnabled
//!    As the diagnostics log every SQL being prepared, you can use this method to programmatically enable/disable diagnostics,
//!    so that only the code is diagnosed that you are interested in.
//!  - BentleyApi::BeSQLite::StatementDiagnostics::LogComment
//!    This will add the specified comment to the diagnostics output. This allows you to define sections
//!    in the diagnostics output
// @bsiclass
//=======================================================================================
struct StatementDiagnostics
    {
private:
    StatementDiagnostics();

public:
    //! Globally turn on or off statement diagnostics. If turned on, the logging configuration
    //! determines what is actually logged and what not. So use this method to globally disable
    //! diagnostics from code which you are not interested in.
    //! @param[in] isEnabled if true, diagnostics is enabled. If false, diagnostics is disabled.
    BE_SQLITE_EXPORT static void SetIsEnabled(bool isEnabled);

    //! @param[in] comment Comment to add to diagnostics output
    BE_SQLITE_EXPORT static void LogComment(Utf8CP comment);
    };
#endif

//=======================================================================================
//! A Blob handle for incremental I/O. See sqlite3_blob_open for details.
// @bsiclass
//=======================================================================================
struct BlobIO
{
private:
    SqlDbBlobP m_blob;

public:
    SqlDbBlobP GetBlobP() {return m_blob;}
    BlobIO() {m_blob=0;}
    ~BlobIO() {Close();}

    //! Open a Blob for incremental I/O.
    //! @param[in] db The database that holds the blob to be opened.
    //! @param[in] tableName The name of the table that holds the blob to be opened.
    //! @param[in] columnName The column that holds the blob to be opened.
    //! @param[in] row The rowId holding the blob.
    //! @param[in] writable If true, blob is opened for read/write access, otherwise it is opened readonly.
    //! @param[in] dbName The name of the database attachment to open. If nullptr, use "main".
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_open
    BE_SQLITE_EXPORT DbResult Open(DbR db, Utf8CP tableName, Utf8CP columnName, uint64_t row, bool writable, Utf8CP dbName=0);

    //! Move an existing opened BlobIO to a new row in the same table.
    //! @param[in] row The new rowId
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_reopen
    BE_SQLITE_EXPORT DbResult ReOpen(uint64_t row);

    //! Close an opened BlobIO
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_close
    BE_SQLITE_EXPORT DbResult Close();

    //! Read data from an opened BlobIO.
    //! @param[out] data A buffer into which the data is copied.
    //! @param[in] numBytes The number of bytes to copy to data.
    //! @param[in] offset The offset in bytes to the first byte to be copied.
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_read
    BE_SQLITE_EXPORT DbResult Read(void* data, int numBytes, int offset);

    //! Write data to an opened BlobIO.
    //! @param[in] data A buffer from which the data is copied.
    //! @param[in] numBytes The number of bytes to copy from data.
    //! @param[in] offset The offset in bytes to the first byte to be copied.
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_write
    BE_SQLITE_EXPORT DbResult Write(const void* data, int numBytes, int offset);

    //! Get the size of an opened blob
    //! @return The number of bytes in the current blob.
    //! @see sqlite3_blob_bytes
    BE_SQLITE_EXPORT int GetNumBytes() const;

    //! Determine whether this BlobIO was successfully opened.
    bool IsValid() const {return nullptr != m_blob;}
};

//=======================================================================================
//! A "value" from a BeSQLite function.
// @bsiclass
//=======================================================================================
struct DbValue
{
protected:
    SqlValueP m_val;

public:
    DbValue(SqlValueP val) : m_val(val) {}

    bool IsValid() const {return nullptr != m_val;}                    //!< return true if this value is valid
    bool IsNull()  const {return DbValueType::NullVal == GetValueType();} //!< return true if this value is null
    SqlValueP GetSqlValueP() const {return m_val;}  //!< for direct use of sqlite3 api

    BE_SQLITE_EXPORT DbValueType GetValueType() const;      //!< see sqlite3_value_type
    BE_SQLITE_EXPORT DbValueType GetNumericType() const;    //!< see sqlite3_value_numeric_type
    BE_SQLITE_EXPORT int         GetValueBytes() const;     //!< see sqlite3_value_bytes
    BE_SQLITE_EXPORT void const* GetValueBlob() const;      //!< see sqlite3_value_blob
    BE_SQLITE_EXPORT Utf8CP      GetValueText() const;      //!< see sqlite3_value_text
    BE_SQLITE_EXPORT int         GetValueInt() const;       //!< see sqlite3_value_int
    BE_SQLITE_EXPORT int64_t     GetValueInt64() const;     //!< see sqlite3_value_int64
    uint64_t GetValueUInt64() const {return (uint64_t) GetValueInt64();}
    BE_SQLITE_EXPORT double      GetValueDouble() const;    //!< see sqlite3_value_double
    BE_SQLITE_EXPORT BeGuid      GetValueGuid() const;      //!< get the value as a GUID
    template <class T_Id> T_Id   GetValueId() const {return T_Id(GetValueUInt64());}

    BE_SQLITE_EXPORT Utf8String Format(int detailLevel) const; //!< for debugging purposes.
};

//=======================================================================================
//! A duplicated "value" from a BeSQLite function
//! @remarks Used when the sqlite value may refer to unprotected memory, and needs to
//! be protected by duplication. @see sqlite3_value_dup
// @bsiclass
//=======================================================================================
struct DbDupValue : DbValue, NonCopyableClass
{
    BE_SQLITE_EXPORT DbDupValue(SqlValueP val);
    DbDupValue(DbDupValue&& other) : DbValue(other.m_val) {other.m_val = nullptr;}
    BE_SQLITE_EXPORT DbDupValue& operator=(DbDupValue&& other);
    BE_SQLITE_EXPORT ~DbDupValue();
};

//=======================================================================================
//! A user-defined function that can be added to a Db connection and then used in SQL.
//! See http://www.sqlite.org/capi3ref.html#sqlite3_create_function.
//! WARNING: The implementation of a DbFunction is forbidden from creating or using
//! a CachedStatement. Doing so can easily result in deadlock due to contention between
//! the sqlite mutex and the StatementCache mutex.
// @bsiclass
//=======================================================================================
struct DbFunction : NonCopyableClass
{
public:
    //=======================================================================================
    //! The "context" supplied to DbFunctions that can be used to set result values.
    // @bsiclass
    //=======================================================================================
    struct Context
        {
        enum class CopyData : int {No = 0, Yes = -1};                                     //!< see sqlite3_destructor_type
        BE_SQLITE_EXPORT void SetResultBlob(void const* value, int length, CopyData copy=CopyData::Yes); //!< see sqlite3_result_blob
        BE_SQLITE_EXPORT void SetResultDouble(double);                                //!< see sqlite3_result_double
        BE_SQLITE_EXPORT void SetResultError(Utf8CP, int len=-1);                     //!< see sqlite3_result_error
        BE_SQLITE_EXPORT void SetResultError_toobig();                                //!< see sqlite3_result_error_toobig
        BE_SQLITE_EXPORT void SetResultError_nomem();                                 //!< see sqlite3_result_error_nomem
        BE_SQLITE_EXPORT void SetResultError_code(int);                               //!< see sqlite3_result_error_code
        BE_SQLITE_EXPORT void SetResultInt(int);                                      //!< see sqlite3_result_int
        BE_SQLITE_EXPORT void SetResultInt64(int64_t);                                //!< see sqlite3_result_int64
        BE_SQLITE_EXPORT void SetResultNull();                                        //!< see sqlite3_result_null
        BE_SQLITE_EXPORT void SetResultText(Utf8CP value, int length, CopyData);      //!< see sqlite3_result_text
        BE_SQLITE_EXPORT void SetResultZeroblob(int length);                          //!< see sqlite3_result_zeroblob
        BE_SQLITE_EXPORT void SetResultValue(DbValue);                                //!< see sqlite3_result_value
        BE_SQLITE_EXPORT void* GetAggregateContext(int nbytes);
        };

private:
    Utf8String  m_name;
    int         m_nArgs;
    DbValueType m_returnType;

protected:
    //! Initializes a new DbFunction instance
    //! @param[in] name Function name
    //! @param[in] nArgs Number of function args
    //! @param[in] returnType Function return type. DbValueType::NullVal means that the return type is unspecified.
    DbFunction(Utf8CP name, int nArgs, DbValueType returnType) : m_name(name), m_nArgs(nArgs), m_returnType(returnType) {}

public:
    virtual bool _IsAggregate() {return false;}
    virtual ~DbFunction() {}

    Utf8CP GetName() const {return m_name.c_str();} //!< Get the name of this function
    int GetNumArgs() const {return m_nArgs;}    //!< Get the number of arguments to this function
    DbValueType GetReturnType() const {return m_returnType;}//!< Gets the return type of the function.

    //! Set the result of this function to: error due to illegal input.
    void SetInputError(Context& ctx) {ctx.SetResultError(Utf8PrintfString("Illegal input to %s", GetName()).c_str());}
};

//=======================================================================================
//! A user-defined scalar function. See discussion of scalar functions at http://www.sqlite.org/capi3ref.html#sqlite3_create_function.
//! This object is must survive as long as the Db to which it is added survives, or until it is removed.
// @bsiclass
//=======================================================================================
struct ScalarFunction : DbFunction
{
    //! WARNING: The implementation of _ComputeScalar is forbidden from creating or using
    //! a CachedStatement. Doing so can easily result in deadlock due to contention between
    //! the sqlite mutex and the StatementCache mutex.
    virtual void _ComputeScalar(Context&, int nArgs, DbValue* args) = 0;   //<! see "xFunc" in sqlite3_create_function

    //! Initializes a new ScalarFunction instance
    //! @param[in] name Function name
    //! @param[in] nArgs Number of function args
    //! @param[in] returnType Function return type. DbValueType::NullVal means that the return type is unspecified.
    ScalarFunction(Utf8CP name, int nArgs, DbValueType returnType = DbValueType::NullVal) : DbFunction(name, nArgs, returnType) {}
};
//=======================================================================================
//! A user-defined aggregate function. See discussion of aggregate functions at http://www.sqlite.org/capi3ref.html#sqlite3_create_function.
//! This object is must survive as long as the Db to which it is added survives, or until it is removed.
// @bsiclass
//=======================================================================================
struct AggregateFunction : DbFunction
{
    bool _IsAggregate() override {return true;}

    //! WARNING: The implementations of _StepAggregate and _FinishAggregate are forbidden from creating or using
    //! a CachedStatement. Doing so can easily result in deadlock due to contention between
    //! the sqlite mutex and the StatementCache mutex.
    virtual void _StepAggregate(Context&, int nArgs, DbValue* args) = 0; //<! see "xStep" in sqlite3_create_function
    virtual void _FinishAggregate(Context&) = 0;                         //<! see "xFinal" in sqlite3_create_function

    //! Initializes a new AggregateFunction instance
    //! @param[in] name Function name
    //! @param[in] nArgs Number of function args
    //! @param[in] returnType Function return type. DbValueType::NullVal means that the return type is unspecified.
    AggregateFunction(Utf8CP name, int nArgs, DbValueType returnType = DbValueType::NullVal) : DbFunction(name, nArgs, returnType) {}
};

//=======================================================================================
//! A user-defined implementation of the SQLite sqlite3_rtree_query_callback function for using the MATCH keyword for RTree queries.
//! See https://www.sqlite.org/rtree.html for implementation details.
// @bsiclass
//=======================================================================================
struct RTreeMatchFunction : DbFunction
{
    enum class Within : int
    {
        Outside = 0,
        Partly = 1,
        Inside = 2,
    };

    //=======================================================================================
    //! This is a copy of sqlite3_rtree_query_info
    // @bsiclass
    //=======================================================================================
    struct QueryInfo
    {
        void*   m_context;
        int     m_nParam;
        double* m_param;
        void*   m_user;
        void    (*m_xDelUser)(void*);
        double* m_coords;
        unsigned int* m_nQueue;
        int     m_nCoord;
        int     m_level;
        int     m_maxLevel;
        int64_t m_rowid;
        double  m_parentScore;
        Within  m_parentWithin;
        mutable Within m_within;
        mutable double m_score;
        DbValue* m_args;       // SQL values of parameters
    };

    //! this method is called for every internal and leaf node in an sqlite rtree vtable.
    //! @see sqlite3_rtree_query_callback.
    //! WARNING: The implementation of _TestRange is forbidden from creating or using
    //! a CachedStatement. Doing so can easily result in deadlock due to contention between
    //! the sqlite mutex and the StatementCache mutex.
    virtual int _TestRange(QueryInfo const&) = 0;

    RTreeMatchFunction(Utf8CP name, int nArgs) : DbFunction(name, nArgs, DbValueType::NullVal) {}
};

//=======================================================================================
//! This interface should be implemented to supply the first argument to the BeSQLite function "InVirtualSet".
//! It provides a way to use an in-memory "set of values" in an SQL statement, without having to create a temporary table.
//! For example, to find rows of MyTable that have Owner=1 and Vendor in a list of vendors held in a memory, use the SQL statement:
//! "SELECT * FROM MyTable WHERE Owner=1 AND InVirtualSet(?,Vendor)" and bind the first argument of InVirtualSet using BeSQLite::Statement::BindVirtualSet
//! to an object that implements this interface, returning true for _IsInSet for the desired vendors.
// @bsiclass
//=======================================================================================
struct VirtualSet
{
    //! Test whether a value, or combination of values, is in the virtual set. Implementation should be as efficient as possible.
    //! @param[in] nVals The number of entries in vals array holding the values supplied as the second and later arguments to the "inVirtualSet" SQL function.
    //! For example, with  "SELECT * FROM MyTable WHERE InVirtualSet(@myset,Cost,Vendor)" the values of the Cost and Vendor columns will be passed
    //! as vals[0] and vals[1] respectively.
    //! @param[in] vals The array of values. Use the DbValue::Get... methods to get the column data from vals.
    //! @return true if data in vals is in the set.
    virtual bool _IsInSet(int nVals, DbValue const* vals) const = 0;
    virtual ~VirtualSet() {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeIdSet : bset<BeInt64Id>
{
private:
    Utf8String ToReadableString() const;
    void FromCompactString(Utf8StringCR);
    void FromReadableString(Utf8StringCR);

    BE_SQLITE_EXPORT static void SaveCompactRange(Utf8StringR str, int64_t increment, int64_t len);
public:
    enum class StringFormat {Compact=0, Readable=1};

    BE_SQLITE_EXPORT void FromString(Utf8StringCR in);
    BE_SQLITE_EXPORT Utf8String ToString() const;
    Utf8String ToCompactString() const { return ToCompactString(*this); }

    /*---------------------------------------------------------------------------------**//**
    * Convert a set of BeInt64Ids to a compact string representation. IDs are often quite large values
    * due to inclusion of briefcase ID and sequences of IDs often follow a particular
    * pattern (e.g. [x,x+2,x+4,x+6,x+8] is common with sets of DgnCategoryIds). IdSets are
    * ordered and the deltas between two consecutive values tend to be small with occasional
    * larger jumps between IDs with different briefcase IDs.
    * Given that, we can reduce the length of the string and the complexity of parsing it by:
    *   - Using base-16 representation
    *   - Writing the actual value of only the first element
    *   - Writing each subsequent value in terms of the positive delta from the preceding value
    *   - Collapsing ranges of identical deltas
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename T_Id> static Utf8String ToCompactString(bset<T_Id> const& ids)
        {
        Utf8String str;

        int64_t prevId = 0;
        int64_t rangeIncrement = 0;
        int64_t rangeLen = 0;

        auto iter = ids.begin();
        if (iter != ids.end() && !iter->IsValid())
            ++iter; // omit invalid Ids

        for (/**/ ; iter != ids.end(); ++iter)
            {
            auto elem = *iter;
            int64_t curId = elem.GetValue();
            int64_t curIncrement = curId - prevId;
            prevId = curId;

            if (0 == rangeLen)
                {
                rangeIncrement = curIncrement;
                rangeLen = 1;
                }
            else if (curIncrement == rangeIncrement)
                {
                ++rangeLen;
                }
            else
                {
                SaveCompactRange(str, rangeIncrement, rangeLen);
                rangeIncrement = curIncrement;
                rangeLen = 1;
                }
            }

        if (0 < rangeLen)
            SaveCompactRange(str, rangeIncrement, rangeLen);

        return str;
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
template<typename IdType> struct IdSet : VirtualSet
{
private:
    BeIdSet m_set;

    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        BeAssert(nVals == 1);
        return Contains(IdType(vals[0].GetValueUInt64()));
        }
public:
    IdSet()
        {
        static_assert(std::is_base_of<BeInt64Id, IdType>::value && sizeof(BeInt64Id) == sizeof(IdType), "IdSet may only contain BeInt64Ids or subclasses of it of the same size.");
        }

    explicit IdSet(BeIdSet&& ids) : m_set(std::move(ids))
        {
        static_assert(std::is_base_of<BeInt64Id, IdType>::value && sizeof(BeInt64Id) == sizeof(IdType), "IdSet may only contain BeInt64Ids or subclasses of it of the same size.");
        }

    virtual ~IdSet() {}

    typedef IdSet<IdType> T_Self;
    typedef bset<IdType> T_SetType;
    typedef typename T_SetType::const_iterator const_iterator;
    typedef typename T_SetType::iterator iterator;

    const_iterator begin() const {return ((T_SetType const&)m_set).begin();}
    const_iterator end() const {return ((T_SetType const&)m_set).end();}
    const_iterator find(IdType id) const {return ((T_SetType const&)m_set).find(id);}
    bool operator==(T_Self const& other) const {return m_set==other.m_set;}
    bool operator!=(T_Self const& other) const {return m_set!=other.m_set;}
    bool empty() const {return m_set.empty();}
    void clear() {m_set.clear();}
    size_t size() const {return m_set.size();}
    bpair<iterator,bool> insert(IdType const& val) {BeAssert(val.IsValid()); return ((T_SetType&)m_set).insert(val);}
    void insert(const_iterator first, const_iterator last) {((T_SetType&)m_set).insert(first,last);}
    size_t erase(IdType const& val) {return ((T_SetType&)m_set).erase(val);}
    iterator erase(iterator it) {return ((T_SetType&)m_set).erase(it);}
    bool Contains(IdType id) const {return end() != find(id);}
    void FromString(Utf8StringCR in) {m_set.FromString(in);}
    Utf8String ToString() const {return m_set.ToString();}

    BeIdSet const& GetBeIdSet() const {return m_set;}
};

//=======================================================================================
//! A user-defined callback which is invoked whenever a row is updated, inserted or
//! deleted in a rowid table.
//! Call Db::AddDataUpdateCallback to register a callback with the db connection
//! @see https://sqlite.org/c3ref/update_hook.html for details
// @bsiclass
//=======================================================================================
struct DataUpdateCallback : NonCopyableClass
    {
public:
    enum class SqlType
        {
        Insert, //!< Row, for which callback was invoked, was inserted
        Update, //!< Row, for which callback was invoked, was updated
        Delete //!< Row, for which callback was invoked, was deleted
        };

protected:
    DataUpdateCallback() {}

public:
    virtual ~DataUpdateCallback() {}

    //! Called by SQLite when a row is updated, inserted or deleted.
    //! @param[in] sqlType SQL operation that caused the callback to be invoked
    //! @param[in] dbName Name of the database containing the table of the affected row
    //! @param[in] tableName Name of the table containing the affected row
    //! @param[in] rowid Rowid of the affected row. In case of SqlType::Update, this is the rowid after the update
    //! takes place
    virtual void _OnRowModified(SqlType sqlType, Utf8CP dbName, Utf8CP tableName, BeInt64Id rowid) = 0;
    };

//=======================================================================================
//! Wraps sqlite3_mprintf. Adds convenience that destructor frees memory.
// @bsiclass
//=======================================================================================
struct SqlPrintfString final
{
private:
    Utf8P m_str;
public:
    //! @see sqlite3_mprintf
    BE_SQLITE_EXPORT SqlPrintfString(Utf8CP fmt, ...);
    BE_SQLITE_EXPORT ~SqlPrintfString();
    operator Utf8CP(){return m_str;}
    Utf8CP GetUtf8CP() {return m_str;}
};

//=======================================================================================
//! Holds a mutex to synchronize multi-thread access to data.
// @bsiclass
//=======================================================================================
struct BeDbMutex : NonCopyableClass
{
private:
    void*  m_mux;
public:
    enum class MutexType : bool {Fast=0, Recursive=1};
    BE_SQLITE_EXPORT BeDbMutex(MutexType mutexType=MutexType::Fast);       //!< create a new SQLite mutex, see sqlite3_mutex_alloc
    BE_SQLITE_EXPORT ~BeDbMutex();      //!< free mutex
    BE_SQLITE_EXPORT void Enter();      //!< acquire mutex's lock
    BE_SQLITE_EXPORT void Leave();      //!< release mutex's lock
#ifndef NDEBUG
    BE_SQLITE_EXPORT bool IsHeld();
#endif
};

//=======================================================================================
//! A convenience class for acquiring and releasing a mutex lock. Lock is acquired on construction and released on destruction.
// @bsiclass
//=======================================================================================
struct BeDbMutexHolder final : NonCopyableClass
{
    BeDbMutex& m_mutex;
    BeDbMutexHolder(BeDbMutex& mutex) : m_mutex(mutex) {m_mutex.Enter();}
    ~BeDbMutexHolder() {m_mutex.Leave();}
};

//=======================================================================================
//! Holds a mutex to synchronize multi-thread access to data.
// @bsiclass
//=======================================================================================
struct BeSqliteDbMutex final : NonCopyableClass
{
private:
    void*  m_mux;
public:
    BE_SQLITE_EXPORT BeSqliteDbMutex(Db& db);
    BE_SQLITE_EXPORT void Enter();      //!< acquire mutex's lock
    BE_SQLITE_EXPORT void Leave();      //!< release mutex's lock
#ifndef NDEBUG
    BE_SQLITE_EXPORT bool IsHeld();
#endif
};

//=======================================================================================
//! A convenience class for acquiring and releasing the Db's mutex.
// @bsiclass
//=======================================================================================
struct BeSqliteDbMutexHolder final : NonCopyableClass
{
    BeSqliteDbMutex m_mutex;
    BeSqliteDbMutexHolder(Db& db) : m_mutex(db) {m_mutex.Enter();}
    ~BeSqliteDbMutexHolder() {m_mutex.Leave();}
};

//=======================================================================================
//! A reference-counted Statement. Statement is freed when last reference is Released.
// @bsiclass
//=======================================================================================
struct CachedStatement final : Statement
{
    friend struct StatementCache;
private:
    mutable BeAtomic<uint32_t> m_refCount;
    bool m_inCache;
    struct StatementCache const& m_myCache;
    Utf8CP  m_sql;
    CachedStatement(Utf8CP sql, struct StatementCache const&);
    ~CachedStatement();

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    uint32_t AddRef() const {return m_refCount.IncrementAtomicPre();}
    uint32_t GetRefCount() const {return m_refCount.load();}
    BE_SQLITE_EXPORT uint32_t Release();
    Utf8CP GetSQL() const {return m_sql;}

    //! CachedStatements can never be Finalized externally. That will corrupt the cache.
    void Finalize() = delete;
};

typedef RefCountedPtr<CachedStatement> CachedStatementPtr;

//=======================================================================================
//! A cache of SharedStatements that can be reused without re-Preparing. It can be very expensive to Prepare an SQL statement,
//! so this class provides a way to save previously prepared statements for reuse (note, a prepared Statement is specific to a
//! particular SQLite database, so there is a StatementCache for each BeSQLite::Db)
//! By default, the cache holds 20 SharedStatements and releases the oldest statement when a new entry is added to a full cache.
// @bsiclass
//=======================================================================================
struct StatementCache final: NonCopyableClass
{
    friend struct CachedStatement;
private:

    typedef std::list<CachedStatementPtr> Entries;
    mutable BeMutex* m_mutex;
    bool m_ownMutex;
    mutable Entries m_entries;
    uint32_t m_size;
    Entries::iterator FindEntry(Utf8CP) const;
    BE_SQLITE_EXPORT void AddStatement(CachedStatementPtr& newEntry, Utf8CP sql) const;
    BE_SQLITE_EXPORT void FindStatement(CachedStatementPtr&, Utf8CP) const;

public:
    BE_SQLITE_EXPORT explicit StatementCache(uint32_t size, BeMutex* inheritedMutex = nullptr);
    BE_SQLITE_EXPORT ~StatementCache();

    BE_SQLITE_EXPORT DbResult GetPreparedStatement(CachedStatementPtr&, DbFile const& dbFile, Utf8CP sqlString, bool logError = true) const;
    BE_SQLITE_EXPORT void Dump() const;
    BE_SQLITE_EXPORT void Empty();
    bool IsEmpty() const {return m_entries.empty();}
};

//=======================================================================================
//! Values that can be bound to named parameters of an SQL statement.
//! This is useful for cases where one part of the API constructs a part of an SQL statement and
//! wishes to delegate additional criteria to be added by a caller (e.g. an iterator.) It is also
//! very useful to avoid SQL injection vulnerabilities when receiving input from a user.
//! The names must match the parameter names in the SQL statement.
//! For example:
//! @code
//!  Statement stmt;
//!  DbResult rc = stmt.Prepare(db, "SELECT * FROM Widget WHERE name LIKE @partname AND quant>@needed");
//!  NamedParams params;
//!  params.AddStringParameter("@partname", "bearing 21");
//!  params.AddIntegerParameter("@needed", 200);
//!  params.Bind(stmt);
//! @endcode
// @bsiclass
//=======================================================================================
struct NamedParams
{
public:
    struct SqlParameter : RefCountedBase
    {
        Utf8String m_name;
        SqlParameter(Utf8CP name) : m_name(name) {}
        virtual void _Bind(Statement&) = 0;
    };

private:
    typedef RefCountedPtr<SqlParameter> SqlParameterPtr;
    Utf8String m_where;
    bvector<SqlParameterPtr>  m_params;

public:
    //! Add a parameter value to this set of NamedParams.
    //! @param[in] param the parameter to add
    //! @note If more than one parameter exist with the same parameter name, the last one is used.
    void AddSqlParameter(SqlParameter* param) {m_params.push_back(param);}

    //! Add a String parameter value to this NamedParams.
    //! @param[in] name The name of the parameter (including the "@" or ":" used in the SQL).
    //! @param[in] val The value to be bound to the parameter.
    BE_SQLITE_EXPORT void AddStringParameter(Utf8CP name, Utf8CP val);

    //! Add an integer parameter value to this NamedParams.
    //! @param[in] name The name of the parameter (including the "@" or ":" used in the SQL).
    //! @param[in] val The value to be bound to the parameter.
    BE_SQLITE_EXPORT void AddIntegerParameter(Utf8CP name, uint64_t val);

    //! Add a double parameter value to this NamedParams.
    //! @param[in] name The name of the parameter (including the "@" or ":" used in the SQL).
    //! @param[in] val The value to be bound to the parameter.
    BE_SQLITE_EXPORT void AddDoubleParameter(Utf8CP name, double val);

    //! Add a blob parameter value to this NamedParams.
    //! @param[in] name The name of the parameter (including the "@" or ":" used in the SQL).
    //! @param[in] data The data value to be bound to the parameter.
    //! @param[in] size The size value to be bound to the parameter.
    //! @param[in] copy The Statement::MakeCopy value to be bound to the parameter.
    BE_SQLITE_EXPORT void AddBlobParameter(Utf8CP name, void const* data, int size, Statement::MakeCopy copy);

    //! ctor for NamedParams
    //! @param[in] where an optional part of the where clause for a SELECT statement.
    NamedParams(Utf8CP where=nullptr) {SetWhere(where);}

    //! Change the value of the SQL Where clause for this NamedParam.
    //! @param[in] where the new WHERE clause
    //! @note it is not necessary to add "WHERE " or "AND " to your Where clause (since it's impossible for you to know which
    //! is correct without knowing what criteria already exists.) Just supply your part of the Where clause and it will be
    //! combined with other criteria as appropriate.
    BE_SQLITE_EXPORT void SetWhere(Utf8CP where);

    //! Get the Where clause for this NamedParam
    Utf8StringCR GetWhere() const {return m_where;}

    //! Bind all of the added parameters to the supplied statement.
    //! @param[in] stmt the statement containing the named parameters to which these values are to be bound.
    BE_SQLITE_EXPORT void Bind(Statement& stmt) const;
};

//=======================================================================================
//! Base class for an Iterator of a BeSQLite::Db table. Table implementers should derive from this class to provide a convenient C++
//! iterator for the rows of a table.
//! <p>DbTableIterator::Entry objects only valid for the lifetime of their DbTableIterator. This is not valid:
//! @code
//!          DbTableIterator::Entry e = table.MakeIterator().begin();
//! @endcode
//! because the temporary DbTableIterator goes out of scope, making the Entry invalid. Instead, do this:
//! @code
//!          DbTableIterator iter = table.MakeIterator();
//!          DbTableIterator::Entry e = iter.begin();
//! @endcode
//! and make sure the DbTableIterator is valid for the lifetime of its Entries.
// @bsiclass
//=======================================================================================
struct DbTableIterator  : NonCopyableClass
{
protected:
    mutable DbP m_db;
    mutable CachedStatementPtr m_stmt;
    NamedParams m_params;

    explicit DbTableIterator(DbCR db) : m_db(const_cast<DbP>(&db)) {}

    //! Move ctor. DbTableIterator are not copyable, but they are movable.
    DbTableIterator(DbTableIterator&& rhs) : m_db(rhs.m_db), m_stmt(std::move(rhs.m_stmt)), m_params(rhs.m_params) {}

    BE_SQLITE_EXPORT Utf8String MakeSqlString(Utf8CP sql, bool hasWhere=false) const;

public:
    struct Entry
    {
    protected:
        bool  m_isValid;
        StatementP m_sql;
        Entry(StatementP sql, bool isValid) {m_sql=sql; m_isValid=isValid;}
        void Verify() const {BeAssert(nullptr != m_sql->GetSqlStatementP());}

    public:
        bool IsValid() const {return m_isValid && (nullptr!=m_sql->GetSqlStatementP());}
        void Invalidate() {m_isValid=false;}

        Entry& operator++() {m_isValid=(BeSQLite::BE_SQLITE_ROW == m_sql->Step()); return *this;}
        Entry const& operator* () const {return *this;}
        bool operator!=(Entry const& rhs) const {return (m_isValid != rhs.m_isValid);}
        bool operator==(Entry const& rhs) const {return (m_isValid == rhs.m_isValid);}
    };

    //! Get the prepared statement for this iterator. This can be used to bind parameters before calling /c begin.
    Statement* GetStatement() {return m_stmt.get();}

    //! Get a reference to the NamedParams for this iterator. This is useful for adding additional WHERE clause
    //! filtering to the iterator.
    NamedParams& Params() {return m_params;}
};

//=======================================================================================
//! Defines a callback for providing information on the progress of a compress or
//! decompress operation.
// @bsiclass
//=======================================================================================
struct ICompressProgressTracker
{
    //! @param[in] inSize size of the source file
    //! @param[in] outSize size of the output; -1 if no valid information is available.
    //! Returning anything other than BSISUCCESS causes the operation to abort.
    virtual StatusInt _Progress(uint64_t inSize, int64_t outSize) = 0;
};

//=======================================================================================
// Used to hold large in-memory buffers in chunks, without requiring large mallocs.
// @bsiclass
//=======================================================================================
struct ChunkedArray {
    using Chunk = bvector<Byte>;
    int m_maxSize; // when Append is called and the current chunk is already larger than this, start a new chunk.
    size_t m_size = 0; // total number of bytes in all chunks
    bvector<Chunk> m_chunks;

    void Clear() { m_size = 0; m_chunks.clear(); }
    bool IsEmpty() const { return m_size == 0; }
    ChunkedArray(int maxSize = 64 * 1024) { m_maxSize = maxSize; }
    ChunkedArray(ChunkedArray&& other) { *this = std::move(other); }
    ChunkedArray& operator=(ChunkedArray&& other) {
        m_maxSize = other.m_maxSize;
        m_chunks = std::move(other.m_chunks);
        m_size = other.m_size;
        other.m_size = 0;
        return *this;
    }
    void Append(Byte const* data, int size) {
        m_size += size;
        if (m_chunks.size() > 0 && m_chunks.back().size() < m_maxSize)
            m_chunks.back().insert(m_chunks.back().end(), data, data + size);
        else
            m_chunks.emplace_back(Chunk(data, data + size));
    }
    struct Reader {
        ChunkedArray const& m_array;
        size_t m_offset = 0;
        int m_chunkNum = 0;
        size_t m_currPos = 0;
        Reader(ChunkedArray const& array) : m_array(array) {}
        BE_SQLITE_EXPORT void Read(Byte* data, int *pSize);
        BE_SQLITE_EXPORT int Seek(int64_t offset, bool fromBeginning = true);
    };
};

//=======================================================================================
//! Table for "embedding" data files within a BeSQLite::Db. Embedded files are stored as a series of chunks as blobs, and
//! optionally the can be compressed. Files can either be embedded by importing via a local file, or by supplying an in-memory buffer.
//! Once embedded, files can either be exported into a new local file, or read into an in-memory buffer.
//! Entries are identified within the DbEmbeddedFileTable by a name, which is just a case-insensitive string that must be unique.
//! <p> You can use whatever technique you want to name your embedded files. But generally it is inadvisable to use the full
//! path of a physical file, since those paths will reference disks, directories, etc. that will likely not exist on other
//! target machines or operating systems.
//! <p> Optionally, entries can have a description string and a type string for filtering.
// @bsiclass
//=======================================================================================
struct DbEmbeddedFileTable
{
private:
    friend struct Db;
    DbR m_db;
    DbEmbeddedFileTable(DbR db) : m_db(db) {}

    BeBriefcaseBasedId GetNextEmbedFileId() const; //!< @private

public:
    //=======================================================================================
    //! An Iterator over the entries of a DbEmbeddedFileTable.
    // @bsiclass
    //=======================================================================================
    struct Iterator : DbTableIterator
    {
    public:
        explicit Iterator(DbCR db) : DbTableIterator(db) {}

        struct Entry : DbTableIterator::Entry
        {
            using iterator_category=std::input_iterator_tag;
            using value_type=Entry const;
            using difference_type=std::ptrdiff_t;
            using pointer=Entry const*;
            using reference=Entry const&;

        private:
            friend struct Iterator;
            Entry(StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            BE_SQLITE_EXPORT Utf8CP GetNameUtf8() const;          //!< the name of this embedded file.
            BE_SQLITE_EXPORT Utf8CP GetDescriptionUtf8() const;   //!< the description of this embedded file.
            BE_SQLITE_EXPORT Utf8CP GetTypeUtf8() const;          //!< the type of this embedded file.
            BE_SQLITE_EXPORT uint64_t GetFileSize() const;        //!< the total size, in bytes, of this embedded file.
            BE_SQLITE_EXPORT uint32_t GetChunkSize() const;       //!< the chunk size used to embed this file.
            BE_SQLITE_EXPORT DateTime GetLastModified() const;    //!< the time the file was last modified.
            BE_SQLITE_EXPORT BeBriefcaseBasedId GetId() const;   //!< the id of this embedded file.
            Entry const& operator* () const {return *this;}
        };

        typedef Entry const_iterator;
        typedef const_iterator iterator;
        BE_SQLITE_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry(nullptr, false);}
        BE_SQLITE_EXPORT size_t QueryCount() const;
    };
    //! return an iterator over this embedded file table.
    Iterator MakeIterator() const {return Iterator(m_db);}

    //! Query the values for an embedded file, by name.
    //! @param[in] name the (case insensitive) name by which the file was embedded.
    //! @param[out] totalSize the total size in bytes of the file. May be nullptr.
    //! @param[out] chunkSize the chunk size used to embed this file. May be nullptr.
    //! @param[out] descr the description of this file. May be nullptr.
    //! @param[out] typeStr the type of this file. May be nullptr.
    //! @param[out] lastModified the time the file was last modified. May be nullptr.
    //! @return the id  of the file, if found. If there is no entry of the given name, id will be invalid.
    BE_SQLITE_EXPORT BeBriefcaseBasedId QueryFile(Utf8CP name, uint64_t* totalSize = nullptr, DateTime* lastModified = nullptr,
        uint32_t* chunkSize = nullptr, Utf8StringP descr = nullptr, Utf8StringP typeStr = nullptr );
    BE_SQLITE_EXPORT BeBriefcaseBasedId ImportDbFile(DbResult& stat, Utf8CP name, Utf8CP localFileName, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr, uint32_t chunkSize = 500 * 1024, bool supportRandomAccess = true);

    //! Import a copy of an existing file from the local filesystem into this BeSQLite::Db.
    //! @param[out] stat Success or error code. May be nullptr.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] localFileName the name of a physical file on the local filesystem to be imported. The import will fail if the file doesn't
    //! exist or can't be read.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be nullptr.
    //! @param[in] description a string that describes this entry. May be nullptr.
    //! @param[in] lastModified the time the file was last modified. May be nullptr.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] supportRandomAccess if true, ignore the specified chunkSize and calculate it instead.  Generally, the default should be used.
    //! @return Id of the embedded file
    BE_SQLITE_EXPORT BeBriefcaseBasedId Import(DbResult* stat, bool compress, Utf8CP name, Utf8CP localFileName, DateTime const* lastModified = nullptr,
        Utf8CP typeStr = nullptr, Utf8CP description = nullptr, uint32_t chunkSize = 500 * 1024, bool supportRandomAccess = true);
    BE_SQLITE_EXPORT DbResult ExportDbFile(Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress=nullptr);

    //! Create a new file on the local file system with a copy of the content of an embedded file, by name.
    //! @param[in] localFileName the name for the new file. This method will fail if the file already exists or cannot be created.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] progress the interface to call to report progress.  May be nullptr.
    //! @return BE_SQLITE_OK if the file was successfully exported, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Export(Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress=nullptr);

    //! Extract and de-compress the specified embedded file to the supplied bvector.
    //! @param[in] buffer where to store the extracted bytes
    //! @param[in] name the name by which the file was embedded.
    //! @return BE_SQLITE_OK if the data was successfully extracted, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Read(ChunkedArray& buffer, Utf8CP name);

    //! Replace the content of a previously embedded file with the content of a different file on the local file system.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] localFileName the name of the new file to be embedded. This method will fail if the file does not exist or cannot be read.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] lastModified the time the file was last modified. May be nullptr.
    //! @return BE_SQLITE_OK if the new file was successfully replaced, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Replace(Utf8CP name, Utf8CP localFileName, DateTime const* lastModified = nullptr, uint32_t chunkSize=500*1024);

    //! Add a new entry into this embedded file table. This merely creates an entry and id for the file, it will have no content. This method is used to subsequently
    //! add data from an in-memory buffer via Save.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be nullptr.
    //! @param[in] description a string that describes this entry. May be nullptr.
    //! @param[in] lastModified the time the file was last modified. May be nullptr. If not specified, then no last-modifed time will be recorded for this embedded file.
    //! @return BE_SQLITE_OK if the entry was successfully added, and error status otherwise.
    BE_SQLITE_EXPORT DbResult AddEntry(Utf8CP name, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr);

    //! Save an in-memory buffer as the data for an entry in this embedded file table. This method will replace any existing data for the entry. The entry
    //! must have been previously created, either by Import or AddEntry.
    //! @param[in] data the file data to save.
    //! @param[in] size the number of bytes in data.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] lastModified the updated last-modified time to associate with the embedded file. May be nullptr. If Save is called after AddEntry, then it makes sense to pass nullptr.
    //! If Save is called to update an existing entry, then it makes sense to update its last-modified time. If not specified, then the last-modified time of the embedded file (if any) is left unchanged.
    //! @param[in] compress if true, the file will be compressed as it is imported. This makes the Db smaller, but is also slower to import and later read.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @return BE_SQLITE_OK if the entry was successfully saved, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Save(void const* data, uint64_t size, Utf8CP name, DateTime const* lastModified = nullptr, bool compress=true, uint32_t chunkSize=500*1024);


    //! Remove an entry from this embedded file table, by name. All of its data will be deleted from the Db.
    //! @param[in] name the name by which the file was embedded.
    //! @return BE_SQLITE_OK if the entry was successfully removed, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Remove(Utf8CP name);

    //! Create the Embedded file table if it doesn't exist
    DbResult CreateTable() const;
};

//! SQLite Transaction modes corresponding to https://www.sqlite.org/lang_transaction.html
enum class BeSQLiteTxnMode : int {None=0, Deferred=1, Immediate=2, Exclusive=3,};

//! Determines whether and how the default transaction should be started when a Db is created or opened.
enum class DefaultTxn : int
{
    //! Do not start a default transaction. This is generally not a good idea except for very specialized cases. All access to a database requires a transaction.
    //! So, unless you start a "default" transaction, you must wrap all SQL statements with a \ref Savepoint.
    No = (int) BeSQLiteTxnMode::None,
    //! Create a default "normal" transaction. SQLite will acquire locks as they are needed.
    Yes = (int) BeSQLiteTxnMode::Deferred,
    //! Create a default transaction using SQLite "immediate" mode. This acquires the "reserved" locks on the database (see http://www.sqlite.org/lang_transaction.html)
    //! when the file is opened and then attempts to reacquire them every time the default transaction is committed.
    Immediate = (int) BeSQLiteTxnMode::Immediate,
    //! Create a default transaction using SQLite "exclusive" mode. This acquires all locks on the database (see http://www.sqlite.org/lang_transaction.html)
    //! when the file is opened. The locks are never released until the database is closed. Only exclusive access guarantees that no other process will be able to
    //! gain access (and thereby block access from this connection) to the file when the default transaction is committed. Use of DefaultTxn_Exclusive requires
    //! that the database be opened for read+write access and the open will fail if you attempt to use it on a readonly connection.
    Exclusive = (int) BeSQLiteTxnMode::Exclusive
};

//=======================================================================================
//! Savepoint encapsulates SQLite transactions against a BeSQLite::Db. Savepoint is implemented using
//! SQLite BEGIN, COMMMIT and SAVEPOINT statements, so they can nest (see http://www.sqlite.org/lang_savepoint.html for details). There is no way to
//! hold a BeSQLite transaction open without a Savepoint object. That is, no transaction survives beyond the scope of its
//! Savepoint object. However, Savepoint can be committed, and restarted, and canceled within their lifecycle.
//! <p><h2>Default Transactions</h2>
//! When you create or open a BeSQLite::Db, you can optionally specify that you wish to open a "default transaction". Every BeSQLite::Db holds a
//! Savepoint for this purpose. The default transaction Savepoint remains active for the lifecycle of the BeSQLite::Db, and can be
//! committed and canceled using the Db::   SaveChanges and Db::AbandonChanges methods, respectively.
//! <p>
//! Savepoints are auto-committed on destruction. If you wish to abandon the changes made in a Savepoint, you must call
//! Savepoint::Cancel() on it before it is destroyed.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Savepoint : NonCopyableClass
{
#if !defined (DOCUMENTATION_GENERATOR)
protected:
    friend struct DbFile;
    Db*         m_db;
    DbFile*     m_dbFile;
    int32_t     m_depth;
    BeSQLiteTxnMode  m_txnMode;
    Utf8String  m_name;

    virtual void _OnDeactivate(bool isCommit) {m_depth=-1;}
    BE_SQLITE_EXPORT virtual DbResult _Begin(BeSQLiteTxnMode);
    BE_SQLITE_EXPORT virtual DbResult _Commit(Utf8CP operation);
    BE_SQLITE_EXPORT virtual DbResult _Cancel();

    Savepoint(DbFile& db, Utf8CP name, BeSQLiteTxnMode txnMode) : m_dbFile(&db), m_name(name), m_txnMode(txnMode) {m_db=nullptr; m_depth = -1;}
#endif

public:
    //! Construct a Savepoint against a BeSQLite::Db. Savepoints must have a name and can be nested. Optionally, the savepoint can be started (via Begin)
    //! @param[in] db the database for this Savepoint.
    //! @param[in] name the name of this Savepoint. Does not have to be unique, but must not be nullptr.
    //! @param[in] beginTxn if true Begin() is called in constructor. To see if Begin was successful, check IsActive.
    //! @param[in] txnMode the default transaction mode for this Savepoint
    BE_SQLITE_EXPORT Savepoint(Db& db, Utf8CP name, bool beginTxn=true, BeSQLiteTxnMode txnMode=BeSQLiteTxnMode::Deferred);

    //! dtor for Savepoint. If the Savepoint is still active, it is committed.
    BE_SQLITE_EXPORT virtual ~Savepoint();

    //! Determine whether this Savepoint is active or not.
    bool IsActive() const {return GetDepth() >= 0;}

    //! Get the savepoint depth for this Savepoint. This will always be a number >= 0 if the savepoint is active.
    int32_t GetDepth() const {return m_depth;}

    //! Get the name of this Savepoint.
    Utf8CP GetName() const {return m_name.c_str();}

    //! Get the default transaction mode for this SavePoint
    BeSQLiteTxnMode GetTxnMode() const {return m_txnMode;}

    //! Sets the default transaction mode for this Savepoint.
    void SetTxnMode(BeSQLiteTxnMode mode) {m_txnMode = mode;}

    //! Begin a transaction against the database. Fails if this Savepoint is already active.
    //! See SQLite "BEGIN" and "SAVEPOINT" documentation for other conditions.
    //! @param[in] mode The transaction mode for this transaction
    BE_SQLITE_EXPORT DbResult Begin(BeSQLiteTxnMode mode);

    //! Begin a transaction against the database with the default transaction mode
    DbResult Begin() {return Begin(m_txnMode);}

    //! Commit this Savepoint, if active. Executes the SQLite "RELEASE" command. If this is the outermost transaction, this will save changes
    //! to the disk.
    //! @param[in] operation The name of the operation being committed. If change trackers are active, they may save this string along with the
    //! changeset to identify it.
    //! @note After Commit, the Savepoint is not active. If you mean to save your changes but leave this Savepoint open, call Save.
    //! @note If a Savepoint is active when it is destroyed, it is automatically committed.
    BE_SQLITE_EXPORT DbResult Commit(Utf8CP operation=nullptr);

    //! Commit this transaction and then restart it. If this is the outermost Savepoint, this will save changes to disk but leave
    //! a new transaction active.
    BE_SQLITE_EXPORT DbResult Save(Utf8CP operation);

    //! Cancel this transaction. Executes the SQLite "ROLLBACK" command if the Savepoint is active.
    BE_SQLITE_EXPORT DbResult Cancel();
};

//=======================================================================================
//! DbBuffer use SQLite memory management api https://www.sqlite.org/c3ref/free.html
//! It can be used with certain sqlite api which return buffer that need to be freed by
//! caller code.
// @bsiclass
//=======================================================================================
struct DbBuffer final {
    using SqlMemP = void*;
    using SqlMemCP = const void*;
    using ByteP = uint8_t*;
    using ByteCP = const uint8_t*;
    private:
        SqlMemP m_buff;
        uint64_t m_size;

        void EnsureCapacity(uint64_t);
        void Realloc(uint64_t);

        BE_SQLITE_EXPORT void Free();
        BE_SQLITE_EXPORT void Alloc(uint64_t);
        BE_SQLITE_EXPORT void CopyFrom(SqlMemCP, uint64_t);
        BE_SQLITE_EXPORT void MoveFrom(DbBuffer& buff);
        BE_SQLITE_EXPORT void TakeOwnership(SqlMemP, uint64_t);

    public:
        DbBuffer(): m_size(0),m_buff(0){}
        explicit DbBuffer(uint64_t size): m_size(0),m_buff(0) { Alloc(size); }
        DbBuffer(SqlMemCP buff, uint64_t sz): m_size(0),m_buff(0) { CopyFrom(buff, sz); }
        DbBuffer(SqlMemP buff, uint64_t sz): m_size(0),m_buff(0) { TakeOwnership(buff, sz); }
        DbBuffer(DbBuffer&& rhs): m_size(0),m_buff(0) { MoveFrom(rhs); };
        DbBuffer(DbBuffer const& rhs): m_size(0),m_buff(0) { CopyFrom(rhs.m_buff, rhs.m_size); };
        DbBuffer& operator = (DbBuffer&& rhs) { MoveFrom(rhs); return *this;}
        DbBuffer& operator = (DbBuffer const& rhs) { CopyFrom(rhs.m_buff, rhs.m_size); return *this;}
        ~DbBuffer() { Free(); }

        explicit operator SqlMemCP() const { return m_buff; }
        explicit operator SqlMemP() { return const_cast<SqlMemP>(m_buff); }
        explicit operator ByteCP() const { return (ByteCP)(SqlMemCP)*this;  }
        explicit operator ByteP () { return (ByteP)(SqlMemP)*this;  }

        //! return ptr to allocated memory
        SqlMemP DataP() const { return const_cast<SqlMemP>(m_buff); }

        //! return const ptr to allocated memory
        SqlMemCP Data() const { return m_buff; }

        //! return size of buffer which can be less or equal to Capacity()
        uint64_t Size() const { return m_size; }

        //! check if buffer has been allocated
        bool Empty() const { return m_buff == nullptr && m_size == 0; }

        //! return total size of allocated memory which can be larger then what returned by Size()
        BE_SQLITE_EXPORT uint64_t Capacity() const;

        //! fill buffer with zeros
        BE_SQLITE_EXPORT void Zero();

        //! detach from memory making someone else responsible to free it.
        BE_SQLITE_EXPORT std::pair<SqlMemP,uint64_t> Detach();

        //! see sqlite3_free in https://www.sqlite.org/c3ref/free.html
        BE_SQLITE_EXPORT static void SqliteFree(SqlMemP);

        //! see sqlite3_malloc64 in https://www.sqlite.org/c3ref/free.html
        BE_SQLITE_EXPORT static SqlMemP SqliteAlloc(uint64_t);

        //! see sqlite3_realloc64 in https://www.sqlite.org/c3ref/free.html
        BE_SQLITE_EXPORT static SqlMemP SqliteRealloc(SqlMemP, uint64_t);

        //! see sqlite3_msize in https://www.sqlite.org/c3ref/free.html
        BE_SQLITE_EXPORT static uint64_t SqliteMSize(SqlMemP);
};

//=======================================================================================
//! Every BeSQLite::Db has a table for storing "Properties".
//! Properties are essentially "named values" and are used to store "non-SQL" information in the database. Generally, for each type of
//! Property there are zero, one, or perhaps a small set of instances and they therefore don't warrant having their own table. Property values
//! have both a blob and a text string. Either or both may be nullptr (and typically one or the other is.)
//! For example, the GUID of the database itself is stored in the Properties table.
//! <p>
//! This class forms a "specification" to save and retrieve a type of Property. The specification has three parts: a "Name" string, a "Namespace"
//! and a flag indicating how the property is to be transacted. The combination of Name and Namespace must be unique. The Namespace part
//! identifies a group of Properties that are related somehow. The Name part specifies the Property's meaning within the Namespace. When instances of properties
//! are saved or queried, they are identified by two ids (called "Id" and "SubId"), forming a 2 dimensional array of instances of a given PropertySpec.
//! <h2>Cached Properties</h2>
//! Some properties hold values that are accessed or changed frequently. While the property system is relatively efficient, its primary goal is
//! not access speed, and does not use any memory except while accessing properties. By specifying txnMode=TXN_MODE_Cached, a property's value will
//! be held in memory as it is queried or changed. Changes are saved when the transaction is committed. This makes any such property faster to access at the expense of memory. This mode should be
//! used judiciously, and reserved for only the few properties that warrant it.
// @bsiclass
//=======================================================================================
struct PropertySpec
{
public:
    //! Determine whether a property's data may be compressed.
    enum class Compress
    {
        //! never compress this property
        No=0,

        //! the property value may be compressed before it is saved. The property won't necessarily be compressed unless its size is large enough
        //! (usually 100 bytes) and the actual compression results in a net savings.
        Yes=1
    };

    enum class Mode {Normal=0, Cached=2,};

private:
    Mode m_mode;
    Compress m_compress;
    bool   m_saveIfNull;
    Utf8CP m_name;
    Utf8CP m_namespace;

public:
    //! Construct an instance of a PropertySpec with values for name and namespace
    //! @param[in] name The Name part of this Property specification
    //! @param[in] nameSpace The Namespace part of this Property specification.
    //! @param[in] mode the transaction mode for this property.
    //! @param[in] compress If Compress::Yes, the property value may be compressed before it is saved. The property won't necessarily be compressed unless
    //!            its size is large enough (usually 100 bytes) and the actual compression results in a net savings.
    //! @param[in] saveIfNull If true, this property will be saved even if its value is nullptr. Otherwise it is deleted if its value is Null.
    //! @note name and namespace should always point to static strings.
    PropertySpec(Utf8CP name, Utf8CP nameSpace, Mode mode=Mode::Normal, Compress compress=Compress::Yes, bool saveIfNull=false) : m_name(name), m_namespace(nameSpace), m_compress(compress), m_mode(mode), m_saveIfNull(saveIfNull){}

    //! Copy a PropertySpec, changing only the mode
    PropertySpec(PropertySpec const& other, Mode mode) {*this = other; m_mode=mode;}

    Utf8CP GetName() const {return m_name;}
    Utf8CP GetNamespace() const {return m_namespace;}

    bool IsCached()  const {return Mode::Cached == m_mode;}     //!< Determine whether this PropertySpec is cached or not.
    bool SaveIfNull() const {return m_saveIfNull;}              //!< Determine whether this PropertySpec saves NULL values or not.
    bool IsCompress() const {return Compress::Yes==m_compress;} //!< Determine whether this PropertySpec requests to compress or not.
};

typedef PropertySpec const& PropertySpecCR;

//=======================================================================================
//! Supply a BusyRetry handler to BeSQLite (see https://www.sqlite.org/c3ref/busy_handler.html).
// @bsiclass
//=======================================================================================
struct BusyRetry : RefCountedBase {
private:
    uint32_t m_timeout;
    int m_retries;

public:
    BusyRetry(int retries = 5, uint32_t timeout = 1000) : m_retries(retries), m_timeout(timeout) {}

    //! Called when SQLite is blocked by another connection to a database. The default implementation performs 5 retries
    //! with a 1 second delay. Subclasses can change the timeout period or interact with the user, if necessary.
    //! @param[in] count the number of times this method has been called for this busy event.
    //! @return 0 to stop retrying and return a BE_SQLITE_BUSY error. Any non-zero value will attempt another retry.
    BE_SQLITE_EXPORT virtual int _OnBusy(int count) const;
};

//=======================================================================================
// Cached "briefcase local values"
//! @private
// @bsiclass
//=======================================================================================
struct CachedBLV
{
private:
    Utf8String   m_name;
    bool         m_isUnset;
    mutable bool m_dirty;
    uint64_t     m_value;

public:
    explicit CachedBLV(Utf8CP name);
    Utf8CP GetName() const {return m_name.c_str();}
    uint64_t GetValue() const {BeAssert(!m_isUnset); return m_value;}
    void ChangeValue(uint64_t value, bool initializing = false);
    uint64_t Increment();
    bool IsUnset() const {return m_isUnset;}
    bool IsDirty() const {BeAssert(!m_isUnset); return m_dirty;}
    void SetIsNotDirty() const;
    void Reset();
};

//=======================================================================================
// Cache for uint64_t BriefcaseLocalValues. This is for BLV's that are of type uint64_t and are frequently
// accessed and/or modified. The BLVs are identified in the cache by "registering" their name are thereafter
// accessed by index. The cache is held in memory for performance. It is automatically saved whenever a transaction is committed.
//! @private
// @bsiclass
//=======================================================================================
struct BriefcaseLocalValueCache : NonCopyableClass
{
private:
    friend struct DbFile;
    friend struct Db;
    bvector<CachedBLV> m_cache;
    DbFile& m_dbFile;
    mutable BeMutex m_mutex;

    void Clear();
    bool TryQuery(CachedBLV*&, size_t rlvIndex);

public:
    BriefcaseLocalValueCache(DbFile& dbFile) : m_dbFile(dbFile) {}

    //! Register a BriefcaseLocalValue name with the Db.
    //! @remarks On closing the Db the registration is cleared.
    //! @param[out] rlvIndex Index for the BriefcaseLocalValue used as input to the BriefcaseLocalValue API.
    //! @param[in] rlvName Name of the BriefcaseLocalValue. Db does not make a copy of @p rlvName, so the caller
    //! has to ensure that it remains valid for the entire lifetime of the Db object.
    //! @return BE_SQLITE_OK if registration was successful. Error code, if a BriefcaseLocalValue with the same
    //! name has already been registered.
    BE_SQLITE_EXPORT DbResult Register(size_t& rlvIndex, Utf8CP rlvName);

    //! Look up the BriefcaseLocalValue index for the given name
    //! @param[out] rlvIndex Found index for the BriefcaseLocalValue
    //! @param[in] rlvName Name of the BriefcaseLocalValue
    //! @return true, if the BriefcaseLocalValue index was found, i.e. a BriefcaseLocalValue is registered for @p rlvName,
    //! false otherwise.
    BE_SQLITE_EXPORT bool TryGetIndex(size_t& rlvIndex, Utf8CP rlvName);

    //! Save an BriefcaseLocalValue into the BEDB_TABLE_Local table.
    //! @param[in] rlvIndex The index of the BriefcaseLocalValue to query.
    //! @param[in] value Value to save
    //! @return BE_SQLITE_OK if successful, error code otherwise.
    //! @see RegisterBriefcaseLocalValue
    BE_SQLITE_EXPORT DbResult SaveValue(size_t rlvIndex, uint64_t value);

    //! Read a BriefcaseLocalValue from BEDB_TABLE_Local table.
    //! @param[out] value Retrieved value
    //! @param[in] rlvIndex The index of the BriefcaseLocalValue to query.
    //! @return BE_SQLITE_OK if the value exists, error code otherwise.
    //! @see RegisterBriefcaseLocalValue
    BE_SQLITE_EXPORT DbResult QueryValue(uint64_t& value, size_t rlvIndex);

    //! Increment the BriefcaseLocalValue by one for the given @p rlvIndex.
    //! @param[out] newValue Incremented value
    //! @param[in] rlvIndex The index of the BriefcaseLocalValue to increment.
    //! @return BE_SQLITE_OK in case of success. Error code otherwise. If initialValue is nullptr and
    //!         the BriefcaseLocalValue does not exist, and error code is returned.
    //! @see RegisterBriefcaseLocalValue
    BE_SQLITE_EXPORT DbResult IncrementValue(uint64_t& newValue, size_t rlvIndex);

    // save if supplied value is greater than current value.
    BE_SQLITE_EXPORT DbResult CheckMaxValue(uint64_t value, size_t rlvIndex);
};

//=======================================================================================
//! The Db file's profile version compared to what the current software expects.
// @bsiclass
//=======================================================================================
struct ProfileState final
    {
    enum class Age
        {
        Newer,
        UpToDate,
        Older
        };

    enum class CanOpen
        {
        Readwrite, //<! File can be opened without restrictions
        Readonly, //<! File can only be opened read-only
        No //<! File cannot be opened
        };

    private:
        Age m_age = Age::UpToDate;
        CanOpen m_canOpen = CanOpen::Readwrite;
        bool m_isUpgradable = false;

        ProfileState(Age age, CanOpen canOpen, bool isUpgradable) : m_age(age), m_canOpen(canOpen), m_isUpgradable(isUpgradable) {}
    public:
        static ProfileState UpToDate() { return ProfileState(Age::UpToDate, CanOpen::Readwrite, false); }
        static ProfileState Newer(CanOpen canOpen) { return ProfileState(Age::Newer, canOpen, false); }
        static ProfileState Older(CanOpen canOpen, bool isUpgradable) { return ProfileState(Age::Older, canOpen, isUpgradable); }
        static ProfileState Error() { return ProfileState(Age::UpToDate, CanOpen::No, false); }

        bool operator==(ProfileState const& rhs) const { return m_age == rhs.m_age && m_canOpen == rhs.m_canOpen && m_isUpgradable == rhs.m_isUpgradable; }
        bool operator!=(ProfileState const& rhs) const { return !(*this == rhs); }

        bool IsError() const { return m_age == Age::UpToDate && m_canOpen == CanOpen::No; }
        bool IsUpToDate() const { return m_age == Age::UpToDate; }
        bool IsOlder() const { return m_age == Age::Older; }
        bool IsNewer() const { return m_age == Age::Newer; }

        //! Indicates whether the file is up-to-date or older or newer than what the software expects.
        Age GetAge() const { return m_age; }
        CanOpen GetCanOpen() const { return m_canOpen; }
        //! Indicates whether the file can be upgraded. If yes, open the file with ProfileUpgradeOptions::Upgrade.
        bool IsUpgradable() const { return m_isUpgradable; }

        BE_SQLITE_EXPORT DbResult ToDbResult() const;
        BE_SQLITE_EXPORT ProfileState Merge(ProfileState const& rhs) const;
    };

//=======================================================================================
//! A physical Db file.
// @bsiclass
//=======================================================================================
struct DbFile : NonCopyableClass
{
    friend struct Db;
    friend struct Statement;
    friend struct Savepoint;

protected:
    typedef RefCountedPtr<struct ChangeTracker> ChangeTrackerPtr;
    typedef RefCountedPtr<BusyRetry> BusyRetryPtr;
    typedef bvector<Savepoint*> DbTxns;
    typedef DbTxns::iterator DbTxnIter;
    struct TraceEvents: NonCopyableClass
        {
        DbTrace m_traceFlags;
        TraceRowEvent m_traceRowEvent;
        TraceStmtEvent m_traceStmtEvent;
        TraceProfileEvent m_traceProfileEvent;
        TraceCloseEvent m_traceCloseEvent;
        };

    mutable std::unique_ptr<TraceEvents> m_traceEvents;
    TraceEvents& GetTraceEvents() const;
    bool m_allowImplicitTxns;
    bool m_inCommit;
    mutable bool m_readonly;
    uint64_t m_dataVersion; // for detecting changes from another process
    SqlDbP m_sqlDb;
    BusyRetryPtr m_retry;
    ChangeTrackerPtr m_tracker;
    mutable void* m_cachedProps;
    BriefcaseLocalValueCache m_blvCache;
    BeGuid m_dbGuid;
    Savepoint m_defaultTxn;
    BeBriefcaseId m_briefcaseId;
    StatementCache m_statements;
    DbTxns m_txns;
    std::unique_ptr<ScalarFunction> m_regexFunc, m_regexExtractFunc;
    explicit DbFile(SqlDbP sqlDb, BusyRetry* retry, BeSQLiteTxnMode defaultTxnMode);
    ~DbFile();
    DbResult StartSavepoint(Savepoint&, BeSQLiteTxnMode);
    DbResult StopSavepoint(Savepoint&, bool isCommit, Utf8CP operation);
    void DeactivateSavepoint(Savepoint&);
    void DeactivateSavepoints(DbTxnIter, bool isCommit);
    DbResult CreatePropertyTable(Utf8CP tablename, Utf8CP ddl);
    DbResult SaveCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId, Utf8CP stringData, void const* value, uint32_t size) const;
    struct CachedPropertyMap& GetCachedPropMap() const;
    struct CachedPropertyValue& GetCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    struct CachedPropertyValue* FindCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    DbResult QueryCachedProperty(Utf8String*, void** value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    void DeleteCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId);
    void DeleteCachedPropertyMap();
    void SaveCachedProperties(bool isCommit);
    Utf8String GetLastError(DbResult* lastResult) const;
    void SaveCachedBlvs(bool isCommit);
    BE_SQLITE_EXPORT DbResult SaveProperty(PropertySpecCR spec, Utf8CP strData, void const* value, uint32_t propsize, uint64_t majorId=0, uint64_t subId=0);
    BE_SQLITE_EXPORT bool HasProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryPropertySize(uint32_t& propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryProperty(void* value, uint32_t propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult DeleteProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0);
    BE_SQLITE_EXPORT DbResult DeleteProperties(PropertySpecCR spec, uint64_t* majorId);
    BE_SQLITE_EXPORT int AddFunction(DbFunction& function) const;
    BE_SQLITE_EXPORT int AddRTreeMatchFunction(RTreeMatchFunction& function) const;
    BE_SQLITE_EXPORT int RemoveFunction(DbFunction&) const;
    BE_SQLITE_EXPORT BriefcaseLocalValueCache& GetBLVCache();
    BE_SQLITE_EXPORT DbTrace ConfigTraceEvents(DbTrace, bool) const;
    BE_SQLITE_EXPORT void DisableAllTraceEvents() const;
    void AddDataUpdateCallback(DataUpdateCallback&) const;
    void RemoveDataUpdateCallback() const;
    static int TraceCallback(unsigned, void*, void*, void*);
public:
    Utf8String ExplainQuery(Utf8CP sql, bool explainPlan, bool suppressDiagnostics) const;
    int OnCommit();

    bool CheckImplicitTxn() const {return m_allowImplicitTxns || m_txns.size() > 0;}
    SqlDbP GetSqlDb() const {return m_sqlDb;}
    BE_SQLITE_EXPORT DbTxnState GetTxnState(Utf8CP schema = nullptr) const;
};

//=======================================================================================
//! A BeSQLite database file. This class is a wrapper around the SQLite datatype "sqlite3"
//! @note Refer to the SQLite documentation for the details of SQLite.
//! @nosubgrouping
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Db : NonCopyableClass
{
    friend struct DbFile;
public:
    enum class Encoding {Utf8=0, Utf16=1};
    enum class PageSize : int
    {
        PAGESIZE_1K  = 1024,
        PAGESIZE_512 = PAGESIZE_1K / 2,
        PAGESIZE_2K  = PAGESIZE_1K * 2,
        PAGESIZE_4K  = PAGESIZE_1K * 4,
        PAGESIZE_8K  = PAGESIZE_1K * 8,
        PAGESIZE_16K = PAGESIZE_1K * 16,
        PAGESIZE_32K = PAGESIZE_1K * 32,
        PAGESIZE_64K = PAGESIZE_1K * 64
    };

    //! Whether to open an BeSQLite::Db readonly or readwrite.
    enum class OpenMode {Readonly = 1<<0, ReadWrite = 1<<1, Create = ReadWrite|(1<<2)};

    //=======================================================================================
    //! Options that control whether a profile upgrade should be performed when opening a file
    // @bsienum                                                     06/18
    //=======================================================================================
    enum class ProfileUpgradeOptions
        {
        None, //<! No profile upgrade will be performed. If a profile upgrade was required, opening the file will fail
        Upgrade, //<! Profile upgrade will be performed if necessary.
        };

    //=======================================================================================
    //! Parameters for controlling aspects of the opening of a Db.
    // @bsiclass
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams {
        mutable OpenMode m_openMode;
        DefaultTxn m_startDefaultTxn = DefaultTxn::Yes;
        ProfileUpgradeOptions m_profileUpgradeOptions = ProfileUpgradeOptions::None;
        bool m_rawSQLite = false;

        // Skip the check for SQLite file validity before opening. When using the CloudSqlite mode, the local file is not in SQLite normal format.
        bool m_skipFileCheck = false;

        BusyRetry* m_busyRetry = nullptr;
        mutable bvector<Utf8String> m_queryParams;

        // The "base" name that can be used for creating temporary files related to this Db. The string should be a name related to the current Db filename using
        // some known pattern so that all files named "baseName*" can be deleted externally during cleanup. It must be the name of a file (that may or may not exist)
        // in a writable directory. If not present, use the db file name.
        Utf8String m_tempfileBase;

        //! @param[in] openMode The mode for opening the database
        //! @param[in] startDefaultTxn Whether to start a default transaction on the database.
        //! @param[in] retry Supply a BusyRetry handler for the database connection. The BeSQLite::Db will hold a ref-counted-ptr to the retry object.
        //!                  The default is to not attempt retries Note, many BeSQLite applications (e.g. Bim) rely on a single non-shared connection
        //!                  to the database and do not permit sharing.
        OpenParams(OpenMode openMode, DefaultTxn startDefaultTxn = DefaultTxn::Yes, BusyRetry* retry = nullptr) : m_openMode(openMode), m_startDefaultTxn(startDefaultTxn), m_busyRetry(retry) {}

        //! @param[in] openMode The mode for opening the database
        //! @param[in] profileUpgradeOptions Options for a profile upgrade. If an upgrade is to be performed, the Db must be opened readwrite.
        OpenParams(OpenMode openMode, ProfileUpgradeOptions profileUpgradeOptions) : m_openMode(openMode), m_profileUpgradeOptions(profileUpgradeOptions) { BeAssert(!IsReadonly() || m_profileUpgradeOptions != ProfileUpgradeOptions::Upgrade && "ProfileUpgrade requires readwrite"); }

        //! Sets the Profile Upgrade options.
        //! @note If ProfileUpgradeOptions::Upgrade is specified, the Db must be opened readwrite.
        //! @param[in] options Profile Upgrade options.
        OpenParams& SetProfileUpgradeOptions(ProfileUpgradeOptions options) {
            m_profileUpgradeOptions = options;
            return *this;
        }
        //! Gets the Profile Upgrade options
        ProfileUpgradeOptions GetProfileUpgradeOptions() const { return m_profileUpgradeOptions; }

        //! Use the BeSQLite::Db api on a "raw" SQLite file. This allows use of the BeSQLite api on SQLite databases it did not create.
        //! When BeSQLite::Db creates a database, it adds the BEDB_TABLE_Property and BEDB_TABLE_Local tables
        //! for the Property and RepositoryLocalValue methods. If you attempt to open a SQLite file that doesn't have those
        //! tables, BeSQLite:Db::OpenBeSQLiteDb will fail with the status BE_SQLITE_ERROR_NoPropertyTable. When the "raw sqlite" flag
        //! is on, BeSQLite::Db::OpenBeSQLiteDb will not check for the presence of those tables, and BeSQLite::Db::CreateNewDb will not
        //! create those tables.
        //! @note When this flag is on, all of the Property and RepositoryLocalValue methods of BeSQLite::Db will fail.
        void SetRawSQLite() { m_rawSQLite = true; }

        //! Determine whether the "raw sqlite" flag is on or not
        bool IsRawSQLite() const { return m_rawSQLite; }

        //! Determine whether the open mode is readonly or not
        bool IsReadonly() const { return m_openMode == OpenMode::Readonly; }

        //! set the open mode
        void SetOpenMode(OpenMode openMode) { m_openMode = openMode; }

        //! Determine whether the default transaction should be started on the Db. If DefaultTxn_Yes, the default transaction
        //! will be started on the Db after it is opened. This applies only to the connection returned by
        //! Db::CreateNewDb or Db::OpenBeSQLiteDb; it is not a persistent property of the Db.
        void SetStartDefaultTxn(DefaultTxn val) { m_startDefaultTxn = val; }

        //! Sets a BusyRetry handler
        //! @param[in] retry A BusyRetry handler for the database connection. The BeSQLite::Db will hold a ref-counted-ptr to the retry object.
        //!                  The default is to not attempt retries. Note, many BeSQLite applications (e.g. Bim) rely on a single non-shared connection
        //!                  to the database and do not permit sharing.
        void SetBusyRetry(BusyRetry* retry) { m_busyRetry = retry; }

        //! Open the database as "immutable". This means that SQLite will not hold any locks on the file. Only use this if you're *sure* the
        //! file cannot be changed by any other processes, otherwise very bad things can happen.
        //! @see https://www.sqlite.org/c3ref/open.html
        void SetImmutable() { AddQueryParam("immutable=1"); }

        //! Set a query parameter for open/create call
        void AddQueryParam(Utf8CP value) const { m_queryParams.push_back(value); }

        BE_SQLITE_EXPORT Utf8String SetFromContainer(Utf8CP dbName, CloudContainerP container);
    };

    //=======================================================================================
    //! Parameters for controlling aspects of creating and then opening a Db.
    // @bsiclass
    //=======================================================================================
    struct CreateParams : OpenParams
    {
        Encoding m_encoding;
        PageSize m_pagesize;
        enum ApplicationId : uint64_t {APPLICATION_ID_BeSQLiteDb='BeDb',} m_applicationId;
        bool m_failIfDbExists;
        DateTime m_expirationDate;

        //! @param[in] pagesize The pagesize for the database. Default is 4K.
        //! @param[in] encoding The text encoding mode for the database. The default is UTF-8 and is almost always the best choice.
        //! @param[in] failIfDbExists If true, return an error if a the specified file already exists, otherwise delete existing file.
        //! @param[in] defaultTxn Whether to start the default transaction against the newly created Db. @see SetStartDefaultTxn
        //! @param[in] retry Supply a BusyRetry handler for the database connection. The BeSQLite::Db will hold a ref-counted-ptr to the retry object.
        //!                  The default is to not attempt retries Note, many BeSQLite applications (e.g. Bim) rely on a single non-shared connection
        //!                  to the database and do not permit sharing.
        //! @param[in] dbType Option to create a standalone or a master database.
        explicit CreateParams(PageSize pagesize=PageSize::PAGESIZE_4K, Encoding encoding=Encoding::Utf8, bool failIfDbExists=true,
                DefaultTxn defaultTxn=DefaultTxn::Yes, BusyRetry* retry=nullptr)
                : OpenParams(OpenMode::Create, defaultTxn, retry), m_encoding(encoding), m_pagesize(pagesize),
                m_failIfDbExists(failIfDbExists), m_applicationId(APPLICATION_ID_BeSQLiteDb)
            {}

        //! Set the page size for the newly created database.
        void SetPageSize(PageSize pagesize) {m_pagesize = pagesize;}
        //! Set the text encoding for the newly created database.
        void SetEncoding(Encoding encoding) {m_encoding = encoding;}
        //! Determine whether the create should fail if a file exists with the specified name.
        void SetFailIfDbExist(bool val) {m_failIfDbExists = val;}
        //! Set Application Id to be stored at offset 68 of the SQLite file
        void SetApplicationId(ApplicationId applicationId) {m_applicationId=applicationId;}
        //! Set expiration date for the newly created database.
        void SetExpirationDate(DateTime xdate) {m_expirationDate=xdate;}
    };
    //=======================================================================================
    //! Applications subclass from this class to store in-memory data on a Db via AddAppData.
    // @bsiclass
    //=======================================================================================
    struct AppData : RefCountedBase
    {
        struct Key : NonCopyableClass {};
    };

    using AppDataPtr = RefCountedPtr<AppData>;

protected:

    //=======================================================================================
    // @bsistruct
    //=======================================================================================
    struct AppDataCollection
    {
        using AppData = Db::AppData;
        using AppDataPtr = Db::AppDataPtr;
    private:
        typedef bmap<AppData::Key const*, AppDataPtr, std::less<AppData::Key const*>, 8> Map;

        BeMutex     m_mutex;
        Map         m_map;

        BE_SQLITE_EXPORT void AddInternal(AppData::Key const& key, AppData* data);
        BE_SQLITE_EXPORT AppData* FindInternal(AppData::Key const& key);
    public:
        void Add(AppData::Key const& key, AppData* data) { BeMutexHolder lock(m_mutex); return AddInternal(key, data); }
        StatusInt Drop(AppData::Key const& key);
        AppData* FindRaw(AppData::Key const& key) { BeMutexHolder lock(m_mutex); return FindInternal(key); }
        AppDataPtr Find(AppData::Key const& key) { return FindRaw(key); }
        template<typename T> AppDataPtr FindOrAdd(AppData::Key const& key, T createAppData)
            {
            BeMutexHolder lock(m_mutex);
            AppData* data = FindInternal(key);
            if (nullptr == data)
                {
                data = createAppData();
                AddInternal(key, data);
                }

            return data;
            }

        void Clear() { BeMutexHolder lock(m_mutex); m_map.clear(); }
    };

    DbFile* m_dbFile;
    StatementCache m_statements;
    DbEmbeddedFileTable m_embeddedFiles;
    mutable AppDataCollection m_appData;

    // The query params used to open this database, so that code that opens another connection can reuse them.
    bvector<Utf8String> m_openQueryParams;

    // a fileName that can be used as the "base" for creating temporary files related to this Db. If not present, use the file name.
    Utf8String m_tempfileBase;

    //! Called after a new Db had been created.
    //! Override to perform additional processing when Db is created
    //! @param[in] params - Create parameter used to create the Db.
    //! @return BE_SQLITE_OK on success, error status otherwise
    //! @note implementers should always forward this call to their superclass.
    virtual DbResult _OnDbCreated(CreateParams const& params) {return BE_SQLITE_OK;}

    //! override to perform additional processing when Db is opened
    //! @note implementers should always forward this call to their superclass.
    //! @note this function is invoked before _CheckProfileVersion() and _UpgradeProfile() and therefore should not attempt to access data which depends on the schema version
    virtual DbResult _OnDbOpening() {return QueryDbIds();}

    //! override to perform additional processing when Db is opened
    //! @param[in] params Open parameters
    //! @note implementers should always forward this call to their superclass.
    //! @note this function is invoked after _CheckProfileVersion() and _UpgradeProfile() and can therefore access data which depends on the schema version
    virtual DbResult _OnDbOpened(OpenParams const& params) { return BE_SQLITE_OK; }

    //! override to perform processing when Db is closed
    //! @note implementers should always forward this call to their superclass.
    virtual void _OnDbClose() {}

    //! Called when a new transaction is started on a connection, and a different connection (either in the same process or from another process)
    //! has changed the database since the last transaction from this connection was committed. This gives subclasses an opportunity to clear internal
    //! caches or user interface that may now be invalid.
    //! @note This method is only relevant for connections opened with DefaultTxn::No, since when a default transaction is active, other
    //! processes are blocked from making changes to the database.
    //! @note implementers should always forward this call to their superclass.
    BE_SQLITE_EXPORT virtual void _OnDbChangedByOtherConnection();

    virtual void _OnBeforeSetBriefcaseId(BeBriefcaseId newId) {}
    virtual void _OnAfterSetBriefcaseId() {}
    virtual void _OnDbGuidChange(BeGuid guid) {}

    //! Gets called when a Db is opened and checks whether the file can be opened, i.e
    //! whether the file version matches what the opening API expects.
    //!
    //! Subclasses have to override this method to perform the profile version check when the file is opened.
    //! See Db::OpenBeSQLiteDb for the <b>compatibility rules</b> that subclasses have to implement
    //! in this method.
    //!
    //! ####Developer Checklist
    //! When a profile needs to be modified,
    //! 1. the profile version needs to be incremented according to the
    //! <b>compatibility rules</b> mentioned above. This means concretely:
    //!     - If older versions of the software can still work with the file without any restrictions:
    //!       @b increment the @ref ProfileVersion::GetSub2 "Sub2" version digit.
    //!     - If older versions of the software can only open the file in read-only mode:
    //!       @b increment the @ref ProfileVersion::GetSub1 "Sub1" version digit.
    //!     - If older versions of the software can no longer work with the file:
    //!       @b increment either the @ref ProfileVersion::GetMajor "Major" or the @ref ProfileVersion::GetMinor "Minor"
    //!       version digit.
    //! 2. a new upgrade step needs to be implemented. It upgrades a file from the version prior to your change to
    //!    the new one.
    //!     - If it is not possible to implement upgrade for this profile modification, set the <b>Minimum upgrade
    //!       profile version</b> to the version prior to your change.
    //!
    //! @return Profile state
    BE_SQLITE_EXPORT virtual ProfileState _CheckProfileVersion() const;
    virtual DbResult _OnBeforeProfileUpgrade() { return BE_SQLITE_OK; }
    BE_SQLITE_EXPORT virtual DbResult _UpgradeProfile();
    virtual DbResult _OnAfterProfileUpgrade() { return BE_SQLITE_OK; }
    virtual DbResult _OnDbAttached(Utf8CP filename, Utf8CP dbAlias) const { return BE_SQLITE_OK; }
    virtual DbResult _OnDbDetached(Utf8CP dbAlias) const { return BE_SQLITE_OK; }
    virtual int _OnAddFunction(DbFunction& func) const {return 0;}
    virtual void _OnRemoveFunction(DbFunction& func) const {}
    BE_SQLITE_EXPORT DbResult DoProfileUpgrade();

    friend struct Statement;
    friend struct Savepoint;
    friend struct BeSQLiteProfileManager;

private:
    BE_SQLITE_EXPORT DbResult QueryDbIds();
    DbResult SaveBeDbGuid();
    DbResult SaveBriefcaseId();
    void DoCloseDb();
    DbResult DoOpenDb(Utf8CP dbName, OpenParams const& params);
    DbResult TruncateTable(Utf8CP tableName) const;
    void SetupBlvSaveStmt(Statement& stmt, Utf8CP name);
    DbResult ExecBlvQueryStmt(Statement& stmt, Utf8CP name) const;
    BE_SQLITE_EXPORT DbResult DropTable(Utf8CP tableName, bool requireExists) const;

public:
    BE_SQLITE_EXPORT Db();
    BE_SQLITE_EXPORT virtual ~Db();

    DbFile* GetDbFile() {return m_dbFile;}

    //! SQLite supports the concept of an "implicit" transaction. That is, if no explicit transaction is active when you execute an SQL statement,
    //! SQLite will create an implicit transaction whose scope is the execution of the statement. However, it is rarely a good idea to rely on that behavior,
    //! since the overhead of starting/stopping a transaction can be very large, often much larger than the execution of the statement itself.
    //! Further, the implicit transaction is necessary even for SELECT statements, which can be counterintuitive. To help avoid accidentally using
    //! implicit transactions, a BeSQLite::Db normally flags them as errors by forcing all Prepare and Step methods to assert in a debug
    //! build if no transaction is active. If you really want to allow implicit transactions, turn this flag on.
    void SetAllowImplicitTransactions(bool val) { m_dbFile->m_allowImplicitTxns = val; }

    //! Get the StatementCache for this Db.
    StatementCache& GetStatementCache() const {return const_cast<StatementCache&>(m_statements);}

    //! Get a CachedStatement for this Db. If the SQL string has already been prepared and a CachedStatement exists for it in the cache, it will
    //! be Reset (see Statement::Reset) and returned without the need to re-Prepare it. If, however, the SQL string has not been used before (or maybe just
    //! not recently) then a new CachedStatement will be created and Prepared.
    //! @param[out] statement The CachedStatement for the SQL string.
    //! @param[in] sql The SQL string from which to Prepare the CachedStatement.
    //! @return The result of either Statement::Reset (in the case where we reuse an existing CachedStatement) or Statement::Prepare (in the case
    //! where the SQL string has not yet been prepared).
    BE_SQLITE_EXPORT DbResult GetCachedStatement(CachedStatementPtr& statement, Utf8CP sql, bool logError = true) const;

    CachedStatementPtr GetCachedStatement(Utf8CP sql, bool logError = true) const {
        CachedStatementPtr stmt;
        return BE_SQLITE_OK == GetCachedStatement(stmt, sql, logError) ? stmt : nullptr;
    }

    //! Get an entry in the Db's Savepoint stack.
    //! @param[in] depth The depth of the Savepoint of interest. Must be 0 <= depth < GetCurrentSavepointDepth()
    //! @return A pointer to the Savepoint if depth is valid. nullptr otherwise.
    BE_SQLITE_EXPORT Savepoint* GetSavepoint(int32_t depth) const;

    //! Get the current number of entries in this Db's Savepoint stack.
    BE_SQLITE_EXPORT int32_t GetCurrentSavepointDepth() const;

    //! Attempts to free as much heap memory as possible from database connection
    BE_SQLITE_EXPORT DbResult FreeMemory() const;

    //! Enable/Disable trace flag.
    DbTrace ConfigTraceEvents(DbTrace flags, bool enable) const { return m_dbFile->ConfigTraceEvents(flags, enable); }

    //! Disable trace flag.
    void DisableAllTraceEvents() const { m_dbFile->DisableAllTraceEvents(); }

    //! Trace events.
    BE_SQLITE_EXPORT TraceRowEvent& GetTraceRowEvent() const;
    BE_SQLITE_EXPORT TraceStmtEvent& GetTraceStmtEvent() const;
    BE_SQLITE_EXPORT TraceProfileEvent& GetTraceProfileEvent() const;
    BE_SQLITE_EXPORT TraceCloseEvent& GetTraceCloseEvent() const;

    //! Determine whether there is an active transaction against this Db.
    bool IsTransactionActive() const {return 0 < GetCurrentSavepointDepth();}

    //! Open an existing BeSQLite::Db file.
    //!
    //! ### Backwards Compatibility Contract
    //! When opening a file, the profile version of the file is checked to see whether the current version of the software
    //! can open it or not. If it cannot be opened, a respective error code is returned from this method.
    //!
    //! The version check is performed for @b every %Bentley SQLite profile the file contains. Only if all profiles
    //! in the file succeed the test, the file is eventually opened. Example profiles are the @b %Bim or the @b %ECDb profile
    //! (see also @ref ECDbFile "ECDb versus Bim profile").
    //!
    //! If a client attempts to open a @b newer file and the profile version differs from the expected
    //! - in its major or minor digit: the file cannot be opened. ::BE_SQLITE_ERROR_ProfileTooNew is returned.
    //! - in its sub1 digit: the file can only be opened read-only. ::BE_SQLITE_ERROR_ProfileTooNewForReadWrite is returned.
    //! - in its sub2 digit: the file is fully compatible. ::BE_SQLITE_OK is returned.
    //!
    //! If a client attempts to open an @b older file and the profile version differs from the expected
    //! - in its major or minor digit: the file cannot be opened. ::BE_SQLITE_ERROR_ProfileTooOld is returned.
    //! - in its sub1 digit: the file can only be opened read-only. ::BE_SQLITE_ERROR_ProfileTooOldForReadWrite is returned.
    //! - in its sub2 digit: the file is fully compatible. ::BE_SQLITE_OK is returned.
    //!
    //! @note If the file is older and can be upgraded, pass ProfileUpgradeOptions::Upgrade to the open params.
    //!
    //! @note A Db can have an expiration date and time. See #IsExpired
    //!
    //! @param[in] dbName The name of the BeSQLite::Db database file. Must not be nullptr.
    //! @param[in] openParams the parameters that determine aspects of the opened database file.
    //! @return ::BE_SQLITE_OK if the database was successfully opened, or error code as explained above otherwise.
    BE_SQLITE_EXPORT DbResult OpenBeSQLiteDb(Utf8CP dbName, OpenParams const& openParams);

    //! @copydoc Db::OpenBeSQLiteDb(Utf8CP, OpenParams const&)
    DbResult OpenBeSQLiteDb(BeFileNameCR dbName, OpenParams const& openParams) {return OpenBeSQLiteDb(dbName.GetNameUtf8().c_str(),openParams);}

    //! Open another connection to this Db. Uses the queryParams which were used to open the original connection to the database.
    //! @see Db::OpenBeSQLiteDb
    //! @param[out] newConnection The new "secondary" connection.
    //! @param[in] openParams the parameters that determine aspects of the opened database file.
    //! @return ::BE_SQLITE_OK if the database was successfully opened, or error code as explained above otherwise.
    BE_SQLITE_EXPORT DbResult OpenSecondaryConnection(Db& newConnection, OpenParams openParams) const;

    //! Determines the file's profile state as for its compatibility to be opened with the current version of the profile's API.
    ProfileState CheckProfileVersion() const { return _CheckProfileVersion(); };

    //! Checks a file's profile compatibility to be opened with the current version of the profile's API.
    //!
    //! @see Db::OpenBeSQLiteDb for the compatibility contract for Bentley SQLite profiles.
    //! @param[in] expectedProfileVersion Profile version expected by the API that tries to open the file.
    //! @param[in] actualProfileVersion Profile version of the file to be opened
    //! @param[in] minimumUpgradableProfileVersion Minimum profile version of the file for which the API can auto-upgrade
    //!            the file to the latest version. The version's Sub1 and Sub2 digits must be 0.
    //! @param[in] profileName Name of the profile for logging purposes.
    //! @return Profile state
    BE_SQLITE_EXPORT static ProfileState CheckProfileVersion(ProfileVersion const& expectedProfileVersion, ProfileVersion const& actualProfileVersion, ProfileVersion const& minimumUpgradableProfileVersion, Utf8CP profileName);

    //! Create a new BeSQLite::Db database file, or upgrade an existing SQLite file to be a BeSQLite::Db.
    //! This will open or create the physical file, and then create the default BeSQLite::Db tables and properties.
    //! @param[in] dbName The file name for the new database. If dbName is nullptr, a temporary database is created that is automatically
    //!  deleted when it is closed. If dbName is the value defined by BEDB_MemoryDb, an in-memory database is created.
    //!  If dbName is the name of an existing SQLite file, and if params.m_failIfDbExists is false (which is *not* the default),
    //!  it will open that file and add the BeSQLite::Db tables. If the file already contains the BeSQLite::Db tables, this method
    //!  will fail.
    //! @param[in] dbGuid The guid for the new database. The guid will be saved persistently in the new database.
    //! If the guid is invalid (e.g. by passing "BeGuid()" for dbGuid), a new guid is created by this method.
    //! @param[in] params Parameters about how the database should be created.
    //! @return BE_SQLITE_OK if the database was successfully created, error code otherwise.
    //! @note If the database file exists before this call, its page size and encoding are not changed.
    BE_SQLITE_EXPORT DbResult CreateNewDb(Utf8CP dbName,CreateParams const& params=CreateParams(),  BeGuid dbGuid=BeGuid() );

    DbResult CreateNewDb(BeFileNameCR dbName, CreateParams const& params = CreateParams(), BeGuid dbGuid = BeGuid()) { return CreateNewDb(dbName.GetNameUtf8().c_str(), params, dbGuid); }

    //! Determine whether this Db refers to a currently opened file.
    BE_SQLITE_EXPORT bool IsDbOpen() const;

    //! Close this Db. All Statements against this Db should be deleted before calling this method. The StatementCache and its contents are deleted
    //! automatically. Since more than one Db may point to the same DbFile, this method may or may not result in the actual file being closed. It
    //! therefore returns no status.
    BE_SQLITE_EXPORT void CloseDb();

    //! @return The name of the physical file associated with this Db. nullptr if Db is not opened.
    BE_SQLITE_EXPORT Utf8CP GetDbFileName() const;

    // Get the "base" name that can be used for creating a temporary file related to this Db. Callers should append a unique string to this value (e.g "-tiles")
    // to form a full path for a temporary file. The path is guaranteed to be a filename in an existing writable directory.
    Utf8String GetTempFileBaseName() const { return m_tempfileBase.empty() ? GetDbFileName() : m_tempfileBase; }


    //! Save a BeProjectGuid for this Db.
    BE_SQLITE_EXPORT void SaveProjectGuid(BeGuid);

    //! Query the BeProjectGuid for this Db.
    //! @see SaveMyProjectGuid
    BE_SQLITE_EXPORT BeGuid QueryProjectGuid() const;

    BE_SQLITE_EXPORT void QueryStandaloneEditFlags(BeJsValue) const;
    BE_SQLITE_EXPORT DbResult SaveStandaloneEditFlags(BeJsConst val);

    //! Saves the current date and time as the CreationDate property of this database.
    //! @return BE_SQLITE_OK if property was successfully saved, error status otherwise.
    BE_SQLITE_EXPORT DbResult SaveCreationDate();

    //! Query the CreationDate property of this database.
    //! @param[out] creationDate The date that the database was created.
    //! @return BE_SQLITE_ROW if the CreationDate property was successfully found and creationDate is valid, error status otherwise.
    BE_SQLITE_EXPORT DbResult QueryCreationDate(DateTime& creationDate) const;

    //! Attach another database to this database. This method executes the SQLite "ATTACH" command, but is necessary because
    //! SQLite does not allow a database to be attached if there is a transaction open. If the Db has an open transaction, it is committed before
    //! the ATTACH sql statement is executed and restarted afterwards.
    //! @param[in] filename The name of the file holding the SQLite database to be attached.
    //! @param[in] alias The alias by which the database is attached.
    BE_SQLITE_EXPORT DbResult AttachDb(Utf8CP filename, Utf8CP alias) const;

    //! Detach a previously attached database. This method is necessary for the same reason AttachDb is necessary.
    //! @param[in] alias The alias by which the database was attached.
    BE_SQLITE_EXPORT DbResult DetachDb(Utf8CP alias) const;

    //! Execute a single SQL statement on this Db.
    //! This merely binds, steps, and finalizes the statement. It is no more efficient than performing those steps individually,
    //! but is more convenient.
    //! This method also enforces logs the statement and error if the statement fails. Use TryExecuteSql if you consider failures to be non-exceptional.
    //! @see sqlite3_exec
    BE_SQLITE_EXPORT DbResult ExecuteSql(Utf8CP sql, int(*callback)(void*,int,char**,char**)=0, void* arg=0, char** errmsg=0) const;

    //! Identical to ExecuteSql, but does not log errors nor check implicit transactions
    BE_SQLITE_EXPORT DbResult TryExecuteSql(Utf8CP sql, int (*callback)(void*,int,char**,char**)=0, void* arg=0, char** errmsg=0) const;

    //! return a string that describes the query plan for the specified SQL, or an error message if the SQL is invalid
    BE_SQLITE_EXPORT Utf8String ExplainQuery(Utf8CP sql, bool plan=true) const;

    //! Execute tracked DDL which is captured by changetracker.
    BE_SQLITE_EXPORT DbResult ExecuteDdl(Utf8CP ddl) const;

    //! Create a new table in this Db.
    //! @param[in] tableName The name for the new table.
    //! @param[in] ddl The column definition sql for this table (should not include parentheses).
    BE_SQLITE_EXPORT DbResult CreateTable(Utf8CP tableName, Utf8CP ddl) const;

    //! Create a new table in this Db only if it doesnt exist already
    //! @param[in] tableName The name for the new table.
    //! @param[in] ddl The column definition sql for this table (should not include parentheses).
    BE_SQLITE_EXPORT DbResult CreateTableIfNotExists(Utf8CP tableName, Utf8CP ddl) const;

    //! Drop a table from this Db. Asserts that the table exists.
    //! @param[in] tableName The name of the table to drop.
    //! @return BE_SQLITE_OK if the table was dropped.
    DbResult DropTable(Utf8CP tableName) const { return DropTable(tableName, true); }

    //! Drop a table from this Db, if it exists.
    //! @param[in] tableName The name of the table to try to drop.
    //! @return BE_SQLITE_OK if the table was dropped or did not exist.
    DbResult DropTableIfExists(Utf8CP tableName) const { return DropTable(tableName, false); }

    //! Determine whether a table exists in this Db.
    BE_SQLITE_EXPORT bool TableExists(Utf8CP tableName) const;

    //! Add a column to a table
    //! @param[in] tableName Name of the table. e.g., "test_Employee"
    //! @param[in] columnName Name of the new column. e.g., "TitleId"
    //! @param[in] columnDetails Details of the new column, e.g., "INTEGER REFERENCES test_Title(Id)"
    //! @remarks Internally executes a DDL and records the schema change if necessary.
    //! e.g., "ALTER TABLE test_employee ADD COLUMN columnName INTEGER REFERENCES test_Title(Id)"
    BE_SQLITE_EXPORT DbResult AddColumnToTable(Utf8CP tableName, Utf8CP columnName, Utf8CP columnDetails);

    //! Determine whether a column exists in a table in this Db.
    BE_SQLITE_EXPORT bool ColumnExists(Utf8CP tableName, Utf8CP columnName) const;

    //! Get list of columns in the db table
    //! @param[out] columns contains list of columns in table.
    //! @param[in] tableName Name of the table that column names need to be returned
    BE_SQLITE_EXPORT bool GetColumns(bvector<Utf8String>& columns, Utf8CP tableName) const;

    //! Rename existing table
    //! @param[in] tableName The name of existing table.
    //! @param[in] newTableName new name for the table.
    BE_SQLITE_EXPORT DbResult RenameTable(Utf8CP tableName, Utf8CP newTableName);

    //! Create an index on an existing table
    //! @param[in] indexName Name of the new index
    //! @param[in] tableName Name of the existing table
    //! @param[in] isUnique Pass true to setup a unique index
    //! @param[in] columnName1 Name of the column to create index on
    //! @param[in] columnName2 Optional name of a second column to create a multi-column index
    BE_SQLITE_EXPORT DbResult CreateIndex(Utf8CP indexName, Utf8CP tableName, bool isUnique, Utf8CP columnName1, Utf8CP columnName2 = nullptr);

    //! Free non-essential memory associated with the DB. Typically used when going to the background state on a tablet computer.
    BE_SQLITE_EXPORT void FlushPageCache();

    //! @name Db Properties
    //! @{

    //! @private
    DbResult SaveProperty(PropertySpecCR spec, Utf8StringCR strData, void const* value, uint32_t propsize, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->SaveProperty(spec, strData.c_str(), value, propsize, majorId, subId);}

    //! Save a property value in this Db.
    //! @param[in] spec The PropertySpec of the property to save.
    //! @param[in] value A pointer to a buffer that holds the new value of the property to be saved. May be nullptr
    //! @param[in] propsize The number of bytes in value (ignored if value is nullptr).
    //! @param[in] majorId The major id of the property.
    //! @param[in] subId The subId of the property. The combination of majorId/subId forms a 2 dimensional array of properties of a given PropertySpec,
    //! @return BE_SQLITE_OK if the property was successfully saved, error code otherwise.
    DbResult SaveProperty(PropertySpecCR spec, void const* value, uint32_t propsize, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->SaveProperty(spec, nullptr, value, propsize, majorId, subId);}

    //! Save a (Utf8) text string as a property value in this Db.
    //! @param[in] spec The PropertySpec of the property to save.
    //! @param[in] value The Utf8 sString whose value is to be saved as a property.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property. The combination of majorId/subId forms a 2 dimensional array of properties of a given PropertySpec,
    //! @return BE_SQLITE_OK if the property was successfully saved, error code otherwise.
    DbResult SavePropertyString(PropertySpecCR spec, Utf8CP value, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->SaveProperty(spec, value, nullptr, 0, majorId, subId);}

    //! Save a Utf8String as a property value in this Db.
    //! @param[in] spec The PropertySpec of the property to save.
    //! @param[in] value The Utf8String whose value is to be saved as a property.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property. The combination of majorId/subId forms a 2 dimensional array of properties of a given PropertySpec,
    //! @return BE_SQLITE_OK if the property was successfully saved, error code otherwise.
    DbResult SavePropertyString(PropertySpecCR spec, Utf8StringCR value, uint64_t majorId=0, uint64_t subId=0) {return SavePropertyString(spec, value.c_str(), majorId, subId);}

    //! Determine whether a property exists in the Db.
    //! @param[in] spec The PropertySpec of the property to query.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property.
    //! @return true if the property exists in the Db.
    bool HasProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const {return m_dbFile->HasProperty(spec, majorId, subId);}

    //! Determine the size in bytes of the blob part of a property in the Db.
    //! @param[out] propsize The number of bytes in the property.
    //! @param[in] spec The PropertySpec of the property to query.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property.
    //! @return BE_SQLITE_ROW if the property exists and propsize is valid, error code otherwise.
    //! @note Success is indicated by BE_SQLITE_ROW, *not* BE_SQLITE_OK.
    DbResult QueryPropertySize(uint32_t& propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const {return m_dbFile->QueryPropertySize(propsize, spec, majorId, subId);}

    //! Query the value of the blob part a property from the Db.
    //! @param[out] value A pointer to buffer of propsize bytes to hold the property. On success, the number of bytes of valid data in value will be the
    //!                    lesser of propsize and the actual size of the property in the Db.
    //! @param[in] propsize The number of bytes in value.
    //! @param[in] spec The PropertySpec of the property to query.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property.
    //! @return BE_SQLITE_ROW if the property exists and value is valid, error code otherwise.
    //! @note Success is indicated by BE_SQLITE_ROW, *not* BE_SQLITE_OK.
    DbResult QueryProperty(void* value, uint32_t propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const {return m_dbFile->QueryProperty(value, propsize, spec, majorId, subId);}

    //! Query the value of the string part of a property from the Db.
    //! @param[out] value A Utf8String to fill with the value of the property.
    //! @param[in] spec The PropertySpec of the property to query.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property.
    //! @return BE_SQLITE_ROW if the property exists and value is valid, error code otherwise.
    //! @note Success is indicated by BE_SQLITE_ROW, *not* BE_SQLITE_OK.
    DbResult QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const {return m_dbFile->QueryProperty(value, spec, majorId, subId);}

    //! Delete a property from the Db.
    //! @param[in] spec The PropertySpec of the property to delete.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property.
    //! @return BE_SQLITE_DONE if the property was either deleted or did not exist, error code otherwise.
    DbResult DeleteProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->DeleteProperty(spec, majorId, subId);}

    //! Delete one or more properties from the Db.
    //! @param[in] spec The PropertySpec of the property to delete.
    //! @param[in] majorId A pointer to the majorId of the properties to delete. If nullptr, all properties of spec are deleted. If non-nullptr, all properties of spec with the supplied majorid are deleted.
    //! @return BE_SQLITE_DONE if the property was either deleted or did not exist, error code otherwise.
    DbResult DeleteProperties(PropertySpecCR spec, uint64_t* majorId) {return m_dbFile->DeleteProperties(spec, majorId);}

    //! @name BriefcaseLocalValue
    //! @{
    //! Briefcase Local Values are used for information specific to this copy of the Db. The table BEDB_Table_Local in which
    //! BriefcaseLocalValues are stored is not change tracked or change merged.

    //! Get a reference to the BriefcaseLocalValueCache for this Db.
    BriefcaseLocalValueCache& GetBLVCache() {return m_dbFile->GetBLVCache();}

    //! Query the current value of a Briefcase Local Value of type string.
    //! @param[out] value On success, the value of the BLV.
    //! @param[in] name The name of the BLV.
    //! @return BE_SQLITE_ROW if the value exists and value is valid, error status otherwise.
    BE_SQLITE_EXPORT DbResult QueryBriefcaseLocalValue(Utf8StringR value, Utf8CP name) const;
    BE_SQLITE_EXPORT DbResult QueryBriefcaseLocalValue(uint64_t& value, Utf8CP name) const;

    //! Save a new value of a Briefcase Local Value of type string. If the RLC already exists, its value is replaced.
    //! @param[in] name The name of the BLV.
    //! @param[in] value The new value for BLV name.
    //! @return BE_SQLITE_DONE if the value was saved, error status otherwise.
    BE_SQLITE_EXPORT DbResult SaveBriefcaseLocalValue(Utf8CP name, Utf8StringCR value);
    BE_SQLITE_EXPORT DbResult SaveBriefcaseLocalValue(Utf8CP name, uint64_t value);

    //! Delete a Briefcase Local Value
    BE_SQLITE_EXPORT DbResult DeleteBriefcaseLocalValue(Utf8CP name);
    //! @}

    //! @return the rowid from the last insert statement for this Db.
    //! @see sqlite3_last_insert_rowid
    BE_SQLITE_EXPORT int64_t GetLastInsertRowId() const;

    //! @return the number of rows modified by the last statement for this Db.
    //! @see sqlite3_changes
    BE_SQLITE_EXPORT int GetModifiedRowCount() const;

    //! @return the total number of rows modified since the database connection was opened.
    //! @see sqlite3_total_changes
    BE_SQLITE_EXPORT int GetTotalModifiedRowCount() const;

    //! @return The last error message for this Db.
    //! @param[out] lastResult The last error code for this Db.
    //! @see sqlite3_errmsg, sqlite3_errcode
    BE_SQLITE_EXPORT Utf8String GetLastError(DbResult* lastResult = nullptr) const;

    //! Causes any pending database operation to abort and return at its earliest opportunity.
    //! @see sqlite3_interrupt
    BE_SQLITE_EXPORT void Interrupt() const;

    //! Commit the outermost transaction, writing changes to the file. Then, restart the transaction.
    //! @param[in] changesetName The name of the operation that generated these changes. If transaction tracking is enabled,
    //! this will be saved with the changeset and reported as the operation that is "undone/redone" if this changeset is reversed or reinstated.
    //! @note, this will commit any nested transactions.
    BE_SQLITE_EXPORT DbResult SaveChanges(Utf8CP changesetName=nullptr);
    DbResult SaveChanges(Utf8StringCR changesetName) {return SaveChanges(changesetName.c_str());}

    //! Abandon (cancel) the outermost transaction, discarding all changes since last save. Then, restart the transaction.
    //! @note, this will cancel any nested transactions.
    BE_SQLITE_EXPORT DbResult AbandonChanges();

    //! Get the raw SqlDbP for this Db. This is only necessary to call sqlite3_xx functions directly.
    BE_SQLITE_EXPORT SqlDbP GetSqlDb() const;

    //! Get the GUID of this Db.
    BE_SQLITE_EXPORT BeGuid GetDbGuid() const;

    //! Get the (local) BeBriefcaseId of this Db. Every copy of the Db must have a unique BeBriefcaseId.
    BeBriefcaseId GetBriefcaseId() const {return m_dbFile->m_briefcaseId;}

    //! Get a new value for a BeServerIssuedId from the server
    //! @param [in,out] value the new value of the BeServerIssuedId
    //! @param [in] tableName the name of the table holding the BeServerIssuedId
    //! @param [in] columnName the name of the column holding the BeServerIssuedId
    //! @param [in] json parameters that the server can use to create the new row in the specified table.
    BE_SQLITE_EXPORT DbResult GetServerIssuedId(BeServerIssuedId& value, Utf8CP tableName, Utf8CP columnName, Utf8CP json=nullptr);

    //! Determine whether this Db was opened readonly.
    bool IsReadonly() const { return m_dbFile->m_readonly; }

    //! Get the DbEmbeddedFileTable for this Db.
    BE_SQLITE_EXPORT DbEmbeddedFileTable& EmbeddedFiles();

    //! Add Db::AppData to this Db.
    //! @param[in] key A unique key to differentiate \c appData from all of the other AppData objects stored on this Db.
    //! By convention, uniqueness is enforced by using the address of a (any) static object. Since all applications
    //! share the same address space, every key will be unique. Note that this means that the value of your key will be different for
    //! every session. But that's OK, the only thing important about \c key is that it be unique.
    //! If AppData with this key already exists on this Db, it is dropped and replaced with \a appData.
    //! @param[in] appData The application's instance of a subclass of AppData to store on this Db.
    //! @note This function is thread-safe.
    BE_SQLITE_EXPORT void AddAppData(AppData::Key const& key, AppData* appData) const;

    //! Remove a Db::AppData object from this Db by its key.
    //! @param[in] key The key to find the appropriate AppData. See discussion of keys in AddAppData.
    //! @return SUCCESS if a AppData object with \c key was found and was dropped. ERROR if no AppData with \c key exists.
    //! @note This function is thread-safe.
    BE_SQLITE_EXPORT StatusInt DropAppData(AppData::Key const& key) const;

    //! Search for the Db::AppData on this Db with \c key. See discussion of keys in AddAppData.
    //! @return A pointer to the AppData object with \c key. nullptr if not found.
    //! @note This function is thread-safe.
    BE_SQLITE_EXPORT AppDataPtr FindAppData(AppData::Key const& key) const;

    template<typename T> RefCountedPtr<T> FindAppDataOfType(AppData::Key const& key) const
        {
        auto found = FindAppData(key);
        return dynamic_cast<T*>(found.get());
        }

    //! Search for the Db::AppData on this Db with \c key. If no such AppData yet exists, the supplied \c createAppData function
    //! will be invoked to create it, and the returned AppData* will be added to the Db.
    //! @param[in] key The key to find the appropriate AppData. See discussion of keys in AddAppData.
    //! @param[in] createAppData A function object taking no arguments and returning a Db::AppData*, to be invoked if the specified key does not yet exist.
    //! @return The existing or newly-added Db::AppData*
    //! @note This function is thread-safe and avoids race conditions between FindAppData() and AddAppData().
    template<typename T> AppDataPtr FindOrAddAppData(AppData::Key const& key, T createAppData) const { return m_appData.FindOrAdd(key, createAppData); }

    //! Search for the Db::AppData on this Db with \c key. If no such AppData yet exists, the supplied \c createAppData function
    //! will be invoked to create it, and the returned AppData* will be added to the Db.
    //! @param[in] key The key to find the appropriate AppData. See discussion of keys in AddAppData.
    //! @param[in] createAppData A function object taking no arguments and returning a Db::AppData*, to be invoked if the specified key does not yet exist.
    //! @return A ref-counted pointer to the existing or newly-added Db::AppData, as the derived type returned by invoking createAppData().
    //! @note This function is thread-safe and avoids race conditions between FindAppData() and AddAppData().
    template<typename T> auto ObtainAppData(AppData::Key const& key, T createAppData) const -> RefCountedPtr<typename std::remove_pointer<decltype(createAppData())>::type>
        {
        using U = decltype(createAppData());
        AppDataPtr data = FindOrAddAppData(key, createAppData);
        return static_cast<U>(data.get());
        }

    //! Dump statement results to stdout (for debugging purposes, only, e.g. to examine data in a temp table)
    BE_SQLITE_EXPORT void DumpSqlResults(Utf8CP sql);

    //! Return the DbResult as a string. This is useful for debugging SQL problems.
    BE_SQLITE_EXPORT static Utf8CP InterpretDbResult(DbResult result);

    //! Set one of the internal SQLite limits for this database. See documentation at sqlite3_limit for argument details.
    BE_SQLITE_EXPORT int SetLimit(DbLimits id, int newVal) const;
    BE_SQLITE_EXPORT int GetLimit(DbLimits id) const;

    //! Add a DbFunction to this Db for use in SQL. See sqlite3_create_function for return values. The DbFunction object must remain valid
    //! while this Db is valid, or until it is removed via #RemoveFunction.
    BE_SQLITE_EXPORT int AddFunction(DbFunction& func) const;

    //! Remove a previously added DbFunction from this Db. See sqlite3_create_function for return values.
    BE_SQLITE_EXPORT int RemoveFunction(DbFunction& func) const;

    BE_SQLITE_EXPORT int AddRTreeMatchFunction(RTreeMatchFunction& func) const;

    //! Add a callback to this Db that is invoked whenever a row is updated, inserted or deleted in a
    //! <a href="https://sqlite.org/rowidtable.html">rowid</a> table.
    //! @see DataUpdateHook
    //! @see https://sqlite.org/c3ref/update_hook.html
    BE_SQLITE_EXPORT void AddDataUpdateCallback(DataUpdateCallback&) const;
    //! Removes the data change callback from this Db
    //! @see DataUpdateHook
    //! @see https://sqlite.org/c3ref/update_hook.html
    BE_SQLITE_EXPORT void RemoveDataUpdateCallback() const;

    BE_SQLITE_EXPORT DbResult ChangeDbGuid(BeGuid);
    BE_SQLITE_EXPORT DbResult ResetBriefcaseId(BeBriefcaseId);

    //! Set or replace the ChangeTracker for this Db.
    //! @param[in] tracker The new ChangeTracker for this Db.
    //! @note If this Db already has a ChangeTracker active, the new one replaces it.
    BE_SQLITE_EXPORT void SetChangeTracker(ChangeTracker* tracker);

    Savepoint* GetDefaultTransaction() const {return (m_dbFile != nullptr) ? &m_dbFile->m_defaultTxn : nullptr;}

    //! Check if the Db is at or beyond its expiration date.
    //! @see QueryExpirationDate, CreateParams::SetExpirationDate
    BE_SQLITE_EXPORT bool IsExpired() const;

    //! Query the ExpirationDate property of this database.
    //! @param[out] expirationDate The expiration date, if any.
    //! @return BE_SQLITE_ROW if the ExpirationDate property was successfully found, BE_SQLITE_NOTFOUND if this database does not have
    //! an expiration data, or BE_SQLITE_ERROR if the expiration date could not be read.
    //! @see SaveExpirationDate, IsExpired
    BE_SQLITE_EXPORT DbResult QueryExpirationDate(DateTime& expirationDate) const;

    //! Saves the specified date as the ExpirationDate property of the database.
    //! @param[in] expirationDate The expiration date. <em>Must be UTC.</em>
    //! @return BE_SQLITE_OK if property was successfully saved, or a non-zero error status if the expiration date is invalid, is not UTC, or could not be saved to the database.
    //! @see QueryExpirationDate, IsExpired
    BE_SQLITE_EXPORT DbResult SaveExpirationDate(DateTime const& expirationDate);

    //! Vacuum current db into a new file
    //! @param newFileName path to new db
    //! @return BE_SQLITE_OK if successful
    BE_SQLITE_EXPORT DbResult VacuumInto(Utf8CP newFileName);

    //! Vacuum a file and optionally change page_size.
    //! @param newPageSizeInBytes Must be size in bytes for a page as described by sqlite.
    BE_SQLITE_EXPORT DbResult Vacuum(int newPageSizeInBytes = 0);

    BE_SQLITE_EXPORT DbResult RestartDefaultTxn();

    //! DO NOT call this under normal circumstances. It is for obscure cases where you are opening an untrusted file (i.e. NOT from the hub).
    //! Opens the specified database, performs a sqlite integrity check, and closes it. Returns BE_SQLITE_OK if the check was successful, otherwise BE_SQLITE_CORRUPT or other chained errors if there was a failure.
    BE_SQLITE_EXPORT static DbResult CheckDbIntegrity(BeFileNameCR dbFileName);

    BE_SQLITE_EXPORT DbBuffer Serialize(const char *zSchema = nullptr) const;
    BE_SQLITE_EXPORT static DbResult Deserialize(DbBuffer& buffer, DbR db, DbDeserializeOptions opts = DbDeserializeOptions::FreeOnClose, const char *zSchema = nullptr, std::function<void(DbR)> beforeDefaultTxnStarts = nullptr);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SQLiteTraceScope final: NonCopyableClass
    {
    private:
        std::vector<cancel_callback_type> m_cancelCbList;
        bmap<Utf8String, std::pair<int64_t, int64_t>> m_profile;

    public:
        BE_SQLITE_EXPORT  SQLiteTraceScope(DbTrace categories, DbCR db, Utf8CP loggerName = "SQLite");
        BE_SQLITE_EXPORT ~SQLiteTraceScope();
    };

//=======================================================================================
//! Error values returned from the ZLib functions. See ZLib documentation for details.
// @bsiclass
//=======================================================================================
enum ZipErrors
{
    ZIP_SUCCESS = 0,
    ZIP_ERROR_BASE              = 0x15000,
    ZIP_ERROR_WRITE_ERROR       = ZIP_ERROR_BASE + 0x01,
    ZIP_ERROR_COMPRESSION_ERROR = ZIP_ERROR_BASE + 0x02,
    ZIP_ERROR_END_OF_DATA       = ZIP_ERROR_BASE + 0x03,
    ZIP_ERROR_BUFFER_FULL       = ZIP_ERROR_BASE + 0x04,
    ZIP_ERROR_BLOB_READ_ERROR   = ZIP_ERROR_BASE + 0x05,
    ZIP_ERROR_FILE_DOES_NOT_EXIST = ZIP_ERROR_BASE + 0x06,
    ZIP_ERROR_CANNOT_OPEN_OUTPUT = ZIP_ERROR_BASE + 0x07,
    ZIP_ERROR_UNKNOWN            = ZIP_ERROR_BASE + 0x08,
    ZIP_ERROR_BAD_DATA           = ZIP_ERROR_BASE + 0x09,
    ZIP_ERROR_CANNOT_OPEN_INPUT  = ZIP_ERROR_BASE + 0x0A,
    ZIP_ERROR_ABORTED            = ZIP_ERROR_BASE + 0x0B,
    ZIP_ERROR_READ_ERROR         = ZIP_ERROR_BASE + 0x0C,
};

//=======================================================================================
//! Utility to compress and write data to a blob using "Snappy" compression. Snappy is faster (usually 2x) both to
//! compress and decompress, but usually results in about 10-20% larger storage requirement than zip.
//! (see http://code.google.com/p/snappy)
// @bsiclass
//=======================================================================================
struct SnappyToBlob
{
private:
    struct SnappyChunk
    {
        uint16_t*  m_data;
        SnappyChunk(uint32_t size) {m_data = (uint16_t*) BeSQLiteLib::MallocMem(size + 2);} // 2 bytes for compressed size at start of data
        uint16_t GetChunkSize() {return m_data[0];}
        ~SnappyChunk() {BeSQLiteLib::FreeMem(m_data);}
    };

    bvector<SnappyChunk*> m_chunks;
    Byte*   m_rawBuf;
    uint32_t m_currChunk;
    uint32_t m_rawSize;
    uint32_t m_rawCurr;
    uint32_t m_rawAvail;
    uint32_t m_unsnappedSize;

public:
    BE_SQLITE_EXPORT SnappyToBlob();
    BE_SQLITE_EXPORT ~SnappyToBlob();
    uint32_t GetCurrChunk() {return m_currChunk;}
    Byte*  GetChunkData(int i) {return (Byte*) m_chunks[i]->m_data;}

    //! Get the number of bytes in the raw (uncompressed) stream.
    uint32_t GetUnCompressedSize() const {return m_unsnappedSize;}

    //! Finish the compression and get the number of bytes required to store the compressed stream. This becomes the size of the blob.
    BE_SQLITE_EXPORT uint32_t GetCompressedSize();

    //! Start or Restart compressing data.
    BE_SQLITE_EXPORT void Init();

    //! Write data to be compressed into this object. This method can be called any number of times after #Init has been called until #Finish is called.
    //! @param[in] data the data to be written.
    //! @param[in] size number of bytes in data.
    BE_SQLITE_EXPORT void Write(Byte const* data, uint32_t size);

    //! Finish the compression. After this call, no additional data may be written to this blob until #Init is called.
    BE_SQLITE_EXPORT void Finish();

    void DoSnappy(Byte const* data, uint32_t size) {Init(); Write(data,size); Finish();}

    //! Save this compressed value as a blob in a Db.
    //! @param[in] db the SQLite database to write
    //! @param[in] tableName the name of the table to write
    //! @param[in] column the column to write
    //! @param[in] rowId the rowId to write
    //! @note The cell in the db at tableName[column,rowId] must be an existing blob of #GetCompressedSize bytes. This method cannot be used to
    //! change the size of a blob.
    //! @see sqlite3_blob_open, sqlite3_blob_write, sqlite3_blob_close
    BE_SQLITE_EXPORT DbResult SaveToRow(DbR db, Utf8CP tableName, Utf8CP column, int64_t rowId);

    //! Save this compressed value as a blob in a Db.
    //! @remarks Opening and destroying the BlobIO handle must be done by the caller.
    //! @param[in] blobIO the BlobIO handle used to write the BLOB to the SQLite database. It must already be opened in read-write mode.
    //! @note The cell in the db to which @p blobIO points to must be an existing blob of #GetCompressedSize bytes. This method cannot be used to
    //! change the size of a blob.
    //! @see sqlite3_blob_open, sqlite3_blob_write, sqlite3_blob_close
    BE_SQLITE_EXPORT DbResult SaveToRow(BlobIO& blobIO);

    //! Obtain a thread-local SnappyToBlob. The returned object is deleted when the calling thread exits and should never be shared between threads.
    BE_SQLITE_EXPORT static SnappyToBlob& GetForThread();
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SnappyReader
{
    static const uint32_t SNAPPY_UNCOMPRESSED_BUFFER_SIZE = (34*1024);

    virtual ~SnappyReader() {}
    virtual ZipErrors _Read(Byte* data, uint32_t size, uint32_t& actuallyRead) = 0;

    static uint32_t GetUncompressedBufferSize() {return SNAPPY_UNCOMPRESSED_BUFFER_SIZE;}
};

//=======================================================================================
//! Utility to read Snappy-compressed data from memory, typically from an image of a blob.
// @bsiclass
//=======================================================================================
struct SnappyFromMemory : SnappyReader
{
private:
    Byte*   m_uncompressed;
    Byte*   m_uncompressCurr;
    uint16_t m_uncompressAvail;
    uint16_t m_uncompressSize;
    Byte*   m_blobData;
    uint32_t m_blobOffset;
    uint32_t m_blobBytesLeft;

    ZipErrors ReadNextChunk();
    ZipErrors TransferFromBlob(void* data, uint32_t numBytes, int offset);

public:
    BE_SQLITE_EXPORT SnappyFromMemory(void* uncompressedBuffer, uint32_t uncompressedBufferSize);
    BE_SQLITE_EXPORT void Init(void* blobBuffer, uint32_t blobBufferSize);
    BE_SQLITE_EXPORT virtual ZipErrors _Read(Byte* data, uint32_t size, uint32_t& actuallyRead) override;

    //! Obtain a thread-local SnappyFromMemory. The returned object is deleted when the calling thread exits and should never be shared between threads.
    BE_SQLITE_EXPORT static SnappyFromMemory& GetForThread();
};

//=======================================================================================
//! Utility to read Snappy-compressed data from a blob in a database.
// @bsiclass
//=======================================================================================
struct SnappyFromBlob : SnappyReader
{
private:
    Byte*   m_uncompressed;
    Byte*   m_uncompressCurr;
    Byte*   m_blobData;
    BlobIO  m_blobIO;
    uint32_t m_blobBufferSize;
    uint32_t m_blobOffset;
    uint32_t m_blobBytesLeft;
    uint16_t m_uncompressAvail;

    ZipErrors ReadNextChunk();

public:
    BE_SQLITE_EXPORT SnappyFromBlob();
    BE_SQLITE_EXPORT ZipErrors Init(DbCR db, Utf8CP tableName, Utf8CP column, int64_t rowId);
    BE_SQLITE_EXPORT ~SnappyFromBlob();
    BE_SQLITE_EXPORT virtual ZipErrors _Read(Byte* data, uint32_t size, uint32_t& actuallyRead) override;
    BE_SQLITE_EXPORT void Finish() ;
    ZipErrors ReadAndFinish(Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead) {auto stat=_Read(data, bufSize, bytesActuallyRead); Finish(); return stat;}
    BE_SQLITE_EXPORT ZipErrors ReadToChunkedArray(ChunkedArray& array, uint32_t bufSize);

    //! Obtain a thread-local SnappyFromBlob. The returned object is deleted when the calling thread exits and should never be shared between threads.
    BE_SQLITE_EXPORT static SnappyFromBlob& GetForThread();
};

#define BEDB_PROPSPEC_NAMESPACE "be_Db"
#define BEDB_PROPSPEC_EMBEDBLOB_NAME "EmbdBlob"
//=======================================================================================
//! A property specification for the "be_Db" namespace. The "be_Db" namespace name is reserved for Properties that are
//! created by the BeSQLite::Db API itself and should not be used by other software layers.
// @bsiclass
//=======================================================================================
struct PropSpec : PropertySpec
{
    PropSpec(Utf8CP name, PropertySpec::Compress compress = PropertySpec::Compress::Yes) : PropertySpec(name, BEDB_PROPSPEC_NAMESPACE, Mode::Normal, compress) {}
};

//=======================================================================================
//! The names of properties in the "be_Db" namespace. These properties are
//! common to all Db files. These properties are created by the BeSQLite::Db API itself
//! and should not be used by other software layers.
// @bsiclass
//=======================================================================================
struct Properties
{
    static PropSpec DbGuid()            {return PropSpec("DbGuid");}
    static PropSpec ProfileVersion()    {return PropSpec("SchemaVersion");}
    static PropSpec ProjectGuid()       {return PropSpec("ProjectGuid");}
    static PropSpec EmbeddedFileBlob()  {return PropSpec(BEDB_PROPSPEC_EMBEDBLOB_NAME, PropertySpec::Compress::No);}
    static PropSpec CreationDate()      {return PropSpec("CreationDate");}
    static PropSpec ExpirationDate()    {return PropSpec("ExpirationDate");}

    //! Build version of BeSqlite (e.g. 06.10.00.00) used to create this database; useful for diagnostics.
    static PropSpec BeSQLiteBuild()     {return PropSpec("BeSQLiteBuild");}
};

//=======================================================================================
//! Utility to compress/decompress Db-s
// @bsiclass
//=======================================================================================
struct LzmaUtility
{
    //! Compress a Db file
    static BE_SQLITE_EXPORT ZipErrors CompressDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progressTracker, uint32_t dictionarySize, bool supportRandomAccess);

    // Decompress a Db file
    static BE_SQLITE_EXPORT ZipErrors DecompressDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progress);

    //! Decompress the blob of an EmbeddedFile in the Db. It is assumed that the blob does not have its own header and trailer, but rather the standard EmbeddedFile headers.
    //! @remarks To be deprecated
    static ZipErrors DecompressEmbeddedBlob(bvector<Byte>&out, uint32_t expectedSize, void const*inputBuffer, uint32_t inputSize, Byte*header, uint32_t headerSize); //!< @private
};

END_BENTLEY_SQLITE_NAMESPACE
