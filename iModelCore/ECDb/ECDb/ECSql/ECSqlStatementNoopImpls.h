/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementNoopImpls.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include "ECSqlField.h"
#include "IECSqlPrimitiveValue.h"
#include "IECSqlPrimitiveBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//**************** NoopECSqlBinder **************************************
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    05/2013
//+===============+===============+===============+===============+===============+======
struct NoopECSqlBinder : public IECSqlBinder, IECSqlPrimitiveBinder, IECSqlStructBinder, IECSqlArrayBinder
    {
private:
    ECSqlStatus m_errorStatus;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override
        {
        return *this;
        }
    virtual IECSqlStructBinder& _BindStruct() override
        {
        return *this;
        }

    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity)
        {
        return *this;
        }
    
    virtual ECSqlStatus _BindNull() override
        {
        return m_errorStatus;
        }

    // IECSqlPrimitiveBinder
    virtual ECSqlStatus _BindBoolean(bool value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindDateTime(uint64_t julianDayTicksHns, DateTime::Info const* metadata) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const* metadata) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindDouble(double value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindGeometryBlob(void const* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindInt(int value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindInt64(int64_t value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override
        {
        return m_errorStatus;
        }

    virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override
        {
        return m_errorStatus;
        }

    // IECSqlStructBinder
    virtual IECSqlBinder& _GetMember(Utf8CP structMemberPropertyName) override
        {
        return *this;
        }

    virtual IECSqlBinder& _GetMember(ECN::ECPropertyId structMemberPropertyId) override
        {
        return *this;
        }

    // IECSqlArrayBinder
    virtual IECSqlBinder& _AddArrayElement() override
        {
        return *this;
        }


public:
    NoopECSqlBinder(ECSqlStatus errorStatus)
        : m_errorStatus(errorStatus)
        {}

    IECSqlPrimitiveBinder& BindPrimitive()
        {
        return _BindPrimitive();
        }

    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    05/2013
//+===============+===============+===============+===============+===============+======
struct NoopECSqlBinderFactory : NonCopyableClass
    {
private:
    static std::map<ECSqlStatus::Status, std::unique_ptr<NoopECSqlBinder>> s_flyweightBinderMap;
    //static class
    NoopECSqlBinderFactory();
    ~NoopECSqlBinderFactory();

public:
    static NoopECSqlBinder& GetBinder(ECSqlStatus status);
    };


//**************** NoopECSqlValue **************************************
//=======================================================================================
//! No-op implementation for IECSqlValue
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct NoopECSqlValue : public IECSqlValue, IECSqlPrimitiveValue, IECSqlStructValue, IECSqlArrayValue
    {
private:
    ECSqlColumnInfo m_dummyColumnInfo;

    static NoopECSqlValue s_singleton;

    NoopECSqlValue();
    ~NoopECSqlValue();

    //IECSqlValue
    virtual ECSqlColumnInfoCR _GetColumnInfo() const override
        {
        return m_dummyColumnInfo;
        }

    virtual bool _IsNull() const override
        {
        return true;
        }

    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override
        {
        return *this;
        };

    virtual IECSqlStructValue const& _GetStruct() const override
        {
        return *this;
        }

    virtual IECSqlArrayValue const& _GetArray() const override
        {
        return *this;
        }

    //IECSqlPrimitiveValue
    virtual void const* _GetBinary(int* binarySize) const override
        {
        if (binarySize != nullptr)
            *binarySize = -1;

        return nullptr;
        }

    virtual bool _GetBoolean() const override
        {
        return false;
        }

    virtual uint64_t _GetDateTimeJulianDaysHns(DateTime::Info& metadata) const override
        {
        return 0ULL;
        }

    virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override
        {
        return 0.0;
        }

    virtual double _GetDouble() const override
        {
        return 0.0;
        }

    virtual int _GetInt() const override
        {
        return 0;
        }

    virtual int64_t _GetInt64() const override
        {
        return 0LL;
        }

    virtual Utf8CP _GetText() const override
        {
        return nullptr;
        }

    virtual DPoint2d _GetPoint2D () const override
        {
        return DPoint2d::From(0.0, 0.0);
        }

    virtual DPoint3d _GetPoint3D () const override
        {
        return DPoint3d::From(0.0, 0.0, 0.0);
        }

    virtual IGeometryPtr _GetGeometry() const override
        {
        return nullptr;
        }

    virtual void const* _GetGeometryBlob(int* blobSize) const override
        {
        if (blobSize != nullptr)
            *blobSize = -1;

        return nullptr;
        }

    //IECSqlStructValue
    virtual int _GetMemberCount() const override
        {
        return -1;
        }

    virtual IECSqlValue const& _GetValue(int structMemberIndex) const override
        {
        return *this;
        }

    //IECSqlArrayValue
    virtual int _GetArrayLength() const override
        {
        return -1;
        }

    virtual void _MoveNext(bool onInitializingIterator) const override
        {
        }

    virtual bool _IsAtEnd() const override
        {
        return true;
        }

    virtual IECSqlValue const* _GetCurrent() const override
        {
        return nullptr;
        }

public:
    IECSqlPrimitiveValue const& GetPrimitive() const
        {
        return _GetPrimitive();
        }

    static NoopECSqlValue const& GetSingleton()
        {
        return s_singleton;
        }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
