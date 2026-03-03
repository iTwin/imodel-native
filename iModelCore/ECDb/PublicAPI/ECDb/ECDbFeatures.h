/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Lifecycle state of a feature in the built-in registry.
//!
//! A feature progresses through a defined lifecycle:
//!
//!   Experimental → Stable → Deprecated
//!
//! @see ECDbFeatureDescriptor, ECDbFeatureRegistry
// @bsienum
//+===============+===============+===============+===============+===============+======
enum class ECDbFeatureStatus
    {
    //! Under active development. Must be explicitly opted into. May be dropped or changed
    //! without notice. Not suitable for production iModels.
    Experimental = 0,

    //! Production-ready and rolled out across the ecosystem. Features promoted to Stable
    //! will not be removed without a full deprecation cycle.
    Stable = 1,

    //! Was Stable, but is being retired. iModels using this feature will receive warnings
    //! with migration guidance during the deprecation period.
    Deprecated = 2,
    };

//=======================================================================================
//! Descriptor for a single well-known feature in the built-in registry.
//!
//! @see ECDbFeatureRegistry
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbFeatureDescriptor final
    {
private:
    Utf8String m_name;
    Utf8String m_label;
    Utf8String m_description;
    ECDbFeatureStatus m_status;
    bool m_enabledByDefault;  //!< auto-enabled on new iModels when Stable
    bool m_toggleable;        //!< DisableFeature is legal at runtime
    bool m_ephemeral;         //!< session-scoped; enabling does NOT persist to file

public:
    ECDbFeatureDescriptor() = delete;

    ECDbFeatureDescriptor(Utf8CP name, Utf8CP label, Utf8CP description,
                          ECDbFeatureStatus status, bool enabledByDefault,
                          bool toggleable, bool ephemeral)
        : m_name(name), m_label(label), m_description(description),
          m_status(status), m_enabledByDefault(enabledByDefault),
          m_toggleable(toggleable), m_ephemeral(ephemeral) {}

    //! Unique string identifier, e.g. "strict-schema-loading".
    Utf8StringCR GetName()        const { return m_name; }
    //! Short human-readable label, e.g. "Strict Schema Loading".
    Utf8StringCR GetLabel()       const { return m_label; }
    //! Longer description suitable for error messages and tooling UI.
    Utf8StringCR GetDescription() const { return m_description; }
    //! Lifecycle status of this feature.
    ECDbFeatureStatus GetStatus() const { return m_status; }
    //! True if this Stable feature is auto-enabled on newly created iModels.
    bool IsEnabledByDefault()     const { return m_enabledByDefault; }
    //! True if DisableFeature() is a legal runtime call for this feature.
    bool IsToggleable()           const { return m_toggleable; }
    //! True if enabling this feature is session-scoped and NOT persisted to the file.
    bool IsEphemeral()            const { return m_ephemeral; }

    //! Returns a C-string representation of status, e.g. "Experimental".
    ECDB_EXPORT static Utf8CP StatusToString(ECDbFeatureStatus status);
    };

//=======================================================================================
//! Process-wide registry of all well-known features shipped with this ECDb release.
//!
//! The registry is read-only after process startup.
//! Use ECDbFeatureRegistry::GetInstance() to access the singleton.
//!
//! @see ECDbFeatureDescriptor, ECDbFeatureSet
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbFeatureRegistry final
    {
private:
    std::vector<ECDbFeatureDescriptor> m_features;

    ECDbFeatureRegistry() { RegisterBuiltIns(); }

    ECDbFeatureRegistry(ECDbFeatureRegistry const&) = delete;
    ECDbFeatureRegistry& operator=(ECDbFeatureRegistry const&) = delete;

    void RegisterBuiltIns();

public:
    // -----------------------------------------------------------------------
    //! Well-known feature name constants.
    // -----------------------------------------------------------------------

    //! Strict Schema Loading (Experimental)
    //!
    //! When enabled, the ECSchema XML parser runs in strict mode:
    //!   - Unknown XML attributes on known elements     -> error (currently silently ignored)
    //!   - Unknown XML elements in known positions      -> error (currently silently ignored)
    //!   - Unrecognised enum values in typed attributes -> error (currently inconsistent)
    ECDB_EXPORT static Utf8CP STRICT_SCHEMA_LOADING;

    //! Returns the singleton process-wide registry instance.
    ECDB_EXPORT static ECDbFeatureRegistry const& GetInstance();

    //! Returns all registered feature descriptors in registration order.
    std::vector<ECDbFeatureDescriptor> const& GetAll() const { return m_features; }

    //! Finds a descriptor by name (case-insensitive). Returns nullptr if not found.
    ECDB_EXPORT ECDbFeatureDescriptor const* Find(Utf8CP name) const;

    //! Returns true if @p name matches a registered feature (any status).
    bool Contains(Utf8CP name) const { return Find(name) != nullptr; }
    };

//=======================================================================================
//! The set of feature names enabled on a specific iModel file.
//!
//! @see ECDb::GetEnabledFeatures, ECDb::EnableFeature
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbFeatureSet final
    {
private:
    bset<Utf8String, CompareIUtf8Ascii> m_enabled;

public:
    ECDbFeatureSet() = default;
    ECDbFeatureSet(ECDbFeatureSet const&) = default;
    ECDbFeatureSet& operator=(ECDbFeatureSet const&) = default;

    //! Returns true if the named feature is in this set (case-insensitive).
    bool IsEnabled(Utf8CP name) const { return m_enabled.find(name) != m_enabled.end(); }

    //! Returns the underlying ordered, case-insensitive set.
    bset<Utf8String, CompareIUtf8Ascii> const& GetAll() const { return m_enabled; }

    //! Number of features in this set.
    size_t Count() const { return m_enabled.size(); }

    //! @private Adds a feature name. Does NOT persist to file.
    void Add(Utf8StringCR name)    { m_enabled.insert(name); }
    //! @private Removes a feature name.
    void Remove(Utf8StringCR name) { m_enabled.erase(name); }
    //! @private Clears all entries.
    void Clear()                   { m_enabled.clear(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
