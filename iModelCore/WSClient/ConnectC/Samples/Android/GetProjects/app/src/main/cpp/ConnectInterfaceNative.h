//
// Created by Robert.Priest on 2/16/2017.
//
#include <string>
#include <list>
#include <map>
#include <Bentley/Bentley.h>
#include <Logging/bentleylogging.h>
#include <WebServices/ConnectC/CWSCCPublic.h>
#include <WebServices/ConnectC/CWSCCBufferPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaGenPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaBufferGenPublic.h>
#include <WebServices/ConnectC/IMSSearch/IMSSearchPublic.h>

#ifndef CONNECTCTEST1_CONNECTINTERFACE_H
#define CONNECTCTEST1_CONNECTINTERFACE_H



class ConnectInterfaceNative {


private:
    bool m_isInitialized = false;
    CWSCCHANDLE m_apiHandle = 0;
    void GetAPIHandle();
    int InsertPropertyIntoMap(CWSCCDATABUFHANDLE project, int propertyId, std::map<int, wstring>* map);
    int InsertPropertyIntoMap(CWSCCDATABUFHANDLE project, int propertyId, int mapIndex, std::map<int, wstring>* map);

public:
    static void Initialize( std::wstring appDir, std::wstring tempDir, std::wstring externalStorageDir, std::wstring deviceId, void* secStore);
    std::string GetConnectUserName();
    int GetProjects(std::map<wstring, wstring>* idToNameMap);
    int GetProjectPropertyMapV4(wstring projectId, std::map<int, wstring>* map);
    ConnectInterfaceNative();
    ~ConnectInterfaceNative();
};

#endif //CONNECTCTEST1_CONNECTINTERFACE_H
