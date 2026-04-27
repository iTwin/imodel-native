/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <cmath>
#include <ECDb/IECSqlValue.h>
#include <ECDb/ECSqlColumnInfo.h>
#include <BeSQLite/BeSQLite.h>
#include "ECSql/JsonECSqlValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Base class for all IECSqlValue implementations that serve changeset rows.
//! Holds ECSqlColumnInfo and delegates unimplemented accessors to NoopECSqlValue so
//! subclasses only override what is meaningful for their property type.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueBase : IECSqlValue {
protected:
    ECSqlColumnInfo m_columnInfo;

    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }

    // All defaults delegate to NoopECSqlValue::GetSingleton() -- implemented in the .cpp.
    bool         _IsNull() const override;
    void const*  _GetBlob(int* blobSize) const override;
    bool         _GetBoolean() const override;
    double       _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    uint64_t     _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double       _GetDouble() const override;
    int          _GetInt() const override;
    int64_t      _GetInt64() const override;
    Utf8CP       _GetText() const override;
    DPoint2d     _GetPoint2d() const override;
    DPoint3d     _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    IECSqlValue const&         _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    int                        _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;

    explicit ChangesetValueBase(ECSqlColumnInfo const& colInfo);
    ~ChangesetValueBase() {}
};

//=======================================================================================
//! IECSqlValue for a single-column property (primitive, system, blob, array, etc.).
//! Stores the DbValue directly; callers are responsible for ensuring pointer validity
//! for the lifetime of this object.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetPrimitiveValue final : ChangesetValueBase {
private:
    DbValue        m_value;
    DateTime::Info m_datetimeInfo;

    bool         _IsNull() const override;
    void const*  _GetBlob(int* blobSize) const override;
    bool         _GetBoolean() const override;
    double       _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    uint64_t     _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double       _GetDouble() const override;
    int          _GetInt() const override;
    int64_t      _GetInt64() const override;
    Utf8CP       _GetText() const override;
    IGeometryPtr _GetGeometry() const override;

public:
    ChangesetPrimitiveValue(ECSqlColumnInfo const& colInfo, DbValue const& value, DateTime::Info const& dtInfo = DateTime::Info());
    ~ChangesetPrimitiveValue() {}
};

//=======================================================================================
//! IECSqlValue for a virtual class-id column whose value is fixed at mapping time
//! (e.g. RelECClassId when the relationship has only one concrete class).
//! Mirrors ClassIdECSqlField but implements IECSqlValue instead of ECSqlField.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetFixedInt64Value final : ChangesetValueBase {
private:
    BeInt64Id          m_id;
    mutable Utf8String m_idStr;

    bool    _IsNull()  const override { return !m_id.IsValid(); }
    int64_t _GetInt64() const override { return (int64_t)m_id.GetValueUnchecked(); }
    Utf8CP  _GetText() const override;

public:
    //! @p id may be any BeInt64Id-derived type (ECClassId, ECInstanceId, …).
    ChangesetFixedInt64Value(ECSqlColumnInfo const& colInfo, BeInt64Id const& id);
    ~ChangesetFixedInt64Value() {}
};

//=======================================================================================
//! IECSqlValue for a Point2d property. Coordinates are extracted from the supplied
//! DbValues immediately at construction; pointers need not survive beyond this call.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetPoint2dValue final : ChangesetValueBase {
private:
    DPoint2d m_point;
    bool     m_isNull { true };

    bool     _IsNull()    const override { return m_isNull; }
    DPoint2d _GetPoint2d() const override { return m_point; }

public:
    //! Constructs from raw coordinate values; used when coordinates are fetched from the live DB.
    ChangesetPoint2dValue(ECSqlColumnInfo const& colInfo, double x, double y);
    ~ChangesetPoint2dValue() {}
};

//=======================================================================================
//! IECSqlValue for a Point3d property. Coordinates are extracted from the supplied
//! DbValues immediately at construction; pointers need not survive beyond this call.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetPoint3dValue final : ChangesetValueBase {
private:
    DPoint3d m_point;
    bool     m_isNull { true };

    bool     _IsNull()    const override { return m_isNull; }
    DPoint3d _GetPoint3d() const override { return m_point; }

public:
    //! Constructs from raw coordinate values; used when coordinates are fetched from the live DB.
    ChangesetPoint3dValue(ECSqlColumnInfo const& colInfo, double x, double y, double z);
    ~ChangesetPoint3dValue() {}
};

//=======================================================================================
//! IECSqlValue for an array (or struct-array) property.
//! Parses the JSON stored in the SQLite column at construction and delegates all
//! accessors to JsonECSqlValue, mirroring ArrayECSqlField exactly.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetArrayValue final : ChangesetValueBase {
private:
    mutable rapidjson::Document m_json;
    std::unique_ptr<JsonECSqlValue> m_jsonValue;

    bool         _IsNull() const override;
    void const*  _GetBlob(int* blobSize) const override;
    bool         _GetBoolean() const override;
    double       _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    uint64_t     _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double       _GetDouble() const override;
    int          _GetInt() const override;
    int64_t      _GetInt64() const override;
    Utf8CP       _GetText() const override;
    DPoint2d     _GetPoint2d() const override;
    DPoint3d     _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    IECSqlValue const&         _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    int                        _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;

public:
    //! Parses the JSON text from @p value immediately; @p value need not survive beyond this call.
    ChangesetArrayValue(ECSqlColumnInfo const& colInfo, DbValue const& value, ECDbCR ecdb);
    ~ChangesetArrayValue() {}
};

//=======================================================================================
//! IECSqlValue for a struct property. Implements IECSqlValueIterable so callers can
//! iterate over its member values.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetStructValue final : ChangesetValueBase, IECSqlValueIterable {
private:
    //=======================================================================================
    // @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct IteratorState final : IECSqlValueIterable::IIteratorState {
    private:
        mutable size_t             m_idx;
        ChangesetStructValue const& m_owner;

        IteratorState(IteratorState const& rhs) : m_idx(rhs.m_idx), m_owner(rhs.m_owner) {}

        std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
        void              _MoveToNext(bool onInitializing) const override { if (!onInitializing) ++m_idx; }
        bool              _IsAtEnd()    const override { return m_idx >= m_owner.m_members.size(); }
        IECSqlValue const& _GetCurrent() const override { return *m_owner.m_members[m_idx]; }

    public:
        explicit IteratorState(ChangesetStructValue const& owner) : m_idx(0), m_owner(owner) {}
    };

    std::vector<std::unique_ptr<IECSqlValue>> m_members;
    std::vector<Utf8String>                   m_names;   //!< parallel to m_members

    bool                       _IsNull() const override;
    IECSqlValue const&         _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable()   const override { return *this; }
    const_iterator             _CreateIterator()       const override { return const_iterator(std::unique_ptr<IIteratorState>(new IteratorState(*this))); }

public:
    explicit ChangesetStructValue(ECSqlColumnInfo const& colInfo);
    void AppendMember(Utf8StringCR name, std::unique_ptr<IECSqlValue> member);
    ~ChangesetStructValue() {}
};

//=======================================================================================
//! IECSqlValue for a navigation property. Exposes id and relClassId as struct members
//! and supports iteration over those two sub-values.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetNavValue final : ChangesetValueBase, IECSqlValueIterable {
private:
    //=======================================================================================
    // @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct IteratorState final : IECSqlValueIterable::IIteratorState {
    private:
        enum class State : uint8_t { Start = 0, Id = 1, RelClassId = 2, End = 3 };

        mutable State           m_state { State::Start };
        ChangesetNavValue const& m_owner;

        IteratorState(IteratorState const& rhs) : m_state(rhs.m_state), m_owner(rhs.m_owner) {}

        std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
        void              _MoveToNext(bool /*onInitializing*/) const override { m_state = (State)((uint8_t)m_state + 1); }
        bool              _IsAtEnd()    const override { return m_state == State::End; }
        IECSqlValue const& _GetCurrent() const override;

    public:
        explicit IteratorState(ChangesetNavValue const& owner) : m_owner(owner) {}
    };

    std::unique_ptr<IECSqlValue> m_id;
    std::unique_ptr<IECSqlValue> m_relClassId;

    bool                       _IsNull() const override { return m_id == nullptr || m_id->IsNull(); }
    IECSqlValue const&         _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable()   const override { return *this; }
    const_iterator             _CreateIterator()       const override { return const_iterator(std::unique_ptr<IIteratorState>(new IteratorState(*this))); }

public:
    ChangesetNavValue(ECSqlColumnInfo const& colInfo, std::unique_ptr<IECSqlValue> id, std::unique_ptr<IECSqlValue> relClassId);
    ~ChangesetNavValue() {}
};

END_BENTLEY_SQLITE_EC_NAMESPACE