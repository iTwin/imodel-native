/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayToColumnECSqlBinder.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"
#include "IECSqlPrimitiveBinder.h"
#include "PrimitiveToSingleColumnECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayToColumnECSqlBinder : public ECSqlBinder, IECSqlArrayBinder
    {
private:
    //=======================================================================================
    //! @bsiclass                                                Affan.Khan      01/2014
    //+===============+===============+===============+===============+===============+======
    struct ArrayElementBinder : IECSqlBinder, IECSqlPrimitiveBinder
        {
    private:
        UInt32 m_arrayElementIndex;
        UInt32 m_arrayPropertyIndex;
        IECInstanceP m_instance;
        ECSqlTypeInfo const& m_arrayTypeInfo;
        ECSqlStatusContext& m_statusContext;

        virtual IECSqlPrimitiveBinder& _BindPrimitive () override;
        virtual IECSqlStructBinder& _BindStruct () override;
        virtual IECSqlArrayBinder& _BindArray (UInt32 initialCapacity) override;
        virtual ECSqlStatus _BindNull () override;

        virtual ECSqlStatus _BindBoolean (bool value) override;
        virtual ECSqlStatus _BindBinary (const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
        virtual ECSqlStatus _BindDateTime (UInt64 julianDayTicksHns, DateTime::Info const* metadata) override;
        virtual ECSqlStatus _BindDouble (double value) override;
        virtual ECSqlStatus _BindGeometryBlob (const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
        virtual ECSqlStatus _BindInt (int value) override;
        virtual ECSqlStatus _BindInt64 (Int64 value) override;
        virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override;
        virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override;
        virtual ECSqlStatus _BindText (Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
        virtual ECSqlStatus _BindId (BeRepositoryBasedId value) override;

        ECSqlStatus VerifyType (PrimitiveType type) const;
        ECSqlStatus SetValue (ECValueCR value);

    public:
        ArrayElementBinder (ECSqlStatusContext& statusContext, ECSqlTypeInfo const& arrayTypeInfo, UInt32 arrayPropertyIndex);
        ~ArrayElementBinder () {}
        void Initialize (UInt32 arrayElementIndex, IECInstanceR instance);
        };

private:
    const UInt32 ARRAY_PROPERTY_INDEX = 1;

    mutable StandaloneECInstancePtr m_instance;
    mutable ArrayElementBinder m_arrayElementBinder;
    mutable int m_currentArrayIndex;
    ECClassCP m_arrayStorageClass;
    UInt32 m_initialCapacity;
    int m_sqliteIndex;

    virtual void _SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
    virtual void _OnClearBindings () override;
    virtual ECSqlStatus _OnBeforeStep () override;

    virtual IECSqlBinder& _AddArrayElement () override;
    virtual ECSqlStatus _BindNull () override;
    virtual IECSqlPrimitiveBinder& _BindPrimitive () override;
    virtual IECSqlStructBinder& _BindStruct () override;
    virtual IECSqlArrayBinder& _BindArray (UInt32 initialCapacity) override;

    UInt32 GetCurrentArrayLength () const { return (UInt32) (m_currentArrayIndex + 1); }
    StandaloneECInstanceP GetInstance (bool create) const;

public:
    PrimitiveArrayToColumnECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo);
    ~PrimitiveArrayToColumnECSqlBinder () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
