/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PrivateApi/BuildingEventsHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_NAMESPACE
#define BuildingEventsHandlerPtr RefCountedPtr<BuildingEventsHandler>

struct BuildingEventsHandler : RefCountedBase, DgnClientFx::IMessageHandler
    {
    private:
        static BuildingEventsHandlerPtr s_instance;
        ~BuildingEventsHandler();

    public:
        BUILDINGUTILS_EXPORT static BuildingEventsHandlerPtr GetInstance();

    protected:
        virtual void                _HandleMessage(DgnClientFx::JsonMessage const&, DgnClientFx::MessageResponse& response) override;
        virtual bset<Utf8String>    _GetMessageTypes() const override;
    };

END_BUILDING_NAMESPACE