#/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ProfilesLogging.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Profiles\ProfilesDefinitions.h>
#include <Logging\bentleylogging.h>

#define PROFILES_LOG (*NativeLogging::LoggingManager::GetLogger (PRF_LOGGER_NAMESPACE))

/*---------------------------------------------------------------------------------**//*
* Helper class used to log common errors of Profiles domain.
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesLog
    {
    ProfilesLog() = delete;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedDelete_ProfileHasReference (Utf8CP pReferenecedProfileName, Dgn::DgnElementId referencedProfileId,
                                                  Utf8CP pReferencingProfileName, Dgn::DgnElementId referencingProfileId)
        {
        Utf8Char referencedIdBuffer[Dgn::DgnElementId::ID_STRINGBUFFER_LENGTH];
        Utf8Char referencingIdBuffer[Dgn::DgnElementId::ID_STRINGBUFFER_LENGTH];

        referencedProfileId.ToString (referencedIdBuffer, BeInt64Id::UseHex::Yes);
        referencingProfileId.ToString (referencingIdBuffer, BeInt64Id::UseHex::Yes);

        Utf8String errorString = "Failed to delete " + Utf8String (pReferenecedProfileName) + " instance (id: %s), because it is being referenced by " +
                                 Utf8String (pReferencingProfileName) + " instance (id: %s).";
        PROFILES_LOG.errorv (errorString.c_str(), referencedIdBuffer, referencingIdBuffer);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedCopyFrom_InvalidElement (Utf8CP pProfileClassName, Dgn::DgnElementId profileId, Dgn::DgnElementId sourceElementId)
        {
        Utf8Char profileIdBuffer[Dgn::DgnElementId::ID_STRINGBUFFER_LENGTH];
        Utf8Char sourceElementIdBuffer[Dgn::DgnElementId::ID_STRINGBUFFER_LENGTH];

        profileId.ToString (profileIdBuffer, BeInt64Id::UseHex::Yes);
        sourceElementId.ToString (sourceElementIdBuffer, BeInt64Id::UseHex::Yes);

        Utf8String errorString = "Failed to copy from DgnElement instance (id: %s) to a " + Utf8String (pProfileClassName) + " instance (id: %s)"
                                 ", because DgnElement class is incompatible (failed cast).";
        PROFILES_LOG.errorv (errorString.c_str(), profileIdBuffer, sourceElementIdBuffer);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_InvalidGeometry (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile geometry is invalid.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_InvalidRange_Not2d (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile is not 2D.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_InvalidRange_ZNon0 (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile is elevated above XY plane.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_InvalidArea (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile has no area.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_NotClosed (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile is not closed.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_NotContinious (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile is not of continious perimeter.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_NotSinglePerimeter (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile is not of single perimeter.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_UnhandledShape (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile's shape is not supported.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_ShapeIsNotRegion (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile's shape is not a region type.";
        PROFILES_LOG.errorv (errorString.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     03/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FailedValidate_SelfIntersecting (Utf8CP pProfileClassName)
        {
        Utf8String errorString = "Failed to validate profile " + Utf8String (pProfileClassName) + ", because profile's shape is self intersecting.";
        PROFILES_LOG.errorv (errorString.c_str());
        }
    };
