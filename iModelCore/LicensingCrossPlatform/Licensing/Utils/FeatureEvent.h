#pragma once

#include <Licensing/Licensing.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

namespace FeatureEvent {
    LICENSING_EXPORT Utf8String ToJson(
        Utf8StringCR featureId,
        int productId,
        Utf8StringCR featureString,
        BeVersionCR version,
        Utf8StringCR deviceId,
        Utf8StringCR projectId
    );
}

END_BENTLEY_LICENSING_NAMESPACE