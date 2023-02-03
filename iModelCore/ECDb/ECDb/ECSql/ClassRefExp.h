/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassRefExp : Exp
    {
protected:
    explicit ClassRefExp (Type type) : Exp (type) {}

public:
    virtual ~ClassRefExp () {}
    };


struct DerivedPropertyExp;
struct RangeClassRefExp;

//********* ClassRefExp subclasses ***************************
//=======================================================================================
//! @bsiclass
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

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyMatchOptions {
    private:
        Utf8String m_alias;
        RangeClassInfo m_rangeInfo;

    public:
        Utf8StringCR GetAlias() const { return m_alias; }
        PropertyMatchOptions &SetAlias(Utf8CP alias) {
            m_alias = alias;
            return *this;
        }
        RangeClassInfo const& GetRangeInfo() const { return m_rangeInfo; }
        PropertyMatchOptions &SetRangeInfo(RangeClassInfo const& info) {
            m_rangeInfo = info;
            return *this;
        }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyMatchResult {
    private:
        PropertyPath m_originalPath;
        PropertyPath m_resolvedPath;
        ECN::ECPropertyCP m_property;
        DerivedPropertyExp const* m_derivedProperty;
        PropertyMap const *m_propertyMap;
        PropertyMatchOptions m_options;
        int m_confidence; /* -ive < 0 < +ive*/
        bool m_columnInCTE;

    public:
        PropertyMatchResult():
            m_derivedProperty(nullptr),m_propertyMap(nullptr),m_confidence(0),m_columnInCTE(false), m_property(nullptr){}
        PropertyMatchResult(PropertyMatchOptions options, PropertyPath const& originalPath, PropertyPath const& resolvedPath, DerivedPropertyExp const& result, int confidence, bool columnInCTE = false)
            :m_originalPath(originalPath), m_resolvedPath(resolvedPath),m_derivedProperty(&result), m_propertyMap(nullptr), m_options(options), m_confidence(confidence), m_columnInCTE(columnInCTE), m_property(nullptr) {}
        PropertyMatchResult(PropertyMatchOptions options,PropertyPath const& originalPath, PropertyPath const& resolvedPath, PropertyMap const& result, int confidence)
            :m_originalPath(originalPath), m_resolvedPath(resolvedPath),m_derivedProperty(nullptr), m_propertyMap(&result), m_options(options), m_confidence(confidence),m_columnInCTE(false), m_property(nullptr){}
        PropertyMatchResult(PropertyMatchOptions options,PropertyPath const& originalPath, PropertyPath const& resolvedPath, ECN::ECPropertyCR result, int confidence)
            :m_originalPath(originalPath), m_resolvedPath(resolvedPath),m_derivedProperty(nullptr), m_propertyMap(nullptr), m_options(options), m_confidence(confidence),m_columnInCTE(false), m_property(&result){}
        bool isValid() const {
            //BeAssert(!m_resolvedPath.IsEmpty() && !m_originalPath.IsEmpty() && (m_derivedProperty != nullptr || m_propertyMap != nullptr || m_property != nullptr));
            return !m_resolvedPath.IsEmpty() && !m_originalPath.IsEmpty() && (m_derivedProperty != nullptr || m_propertyMap != nullptr || m_property != nullptr);
            }
        int Confidence() const { return m_confidence; }
        PropertyPath const& OriginalPath() const { return m_originalPath; }
        PropertyPath const& ResolvedPath() const { return m_resolvedPath; }
        DerivedPropertyExp const *GetDerivedProperty() const { return m_derivedProperty; }
        PropertyMap const *GetPropertyMap() const { return m_propertyMap; }
        ECN::ECPropertyCP GetProperty() const { return m_property; }
        PropertyMatchOptions const &Options() const { return m_options; }
        bool IsDerivedProperty() const { return m_derivedProperty != nullptr; }
        bool IsPropertyMap() const { return m_propertyMap != nullptr; }
        bool IsPropertyDefinition() const { return m_property != nullptr; }
        bool IsResolvedPathDifferent() const { return m_originalPath.ToString() != m_resolvedPath.ToString(); }
        bool IsColumnInCTE() const { return m_columnInCTE; }
        static PropertyMatchResult NotFound() { return PropertyMatchResult(); }
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PolymorphicInfo final
    {
enum class Type
    {
    Only,
    All,
    Default = All
    };

private:
    bool m_disqualify;
    Type m_type;

public:
    PolymorphicInfo(): m_disqualify(false), m_type(Type::Default) {}
    PolymorphicInfo(Type type, bool disqualify): m_disqualify(disqualify), m_type(type) {}
    PolymorphicInfo(PolymorphicInfo&&) = default;
    PolymorphicInfo(PolymorphicInfo const&) = default;
    PolymorphicInfo& operator = (PolymorphicInfo&&) = default;
    PolymorphicInfo& operator = (PolymorphicInfo const&) = default;
    bool IsPolymorphic() const { return m_type != Type::Only; }
    bool IsOnly() const { return !IsPolymorphic(); }
    bool IsDisqualified() const { return m_disqualify; }
    void SetType(Type type) { m_type = type; }
    void SetDisqualified(bool v) { m_disqualify = v; }
    static bool IsOnlyToken(Utf8StringCR str) { return str.EqualsIAscii("ONLY"); }
    static bool IsAllToken(Utf8StringCR str) { return str.EqualsIAscii("ALL"); }
    static bool TryParseToken(Type& type, Utf8StringCR str);
    static PolymorphicInfo Only();
    static PolymorphicInfo All();
    Utf8String ToECSql() const;
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct RangeClassRefExp : ClassRefExp
    {
private:
    Utf8String m_alias;
    PolymorphicInfo m_isPolymorphicConstraint;

    virtual Utf8StringCR _GetId() const = 0;
    virtual void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const = 0;
    virtual PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const = 0;
protected:
    explicit RangeClassRefExp (Type type, PolymorphicInfo polymorphicInfo) : ClassRefExp (type), m_isPolymorphicConstraint(polymorphicInfo) {}

public:
    virtual ~RangeClassRefExp () {}

    Utf8StringCR GetId() const { return _GetId();}
    Utf8StringCR GetAlias() const { return m_alias;}
    PolymorphicInfo const& GetPolymorphicInfo() const { return m_isPolymorphicConstraint;}

    void ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const { _ExpandSelectAsterisk(expandedSelectClauseItemList, ctx); }
    PropertyMatchResult FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const { return _FindProperty(ctx, propertyPath, options); }
    void SetAlias (Utf8StringCR alias) { m_alias = alias;}
   };

struct MemberFunctionCallExp;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassNameExp final : RangeClassRefExp
    {
friend struct ECSqlParser;
public:
    //=======================================================================================
    //! @bsiclass
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
    Utf8String m_tableSpace;
    std::shared_ptr<Info> m_info;
    bool m_disqualifyPrimaryJoin;
    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    Utf8StringCR _GetId() const override;
    void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
    void _ToECSql(ECSqlRenderContext&) const override;
    PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;
    Utf8String _ToString () const override;

public:
    ClassNameExp(Utf8StringCR className, Utf8StringCR schemaAlias, Utf8CP tableSpace, std::shared_ptr<Info> info, PolymorphicInfo polymorphicInfo = PolymorphicInfo(), std::unique_ptr<MemberFunctionCallExp> memberFuntionCall = nullptr, bool disqualifyPrimaryJoin = false);
    bool HasMetaInfo() const { return m_info != nullptr;}
    ClassNameExp::Info const& GetInfo() const { return *m_info;}

    Utf8String GetFullName() const;
    Utf8StringCR GetTableSpace() const { return m_tableSpace; }
    Utf8StringCR GetSchemaName() const { return m_schemaAlias; }
    Utf8StringCR GetClassName() const { return m_className;}
    bool DisqualifyPrimaryJoin() const {return m_disqualifyPrimaryJoin;}
    MemberFunctionCallExp const* GetMemberFunctionCallExp() const
        {
        if (GetChildren().empty())
            return nullptr;

        Exp const* childExp = GetChildren()[0];
        if (childExp->GetType() != Exp::Type::MemberFunctionCall)
            return nullptr;

        return childExp->GetAsCP<MemberFunctionCallExp>();
        }
    };
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct TableValuedFunctionExp : RangeClassRefExp
    {
friend struct ECSqlParser;
private:
    ECN::ECEntityClassCP m_virtualEntityClass;
    Utf8String m_schemaName;
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual Utf8StringCR _GetId() const override;
    virtual void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
    virtual PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const override;
    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString () const override;

public:
    explicit TableValuedFunctionExp (Utf8StringCR schemaName, std::unique_ptr<MemberFunctionCallExp> func, PolymorphicInfo isPolymorphic);
public:
    MemberFunctionCallExp const* GetFunctionExp() const {return GetChild<MemberFunctionCallExp>(0); }
    ECN::ECEntityClassCP GetClass() const { return m_virtualEntityClass; }
    Utf8StringCR GetSchemaName() const { return m_schemaName; }
    virtual ~TableValuedFunctionExp () {}
   };
END_BENTLEY_SQLITE_EC_NAMESPACE