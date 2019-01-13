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

    };
