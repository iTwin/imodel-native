/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECDbFeatures.h>
#include "ProfileManager.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Manages the feature state for a single ECDb connection.
//!
//! Responsibilities:
//!   - Load the ECDbFeatureSet from be_prop when a file is opened.
//!   - Persist the ECDbFeatureSet to be_prop when a feature is enabled/disabled.
//!   - Enforce business rules around feature enablement.
//!   - Enable all Stable + enabledByDefault features automatically on new file creation.
//!
//! The ECDbFeatureManager is owned by ECDb::Impl and follows the ECDb connection lifetime.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbFeatureManager final
    {
private:
    ECDbR m_ecdb;
    mutable ECDbFeatureSet m_enabledFeatures;

    static PropertySpec GetPropertySpec();

    //! Loads the feature set from be_prop into m_enabledFeatures.
    //! Does not fail if the property does not yet exist (returns SUCCESS with empty set).
    BentleyStatus LoadFromDb() const;

    //! Persists the current in-memory feature set to be_prop.
    BentleyStatus SaveToDb() const;

public:
    explicit ECDbFeatureManager(ECDbR ecdb) : m_ecdb(ecdb) {}

    //not copyable
    ECDbFeatureManager(ECDbFeatureManager const&) = delete;
    ECDbFeatureManager& operator=(ECDbFeatureManager const&) = delete;

    //! Called when the ECDb profile is first created (new file).
    //! Enables all Stable + enabledByDefault features in the registry.
    BentleyStatus OnProfileCreated() const;

    //! Called when an existing file is opened. Loads features from be_prop.
    BentleyStatus OnDbOpened() const;

    //! Clears the in-memory cache and reloads from be_prop.
    //! Call this after a changeset is applied to the file.
    void Reload() const { m_enabledFeatures.Clear(); LoadFromDb(); }

    //! Called when the file is closed. Resets in-memory state.
    void OnDbClose() { m_enabledFeatures.Clear(); }

    //! Returns the set of features persisted in this file.
    //! Lazily loads from be_prop on first access.
    ECDbFeatureSet const& GetEnabledFeatures() const;

    //! Enables a feature on this file (transactional, requires active write transaction).
    //! Idempotent: returns SUCCESS if already enabled.
    //! @return SUCCESS, or ERROR with a reason logged via the ECDb issue reporter.
    BentleyStatus EnableFeature(Utf8CP name) const;

    //! Disables a feature on this file. Returns ERROR if the feature is not toggleable.
    //! @return SUCCESS, or ERROR with a reason logged via the ECDb issue reporter.
    BentleyStatus DisableFeature(Utf8CP name) const;

    //! Convenience: returns true if the named feature is enabled in this file.
    bool IsEnabled(Utf8CP name) const { return GetEnabledFeatures().IsEnabled(name); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
