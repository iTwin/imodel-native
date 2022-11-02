pugixml library used by ecobjects native.
Download at https://github.com/zeux/pugixml/releases
Documentation is at https://pugixml.org/docs/quickstart.html

When updating to a new version, pick the 2 hpp and 1 cpp files from the zip and replace them in this folder.
Make this modification to pugiconfig.hpp:

Add these lines:

    #if defined (_WIN32)
        #ifdef __PUGIXML_BUILD__
            #define PUGIXML_API __declspec(dllexport)
        #else
            #define PUGIXML_API __declspec(dllimport)
        #endif
    #endif
