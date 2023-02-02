/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


struct PragmaVal {
private:
    enum class Type {
        Integer,
        Double,
        String,
        Bool,
        Name,
        Null,
        Empty,
    };
    Type m_type;

    std::string m_str;
    union {
        int64_t m_integer;
        double m_double;
        bool m_bool;
    };
    explicit PragmaVal(Type t):m_type(t){}
public:
    PragmaVal():m_type(Type::Empty){}
    PragmaVal(PragmaVal&&) = default;
    PragmaVal(const PragmaVal&) = default;
    PragmaVal& operator =(PragmaVal&&) = default;
    PragmaVal& operator =(const PragmaVal&) = default;
    explicit PragmaVal(int64_t val):m_type(Type::Integer), m_integer(val){}
    explicit PragmaVal(double val):m_type(Type::Double), m_double(val){}
    explicit PragmaVal(bool val):m_type(Type::Bool), m_bool(val){}
    explicit PragmaVal(std::string val, bool isName):m_type(isName?Type::Name:Type::String), m_str(val){}
    bool IsBool() const { return m_type == Type::Bool; }
    bool IsInteger() const { return m_type == Type::Integer; }
    bool IsDouble() const { return m_type == Type::Double; }
    bool IsString() const { return m_type == Type::String; }
    bool IsName() const { return m_type == Type::Name; }
    bool IsEmpty() const { return m_type == Type::Empty; }
    bool IsNull() const { return m_type == Type::Null; }
    bool IsNumeric() const { return IsInteger() || IsDouble(); }
    int64_t GetInteger() const;
    double GetDouble() const;
    bool GetBool() const;
    PragmaVal& operator = (int64_t v) { m_integer = v; m_type = Type::Integer; return *this;}
    PragmaVal& operator = (double v) { m_double = v; m_type = Type::Double; return *this;}
    PragmaVal& operator = (std::string const&v) { m_str = v; m_type = Type::String; return *this;}
    PragmaVal& operator = (bool v) { m_bool = v; m_type = Type::Bool; return *this;}
    PragmaVal& operator = (std::nullptr_t v) { m_str.clear(); m_integer = 0; m_type = Type::Null; return *this;}
    void SetName(std::string const& name) { m_str = name; m_type = Type::Name; }
    std::string GetString() const;
    std::string GetName() const;
    static PragmaVal const& Null();
};
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaStatementExp final : Exp {
private:
    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override { return FinalizeParseStatus::Completed; }
    bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override { return false; }
    void _ToECSql(ECSqlRenderContext& ctx) const override { }
    Utf8String _ToString() const override { return "Pragma"; }

    PragmaVal m_val;
    Utf8String m_name;
    bool m_readValue;
    std::vector<Utf8String> m_pathTokens;

 public:
     PragmaStatementExp(Utf8StringCR name, PragmaVal val, bool readVal, std::vector<Utf8String> pathTokens)
        : Exp(Exp::Type::Pragma), m_name(name), m_val(val), m_readValue(readVal), m_pathTokens(pathTokens){}
     Utf8StringCR GetName() const { return m_name; }
     bool IsReadValue() const { return m_readValue;}
     std::vector<Utf8String> const& GetPathTokens() const { return m_pathTokens; }
     PragmaVal const& GetValue() const { return m_val; }
};


END_BENTLEY_SQLITE_EC_NAMESPACE

