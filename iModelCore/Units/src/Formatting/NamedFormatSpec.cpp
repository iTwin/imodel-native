/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/NamedFormatSpec.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec() : m_specType(FormatSpecType::None), m_problem(FormatProblemCode::NotInitialized) 
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec(NamedFormatSpecCR other)
    : m_name(other.m_name), m_description(other.m_description), 
    m_displayLabel(other.m_displayLabel), m_specType(other.m_specType), m_problem(other.m_problem)
    {
    if (other.HasNumeric())
        m_numericSpec = other.m_numericSpec;
    if (other.HasComposite())
        m_compositeSpec = other.m_compositeSpec;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec(Utf8StringCR name, NumericFormatSpecCR numSpec)
    : m_name(name), m_specType(FormatSpecType::None), m_numericSpec(numSpec), m_problem(FormatProblemCode::NoProblems)
    {
    if (name.empty())
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Utf8StringCR name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec)
    : NamedFormatSpec(name, numSpec)
    {
    m_compositeSpec = compSpec;
    if (m_compositeSpec.IsProblem())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
        }
    else
        {
        switch (m_compositeSpec.GetUnitCount())
            {
            case 1:
                m_specType = FormatSpecType::Single;
                break;
            case 2:
                m_specType = FormatSpecType::Double;
                break;
            case 3:
                m_specType = FormatSpecType::Triple;
                break;
            case 4:
                m_specType = FormatSpecType::Quad;
                break;
            default:
                m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz
//----------------------------------------------------------------------------------------
void NamedFormatSpec::FromJson(Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    FromJson(jval, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::FromJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    *this = NamedFormatSpec();
    m_problem = FormatProblemCode::NoProblems;
    if (jval.empty())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidJsonObject);
        return;
        }

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_SpecName()) == 0)
            m_name = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecDescript()) == 0)
            m_description = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecLabel()) == 0)
            m_displayLabel = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_NumericFormat()) == 0)
            m_numericSpec = NumericFormatSpec(val);
        else if (BeStringUtilities::StricmpAscii(paramName, json_CompositeFormat()) == 0)
            m_compositeSpec.LoadJsonData(val, context);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool NamedFormatSpec::IsIdentical(NamedFormatSpecCR other) const
    {
    if (m_name != other.m_name)
        return false;
    if (m_specType != other.m_specType)
        return false;
    if (HasNumeric() && !m_numericSpec.IsIdentical(other.m_numericSpec))
        return false;
    if (HasComposite() && !m_compositeSpec.IsIdentical(other.m_compositeSpec))
        return false;
    if (m_problem.GetProblemCode() != other.m_problem.GetProblemCode())
        return false;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value NamedFormatSpec::ToJson(bool verbose) const
    {
    Json::Value jNFS;
    jNFS[json_SpecName()] = m_name;
    if (!m_description.empty())
        jNFS[json_SpecDescript()] = m_description;
    if (!m_displayLabel.empty())
        jNFS[json_SpecLabel()] = m_displayLabel;

    jNFS[json_NumericFormat()] = m_numericSpec.ToJson(verbose);
    Json::Value jcs = m_compositeSpec.ToJson();
    if (!jcs.empty())
        jNFS[json_CompositeFormat()] = jcs;
    return jNFS;
    }

END_BENTLEY_FORMATTING_NAMESPACE
