#include "ECDbValidationChecks.h"
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

std::vector<ECDbValidationChecks::SummarizedResult> ECDbValidationChecks::PerformAllChecks(ECDbCR ecdb) {
    struct CheckData {
        BentleyM0200::Utf8String checkName;
        std::vector<ECDbValidationChecks::CheckResult> (*checkFunction)(ECDbCR);
    };

    std::vector<CheckData> checks = {
        {"nav_prop_id_check", ECDbValidationChecks::NavigationPropertyIdCheck},
        {"class_id_check", ECDbValidationChecks::ClassIdCheck},
    };

    std::vector<ECDbValidationChecks::SummarizedResult> result;

    bool continueChecks = true;

    for(auto & check: checks) {
        if(continueChecks) {
            result.push_back ({
                check.checkName,
                check.checkFunction(ecdb)[0].status
            });

            if(result.back().status == "Failed") 
                continueChecks = false;
        }
        else {
            result.push_back ({
                check.checkName,
                "Did not run"
            });
        }
    }

    return result;
}

std::vector<ECDbValidationChecks::CheckResult> ECDbValidationChecks::NavigationPropertyIdCheck(ECDbCR ecdb) {
    bool checkSuccessful = true;

    std::vector<ECDbValidationChecks::CheckResult> result;

    ECSqlStatement navigationValidityCheck;

    navigationValidityCheck.Prepare(ecdb, "SELECT ec_classname(propdef.Class.id), propdef.Name FROM meta.ECPropertyDef as propdef LEFT JOIN meta.ECClassdef as classdef ON propdef.NavigationRelationshipClass.id = classdef.ECInstanceId WHERE propdef.NavigationRelationshipClass IS NOT NULL AND (classdef.ECInstanceId IS NULL OR classdef.Type != 1)");

    struct PropertyData {
        Utf8String className;
        Utf8String name;
    };

    std::vector<PropertyData> properties = {};

    while(BE_SQLITE_ROW == navigationValidityCheck.Step()) {
        properties.push_back({navigationValidityCheck.GetValueText(0), navigationValidityCheck.GetValueText(1)});
        }

    if(properties.size() != 0) {
        for(auto& property : properties) {
            ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            "Could not find classId of navigation property %s in class %s", property.name.c_str(), property.className.c_str());

            result.push_back({
                "Failed",
                Utf8PrintfString("Could not find classId of navigation property %s in class %s", property.name.c_str(), property.className.c_str()).c_str()
                });
    
            checkSuccessful = false;
            }
        }

    if(checkSuccessful)
        {
        result.push_back({
            "Passed",
            ""
            });
        return result;
        }

    return result;
}

std::vector<ECDbValidationChecks::CheckResult> ECDbValidationChecks::ClassIdCheck(ECDbCR ecdb) {
    bool checkSuccessful = true;

    std::vector<ECDbValidationChecks::CheckResult> result;

    ECSqlStatement classDefStatement;

    classDefStatement.Prepare(ecdb,
    SqlPrintfString("SELECT ec_classname(c.ECInstanceId, 's.c')" 
        "FROM meta.ECClassDef as c "
        "WHERE c.Type != %d AND c.Type != %d AND ec_classname(c.ECInstanceId, 's') != 'ECDbSystem'",
        ECClassType::CustomAttribute, ECClassType::Struct));

    std::vector<BentleyM0200::Utf8String> classNames = {};

    while(BE_SQLITE_ROW == classDefStatement.Step()) {
        classNames.push_back(classDefStatement.GetValueText(0));
        }

    for(auto& className : classNames) {
        ECSqlStatement checkStatement;
        checkStatement.Prepare(ecdb,
        SqlPrintfString("SELECT COUNT(*) "
            "FROM %s as c "
            "LEFT JOIN meta.ECClassDef on c.ECClassId = ECClassDef.ECInstanceId "
            "WHERE ECClassDef.ECInstanceId IS NULL", 
            className.c_str()));

        checkStatement.Step();
        if(checkStatement.GetValueInt(0) != 0) {
            ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue,
                "Could not find definition of class %s", className.c_str());

            result.push_back({
                "Failed",
                Utf8PrintfString("Could not find definition of class %s", className.c_str()).c_str()
                });
            checkSuccessful = false;
            }
        }

    if(checkSuccessful)
        {
        result.push_back({
                "Passed",
                ""
                });
        return result;
        }

    return result;
}

END_BENTLEY_SQLITE_EC_NAMESPACE