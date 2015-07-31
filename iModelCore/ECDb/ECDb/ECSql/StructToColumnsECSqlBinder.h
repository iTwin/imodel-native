/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructToColumnsECSqlBinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct StructToColumnsECSqlBinder : public ECSqlBinder, IECSqlStructBinder
    {
private:
    //=======================================================================================
    //! @bsiclass                                                Krischan.Eberle      03/2014
    //+===============+===============+===============+===============+===============+======
    struct MemberBinderInfo
        {
    private:
        ECSqlBinder* m_memberBinder;
        int m_ecsqlComponentIndexOffset;

    public:
        MemberBinderInfo (ECSqlBinder& memberBinder, int ecsqlComponentIndexOffset)
            : m_memberBinder (&memberBinder), m_ecsqlComponentIndexOffset (ecsqlComponentIndexOffset)
            {}
        //use compiler generated dtor/copy ctor/copy assignment op

        ECSqlBinder& GetMemberBinder () const { return *m_memberBinder; }
        //! Gets the component index of the parent struct binder that maps to the first component
        //! of this member's binder.
        int GetECSqlComponentIndexOffset () const { return m_ecsqlComponentIndexOffset; }
        };

    std::map<ECN::ECPropertyId, std::unique_ptr<ECSqlBinder>> m_memberBinders;
    std::vector<MemberBinderInfo> m_ecsqlComponentIndexToMemberBinderMapping;

    //only needed at prepare time to set up the binder
    virtual void _SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
    virtual void _OnClearBindings () override;
    virtual ECSqlStatus _OnBeforeStep () override;

    //these are needed by the actual binding API
    virtual IECSqlBinder& _GetMember (Utf8CP structMemberPropertyName) override;
    virtual IECSqlBinder& _GetMember (ECN::ECPropertyId structMemberPropertyId) override;
    virtual ECSqlStatus _BindNull () override;
    virtual IECSqlPrimitiveBinder& _BindPrimitive () override;
    virtual IECSqlStructBinder& _BindStruct () override;
    virtual IECSqlArrayBinder& _BindArray (uint32_t initialCapacity) override;

    void Initialize ();
public:
    StructToColumnsECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo);

    ~StructToColumnsECSqlBinder () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
