#include "DgnPlatformInternal.h"
#include "ClassHandlers.h"

using IClassHandler = InstanceRepository::IClassHandler;

namespace Handlers {
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct Element : IClassHandler {
        constexpr static auto ClassName = "BisCore:Element";
        Element(ECDbCR db, ECN::ECClassId classId) : IClassHandler(db, classId) {}
        PropertyHandlerResult OnNextId(ECInstanceId& id) override {
            GetDb<DgnDb>().GetElementIdSequence().GetNextValue(id);
            return PropertyHandlerResult::Handled;
        };
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct RenderMaterial : IClassHandler {
        constexpr static auto ClassName = "BisCore:RenderMaterial";
        RenderMaterial(ECDbCR db, ECN::ECClassId classId) : IClassHandler(db, classId) {}
        void OnAfterReadInstance(BeJsValue& instance, const BeJsConst&, JsFormat fmt) {
            if (instance.isObjectMember("materialAssets")) {
                return;
            }
            BeJsValue materialAssets = instance["materialAssets"];
            if (!(materialAssets.hasMember("renderMaterial") && materialAssets["renderMaterial"].hasMember("Map"))) {
                return;
            }
            BeJsValue map = materialAssets["renderMaterial"]["Map"];
            map.ForEachProperty([&](Utf8CP memberName, BeJsConst memberJson) {
                if (memberJson.isNumericMember("TextureId")) {
                    // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                    auto textureIdAsStringForLogging = memberJson["TextureId"].Stringify();
                    auto textureId = memberJson["TextureId"].GetId64<DgnTextureId>();
                    auto textureIdJson = map[memberName]["TextureId"];
                    (BeJsValue&)textureIdJson = textureId;
                    if (!textureId.IsValid()) {
                        ECInstanceId elementId;
                        if (TryGetId(elementId, instance, fmt)) {
                            LOG.warningv("RenderMaterialId: %s, had a TextureId %s which could not be converted to a valid id.", elementId.ToHexStr().c_str(), textureIdAsStringForLogging.c_str());
                            BeAssert(false && "RenderMaterial had a textureId that we converted to invalid.");
                        }
                    }
                }
                return false;
            });
        }
    };
}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool RegisterBisCoreHandlers(DgnDbR db) {
    bool rc = true;
    auto& repo = db.GetInstanceRepository();
    rc &= repo.RegisterClassHandler<Handlers::Element>(Handlers::Element::ClassName);
    rc &= repo.RegisterClassHandler<Handlers::RenderMaterial>(Handlers::RenderMaterial::ClassName);
    return rc;
}