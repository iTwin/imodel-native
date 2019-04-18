/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ UnifiedSelectionSample_Include.sampleCode
// the required header file for working with rules-driven presentation manager
#include <ECPresentation/SelectionManager.h>

// the required namespace for ECPresentation
USING_NAMESPACE_BENTLEY_ECPRESENTATION
//__PUBLISH_EXTRACT_END__


static SelectionManager* m_manager = nullptr;
static ISelectionChangesListener* m_selectionChangesListener = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                11/2017
//---------------------------------------------------------------------------------------
static IConnectionCache* GetConnectionsCache() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
static SelectionSyncHandlerPtr CreateMySelectionChangesSyncHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
void setup()
    {
    //__PUBLISH_EXTRACT_START__ UnifiedSelectionSample_SetUp.sampleCode
    // Create the unified selection manager
    m_manager = new SelectionManager(*GetConnectionsCache());

    // Register selection change listeners
    m_manager->AddListener(*m_selectionChangesListener);

    // Register selection change synchronization handlers which will sync selection with other parts 
    // of the app (e.g. UI controls)
    m_manager->AddSyncHandler(*CreateMySelectionChangesSyncHandler());
    //__PUBLISH_EXTRACT_END__
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                08/2017
//---------------------------------------------------------------------------------------
void cleanup()
    {
    //__PUBLISH_EXTRACT_START__ UnifiedSelectionSample_CleanUp.sampleCode
    // Destroy the selection manager
    DELETE_AND_CLEAR(m_manager);
    //__PUBLISH_EXTRACT_END__
    }