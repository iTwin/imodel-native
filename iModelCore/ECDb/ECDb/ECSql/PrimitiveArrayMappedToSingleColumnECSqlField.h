/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayMappedToSingleColumnECSqlField.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include "IECSqlPrimitiveValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayMappedToSingleColumnECSqlField : public ECSqlField, public IECSqlArrayValue
    {
private:
    struct ArrayElementValue : public IECSqlValue, IECSqlPrimitiveValue
        {
    private:
        ECN::ECValue m_value;
        ECSqlColumnInfo m_columnInfo;
        ECSqlStatusContext& m_statusContext;

    private:
        //IECSqlValue
        virtual bool _IsNull () const override;
        virtual ECSqlColumnInfoCR _GetColumnInfo () const override;
        virtual IECSqlPrimitiveValue const& _GetPrimitive () const override { return *this; }
        virtual IECSqlStructValue const& _GetStruct () const override;
        virtual IECSqlArrayValue const& _GetArray () const override;

        //IECSqlPrimitiveValue
        virtual void const* _GetBinary (int* binarySize) const override;
        virtual bool _GetBoolean () const override;
        virtual uint64_t _GetDateTimeJulianDays (DateTime::Info& metadata) const override;
        virtual double _GetDouble () const override;
        virtual int _GetInt () const override;
        virtual int64_t _GetInt64 () const override;
        virtual Utf8CP _GetText () const override;
        virtual DPoint2d _GetPoint2D () const override;
        virtual DPoint3d _GetPoint3D () const override;
        virtual IGeometryPtr _GetGeometry() const override;
        virtual void const* _GetGeometryBlob(int* blobSize) const override;

        bool CanRead (ECN::PrimitiveType requestedType) const;
        void ResetStatus () const;

    public:
        explicit ArrayElementValue (ECSqlStatusContext& statusContext);
        void Init (ECSqlColumnInfoCR parentColumnInfo);

        BentleyStatus SetValue (ECN::IECInstanceCR instance, uint32_t arrayIndex, DateTime::Info const& dateTimeMetadata);
        void Reset ();
        };

private:
    ECN::ECClassCR m_primitiveArraySystemClass;
    int m_sqliteColumnIndex;
    ECN::IECInstancePtr m_arrayValueECInstance;
    ECN::ArrayInfo m_arrayInfo;
    DateTime::Info m_datetimeMetadata;

    mutable int m_currentArrayIndex;
    mutable ArrayElementValue m_arrayElement; 

    //IECSqlValue
    virtual bool _IsNull () const override;
    virtual IECSqlPrimitiveValue const& _GetPrimitive () const override;
    virtual IECSqlStructValue const& _GetStruct () const override;
    virtual IECSqlArrayValue const& _GetArray () const override { return *this; }

    //IECSqlArrayValue
    virtual void _MoveNext (bool onInitializingIterator) const override;
    virtual bool _IsAtEnd () const override;
    virtual IECSqlValue const* _GetCurrent () const override;
    virtual int _GetArrayLength () const override;

    //ECSqlField
    virtual ECSqlStatus _Reset (ECSqlStatusContext& statusContext) override;
    virtual ECSqlStatus _Init (ECSqlStatusContext& statusContext) override;
    
    void DoReset () const;
    ECN::IECInstanceCP GetArrayValueECInstance () const;

public:
    PrimitiveArrayMappedToSingleColumnECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int sqliteColumnIndex, ECN::ECClassCR primitiveArraySystemClass);
    ~PrimitiveArrayMappedToSingleColumnECSqlField () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
