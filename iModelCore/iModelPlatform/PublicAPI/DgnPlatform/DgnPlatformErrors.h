/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


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
    REVISION_ERROR_BASE             = 0x16000,
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
    InDynamicTransaction   = DGNDB_ERROR_BASE + 19,
    InvalidCategory        = DGNDB_ERROR_BASE + 20,
    InvalidCode            = DGNDB_ERROR_BASE + 21,
    InvalidCodeSpec        = DGNDB_ERROR_BASE + 22,
    InvalidId              = DGNDB_ERROR_BASE + 23,
    InvalidName            = DGNDB_ERROR_BASE + 24,
    InvalidParent          = DGNDB_ERROR_BASE + 25,
    InvalidProfileVersion  = DGNDB_ERROR_BASE + 26,
    IsCreatingRevision     = DGNDB_ERROR_BASE + 27,
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
    CannotCreateRevision = 0x15007, //!< An operation required creation of a DgnRevision, which failed
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

//! Status codes for the Revision API
enum class RevisionStatus : int
    {
    Success = SUCCESS, //!< Success
    ApplyError = REVISION_ERROR_BASE + 1, //!< Error applying a revision when merging, reversing or reinstating it.
    ChangeTrackingNotEnabled, //!< Change tracking has not been enabled. The Revision API mandates this.
    CorruptedChangeStream, //!< Contents of the change stream are corrupted and does not match the Revision
    FileNotFound, //!< File containing the changes to the revision is not found
    FileWriteError, //!< Error writing the contents of the revision to the backing change stream file
    HasLocalChanges, //!< Cannot perform the operation since the Db has local changes
    HasUncommittedChanges, //!< Cannot perform the operation since current transaction has uncommitted changes
    InvalidId, //!< Invalid Revision Id
    InvalidVersion, //! !< Invalid version of the revision
    InDynamicTransaction, //!< Cannot perform the operation since system is in the middle of a dynamic transaction
    IsCreatingRevision, //!< Cannot perform operation since system is in the middle of a creating a revision
    IsNotCreatingRevision, //!< Cannot perform operation since the system is not creating a revision
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
