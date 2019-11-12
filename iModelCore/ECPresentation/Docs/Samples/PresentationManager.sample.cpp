/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ PresentationManagerSample_Include.sampleCode
// the required header file for working with rules-driven presentation manager
#include <ECPresentation/RulesDriven/PresentationManager.h>

// the required namespace for ECPresentation
USING_NAMESPACE_BENTLEY_ECPRESENTATION
//__PUBLISH_EXTRACT_END__

#define PLATFORM_ASSETS_DIRECTORY BeFileName(L"")
#define TEMPORARY_DIRECTORY BeFileName(L"")
#define RULESETS_DIRECTORY ""
static RulesDrivenECPresentationManager* m_manager = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                11/2017
//---------------------------------------------------------------------------------------
static IConnectionManager* GetConnectionsManager() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static IJsonLocalState* GetLocalState() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static IECPropertyFormatter* GetECPropertyFormatter() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static ILocalizationProvider* GetLocalizationProvider() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static IECInstanceChangeHandlerPtr CreateECInstanceChangeHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static ECInstanceChangeEventSourcePtr CreateECInstanceChangeEventSource() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static IUpdateRecordsHandler* GetUpdateRecordsHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
void setup()
    {
    //__PUBLISH_EXTRACT_START__ PresentationManagerSample_SetUp.sampleCode
    // Create the manager 
    RulesDrivenECPresentationManager::Paths paths(PLATFORM_ASSETS_DIRECTORY, TEMPORARY_DIRECTORY);
    m_manager = new RulesDrivenECPresentationManager(*GetConnectionsManager(), paths);

    // Register the manager so it can be accessed statically
    IECPresentationManager::RegisterImplementation(m_manager);

    // (optional) Register local state to be used for storing persistent settings. Settings aren't
    // persisted if local state is not set.
    m_manager->SetLocalState(GetLocalState());

    // (optional) Register a property formatter responsible for formatting ECInstance property values. Property values don't
    // get formatted if the formatter is not registered
    m_manager->SetECPropertyFormatter(GetECPropertyFormatter());

    // (optional) Register localization provider which knows how to find both ECPresentation and app's
    // localized strings. By default BeSQLite's L10N API is used - it can only find ECPresentation's
    // strings.
    m_manager->SetLocalizationProvider(GetLocalizationProvider());

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
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
void cleanup()
    {
    //__PUBLISH_EXTRACT_START__ PresentationManagerSample_CleanUp.sampleCode
    // Destroy and unregister the manager
    DELETE_AND_CLEAR(m_manager);
    IECPresentationManager::RegisterImplementation(nullptr);
    //__PUBLISH_EXTRACT_END__
    }