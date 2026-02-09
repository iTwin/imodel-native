/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include "Exp.h"
#include "ECSqlTypeInfo.h"
#include "../IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OptionExp final : Exp {
   private:
    Utf8String m_name;
    Utf8String m_val;
    ECSqlTypeInfo m_valType;
    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "OptionExp"; }

   public:
    OptionExp(Utf8CP name, Utf8CP val, ECSqlTypeInfo valType) : Exp(Type::Option), m_name(name), m_val(val), m_valType(valType) {}

    Utf8CP GetName() const { return m_name.c_str(); }
    bool IsNameValuePair() const { return !m_val.empty(); }
    Utf8CP GetValue() const { return m_val.c_str(); }
    ECSqlTypeInfo const& GetValType() const { return m_valType; }
    bool asBool() const;
};

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OptionsExp final : Exp {
    using OptionMap = bmap<Utf8CP, size_t, CompareIUtf8Ascii>;

   public:
    static Utf8CP constexpr NOECCLASSIDFILTER_OPTION = "NoECClassIdFilter";
    static Utf8CP constexpr READONLYPROPERTIESAREUPDATABLE_OPTION = "ReadonlyPropertiesAreUpdatable";
    static Utf8CP constexpr ENABLE_EXPERIMENTAL_FEATURES = "ENABLE_EXPERIMENTAL_FEATURES";
    static Utf8CP constexpr USE_JS_PROP_NAMES = "USE_JS_PROP_NAMES";
    static Utf8CP constexpr DO_NOT_TRUNCATE_BLOB = "DO_NOT_TRUNCATE_BLOB";

   private:
    OptionMap m_optionsByName;

    void _ToECSql(ECSqlRenderContext&) const override;
    void _ToJson(BeJsValue, JsonFormat const&) const override;
    Utf8String _ToString() const override { return "OptionsExp"; }

   public:
    OptionsExp() : Exp(Type::Options) {}
    OptionMap const& GetOptionMap() const { return m_optionsByName; }
    BentleyStatus AddOptionExp(std::unique_ptr<OptionExp> optionExp, IssueDataSource const&);
    //! Checks whether an option with the given name was defined.
    //! If it exists and if it has a value, the value is checked for truth.
    //! Ex: ECSQLOPTIONS opt1 opt2=True opt3=1 opt4=False opt5=0
    //! HasOption(opt1) returns true
    //! HasOption(opt2) returns true
    //! HasOption(opt3) returns true
    //! HasOption(opt4) returns false
    //! HasOption(opt5) returns false
    //! HasOption(blabla) returns false
    //! Options are case-insensitive
    bool HasOption(Utf8CP optionName) const;

    //! Options are case-insensitive
    bool TryGetOption(OptionExp const*&, Utf8CP optionName) const;

    //! Find option
    static OptionExp const* FindLocalOrInheritedOption(Utf8CP optionName, ExpCR exp);
    template <typename T>
    static T FindLocalOrInheritedOption(ExpCR exp, Utf8CP optionName, std::function<T(OptionExp const&)> foundCallback, std::function<T()> notFoundCallback) {
        if (auto opt = FindLocalOrInheritedOption(optionName, exp))
            return foundCallback(*opt);
        return notFoundCallback();
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
