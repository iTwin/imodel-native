/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <system_error>
#include "iModelBridgeFwkTypes.h"

#ifdef __IMODEL_BRIDGE_BUILD__
#define IMODEL_BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODEL_BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif


BEGIN_BENTLEY_NAMESPACE namespace iModel {namespace Hub {
    struct Error;
    }}END_BENTLEY_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Abeesh.Basheer   06/19
//=======================================================================================
BEGIN_BENTLEY_DGN_NAMESPACE

//!iModelBridgeErrorId is a union of all the possible errors returned from the underlying libraries. the standard error_category
//! can be used to get more detailed error
enum class iModelBridgeErrorId
    {
    Unknown = -1,
    Success = 0,
    //Leaving a gap to deal with all the iModelHub Error Ids that start at 0 and stop before 600
    Usage_Error = 10000,
    Converter_Error,
    Local_error,
    Unhandled_exception,
    MissingBridgeInRegistry,
    MissingFunctionExport,
    MissingInstance,
    ProjectwiseError,
    FailedToLockChannelParent,
    // The error status could also be a RepositoryStatus, which starts at 86016 (0x15000)
    };


class iModelBridgeError_category : public std::error_category
    {

    };

class iModelHubError_category: public std::error_category
    {

    };  

struct iModelBridgeError
    {
    iModelBridgeErrorId m_id;
    Utf8String          m_message;
    Utf8String          m_description;
    Json::Value         m_extendedData;

    IMODEL_BRIDGE_EXPORT iModelBridgeError();
    IMODEL_BRIDGE_EXPORT iModelBridgeError(BentleyApi::iModel::Hub::Error const& hubError);

    //!Write a structured error message for calling applications.
    IMODEL_BRIDGE_EXPORT void WriteErrorMessage(BeFileNameCR errorFileName);

    int GetIntErrorId() { return static_cast<int> (m_id); }
    BentleyStatus GetBentleyStatus() { return static_cast<BentleyStatus> (m_id); }
    };

END_BENTLEY_DGN_NAMESPACE

// Tell the C++ 11 STL metaprogramming that enum iModelBridgeError
// is registered with the standard error code system
namespace std{
    template <> struct is_error_code_enum<BentleyApi::Dgn::iModelBridgeError> : true_type
    {};

    //!Bringing
    //template <> struct is_error_code_enum<BentleyApi::iModel::Hub::Error::Id> : true_type
    //{};
}