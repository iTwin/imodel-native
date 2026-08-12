/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/SchemaManager.h>
#include <functional>
#include <optional>
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Write-side counterpart of PropertyReader. Wraps the IECSqlBinder for a single root
//! property of the instance being written together with the ECProperty it belongs to.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyBinderRef final {
    using Finder = std::function<std::optional<PropertyBinderRef>(Utf8CP)>;

private:
    ECN::ECPropertyCP m_property;
    IECSqlBinder* m_binder;

public:
    PropertyBinderRef(ECN::ECPropertyCR prop, IECSqlBinder& binder) : m_property(&prop), m_binder(&binder) {}
    PropertyBinderRef(PropertyBinderRef const&) = default;
    PropertyBinderRef& operator=(PropertyBinderRef const&) = default;
    ECN::ECPropertyCR GetProperty() const { return *m_property; }
    IECSqlBinder& GetBinder() const { return *m_binder; }
};

//=======================================================================================
//! @internal High speed bulk instance writer.
//!
//! This is the write-side mirror of InstanceReader. Like InstanceReader it does not
//! prepare ECSQL. Instead it generates and prepares one raw SQLite statement per table
//! of the class map and creates the binders directly from the class map's property maps.
//! Values are bound through IECSqlBinder, exactly like InstanceReader exposes values
//! through IECSqlValue.
//!
//! Update supports partial updates at the granularity of a <b>hierarchy level</b>. A level is
//! the ECClass that declares a property, so the properties of Foo : Goo : Base partition into
//! a Base, a Goo and a Foo segment, each backed by its own UPDATE statement. Only the levels
//! the caller actually wrote are executed, so an update that touches derived properties never
//! rewrites the base columns and never maintains indexes over them. Level statements are
//! shared between sibling classes, because an inherited property resolves to the same table
//! and the same column in every subclass of a table per hierarchy mapping.
//!
//! A mixin is never a level of its own. A mixin is an interface rather than a storage class:
//! its properties are merged into the entity class that implements it and are mapped by that
//! class, so they group with the implementing class's own local properties.
//!
//! @warning Partiality is level granular, not property granular. Requesting a binder for any
//! property of a level executes that level's statement, which writes <b>all</b> of that level's
//! columns. Properties of a written level that the caller did not bind are set to NULL. Callers
//! must therefore supply a whole level or none of it.
//!
//! Remarks:
//!  - Requesting a binder marks the property, and therefore its level, as written. Requesting a
//!    binder but never calling any Bind* method on it is equivalent to calling BindNull().
//!  - Entity classes and link table relationship classes are supported. Foreign key
//!    (end table) relationship classes are not supported.
//!  - The property identified by the ClassHasCurrentTimeStampProperty custom attribute is
//!    never written. It is owned by the database trigger.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct BulkInstanceWriter final {
public:
    enum class WriterOp {
        Insert,
        Update,
    };

    //=======================================================================================
    //! Handed to the caller's bind callback. Provides access to the binders of the root
    //! properties of the class being written.
    //+===============+===============+===============+===============+===============+======
    struct IBindContext {
    protected:
        virtual IECSqlBinder& _GetBinder(int propertyIndex) const = 0;
        virtual IECSqlBinder* _FindBinder(Utf8CP propertyName) const = 0;
        virtual int _GetPropertyCount() const = 0;
        virtual int _GetPropertyIndex(Utf8CP propertyName) const = 0;
        virtual ECN::ECPropertyCP _GetProperty(int propertyIndex) const = 0;
        virtual int _GetLevelCount() const = 0;
        virtual ECN::ECClassCP _GetLevelClass(int levelIndex) const = 0;
        virtual int _GetPropertyLevel(int propertyIndex) const = 0;

    public:
        virtual ~IBindContext() {}

        //! Number of root properties of the class (including system properties).
        int GetPropertyCount() const { return _GetPropertyCount(); }
        //! Index of a root property or -1 if it does not exist. Lookup is case insensitive.
        //! Looking up an index does not mark the property as written.
        int GetPropertyIndex(Utf8CP propertyName) const { return _GetPropertyIndex(propertyName); }
        //! ECProperty at the given index or nullptr if the index is out of range.
        ECN::ECPropertyCP GetProperty(int propertyIndex) const { return _GetProperty(propertyIndex); }
        //! Binder for the root property at the given index. Marks the property as written.
        //! Returns a no-op binder if the index is out of range.
        IECSqlBinder& GetBinder(int propertyIndex) const { return _GetBinder(propertyIndex); }
        //! Binder for the named root property. Marks the property as written.
        //! Returns nullptr if the property does not exist. Lookup is case insensitive.
        IECSqlBinder* FindBinder(Utf8CP propertyName) const { return _FindBinder(propertyName); }
        //! Number of hierarchy levels the class's writable properties partition into.
        //! Levels are ordered root -> leaf. Only meaningful during an update.
        int GetLevelCount() const { return _GetLevelCount(); }
        //! The ECClass that declares the properties of the given level, or nullptr if the
        //! index is out of range.
        ECN::ECClassCP GetLevelClass(int levelIndex) const { return _GetLevelClass(levelIndex); }
        //! Level of a root property, or -1 if the index is out of range. Writing any property
        //! of a level writes every property of that level, so this tells a caller exactly
        //! which other properties it has to supply.
        int GetPropertyLevel(int propertyIndex) const { return _GetPropertyLevel(propertyIndex); }
        //! Convenience finder with the same shape as InstanceReader's PropertyReader::Finder.
        std::optional<PropertyBinderRef> Find(Utf8CP propertyName) const {
            auto binder = _FindBinder(propertyName);
            if (binder == nullptr)
                return std::nullopt;

            const auto index = _GetPropertyIndex(propertyName);
            auto prop = _GetProperty(index);
            if (prop == nullptr)
                return std::nullopt;

            return PropertyBinderRef(*prop, *binder);
        }
    };

    using BindCallback = std::function<void(IBindContext const&)>;

    //=======================================================================================
    //+===============+===============+===============+===============+===============+======
    struct Options {
    private:
        WriterOp m_op;

    protected:
        explicit Options(WriterOp op) : m_op(op) {}

    public:
        WriterOp GetOp() const { return m_op; }
        bool IsInsert() const { return m_op == WriterOp::Insert; }
        bool IsUpdate() const { return m_op == WriterOp::Update; }
    };

    //=======================================================================================
    //+===============+===============+===============+===============+===============+======
    struct InsertOptions final : Options {
        enum class InstanceIdMode {
            Auto,   //!< ECDb generates the ECInstanceId from its instance id sequence
            Manual, //!< use the ECInstanceId set through UseInstanceId()
        };

    private:
        ECInstanceId m_instanceId;
        InstanceIdMode m_instanceIdMode = InstanceIdMode::Auto;

    public:
        InsertOptions() : Options(WriterOp::Insert) {}
        ECInstanceId GetInstanceId() const { return m_instanceId; }
        InstanceIdMode GetInstanceIdMode() const { return m_instanceIdMode; }
        InsertOptions& UseInstanceId(ECInstanceId id) {
            m_instanceId = id;
            m_instanceIdMode = InstanceIdMode::Manual;
            return *this;
        }
        InsertOptions& UseAutoECInstanceId() {
            m_instanceId = ECInstanceId();
            m_instanceIdMode = InstanceIdMode::Auto;
            return *this;
        }
    };

    //=======================================================================================
    //+===============+===============+===============+===============+===============+======
    struct UpdateOptions final : Options {
        enum class UpdateMode {
            //!< only the hierarchy levels the callback requested a binder in are written. Every
            //!< column of a written level is written, every column of an untouched level keeps
            //!< its current value
            Partial,
            //!< every level of the class is written. The callback is expected to supply the
            //!< full instance: properties it does not bind are set to NULL
            Full,
        };

    private:
        bool m_failIfNoRowChanged = false;
        UpdateMode m_mode = UpdateMode::Partial;
        bool m_forceAllLevels = false;
        std::set<ECN::ECClassId> m_forcedLevels;

    public:
        UpdateOptions() : Options(WriterOp::Update) {}
        bool GetFailIfNoRowChanged() const { return m_failIfNoRowChanged; }
        //! If set, Update() returns BE_SQLITE_NOTFOUND when the instance does not exist.
        UpdateOptions& FailIfNoRowChanged(bool v) {
            m_failIfNoRowChanged = v;
            return *this;
        }

        UpdateMode GetUpdateMode() const { return m_mode; }
        bool IsPartialUpdate() const { return m_mode == UpdateMode::Partial; }
        bool IsFullUpdate() const { return m_mode == UpdateMode::Full; }
        //! Writes only the levels the callback binds into. This is the default.
        UpdateOptions& UsePartialUpdate() {
            m_mode = UpdateMode::Partial;
            return *this;
        }
        //! Writes every level of the class. The callback must supply the full instance, any
        //! property it does not bind is set to NULL.
        UpdateOptions& UseFullUpdate() {
            m_mode = UpdateMode::Full;
            return *this;
        }

        bool GetForceAllLevels() const { return m_forceAllLevels; }
        //! Executes every level's statement even when the callback wrote nothing into it. The
        //! properties of an untouched level are set to NULL, so this is only meaningful when
        //! the callback supplies them. Off by default. Its purpose is to fire database triggers
        //! (for example the ClassHasCurrentTimeStampProperty trigger, whose property the writer
        //! never writes itself) on a level that would otherwise be skipped.
        UpdateOptions& ForceLevels() {
            m_forceAllLevels = true;
            return *this;
        }
        //! Same as ForceLevels() but restricted to the level declared by the given ECClass.
        UpdateOptions& ForceLevel(ECN::ECClassId levelClassId) {
            if (levelClassId.IsValid())
                m_forcedLevels.insert(levelClassId);

            return *this;
        }
        bool IsLevelForced(ECN::ECClassId levelClassId) const {
            return m_forceAllLevels || m_forcedLevels.find(levelClassId) != m_forcedLevels.end();
        }
    };

    struct Impl;

private:
    Impl* m_pImpl;

public:
    BulkInstanceWriter(BulkInstanceWriter const&) = delete;
    BulkInstanceWriter& operator=(BulkInstanceWriter const&) = delete;
    ECDB_EXPORT explicit BulkInstanceWriter(ECDbCR ecdb, uint32_t cacheSize = 40);
    ECDB_EXPORT ~BulkInstanceWriter();

    //! Inserts a new instance of the given class. The callback is invoked to bind the values.
    ECDB_EXPORT DbResult Insert(ECN::ECClassId classId, BindCallback callback, InsertOptions const& options, ECInstanceKey& key);
    ECDB_EXPORT DbResult Insert(ECN::ECClassId classId, BindCallback callback, InsertOptions const& options);
    //! Updates an existing instance. In the default partial mode only the hierarchy levels the
    //! callback requested a binder in are written; every column of a written level is written and
    //! every column of an untouched level keeps its current value. In full mode
    //! (UpdateOptions::UseFullUpdate) every level is written and the callback must supply the
    //! full instance, any property it does not bind is set to NULL.
    //! @note Partiality is level granular. Binding one property of a level writes all of that
    //! level's columns, so properties of that level the callback did not bind become NULL. Use
    //! IBindContext::GetPropertyLevel to discover which properties belong together.
    //! @note The bind callback is always invoked exactly once. UPDATE statements no longer depend
    //! on the set of written properties, so no discovery pass is needed.
    ECDB_EXPORT DbResult Update(ECInstanceKeyCR key, BindCallback callback, UpdateOptions const& options);
    ECDB_EXPORT DbResult Update(ECInstanceKeyCR key, BindCallback callback);

    ECDB_EXPORT Utf8StringCR GetLastError() const;
    ECDB_EXPORT void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
