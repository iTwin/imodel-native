#pragma once
#include <ProfilesDomain\ProfilesModel.h>
#include <ProfilesDomain\ProfilesPartition.h>
#include <ProfilesDomain\ProfilesDomainDefinitions.h>
#include <Bentley\Bentley.h>

#include <Bentley/WString.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


struct ProfilesTestUtils
    {
    static Profiles::ProfilesPartitionPtr CreateProfilesPartition();
    static Profiles::ProfileDefinitionModelPtr GetProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);
    static Utf8String BuildDefinitionModelCode(Utf8StringCR modelCodeName);
    static Profiles::ProfileDefinitionModelPtr CreateProfilesModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject = nullptr);

    static Utf8String                          CreateCodeSpecNameFromECClass(ECN::ECClassCP ecClass) { Utf8String codeSpecName = ecClass->GetSchema().GetName() + "-" + ecClass->GetName(); return codeSpecName; }
    static Dgn::DgnCode                        CreateCode(Dgn::DgnModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, model, codeValue); }
    static Dgn::DgnCode                        CreateCode(Dgn::DgnModelCR model, ECN::ECClassCP ecClass, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(CreateCodeSpecNameFromECClass(ecClass).c_str(), model, codeValue); }
    template <class T> static RefCountedPtr<T> QueryById(Dgn::DgnModelCR model, Dgn::DgnElementId id) { Dgn::DgnDbR    db = model.GetDgnDb(); return db.Elements().GetForEdit<T>(id); }
    template <class T> static RefCountedPtr<T> QueryByCode(Dgn::DgnModelCR model, Dgn::DgnCodeCR code) { Dgn::DgnDbR  db = model.GetDgnDb(); return QueryById<T>(model, db.Elements().QueryElementIdByCode(code)); }
    template <class T> static RefCountedPtr<T> QueryByCodeValue(Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = CreateCode(model, codeValue); return QueryByCode<T>(model, code); }
    template <class T> static RefCountedPtr<T> QueryByCodeValue(Utf8CP codeSpecName, Dgn::DgnModelCR model, Utf8StringCR codeValue) { Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(codeSpecName, model, codeValue); return QueryByCode<T>(model, code); }
    };

