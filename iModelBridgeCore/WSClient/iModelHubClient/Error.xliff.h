/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <BeSQLite/L10N.h>
#include <WebServices/iModelHub/Client/Error.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(ErrorL10N, iModelHubError)
    //Missing argument errors
    L10N_STRING(MESSAGE_CredentialsNotSet)                 // =="Credentials were not specified."==
    L10N_STRING(MESSAGE_InvalidServerURL)                  // =="Server URL was not specified."==
    L10N_STRING(MESSAGE_InvalidiModelName)                 // =="iModel name is not specified."==
    L10N_STRING(MESSAGE_InvalidiModelId)                   // =="iModel id is not specified."==
    L10N_STRING(MESSAGE_InvalidiModelConnection)           // =="iModel connection is not specified."==
    L10N_STRING(MESSAGE_InvalidChangeSet)                  // =="ChangeSet id is not specified."==

    //User errors
    L10N_STRING(MESSAGE_UserDoesNotExist)                  // =="User could not be found."==
    //iModel errors
    L10N_STRING(MESSAGE_SeedFileNotFound)                  // =="Could not find uninitialized seed file."==
    L10N_STRING(MESSAGE_iModelIsNotInitialized)            // =="iModel is not initialized."==

    //Initialization errors
    L10N_STRING(MESSAGE_FileIsNotYetInitialized)           // =="File is not yet initialized."==
    L10N_STRING(MESSAGE_FileIsOutdated)                    // =="File could not be initialized. File is too old."==
    L10N_STRING(MESSAGE_FileCodeTooLong)                   // =="File could not be initialized. File has one or more codes exceeding the maximum code length limit."==
    L10N_STRING(MESSAGE_FileInitializationFailed)          // =="File could not be initialized. Unexpected error has occured."==

    //Briefcase errors
    L10N_STRING(MESSAGE_BriefcaseIsReadOnly)               // =="Briefcase is read only."==
    L10N_STRING(MESSAGE_FileNotOpen)                       // =="Briefcase file is not open."==
    L10N_STRING(MESSAGE_FileNotFound)                      // =="Briefcase file could not be found or there was issue opening it."==
    L10N_STRING(MESSAGE_PullIsRequired)                    // =="Pull is required."==
    L10N_STRING(MESSAGE_TrackingNotEnabled)                // =="Change tracking is not enabled on the briefcase."==
    L10N_STRING(MESSAGE_BriefcaseWrongURL)                 // =="Briefcase belongs to another server."==
    L10N_STRING(MESSAGE_FileAlreadyExists)                 // =="Could not acquire briefcase. File with that name already exists."==
    L10N_STRING(MESSAGE_FileIsNotBriefcase)                // =="File is not a valid briefcase."==
    L10N_STRING(MESSAGE_BriefcaseOutdated)                 // =="Briefcase is no longer valid. Seed file has been replaced."==

    //ChangeSet errors
    L10N_STRING(MESSAGE_ApplyError)                        // =="Error applying change set."==
    L10N_STRING(MESSAGE_ChangeSetManagerError)             // =="ChangeSet manager error has occured."==
    L10N_STRING(MESSAGE_MergeSchemaChangesOnOpen)          // =="Briefcase needs to be reopened in order to merge changes from the server."==
    L10N_STRING(MESSAGE_ReverseOrReinstateSchemaChangesOnOpen) // =="Briefcase needs to be reopened in order to reverse or reinstate changes."==
    L10N_STRING(MESSAGE_ChangeSetDoesNotExist)             // =="ChangeSet could not be found."==
    L10N_STRING(MESSAGE_ConflictsAggregate)                // =="Some conflicts occured during request execution."==

    //Code template errors
    L10N_STRING(MESSAGE_CodeSequenceRequestError)          // =="Code sequence request formatting has failed."==
    L10N_STRING(MESSAGE_CodeSequenceResponseError)         // =="Code sequence response parsing has failed."==

    //Event service errors
    L10N_STRING(MESSAGE_EventCallbackAlreadySubscribed)    // =="Event callback is already registered."==
    L10N_STRING(MESSAGE_EventServiceSubscribingError)      // =="Could not subscribe to events."==
    L10N_STRING(MESSAGE_EventCallbackNotFound)             // =="Could not find event callback."==
    L10N_STRING(MESSAGE_EventCallbackNotSpecified)         // =="Event callback is not specified."==
    L10N_STRING(MESSAGE_NoEventsFound)                     // =="No events have been found."==
    L10N_STRING(MESSAGE_NotSubscribedToEventService)       // =="Not subscribed to event service."==
    L10N_STRING(MESSAGE_NoSASFound)                        // =="SAS token could not be parsed from server response."==
    L10N_STRING(MESSAGE_NoSubscriptionFound)               // =="Subscription information could not be parsed from server response."==

    //Default message
    L10N_STRING(MESSAGE_Unknown)                           // =="Unexpected error has occured."==
BENTLEY_TRANSLATABLE_STRINGS_END

#define ErrorLocalizedString(K) ErrorL10N::GetString(ErrorL10N::K())
