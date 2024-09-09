/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "Exp.h"
#include "ECSqlTypeInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//forward declarations of referenced Exp classes
struct ParameterExp;
struct ValueExp;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ComputedExp : Exp
    {
private:
    friend struct ECSqlParser;

    bool m_hasParentheses;
    ECSqlTypeInfo m_typeInfo;

protected:
    explicit ComputedExp(Type type) : Exp(type), m_hasParentheses(false) {}

    void SetHasParentheses() { m_hasParentheses = true; }

public:
    virtual ~ComputedExp () {}
    void SetTypeInfo(ECSqlTypeInfo const& typeInfo) { m_typeInfo = typeInfo; }
    bool HasParentheses() const { return m_hasParentheses; }
    ECSqlTypeInfo const& GetTypeInfo () const {return m_typeInfo;}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
