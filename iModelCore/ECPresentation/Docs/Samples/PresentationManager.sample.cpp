/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ PresentationManagerSample_Include.sampleCode
// the required header file for working with rules-driven presentation manager
#include <ECPresentation/ECPresentationManager.h>

// the required namespace for ECPresentation
USING_NAMESPACE_BENTLEY_ECPRESENTATION
//__PUBLISH_EXTRACT_END__

#define PLATFORM_ASSETS_DIRECTORY BeFileName(L"")
#define TEMPORARY_DIRECTORY BeFileName(L"")
#define RULESETS_DIRECTORY ""
static ECPresentationManager* m_manager = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IConnectionManager* GetConnectionsManager() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IJsonLocalState* GetLocalState() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IECPropertyFormatter* GetECPropertyFormatter() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IECInstanceChangeHandlerPtr CreateECInstanceChangeHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static ECInstanceChangeEventSourcePtr CreateECInstanceChangeEventSource() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IUpdateRecordsHandler* GetUpdateRecordsHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void setup()
    {
    //__PUBLISH_EXTRACT_START__ PresentationManagerSample_SetUp.sampleCode
    // Create the manager
    ECPresentationManager::Paths paths(PLATFORM_ASSETS_DIRECTORY, TEMPORARY_DIRECTORY);
    m_manager = new ECPresentationManager(*GetConnectionsManager(), paths);

    // Register the manager so it can be accessed statically
    IECPresentationManager::RegisterImplementation(m_manager);

    // (optional) Register local state to be used for storing persistent settings. Settings aren't
    // persisted if local state is not set.
    m_manager->SetLocalState(GetLocalState());

    // (optional) Register a property formatter responsible for formatting ECInstance property values. Property values don't
    // get formatted if the formatter is not registered
    m_manager->SetECPropertyFormatter(GetECPropertyFormatter());

    // Register ruleset locaters
    m_manager->GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(RULESETS_DIRECTORY));
    //__PUBLISH_EXTRACT_END__

    //__PUBLISH_EXTRACT_START__ PresentationManagerSample_SetUp_UpdateRelated.sampleCode
    // Register change handlers which are responsible for making ECInstance changes
    m_manager->RegisterECInstanceChangeHandler(*CreateECInstanceChangeHandler());

    // Register change event sources which trigger update events
    m_manager->RegisterECInstanceChangeEventSource(*CreateECInstanceChangeEventSource());

    // Set update records handler which applies view model changes to the UI
    m_manager->RegisterUpdateRecordsHandler(*GetUpdateRecordsHandler());
    //__PUBLISH_EXTRACT_END__
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void cleanup()
    {
    //__PUBLISH_EXTRACT_START__ PresentationManagerSample_CleanUp.sampleCode
    // Destroy and unregister the manager
    DELETE_AND_CLEAR(m_manager);
    IECPresentationManager::RegisterImplementation(nullptr);
    //__PUBLISH_EXTRACT_END__
    }
