#pragma once
#include "ScalableMeshPCH.h"
#include <ScalableMesh/IScalableMeshATP.h>
#ifdef SCALABLE_MESH_ATP


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
IScalableMeshATP* IScalableMeshATP::mInstance = nullptr;

std::mutex s_singletonMutex;

IScalableMeshATP*   IScalableMeshATP::GetInstance()
    {
    std::lock_guard<std::mutex> lock(s_singletonMutex);
    if (mInstance == nullptr)
        {
        mInstance = new IScalableMeshATP();
        }
    return mInstance;
    }

StatusInt IScalableMeshATP::StoreInt(WString name, int64_t value)
    {
    IScalableMeshATP* instance = IScalableMeshATP::GetInstance();
    instance->dict_of_ints[name] = value;
    return SUCCESS;
    }

StatusInt IScalableMeshATP::StoreDouble(WString name, double value)
    {
    IScalableMeshATP* instance = IScalableMeshATP::GetInstance();
    instance->dict_of_doubles[name] = value;
    return SUCCESS;
    }

StatusInt IScalableMeshATP::GetInt(WString name, int64_t& value)
    {
    IScalableMeshATP* instance = IScalableMeshATP::GetInstance();
    if (instance->dict_of_ints.count(name) == 0) return ERROR;
    value = instance->dict_of_ints[name];
    return SUCCESS;
    }

StatusInt IScalableMeshATP::GetDouble(WString name, double& value)
    {
    IScalableMeshATP* instance = IScalableMeshATP::GetInstance();
    if (instance->dict_of_doubles.count(name) == 0) return ERROR;
    value = instance->dict_of_doubles[name];
    return SUCCESS;
    }
END_BENTLEY_SCALABLEMESH_NAMESPACE
#endif