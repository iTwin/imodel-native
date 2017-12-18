#pragma once

#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
//#include "C:\BSW\Bim0200Dev\src\WSClient\iModelHubClient\Utils.h"
#include <WebServices\iModelHub\Utils.h>
#include "FakeServer.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP

class RequestHandler
    {
    WCharCP serverPath;
    public:
        RequestHandler();
        ~RequestHandler();
        Response PerformGetRequest(Request req);
        Response PerformOtherRequest(Request req);
        Response CreateiModel(Request req);

    private:

    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
