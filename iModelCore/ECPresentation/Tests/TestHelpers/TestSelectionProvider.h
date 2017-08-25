/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestSelectionProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/SelectionManager.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct TestSelectionProvider : ISelectionProvider
{
private:
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_selections;
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_subSelections;
protected:
    INavNodeKeysContainerCPtr _GetSelection(ECDbR connection) const override
        {
        auto iter = m_selections.find(&connection);
        return (m_selections.end() != iter) ? iter->second : nullptr;
        }
    INavNodeKeysContainerCPtr _GetSubSelection(ECDbR connection) const override
        {
        auto iter = m_subSelections.find(&connection);
        return (m_subSelections.end() != iter) ? iter->second : nullptr;
        }
public:
    void SetSelection(ECDbCR connection, INavNodeKeysContainerCR selection) {m_selections[&connection] = &selection;}
    void SetSubSelection(ECDbCR connection, INavNodeKeysContainerCR selection) {m_subSelections[&connection] = &selection;}
};

END_ECPRESENTATIONTESTS_NAMESPACE