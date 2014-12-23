#ifndef VORTEX_FEATURE_FIT_API_DLL_EXP_H
#define VORTEX_FEATURE_FIT_API_DLL_EXP_H

#ifdef VORTEX_FEATURE_FIT_API
#define PTVFIT_API __declspec(dllexport)
#else
#define PTVFIT_API __declspec(dllimport)
#endif

#endif