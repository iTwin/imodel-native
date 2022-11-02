/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ UnifiedSelectionSample_Include.sampleCode
// the required header file for working with rules-driven presentation manager
#include <ECPresentation/SelectionManager.h>

// the required namespace for ECPresentation
USING_NAMESPACE_BENTLEY_ECPRESENTATION
//__PUBLISH_EXTRACT_END__


static SelectionManager* m_manager = nullptr;
static ISelectionChangesListener* m_selectionChangesListener = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static IConnectionCache* GetConnectionsCache() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static SelectionSyncHandlerPtr CreateMySelectionChangesSyncHandler() {return nullptr;}

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void cleanup()
    {
    //__PUBLISH_EXTRACT_START__ UnifiedSelectionSample_CleanUp.sampleCode
    // Destroy the selection manager
    DELETE_AND_CLEAR(m_manager);
    //__PUBLISH_EXTRACT_END__
    }