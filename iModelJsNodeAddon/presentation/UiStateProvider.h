/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECPresentation/IUiStateProvider.h>
#include "ECPresentationUtils.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUiStateProvider : IUiStateProvider, IConnectionsListener
{
private:
    mutable BeMutex m_mutex;
    ECPresentationManager const* m_manager;
    bmap<bpair<IConnectionCP, Utf8String>, std::unique_ptr<UiState>> m_uiState;

private:
    UiState& GetUiState(IConnectionCR, Utf8StringCR);

protected:
    void _OnConnectionEvent(ConnectionEvent const&) override;
    std::shared_ptr<UiState> _GetUiState(IConnectionCR, Utf8StringCR) const override;
    bvector<RulesetUiState> _GetUiState(IConnectionCR) const override;
    bvector<ConnectionUiState> _GetUiState(Utf8StringCR) const override;

public:
    IModelJsECPresentationUiStateProvider() : m_manager(nullptr) {}
    ~IModelJsECPresentationUiStateProvider();
    ECPresentationResult UpdateHierarchyState(ECPresentationManager& manager, ECDbCR, Utf8StringCR rulesetId, BeJsConst stateChanges);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
