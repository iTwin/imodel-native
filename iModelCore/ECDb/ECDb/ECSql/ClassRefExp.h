/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ClassRefExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ClassRefExp : Exp
    {   
protected:
    explicit ClassRefExp (Type type) : Exp (type) {}

public:
    virtual ~ClassRefExp () {}
    };


struct PropertyNameExp;

//********* ClassRefExp subclasses ***************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct RangeClassRefExp : ClassRefExp
    {
private:
    Utf8String m_alias;
    bool m_isPolymorphic;

    virtual Utf8StringCR _GetId() const = 0;
    virtual bool _ContainsProperty(Utf8StringCR propertyName) const = 0;
    virtual BentleyStatus _CreatePropertyNameExpList (ECSqlParseContext const&, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const = 0;

protected:
    explicit RangeClassRefExp (Type type, bool isPolymorphic) : ClassRefExp (type), m_isPolymorphic(isPolymorphic) {}

public:
    virtual ~RangeClassRefExp () {}

    Utf8StringCR GetId() const { return _GetId();}
    Utf8StringCR GetAlias() const { return m_alias;}
    bool IsPolymorphic() const { return m_isPolymorphic;}

    BentleyStatus CreatePropertyNameExpList(ECSqlParseContext const& ctx, std::function<void(std::unique_ptr<PropertyNameExp>&)> addDelegate) const { return _CreatePropertyNameExpList(ctx, addDelegate); }
    bool ContainsProperty(Utf8StringCR propertyName) const { return _ContainsProperty(propertyName); }
    void SetAlias (Utf8StringCR alias) { m_alias = alias;}
   };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct RangeClassInfo final
    {
    enum class Scope
        {
        Local,
        Inherited
        };

    private:
        RangeClassRefExp const* m_exp = nullptr;
        Scope m_scope = Scope::Local;

    public:
        RangeClassInfo() {}
        RangeClassInfo(RangeClassRefExp const& exp, Scope scope) :m_exp(&exp), m_scope(scope) {}

        bool IsValid() const { return m_exp != nullptr; }
        Scope GetScope() const { BeAssert(IsValid()); return m_scope; }
        bool IsLocal() const { BeAssert(IsValid()); return m_scope == Scope::Local; }
        bool IsInherited() const { BeAssert(IsValid()); return m_scope == Scope::Inherited; }
        RangeClassRefExp const& GetExp() const { BeAssert(IsValid()); return *m_exp; }
    };

struct MemberFunctionCallExp;
//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ClassNameExp final : RangeClassRefExp
    {
friend struct ECSqlParser;
public:
    //=======================================================================================
    //! @bsiclass                                                Affan.Khan      05/2013
    //+===============+===============+===============+===============+===============+======
    struct Info
        {
    private:
        ClassMap const& m_classMap;

    public:
        explicit Info (ClassMap const& classMap) : m_classMap(classMap) {}

        ClassMap const& GetMap () const { return m_classMap; }
        static std::shared_ptr<Info> Create(ClassMap const& classMap) { return std::make_shared<Info>(classMap); }
        };

private:
    Utf8String m_className;
    Utf8String m_schemaAlias;
    Utf8String m_catalogName;
    std::shared_ptr<Info> m_info;
    
    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    Utf8StringCR _GetId() const override;
    bool _ContainsProperty(Utf8StringCR propertyName) const override;
    BentleyStatus _CreatePropertyNameExpList(ECSqlParseContext const&, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const override;
    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString () const override;

public:
    ClassNameExp(Utf8StringCR className, Utf8StringCR schemaAlias, Utf8StringCP catalogName, std::shared_ptr<Info> info, bool isPolymorphic = true, std::unique_ptr<MemberFunctionCallExp> memberFuntionCall = nullptr);
    bool HasMetaInfo() const { return m_info != nullptr;}
    ClassNameExp::Info const& GetInfo() const { return *m_info;}

    Utf8String GetFullName() const;
    MemberFunctionCallExp const* GetMemberFunctionCall() const { if (GetChildrenCount() == 1) return GetChildren()[0]->GetAsCP<MemberFunctionCallExp>(); return nullptr; }
    Utf8StringCR GetClassName() const { return m_className;}
    Utf8StringCR GetSchemaName() const { return m_schemaAlias;}
    Utf8StringCR GetCatalogName() const { return m_catalogName;}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE