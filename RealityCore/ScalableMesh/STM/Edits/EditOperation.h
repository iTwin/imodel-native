/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/bvector.h>
#include <Bentley/RefCounted.h>
#include <DgnPlatform/ClipVector.h>
#include <Geom/Polyface.h>

struct EditOperation : RefCountedBase
    {

    public:
        ClipVectorPtr m_toRemoveVector;
        PolyfaceHeaderPtr m_toInsertPoly;

        enum class Op
            {
            REMOVE = 0,
            REPLACE
            };

        Op m_opType;

        EditOperation(Op opType, ClipVectorCP removeVec = nullptr, PolyfaceQueryCP insertMesh = nullptr) : m_opType(opType)
            {
            if (removeVec != nullptr) m_toRemoveVector = ClipVector::CreateCopy(*removeVec);
            if (insertMesh != nullptr) m_toInsertPoly = insertMesh->Clone();
            };

        ~EditOperation()
            {};

        static RefCountedPtr<EditOperation> Create(Op opType, ClipVectorCP removeVec = nullptr, PolyfaceQueryCP insertMesh = nullptr)
            {
            return new EditOperation(opType, removeVec, insertMesh);
            }

    };
