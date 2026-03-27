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
        virtual bool _IsNull() const override;
        virtual void const* _GetBlob(int* blobSize) const override;
        virtual bool _GetBoolean() const override;
        virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
        virtual uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
        virtual double _GetDouble() const override;
        virtual int _GetInt() const override;
        virtual IGeometryPtr _GetGeometry() const override;
        virtual DPoint2d _GetPoint2d() const override;
        virtual DPoint3d _GetPoint3d() const override;
        virtual IECSqlValue const& _GetStructMemberValue(Utf8CP structMemberName) const override;
        virtual IECSqlValueIterable const& _GetStructIterable() const override;
        virtual int _GetArrayLength() const override;
        virtual IECSqlValueIterable const& _GetArrayIterable() const override;
        virtual Utf8CP _GetText() const override;
        virtual int64_t _GetInt64() const override;

    public:
        //! Constructor for Statement-based path (InstanceReader)
        ClassIdECSqlField(ECSqlSelectPreparedStatement& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECClassId classId)
            :ECSqlField(ecsqlStatement.GetECDb(), ecsqlStatement.GetSqliteStatement(), ecsqlColumnInfo, false, false), m_classId(classId){}

        //! Constructor for Change-based path (changeset reader)
        ClassIdECSqlField(ECDbCR ecdb, Changes::Change const& change, Changes::Change::Stage const& stage, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECClassId classId)
            :ECSqlField(ecdb, change, stage, ecsqlColumnInfo, false, false), m_classId(classId){}
};

END_BENTLEY_SQLITE_EC_NAMESPACE
