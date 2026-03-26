/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! Replaces a virtual ClassId/SourceClassId/TargetClassId with a constant ClassId field.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassIdECSqlField : ECSqlField {
    private:
        ECN::ECClassId m_classId;
        mutable Utf8String m_idStr;
    private:
        virtual bool _IsNull() const override { return !m_classId.IsValid(); }
        virtual void const* _GetBlob(int* blobSize) const override { return NoopECSqlValue::GetSingleton().GetBlob(blobSize);}
        virtual bool _GetBoolean() const override { return NoopECSqlValue::GetSingleton().GetBoolean();}
        virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);}
        virtual uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);}
        virtual double _GetDouble() const override { return NoopECSqlValue::GetSingleton().GetDouble();}
        virtual int _GetInt() const override { return NoopECSqlValue::GetSingleton().GetInt();}
        virtual IGeometryPtr _GetGeometry() const override { return NoopECSqlValue::GetSingleton().GetGeometry();}
        virtual DPoint2d _GetPoint2d() const override { return NoopECSqlValue::GetSingleton().GetPoint2d();}
        virtual DPoint3d _GetPoint3d() const override { return NoopECSqlValue::GetSingleton().GetPoint3d();}
        virtual IECSqlValue const& _GetStructMemberValue(Utf8CP structMemberName) const  override { return NoopECSqlValue::GetSingleton();}
        virtual IECSqlValueIterable const& _GetStructIterable() const  override { return NoopECSqlValue::GetSingleton().GetStructIterable();}
        virtual int _GetArrayLength() const override  { return NoopECSqlValue::GetSingleton().GetArrayLength();}
        virtual IECSqlValueIterable const& _GetArrayIterable() const override { return NoopECSqlValue::GetSingleton().GetArrayIterable();}
        virtual Utf8CP _GetText() const override {
            if (m_idStr.empty()) {
                m_idStr = m_classId.ToHexStr();
            }
            return m_idStr.c_str();
        }
        virtual int64_t _GetInt64() const override {
            return static_cast<int64_t>(m_classId.GetValueUnchecked());
        }

    public:
        //! Constructor for Statement-based path (InstanceReader)
        ClassIdECSqlField(ECSqlSelectPreparedStatement& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECClassId classId)
            :ECSqlField(ecsqlStatement.GetECDb(), ecsqlStatement.GetSqliteStatement(), ecsqlColumnInfo, false, false), m_classId(classId){}

        //! Constructor for Change-based path (changeset reader)
        ClassIdECSqlField(ECDbCR ecdb, Changes::Change const& change, Changes::Change::Stage const& stage, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECClassId classId)
            :ECSqlField(ecdb, change, stage, ecsqlColumnInfo, false, false), m_classId(classId){}
};

END_BENTLEY_SQLITE_EC_NAMESPACE
