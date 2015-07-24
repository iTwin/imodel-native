/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ClassRefExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
DEFINE_EXPR_TYPE(ClassRef) 
private:
    virtual Utf8String _ToString () const override { return "ClassRef"; }

protected:
    ClassRefExp () : Exp () {}
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
    DEFINE_EXPR_TYPE(RangeClassRef) 
private:
    Utf8String m_alias;
    bool m_isPolymorphic;
    virtual Utf8StringCR _GetId() const = 0;
    virtual bool _ContainProperty(Utf8CP propertyName) const = 0;
    virtual ECSqlStatus _CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const = 0;

protected:
    RangeClassRefExp () : ClassRefExp (), m_isPolymorphic(true) {}
    explicit RangeClassRefExp (bool isPolymorphic) : ClassRefExp (), m_isPolymorphic(isPolymorphic) {}

public:
    virtual ~RangeClassRefExp () {}

    Utf8StringCR GetId() const { return _GetId();}
    Utf8StringCR GetAlias() const { return m_alias;}
    bool IsPolymorphic() const { return m_isPolymorphic;}

    ECSqlStatus CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const;

    bool ContainProperty(Utf8CP propertyName) const;
    
    void SetAlias (Utf8StringCR alias) { m_alias = alias;}
   };

typedef std::vector<RangeClassRefExp const*> RangeClassRefList;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ClassNameExp : RangeClassRefExp
    {
friend struct ECSqlParser;
DEFINE_EXPR_TYPE(ClassName) 

public:
    //=======================================================================================
    //! @bsiclass                                                Affan.Khan      05/2013
    //+===============+===============+===============+===============+===============+======
    struct Info
        {
    private:
        IClassMap const& m_classMap;

    public:
        explicit Info (IClassMap const& classMap) : m_classMap(classMap) {}

        IClassMap const& GetMap () const { return m_classMap; }
        static std::shared_ptr<Info> Create(IClassMap const& classMap) { return std::make_shared<Info>(classMap); }
        };

private:
    Utf8String m_className;
    Utf8String m_schemaPrefix;
    Utf8String m_catalogName;
    std::shared_ptr<Info> m_info;

    virtual Utf8StringCR _GetId() const override
        {
        if (GetAlias().empty())
            return m_className;

        return GetAlias();
        }

    virtual bool _ContainProperty(Utf8CP propertyName) const override
        {
        PRECONDITION(m_info != nullptr, false);
        auto propertyMap = m_info->GetMap().GetPropertyMap(WString(propertyName, BentleyCharEncoding::Utf8).c_str());
        return propertyMap != nullptr;
        }

    virtual ECSqlStatus _CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const override;

    virtual Utf8String _ToECSql() const override
        {
        Utf8String tmp;
        if (!IsPolymorphic())
            tmp.append("ONLY ");

        tmp.append(GetFullName());

        if (!GetAlias().empty())
            tmp.append(" ").append(GetAlias());
        return tmp;
        }

    virtual Utf8String _ToString () const override
        {
        Utf8String str ("ClassName [Catalog: ");
        str.append (m_catalogName.c_str ()).append (", Schema prefix: ").append (m_schemaPrefix.c_str ());
        str.append (", Class: ").append (m_className).append (", Alias: ").append (GetAlias ()).append ("]");
        return str;
        }

public:
    ClassNameExp(Utf8CP className, Utf8CP schemaPrefix, Utf8CP catalog, std::shared_ptr<Info> info, bool isPolymorphic = true)
        : RangeClassRefExp(isPolymorphic), m_className(className), m_schemaPrefix(schemaPrefix), m_catalogName(catalog), m_info(info)
        {}

    bool HasMetaInfo() const { return m_info != nullptr;}
    ClassNameExp::Info const&  GetInfo() const { return *m_info;}

    Utf8String GetFullName () const
        {
        Utf8String fullName;
        if (!m_catalogName.empty ())
            fullName.append (m_catalogName).append (".");

        if (!m_schemaPrefix.empty ())
            fullName.append (m_schemaPrefix).append (".");

        fullName.append (m_className);

        return std::move (fullName);
        }

    Utf8StringCR GetClassName() const { return m_className;}
    Utf8StringCR GetSchemaName() const { return m_schemaPrefix;}
    Utf8StringCR GetCatalogName() const { return m_catalogName;}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE