#pragma once
#if WINDOWS
#ifdef __CLOUD_DATA_SOURCE_BUILD__ 
    #define CLOUD_EXPORT __declspec(dllexport)
#else
    #define CLOUD_EXPORT __declspec(dllimport)
#endif
#else
    #define CLOUD_EXPORT
#endif
