/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Exp.h"
#include "../IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OptionExp final : Exp
    {
private:
    Utf8String m_name;
    Utf8String m_val;

    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString() const override { return "OptionExp"; }

public:
    OptionExp(Utf8CP name, Utf8CP val) : Exp(Type::Option), m_name(name), m_val(val) {}

    Utf8CP GetName() const { return m_name.c_str(); }
    bool IsNameValuePair() const { return !m_val.empty(); }
    Utf8CP GetValue() const { return m_val.c_str(); }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OptionsExp final : Exp
    {
public:
    static Utf8CP const NOECCLASSIDFILTER_OPTION;
    static Utf8CP const READONLYPROPERTIESAREUPDATABLE_OPTION;
    static Utf8CP const ENABLE_EXPERIMENTAL_FEATURES;

private:
    bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_optionsByName;

    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString() const override { return "OptionsExp"; }

public:
    OptionsExp() :Exp(Type::Options) {}
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
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
