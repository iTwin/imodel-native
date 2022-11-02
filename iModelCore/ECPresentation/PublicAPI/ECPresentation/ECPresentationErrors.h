/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<
    typename TBaseException,
    typename = std::enable_if_t<std::is_base_of<std::exception, TBaseException>::value>
>
struct ECPresentationException : TBaseException
{
protected:
    template<
        typename T = TBaseException,
        typename = std::enable_if_t<std::is_default_constructible<T>::value>
    >
    ECPresentationException() : T() {}

    template<
        typename T = TBaseException,
        typename = std::enable_if_t<std::is_constructible<T, Utf8CP>::value>
    >
    ECPresentationException(Utf8CP message) : T(message) {}

    template<
        typename T = TBaseException,
        typename = std::enable_if_t<std::is_constructible<T, Utf8String>::value>
    >
    ECPresentationException(Utf8String message) : T(message) {}

public:
    // TODO: add telemetry data, diagnostics, etc.
};

/*=================================================================================**//**
* The exception is thrown when an ECPresentation request is cancelled. The cancellation
* may happen due to cancellation request on the returned `folly::Future` or due to other factors
* like the termination of `ECPresentationManager` or closing of the iModel.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CancellationException : ECPresentationException<std::exception>
    {};

/*=================================================================================**//**
* This is thrown when library connection to Db is unusable due to being busy (we get
* BE_SQLITE_BUSY trying to run a query).
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DbConnectionBusyException : ECPresentationException<std::runtime_error>
    {
    DbConnectionBusyException() 
        : ECPresentationException<std::runtime_error>("IModel is locked and can not be read")
        {}
    };

/*=================================================================================**//**
* This is thrown when library connection to Db is interrupted while data is still
* being queried.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DbConnectionInterruptException : ECPresentationException<std::exception>
    {};

/*=================================================================================**//**
* This is thrown when an internal library error occurs. The error message will most likely
* be irrelevant to requestor, but could help in the debugging process.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InternalError : ECPresentationException<std::runtime_error>
    {
    DEFINE_T_SUPER(ECPresentationException<std::runtime_error>)
    InternalError(Utf8CP message = "") : T_Super(message) {}
    InternalError(Utf8String message) : T_Super(message) {}
    };

/*=================================================================================**//**
* This is thrown when an invalid argument is passed when requesting presentation data. A
* couple of examples would be an ID of non-existing ruleset or referencing a non-existing
* ECClass.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InvalidArgumentException : ECPresentationException<std::invalid_argument>
    {
    DEFINE_T_SUPER(ECPresentationException<std::invalid_argument>)
    InvalidArgumentException(Utf8CP message = "") : T_Super(message) {}
    InvalidArgumentException(Utf8String message) : T_Super(message) {}
    };

#ifdef WIP_LIMITING_RESULT_SETS_SIZE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ResultSetTooLargeError : ECPresentationException<std::length_error>
{
private:
    size_t m_allowedSize;
    Nullable<PossiblyApproximate<size_t>> m_actualSize;
public:
    ResultSetTooLargeError(size_t allowedSize, Nullable<PossiblyApproximate<size_t>> actualSize = nullptr)
        : m_allowedSize(allowedSize), m_actualSize(actualSize)
        {}
    size_t GetAllowedSize() const {return m_allowedSize;}
    void SetActualSize(Nullable<PossiblyApproximate<size_t>> value) {m_actualSize = value;}
    Nullable<PossiblyApproximate<size_t>> const& GetActualSize() const {return m_actualSize;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TooManyInstancesError : ResultSetTooLargeError
{
private:
    ContentDescriptorCPtr m_descriptor;
public:
    TooManyInstancesError(size_t allowedSize, Nullable<PossiblyApproximate<size_t>> actualSize = nullptr, ContentDescriptorCP descriptor = nullptr)
        : ResultSetTooLargeError(allowedSize, actualSize), m_descriptor(descriptor)
        {}
    void SetDescriptor(ContentDescriptorCP value) {m_descriptor = value;}
    ContentDescriptorCP GetDescriptor() const {return m_descriptor.get();}
};
#endif

END_BENTLEY_ECPRESENTATION_NAMESPACE
