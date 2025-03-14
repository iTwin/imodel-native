#include "ClassHandlers.h"
#include "DgnPlatformInternal.h"

using IClassHandler = InstanceRepository::IClassHandler;
using WriteArgs = InstanceRepository::WriteArgs;
using ReadArgs = InstanceRepository::ReadArgs;

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
    // struct GeometricElement : IClassHandler {
    //     constexpr static auto ClassName = "BisCore:GeometricElement";
    //     GeometricElement(ECDbCR db, ECN::ECClassId classId) : IClassHandler(db, classId) {}
    //     virtual PropertyHandlerResult OnBindProperty(WriteArgs& args) {

    //         return PropertyHandlerResult::Continue;
    //     };
    //     virtual PropertyHandlerResult OnReadProperty(ReadArgs& args) {
    //         args.GetProperty().GetName().EqualsIAscii("Category");

    //         return PropertyHandlerResult::Continue;
    //     };
    // };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct RenderMaterial : IClassHandler {
        constexpr static auto ClassName = "BisCore:RenderMaterial";
        RenderMaterial(ECDbCR db, ECN::ECClassId classId) : IClassHandler(db, classId) {}

        void OnAfterReadInstance(BeJsValue& instance, const BeJsConst&, JsFormat fmt) {
            BeAssert(fmt == JsFormat::JsName);
            auto map = BeJsPath::Extract(instance, "jsonProperties.materialAssets.renderMaterial.Map");
            if (map.has_value()) {
                map.value().ForEachProperty([&](auto memberName, auto memberJson) {
                    if (memberJson.isNumericMember("TextureId")) {
                        // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                        auto textureIdAsStringForLogging = memberJson["TextureId"].Stringify();
                        auto textureId = memberJson["TextureId"].GetId64<DgnTextureId>();
                        auto textureIdJson = map->Get(memberName)["TextureId"];
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