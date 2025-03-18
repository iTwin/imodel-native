#include "ClassHandlers.h"
#include "DgnPlatformInternal.h"

using IClassHandler = InstanceRepository::IClassHandler;

namespace Handlers {
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct Element : IClassHandler {
        constexpr static auto ClassName = "BisCore:Element";
        void OnNextId(ECInstanceId& id) override {
            if (GetDb<DgnDb>().GetElementIdSequence().GetNextValue(id) != BE_SQLITE_OK) {
                SetError("Failed to get next id for Element");
            }
        };

        PropertyHandlerResult OnBindECProperty(ECN::ECPropertyCR property, BeJsConst val, IECSqlBinder& binder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            return PropertyHandlerResult::Continue;
        }

        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (BeStringUtilities::StricmpAscii(property, "code") == 0) {
                auto code = DgnCode::FromJson(val, GetDb<DgnDb>(), true);
                rc = finder("CodeValue")->GetBinder().BindText(code.GetValue().GetUtf8CP(), IECSqlBinder::MakeCopy::Yes);
                if (rc != ECSqlStatus::Success) {
                    return PropertyHandlerResult::Handled;
                }
                rc = finder("CodeScope")->GetBinder().BindNavigation(code.GetScopeElementId(GetDbR<DgnDb>()));
                if (rc != ECSqlStatus::Success) {
                    return PropertyHandlerResult::Handled;
                }
                rc = finder("CodeSpec")->GetBinder().BindNavigation(code.GetCodeSpecId());
                return PropertyHandlerResult::Handled;
            }

            return PropertyHandlerResult::Continue;
        }

        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            auto value = finder("CodeValue")->GetReader().GetText();
            auto scopeElementId = finder("CodeScope")->GetReader().GetNavigation<DgnElementId>(nullptr);
            auto specId = finder("CodeSpec")->GetReader().GetNavigation<CodeSpecId>(nullptr);
            auto code = DgnCode::CreateWithDbContext(GetDb<DgnDb>(), specId, scopeElementId, value);
            code.ToJson(instance["code"]);

            instance["model"] = finder("Model")->GetReader().GetNavigation<ECInstanceId>(nullptr);
            return ECSqlStatus::Success;
        }
    };
    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct Model : IClassHandler {
        constexpr static auto ClassName = "BisCore:Model";
        void OnNextId(ECInstanceId& id) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            auto modeledElmentId = BeJsPath::Extract(GetInstance(), "$.modeledElement.id");
            if (modeledElmentId.has_value()) {
                id = modeledElmentId->GetId64<ECInstanceId>();
            }
        };
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct GeometricElement : IClassHandler {
        constexpr static auto ClassName = "BisCore:GeometricElement";
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct GeometricElement3d : IClassHandler {
        constexpr static auto ClassName = "BisCore:GeometricElement3d";
        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            BeAssert(GetFormat() == JsFormat::JsName);
            auto origin = finder("Origin")->GetReader().GetPoint3d();
            auto yaw = finder("Yaw")->GetReader().GetDouble();
            auto pitch = finder("Pitch")->GetReader().GetDouble();
            auto roll = finder("Roll")->GetReader().GetDouble();
            auto boxLow = finder("BBoxLow")->GetReader().GetPoint3d();
            auto boxHi = finder("BBoxHigh")->GetReader().GetPoint3d();

            Placement3d placement(
                origin,
                YawPitchRollAngles(
                    Angle::FromDegrees(yaw),
                    Angle::FromDegrees(pitch),
                    Angle::FromDegrees(roll)),
                ElementAlignedBox3d(boxLow.x, boxLow.y, boxLow.z, boxHi.x, boxHi.y, boxHi.z));
            placement.ToJson(instance["placement"]);
        }
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            BeAssert(GetFormat() == JsFormat::JsName);
            if (BeStringUtilities::StricmpAscii(property, "placement") == 0) {
                auto& originBinder = finder("Origin")->GetBinder();
                auto& yawBinder = finder("Yaw")->GetBinder();
                auto& pitchBinder = finder("Pitch")->GetBinder();
                auto& rollBinder = finder("Roll")->GetBinder();
                auto& bboxLowBinder = finder("BBoxLow")->GetBinder();
                auto& bboxHighBinder = finder("BBoxHigh")->GetBinder();

                Placement3d newPlacement;
                newPlacement.FromJson(val);
                if (!newPlacement.IsValid()) {
                    originBinder.BindNull();
                    yawBinder.BindNull();
                    pitchBinder.BindNull();
                    rollBinder.BindNull();
                    bboxLowBinder.BindNull();
                    bboxHighBinder.BindNull();
                    return PropertyHandlerResult::Handled;
                } else {
                    rc = originBinder.BindPoint3d(newPlacement.GetOrigin());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    rc = yawBinder.BindDouble(newPlacement.GetAngles().GetYaw().Degrees());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    rc = pitchBinder.BindDouble(newPlacement.GetAngles().GetPitch().Degrees());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    rc = rollBinder.BindDouble(newPlacement.GetAngles().GetRoll().Degrees());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    rc = bboxLowBinder.BindPoint3d(newPlacement.GetElementBox().low);
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    rc = bboxHighBinder.BindPoint3d(newPlacement.GetElementBox().high);
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        }
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct RenderMaterial : IClassHandler {
        constexpr static auto ClassName = "BisCore:RenderMaterial";

        virtual ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder _) override {
            BeAssert(GetFormat() == JsFormat::JsName);
            auto map = BeJsPath::Extract(instance, "$.jsonProperties.materialAssets.renderMaterial.Map");
            if (map.has_value()) {
                map.value().ForEachProperty([&](auto memberName, auto memberJson) {
                    if (memberJson.isNumericMember("TextureId")) {
                        // Fix IDs that were previously stored as 64-bit integers rather than as ID strings.
                        auto textureIdAsStringForLogging = memberJson["TextureId"].Stringify();
                        auto textureId = memberJson["TextureId"].GetId64<DgnTextureId>();
                        auto textureIdJson = map->Get(memberName)["TextureId"];
                        (BeJsValue&)textureIdJson = textureId;
                        if (!textureId.IsValid()) {
                            ECInstanceId elementId = ParseInstanceId();
                            if (elementId.IsValid()) {
                                LOG.warningv("RenderMaterialId: %s, had a TextureId %s which could not be converted to a valid id.", elementId.ToHexStr().c_str(), textureIdAsStringForLogging.c_str());
                                BeAssert(false && "RenderMaterial had a textureId that we converted to invalid.");
                            }
                        }
                    }
                    return false;
                });
            }
            return ECSqlStatus::Success;
        }
    };
}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool RegisterBisCoreHandlers(DgnDbR db) {
    bool rc = true;
    auto& repo = db.GetInstanceRepository();
    rc &= repo.RegisterClassHandler<Handlers::Element>(Handlers::Element::ClassName,
                                                       {
                                                           "CodeValue",
                                                           "CodeScope",
                                                           "CodeSpec",
                                                       });
    rc &= repo.RegisterClassHandler<Handlers::GeometricElement3d>(Handlers::GeometricElement3d::ClassName,
                                                                  {
                                                                      "Origin",
                                                                      "Yaw",
                                                                      "Pitch",
                                                                      "Roll",
                                                                      "BBoxLow",
                                                                      "BBoxHigh",
                                                                  });
    rc &= repo.RegisterClassHandler<Handlers::RenderMaterial>(Handlers::RenderMaterial::ClassName);
    rc &= repo.RegisterClassHandler<Handlers::Model>(Handlers::Model::ClassName);
    return rc;
}