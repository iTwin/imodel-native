/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/BeSQLite.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <BentleyApi/BentleyApi.h>
#include <Bentley/bset.h>

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
database. We refer to each user's copy of the database as a "repository" (following the convention of Mercurial and Git
for Distributed Revision Control, see http://en.wikipedia.org/wiki/Distributed_revision_control.) Each repository stores
its own 4-byte identifier, referred to as its @c BeRepositoryId, that distinguishes it from all other repositories.
Changes made in each repository can then be merged together in an orderly fashion by coordinating with a team server.
Team servers are also responsible for assigning and tracking BeRepositoryIds.

@section OVRBeSQLiteIds 3. Ids in BeSQLite

Since each user has a local copy of the repository, Ids in BeSQLite must be carefully managed. There are several
approaches:

    -# Use Globally Unique Identifiers (GUID, See #BeGuid). A globally unique identifier is a 16 byte randomly assigned
value. 16 bytes hold enough data that there is virtually no chance of collision. Unfortunately, 16 bytes will not fit
into registers of current generation computers, making them relatively expensive to handle programmatically. For large
tables, GUIDs can add significant overhead for lookups and are much more expensive than 8-byte values. This usually
makes them a <i>poor choice for primary keys.</i> However, as the name implies, when you use a GUID, you never need to
worry about uniqueness.

    -# Use #RepositoryBasedIds. A RepositoryBasedId is an 8-byte number that is designed to be unique by combining the
4-byte BeRepositoryId with a unique-within-the-repository 4-byte value. This limits the number of potential values
within a BeRepository to 2^32 (4 billion). Of course it permits 4 billion different BeRepositories. It is often the best
compromise of performance and flexibility for tables with lots of activity.

    -# Use #BeServerIssuedId. A BeServerIssuedId is a 4-byte number that is unique because it is issued by a
synchronized id-administration server, enforced outside of the scope of BeSQLite. BeServerIssuedIds therefore cannot be
created locally and require a connection to the (one, globally administered) server to obtain a new value. Generally
they are useful for "rarely changed but often used" ids such as styles and fonts, etc.

@section OVRBeSQLiteProperties 4. Properties and Settings

Every BeSQlite database has a table named "be_Prop" that can be used to save name-id-value triplets, referred to as
Properties. Properties are often used for storing ad-hoc values that don't warrant the creation of a specific
class/table. The "name" for a Property is defined by a #PropertySpec that includes a two-part namespace/name to uniquely
identify the property. The id for a Property is also a two-part majorId/subId pair to permit arrays.

A PropertySpec has a flag that indicates that it is a "Setting." When Settings are changed, their value remains in
effect only for the duration of the session, unless an explicit call to #Db::SaveSettings is made. If the database is
closed without a call to Db::SaveSettings, the changes are lost.

@section OVRBeSQLiteProperties 5. Repository Local Values

Every BeSQLite database has a table named "be_local" that can be used to save name-value pairs that are specific to the
BeRepository and not merged with the team server. They are used to keep track of the state of the local file and are not
considered part of the "real" database.

@section OVRBeSQLiteEmbeddedFiles 6. Embedded Files

Every BeSQLite database has a table named "be_EmbedFile" that holds copies of files that are "embedded in" the database.
These files are stored as blobs, and are not directly accessible by external applications. Instead, BeSQLite provides
methods to extract them into temporary locations.

@section OVRBeSQLiteLanguageSupport 7. Support for language-specific collation and case-folding

By default, BeSQLite does not support language-specific collation, and performs case-folding only for the ASCII
character set. However, applications can extend BeSQLite by implementing the #BeSQLiteLib::ILanguageSupport interface.

*/

//__PUBLISH_SECTION_END__

#ifdef __BE_SQLITE_HOST_DLL__
    #define BE_SQLITE_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
#define BE_SQLITE_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__
#endif


//  Copied from sqlite3-1.c
#define SQLITE_PENDING_BYTE     (0x40000000)
#define SQLITE_RESERVED_BYTE    (PENDING_BYTE+1)
#define SQLITE_SHARED_FIRST     (PENDING_BYTE+2)
#define SQLITE_SHARED_SIZE      510

//__PUBLISH_SECTION_START__
#define BEDB_TABLE_Property     "be_Prop"
#define BEDB_TABLE_EmbeddedFile "be_EmbedFile"
#define BEDB_TABLE_Local        "be_Local"
#define BEDB_MemoryDb           ":memory:"

#define PROPERTY_APPNAME_Package "pkge_Main"
#define PROPERTY_APPNAME_Imodel  "imodel"

#define BEGIN_BENTLEY_SQLITE_NAMESPACE BEGIN_BENTLEY_API_NAMESPACE namespace BeSQLite {
#define END_BENTLEY_SQLITE_NAMESPACE   } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SQLITE using namespace BentleyApi::BeSQLite;

#include <Bentley/BeFile.h>
#include <Bentley/BeThread.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/RefCounted.h>
#include <Bentley/DateTime.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeVersion.h>
#include "BeAppData.h"
//__PUBLISH_SECTION_END__

#include <Bentley/BeAssert.h>
#include <zlib/zlib.h>
//__PUBLISH_SECTION_START__

// this is used to quiet compiler warnings for variables only used in asserts
#define UNUSED_VARIABLE(x) (void)(x)

#define BESQLITE_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_SQLITE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_SQLITE_NAMESPACE

BENTLEY_API_TYPEDEFS (BeGuid);
BESQLITE_TYPEDEFS (Db);
BESQLITE_TYPEDEFS (DbFile);
BESQLITE_TYPEDEFS (Statement);

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

typedef struct _CLzma2EncProps CLzma2EncProps;
typedef struct _CLzmaEncProps CLzmaEncProps;
#endif // DOCUMENTATION_GENERATOR

BEGIN_BENTLEY_API_NAMESPACE
typedef struct BeGuid BeDbGuid;
typedef Byte const* ByteCP;

//=======================================================================================
//! A 16-byte Globally Unique Id. A value of all zeros means "Invalid Id".
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct BeGuid
{
    union{struct _GUID g; uint64_t u[2]; uint32_t i[4]; uint8_t b[16];} m_guid;

    //! Construct a new BeGuid. Optionally, initialize to a new unique value.
    //! @param[in] createValue if true, the BeGuid will hold a new globally unique value, otherwise the value is invalid (all zeros).
    explicit BeGuid(bool createValue=true) {if (createValue) Create(); else Invalidate();}

    //! Initialize this BeGuid from two 64 bit values. Caller must ensure these values constitute a valid BeGuid.
    void Init(uint64_t u1, uint64_t u0){m_guid.u[0]=u0; m_guid.u[1]=u1;}

    //! Initialize this BeGuid from four 32 bit values. Caller must ensure these values constitute a valid BeGuid.
    void Init(uint32_t i3, uint32_t i2, uint32_t i1, uint32_t i0){m_guid.i[0]=i0; m_guid.i[1]=i1; m_guid.i[2]=i2; m_guid.i[3]=i3;}

    //! Compare two BeGuids for equality
    bool operator==(BeGuid const& rhs) const {return rhs.m_guid.u[0]==m_guid.u[0] && rhs.m_guid.u[1]==m_guid.u[1];}

    //! Compare two BeGuids for inequality
    bool operator!=(BeGuid const& rhs) const {return !(*this==rhs);}

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
//! A unique Id for a BeRepository (a particular copy of a BeSQLite::Db is referred to as a BeRepository.)
//! Whenever more than one BeRepository of a the same Db exists, each of them must have a unique identifier to facilitate
//! change merging via BeRepositoryBasedId's.
//! <p> This strategy relies on of uniqueness of BeRepositoryId's, but that must be enforced by infrastructure outside of BeSQLite.
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct BeRepositoryId
{
    int32_t m_id;
    BeRepositoryId GetNextRepositoryId() const {return BeRepositoryId(m_id+1);}
    BeRepositoryId() {Invalidate();}            //!< Construct an invalid BeRepositoryId.
    explicit BeRepositoryId(int32_t u) {m_id=u;} //!< Construct a BeRepositoryId from a 32 bit value.
    void Invalidate() {m_id = -1;}              //!< Set this BeRepositoryId to the invalid id value (-1).
    bool IsValid() const {return m_id >= 0 ;}   //!< Test to see whether this RepositoryId is valid. Negative Ids are not valid.
    int32_t GetValue() const {BeAssert(IsValid()); return m_id;}      //!< Get the repository id as an Int32
};

//=======================================================================================
// Base class for 2-part 64 bit Ids. Subclasses must supply Validate and Invalidate methods.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
template <typename Derived> struct BeInt64Id
{
    int64_t m_id;

    //! Construct an invalid BeInt64Id
    BeInt64Id() {static_cast<Derived*>(this)->Invalidate();}

    //! Construct a BeInt64Id from a 64 bit value.
    explicit BeInt64Id(int64_t u) : m_id(u) {}

    //! Move constructor.
    BeInt64Id(BeInt64Id&& rhs) {m_id = rhs.m_id;}

    //! Construct a copy.
    BeInt64Id(BeInt64Id const& rhs) {m_id = rhs.m_id;}

    //! Construct a BeInt64Id from a RepositoryId value and an id.
    BeInt64Id(BeRepositoryId repositoryId, uint32_t id) : m_id((((int64_t) repositoryId.GetValue()) << 32 | id)) {}

    bool IsValid() const {return static_cast<Derived const*>(this)->Validate();}

    //! Compare two BeInt64Id for equality
    bool operator==(BeInt64Id const& rhs) const {return rhs.m_id==m_id;}

    //! Compare two BeInt64Id for inequality
    bool operator!=(BeInt64Id const& rhs) const {return !(*this==rhs);}

    //! Compare two BeInt64Id
    bool operator<(BeInt64Id const& rhs) const  {return m_id<rhs.m_id;}
    bool operator<=(BeInt64Id const& rhs) const {return m_id<=rhs.m_id;}
    bool operator>(BeInt64Id const& rhs) const  {return m_id>rhs.m_id;}
    bool operator>=(BeInt64Id const& rhs) const {return m_id>=rhs.m_id;}

    //! Get the 64 bit value of this BeInt64Id
    int64_t GetValue() const {BeAssert(IsValid()); return m_id;}

    void UseNext() {++m_id; BeAssert(IsValid());}

    //! Get the 64 bit value of this BeGuid. Does not check for valid value in debug builds.
    int64_t GetValueUnchecked() const {return m_id;}
    };

//=======================================================================================
//! A 8-byte value that is locally unique within a BeRepository. Since BeRepositoryId's are forced to be unique externally, a BeRepositoryBasedId
//! can be assumed to be globally unique. This provides a more efficient strategy for id values than using true 128-bit GUIDs.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct BeRepositoryBasedId : BeInt64Id<BeRepositoryBasedId>
{
    BeRepositoryBasedId() {Invalidate();}

    //! Construct a BeRepositoryBasedId from a 64 bit value.
    explicit BeRepositoryBasedId(int64_t val) : BeInt64Id(val) {}

    //! Move constructor.
    BeRepositoryBasedId(BeRepositoryBasedId&& rhs) : BeInt64Id<BeRepositoryBasedId> (std::move(rhs)) {}

    //! Constructs a copy.
    BeRepositoryBasedId(BeRepositoryBasedId const& rhs) : BeInt64Id<BeRepositoryBasedId>(rhs) {}

    //! Copies values from another instance.
    BeRepositoryBasedId& operator=(BeRepositoryBasedId const& rhs) {m_id = rhs.m_id; return *this;}

    //! Construct a BeRepositoryBasedId from a RepositoryId value and an id.
    BeRepositoryBasedId(BeRepositoryId repositoryId, uint32_t id) : BeInt64Id(repositoryId,id) {}

    //! Test to see whether this BeRepositoryBasedId is valid. 0 and -1 are not valid ids.
    bool Validate() const {return m_id!=0 && m_id!=-1;}

    //! Set this BeRepositoryBasedId to an invalid value (-1).
    void Invalidate() {m_id = -1;}
};

#define BEREPOSITORYBASED_ID_SUBCLASS(classname,superclass) struct classname : superclass {classname() : superclass() {}  \
    classname(classname&& rhs) : superclass(std::move(rhs)) {} \
    classname(classname const& rhs) : superclass(rhs) {} \
    classname& operator=(classname const& rhs) {m_id = rhs.m_id; return *this;} \
    explicit classname(int64_t v) : superclass(v) {} \
    classname(BeRepositoryId repositoryId, uint32_t id) : superclass(repositoryId,id){} \
    private: explicit classname(int32_t v) : superclass() {} /* private to catch int vs. Id issues */ };

#define BEREPOSITORYBASED_ID_CLASS(classname) BEREPOSITORYBASED_ID_SUBCLASS(classname,BeRepositoryBasedId)

//=======================================================================================
// Base class for 32 bit Ids. Subclasses must supply GetInvalidValue.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
template <typename Derived, uint32_t s_invalidValue> struct BeUInt32Id
{
    uint32_t m_id;

    //! Construct an invalid BeUInt32Id
    BeUInt32Id() {Invalidate();}

    //! Construct a BeUInt32Id from a 32 bit value.
    explicit BeUInt32Id(uint32_t u) : m_id(u) {}

    bool IsValid() const {return Validate();}

    //! Compare two BeUInt32Id's for equality
    bool operator==(BeUInt32Id const& rhs) const {return rhs.m_id==m_id;}

    //! Compare two BeUInt32Id's for inequality
    bool operator!=(BeUInt32Id const& rhs) const {return !(*this==rhs);}

    //! Compare two BeUInt32Id's
    bool operator<(BeUInt32Id const& rhs) const  {return m_id<rhs.m_id;}
    bool operator<=(BeUInt32Id const& rhs) const {return m_id<=rhs.m_id;}
    bool operator>(BeUInt32Id const& rhs) const  {return m_id>rhs.m_id;}
    bool operator>=(BeUInt32Id const& rhs) const {return m_id>=rhs.m_id;}

    //! Get the 32 bit value of this BeUInt32Id
    uint32_t GetValue() const {static_cast<Derived const*>(this)->CheckValue(); return m_id;}

    //! Test to see whether this BeServerIssuedId is valid.
    bool Validate() const {return m_id!=s_invalidValue;}

    //! Set this BeRepositoryBasedId to the invalid value).
    void Invalidate() {m_id = s_invalidValue;}

    //! only for internal callers that understand the semantics of invalid IDs.
    uint32_t GetValueUnchecked() const {return m_id;}
};

//=======================================================================================
//! A 4-byte Id value that must be requested from an external authority that enforces uniqueness.
// @bsiclass                                                    Keith.Bentley   02/13
//=======================================================================================
struct BeServerIssuedId : BeUInt32Id<BeServerIssuedId,0xffffffff>
{
    //! Construct an invalid BeServerIssuedId
    BeServerIssuedId() {Invalidate();}

    //! Construct a BeServerIssuedId from a 32 bit value.
    explicit BeServerIssuedId(uint32_t u) : BeUInt32Id (u) {}

    void CheckValue() const { BeAssert (IsValid()); }
};

#define BESERVER_ISSUED_ID_CLASS(classname) struct classname : BeServerIssuedId {classname() : BeServerIssuedId() {} explicit classname(uint32_t u) : BeServerIssuedId(u) {}};

//=======================================================================================
//! An 8-byte "Locally Unique" Id. A value of all zeros means invalid. Generally, this type is used for Ids whose values are
//! maintained on a per-file basis (hence, "local" means "within this file").
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct BeLuid
{
    union{int64_t u; int32_t i[2]; int16_t s[4]; char b[8];} m_luid;

    //! Construct an invalid BeLuid
    BeLuid() {Invalidate();}

    //! Construct a BeLuid from a 64 bit value.
    explicit BeLuid(int64_t u) {Init(u);}

    //! Construct a BeLuid from two 32 bit values.
    BeLuid(int32_t i1, int32_t i0) {Init(i1,i0);}

    //! Initialize this BeLuid from a 64 bit value.
    void Init(int64_t u){m_luid.u=u;}

    //! Initialize this BeLuid from two 32 bit values.
    void Init(int32_t i1, int32_t i0){m_luid.i[0]=i0; m_luid.i[1]=i1;}

    //! Initialize this BeLuid from four 16 bit values.
    void Init(int16_t s3, int16_t s2, int16_t s1, int16_t s0){m_luid.s[0]=s0; m_luid.s[1]=s1; m_luid.s[2]=s2; m_luid.s[3]=s3;}

    //! Compare two BeLuids for equality
    bool operator==(BeLuid const& rhs) const {return rhs.m_luid.u==m_luid.u;}

    //! Compare two BeLuids for inequality
    bool operator!=(BeLuid const& rhs) const {return !(*this==rhs);}

    //! Set this BeLuid to the invalid id value (all zeros).
    void Invalidate() {m_luid.u = 0;}

    //! Test to see whether this BeLuid is non-zero
    bool IsValid() const {return 0!=m_luid.u;}

    // Get the 64 bit value of this BeLuid
    int64_t GetValue() const {BeAssert(IsValid()); return  m_luid.u;}

    //! Assign a new random id for this BeLuid. Old value is overwritten.
    BE_SQLITE_EXPORT void CreateRandom();
};

END_BENTLEY_API_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
typedef struct sqlite3_blob* SqlDbBlobP;
typedef struct sqlite3* SqlDbP;
typedef struct sqlite3& SqlDbR;
typedef struct sqlite3_stmt* SqlStatementP;
typedef struct sqlite3_session* SqlSessionP;
typedef struct sqlite3_changeset_iter* SqlChangesetIterP;
typedef struct Mem* SqlValueP;
#endif

struct ICompressProgressTracker;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/11
//=======================================================================================
enum
{
    DbUserVersion           = 10,  //!< the "user" version of SQLite databases created by this version of the BeSQLite library
    NoCompressionLevel      = 0,   //!< do not compress data
    FastCompressionLevel    = 1,   //!< use the fastest compression level (see Zlib documentation for explanation).
    DefaultCompressionLevel = 3,   //!< use the default compression level. This is pretty good tradeoff for size vs. speed.
    MaxCompressionLevel     = 9,   //!< the maximum compression level. Can be very slow but results in smallest size.
};

//=======================================================================================
// The Schema version for BeSQLite database files created by this version of the BeSQLite library
// @bsiclass                                                    Keith.Bentley   11/12
//=======================================================================================
enum DbSchemaValues
    {
    BEDB_CURRENT_VERSION_Major = 3,
    BEDB_CURRENT_VERSION_Minor = 1,
    BEDB_CURRENT_VERSION_Sub1  = 0,
    BEDB_CURRENT_VERSION_Sub2  = 1,

    BEDB_SUPPORTED_VERSION_Major = BEDB_CURRENT_VERSION_Major,  // oldest version of the db schema supported by current api
    BEDB_SUPPORTED_VERSION_Minor = 0,
    BEDB_SUPPORTED_VERSION_Sub1  = 0,
    BEDB_SUPPORTED_VERSION_Sub2  = 0,
    };

enum IModelSchemaValues
    {
    PACKAGE_CURRENT_VERSION_Major = 1,
    PACKAGE_CURRENT_VERSION_Minor = 1,
    PACKAGE_CURRENT_VERSION_Sub1  = 0,
    PACKAGE_CURRENT_VERSION_Sub2  = 0,

    PACKAGE_SUPPORTED_VERSION_Major = PACKAGE_CURRENT_VERSION_Major,  // oldest version of the package schema supported by current api
    PACKAGE_SUPPORTED_VERSION_Minor = PACKAGE_CURRENT_VERSION_Minor,
    PACKAGE_SUPPORTED_VERSION_Sub1  = 0,
    PACKAGE_SUPPORTED_VERSION_Sub2  = 0,
    };

//=======================================================================================
//! A 4-digit number that specifies the version of the "schema" of a Db
// @bsiclass                                                    Keith.Bentley   02/12
//=======================================================================================
struct SchemaVersion : BeVersion
{
public:
    SchemaVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : BeVersion(major, minor, sub1, sub2) {}
    explicit SchemaVersion(Utf8CP json) { FromJson(json); }
    BE_SQLITE_EXPORT Utf8String ToJson() const;
    BE_SQLITE_EXPORT void FromJson(Utf8CP);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/11
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
    BE_SQLITE_CONSTRAINT_BASE  = 19,   //!< Abort due to constraint violation. See extended error values.
    BE_SQLITE_MISMATCH    = 20,   //!< Data type mismatch
    BE_SQLITE_MISUSE      = 21,   //!< Library used incorrectly
    BE_SQLITE_NOLFS       = 22,   //!< Uses OS features not supported on host
    BE_SQLITE_AUTH        = 23,   //!< Authorization denied
    BE_SQLITE_FORMAT      = 24,   //!< Auxiliary database format error
    BE_SQLITE_RANGE       = 25,   //!< 2nd parameter to Bind out of range
    BE_SQLITE_NOTADB      = 26,   //!< File opened that is not a database file
    BE_SQLITE_ROW         = 100,  //!< Step() has another row ready
    BE_SQLITE_DONE        = 101,  //!< Step() has finished executing

    BE_SQLITE_IOERR_READ              = (BE_SQLITE_IOERR    | (1<<8)),
    BE_SQLITE_IOERR_SHORT_READ        = (BE_SQLITE_IOERR    | (2<<8)),
    BE_SQLITE_IOERR_WRITE             = (BE_SQLITE_IOERR    | (3<<8)),
    BE_SQLITE_IOERR_FSYNC             = (BE_SQLITE_IOERR    | (4<<8)),
    BE_SQLITE_IOERR_DIR_FSYNC         = (BE_SQLITE_IOERR    | (5<<8)),
    BE_SQLITE_IOERR_TRUNCATE          = (BE_SQLITE_IOERR    | (6<<8)),
    BE_SQLITE_IOERR_FSTAT             = (BE_SQLITE_IOERR    | (7<<8)),
    BE_SQLITE_IOERR_UNLOCK            = (BE_SQLITE_IOERR    | (8<<8)),
    BE_SQLITE_IOERR_RDLOCK            = (BE_SQLITE_IOERR    | (9<<8)),
    BE_SQLITE_IOERR_DELETE            = (BE_SQLITE_IOERR    | (10<<8)),
    BE_SQLITE_IOERR_BLOCKED           = (BE_SQLITE_IOERR    | (11<<8)),
    BE_SQLITE_IOERR_NOMEM             = (BE_SQLITE_IOERR    | (12<<8)),
    BE_SQLITE_IOERR_ACCESS            = (BE_SQLITE_IOERR    | (13<<8)),
    BE_SQLITE_IOERR_CHECKRESERVEDLOCK = (BE_SQLITE_IOERR    | (14<<8)),
    BE_SQLITE_IOERR_LOCK              = (BE_SQLITE_IOERR    | (15<<8)),
    BE_SQLITE_IOERR_CLOSE             = (BE_SQLITE_IOERR    | (16<<8)),
    BE_SQLITE_IOERR_DIR_CLOSE         = (BE_SQLITE_IOERR    | (17<<8)),
    BE_SQLITE_IOERR_SHMOPEN           = (BE_SQLITE_IOERR    | (18<<8)),
    BE_SQLITE_IOERR_SHMSIZE           = (BE_SQLITE_IOERR    | (19<<8)),
    BE_SQLITE_IOERR_SHMLOCK           = (BE_SQLITE_IOERR    | (20<<8)),
    BE_SQLITE_IOERR_SHMMAP            = (BE_SQLITE_IOERR    | (21<<8)),
    BE_SQLITE_IOERR_SEEK              = (BE_SQLITE_IOERR    | (22<<8)),
    BE_SQLITE_IOERR_DELETE_NOENT      = (BE_SQLITE_IOERR    | (23<<8)),

    BE_SQLITE_ERROR_FileExists        = (BE_SQLITE_IOERR | (1<<24)),  //!< attempt to create a new file when a file by that name already exists
    BE_SQLITE_ERROR_AlreadyOpen       = (BE_SQLITE_IOERR | (2<<24)),  //!< attempt to open a BeSQLite::Db that is already in use somewhere.
    BE_SQLITE_ERROR_NoPropertyTable   = (BE_SQLITE_IOERR | (3<<24)),  //!< attempt to open a BeSQLite::Db that doesn't have a property table.
    BE_SQLITE_ERROR_FileNotFound      = (BE_SQLITE_IOERR | (4<<24)),  //!< the database name is not a file.
    BE_SQLITE_ERROR_NoTxnActive       = (BE_SQLITE_IOERR | (5<<24)),  //!< there is no transaction active and the database was opened with AllowImplicitTransactions=false
    BE_SQLITE_ERROR_BadDbSchema       = (BE_SQLITE_IOERR | (6<<24)), //!< wrong BeSQLite schema version
    BE_SQLITE_ERROR_InvalidProfileVersion = (BE_SQLITE_IOERR | (7<<24)),  //!< Profile (aka application level BeSQLite schema) of file could not be determined.
    BE_SQLITE_ERROR_ProfileUpgradeFailed = (BE_SQLITE_IOERR | (8<<24)),  //!< Upgrade of profile (aka application level BeSQLite schema) of file failed.
    BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite = (BE_SQLITE_IOERR | (9<<24)),  //!< Upgrade of profile (aka application level SQLite schema) of file failed because the file could not be reopened in read-write mode.
    BE_SQLITE_ERROR_ProfileTooOld     = (BE_SQLITE_IOERR | (10<<24)),  //!< Profile (aka application level BeSQLite schema) of file is too old. Therefore file cannot be opened.
    BE_SQLITE_ERROR_ProfileTooNewForReadWrite = (BE_SQLITE_IOERR | (11<<24)),  //!< Profile (aka application level schema) of file is too new for read-write access. Therefore file can only be opened read-only.
    BE_SQLITE_ERROR_ProfileTooNew     = (BE_SQLITE_IOERR | (12<<24)),  //!< Profile (aka application level SQLite schema) of file is too new. Therefore file cannot be opened.
    BE_SQLITE_ERROR_ChangeTrackError  = (BE_SQLITE_IOERR | (13<<24)),  //!< attempt to commit with active changetrack

    BE_SQLITE_LOCKED_SHAREDCACHE      = (BE_SQLITE_LOCKED   | (1<<8)),

    BE_SQLITE_BUSY_RECOVERY           = (BE_SQLITE_BUSY     | (1<<8)),

    BE_SQLITE_CANTOPEN_NOTEMPDIR      = (BE_SQLITE_CANTOPEN | (1<<8)),
    BE_SQLITE_CANTOPEN_ISDIR          = (BE_SQLITE_CANTOPEN | (2<<8)),
    BE_SQLITE_CANTOPEN_FULLPATH       = (BE_SQLITE_CANTOPEN | (3<<8)),

    BE_SQLITE_CORRUPT_VTAB            = (BE_SQLITE_CORRUPT | (1<<8)),

    BE_SQLITE_READONLY_RECOVERY       = (BE_SQLITE_READONLY | (1<<8)),
    BE_SQLITE_READONLY_CANTLOCK       = (BE_SQLITE_READONLY | (2<<8)),
    BE_SQLITE_READONLY_ROLLBACK       = (BE_SQLITE_READONLY | (3<<8)),

    BE_SQLITE_ABORT_ROLLBACK          = (BE_SQLITE_ABORT | (2<<8)),

    BE_SQLITE_CONSTRAINT_CHECK        = (BE_SQLITE_CONSTRAINT_BASE | (1<<8)),
    BE_SQLITE_CONSTRAINT_COMMITHOOK   = (BE_SQLITE_CONSTRAINT_BASE | (2<<8)),
    BE_SQLITE_CONSTRAINT_FOREIGNKEY   = (BE_SQLITE_CONSTRAINT_BASE | (3<<8)),
    BE_SQLITE_CONSTRAINT_FUNCTION     = (BE_SQLITE_CONSTRAINT_BASE | (4<<8)),
    BE_SQLITE_CONSTRAINT_NOTNULL      = (BE_SQLITE_CONSTRAINT_BASE | (5<<8)),
    BE_SQLITE_CONSTRAINT_PRIMARYKEY   = (BE_SQLITE_CONSTRAINT_BASE | (6<<8)),
    BE_SQLITE_CONSTRAINT_TRIGGER      = (BE_SQLITE_CONSTRAINT_BASE | (7<<8)),
    BE_SQLITE_CONSTRAINT_UNIQUE       = (BE_SQLITE_CONSTRAINT_BASE | (8<<8)),
    BE_SQLITE_CONSTRAINT_VTAB         = (BE_SQLITE_CONSTRAINT_BASE | (9<<8)),
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum class DbOpcode : int
{
    Delete  = 9,
    Insert  = 18,
    Update  = 23,
};

inline int GetBaseDbResult(DbResult val) {return 0xff & val;}
inline bool TestBaseDbResult(DbResult val1, DbResult val2) {return GetBaseDbResult(val1) == GetBaseDbResult(val2);}
inline bool IsConstraintDbResult(DbResult val1) {return GetBaseDbResult(val1) == BE_SQLITE_CONSTRAINT_BASE;}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/11
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
// @bsiclass                                                    Keith.Bentley   04/11
//=======================================================================================
struct BeSQLiteLib
{
public:
    //=======================================================================================
    //! This is an interface class that allows applications to provide custom language processing for SQL case and collation operations.
    //! While a single static instance of this class is registered, collations are registered on a per-database basis. They are <i>not</i> expected to vary per database.
    // @bsiclass                                                    Jeff.Marker     01/14
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
        virtual void _Lower(Utf16CP source, int sourceLen, Utf16CP result, int resultLen) = 0;

        //! Converts source to upper-case into result according to localeName. result cannot be reallocated, and is typically over-allocated based on source.
        //! This is called when the SQL scalar function UPPER is processed.
        virtual void _Upper(Utf16CP source, int sourceLen, Utf16CP result, int resultLen) = 0;

        //! Registers a collection of collations with the database.
        //! This is called when every database is opened, and collatorFreeFunc is called when the database is closed for each collator provided.
        virtual void _InitCollation(bvector<CollationEntry>& collations, CollationUserDataFreeFunc& collatorFreeFunc) = 0;

        //! Compares two strings for sorting purposes. collator is the m_collator object provided in the corresponding CollationEntry.
        //! This is called when a custom collation is processed in a SQL query (e.g. in an ORDER BY clause).
        virtual int _Collate(Utf16CP lhs, int lhsLen, Utf16CP rhs, int rhsLen, void* collator) = 0;

        //! Maps the given UTF-32 character to its case folding equivalent (i.e. a normalized form used for comparison). This is primarily used in the LIKE operator.
        //! If the character has no case folding equivalent, the character itself is returned.
        virtual uint32_t _FoldCase(uint32_t) = 0;
    }; // ILanguageSupport

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
    static void Randomness(int numBytes, void* random);

    //! Allocate a buffer using SQLite's memory manager.
    static void* MallocMem(int sz);

    //! Re-allocate a buffer returned from MallocMem.
    static void* ReallocMem(void* p, int sz);

    //! Free memory allocated by MallocMem.
    static void FreeMem(void* p);

    BE_SQLITE_EXPORT static int CloseSqlDb(void* p);

    BE_SQLITE_EXPORT static void SetDownloadAdmin(struct IDownloadAdmin&);

    BE_SQLITE_EXPORT static struct IDownloadAdmin* GetDownloadAdmin();

    //! Sets the static ILanguageSupport object for handling custom language processing.
    //! This should be called once per session before opening any databases and applies to all future opened databases.
    BE_SQLITE_EXPORT static void SetLanguageSupport(ILanguageSupport*);

    //! Gets the current ILanguageSupport. Can return NULL.
    BE_SQLITE_EXPORT static ILanguageSupport* GetLanguageSupport();
};

//=======================================================================================
//! A wrapper for a SQLite Prepared Statement. Every BeSQLite::Statement object associated with a BeSQLite::Db must be deleted
//! before the database is closed.
// @bsiclass                                                    Keith.Bentley   12/10
//=======================================================================================
struct Statement : NonCopyableClass
{
private:
    SqlStatementP m_stmt;

    DbResult DoPrepare(SqlDbP db, Utf8CP sql);

public:
    enum class MakeCopy : bool {No=0, Yes=1};
    explicit Statement(SqlStatementP stmt) {m_stmt=stmt;}

    //! construct a new blank Statement.
    Statement() {m_stmt=nullptr; }
    Statement(BeSQLiteDbCR db, Utf8CP sql) {m_stmt=nullptr; Prepare(db, sql);}
    ~Statement() {Finalize();}

    SqlStatementP& GetStmtR() {return m_stmt;} //! @private internal use only
    DbResult Prepare(DbFileCR, Utf8CP sql); //! @private internal use only

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

    //! Perform a single step on this (previously prepared) Statement
    //! @see sqlite3_step
    BE_SQLITE_EXPORT DbResult Step();

    //! Reset this Statement
    //! @see sqlite3_reset
    BE_SQLITE_EXPORT DbResult Reset();

    //! Clear the bindings of this Statement
    //! @see sqlite3_clear_bindings
    BE_SQLITE_EXPORT DbResult ClearBindings();

    //! Bind an integer value to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int
    BE_SQLITE_EXPORT DbResult BindInt(int paramNum, int value);

    //! Bind an Int64 value to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int64
    BE_SQLITE_EXPORT DbResult BindInt64(int paramNum, int64_t value);

    //! Bind a BeRepositoryBasedId value to a parameter of this (previously prepared) Statement. Binds NULL if the id is not valid.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    template <class T_Id> DbResult BindId(int paramNum, BeInt64Id<T_Id> value) {return value.IsValid() ? BindInt64(paramNum,value.GetValue()) : BindNull(paramNum);}

    //! Bind a BeRepositoryBasedId value to a parameter of this (previously prepared) Statement. Binds NULL if the id is not valid.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    DbResult BindId(int paramNum, BeRepositoryBasedId value) {return value.IsValid() ? BindInt64(paramNum,value.GetValue()) : BindNull(paramNum);}

    //! Bind a BeServerIssuedId value to a parameter of this (previously prepared) Statement. Binds NULL if the id is not valid.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    DbResult BindId(int paramNum, BeServerIssuedId value) {return value.IsValid() ? BindInt(paramNum,value.GetValue()) : BindNull(paramNum);}

    //! Bind a double value to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_double
    BE_SQLITE_EXPORT DbResult BindDouble(int paramNum, double value);

    //! Bind the value of a Utf8String to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] stringValue the value to bind.
    //! @param[in] makeCopy Make a private copy of the string in the Statement. Only pass Statement::MakeCopy::No if stringValue will remain valid until the Statement's bindings are cleared.
    //! @see sqlite3_bind_text
    DbResult BindText(int paramNum, Utf8StringCR stringValue, MakeCopy makeCopy) {return BindText(paramNum, stringValue.c_str(), makeCopy, (int)stringValue.size());}

    //! Bind the value of a Utf8CP to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] stringValue the value to bind.
    //! @param[in] makeCopy Make a private copy of the string in the Statement. Only pass Statement::MakeCopy::No if stringValue will remain valid until the Statement's bindings are cleared.
    //! @param[in] nBytes The number of bytes (not characters) in @p stringValue. If negative, it will be calculated from stringValue. Passing this value is only an optimization.
    //! @see sqlite3_bind_text
    BE_SQLITE_EXPORT DbResult BindText(int paramNum, Utf8CP stringValue, MakeCopy makeCopy, int nBytes=-1);

    //! Bind a BeGuid to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @note BeGuids are saved as a 16-byte blob in the database.
    BE_SQLITE_EXPORT DbResult BindGuid(int paramNum, BeGuidCR value);

    //! Bind a BeLuid to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] value the value to bind.
    //! @see sqlite3_bind_int64
    BE_SQLITE_EXPORT DbResult BindLuid(int paramNum, BeLuid value);

    //! Bind a zero-blob of the specified size to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] size The number of bytes for the blob.
    //! @see sqlite3_bind_zeroblob
    BE_SQLITE_EXPORT DbResult BindZeroBlob(int paramNum, int size);

    //! Bind a blob to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] blobValue the value to bind.
    //! @param[in] size The number of bytes in blobValue
    //! @param[in] makeCopy Make a private copy of the blob in the Statement. Only pass Statement::MakeCopy::No if blobValue will remain valid until the Statement's bindings are cleared.
    //! @see sqlite3_bind_blob
    BE_SQLITE_EXPORT DbResult BindBlob(int paramNum, void const* blobValue, int size, MakeCopy makeCopy);

    //! Bind a null value to a parameter of this (previously prepared) Statement
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @see sqlite3_bind_null
    BE_SQLITE_EXPORT DbResult BindNull(int paramNum);

    //! Bind a VirtualSet. Must be the first parameter of the "InVirtualSet" BeSQLite function.
    //! @param[in] paramNum the SQL parameter number to bind.
    //! @param[in] vSet the VirtualSet to bind.
    //! @see BeSQLite::VirtualSet
    BE_SQLITE_EXPORT DbResult BindVirtualSet(int paramNum, struct VirtualSet const& vSet);

    //! Get the number of columns resulting from Step on this Statement
    //! @see sqlite3_column_count
    BE_SQLITE_EXPORT int GetColumnCount();

    //! Get the type for a column of the result of Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_type
    BE_SQLITE_EXPORT DbValueType GetColumnType(int col);

    //! Determine whether the column value is NULL.
    bool IsColumnNull(int col) {return DbValueType::NullVal == GetColumnType(col);}

    //! Get the name of a column of the result of Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_name
    BE_SQLITE_EXPORT Utf8CP GetColumnName(int col);

    //! Get the number of bytes in a column of the result of Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_bytes
    BE_SQLITE_EXPORT int GetColumnBytes(int col);

    //! Get the number of bytes in a column as a utf16 string. This is only valid after a call to GetValueUtf16
    //! @param[in] col The column of interest
    //! @see sqlite3_column_bytes16
    BE_SQLITE_EXPORT int GetColumnBytes16(int col);

    //! Get the value of a column in the result of Step as a blob
    //! @param[in] col The column of interest
    //! @see sqlite3_column_blob
    BE_SQLITE_EXPORT void const* GetValueBlob(int col);

    //! Get the value of a column in the result of Step as a UTF-8 string
    //! @param[in] col The column of interest
    //! @see sqlite3_column_text
    BE_SQLITE_EXPORT Utf8CP GetValueText(int col);

    //! Get an integer value from a column returned from Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int
    BE_SQLITE_EXPORT int GetValueInt(int col);

    //! Get an Int64 value from a column returned from Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int64
    BE_SQLITE_EXPORT int64_t GetValueInt64(int col);

    //! Get a double value from a column returned from Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_double
    BE_SQLITE_EXPORT double GetValueDouble(int col);

    //! Get a BeLuid value from a column returned from Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_int64
    BeLuid GetValueLuid(int col) {return BeLuid((uint64_t) GetValueInt64(col));}

    //! Get a BeRepositoryBasedId value from a column returned from Step
    //! @param[in] col The column of interest
    template <class T_Id> T_Id GetValueId(int col) {if (!IsColumnNull(col)) {return T_Id(GetValueInt64(col));} return T_Id();}

    //! Get a BeGuid value from a column returned from Step
    //! @param[in] col The column of interest
    //! @see sqlite3_column_blob
    BE_SQLITE_EXPORT BeGuid GetValueGuid(int col);

    //! Get the index of a bound parameter by name.
    //! @param[in] name the name of the bound parameter
    //! @see sqlite3_bind_parameter_index
    BE_SQLITE_EXPORT int GetParameterIndex(Utf8CP name);

    //! Get a saved copy of the original SQL text used to prepare this Statement
    //! @see sqlite3_sql
    BE_SQLITE_EXPORT Utf8CP GetSql() const;

    //! Dump query results to stdout, for debugging purposes
    BE_SQLITE_EXPORT void DumpResults();

    SqlStatementP GetSqlStatementP() const {return m_stmt;}  // for direct use of sqlite3 api
    operator SqlStatementP(){return m_stmt;}                 // for direct use of sqlite3 api
};

//=======================================================================================
//! A Blob handle for incremental I/O. See sqlite3_blob_open for details.
// @bsiclass                                                    Keith.Bentley   06/11
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
    //! @param[in] dbName The name of the database attachment to open. If NULL, use "main".
    //! @return BE_SQLITE_OK on success, error status otherwise.
    //! @see sqlite3_blob_open
    BE_SQLITE_EXPORT DbResult Open(DbR db, Utf8CP tableName, Utf8CP columnName, uint64_t row, bool writable, Utf8CP dbName=0);

    //! Move and existing opened BlobIO to a new row in the same table.
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
    bool IsValid() const {return NULL != m_blob;}
};

//=======================================================================================
//! When enabled, this class maintains a list of "changed rows" (inserts, updates and deletes) for a BeSQLite::Db. This information is
//! stored in memory by this class. At appropriate boundaries, applications convert the contents of a ChangeTracker
//! into a ChangeSet that can be saved as a blob and recorded.
//! @note @li ChangeTrackers keep track of "net changes" so, for example, if a row is added and then subsequently deleted
//! there is no net change. Likewise, if a row is changed more than once, only the net changes are stored and
//! intermediate values are not.
//! @note @li A single database may have more than one ChangeTracker active.
//! @note @li You cannot use nested transactions (via the @ref Savepoint "Savepoint API)
//! if a ChangeTracker is enabled for that file.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct ChangeTracker : RefCountedBase
{
protected:
    bool            m_isTracking;
    Db*             m_db;
    SqlSessionP     m_session;
    Utf8String      m_name;

    friend struct Db;
    friend struct DbFile;
    enum class OnCommitStatus {Continue=0, Abort=1};
    enum class TrackChangesForTable : bool {No=0, Yes=1};

    BE_SQLITE_EXPORT DbResult CreateSession();
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) = 0;
    virtual void _OnSettingsSave() {}
    virtual void _OnSettingsSaved() {}
    void SetDb(Db* db) {m_db = db;}
    Db* GetDb() {return m_db;}
    Utf8CP GetName() const {return m_name.c_str();}

public:
    ChangeTracker(Utf8CP name=NULL) : m_name(name) {m_session=0; m_db=0; m_isTracking=false;}
    virtual ~ChangeTracker() {EndTracking();}
    virtual TrackChangesForTable _FilterTable(Utf8CP tableName) {return TrackChangesForTable::Yes;}
    SqlSessionP GetSqlSession() {return m_session;}

    //! Track all of the differences between the Db of this ChangeTracker and a changed copy of that database.
    //! @param[out] errMsg  If not null, an explanatory error message is returned in case of failure
    //! @param[in] baseFile A different version of the same db
    //! @return BE_SQLITE_OK if hangeset was created; else a non-zero error status if the diff failed. Returns BE_SQLITE_MISMATCH if the two Dbs have different GUIDs.
    //! @note This function will return an error if the two files have different DbGuids. 'baseFile' must identify a version of Db.
    BE_SQLITE_EXPORT DbResult DifferenceToDb(Utf8StringP errMsg, BeFileNameCR baseFile);

    //! Turn off change tracking for a database.
    BE_SQLITE_EXPORT void EndTracking();

    //! Temporarily suspend or resume change tracking
    //! @param[in] val if true, enable tracking.
    BE_SQLITE_EXPORT bool EnableTracking(bool val);

    //! turn on or off the "indirect changes" flag. All changes are marked as either direct or indirect according to the state of this flag.
    //! @param[in] val if true, changes are marked as indirect.
    BE_SQLITE_EXPORT void SetIndirectChanges(bool val);

    //! Determine whether any changes have been tracked by this ChangeTracker.
    BE_SQLITE_EXPORT bool HasChanges();

    //! Clear the contents of this ChangeTracker and re-start it.
    void Restart() {EndTracking(); EnableTracking(true);}
    bool IsTracking() const {return m_isTracking;}

};

//=======================================================================================
//! A "value" from a BeSQLite function.
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct DbValue
{
    SqlValueP m_val;
    DbValue(SqlValueP val) : m_val(val)  {}

    bool IsValid() const {return NULL != m_val;}                    //!< return true if this value is valid
    bool IsNull()  const {return DbValueType::NullVal == GetValueType();} //!< return true if this value is null
    BE_SQLITE_EXPORT DbValueType GetValueType() const;      //!< see sqlite3_value_type
    BE_SQLITE_EXPORT DbValueType GetNumericType() const;    //!< see sqlite3_value_numeric_type
    BE_SQLITE_EXPORT int         GetValueBytes() const;     //!< see sqlite3_value_bytes
    BE_SQLITE_EXPORT void const* GetValueBlob() const;      //!< see sqlite3_value_blob
    BE_SQLITE_EXPORT Utf8CP      GetValueText() const;      //!< see sqlite3_value_text
    BE_SQLITE_EXPORT int         GetValueInt() const;       //!< see sqlite3_value_int
    BE_SQLITE_EXPORT int64_t     GetValueInt64() const;     //!< see sqlite3_value_int64
    BE_SQLITE_EXPORT double      GetValueDouble() const;    //!< see sqlite3_value_double
    BE_SQLITE_EXPORT BeLuid      GetValueLuid() const;      //!< get the value as a Locally unique id
    BE_SQLITE_EXPORT BeGuid      GetValueGuid() const;      //!< get the value as a GUID
    template <class T_Id> T_Id   GetValueId() const {return T_Id (GetValueInt64());}

    BE_SQLITE_EXPORT Utf8String Format(int detailLevel) const; //!< for debugging purposes.
};

//=======================================================================================
//! A user-defined function that can be added to a Db connection and then used in SQL.
//! See http://www.sqlite.org/capi3ref.html#sqlite3_create_function.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct DbFunction : NonCopyableClass
{
public:
    //=======================================================================================
    //! The "context" supplied to DbFunctions that can be used to set result values.
    // @bsiclass                                                    Keith.Bentley   06/14
    //=======================================================================================
    struct Context
        {
        enum class CopyData : int { No = 0, Yes = -1 };                                     //!< see sqlite3_destructor_type
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
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct ScalarFunction : DbFunction
{
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
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct AggregateFunction : DbFunction
{
    bool _IsAggregate() override {return true;}
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
// @bsiclass                                                    Keith.Bentley   05/15
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
    // @bsiclass                                                    Keith.Bentley   04/14
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
    virtual int _TestRange(QueryInfo const&) = 0;

    RTreeMatchFunction(Utf8CP name, int nArgs) : DbFunction(name, nArgs, DbValueType::NullVal) {}
};

//=======================================================================================
//! An aggregate function for queries using the range tree.
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct RTreeAcceptFunction : AggregateFunction
{
    //! This class becomes the callback for an SQL statement that uses a RTree vtable via the "MATCH rTreeMatch(1)" syntax.
    //! It's constructor registers itself as the callback object and it's destructor unregisters itself.
    struct Tester : RTreeMatchFunction
    {
    DbR m_db;

    BE_SQLITE_EXPORT Tester(DbR db);
    virtual void _StepRange(DbFunction::Context&, int nArgs, DbValue* args) = 0;

    //! Perform the RTree search by calling "Step" on the supplied statement. This object becomes the callback function for the "rTreeMatch(1)" sql function
    //! (e.g. "SELECT id FROM rtree_vtable WHERE id MATCH rTreeMatch(1)"
    //! @note this method registers the rTreeMatch function with SQLite, so it is not possible to call Step directly.
    BE_SQLITE_EXPORT DbResult StepRTree(Statement&);
    };

protected:
    Tester* m_rangeTest;
    void _FinishAggregate(DbFunction::Context&) override {}
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override {m_rangeTest->_StepRange(ctx, nArgs, args);}

public:
    RTreeAcceptFunction() : AggregateFunction("rTreeAccept", 1) {m_rangeTest = nullptr;}

    void SetTester(Tester* tester) {m_rangeTest=tester;}
    Tester* GetTester() const {return m_rangeTest;}
};

//=======================================================================================
//! This interface should be implemented to supply the first argument to the BeSQLite function "InVirtualSet".
//! It provides a way to use an in-memory "set of values" in an SQL statement, without having to create a temporary table.
//! For example, to find rows of MyTable that have Owner=1 and Vendor in a list of vendors held in a memory, use the SQL statement:
//! "SELECT * FROM MyTable WHERE Owner=1 AND InVirtualSet(?,Vendor)" and bind the first argument of InVirtualSet using BeSQLite::Statment::BindVirtualSet
//! to an object that implements this interface, returning true for _IsInSet for the desired vendors.
// @bsiclass                                                    Keith.Bentley   11/13
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
};

//=======================================================================================
//! An Iterator for a ChangeSet. This class is used to step through the individual changes within a ChangeSet.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct Changes
{
private:
    struct ChangeSet& m_changeset;
    mutable SqlChangesetIterP m_iter;
    void Finalize() const;

public:
    Changes(Changes const& other) : m_changeset(other.m_changeset) {m_iter=0;}

    //! Construct an iterator for a Changeset
    Changes(ChangeSet& changeset) : m_changeset(changeset) {m_iter=0;};
    BE_SQLITE_EXPORT ~Changes();

    //! A single change to a database row.
    struct Change : std::iterator<std::input_iterator_tag, Change const>
        {
        private:
            bool  m_isValid;
            SqlChangesetIterP m_iter;

        public:
            Change(SqlChangesetIterP iter, bool isValid) {m_iter=iter; m_isValid=isValid;}
            //! get the "operation" that happened to this row.
            //! @param[out] tableName the name of the table to which the change was made. Changes within a ChangeSet are always
            //! sorted by table. So, all of the changes for a given table will appear in order before any changes to another table.
            //! However, it is not possible to predict the order of tables within a ChangeSet.
            //! @param[out] nCols the number of columns in the changed table.
            //! @param[out] opcode the opcode of the change. One of SQLITE_INSERT, SQLITE_DELETE, or SQLITE_UPDATE.
            //! @param[out] indirect true if the change was an indirect change.
            BE_SQLITE_EXPORT DbResult GetOperation(Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect) const;

            //! get the columns that form the primary key for the changed row.
            BE_SQLITE_EXPORT DbResult GetPrimaryKeyColumns(Byte** cols, int* nCols) const;

            //! get the previous value of a changed column.
            //! @param colNum the column number desired.
            //! @note this method is only valid if the opcode is either SQLITE_DELETE or SQLITE_UPDATE.
            //! For SQLITE_UPDATE, any column that was not affected by the change will be invalid.
            BE_SQLITE_EXPORT DbValue GetOldValue(int colNum) const;

            //! get the new value of a changed column.
            //! @param colNum the column number desired.
            //! @note this method is only valid if the opcode is either SQLITE_INSERT or SQLITE_UPDATE.
            //! For SQLITE_UPDATE, any column that was not affected by the change will be invalid.
            BE_SQLITE_EXPORT DbValue GetNewValue(int colNum) const;

            enum class Stage : bool {Old=0, New=1};
            DbValue GetValue(int colNum, Stage stage) const {return (stage==Stage::Old)? GetOldValue(colNum) : GetNewValue(colNum);}

            //! Move the iterator to the next entry in the ChangeSet. After the last valid entry, the entry will be invalid.
            BE_SQLITE_EXPORT Change& operator++();

            bool IsValid() const {return m_isValid;}

            Change const& operator* () const {return *this;}
            bool operator!=(Change const& rhs) const {return (m_iter != rhs.m_iter) || (m_isValid != rhs.m_isValid);}
            bool operator==(Change const& rhs) const {return (m_iter == rhs.m_iter) && (m_isValid == rhs.m_isValid);}

            //! Dump to stdout for debugging purposes.
            BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const;

            void Dump(Db const& db, bool isPatchSet, int detailLevel) const {bset<Utf8String> tablesSeen; Dump(db, isPatchSet, tablesSeen, detailLevel);}

            //! Dump one or more columns to stdout for debugging purposes.
            BE_SQLITE_EXPORT void DumpColumns(int startCol, int endCol, Changes::Change::Stage stage, bvector<Utf8String> const& columns, int detailLevel) const;

            //! Format the primary key columns of this change for debugging purposes.
            BE_SQLITE_EXPORT Utf8String FormatPrimarykeyColumns(bool isInsert, int detailLevel) const;
        };

    typedef Change const_iterator;

    //! Get the first entry in the ChangeSet.
    BE_SQLITE_EXPORT Change begin() const;

    //! Get the last entry in the ChangeSet.
    Change end() const {return Change(m_iter, false);}
};

//=======================================================================================
//! A set of changes to database rows. A ChangeSet is a contiguous set of bytes that serializes everything
//! tracked by a ChangeTracker. It can be constructed from a ChangeTracker and then saved
//! to remember what happened. Then, a ChangeTracker can be recreated from a previously saved state. One
//! way to use a ChangeSet is to transmit it to another computer holding another copy of the same database and then
//! "Apply" those changes. In this way two sets of changes to database can be merged to form a single database
//! with both sets of changes.
//! Optionally, a ChangeSet may be "inverted" from its saved state - meaning it will hold a ChangeSet that reverses all
//! of the original changes (inserts become deletes and vice versa, updates restore the original values.)
//! In this way, ChangeSets can be used to implement application Undo.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct ChangeSet : NonCopyableClass
{
    enum class SetType : bool {Full=0, Patch=1};
    enum class ApplyChangesForTable : bool {No=0, Yes=1};
    enum class ConflictCause : int {Data=1, NotFound=2, Conflict=3, Constraint=4, ForeignKey=5};
    enum class ConflictResolution : int {Skip=0, Replace=1, Abort=2};

private:
    int    m_size;
    void*  m_changeset;

public:
    virtual ApplyChangesForTable _FilterTable(Utf8CP tableName) {return ApplyChangesForTable::Yes;}
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) = 0;

public:
    //! construct a blank, empty ChangeSet
    ChangeSet() {m_size=0; m_changeset=nullptr;}
    ~ChangeSet() {Free();}

    //! Free the data held by this ChangeSet.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    BE_SQLITE_EXPORT void Free();

    //! Re-create this ChangeSet from data from a previously saved ChangeSet.
    BE_SQLITE_EXPORT DbResult FromData(int size, void const* data, bool invert);

    //! Create a ChangeSet or PatchSet from a ChangeTracker. The ChangeSet can then be saved persistently.
    //! @param[in] tracker  ChangeTracker from which to create ChangeSet or PatchSet
    //! @param[in] setType  whether to create a full ChangeSet or just a PatchSet
    BE_SQLITE_EXPORT DbResult FromChangeTrack(ChangeTracker& tracker, SetType setType=SetType::Full);

    //! Apply all of the changes in a ChangeSet to the supplied database.
    //! @param[in] db the database to which the changes are applied.
    BE_SQLITE_EXPORT DbResult ApplyChanges(DbR db);

    BE_SQLITE_EXPORT DbResult ConcatenateWith(ChangeSet const& second);

    //! Get the number of bytes in this ChangeSet.
    int GetSize() const {return  m_size;}

    //! Get a pointer to the data for this ChangeSet.
    void const* GetData() const {return  m_changeset;}

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() {return 0 != m_changeset;}

    //! Dump to stdout for debugging purposes.
    BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet=false, int detailLevel=0) const;

    //! Get a description of a conflict cause for debugging purposes.
    BE_SQLITE_EXPORT static Utf8String InterpretConflictCause(ChangeSet::ConflictCause);

};

//=======================================================================================
//! Wraps sqlite3_mprintf. Adds convenience that destructor frees memory.
// @bsiclass                                                    Keith.Bentley   04/11
//=======================================================================================
struct SqlPrintfString
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
// @bsiclass                                                    Keith.Bentley   06/11
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
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct BeDbMutexHolder : NonCopyableClass
{
    BeDbMutex& m_mutex;
    BeDbMutexHolder(BeDbMutex& mutex) : m_mutex(mutex) {m_mutex.Enter();}
    ~BeDbMutexHolder() {m_mutex.Leave();}
};

//=======================================================================================
//! A reference-counted Statement. Statement is freed when last reference is Released.
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct CachedStatement : Statement
{
private:
    mutable uint32_t m_refCount;
    Utf8CP  m_sql;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    uint32_t AddRef() const {return ++m_refCount;}
    uint32_t GetRefCount() const {return m_refCount;}
    BE_SQLITE_EXPORT uint32_t Release();
    BE_SQLITE_EXPORT CachedStatement(Utf8CP sql);
    BE_SQLITE_EXPORT ~CachedStatement();
    Utf8CP GetSQL() const {return m_sql;}
};

typedef RefCountedPtr<CachedStatement> CachedStatementPtr;

//=======================================================================================
//! A cache of SharedStatements that can be reused without re-Preparing. It can be very expensive to Prepare an SQL statement,
//! so this class provides a way to save previously prepared statements for reuse (note, a prepared Statement is specific to a
//! particular SQLite database, so there is a StatementCache for each BeSQLite::Db)
//! By default, the cache holds 20 SharedStatements and releases the oldest statement when a new entry is added to a full cache.
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct StatementCache : NonCopyableClass
{
private:
    mutable BeDbMutex m_mutex;
    mutable bvector<CachedStatementPtr> m_entries;
    bvector<CachedStatementPtr>::iterator FindEntry(Utf8CP) const;
    BE_SQLITE_EXPORT CachedStatement& AddStatement(Utf8CP) const;
    BE_SQLITE_EXPORT CachedStatement* FindStatement(Utf8CP) const;

public:
    StatementCache(int size){m_entries.reserve(size);}
    ~StatementCache() {Empty();}

    BE_SQLITE_EXPORT DbResult GetPreparedStatement(CachedStatementPtr&, DbFile const& dbFile, Utf8CP sqlString) const;
    BE_SQLITE_EXPORT void Dump();
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
// @bsiclass                                                    Keith.Bentley   11/13
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
    NamedParams(Utf8CP where=NULL) {SetWhere(where);}

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
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct DbTableIterator
{
private:
    DbTableIterator& operator=(DbTableIterator const& rhs);

protected:
    mutable DbP    m_db;
    mutable CachedStatementPtr m_stmt;
    NamedParams m_params;

    explicit DbTableIterator(DbCR db) : m_db((DbP)&db) {}
    BE_SQLITE_EXPORT Utf8String MakeSqlString(Utf8CP sql, bool hasWhere=false) const;

public:
    struct Entry
    {
    protected:
        bool  m_isValid;
        StatementP m_sql;
        Entry(StatementP sql, bool isValid) {m_sql=sql; m_isValid=isValid;}
//__PUBLISH_SECTION_END__
        void Verify() const {BeAssert(NULL != m_sql->GetSqlStatementP());}
//__PUBLISH_SECTION_START__

    public:
        bool IsValid() const {return m_isValid && (NULL!=m_sql->GetSqlStatementP());}
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
//! Table for "embedding" data files within a BeSQLite::Db. Embedded files are stored as a series of chunks as blobs, and
//! optionally the can be compressed. Files can either be embedded by importing via a local file, or by supplying an in-memory buffer.
//! Once embedded, files can either be exported into a new local file, or read into an in-memory buffer.
//! Entries are identified within the DbEmbeddedFileTable by a name, which is just a case-insensitive string that must be unique.
//! <p> You can use whatever technique you want to name your embedded files. But generally it is inadvisable to use the full
//! path of a physical file, since those paths will reference disks, directories, etc. that will likely not exist on other
//! target machines or operating systems.
//! <p> Optionally, entries can have a description string and a type string for filtering.
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct DbEmbeddedFileTable
{
private:
    friend struct Db;
    DbR m_db;
    DbEmbeddedFileTable(DbR db) : m_db(db) {}

    //__PUBLISH_SECTION_END__
    BeRepositoryBasedId GetNextEmbedFileId() const;
    //__PUBLISH_SECTION_START__

public:
    //=======================================================================================
    //! An Iterator over the entries of a DbEmbeddedFileTable.
    // @bsiclass                                                    Keith.Bentley   03/12
    //=======================================================================================
    struct Iterator : DbTableIterator
    {
    public:
        explicit Iterator(DbCR db) : DbTableIterator(db) {}

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            BE_SQLITE_EXPORT Utf8CP GetNameUtf8() const;          //!< the name of this embedded file.
            BE_SQLITE_EXPORT Utf8CP GetDescriptionUtf8() const;   //!< the description of this embedded file.
            BE_SQLITE_EXPORT Utf8CP GetTypeUtf8() const;          //!< the type of this embedded file.
            BE_SQLITE_EXPORT uint64_t GetFileSize() const;           //!< the total size, in bytes, of this embedded file.
            BE_SQLITE_EXPORT uint32_t GetChunkSize() const;          //!< the chunk size used to embed this file.
            BE_SQLITE_EXPORT DateTime GetLastModified() const;     //!< the time the file was last modified.
            BE_SQLITE_EXPORT BeRepositoryBasedId GetId() const;    //!< the id of this embedded file.
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
    //! @param[out] totalSize the total size in bytes of the file. May be NULL.
    //! @param[out] chunkSize the chunk size used to embed this file. May be NULL.
    //! @param[out] descr the description of this file. May be NULL.
    //! @param[out] typeStr the type of this file. May be NULL.
    //! @param[out] lastModified the time the file was last modified. May be NULL.
    //! @return the id  of the file, if found. If there is no entry of the given name, id will be invalid.
    BE_SQLITE_EXPORT BeRepositoryBasedId QueryFile(Utf8CP name, uint64_t* totalSize = nullptr, uint32_t* chunkSize = nullptr, Utf8StringP descr = nullptr, Utf8StringP typeStr = nullptr, DateTime* lastModified = nullptr);

     //! Import a copy of an existing file from the local filesystem into this BeSQLite::Db.  ImportWithoutCompressing just makes a binary copy of the file
     //! without compressing it. Many file formats are already compressed. There is not much benefit to compressing for these formats.
    //! @param[out] stat Success or error code. May be NULL.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] localFileName the name of a physical file on the local filesystem to be imported. The import will fail if the file doesn't
    //! exist or can't be read.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be NULL.
    //! @param[in] description a string that describes this entry. May be NULL.
    //! @param[in] lastModified the time the file was last modified. May be NULL.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @return Id of the embedded file
    BE_SQLITE_EXPORT BeRepositoryBasedId ImportWithoutCompressing(DbResult* stat, Utf8CP name, Utf8CP localFileName, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr, uint32_t chunkSize = 500 * 1024);

    //! Import a copy of an existing SQLite file from the local filesystem into this BeSQLite::Db after validating the file to be imported.
    //! @param[out] stat Success or error code. May be NULL.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] localFileName the name of a physical file on the local filesystem to be imported. The import will fail if the file doesn't
    //! exist or can't be read.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be NULL.
    //! @param[in] description a string that describes this entry. May be NULL.
    //! @param[in] lastModified the time the file was last modified. May be NULL.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] supportRandomAccess if true, ignore the specified chunkSize and calculate it instead.  Generally, the default should be used.
    //! @return Id of the embedded file
    //! @remarks
    //! ImportDb may return one of these DbResult values via the stat parameter:
    //!     - BE_SQLITE_NOTADB if the input file is not an SQLite database file.
    //!     - BE_SQLITE_CORRUPT_VTAB if the input file appears to be an SQLite database file but fails some consistency check.
    //!     - BE_SQLITE_CONSTRAINT_BASE if the database already contains an entry of the same name.
    //!     - BE_SQLITE_BUSY if the file to be embedded has a pending write or is open in DefaultTxn_Immediate or DefaultTxn_Exclusive transaction mode.  This conflict can be caused by
    //!       any process on the system.
    //!     - BE_SQLITE_IOERR if ImportDbFile failed to read the proper number of bytes from the file.
    //!     - BE_SQLITE_IOERR_WRITE if a write failed.
    //!     - BE_SQLITE_ERROR if there was an SQLite error and it was not possible to provide more detail.
    BE_SQLITE_EXPORT BeRepositoryBasedId ImportDbFile (DbResult& stat, Utf8CP name, Utf8CP localFileName, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr, uint32_t chunkSize = 500 * 1024, bool supportRandomAccess = true);

    //! Import a copy of an existing file from the local filesystem into this BeSQLite::Db.
    //! @param[out] stat Success or error code. May be NULL.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] localFileName the name of a physical file on the local filesystem to be imported. The import will fail if the file doesn't
    //! exist or can't be read.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be NULL.
    //! @param[in] description a string that describes this entry. May be NULL.
    //! @param[in] lastModified the time the file was last modified. May be NULL.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] supportRandomAccess if true, ignore the specified chunkSize and calculate it instead.  Generally, the default should be used.
    //! @return Id of the embedded file
    BE_SQLITE_EXPORT BeRepositoryBasedId Import(DbResult* stat, Utf8CP name, Utf8CP localFileName, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr, uint32_t chunkSize = 500 * 1024, bool supportRandomAccess = true);

    //! @deprecated Please use the other overload instead.
    //! Import a copy of an existing file from the local file system into this BeSQLite::Db.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] localFileName the name of a physical file on the local file system to be imported. The import will fail if the file doesn't
    //! exist or can't be read.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be NULL.
    //! @param[in] description a string that describes this entry. May be NULL.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] supportRandomAccess if true, ignore the specified chunkSize and calculate it instead.  Generally, the default should be used.
    //! @return BE_SQLITE_OK if the file was successfully imported, and error status otherwise.
    DbResult Import(Utf8CP name, Utf8CP localFileName, Utf8CP typeStr, Utf8CP description = nullptr, uint32_t chunkSize = 500 * 1024, bool supportRandomAccess = true)
        {
        DbResult stat = BE_SQLITE_OK;
        Import(&stat, name, localFileName, typeStr, description, nullptr, chunkSize, supportRandomAccess);
        return stat;
        }

    //! Create a new file on the local file system with a copy of the content of an embedded file, by name and verify that the generated file is a valid SQLite database.
    //! @param[in] localFileName the name for the new file. This method will fail if the file already exists or cannot be created.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @return BE_SQLITE_OK if the file was successfully exported, and error status otherwise.
    //! @remarks When ExportDbFile returns an error other than BE_SQLITE_ERROR_FileExists it also deletes the file specified in the localFileName parameter.  If
    //! ExportDbFile creates the output file and subsequently detects an error it deletes the file before returning an error status.
    //! ExportDbFile can return any of these error status codes:
    //!     - BE_SQLITE_NOTADB if the extracted file is not an SQLite database file.
    //!     - BE_SQLITE_CORRUPT_VTAB if the extracted file appears to be an SQLite database file but fails some consistency check.
    //!     - BE_SQLITE_ERROR_FileExists if the target file exists.
    //!     - BE_SQLITE_MISMATCH if the input file does not have a proper header for a compressed imodel.
    //!     - BE_SQLITE_ERROR if there was an SQLite error and it was not possible to provide more detail.
    //!     - BE_SQLITE_FULL if it failed to write because the disk is full.
    //!     - BE_SQLITE_IOERR_WRITE if a write failed with an error other than disk full.
    //!     - BE_SQLITE_IOERR_READ, BE_SQLITE_IOERR_SHORT_READ if the extracted file is not the expected size; the DbEmbeddedFileTable specifies the expected file size.
    //!     - BE_SQLITE_ABORT if the ICompressProgressTracker provided via the progress argument requested the operation be aborted.
    BE_SQLITE_EXPORT DbResult ExportDbFile (Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress=nullptr);

    //! Create a new file on the local file system with a copy of the content of an embedded file, by name.
    //! @param[in] localFileName the name for the new file. This method will fail if the file already exists or cannot be created.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @return BE_SQLITE_OK if the file was successfully exported, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Export(Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress=nullptr);

    //! Extract and de-compress the specified embedded file to the supplied bvector.
    //! @param[in] buffer where to store the extracted bytes
    //! @param[in] name the name by which the file was embedded.
    //! @return BE_SQLITE_OK if the data was successfully extracted, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Read(bvector<Byte>& buffer, Utf8CP name);

    //! Replace the content of a previously embedded file with the content of a different file on the local file system.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] localFileName the name of the new file to be embedded. This method will fail if the file does not exist or cannot be read.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @param[in] lastModified the time the file was last modified. May be NULL.
    //! @return BE_SQLITE_OK if the new file was successfully replaced, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Replace(Utf8CP name, Utf8CP localFileName, uint32_t chunkSize=500*1024, DateTime const* lastModified = nullptr);

    //! Add a new entry into this embedded file table. This merely creates an entry and id for the file, it will have no content. This method is used to subsequently
    //! add data from an in-memory buffer via Save.
    //! @param[in] name the (case insensitive) name by which the file will be known, once it is embedded. Must be unique.
    //! @param[in] typeStr a string that identifies the kind of file this entry holds. This can be used for filtering the list of embedded files. Should not be NULL.
    //! @param[in] description a string that describes this entry. May be NULL.
    //! @param[in] lastModified the time the file was last modified. May be NULL.
    //! @return BE_SQLITE_OK if the entry was successfully added, and error status otherwise.
    BE_SQLITE_EXPORT DbResult AddEntry(Utf8CP name, Utf8CP typeStr, Utf8CP description = nullptr, DateTime const* lastModified = nullptr);

    //! Save an in-memory buffer as the data for an entry in this embedded file table. This method will replace any existing data for the entry. The entry
    //! must have been previously created, either by Import or AddEntry.
    //! @param[in] data the file data to save.
    //! @param[in] size the number of bytes in data.
    //! @param[in] name the name by which the file was embedded.
    //! @param[in] compress if true, the file will be compressed as it is imported. This makes the Db smaller, but is also slower to import and later read.
    //! @param[in] chunkSize the maximum number of bytes that are saved in a single blob to hold this file. There are many tradeoffs involved in
    //! choosing a good chunkSize, so be careful to test for optimal size. Generally, the default is fine.
    //! @return BE_SQLITE_OK if the entry was successfully saved, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Save(void const* data, uint64_t size, Utf8CP name, bool compress=true, uint32_t chunkSize=500*1024);


    //! Remove an entry from this embedded file table, by name. All of its data will be deleted from the Db.
    //! @param[in] name the name by which the file was embedded.
    //! @return BE_SQLITE_OK if the entry was successfully removed, and error status otherwise.
    BE_SQLITE_EXPORT DbResult Remove(Utf8CP name);

    //__PUBLISH_SECTION_END__
    //! Create the Embedded file table if it doesn't exist
    DbResult CreateTable() const;
    static bool HasLastModifiedColumn(DbR db);
    //__PUBLISH_SECTION_START__
};

//! SQLite Transaction modes corresponding to https://www.sqlite.org/lang_transaction.html
enum class BeSQLiteTxnMode {None=0, Deferred=1, Immediate=2, Exclusive=3,};

//! Determines whether and how the default transaction should be started when a Db is created or opened.
enum StartDefaultTransaction
    {
    //! Do not start a default transaction. This is generally not a good idea except for very specialized cases. All access to a database requires a transaction.
    //! So, unless you start a "default" transaction, you must wrap all SQL statements with a \ref Savepoint.
    DefaultTxn_No = (int) BeSQLiteTxnMode::None,
    //! Create a default "normal" transaction. SQLite will acquire locks as they are needed.
    DefaultTxn_Yes = (int) BeSQLiteTxnMode::Deferred,
    //! Create a default transaction using SQLite "immediate" mode. This acquires the "reserved" locks on the database (see http://www.sqlite.org/lang_transaction.html)
    //! when the file is opened and then attempts to reacquire them every time the default transaction is committed.
    DefaultTxn_Immediate = (int) BeSQLiteTxnMode::Immediate,
    //! Create a default transaction using SQLite "exclusive" mode. This acquires all locks on the database (see http://www.sqlite.org/lang_transaction.html)
    //! when the file is opened. The locks are never released until the database is closed. Only exclusive access guarantees that no other process will be able to
    //! gain access (and thereby block access from this connection) to the file when the default transaction is committed. Use of DefaultTxn_Exclusive requires
    //! that the database be opened for read+write access and the open will fail if you attempt to use it on a readonly connection.
    DefaultTxn_Exclusive = (int) BeSQLiteTxnMode::Exclusive
    };

//=======================================================================================
//! Savepoint encapsulates SQLite transactions against a BeSQLite::Db. Savepoint is implemented using
//! SQLite BEGIN, COMMMIT and SAVEPOINT statements, so they can nest (see http://www.sqlite.org/lang_savepoint.html for details). There is no way to
//! hold a BeSQLite transaction open without a Savepoint object. That is, no transaction survives beyond the scope of its
//! Savepoint object. However, Savepoint can be committed, and restarted, and canceled within their lifecycle.
//! <p><h2>Default Transactions</h2>
//! When you create or open a BeSQLite::Db, you can optionally specify that you wish to open a "default transaction". Every BeSQLite::Db holds a
//! Savepoint for this purpose. The default transaction Savepoint remains active for the lifecycle of the BeSQLite::Db, and can be
//! committed and canceled using the Db::SaveChanges and Db::AbandonChanges methods, respectively.
//! <p>
//! Savepoints are auto-committed on destruction. If you wish to abandon the changes made in a Savepoint, you must call
//! Savepoint::Cancel() on it before it is destroyed.
// @bsiclass                                                    Keith.Bentley   03/12
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
    //! @param[in] name the name of this Savepoint. Does not have to be unique, but must not be NULL.
    //! @param[in] beginTxn if true Begin() is called in constructor. To see if Begin was successful, check IsActive.
    //! @param[in] txnMode the default transaction mode for this Savepoint
    BE_SQLITE_EXPORT Savepoint(Db& db, Utf8CP name, bool beginTxn=true, BeSQLiteTxnMode txnMode=BeSQLiteTxnMode::Deferred);

    //! dtor for Savepoint. If the Savepoint is still active, it is committed.
    virtual ~Savepoint() {Commit(nullptr);}

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
    DbResult Save(Utf8CP operation) {Commit(operation); return Begin();}

    //! Cancel this transaction. Executes the SQLite "ROLLBACK" command if the Savepoint is active.
    BE_SQLITE_EXPORT DbResult Cancel();
};

//=======================================================================================
//! Every BeSQLite::Db has a table for storing "Properties".
//! Properties are essentially "named values" and are used to store "non-SQL" information in the database. Generally, for each type of
//! Property there are zero, one, or perhaps a small set of instances and they therefore don't warrant having their own table. Property values
//! have both a blob and a text string. Either or both may be NULL (and typically one or the other is.)
//! For example, the GUID of the database itself is stored in the Properties table.
//! <p>
//! This class forms a "specification" to save and retrieve a type of Property. The specification has three parts: a "Name" string, a "Namespace"
//! and a flag indicating how the property is to be transacted. The combination of Name and Namespace must be unique. The Namespace part
//! identifies a group of Properties that are related somehow. The Name part specifies the Property's meaning within the Namespace. When instances of properties
//! are saved or queried, they are identified by two ids (called "Id" and "SubId"), forming a 2 dimensional array of instances of a given PropertySpec.
//! <p>
//! <h2>Settings Properties</h2>
//! If a PropertySpec indicates txnMode=TXN_MODE_Setting, then changes to its values are held in memory and are not saved to the database unless/until
//! Db::SaveSettings is called. If the Db is closed without a call to Db::SaveSettings, the changes are lost. Note that QueryProperty will
//! always return the most recent value of a setting, even if it hasn't be saved persistently yet.
//! <h2>Cached Properties</h2>
//! Some properties hold values that are accessed or changed frequently. While the property system is relatively efficient, its primary goal is
//! not access speed, and does not use any memory except while accessing properties. By specifying txnMode=TXN_MODE_Cached, a property's value will
//! be held in memory as it is queried or changed. Changes are saved when the transaction is committed. This makes any such property faster to access at the expense of memory. This mode should be
//! used judiciously, and reserved for only the few properties that warrant it.
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct PropertySpec
    {
public:
    //! Determine whether a property's data may be compressed.
    enum WantCompress
    {
        //! never compress this property
        COMPRESS_PROPERTY_No=0,

        //! the property value may be compressed before it is saved. The property won't necessarily be compressed unless its size is large enough
        //! (usually 100 bytes) and the actual compression results in a net savings.
        COMPRESS_PROPERTY_Yes=1
    };
    enum ProperyTxnMode {TXN_MODE_Normal=0, TXN_MODE_Setting=1, TXN_MODE_Cached=2,};
private:
    ProperyTxnMode m_txnMode;
    WantCompress m_compress;
    bool   m_saveIfNull;
    Utf8CP m_name;
    Utf8CP m_namespace;

public:
    //! Construct an instance of a PropertySpec with values for name and namespace
    //! @param[in] name The Name part of this Property specification
    //! @param[in] nameSpace The Namespace part of this Property specification.
    //! @param[in] txnMode the transaction mode for this property.
    //! @param[in] compress If COMPRESS_PROPERTY_Yes, the property value may be compressed before it is saved. The property won't necessarily be compressed unless
    //!            its size is large enough (usually 100 bytes) and the actual compression results in a net savings.
    //! @param[in] saveIfNull If true, this property will be saved even if its value is NULL. Otherwise it is deleted if its value is NULL.
    //! @note name and namespace should always point to static strings.
    PropertySpec(Utf8CP name, Utf8CP nameSpace, ProperyTxnMode txnMode=TXN_MODE_Normal, WantCompress compress=COMPRESS_PROPERTY_Yes, bool saveIfNull=false) : m_name(name), m_namespace(nameSpace), m_compress(compress), m_txnMode(txnMode), m_saveIfNull(saveIfNull){}

    //! Copy a PropertySpec, changing only the setting flag
    PropertySpec(PropertySpec const& other, ProperyTxnMode txnMode) {*this = other; m_txnMode=txnMode;}

    Utf8CP GetName() const {return m_name;}
    Utf8CP GetNamespace() const {return m_namespace;}

    //! Determine whether this PropertySpec refers to a setting or not.
    bool IsSetting() const {return TXN_MODE_Setting == m_txnMode;}
    //! Determine whether this PropertySpec is cached or not.
    bool IsCached()  const {return TXN_MODE_Cached == m_txnMode;}
    //! Determine whether this PropertySpec saves NULL values or not.
    bool SaveIfNull() const {return m_saveIfNull;}
    //! Determine whether this PropertySpec requests to compress or not.
    bool IsCompress() const {return COMPRESS_PROPERTY_Yes==m_compress;}
    };

typedef PropertySpec const& PropertySpecCR;

//=======================================================================================
// @bsiclass                                                    John.Gooding    03/13
//=======================================================================================
struct PackageProperty
{
    struct Spec : PropertySpec
        {
        Spec(Utf8CP name) : PropertySpec(name, PROPERTY_APPNAME_Package, PropertySpec::TXN_MODE_Normal) {}
        };

    static Spec SchemaVersion()   {return Spec("SchemaVersion");}
    static Spec Name()            {return Spec("Name");}
    static Spec Description()     {return Spec("Description");}
    static Spec Client()          {return Spec("Client");}
};

//=======================================================================================
//! Standard properties that an i-model publisher program should add to the .imodel
//! and .idgndb files that it creates.
// @bsiclass                                                    Shaun.Sewall    05/13
//=======================================================================================
struct ImodelProperty
{
    struct Spec : PropertySpec
        {
        Spec(Utf8CP name) : PropertySpec(name, PROPERTY_APPNAME_Imodel, TXN_MODE_Normal) {}
        };

    static Spec PublisherProgram()  {return Spec("PublisherProgram");}
    static Spec PublisherVersion()  {return Spec("PublisherVersion");}
    static Spec ImodelType()        {return Spec("imodelType");}
};

//=======================================================================================
//! Supply a BusyRetry handler to BeSQLite (see https://www.sqlite.org/c3ref/busy_handler.html).
// @bsiclass                                                    Keith.Bentley   10/14
//=======================================================================================
struct BusyRetry : RefCountedBase
{
    //! Called when SQLite is blocked by another connection to a database. The default implementation performs 5 retries
    //! with a 1 second delay. Subclasses can change the timeout period or interact with the user, if necessary.
    //! @param[in] count the number of times this method has been called for this busy event.
    //! @return 0 to stop retrying and return a BE_SQLITE_BUSY error. Any non-zero value will attempt another retry.
    virtual int _OnBusy(int count) const {if (count>4) return 0; BeThreadUtilities::BeSleep(1000); return 1;}
};

//=======================================================================================
// Cached "repository local values"
// @bsiclass                                                    Keith.Bentley   12/12
//=======================================================================================
struct CachedRLV
    {
private:
    Utf8String   m_name;
    bool         m_isUnset;
    mutable bool m_dirty;
    int64_t      m_value;

public:
    explicit CachedRLV(Utf8CP name) : m_name(name) {BeAssert(!Utf8String::IsNullOrEmpty(name)); Reset();}
    Utf8CP GetName() const {return m_name.c_str();}
    int64_t GetValue() const {BeAssert(!m_isUnset); return m_value;}
    void ChangeValue(int64_t value, bool initializing = false) {m_isUnset=false; m_dirty=!initializing; m_value=value;}
    int64_t Increment() {BeAssert(!m_isUnset); m_dirty = true; m_value++; return m_value;}
    bool IsUnset() const { return m_isUnset; }
    bool IsDirty() const {BeAssert(!m_isUnset); return m_dirty;}
    void SetIsNotDirty() const {BeAssert(!m_isUnset); m_dirty = false;}
    void Reset() {m_isUnset=true; m_dirty=false; m_value=-1LL;}
    };

//=======================================================================================
// Cache for int64_t RepositoryLocalValues. This is for RLV's that are of type int64_t and are frequently
// accessed and/or modified. The RLVs are identified in the cache by "registering" their name are thereafter
// accessed by index. The cache is held in memory for performance. It is automatically saved whenever a transaction is committed.
// @bsiclass                                                    Krischan.Eberle     07/14
//=======================================================================================
struct RepositoryLocalValueCache : NonCopyableClass
    {
private:
    friend struct DbFile;
    friend struct Db;
    bvector<CachedRLV> m_cache;
    DbFile& m_dbFile;

    void Clear();
    bool TryQuery(CachedRLV*&, size_t rlvIndex);

public:
    RepositoryLocalValueCache(DbFile& dbFile) : m_dbFile(dbFile) {}

    //! Register a RepositoryLocalValue name with the Db.
    //! @remarks On closing the Db the registration is cleared.
    //! @param[out] rlvIndex Index for the RepositoryLocalValue used as input to the RepositoryLocalValue API.
    //! @param[in] rlvName Name of the RepositoryLocalValue. Db does not make a copy of @p rlvName, so the caller
    //! has to ensure that it remains valid for the entire lifetime of the Db object.
    //! @return BE_SQLITE_OK if registration was successful. Error code, if a RepositoryLocalValue with the same
    //! name has already been registered.
    BE_SQLITE_EXPORT DbResult Register(size_t& rlvIndex, Utf8CP rlvName);

    //! Look up the RepositoryLocalValue index for the given name
    //! @param[out] rlvIndex Found index for the RepositoryLocalValue
    //! @param[in] rlvName Name of the RepositoryLocalValue
    //! @return true, if the RepositoryLocalValue index was found, i.e. a RepositoryLocalValue is registered for @p rlvName,
    //! false otherwise.
    BE_SQLITE_EXPORT bool TryGetIndex(size_t& rlvIndex, Utf8CP rlvName);

    //! Save an RepositoryLocalValue into the BEDB_TABLE_Local table.
    //! @param[in] rlvIndex The index of the RepositoryLocalValue to query.
    //! @param[in] value Value to save
    //! @return BE_SQLITE_OK if successful, error code otherwise.
    //! @see RegisterRepositoryLocalValue
    BE_SQLITE_EXPORT DbResult SaveValue(size_t rlvIndex, int64_t value);

    //! Read a RepositoryLocalValue from BEDB_TABLE_Local table.
    //! @param[out] value Retrieved value
    //! @param[in] rlvIndex The index of the RepositoryLocalValue to query.
    //! @return BE_SQLITE_OK if the value exists, error code otherwise.
    //! @see RegisterRepositoryLocalValue
    BE_SQLITE_EXPORT DbResult QueryValue(int64_t& value, size_t rlvIndex);

    //! Increment the RepositoryLocalValue by one for the given @p rlvIndex.
    //! @param[out] newValue Incremented value
    //! @param[in] rlvIndex The index of the RepositoryLocalValue to increment.
    //! @return BE_SQLITE_OK in case of success. Error code otherwise. If initialValue is nullptr and
    //!         the RepositoryLocalValue does not exist, and error code is returned.
    //! @see RegisterRepositoryLocalValue
    BE_SQLITE_EXPORT DbResult IncrementValue(int64_t& newValue, size_t rlvIndex);
    };

//=======================================================================================
//! A physical Db file.
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct DbFile
{
    friend struct Db;
    friend struct Statement;
    friend struct Savepoint;
    friend struct RTreeAcceptFunction::Tester;

private:
    size_t  m_repositoryIdRlvIndex;

protected:
    typedef RefCountedPtr<ChangeTracker> ChangeTrackerPtr;

    bool            m_settingsTableCreated;
    bool            m_settingsDirty;
    bool            m_allowImplicitTxns;
    bool            m_inCommit;
    SqlDbP          m_sqlDb;
    uint64_t        m_dataVersion; // for detecting changes from another process
    RefCountedPtr<BusyRetry> m_retry;
    mutable void*   m_cachedProps;
    RepositoryLocalValueCache m_rlvCache;
    BeDbGuid        m_dbGuid;
    Savepoint       m_defaultTxn;
    BeRepositoryId  m_repositoryId;
    bmap<Utf8String,ChangeTrackerPtr> m_trackers;
    StatementCache  m_statements;
    typedef bvector<Savepoint*> DbTxns;
    typedef DbTxns::iterator DbTxnIter;
    DbTxns          m_txns;

    mutable RTreeAcceptFunction m_rtreeMatch;
    mutable struct
        {
        bool m_readonly:1;
        bool m_rtreeMatchValid:1;
        int  m_dummy:30;
        }  m_flags;

    explicit DbFile(SqlDbP sqlDb, BusyRetry* retry, BeSQLiteTxnMode defaultTxnMode);
    ~DbFile();
    void InitRTreeMatch() const;
    void SetRTreeMatch(RTreeAcceptFunction::Tester* tester) const;
    DbResult StartSavepoint(Savepoint&, BeSQLiteTxnMode);
    DbResult StopSavepoint(Savepoint&, bool isCommit, Utf8CP operation);
    DbResult CreatePropertyTable(Utf8CP tablename, Utf8CP ddl, bool temp);
    DbResult SaveCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId, Utf8CP stringData, void const* value, uint32_t size) const;
    struct CachedProperyMap& GetCachedPropMap() const;
    struct CachedPropertyValue& GetCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    struct CachedPropertyValue* FindCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    DbResult QueryCachedProperty(Utf8String*, void** value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const;
    void DeleteCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId);
    void DeleteCachedPropertyMap();
    void SaveCachedProperties(bool isCommit);
    Utf8CP GetLastError(DbResult* lastResult) const;
    void SaveCachedRlvs(bool isCommit);

public:
    int OnCommit();
    bool CheckImplicitTxn() const {return m_allowImplicitTxns || m_txns.size() > 0;}
    bool UseSettingsTable(PropertySpecCR spec) const;
    SqlDbP GetSqlDb() const {return m_sqlDb;}

protected:
    BE_SQLITE_EXPORT DbResult SaveProperty(PropertySpecCR spec, Utf8CP strData, void const* value, uint32_t propsize, uint64_t majorId=0, uint64_t subId=0);
    BE_SQLITE_EXPORT bool HasProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryPropertySize(uint32_t& propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryProperty(void* value, uint32_t propsize, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT DbResult QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0) const;
    BE_SQLITE_EXPORT void SaveSettings(Utf8CP changesetName);
    BE_SQLITE_EXPORT DbResult DeleteProperty(PropertySpecCR spec, uint64_t majorId=0, uint64_t subId=0);
    BE_SQLITE_EXPORT DbResult DeleteProperties(PropertySpecCR spec, uint64_t* majorId);
    BE_SQLITE_EXPORT int AddFunction(DbFunction& function) const;
    BE_SQLITE_EXPORT int AddRTreeMatchFunction(RTreeMatchFunction& function) const;
    BE_SQLITE_EXPORT int RemoveFunction(DbFunction&) const;
    BE_SQLITE_EXPORT RepositoryLocalValueCache& GetRLVCache();
};

//=======================================================================================
//! Applications subclass from this class to store in-memory data on a Db via Db::AddAppData.
// @bsiclass
//=======================================================================================
struct DbAppData
{
    struct Key : AppDataKey {};

    virtual ~DbAppData() {}

    //! This method is called whenever this object is removed from its host BeSQLite::Db. This can happen either by an explicit call to
    //! Db::DropAppData, or because the Db itself is being deleted (in which case all of its DbAppData is notified \b before the Db is deleted).
    //! @param[in] host The host Db on which this DbAppData resided.
    virtual void _OnCleanup(DbR host) = 0;
};

//=======================================================================================
//! A SQLite database. This class is a wrapper around the SQLite datatype "sqlite3"
//! @note Refer to the SQLite documentation for the details of SQLite.
//! @nosubgrouping
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Db : NonCopyableClass
{
    friend struct DbFile;
public:
    enum class Encoding {Utf8=0, Utf16=1};
    enum class PageSize : int
        {
        PAGESIZE_512 = 512,
        PAGESIZE_1K  = 1024,
        PAGESIZE_2K  = PAGESIZE_1K * 2,
        PAGESIZE_4K  = PAGESIZE_1K * 4,
        PAGESIZE_8K  = PAGESIZE_1K * 8,
        PAGESIZE_16K = PAGESIZE_1K * 16,
        PAGESIZE_32K = PAGESIZE_1K * 32,
        PAGESIZE_64K = PAGESIZE_1K * 64
        };

    //! Whether to open an BeSQLite::Db readonly or readwrite.
    enum OpenMode {OPEN_Readonly = 1<<0, OPEN_ReadWrite = 1<<1, OPEN_Create = OPEN_ReadWrite|(1<<2), OPEN_SharedCache = 1<<17, };


    //=======================================================================================
    //! Parameters for controlling aspects of the opening of a Db.
    // @bsiclass                                                    Keith.Bentley   11/10
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams
    {
        mutable OpenMode m_openMode;
        StartDefaultTransaction m_startDefaultTxn;
        mutable bool  m_skipSchemaCheck;
        bool          m_rawSQLite;
        BusyRetry*    m_busyRetry;

        BE_SQLITE_EXPORT virtual bool _ReopenForSchemaUpgrade(Db&) const;

        //! @param[in] openMode The mode for opening the database
        //! @param[in] startDefaultTxn Whether to start a default transaction on the database.
        //! @param[in] retry Supply a BusyRetry handler for the database connection. The BeSQLite::Db will hold a ref-counted-ptr to the retry object.
        //!                  The default is to not attempt retries Note, many BeSQLite applications (e.g. DgnDb) rely on a single non-shared connection
        //!                  to the database and do not permit sharing.
        BE_SQLITE_EXPORT explicit OpenParams(OpenMode openMode, StartDefaultTransaction startDefaultTxn=DefaultTxn_Yes, BusyRetry* retry=nullptr);

        //! Use the BeSQLite::Db api on a "raw" SQLite file. This allows use of the BeSQLite api on SQLite databases it did not create.
        //! When BeSQLite::Db creates a database, it adds the BEDB_TABLE_Property and BEDB_TABLE_Local tables
        //! for the Property and RepsitoryLocalValue methods. If you attempt to open a SQLite file that doesn't have those
        //! tables, BeSQLite:Db::OpenBeSQLiteDb will fail with the status BE_SQLITE_ERROR_NoPropertyTable. When the "raw sqlite" flag
        //! is on, BeSQLite::Db::OpenBeSQLiteDb will not check for the presence of those tables, and BeSQLite::Db::CreateNewDb will not
        //! create those tables.
        //! @note When this flag is on, all of the Property and RepsositoryLocalValue methods of BeSQLite::Db will fail.
        void SetRawSQLite() {m_rawSQLite=true;}

        //! Determine whether the "raw sqlite" flag is on or not
        bool IsRawSQLite() const {return m_rawSQLite;}

        //! Determine whether the open mode is readonly or not
        bool IsReadonly() const {return m_openMode==OPEN_Readonly;}

        //! set the open mode
        void SetOpenMode(OpenMode openMode) {m_openMode = openMode;}

        //! Determine whether the default transaction should be started on the Db. If DefaultTxn_Yes, the default transaction
        //! will be started on the Db after it is opened. This applies only to the connection returned by
        //! Db::CreateNewDb or Db::OpenBeSQLiteDb; it is not a persistent property of the Db.
        void SetStartDefaultTxn(StartDefaultTransaction val) {m_startDefaultTxn = val;}
    };

    //=======================================================================================
    //! Parameters for controlling aspects of creating and then opening a Db.
    // @bsiclass                                                    Keith.Bentley   11/10
    //=======================================================================================
    struct CreateParams : OpenParams
    {
        Encoding m_encoding;
        PageSize m_pagesize;
        enum CompressedDb {CompressDb_None=0,CompressDb_Zlib=1,CompressDb_Snappy=2,} m_compressedDb;
        enum ApplicationId : uint64_t {APPLICATION_ID_BeSQLiteDb='BeDb',} m_applicationId;
        bool m_failIfDbExists;
        DateTime m_expirationDate;

        //! @param[in] pagesize The pagesize for the database. Default is 4K.
        //! @param[in] encoding The text encoding mode for the database. The default is UTF-8 and is almost always the best choice.
        //! @param[in] failIfDbExists If true, return an error if a the specified file already exists, otherwise delete existing file.
        //! @param[in] defaultTxn Whether to start the default transaction against the newly created Db. @see SetStartDefaultTxn
        //! @param[in] retry Supply a BusyRetry handler for the database connection. The BeSQLite::Db will hold a ref-counted-ptr to the retry object.
        //!                  The default is to not attempt retries Note, many BeSQLite applications (e.g. DgnDb) rely on a single non-shared connection
        //!                  to the database and do not permit sharing.
        explicit CreateParams(PageSize pagesize=PageSize::PAGESIZE_4K, Encoding encoding=Encoding::Utf8, bool failIfDbExists=true,
                      StartDefaultTransaction defaultTxn=DefaultTxn_Yes, BusyRetry* retry=nullptr) : OpenParams(OPEN_Create, defaultTxn, retry)
              {m_encoding=encoding; m_pagesize=pagesize; m_compressedDb=CompressDb_None; m_failIfDbExists=failIfDbExists; m_applicationId=APPLICATION_ID_BeSQLiteDb;}

        //! Set the page size for the newly created database.
        void SetPageSize(PageSize pagesize) {m_pagesize = pagesize;}
        //! Set the text encoding for the newly created database.
        void SetEncoding(Encoding encoding) {m_encoding = encoding;}
        //! Determine whether the create should fail if a file exists with the specified name.
        void SetFailIfDbExist(bool val) {m_failIfDbExists = val;}
        //! Set the compression mode for the new Db. Default is CompressDb_None.
        void SetCompressMode(CompressedDb val) {m_compressedDb=val;}
        //! Set Application Id to be stored at offset 68 of the SQLite file
        void SetApplicationId(ApplicationId applicationId) {m_applicationId=applicationId;}
        //! Set expiration date for the newly created database.
        void SetExpirationDate(DateTime xdate) {m_expirationDate=xdate;}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   09/13
    //=======================================================================================
    struct DbAppDataList : NonCopyableClass
    {
    private:
        friend struct Db;
        typedef AppDataList<DbAppData, DbAppData::Key, DbR> T_AppDataList;
        DbR            m_db;
        T_AppDataList  m_entries;
        DbAppDataList(Db&);

    public:

        //@{
        //! Add an application data object onto this Db. After this call, the \c appData object will be notified for the events in the
        //! DbAppData interface. This is a useful technique for applications to store data that is specific to this Db, as it
        //! can be retrieved quickly through a pointer to this Db and its life cycle can be made to match the Db.
        //! @param[in] key A unique key to differentiate \c appData from all of the other DbAppData objects stored on this Db.
        //! By convention, uniqueness is enforced by using the address of a (any) static object. Since all applications
        //! share the same address space, every key will be unique. Note that this means that the value of your key will be different for
        //! every session. But that's OK, the only thing important about \c key is that it be unique.
        //! @param[in] appData The application's instance of a subclass of DbAppData to store on this Db.
        //! @return SUCCESS if \c appData was successfully added to this Db. Note that it is not legal to add or drop DbAppData
        //! to/from this Db from within any of the methods of DbAppData.
        BE_SQLITE_EXPORT StatusInt Add(DbAppData::Key const& key, DbAppData* appData);

        //! Remove a DbAppData object from this Db by its key.
        //! @param[in] key The key to find the appropriate DbAppData. See discussion of keys in #Add.
        //! @return SUCCESS if a DbAppData object with \c key was found and was dropped. ERROR if no DbAppData with \c key exists.
        //! @note After the DbAppData object is removed, its DbAppData::_OnCleanup method is called.
        BE_SQLITE_EXPORT StatusInt Drop(DbAppData::Key const& key);

        //! Search for the DbAppData on this Db with \c key. See discussion of keys in #Add.
        //! @return A pointer to the DbAppData object with \c key. NULL if not found.
        BE_SQLITE_EXPORT DbAppData* Find(DbAppData::Key const& key) const;
    };

private:

protected:
    DbFile* m_dbFile;
    mutable StatementCache* m_statements;
    DbEmbeddedFileTable m_embeddedFiles;
    DbAppDataList m_appData;

    //! Gets called at the end of creating a new Db.
    //! Override to perform additional processing when Db is created
    //! @param[in] params - Create parameter used to create the Db.
    //! @return Code indicating success or error
    BE_SQLITE_EXPORT virtual DbResult _OnDbCreated(CreateParams const& params);
    virtual void _OnSaveSettings() {}                           //!< override to perform additional processing on save settings
    BE_SQLITE_EXPORT virtual DbResult _OnDbOpened();            //!< override to perform additional processing when Db is opened
    virtual void _OnDbClose() {}

    //! Called when a new transaction is started on a connection, and a different connection (either in the same process or from another process)
    //! has changed the database since the last transaction from this connection was committed. This give subclasses an opportunity to clear internal
    //! caches or user interface that may now be invalid.
    //! @note This method is only relevant for connections opened with DefaultTxn_No, since when a default transaction is active, other
    //! processes are blocked from making changes to the database.
    //! @note implementers should always forward this call to their superclass (e.g. T_Super::_OnDbChangedByOtherConnection();)
    BE_SQLITE_EXPORT virtual void _OnDbChangedByOtherConnection();

    virtual DbResult _OnRepositoryIdChanged(BeRepositoryId newRepositoryId) {return BE_SQLITE_OK;}    //!< override to perform additional processing when repository id is changed

    //! Gets called when a Db is opened and checks whether the file can be opened, i.e
    //! whether the file version matches what the opening API expects
    //!
    //! Subclasses have to override this method to perform the profile version check when the file is opened.
    //! See Db::OpenBeSQLiteDb for the <b>backwards compatibility rules</b> that subclasses have to implement
    //! in this method.
    //!
    //! ####Developer Checklist
    //! When a profile needs to be modified,
    //! 1. the profile version needs to be incremented according to the
    //! <b>backwards compatibility rules</b> mentioned above. This means concretely:
    //!     - If older versions of the software can still work with the file without any restrictions:
    //!       @b increment the @ref SchemaVersion::GetSub2 "Sub2" version digit.
    //!     - If older versions of the software can only open the file in read-only mode:
    //!       @b increment the @ref SchemaVersion::GetSub1 "Sub1" version digit.
    //!     - If older versions of the software can no longer work with the file:
    //!       @b increment either the @ref SchemaVersion::GetMajor "Major" or the @ref SchemaVersion::GetMinor "Minor"
    //!       version digit.
    //! 2. a new auto-upgrade step needs to be implemented. It auto-upgrades a file from the version prior to your change to
    //!    the new one.
    //!     - If it is not possible to implement auto-upgrade for this profile modification, set the <b>Minimum auto-upgrade
    //!       profile version</b> to the version prior to your change.
    //!
    //! @param[in] params Open parameters
    //! @return Code indicating success or error.
    virtual DbResult _VerifySchemaVersion(OpenParams const& params) {return BE_SQLITE_OK;}

    virtual int _OnAddFunction(DbFunction& func) const { return 0; }
    virtual void _OnRemoveFunction(DbFunction& func) const {}

    friend struct Statement;
    friend struct Savepoint;
    friend struct RTreeAcceptFunction::Tester;
    friend struct BeSQLiteProfileManager;
    DbResult QueryDbIds();
    DbResult SaveBeDbGuid();
    DbResult SaveRepositoryId();

private:
    void DoCloseDb();
    DbResult DoOpenDb(Utf8CP dbName, OpenParams const& params);

    //! Empties a table.
    //! @param[in] tableName Name of the table to empty.
    //! @return BE_SQLITE_OK if successful, respective error code otherwise.
    DbResult TruncateTable(Utf8CP tableName);

    //! Deletes all repository local values.
    //! @return BE_SQLITE_OK if successful, respective error code otherwise.
    DbResult DeleteRepositoryLocalValues();

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
    BE_SQLITE_EXPORT void SetAllowImplictTransactions(bool val);

    //! Get the StatementCache for this Db. The StatmentCache for a Db is not allocated until the first call to this method.
    BE_SQLITE_EXPORT StatementCache& GetStatementCache() const;

    //! Get a CachedStatement for this Db. If the SQL string has already been prepared and a CachedStatement exists for it in the cache, it will
    //! be Reset (see Statement::Reset) and returned without the need to re-Prepare it. If, however, the SQL string has not been used before (or maybe just
    //! not recently) then a new CachedStatement will be created and Prepared.
    //! @param[out] statement The CachedStatement for the SQL string.
    //! @param[in] sql The SQL string from which to Prepare the CachedStatement.
    //! @return The result of either Statement::Reset (in the case where we reuse an existing CachedStatement) or Statement::Prepare (in the case
    //! where the SQL string has not yet been prepared).
    BE_SQLITE_EXPORT DbResult GetCachedStatement(CachedStatementPtr& statement, Utf8CP sql) const;

    CachedStatementPtr GetCachedStatement(Utf8CP sql) const {CachedStatementPtr stmt; GetCachedStatement(stmt, sql); return stmt;}

    //! Get an entry in the Db's Savepoint stack.
    //! @param[in] depth The depth of the Savepoint of interest. Must be 0 <= depth < GetCurrentSavepointDepth()
    //! @return A pointer to the Savepoint if depth is valid. NULL otherwise.
    BE_SQLITE_EXPORT Savepoint* GetSavepoint(int32_t depth) const;

    //! Get the current number of entries in this Db's Savepoint stack.
    BE_SQLITE_EXPORT int32_t GetCurrentSavepointDepth() const;

    //! Determine whether there is an active transaction against this Db.
    bool IsTransactionActive() const {return 0 < GetCurrentSavepointDepth();}

    //! Open an existing BeSQLite::Db file.
    //!
    //! ### Backwards Compatibility Contract
    //! When opening a file, %BeSQLite performs a version check on the file to see whether the current version of the software
    //! can open it or not. If it cannot be opened, a respective error code is returned from this method.
    //!
    //! The version check is performed for @b every %Bentley SQLite profile the file contains. Only if all profiles
    //! in the file succeed the test, the file is eventually opened. Example profiles are the @b %DgnDb or the @b %ECDb profile
    //! (see also @ref ECDbFile "ECDb versus DgnDb profile").
    //!
    //! #### Auto-upgrade of older files
    //! Each profile defines a version threshold, the so-called <b>minimum auto-upgradeable profile version</b>, up to which the
    //! file's profile is being auto-upgraded when opening it.
    //!
    //! For the auto-upgrade the software attempts to re-open the file in read-write mode, if it wasn't already.
    //! If it was unable to obtain a write lock on the file, the auto-upgrade fails.
    //!
    //! @note After a file was auto-upgraded you might not be able to use it with older versions of the software. So you
    //! should keep a copy of the file, if you need to use the file with older versions.
    //!
    //! #### Compatibility Rules
    //! For a <b>given profile</b> the compatibility rules are as follows.
    //!
    //! If a client attempts to open a file with a profile
    //! - @b older than the <b>minimum auto-upgradeable profile version</b>, the file cannot be opened and
    //! ::BE_SQLITE_ERROR_ProfileTooOld is returned
    //! - @b newer than the <b>minimum auto-upgradeable profile version</b> but @b older than the <b>profile version
    //! expected by the software</b>, the software attempts to <b>auto-upgrade</b> the file's profile.<br>
    //! ::BE_SQLITE_OK is returned if the auto-upgrade succeeded, ::BE_SQLITE_ERROR_ProfileUpgradeFailed is returned
    //! if the auto-upgrade failed.<br>
    //! If no auto-upgrade exists for the profile and the profile's @ref SchemaVersion::GetMajor "Major" and
    //! @ref SchemaVersion::GetMinor "Minor" version digits are
    //!     - equal to the expected profile version, the profile is compatible without restriction
    //!     - less than the expected profile version, the file cannot be opened and ::BE_SQLITE_ERROR_ProfileTooOld is returned.
    //! - @b equal to the version expected by the software, the profile is obviously compatible.
    //! - @b newer than the version expected by the software and
    //!     - if file profile's @ref SchemaVersion::GetSub2 "Sub2" version digit is @b newer than expected, the profile is compatible
    //!     without restriction
    //!     - if file profile's @ref SchemaVersion::GetSub1 "Sub1" version digit is @b newer than expected, file can only be
    //!     opened read-only. Therefore ::BE_SQLITE_OK is returned if client requested file to be opened read-only.
    //!     ::BE_SQLITE_ERROR_ProfileTooNewForReadWrite is returned otherwise.
    //!     - if file profile's @ref SchemaVersion::GetMajor "Major" or @ref SchemaVersion::GetMinor "Minor" version digit are
    //!     @b newer, the file cannot be opened and ::BE_SQLITE_ERROR_ProfileTooNew is returned.
    //!
    //! @note A Db can have an expiration date and time. See #IsExpired
    //!
    //! @param[in] dbName The name of the BeSQLite::Db database file. Must not be NULL.
    //! @param[in] openParams the parameters that determine aspects of the opened database file.
    //! @return ::BE_SQLITE_OK if the database was successfully opened, or error code as explained above otherwise.
    BE_SQLITE_EXPORT DbResult OpenBeSQLiteDb(Utf8CP dbName, OpenParams const& openParams);

    //! @copydoc Db::OpenBeSQLiteDb(Utf8CP, OpenParams const&)
    DbResult OpenBeSQLiteDb(BeFileNameCR dbName, OpenParams const& openParams) {return OpenBeSQLiteDb(dbName.GetNameUtf8().c_str(),openParams);}

    //! Create a new BeSQLite::Db database file, or upgrade an existing SQLite file to be a BeSQLite::Db.
    //! This will open or create the physical file, and then create the default BeSQLite::Db tables and properties.
    //! @param[in] dbName The file name for the new database. If dbName is NULL, a temporary database is created that is automatically
    //!  deleted when it is closed. If dbName is the value defined by BEDB_MemoryDb, an in-memory database is created.
    //!  If dbName is the name of an existing SQLite file, and if params.m_failIfDbExists is false (which is *not* the default),
    //!  it will open that file and add the BeSQLite::Db tables. If the file already contains the BeSQLite::Db tables, this method
    //!  will fail.
    //! @param[in] dbGuid The guid for the new database. The guid will be saved persistently in the new database.
    //! If the guid is invalid (e.g. by passing "BeGuid()" for dbGuid), a new guid is created by this method.
    //! @param[in] params Parameters about how the database should be created.
    //! @return BE_SQLITE_OK if the database was successfully created, error code otherwise.
    //! @note If the database file exists before this call, its page size and encoding are not changed.
    BE_SQLITE_EXPORT DbResult CreateNewDb(Utf8CP dbName, BeDbGuid dbGuid=BeDbGuid(), CreateParams const& params=CreateParams());

    DbResult CreateNewDb(BeFileNameCR dbName, BeDbGuid dbGuid=BeDbGuid(), CreateParams const& params=CreateParams())
                        {return CreateNewDb(dbName.GetNameUtf8().c_str(), dbGuid, params);}

    //! Determine whether this Db refers to a currently opened file.
    BE_SQLITE_EXPORT bool IsDbOpen() const;

    //! Close this Db. All Statements against this Db should be deleted before calling this method. The StatementCache and its contents are deleted
    //! automatically. Since more than one Db may point to the same DbFile, this method may or may not result in the actual file being closed. It
    //! therefore returns no status.
    BE_SQLITE_EXPORT void CloseDb();

    //! @return The name of the physical file associated with this Db. NULL if Db is not opened.
    BE_SQLITE_EXPORT Utf8CP GetDbFileName() const;

    //! Save a BeProjectGuid for this Db.
    BE_SQLITE_EXPORT void SaveProjectGuid(BeGuid);

    //! Query the BeProjectGuid for this Db.
    //! @see SaveMyProjectGuid
    BE_SQLITE_EXPORT BeGuid QueryProjectGuid() const;

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
    BE_SQLITE_EXPORT DbResult AttachDb(Utf8CP filename, Utf8CP alias);

    //! Detach a previously attached database. This method is necessary for the same reason AttachDb is necessary.
    //! @param[in] alias The alias by which the database was attached.
    BE_SQLITE_EXPORT DbResult DetachDb(Utf8CP alias);

    //! Execute a single SQL statement on this Db.
    //! This merely binds, steps, and finalizes the statement. It is no more efficient than performing those steps individually,
    //! but is more convenient.
    //! This method also enforces logs the statement and error if the statement fails. Use TryExecuteSql if you consider failures to be non-exceptional.
    //! @see sqlite3_exec
    BE_SQLITE_EXPORT DbResult ExecuteSql(Utf8CP sql, int(*callback)(void*,int,char**,char**)=0, void* arg=0, char** errmsg=0);

    //! Identical to ExecuteSql, but does not log errors nor check implicit transactions
    BE_SQLITE_EXPORT DbResult TryExecuteSql(Utf8CP sql, int (*callback)(void*,int,char**,char**)=0, void* arg=0, char** errmsg=0);

    //! return a string that describes the query plan for the specified SQL, or an error message if the SQL is invalid
    BE_SQLITE_EXPORT Utf8String ExplainQuery(Utf8CP sql, bool plan=true);

    //! Create a new table in this Db.
    //! @param[in] tableName The name for the new table.
    //! @param[in] ddl The column definition sql for this table (should not include parentheses).
    BE_SQLITE_EXPORT DbResult CreateTable(Utf8CP tableName, Utf8CP ddl);

    //! Drop a table from this Db.
    BE_SQLITE_EXPORT DbResult DropTable(Utf8CP tableName);

    //! Determine whether a table exists in this Db.
    BE_SQLITE_EXPORT bool TableExists(Utf8CP tableName) const;

    //! Determine whether a column exists in a table in this Db.
    BE_SQLITE_EXPORT bool ColumnExists(Utf8CP tableName, Utf8CP columnName) const;

    //! Get list of columns in the db table
    //! @param[out] columns contains list of columns in table.
    //! @param[in] tableName Name of the table that column names need to be returned
    BE_SQLITE_EXPORT bool GetColumns(bvector<Utf8String>& columns, Utf8CP tableName) const;

    //! Rename existing table
    //! @param[in] tableName The name of existing table.
    //! @param[in] newTableName new name for the table.
    BE_SQLITE_EXPORT bool RenameTable(Utf8CP tableName, Utf8CP newTableName);

    //! Free non-essential memory associated with the DB. Typically used when going to the background state on a tablet computer.
    BE_SQLITE_EXPORT void FlushPageCache();

    //! @name Db Properties
    //! @{
    //! Save a property value in this Db.
    //! @param[in] spec The PropertySpec of the property to save.
    //! @param[in] value A pointer to a buffer that holds the new value of the property to be saved. May be NULL
    //! @param[in] propsize The number of bytes in value (ignored if value is NULL).
    //! @param[in] majorId The major id of the property.
    //! @param[in] subId The subId of the property. The combination of majorId/subId forms a 2 dimensional array of properties of a given PropertySpec,
    //! @return BE_SQLITE_OK if the property was successfully saved, error code otherwise.
    DbResult SaveProperty(PropertySpecCR spec, void const* value, uint32_t propsize, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->SaveProperty(spec, NULL, value, propsize, majorId, subId);}

    //! Save a (Utf8) text string as a property value in this Db.
    //! @param[in] spec The PropertySpec of the property to save.
    //! @param[in] value The Utf8 sString whose value is to be saved as a property.
    //! @param[in] majorId The majorId of the property.
    //! @param[in] subId The subId of the property. The combination of majorId/subId forms a 2 dimensional array of properties of a given PropertySpec,
    //! @return BE_SQLITE_OK if the property was successfully saved, error code otherwise.
    DbResult SavePropertyString(PropertySpecCR spec, Utf8CP value, uint64_t majorId=0, uint64_t subId=0) {return m_dbFile->SaveProperty(spec, value, NULL, 0, majorId, subId);}

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
    //! @param[out] propsize The number of bytes in value.
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
    //! @param[in] majorId A pointer to the majorId of the properties to delete. If NULL, all properties of spec are deleted. If non-NULL, all properties of spec with the supplied majorid are deleted.
    //! @return BE_SQLITE_DONE if the property was either deleted or did not exist, error code otherwise.
    DbResult DeleteProperties(PropertySpecCR spec, uint64_t* majorId) {return m_dbFile->DeleteProperties(spec, majorId);}

    //! Make any previous changes to settings properties part of the current transaction. They will be written to disk if/when the transaction is committed.
    //! Unless this method is called after changes are made to setting properties, the changes are held in memory only and not saved to disk.
    BE_SQLITE_EXPORT void SaveSettings(Utf8CP changesetName=nullptr);
    //! @}

    //! @name RepositoryLocalValue
    //! @{
    //! Repository Local Values are used for information specific to this copy of the Db. The table BEDB_Table_Local in which
    //! RepositoryLocalValues are stored is not change tracked or change merged.

    //! Get a reference to the RepositoryLocalValueCache for this Db.
    RepositoryLocalValueCache& GetRLVCache() {return m_dbFile->GetRLVCache();}

    //! Query the current value of a Repository Local Value of type string.
    //! @param[in] name The name of the RLV.
    //! @param[out] value On success, the value of the RLV.
    //! @return BE_SQLITE_ROW if the value exists and value is valid, error status otherwise.
    BE_SQLITE_EXPORT DbResult QueryRepositoryLocalValue(Utf8CP name, Utf8StringR value) const;

    //! Save a new value of a Repository Local Value of type string. If the RLC already exists, its value is replaced.
    //! @param[in] name The name of the RLV.
    //! @param[in] value The new value for RLV name.
    //! @return BE_SQLITE_DONE if the value was saved, error status otherwise.
    BE_SQLITE_EXPORT DbResult SaveRepositoryLocalValue(Utf8CP name, Utf8StringCR value);
    //! @}

    //! @return the rowid from the last insert statement for this Db.
    //! @see sqlite3_last_insert_rowid
    BE_SQLITE_EXPORT int64_t GetLastInsertRowId() const;

    //! @return the number of rows modified by the last statement for this Db.
    //! @see sqlite3_changes
    BE_SQLITE_EXPORT int GetModifiedRowCount() const;

    //! @return The last error message for this Db.
    //! @param[out] lastResult The last error code for this Db.
    //! @see sqlite3_errmsg, sqlite3_errcode
    BE_SQLITE_EXPORT Utf8CP GetLastError(DbResult* lastResult = NULL) const;

    //! Commit the outermost transaction, writing changes to the file. Then, restart the transaction.
    //! @param[in] changesetName The name of the operation that generated these changes. If transaction tracking is enabled, 
    //! this will be saved with the changeset and reported as the operation that is "undone/redone" if this changeset is reversed or reinstated.
    //! @note, this will commit any nested transactions.
    BE_SQLITE_EXPORT DbResult SaveChanges(Utf8CP changesetName=nullptr);

    //! Abandon (cancel) the outermost transaction, discarding all changes since last save. Then, restart the transaction.
    //! @note, this will cancel any nested transactions.
    BE_SQLITE_EXPORT DbResult AbandonChanges();

    //! Get the raw SqlDbP for this Db. This is only necessary to call sqlite3_xx functions directly.
    BE_SQLITE_EXPORT SqlDbP GetSqlDb() const;

    //! Get the GUID of this Db.
    BE_SQLITE_EXPORT BeDbGuid GetDbGuid() const;

    //! Get the (local) BeRepositoryId of this Db. Every copy of the Db must have a unique BeRepositoryId.
    BE_SQLITE_EXPORT BeRepositoryId GetRepositoryId() const;

    //! Get the next available (unused) value for a BeRepositoryBasedId for the supplied Table/Column.
    //! @param [in,out] value the next available value of the BeRepositoryBasedId
    //! @param [in] tableName the name of the table holding the BeRepositoryBasedId
    //! @param [in] columnName the name of the column holding the BeRepositoryBasedId
    //! @param [in] whereParam optional addtional where criteria. Supply both the additional where clause (do not include the WHERE keyword) and any
    //! parameters to bind.
    BE_SQLITE_EXPORT DbResult GetNextRepositoryBasedId(BeRepositoryBasedId& value, Utf8CP tableName, Utf8CP columnName, NamedParams* whereParam=NULL);

    //! Determine whether this Db was opened readonly.
    BE_SQLITE_EXPORT bool IsReadonly() const;

    //! Get the DbEmbeddedFileTable for this Db.
    BE_SQLITE_EXPORT DbEmbeddedFileTable& EmbeddedFiles();

    BE_SQLITE_EXPORT DbAppDataList& AppData() const;           //!< The DbAppData attached to this Db.

    //! Dump statement results to stdout (for debugging purposes, only, e.g. to examine data in a temp table)
    BE_SQLITE_EXPORT void DumpSqlResults(Utf8CP sql);

    //! Return the DbResult as a string. This is useful for debugging SQL problems.
    BE_SQLITE_EXPORT static Utf8CP InterpretDbResult(DbResult result);

    //! Set one of the internal SQLite limits for this database. See documentation at sqlite3_limit for argument details.
    BE_SQLITE_EXPORT int SetLimit(int id, int newVal);

    //! Add a DbFunction to this Db for use in SQL. See sqlite3_create_function for return values. The DbFunction object must remain valid
    //! while this Db is valid, or until it is removed via #RemoveFunction.
    BE_SQLITE_EXPORT int AddFunction(DbFunction& func) const;

    //! Remove a previously added DbFunction from this Db. See sqlite3_create_function for return values.
    BE_SQLITE_EXPORT int RemoveFunction(DbFunction& func) const;

    BE_SQLITE_EXPORT int AddRTreeMatchFunction(RTreeMatchFunction& func) const;

    BE_SQLITE_EXPORT void ChangeDbGuid(BeDbGuid);
    BE_SQLITE_EXPORT DbResult ChangeRepositoryId(BeRepositoryId);

//__PUBLISH_SECTION_END__
    BE_SQLITE_EXPORT DbResult AddChangeTracker(ChangeTracker&);
    BE_SQLITE_EXPORT DbResult DropChangeTracker(Utf8CP name);
    BE_SQLITE_EXPORT ChangeTracker* FindChangeTracker(Utf8CP name);
    Savepoint* GetDefaultTransaction() const {return (m_dbFile != nullptr) ? &m_dbFile->m_defaultTxn : nullptr;}

    //! Checks a file's profile compatibility to be opened with the current version of the profile's API.
    //!
    //! @see Db::OpenBeSQLiteDb for the compatibility contract for Bentley SQLite profiles.
    //! @param[out] fileIsAutoUpgradable Returns true if the file's profile version indicates that it is old, but auto-upgradeable.
    //!             false otherwise.
    //!             This method does @b not perform auto-upgrades. The out parameter just indicates to calling code
    //!             whether it has to perform the auto-upgrade or not.
    //! @param[in]  expectedProfileVersion Profile version expected by the API that tries to open the file.
    //! @param[in]  actualProfileVersion Profile version of the file to be opened
    //! @param[in]  minimumUpgradableProfileVersion Minimum profile version of the file for which the API can auto-upgrade
    //!             the file to the latest version. The version's Sub1 and Sub2 digits must be 0.
    //! @param[in]  openModeIsReadonly true if the file is or is going to be opened in read-only mode. false if
    //!             the file is or is going to be opened in read-write mode.
    //! @param[in]  profileName Name of the profile for logging purposes.
    //! @return     BE_SQLITE_OK if profile can be opened in the requested mode, i.e. the compatibility contract is matched.
    //!             BE_SQLITE_Error_ProfileTooOld if file's profile is too old to be opened by this API.
    //!             This error code is also returned if the file is old but not too old to be auto-upgraded.
    //!             Check @p fileIsAutoUpgradable to tell whether the file is auto-upgradeable and not.
    //!             BE_SQLITE_Error_ProfileTooNew if file's profile is too new to be opened by this API.
    //!             BE_SQLITE_Error_ProfileTooNewForReadWrite if file's profile is too new to be opened read-write, i.e. @p openModeIsReadonly is false
    BE_SQLITE_EXPORT static DbResult CheckProfileVersion(bool& fileIsAutoUpgradable, SchemaVersion const& expectedProfileVersion, SchemaVersion const& actualProfileVersion, SchemaVersion const& minimumUpgradableProfileVersion, bool openModeIsReadonly, Utf8CP profileName);

    //! Upgrades this file's BeSQLite profile.
    //! @remarks Unlike the other profiles a BeSQLite profile is never upgraded implicitly. ECDb calls this as it needs
    //! to make sure the profile is up-to-date.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    BE_SQLITE_EXPORT DbResult UpgradeBeSQLiteProfile();
//__PUBLISH_SECTION_START__

    //! Check if the Db is at or beyond its expiration date.
    //! @see QueryExpirationDate, CreateParams::SetExpirationDate
    BE_SQLITE_EXPORT bool IsExpired() const;

    //! Query the ExpirationDate property of this database.
    //! @param[out] xdate   The expiration date, if any.
    //! @return BE_SQLITE_ROW if the ExpirationDate property was successfully found, BE_SQLITE_NOTFOUND if this database does not have an expiration data, or BE_SQLITE_ERROR if the expiration date could not be read.
    //! @see SaveExpirationDate, IsExpired
    BE_SQLITE_EXPORT DbResult QueryExpirationDate (DateTime& xdate) const;

    //! Saves the specified date as the ExpirationDate property of the database.
    //! @param[in] xdate   The expiration date. <em>Must be UTC.</em>
    //! @return BE_SQLITE_OK if property was successfully saved, or a non-zero error status if the expiration date is invalid, is not UTC, or could not be saved to the database.
    //! @see QueryExpirationDate, IsExpired
    BE_SQLITE_EXPORT DbResult SaveExpirationDate (DateTime const& xdate);
};

//=======================================================================================
//! Creating an instance of HighPriorityOperationSequencer notifies HighPriorityOperationSequencer
//! that the subsequent operations are high priority and should be allowed even if
//! high priority is required. It also notifies the range tree operation that it should
//! abort the current range tree query to let the high priority query to continue.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct HighPriorityOperationBlock
    {
    BE_SQLITE_EXPORT HighPriorityOperationBlock();
    BE_SQLITE_EXPORT ~HighPriorityOperationBlock();
    };

//=======================================================================================
//! Creating an instance of RangeTreeOperationBlock notifies the HighPriorityOperationSequencer
//! that the current thread is doing a range tree operation.  That means that operations
//! in this thread are allowed even if they are not high priority. Code that executes
//! within the scope of a RangeTreeOperationBlock must check for IsHighPriorityOperationActive
//! and, if it is, must respond appropriately.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct RangeTreeOperationBlock
    {
    BE_SQLITE_EXPORT RangeTreeOperationBlock(BeSQLite::Db const& sqlDb);
    BE_SQLITE_EXPORT ~RangeTreeOperationBlock();
    };

//=======================================================================================
//! Creating an instance of HighPriorityRequired notifies the HighPriorityOperationSequencer
//! that the current thread is doing an operation that prohibits all but high priority operations.
//! In a release build this has no affect. In a debug build it allows CheckSQLiteOperationAllowed
//! to assert if it detects a BeSQLite operation that is not high priority.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct HighPriorityRequired
    {
    BE_SQLITE_EXPORT HighPriorityRequired();
    BE_SQLITE_EXPORT ~HighPriorityRequired();
    };

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct HighPriorityOperationSequencer
{
friend struct HighPriorityOperationBlock;
friend struct RangeTreeOperationBlock;
friend struct HighPriorityRequired;

private:
    static bool s_isHighPriorityRequired;
    static int  s_inHighPriorityOperation;
    static intptr_t s_rangeTreeThreadId;
    static SqlDbP s_queryDb;

    static void StartRangeTreeOperation(BeSQLite::Db const& queryDb);
    static void EndRangeTreeOperation();
    static void StartHighPriorityOperation();
    static void EndHighPriorityOperation();
    BE_SQLITE_EXPORT static void StartHighPriorityRequired();
    BE_SQLITE_EXPORT static void EndHighPriorityRequired();

public:
    //! This is called at the start of an update that must not block.  Calls to this should
    //! never be nested.
    BE_SQLITE_EXPORT static bool IsHighPriorityOperationActive();
    static void CheckSQLiteOperationAllowed(SqlDbP queryDb);
};

//=======================================================================================
//! Error values returned from the ZLib functions. See ZLib documentation for details.
// @bsiclass                                                    Keith.Bentley   11/10
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
// @bsiclass                                                    Keith.Bentley   08/11
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
    BE_SQLITE_EXPORT void Write(ByteCP data, uint32_t size);

    //! Finish the compression. After this call, no additional data may be written to this blob until #Init is called.
    BE_SQLITE_EXPORT void Finish();

    void DoSnappy(ByteCP data, uint32_t size) {Init(); Write(data,size); Finish();}

    //! Save this compressed value as a blob in a Db.
    //! @param[in] db the SQLite database to write
    //! @param[in] tableName the name of the table to write
    //! @param[in] column the column to write
    //! @param[in] rowId the rowId to write
    //! @note The cell in the db at tableName[column,rowId] must be an existing blob of #GetCompressedSize bytes. This method cannot be used to
    //! change the size of a blob.
    //! @see sqlite3_blob_open, sqlite3_blob_write, sqlite3_blob_close
    BE_SQLITE_EXPORT BentleyStatus SaveToRow(DbR db, Utf8CP tableName, Utf8CP column, int64_t rowId);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/12
//=======================================================================================
struct SnappyReader
{
    virtual ~SnappyReader() {}
    virtual ZipErrors _Read(Byte* data, uint32_t size, uint32_t& actuallyRead) = 0;
};

//=======================================================================================
//! Utility to read Snappy-compressed data from memory, typically from an image of a blob.
// @bsiclass                                                    John.Gooding    05/12
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
    BE_SQLITE_EXPORT static uint32_t GetUncompressedBufferSize();
    BE_SQLITE_EXPORT SnappyFromMemory(void* uncompressedBuffer, uint32_t uncompressedBufferSize);
    BE_SQLITE_EXPORT void Init(void* blobBuffer, uint32_t blobBufferSize);
    BE_SQLITE_EXPORT virtual ZipErrors _Read(Byte* data, uint32_t size, uint32_t& actuallyRead) override;
};

//=======================================================================================
//! Utility to read Snappy-compressed data from a blob in a database.
// @bsiclass                                                    Keith.Bentley   12/10
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
};

//=======================================================================================
//! Defines a callback for providing information on the progress of a compress or
//! decompress operation.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct ICompressProgressTracker
{
    //! @param[in] inSize size of the source file
    //! @param[in] outSize size of the output; -1 if no valid information is available.
    //! Returning anything other than BSISUCCESS causes the operation to abort.
    virtual StatusInt _Progress(uint64_t inSize, int64_t outSize) = 0;
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct ILzmaOutputStream
{
    virtual ZipErrors _Write(void const* data, uint32_t size, uint32_t&bytesWritten) = 0;
    virtual void _SetAlwaysFlush(bool flushOnEveryWrite) = 0;
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct ILzmaInputStream
{
    virtual ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) = 0;
    virtual uint64_t _GetSize() = 0;
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct BeFileLzmaInStream : ILzmaInputStream
{
private:
    BeFile         m_file;
    uint64_t       m_fileSize;
    uint64_t       m_bytesRead;

public:
    virtual ~BeFileLzmaInStream() {}
    BE_SQLITE_EXPORT StatusInt OpenInputFile(BeFileNameCR fileName);
    BE_SQLITE_EXPORT ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override;
    uint64_t _GetSize() override { return m_fileSize; }
    uint64_t GetBytesRead() { return m_bytesRead; }
    BeFile& GetBeFile() { return m_file; }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct BeFileLzmaInFromMemory : ILzmaInputStream
{
private:
    BeDbMutex       m_mutex;
    void const*     m_data;
    uint32_t        m_size;
    uint32_t        m_offset;

public:
    BE_SQLITE_EXPORT BeFileLzmaInFromMemory(void const*data, uint32_t size);
    BE_SQLITE_EXPORT ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override;
    uint64_t _GetSize() override { return m_size; }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct BeFileLzmaOutStream : ILzmaOutputStream
{
private:
    BeFile      m_file;
    uint64_t    m_bytesWritten;

public:
    virtual ~BeFileLzmaOutStream() {}
    BE_SQLITE_EXPORT BeFileStatus CreateOutputFile(BeFileNameCR fileName, bool createAlways = true);
    BE_SQLITE_EXPORT ZipErrors _Write(void const* data, uint32_t size, uint32_t& bytesWritten) override;
    BE_SQLITE_EXPORT void _SetAlwaysFlush(bool flushOnEveryWrite) override;
    uint64_t GetBytesWritten() { return m_bytesWritten; }
    BeFile& GetBeFile() { return m_file; }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct LzmaOutToBvectorStream : ILzmaOutputStream
{
private:
    bvector<Byte>& m_buffer;

public:
    LzmaOutToBvectorStream(bvector<Byte>& v) : m_buffer(v) {}
    virtual ~LzmaOutToBvectorStream() {}
    void Reserve(size_t min);
    BE_SQLITE_EXPORT ZipErrors _Write(void const* data, uint32_t size, uint32_t& bytesWritten) override;
    BE_SQLITE_EXPORT void _SetAlwaysFlush(bool flushOnEveryWrite) override;
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/13
//=======================================================================================
struct LzmaEncoder
    {
private:
    CLzma2EncProps* m_enc2Props;

public:

    void SetBlockSize(uint32_t blockSize);
    BE_SQLITE_EXPORT LzmaEncoder(uint32_t dictionarySize);
    BE_SQLITE_EXPORT ~LzmaEncoder();
    BE_SQLITE_EXPORT ZipErrors CompressDgnDb(Utf8CP targetFile, Utf8CP sourceFile, ICompressProgressTracker* progress, bool supportRandomAccess);
    BE_SQLITE_EXPORT ZipErrors CompressDgnDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progress, bool supportRandomAccess);
    BE_SQLITE_EXPORT ZipErrors Compress(ILzmaOutputStream& out, ILzmaInputStream& in, ICompressProgressTracker* progress, bool supportRandomAccess);
    BE_SQLITE_EXPORT ZipErrors Compress(bvector<Byte>& out, void const *input, uint32_t sizeInput, ICompressProgressTracker* progress, bool supportRandomAccess);
    };

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/13
//=======================================================================================
struct LzmaDecoder
    {
    BE_SQLITE_EXPORT ZipErrors UncompressDgnDb(Utf8CP targetFile, Utf8CP sourceFile, ICompressProgressTracker* progress);
    BE_SQLITE_EXPORT ZipErrors UncompressDgnDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progress);
    BE_SQLITE_EXPORT ZipErrors Uncompress(ILzmaOutputStream& out, ILzmaInputStream& in, bool isLzma2, ICompressProgressTracker* progress = NULL);
    BE_SQLITE_EXPORT ZipErrors Uncompress(bvector<Byte>&out, void const*inputBuffer, uint32_t inputSize);
    //! Use this method to decompress a blob of an embedded file. It is assumed that the blob does not have its own header and trailer.
    BE_SQLITE_EXPORT ZipErrors UncompressDgnDbBlob(bvector<Byte>&out, uint32_t expectedSize, void const*inputBuffer, uint32_t inputSize, Byte*header, uint32_t headerSize);
    };

#define SQLITE_FORMAT_SIGNATURE     "SQLite format 3"
#define DOWNLOAD_FORMAT_SIGNATURE   "Download SQLite"
#define SQLZLIB_FORMAT_SIGNATURE    "ZV-zlib"
#define SQLSNAPPY_FORMAT_SIGNATURE  "ZV-snappy"

//__PUBLISH_SECTION_START__

#define BEDB_PROPSPEC_NAMESPACE "be_Db"
#define BEDB_PROPSPEC_EMBEDBLOB_NAME "EmbdBlob"
//=======================================================================================
//! A property specification for the "be_Db" namespace. The "be_Db" namespace name is reserved for Properties that are
//! created by the BeSQLite::Db API itself and should not be used by other software layers.
// @bsiclass                                                    Keith.Bentley   03/11
//=======================================================================================
struct PropSpec : PropertySpec
    {
    PropSpec(Utf8CP name, PropertySpec::WantCompress compress = PropertySpec::COMPRESS_PROPERTY_Yes) : PropertySpec(name, BEDB_PROPSPEC_NAMESPACE, TXN_MODE_Normal, compress) {}
    };

//=======================================================================================
//! The names of properties in the "be_Db" namespace. These properties are
//! common to all Db files. These properties are created by the BeSQLite::Db API itself
//! and should not be used by other software layers.
// @bsiclass                                                    Keith.Bentley   03/11
//=======================================================================================
struct Properties
    {
    static PropSpec DbGuid()            {return PropSpec("DbGuid");}
    static PropSpec SchemaVersion()     {return PropSpec("SchemaVersion");}
    static PropSpec ProjectGuid()       {return PropSpec("ProjectGuid");}
    static PropSpec EmbeddedFileBlob()  {return PropSpec(BEDB_PROPSPEC_EMBEDBLOB_NAME, PropertySpec::COMPRESS_PROPERTY_No);}
    static PropSpec CreationDate()      {return PropSpec("CreationDate");}
    static PropSpec ExpirationDate()    {return PropSpec("ExpirationDate");}
    //! Build version of BeSqlite (e.g. 00.00.00.00) used to create this database; useful for forensic diagnostics.
    static PropSpec BeSQLiteBuild()     {return PropSpec("BeSQLiteBuild");}
    };

END_BENTLEY_SQLITE_NAMESPACE
