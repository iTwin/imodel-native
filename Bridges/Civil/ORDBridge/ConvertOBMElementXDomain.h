/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ConvertOBMElementXDomain.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "ORDBridgeInternal.h"

BEGIN_ORDBRIDGE_NAMESPACE

struct ConvertOBMElementXDomain : Dgn::DgnDbSync::DgnV8::XDomain
{
private:
    ORDConverter& m_converter;
    Dgn::DgnClassId m_graphic3dClassId;
    bset<Bentley::ElementRefP> m_elementsSeen;
    bset<Bentley::ElementRefP> m_bridgeV8RefSet;
    bset<Bentley::ElementRefP> m_corridorV8RefSet;
    static Bentley::ECN::ECRelationshipClassCP FindRelationshipClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className);
    static Bentley::ECN::ECClassCP FindClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className);
    static DgnPlatform::DgnECRelationshipIterable FindRelationships(DgnECInstanceCR instance, Bentley::ECN::ECRelationshipClassCP relClass, Bentley::ECN::ECRelatedInstanceDirection dir);

protected:
    virtual void _DetermineElementParams(Dgn::DgnClassId&, Dgn::DgnCode&, Dgn::DgnCategoryId&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual Result _PreConvertElement(DgnV8EhCR v8el, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    //virtual void _ProcessResults(Dgn::DgnDbSync::DgnV8::ElementConversionResults&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&, Dgn::DgnDbSync::DgnV8::Converter&) override;
    virtual void _OnFinishConversion(Dgn::DgnDbSync::DgnV8::Converter&) override {};

public:
    ConvertOBMElementXDomain(ORDConverter& converter);
    template<class T> RefCountedPtr<T> GetGeometryOwner(Bentley::ObmNET::PierColumnPtr);
    void CreateDgnBridge(Bentley::ObmNET::BridgePtr bridgePtr, bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimID);
    void CreateDgnBridges();
}; // ConvertOBMElementXDomain

END_ORDBRIDGE_NAMESPACE