/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Napi/napi.h>
#include <Bentley/Bentley.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Error code categories
//! @private
//=======================================================================================
enum DgnErrorCategories
    {
    DGNDB_ERROR_BASE                = 0x10000,
    VIEWPORT_ERROR_BASE             = 0x11000,
    LINESTYLE_ERROR_BASE            = 0x12000,
    GEOREFERENCE_ERROR_BASE         = 0x13000,
    REPOSITORY_ERROR_BASE           = 0x15000,
    CHANGESET_ERROR_BASE            = 0x16000,
    IMODELJSNATIVE_ERROR_BASE       = 0x17000,
    };

//=======================================================================================
//! Errors related to a DgnDb
//=======================================================================================
enum class DgnDbStatus : int
    {
    Success                = SUCCESS,
    AlreadyLoaded          = DGNDB_ERROR_BASE + 1,
    AlreadyOpen            = DGNDB_ERROR_BASE + 2,
    BadArg                 = DGNDB_ERROR_BASE + 3,
    BadElement             = DGNDB_ERROR_BASE + 4,
    BadModel               = DGNDB_ERROR_BASE + 5,
    BadRequest             = DGNDB_ERROR_BASE + 6,
    BadSchema              = DGNDB_ERROR_BASE + 7,
    CannotUndo             = DGNDB_ERROR_BASE + 8,
    CodeNotReserved        = DGNDB_ERROR_BASE + 9,
    DeletionProhibited     = DGNDB_ERROR_BASE + 10,
    DuplicateCode          = DGNDB_ERROR_BASE + 11,
    DuplicateName          = DGNDB_ERROR_BASE + 12,
    ElementBlockedChange   = DGNDB_ERROR_BASE + 13,
    FileAlreadyExists      = DGNDB_ERROR_BASE + 14,
    FileNotFound           = DGNDB_ERROR_BASE + 15,
    FileNotLoaded          = DGNDB_ERROR_BASE + 16,
    ForeignKeyConstraint   = DGNDB_ERROR_BASE + 17,
    IdExists               = DGNDB_ERROR_BASE + 18,
    InvalidCategory        = DGNDB_ERROR_BASE + 20,
    InvalidCode            = DGNDB_ERROR_BASE + 21,
    InvalidCodeSpec        = DGNDB_ERROR_BASE + 22,
    InvalidId              = DGNDB_ERROR_BASE + 23,
    InvalidName            = DGNDB_ERROR_BASE + 24,
    InvalidParent          = DGNDB_ERROR_BASE + 25,
    InvalidProfileVersion  = DGNDB_ERROR_BASE + 26,
    IsCreatingChangeset    = DGNDB_ERROR_BASE + 27,
    LockNotHeld            = DGNDB_ERROR_BASE + 28,
    Mismatch2d3d           = DGNDB_ERROR_BASE + 29,
    MismatchGcs            = DGNDB_ERROR_BASE + 30,  //!< The Geographic Coordinate Systems of the source and target are not based on equivalent projections
    MissingDomain          = DGNDB_ERROR_BASE + 31,
    MissingHandler         = DGNDB_ERROR_BASE + 32,
    MissingId              = DGNDB_ERROR_BASE + 33,
    NoGeometry             = DGNDB_ERROR_BASE + 34,
    NoMultiTxnOperation    = DGNDB_ERROR_BASE + 35,
    NotEnabled             = DGNDB_ERROR_BASE + 37,
    NotFound               = DGNDB_ERROR_BASE + 38,
    NotOpen                = DGNDB_ERROR_BASE + 39,
    NotOpenForWrite        = DGNDB_ERROR_BASE + 40,
    NotSameUnitBase        = DGNDB_ERROR_BASE + 41,
    NothingToRedo          = DGNDB_ERROR_BASE + 42,
    NothingToUndo          = DGNDB_ERROR_BASE + 43,
    ParentBlockedChange    = DGNDB_ERROR_BASE + 44,
    ReadError              = DGNDB_ERROR_BASE + 45,
    ReadOnly               = DGNDB_ERROR_BASE + 46,
    ReadOnlyDomain         = DGNDB_ERROR_BASE + 47,
    SQLiteError            = DGNDB_ERROR_BASE + 49,
    TransactionActive      = DGNDB_ERROR_BASE + 50,
    UnitsMissing           = DGNDB_ERROR_BASE + 51,
    UnknownFormat          = DGNDB_ERROR_BASE + 52,
    UpgradeFailed          = DGNDB_ERROR_BASE + 53,
    ValidationFailed       = DGNDB_ERROR_BASE + 54,
    VersionTooNew          = DGNDB_ERROR_BASE + 55,
    VersionTooOld          = DGNDB_ERROR_BASE + 56,
    ViewNotFound           = DGNDB_ERROR_BASE + 57,
    WriteError             = DGNDB_ERROR_BASE + 58,
    WrongClass             = DGNDB_ERROR_BASE + 59,
    WrongDgnDb             = DGNDB_ERROR_BASE + 60,
    WrongDomain            = DGNDB_ERROR_BASE + 61,
    WrongElement           = DGNDB_ERROR_BASE + 62,
    WrongHandler           = DGNDB_ERROR_BASE + 63,
    WrongModel             = DGNDB_ERROR_BASE + 64,
    ConstraintNotUnique    = DGNDB_ERROR_BASE + 65,
    NoGeoLocation          = DGNDB_ERROR_BASE + 66,
    TimeoutFailed          = DGNDB_ERROR_BASE + 67,
    Aborted                = DGNDB_ERROR_BASE + 72,
    };

class DgnDbStatusHelper {
    public:
        static iTwinErrorId GetITwinError(DgnDbStatus status) {
            switch (status)
                {
                case DgnDbStatus::Success: return { "dgn-db", "Success" };
                case DgnDbStatus::AlreadyLoaded: return { "dgn-db", "AlreadyLoaded" };
                case DgnDbStatus::AlreadyOpen: return { "dgn-db", "AlreadyOpen" };
                case DgnDbStatus::BadArg: return { "dgn-db", "BadArg" };
                case DgnDbStatus::BadElement: return { "dgn-db", "BadElement" };
                case DgnDbStatus::BadModel: return { "dgn-db", "BadModel" };
                case DgnDbStatus::BadRequest: return { "dgn-db", "BadRequest" };
                case DgnDbStatus::BadSchema: return { "dgn-db", "BadSchema" };
                case DgnDbStatus::CannotUndo: return { "dgn-db", "CannotUndo" };
                case DgnDbStatus::CodeNotReserved: return { "dgn-db", "CodeNotReserved" };
                case DgnDbStatus::DeletionProhibited: return { "dgn-db", "DeletionProhibited" };
                case DgnDbStatus::DuplicateCode: return { "dgn-db", "DuplicateCode" };
                case DgnDbStatus::DuplicateName: return { "dgn-db", "DuplicateName" };
                case DgnDbStatus::ElementBlockedChange: return { "dgn-db", "ElementBlockedChange" };
                case DgnDbStatus::FileAlreadyExists: return { "dgn-db", "FileAlreadyExists" };
                case DgnDbStatus::FileNotFound: return { "dgn-db", "FileNotFound" };
                case DgnDbStatus::FileNotLoaded: return { "dgn-db", "FileNotLoaded" };
                case DgnDbStatus::ForeignKeyConstraint: return { "dgn-db", "ForeignKeyConstraint" };
                case DgnDbStatus::IdExists: return { "dgn-db", "IdExists" };
                case DgnDbStatus::InvalidCategory: return { "dgn-db", "InvalidCategory" };
                case DgnDbStatus::InvalidCode: return { "dgn-db", "InvalidCode" };
                case DgnDbStatus::InvalidCodeSpec: return { "dgn-db", "InvalidCodeSpec" };
                case DgnDbStatus::InvalidId: return { "dgn-db", "InvalidId" };
                case DgnDbStatus::InvalidName: return { "dgn-db", "InvalidName" };
                case DgnDbStatus::InvalidParent: return { "dgn-db", "InvalidParent" };
                case DgnDbStatus::InvalidProfileVersion: return { "dgn-db", "InvalidProfileVersion" };
                case DgnDbStatus::IsCreatingChangeset: return { "dgn-db", "IsCreatingChangeset" };
                case DgnDbStatus::LockNotHeld: return { "dgn-db", "LockNotHeld" };
                case DgnDbStatus::Mismatch2d3d: return { "dgn-db", "Mismatch2d3d" };
                case DgnDbStatus::MismatchGcs: return { "dgn-db", "MismatchGcs" };
                case DgnDbStatus::MissingDomain: return { "dgn-db", "MissingDomain" };
                case DgnDbStatus::MissingHandler: return { "dgn-db", "MissingHandler" };
                case DgnDbStatus::MissingId: return { "dgn-db", "MissingId" };
                case DgnDbStatus::NoGeometry: return { "dgn-db", "NoGeometry" };
                case DgnDbStatus::NoMultiTxnOperation: return { "dgn-db", "NoMultiTxnOperation" };
                case DgnDbStatus::NotEnabled: return { "dgn-db", "NotEnabled" };
                case DgnDbStatus::NotFound: return { "dgn-db", "NotFound" };
                case DgnDbStatus::NotOpen: return { "dgn-db", "NotOpen" };
                case DgnDbStatus::NotOpenForWrite: return { "dgn-db", "NotOpenForWrite" };
                case DgnDbStatus::NotSameUnitBase: return { "dgn-db", "NotSameUnitBase" };
                case DgnDbStatus::NothingToRedo: return { "dgn-db", "NothingToRedo" };
                case DgnDbStatus::NothingToUndo: return { "dgn-db", "NothingToUndo" };
                case DgnDbStatus::ParentBlockedChange: return { "dgn-db", "ParentBlockedChange" };
                case DgnDbStatus::ReadError: return { "dgn-db", "ReadError" };
                case DgnDbStatus::ReadOnly: return { "dgn-db", "ReadOnly" };
                case DgnDbStatus::ReadOnlyDomain: return { "dgn-db", "ReadOnlyDomain" };
                case DgnDbStatus::SQLiteError: return { "dgn-db", "SQLiteError" };
                case DgnDbStatus::TransactionActive: return { "dgn-db", "TransactionActive" };
                case DgnDbStatus::UnitsMissing: return { "dgn-db", "UnitsMissing" };
                case DgnDbStatus::UnknownFormat: return { "dgn-db", "UnknownFormat" };
                case DgnDbStatus::UpgradeFailed: return { "dgn-db", "UpgradeFailed" };
                case DgnDbStatus::ValidationFailed: return { "dgn-db", "ValidationFailed" };
                case DgnDbStatus::VersionTooNew: return { "dgn-db", "VersionTooNew" };
                case DgnDbStatus::VersionTooOld: return { "dgn-db", "VersionTooOld" };
                case DgnDbStatus::ViewNotFound: return { "dgn-db", "ViewNotFound" };
                case DgnDbStatus::WriteError: return { "dgn-db", "WriteError" };
                case DgnDbStatus::WrongClass: return { "dgn-db", "WrongClass" };
                case DgnDbStatus::WrongDgnDb: return { "dgn-db", "WrongDgnDb" };
                case DgnDbStatus::WrongDomain: return { "dgn-db", "WrongDomain" };
                case DgnDbStatus::WrongElement: return { "dgn-db", "WrongElement" };
                case DgnDbStatus::WrongHandler: return { "dgn-db", "WrongHandler" };
                case DgnDbStatus::WrongModel: return { "dgn-db", "WrongModel" };
                case DgnDbStatus::ConstraintNotUnique: return { "dgn-db", "ConstraintNotUnique" };
                case DgnDbStatus::NoGeoLocation: return { "dgn-db", "NoGeoLocation" };
                case DgnDbStatus::TimeoutFailed: return { "dgn-db", "TimeoutFailed" };
                case DgnDbStatus::Aborted: return { "dgn-db", "Aborted" };
                default: return { "dgn-db", "UnknownDgnDbStatus" };
                }
        }
};

//! iTwin Error Keys for iModelJsNative
enum class IModelJsNativeErrorKey : int
{
    BadArg = IMODELJSNATIVE_ERROR_BASE + 1,
    NotFound,
    NotInitialized,
    NotOpen,
    FontError,
    SchemaError,
    ChangesetError,
    TypeError,
    CompressionError,
    ElementGeometryCacheError,
    GeometryStreamError,
    ECClassError,
    RuntimeError,
    NativeAssertion,
    LockNotHeld
};

class IModelJsNativeErrorKeyHelper {
    public:
    static iTwinErrorId GetITwinError(IModelJsNativeErrorKey key) {
        switch (key)
            {
            case IModelJsNativeErrorKey::BadArg: return {"imodel-native", "BadArg"};
            case IModelJsNativeErrorKey::NotFound: return {"imodel-native", "NotFound"};
            case IModelJsNativeErrorKey::NotInitialized: return {"imodel-native", "NotInitialized"};
            case IModelJsNativeErrorKey::NotOpen: return {"imodel-native", "NotOpen"};
            case IModelJsNativeErrorKey::FontError: return {"imodel-native", "FontError"};
            case IModelJsNativeErrorKey::SchemaError: return {"imodel-native", "SchemaError"};
            case IModelJsNativeErrorKey::ChangesetError: return {"imodel-native", "ChangesetError"};
            case IModelJsNativeErrorKey::TypeError: return {"imodel-native", "TypeError"};
            case IModelJsNativeErrorKey::CompressionError: return {"imodel-native", "CompressionError"};
            case IModelJsNativeErrorKey::ElementGeometryCacheError: return {"imodel-native", "ElementGeometryCacheError"};
            case IModelJsNativeErrorKey::GeometryStreamError: return {"imodel-native", "GeometryStreamError"};
            case IModelJsNativeErrorKey::ECClassError: return {"imodel-native", "ECClassError"};
            case IModelJsNativeErrorKey::RuntimeError: return {"imodel-native", "RuntimeError"};
            case IModelJsNativeErrorKey::NativeAssertion: return {"imodel-native", "NativeAssertion"};
            case IModelJsNativeErrorKey::LockNotHeld: return {"imodel-native", "LockNotHeld"};
            default: return {"imodel-native", "UnknownIModelJsNativeErrorKey"};
            }
    }
};

//! Status Values for DgnViewport methods
enum class ViewportStatus : int
{
    Success = SUCCESS,
    ViewNotInitialized = VIEWPORT_ERROR_BASE + 1,
    AlreadyAttached,
    NotAttached,
    DrawFailure,
    NotResized,
    ModelNotFound,
    InvalidWindow,
    MinWindow,
    MaxWindow,
    MaxZoom,
    MaxDisplayDepth,
    InvalidUpVector,
    InvalidTargetPoint,
    InvalidLens,
    InvalidViewport,
};

//! Return codes for methods which perform repository management operations
enum class RepositoryStatus : int
{
    Success = 0,
    ServerUnavailable = 0x15001, //!< The repository server did not respond to a request
    LockAlreadyHeld = 0x15002, //!< A requested lock was already held by another briefcase
    SyncError = 0x15003, //!< Failed to sync briefcase manager with server
    InvalidResponse = 0x15004, //!< Response from server not understood
    PendingTransactions = 0x15005, //!< An operation requires local changes to be committed or abandoned
    LockUsed = 0x15006, //!< A lock cannot be relinquished because the associated object has been modified
    CannotCreateRevision = 0x15007, //!< An operation required creation of a ChangesetProps, which failed
    InvalidRequest = 0x15008, //!< Request to server not understood
    RevisionRequired = 0x15009, //!< A revision committed to the server must be integrated into the briefcase before the operation can be completed
    CodeUnavailable = 0x1500A, //!< A requested DgnCode is reserved by another briefcase or in use
    CodeNotReserved = 0x1500B, //!< A DgnCode cannot be released because it has not been reserved by the requesting briefcase
    CodeUsed = 0x1500C, //!< A DgnCode cannot be relinquished because it has been used locally
    LockNotHeld = 0x1500D, //!< A required lock is not held by this briefcase
    RepositoryIsLocked = 0x1500E, //!< Repository is currently locked, no changes allowed
};

// The typescript generator require literal values for all enum members...
static_assert((int)RepositoryStatus::ServerUnavailable == REPOSITORY_ERROR_BASE + 1, "Inconsistent enum");

//! Status codes for Changesets
enum class ChangesetStatus : int {
    Success = SUCCESS, //!< Success
    ApplyError = CHANGESET_ERROR_BASE + 1, //!< Error applying a revision when merging, reversing or reinstating it.
    ChangeTrackingNotEnabled, //!< Change tracking has not been enabled. The Revision API mandates this.
    CorruptedChangeStream, //!< Contents of the change stream are corrupted and does not match the Revision
    FileNotFound, //!< File containing the changes to the revision is not found
    FileWriteError, //!< Error writing the contents of the revision to the backing change stream file
    HasLocalChanges, //!< Cannot perform the operation since the Db has local changes
    HasUncommittedChanges, //!< Cannot perform the operation since current transaction has uncommitted changes
    InvalidId, //!< Invalid Revision Id
    InvalidVersion, //! !< Invalid version of the revision
    InDynamicTransaction, //!< Cannot perform the operation since system is in the middle of a dynamic transaction
    IsCreatingChangeset, //!< Cannot perform operation since system is in the middle of a creating a revision
    IsNotCreatingChangeset, //!< Cannot perform operation since the system is not creating a revision
    MergePropagationError, //!< Error propagating the changes after the merge
    NothingToMerge, //!< No revisions to merge
    NoTransactions, //!< No transactions are available to create a revision
    ParentMismatch, //!< Parent revision of the Db does not match the parent id of the revision
    SQLiteError, //!< Error performing a SQLite operation on the Db
    WrongDgnDb, //!< Revision originated in a different Db
    CouldNotOpenDgnDb, //!< Could not open the DgnDb to merge revision
    MergeSchemaChangesOnOpen, //! Cannot merge changes in in an open DgnDb. Close the DgnDb, and process the operation when it is opened.
    ReverseOrReinstateSchemaChanges, //! Cannot reverse or reinstate schema changes.
    ProcessSchemaChangesOnOpen, //! Cannot process changes schema changes in an open DgnDb. Close the DgnDb, and process the operation when it is opened.
    CannotMergeIntoReadonly, //! Cannot merge changes into a Readonly DgnDb.
    CannotMergeIntoReversed, //! Cannot merge changes into a DgnDb that has reversed revisions.
    BadVersionId, // illegal version length
    CorruptedTxn, // Fail to decompress txn
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum LineStyleStatus
{
    LINESTYLE_STATUS_Success                    = BSISUCCESS,
    LINESTYLE_STATUS_Error                      = BSIERROR,
    LINESTYLE_STATUS_BadArgument                = LINESTYLE_ERROR_BASE + 0x01,
    LINESTYLE_STATUS_UnknownResourceError       = LINESTYLE_ERROR_BASE + 0x02,
    LINESTYLE_STATUS_FileNotFound               = LINESTYLE_ERROR_BASE + 0x03,
    LINESTYLE_STATUS_NotSameFile                = LINESTYLE_ERROR_BASE + 0x04,
    LINESTYLE_STATUS_FileReadOnly               = LINESTYLE_ERROR_BASE + 0x07,
    LINESTYLE_STATUS_AlreadyExists              = LINESTYLE_ERROR_BASE + 0x08,
    LINESTYLE_STATUS_BadFormat                  = LINESTYLE_ERROR_BASE + 0x09,
    LINESTYLE_STATUS_StyleNotFound              = LINESTYLE_ERROR_BASE + 0x0a,
    LINESTYLE_STATUS_ComponentNotFound          = LINESTYLE_ERROR_BASE + 0x0b,
    LINESTYLE_STATUS_SQLITE_Error               = LINESTYLE_ERROR_BASE + 0x0c,
    LINESTYLE_STATUS_SQLITE_Constraint          = LINESTYLE_ERROR_BASE + 0x0d,
    LINESTYLE_STATUS_ConvertingComponent        = LINESTYLE_ERROR_BASE + 0x0e,
};

END_BENTLEY_DGN_NAMESPACE
