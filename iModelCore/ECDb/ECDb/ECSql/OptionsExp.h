/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/OptionsExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle 10/2015
//+===============+===============+===============+===============+===============+======
struct OptionExp : Exp
    {
DEFINE_EXPR_TYPE(Option)

private:
    Utf8String m_name;
    Utf8String m_val;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "OptionExp"; }

public:
    explicit OptionExp(Utf8CP name, Utf8CP val) : Exp(), m_name(name), m_val(val) {}

    Utf8CP GetName() const { return m_name.c_str(); }
    bool IsNameValuePair() const { return !m_val.empty(); }
    Utf8CP GetValue() const { return m_val.c_str(); }
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle 10/2015
//+===============+===============+===============+===============+===============+======
struct OptionsExp : Exp
    {
    DEFINE_EXPR_TYPE(Options)

    static Utf8CP const NOECCLASSIDFILTER_OPTION;

private:
    bmap<Utf8CP, size_t, CompareIUtf8> m_optionsByName;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "OptionsExp"; }

public:
    OptionsExp() :Exp() {}
    BentleyStatus AddOptionExp(std::unique_ptr<OptionExp> optionExp);
    //! Options are case-insensitive
    bool TryGetOption(OptionExp const*&, Utf8CP optionName) const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
