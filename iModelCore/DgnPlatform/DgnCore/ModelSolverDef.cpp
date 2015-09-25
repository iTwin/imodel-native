/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ModelSolverDef.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnScript.h>
#include <DgnPlatform/DgnPlatformLib.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ModelSolverDef::ParameterSet::ConvertValuesToJson(Json::Value& json) const
    {
    for (auto const& parm : *this)
        {
        if (BSISUCCESS != ECUtils::ConvertECValueToJson(json[parm.GetName().c_str()], parm.GetValue()))
            return BSIERROR;
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::Parameter::Parameter(Utf8CP n, Scope s, ECN::ECValueCR v) : m_name(n), m_scope(s), m_value(v) {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECValueCR ModelSolverDef::Parameter::GetValue() const {return m_value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::ModelSolverDef(Type type, Utf8CP identifier, bvector<Parameter> const& parameters) : m_type(type), m_name(identifier), m_parameters(parameters) {;}

ModelSolverDef::ModelSolverDef() {m_type = Type::None;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::ParameterSet const& ModelSolverDef::GetParameters() const {return m_parameters;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::ParameterSet& ModelSolverDef::GetParametersR() {return m_parameters;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSolverDef::Solve(GeometricModelR model)
    {
    if (Type::Script == m_type)
        {
        int retval;
        Json::Value parmsJson(Json::objectValue);
        GetParameters().ConvertValuesToJson(parmsJson);
        Json::Value optionsJson(Json::objectValue);
        model.GetSolverOptions(optionsJson);
        DgnDbStatus xstatus = DgnScript::ExecuteModelSolver(retval, model, m_name.c_str(), parmsJson, optionsJson);
        if (xstatus != DgnDbStatus::Success || 0 != retval)
            {
            TxnManager::ValidationError err(TxnManager::ValidationError::Severity::Fatal, "Model solver failed");   // *** NEEDS WORK: Get failure description from ModelSolverDef
            model.GetDgnDb().Txns().ReportError(err);
            }
        }
    else
        {
        // *** TBD: Add support for built-in constraint solvers 

        BeAssert((m_type == Type::None) && "Only Script model solvers supported");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::Parameter const* ModelSolverDef::ParameterSet::GetParameter(Utf8StringCR pname) const
    {
    for (auto const& parameter : *this)
        {
        if (parameter.GetName().EqualsI(pname))
            return &parameter;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::Parameter* ModelSolverDef::ParameterSet::GetParameterP(Utf8StringCR pname)
    {
    return const_cast<Parameter*>(const_cast<ParameterSet*>(this)->GetParameter(pname));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSolverDef::Parameter::SetValue(ECN::ECValueCR valueIn)
    {
    if (!m_value.IsPrimitive())
        return DgnDbStatus::BadArg;
        
    ECN::ECValue value = valueIn;
    if (!value.ConvertToPrimitiveType(m_value.GetPrimitiveType()))
        return DgnDbStatus::BadArg;

    m_value = value;
    return DgnDbStatus::Success;
    }

// *** Persistent values *** Do not change ***
#define PARARMETER_SCOPE_CLASS 0
#define PARARMETER_SCOPE_TYPE 1
#define PARARMETER_SCOPE_INSTANCE 2

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ModelSolverDef::Parameter::ToJson() const
    {
    Json::Value v;
    v["Scope"] = (Scope::Class == m_scope)? PARARMETER_SCOPE_CLASS: (Scope::Type == m_scope)? PARARMETER_SCOPE_TYPE: PARARMETER_SCOPE_INSTANCE;
    v["Name"] = m_name.c_str();
    ECUtils::StoreECValueAsJson(v["Value"], m_value);
    return v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::Parameter::Parameter(Json::Value const& json)
    {
    auto s = json["Scope"].asInt();
    m_scope = (PARARMETER_SCOPE_CLASS == s)? Scope::Class: (PARARMETER_SCOPE_TYPE == s)? Scope::Type: Scope::Instance;
    m_name = json["Name"].asCString();
    ECUtils::LoadECValueFromJson(m_value, json["Value"]);
    }

// *** Persistent values *** Do not change ***
#define SOLVER_TYPE_NONE    0
#define SOLVER_TYPE_SCRIPT  1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ModelSolverDef::ToJson() const
    {
    int tval;
    switch (m_type)
        { 
        case Type::None:   tval = SOLVER_TYPE_NONE;   break;
        case Type::Script: tval = SOLVER_TYPE_SCRIPT; break;
        default:
            tval = 0;
            BeAssert(false);
        }

    Json::Value json (Json::objectValue);
    json["Type"] = tval;
    json["Name"] = m_name.c_str();
    json["Version"] = m_version.c_str();
    json["Parameters"] = m_parameters.ToJson();

    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSolverDef::FromJson(Utf8CP str)
    {
    //  Parse
    Json::Value json(Json::objectValue);
    if (!Json::Reader::Parse(str, json))
        {
        BeAssert(false);
        return;
        }

    //  Validate content
    if (!json.isMember("Type") || !json.isMember("Name") || !json.isMember("Parameters"))
        {
        BeAssert(false);
        return;
        }

    switch (json["Type"].asInt())
        { 
        case SOLVER_TYPE_NONE:   m_type = Type::None;   break;
        case SOLVER_TYPE_SCRIPT: m_type = Type::Script; break;
        default:
            m_type = Type::None;
            BeAssert(false);
        }
    
    //  Extract simple properties
    m_name = json["Name"].asCString();
    m_version = json["Version"].asCString();
    m_parameters = ParameterSet(json["Parameters"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSolverDef::ParameterSet::ParameterSet(Json::Value const& json)
    {
    for (Json::ArrayIndex i=0; i < json.size(); ++i)
        m_parameters.push_back(Parameter(json[i]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ModelSolverDef::ParameterSet::ToJson() const
    {
    Json::Value parametersJson (Json::arrayValue);
    for (auto const& parameter : m_parameters)
        parametersJson.append(parameter.ToJson());
    return parametersJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ModelSolverDef::ParameterSet::ComputeSolutionName() const
    {
    Utf8String str;
    Utf8CP sep = "";
    for (auto const& parameter : m_parameters)
        {
        str.append(sep).append(parameter.GetName()).append(":").append(parameter.GetValue().ToString());
        sep = ",";
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSolverDef::ParameterSet::SetValuesFromECProperties(ECN::IECInstanceCR instance)
    {
    for (auto& parameter : *this)
        {
        ECN::ECValue ecv;
        if (ECN::ECOBJECTS_STATUS_Success != instance.GetValue(ecv, parameter.GetName().c_str()))
            {
            BeDataAssert(false);
            return DgnDbStatus::BadArg;
            }
        parameter.SetValue(ecv);
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSolverDef::ParameterSet::SetValues(ParameterSet const& parmsIn)
    {
    for (auto const& parmIn : parmsIn)
        {
        auto parm = GetParameterP(parmIn.GetName());
        if (nullptr == parm)
            return DgnDbStatus::NotFound;
        DgnDbStatus status = parm->SetValue(parmIn.GetValue());
        if (DgnDbStatus::Success != status)
            return status;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSolverDef::RelocateToDestinationDb(DgnImportContext& context)
    {
    // If I refer to a script, make sure the script has been copied over
    if (Type::Script != GetType())
        return;

    auto& scriptAdmin = T_HOST.GetScriptAdmin();

    Utf8String scriptName = GetName().c_str();
    auto idot = scriptName.find(".");
    if (Utf8String::npos != idot)
        { // The name of the *program* is before the dot. The specific solver function that we use is after the dot.
        scriptName = scriptName.substr(0, idot);
        }

    Utf8String scriptText;
    DgnScriptType stypeFound;
    if (DgnDbStatus::Success == scriptAdmin._FetchScript(scriptText, stypeFound, context.GetDestinationDb(), scriptName.c_str(), DgnScriptType::JavaScript))
        return; // we already have this script

    if (DgnDbStatus::Success != scriptAdmin._FetchScript(scriptText, stypeFound, context.GetSourceDb(), scriptName.c_str(), DgnScriptType::JavaScript))
        {
        BeDataAssert(false && "source script is missing");
        return;
        }

    DgnScriptLibrary scriptLib(context.GetDestinationDb());
    scriptLib.RegisterScript(scriptName.c_str(), scriptText.c_str(), stypeFound, false);
    }
