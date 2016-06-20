/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformErrors.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <BeJavaScriptTools/BeJavaScriptTools.h>

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
    MARKUPPROJECT_ERROR_BASE        = 0x14000,
    REPOSITORY_ERROR_BASE           = 0x15000,
    REVISION_ERROR_BASE             = 0x16000,
    };

//=======================================================================================
//! Errors related to a DgnDb
//=======================================================================================
enum class DgnDbStatus : int
    {
    Success = SUCCESS,
    AlreadyOpen = DGNDB_ERROR_BASE + 1,
    AlreadyLoaded,
    BadArg,
    BadElement,
    BadModel,
    BadRequest,
    BadSchema,
    DuplicateName,
    DuplicateCode,
    ElementBlockedChange,
    FileAlreadyExists,
    FileNotFound,
    FileNotLoaded,
    ForeignKeyConstraint,
    IdExists,
    InvalidId,
    InvalidName,
    InvalidParent,
    InvalidSchemaVersion,
    MismatchGcs, //!< The Geographic Coordinate Systems of the source and target are not based on equivalent projections
    Mismatch2d3d,
    MissingDomain,
    MissingHandler,
    MissingId,
    NoGeometry,
    NotDgnMarkupProject,
    NotEnabled,
    NotFound,
    LockNotHeld,
    NotOpen,
    NotOpenForWrite,
    NotSameUnitBase,
    NothingToRedo,
    NothingToUndo,
    ParentBlockedChange,
    ReadError,
    ReadOnly,
    IsCreatingRevision,
    SQLiteError,
    TransactionActive,
    UnitsMissing,
    UnknownFormat,
    UpgradeFailed,
    ValidationFailed,
    VersionTooNew,
    VersionTooOld,
    ViewNotFound,
    WriteError,
    WrongClass,
    WrongDomain,
    WrongDgnDb,
    WrongElement,
    WrongHandler,
    WrongModel,
    InvalidCategory,
    DeletionProhibited,
    InDynamicTransaction,
    NoMultiTxnOperation,
    InvalidCodeAuthority,
    CodeNotReserved,
    RepositoryManagerError,
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
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
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
};

//__PUBLISH_SECTION_END__
// The typescript generator require literal values for all enum members...
static_assert((int)RepositoryStatus::ServerUnavailable == REPOSITORY_ERROR_BASE + 1, "Inconsistent enum");
//__PUBLISH_SECTION_START__

//! Status codes for the Revision API
enum class RevisionStatus : int
    {
    Success = SUCCESS, //!< Success
    ChangeTrackingNotEnabled = REVISION_ERROR_BASE + 1, //!< Change tracking has not been enabled. The Revision API mandates this.
    CorruptedChangeStream, //!< Contents of the change stream does not match the Revision
    FileNotFound, //!< File containing the changes to the revision is not found
    FileWriteError, //!< Error writing the contents of the revision to the backing change stream file
    InvalidId, //!< Invalid Revision Id
    InDynamicTransaction, //!< Cannot perform the operation since system is in the middle of a dynamic transaction
    IsCreatingRevision, //!< Cannot perform operation since system is in the middle of a creating a revision
    IsNotCreatingRevision, //!< Cannot perform operation since the system is not creating a revision
    MergeError, //!< Error merging changes from the revision to the Db
    MergePropagationError, //!< Error propagating the changes after the merge
    NothingToMerge, //!< No revisions to merge
    NoTransactions, //!< No transactions are available to create a revision
    SQLiteError, //!< Error performing a SQLite operation on the Db
    TransactionHasUnsavedChanges, //!< Cannot perform the operation since current transaction has unsaved changes
    WrongDgnDb, //!< Revision originated in a different Db 
    ParentMismatch, //!< Parent revision of the Db does not match the parent id of the revision
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
