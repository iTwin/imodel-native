/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformErrors.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <BeSQLite/L10N.h>
#include "DgnPlatformBaseType.r.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
    LOCK_ERROR_BASE                 = 0x15000,
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
    RevisionStarted,
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

//! Return codes for methods which perform locking operations
enum class LockStatus : int
{
    Success = SUCCESS,
    ServerUnavailable = LOCK_ERROR_BASE + 1, //!< The locks server did not respond to a lock request
    AlreadyHeld, //!< A requested lock was already held by another briefcase
    SyncError, //!< Failed to sync local locks with server
    InvalidResponse, //!< Response from server not understood
    PendingTransactions, //!< An operation requires local changes to be committed or abandoned
    LockUsed, //!< A lock cannot be relinquished because the associated object has been modified
    CannotCreateRevision, //!< An operation required creation of a DgnRevision, which failed
    InvalidRequest, //!< Request to server not understood
};

/** @cond BENTLEY_SDK_Publisher */

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

/** @endcond */

END_BENTLEY_DGNPLATFORM_NAMESPACE
