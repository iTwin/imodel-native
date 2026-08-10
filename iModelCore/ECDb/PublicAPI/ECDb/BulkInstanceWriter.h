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
//! Update supports partial updates: only the properties for which a binder was requested
//! are written, all other columns retain their current value in the database.
//!
//! Remarks:
//!  - Requesting a binder marks the property as written. Requesting a binder but never
//!    calling any Bind* method on it is equivalent to calling BindNull().
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
            //!< only the properties the callback requested a binder for are written, every other
            //!< column keeps its current value
            Partial,
            //!< every property of the class is written. The callback is expected to supply the
            //!< full instance: properties it does not bind are set to NULL
            Full,
        };

    private:
        bool m_failIfNoRowChanged = false;
        UpdateMode m_mode = UpdateMode::Partial;

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
        //! Writes only the properties the callback binds. This is the default.
        UpdateOptions& UsePartialUpdate() {
            m_mode = UpdateMode::Partial;
            return *this;
        }
        //! Writes every property of the class. The callback must supply the full instance, any
        //! property it does not bind is set to NULL. Never needs a discovery pass, so the
        //! callback is always invoked exactly once.
        UpdateOptions& UseFullUpdate() {
            m_mode = UpdateMode::Full;
            return *this;
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
    //! Updates an existing instance. In the default partial mode only properties for which the
    //! callback requested a binder are written, all other columns keep their current value. In
    //! full mode (UpdateOptions::UseFullUpdate) every property is written and the callback must
    //! supply the full instance, any property it does not bind is set to NULL.
    //! @note Partial UPDATE statements are specialized for the exact set of properties that is
    //! written, so the set has to be known before the values can be bound. The set of the previous
    //! partial update of the same class is used as a guess, which makes the steady state of a bulk
    //! loop a single callback invocation. Whenever the guess is wrong (the first partial update of
    //! a class, or a call that writes a different set than the previous one) the callback is
    //! invoked twice: once to discover the set and once to bind it. Callbacks must therefore be
    //! free of side effects. Full updates always invoke the callback exactly once.
    ECDB_EXPORT DbResult Update(ECInstanceKeyCR key, BindCallback callback, UpdateOptions const& options);
    ECDB_EXPORT DbResult Update(ECInstanceKeyCR key, BindCallback callback);

    ECDB_EXPORT Utf8StringCR GetLastError() const;
    ECDB_EXPORT void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
