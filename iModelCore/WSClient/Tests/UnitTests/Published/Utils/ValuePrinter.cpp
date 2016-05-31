/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/ValuePrinter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ValuePrinter.h"
#include "WebServicesTestsHelper.h"
#include <map>

#define TO_VALUE_STRING_PAIR(value) {value, #value}

std::ostream& operator << (std::ostream &o, ICachingDataSource::Status status)
    {
    static std::map<ICachingDataSource::Status, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::Canceled),
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::DataNotCached),
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::DependencyNotSynced),
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::InternalCacheError),
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::NetworkErrorsOccured),
        TO_VALUE_STRING_PAIR(ICachingDataSource::Status::Success)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, ICachingDataSource::DataOrigin origin)
    {
    static std::map<ICachingDataSource::DataOrigin, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ICachingDataSource::DataOrigin::CachedData),
        TO_VALUE_STRING_PAIR(ICachingDataSource::DataOrigin::CachedOrRemoteData),
        TO_VALUE_STRING_PAIR(ICachingDataSource::DataOrigin::RemoteData),
        TO_VALUE_STRING_PAIR(ICachingDataSource::DataOrigin::RemoteOrCachedData)
        };

    Utf8String name = names[origin];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, CacheStatus status)
    {
    static std::map<CacheStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(CacheStatus::DataNotCached),
        TO_VALUE_STRING_PAIR(CacheStatus::Error),
        TO_VALUE_STRING_PAIR(CacheStatus::OK)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, FileCache location)
    {
    static std::map<FileCache, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(FileCache::Persistent),
        TO_VALUE_STRING_PAIR(FileCache::Temporary)
        };

    Utf8String name = names[location];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, IChangeManager::ChangeStatus status)
    {
    static std::map<IChangeManager::ChangeStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(IChangeManager::ChangeStatus::NoChange),
        TO_VALUE_STRING_PAIR(IChangeManager::ChangeStatus::Created),
        TO_VALUE_STRING_PAIR(IChangeManager::ChangeStatus::Modified),
        TO_VALUE_STRING_PAIR(IChangeManager::ChangeStatus::Deleted)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, IChangeManager::SyncStatus status)
    {
    static std::map<IChangeManager::SyncStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(IChangeManager::SyncStatus::Ready),
        TO_VALUE_STRING_PAIR(IChangeManager::SyncStatus::NotReady)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, ECInstanceKeyCR instance)
    {
    o << Utf8PrintfString("%lld:%lld", instance.GetECClassId(), instance.GetECInstanceId().IsValid() ? instance.GetECInstanceId().GetValue() : 0);
    return o;
    }

std::ostream& operator << (std::ostream &o, DateTimeCR date)
    {
    if (date.IsValid())
        {
        o << date.ToUtf8String();
        }
    else
        {
        o << "Invalid";
        }
    return o;
    }

std::ostream& operator << (std::ostream &o, ECValueCR value)
    {
    o << Utf8String(value.ToString());
    return o;
    }

std::ostream& operator << (std::ostream &o, DbResult value)
    {
    static std::map<DbResult, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(BE_SQLITE_OK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR),
        TO_VALUE_STRING_PAIR(BE_SQLITE_INTERNAL),
        TO_VALUE_STRING_PAIR(BE_SQLITE_PERM),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ABORT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_BUSY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_LOCKED),
        TO_VALUE_STRING_PAIR(BE_SQLITE_NOMEM),
        TO_VALUE_STRING_PAIR(BE_SQLITE_READONLY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_INTERRUPT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CORRUPT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_NOTFOUND),
        TO_VALUE_STRING_PAIR(BE_SQLITE_FULL),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CANTOPEN),
        TO_VALUE_STRING_PAIR(BE_SQLITE_PROTOCOL),
        TO_VALUE_STRING_PAIR(BE_SQLITE_EMPTY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_SCHEMA),
        TO_VALUE_STRING_PAIR(BE_SQLITE_TOOBIG),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_BASE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_MISMATCH),
        TO_VALUE_STRING_PAIR(BE_SQLITE_MISUSE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_NOLFS),
        TO_VALUE_STRING_PAIR(BE_SQLITE_AUTH),
        TO_VALUE_STRING_PAIR(BE_SQLITE_FORMAT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_RANGE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_NOTADB),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ROW),
        TO_VALUE_STRING_PAIR(BE_SQLITE_DONE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_READ),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SHORT_READ),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_WRITE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_FSYNC),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_DIR_FSYNC),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_TRUNCATE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_FSTAT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_UNLOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_RDLOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_DELETE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_BLOCKED),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_NOMEM),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_ACCESS),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_CHECKRESERVEDLOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_LOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_CLOSE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_DIR_CLOSE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SHMOPEN),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SHMSIZE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SHMLOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SHMMAP),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_SEEK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_IOERR_DELETE_NOENT),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_FileExists),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_AlreadyOpen),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_NoPropertyTable),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_FileNotFound),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_NoTxnActive),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_BadDbSchema),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_InvalidProfileVersion),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ProfileUpgradeFailed),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ProfileTooOld),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ProfileTooNewForReadWrite),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ProfileTooNew),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ERROR_ChangeTrackError),
        TO_VALUE_STRING_PAIR(BE_SQLITE_LOCKED_SHAREDCACHE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_BUSY_RECOVERY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CANTOPEN_NOTEMPDIR),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CANTOPEN_ISDIR),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CANTOPEN_FULLPATH),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CORRUPT_VTAB),
        TO_VALUE_STRING_PAIR(BE_SQLITE_READONLY_RECOVERY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_READONLY_CANTLOCK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_READONLY_ROLLBACK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_ABORT_ROLLBACK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_CHECK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_COMMITHOOK),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_FOREIGNKEY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_FUNCTION),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_NOTNULL),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_PRIMARYKEY),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_TRIGGER),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_UNIQUE),
        TO_VALUE_STRING_PAIR(BE_SQLITE_CONSTRAINT_VTAB),
        };

    Utf8String name = names[value];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, ECSqlStatus status)
    {
    static std::map<ECSqlStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ECSqlStatus::IndexOutOfBounds),
        TO_VALUE_STRING_PAIR(ECSqlStatus::InvalidECSql),
        TO_VALUE_STRING_PAIR(ECSqlStatus::NotYetSupported),
        TO_VALUE_STRING_PAIR(ECSqlStatus::ProgrammerError),
        TO_VALUE_STRING_PAIR(ECSqlStatus::Success),
        TO_VALUE_STRING_PAIR(ECSqlStatus::UserError)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, ECSqlStepStatus status)
    {
    static std::map<ECSqlStepStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ECSqlStepStatus::Done),
        TO_VALUE_STRING_PAIR(ECSqlStepStatus::Error),
        TO_VALUE_STRING_PAIR(ECSqlStepStatus::HasRow)
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, const ECInstanceKeyMultiMap::value_type& pair)
    {
    o << ECInstanceKey(pair.first, pair.second);
    return o;
    }

std::ostream& operator << (std::ostream &o, WSError::Status status)
    {
    static std::map<WSError::Status, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(WSError::Status::None),
        TO_VALUE_STRING_PAIR(WSError::Status::Canceled),
        TO_VALUE_STRING_PAIR(WSError::Status::ConnectionError),
        TO_VALUE_STRING_PAIR(WSError::Status::CertificateError),
        TO_VALUE_STRING_PAIR(WSError::Status::ServerNotSupported),
        TO_VALUE_STRING_PAIR(WSError::Status::ReceivedError),
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, WSError::Id errorId)
    {
    static std::map<WSError::Id, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(WSError::Id::Unknown),
        TO_VALUE_STRING_PAIR(WSError::Id::LoginFailed),
        TO_VALUE_STRING_PAIR(WSError::Id::SslRequired),
        TO_VALUE_STRING_PAIR(WSError::Id::NotEnoughRights),
        TO_VALUE_STRING_PAIR(WSError::Id::RepositoryNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::SchemaNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::ClassNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::PropertyNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::InstanceNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::FileNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::NotSupported),
        TO_VALUE_STRING_PAIR(WSError::Id::NoServerLicense),
        TO_VALUE_STRING_PAIR(WSError::Id::NoClientLicense),
        TO_VALUE_STRING_PAIR(WSError::Id::TooManyBadLoginAttempts),
        TO_VALUE_STRING_PAIR(WSError::Id::ServerError),
        TO_VALUE_STRING_PAIR(WSError::Id::BadRequest),
        TO_VALUE_STRING_PAIR(WSError::Id::Conflict),
        };

    Utf8String name = names[errorId];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, ObjectIdCR id)
    {
    o << id.ToString();
    return o;
    }

std::ostream& operator << (std::ostream &o, BeFileNameStatus errorId)
    {
    static std::map<BeFileNameStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(BeFileNameStatus::AccessViolation),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::AlreadyExists),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::CantCreate),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::CantDeleteDir),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::CantDeleteFile),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::FileNotFound),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::IllegalName),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::Success),
        TO_VALUE_STRING_PAIR(BeFileNameStatus::UnknownError)
    };

    Utf8String name = names[errorId];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, BeFileStatus errorId)
    {
    static std::map<BeFileStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(BeFileStatus::AccessViolationError),
        TO_VALUE_STRING_PAIR(BeFileStatus::FileNotFoundError),
        TO_VALUE_STRING_PAIR(BeFileStatus::FileNotOpenError),
        TO_VALUE_STRING_PAIR(BeFileStatus::NotLockedError),
        TO_VALUE_STRING_PAIR(BeFileStatus::ReadError),
        TO_VALUE_STRING_PAIR(BeFileStatus::SharingViolationError),
        TO_VALUE_STRING_PAIR(BeFileStatus::Success),
        TO_VALUE_STRING_PAIR(BeFileStatus::TooManyOpenFilesError),
        TO_VALUE_STRING_PAIR(BeFileStatus::UnknownError)
    };

    Utf8String name = names[errorId];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

namespace rapidjson
    {
    void PrintTo(const Value& value, ::std::ostream* os)
        {
        *os << RapidJsonToString(value);
        }
    void PrintTo(const Document& value, ::std::ostream* os)
        {
        *os << RapidJsonToString(value);
        }
    }

#ifdef WSCLIENT_ENABLE_DUPLICATING_SYMBOLS

std::ostream& operator << (std::ostream &o, CredentialsCR creds)
    {
    o << creds.GetUsername() + ":" + creds.GetPassword();
    return o;
    }

std::ostream& operator << (std::ostream &o, ConnectionStatus status)
    {
    static std::map<ConnectionStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ConnectionStatus::None),
        TO_VALUE_STRING_PAIR(ConnectionStatus::OK),
        TO_VALUE_STRING_PAIR(ConnectionStatus::Canceled),
        TO_VALUE_STRING_PAIR(ConnectionStatus::CouldNotConnect),
        TO_VALUE_STRING_PAIR(ConnectionStatus::ConnectionLost),
        TO_VALUE_STRING_PAIR(ConnectionStatus::Timeout),
        TO_VALUE_STRING_PAIR(ConnectionStatus::UnknownError),
        };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, HttpStatus status)
    {
    o << static_cast<int>(status);
    return o;
    }

std::ostream& operator << (std::ostream &o, BeVersionCR version)
    {
    o << version.ToString().c_str();
    return o;
    }


BEGIN_BENTLEY_NAMESPACE
namespace Json
    {
    void PrintTo(const Value& value, ::std::ostream* os)
        {
        *os << value.toStyledString();
        }
    }

void PrintTo(const Utf8String& value, ::std::ostream* os)
    {
    *os << '"' << value << '"';
    }

void PrintTo(const WString& value, ::std::ostream* os)
    {
    PrintTo(Utf8String(value), os);
    }

void PrintTo(const BeFileName& value, ::std::ostream* os)
    {
    PrintTo(Utf8String(value), os);
    }

void PrintTo(BentleyStatus value, ::std::ostream* os)
    {
    static std::map<BentleyStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(ERROR),
        TO_VALUE_STRING_PAIR(SUCCESS)
        };

    Utf8String name = names[value];
    BeAssert(!name.empty() && "Add missing value");
    *os << name;
    }

END_BENTLEY_NAMESPACE

#endif