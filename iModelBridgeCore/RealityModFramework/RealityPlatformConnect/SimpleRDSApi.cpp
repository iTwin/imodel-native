#include "../RealityPlatform/SimpleRDSApi.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
Utf8String RDSRequestManager::MakeBuddiCall()
    {
    return "https://connect-realitydataservices.bentley.com/";
    }