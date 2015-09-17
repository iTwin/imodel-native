/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParameterValue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include "IECSqlPrimitiveBinder.h"
#include "IECSqlPrimitiveValue.h"
#include "ECSqlTypeInfo.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct ECSqlParameterValue;
struct ArrayECSqlParameterValue;

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlParameterValueFactory
    {
private:
    ECSqlParameterValueFactory();
    ~ECSqlParameterValueFactory();

public:
    static std::unique_ptr<ECSqlParameterValue> Create(ECDbCR, ECSqlTypeInfo const&);
    static std::unique_ptr<ECSqlParameterValue> CreateStruct(ECDbCR, ECSqlTypeInfo const&);
    static std::unique_ptr<ECSqlParameterValue> CreatePrimitive(ECDbCR, ECSqlTypeInfo const&);
    static std::unique_ptr<ArrayECSqlParameterValue> CreateArray(ECDbCR, ECSqlTypeInfo const&);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlParameterValue : public IECSqlBinder, public IECSqlValue
    {
private:
    ECDbCR m_ecdb;
    ECSqlTypeInfo const& m_typeInfo;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

    virtual ECSqlColumnInfoCR _GetColumnInfo() const override;

    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual IECSqlArrayValue const& _GetArray() const override;

    virtual void _Clear() = 0;

    static ECSqlStatus BindTo(ECSqlParameterValue const& from, IECSqlBinder& to);

protected:
    ECSqlParameterValue(ECDbCR, ECSqlTypeInfo const& typeInfo);

    ECSqlTypeInfo const& GetTypeInfo() const { return m_typeInfo; }
    ECDbCR GetECDb() const { return m_ecdb; }

public:
    virtual ~ECSqlParameterValue() {}


    IECSqlPrimitiveBinder& BindPrimitive() const;

    ECSqlStatus BindTo(IECSqlBinder& targetBinder) const;

    void Clear();
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct PrimitiveECSqlParameterValue : ECSqlParameterValue, public IECSqlPrimitiveBinder, public IECSqlPrimitiveValue
    {
private:
    ECN::ECValue m_value;

    virtual ECSqlStatus _BindNull() override;

    virtual ECSqlStatus _BindBoolean(bool value) override;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDouble(double value) override;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindInt(int value) override;
    virtual ECSqlStatus _BindInt64(int64_t value) override;
    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override;
    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override;
    virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;

    virtual bool _IsNull() const override;

    virtual void const* _GetBinary(int* binarySize) const override;
    virtual bool _GetBoolean() const override;
    virtual uint64_t _GetDateTimeJulianDaysHns(DateTime::Info& metadata) const override;
    virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    virtual double _GetDouble() const override;
    virtual int _GetInt() const override;
    virtual int64_t _GetInt64() const override;
    virtual Utf8CP _GetText() const override;
    virtual DPoint2d _GetPoint2D () const override;
    virtual DPoint3d _GetPoint3D () const override;
    virtual IGeometryPtr _GetGeometry() const override;
    virtual void const* _GetGeometryBlob(int* blobSize) const override;
    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;

    virtual void _Clear() override;
    void DoClear();

    bool CanBindValue(ECN::PrimitiveType actualType) const;

public:
    PrimitiveECSqlParameterValue(ECDbCR, ECSqlTypeInfo const& typeInfo);
    virtual ~PrimitiveECSqlParameterValue() {}
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct StructECSqlParameterValue : ECSqlParameterValue, public IECSqlStructBinder, public IECSqlStructValue
    {
private:
    std::map<ECN::ECPropertyId, std::unique_ptr<ECSqlParameterValue>> m_propertyValueMap;

    virtual ECSqlStatus _BindNull() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlBinder& _GetMember(Utf8CP structMemberPropertyName) override;
    virtual IECSqlBinder& _GetMember(ECN::ECPropertyId structMemberPropertyId) override;

    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual bool _IsNull() const override;

    virtual int _GetMemberCount() const override;
    virtual IECSqlValue const& _GetValue(int columnIndex) const override;

    virtual void _Clear() override;
    void DoClear();

public:
    StructECSqlParameterValue(ECDbCR, ECSqlTypeInfo const& typeInfo);
    virtual ~StructECSqlParameterValue() {}

    IECSqlValue const& GetValue(ECN::ECPropertyId structMemberPropertyId) const;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct ArrayECSqlParameterValue : ECSqlParameterValue, public IECSqlArrayBinder, public IECSqlArrayValue
    {
private:
    //=======================================================================================
    //! Collection for the array elements of the parameter value.
    //! Reuses already allocated array element values after a call to ClearBindings.
    // @bsiclass                                                 Krischan.Eberle    08/2014
    //+===============+===============+===============+===============+===============+======
    struct Collection
        {
    public:
        //=======================================================================================
        //! Iterator for Collection
        // @bsiclass                                                 Krischan.Eberle    08/2014
        //+===============+===============+===============+===============+===============+======
        struct const_iterator : std::iterator<std::forward_iterator_tag, ECSqlParameterValue const*>
            {
        private:
            Collection const* m_collection;
            size_t m_maxIndex;
            size_t m_currentIndex;
        
        public:
            const_iterator(Collection const& coll, bool isEndIterator);

            const_iterator(const_iterator const& rhs);
            const_iterator& operator= (const_iterator const& rhs);

            ECSqlParameterValue const* operator* () const;
            const_iterator& operator++ ();
            bool operator== (const_iterator const& rhs) const;
            bool operator!= (const_iterator const& rhs) const;
            };

    private:
        std::vector<std::unique_ptr<ECSqlParameterValue>> m_collection;
        mutable size_t m_size;

    public:
        Collection();

        ECSqlParameterValue* operator[] (size_t index) const;

        //! Indicates whether an already allocated parameter value can be reused or
        //! a new one needs to be appended.
        bool HasNextAllocated() const;
        //! Appends a new parameter value element and increases the capacity by one.
        void Append(std::unique_ptr<ECSqlParameterValue>& value);
        //! Gets the next already allocated parameter value
        ECSqlParameterValue* GetNextAllocated() const;

        size_t Size() const;
        size_t Capacity() const;
        void Reserve(size_t count);
        void Clear();

        const_iterator begin() const;
        const_iterator end() const;
        };

    mutable Collection m_arrayElementValues;
    mutable Collection::const_iterator m_iterator;

    virtual ECSqlStatus _BindNull() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;
    virtual IECSqlBinder& _AddArrayElement() override;

    virtual bool _IsNull() const override;
    virtual void _MoveNext(bool onInitializingIterator = false) const override;
    virtual bool _IsAtEnd() const override;
    virtual IECSqlValue const* _GetCurrent() const override;
    virtual int _GetArrayLength() const override;

    virtual IECSqlArrayValue const& _GetArray() const override { return *this; }

    virtual void _Clear() override;
    void DoClear();

public:
    ArrayECSqlParameterValue(ECDbCR, ECSqlTypeInfo const& typeInfo);
    virtual ~ArrayECSqlParameterValue() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

