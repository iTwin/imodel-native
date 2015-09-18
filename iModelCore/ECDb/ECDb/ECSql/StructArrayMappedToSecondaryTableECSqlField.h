/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayMappedToSecondaryTableECSqlField.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include "ECSqlPrepareContext.h"
#include "ECSqlStatementBase.h"
#include "IECSqlPrimitiveValue.h"
#include "EmbeddedECSqlStatement.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct StructArrayMappedToSecondaryTableECSqlField : public ECSqlField, public IECSqlArrayValue
    {
private:
    struct Reader : public IECSqlValue, public IECSqlStructValue
        {
    private:
        StructArrayMappedToSecondaryTableECSqlField* m_parentField;
        ECSqlColumnInfo m_arrayColumnInfo;
        EmbeddedECSqlStatement m_secondaryECSqlStatement;
        int m_currentArrayIndex;
        int m_arrayLength;
        bool m_isAtEnd;
        int m_hiddenMemberStartIndex;

        virtual bool _IsNull () const override;
        virtual ECSqlColumnInfoCR _GetColumnInfo () const override;
        virtual IECSqlPrimitiveValue const& _GetPrimitive () const override;
        virtual IECSqlStructValue const& _GetStruct () const override;
        virtual IECSqlArrayValue const& _GetArray () const override;

        virtual int _GetMemberCount () const override;
        virtual IECSqlValue const& _GetValue (int columnIndex) const override;

        void Reset ();

    public:
        Reader (StructArrayMappedToSecondaryTableECSqlField& parentField, ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCR arrayProperty);
        ECSqlStatus Reset (bool resetLength);
        void SetHiddenMemberStartIndex (int index);
        int GetArrayLength () const;
        void MoveNext (bool onInitializingIterator);
        bool IsAtEnd () const;
        EmbeddedECSqlStatement& GetSecondaryECSqlStatement () { return m_secondaryECSqlStatement; }
        };

private:
    mutable Reader m_reader; 
    ECSqlPrimitiveBinder m_binder;

    //IECSqlValue
    virtual bool _IsNull () const override;
    virtual IECSqlPrimitiveValue const& _GetPrimitive () const override;
    virtual IECSqlStructValue const& _GetStruct () const override;
    virtual IECSqlArrayValue const& _GetArray () const override;

    //IECSqlArrayValue
    virtual void _MoveNext (bool onInitializingIterator) const override;
    virtual bool _IsAtEnd () const override;
    virtual IECSqlValue const* _GetCurrent () const override;
    virtual int _GetArrayLength () const override;

    //ECSqlField
    virtual ECSqlStatus _Reset () override;
    virtual ECSqlStatus _Init () override;

public:
    StructArrayMappedToSecondaryTableECSqlField (ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCR arrayProperty, ECSqlColumnInfo&& parentColumnInfo);

    virtual Collection const&  GetChildren () const override;
    void SetHiddenMemberStartIndex (int index) { m_reader.SetHiddenMemberStartIndex (index); }
    EmbeddedECSqlStatement& GetSecondaryECSqlStatement() { return m_reader.GetSecondaryECSqlStatement();}
    ECSqlPrimitiveBinder& GetBinder() { return m_binder;}

    };
END_BENTLEY_SQLITE_EC_NAMESPACE