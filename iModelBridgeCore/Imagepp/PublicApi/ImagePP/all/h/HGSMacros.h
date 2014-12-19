//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMacros.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSMacros
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
#pragma once

// Normally included by hstdcpp.h and HDllSupport.h
#ifndef _HDLLNone
#define _HDLLNone
#endif

#include "HGSGraphicToolImplementationFactory.h"
#include "HGSSurfaceImplementationFactory.h"
#include "HGSGraphicToolCapabilities.h"
#include "HGSGraphicToolImplementationCreator.h"

//-----------------------------------------------------------------------------
// Declare the surface
//-----------------------------------------------------------------------------
#define HGS_DECLARE_SURFACE_DLL(HDLL, pi_ClassName) \
    class pi_ClassName##Capabilities : public HGSSurfaceCapabilities \
    { \
        HFC_DECLARE_SINGLETON_DLL(HDLL, pi_ClassName##Capabilities) \
        public: \
                        pi_ClassName##Capabilities(); \
            virtual     ~pi_ClassName##Capabilities() {}; \
    }; \
    class pi_ClassName##Creator : public HGSSurfaceImplementationCreator \
    { \
        HFC_DECLARE_SINGLETON_DLL(HDLL, pi_ClassName##Creator) \
        public: \
                        pi_ClassName##Creator() : HGSSurfaceImplementationCreator() {}; \
            virtual     ~pi_ClassName##Creator() {}; \
            virtual HGSSurfaceImplementation* \
                        Create( const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor) const; \
            virtual HCLASS_ID \
                        GetSurfaceImplementationID() const; \
            virtual const HGSSurfaceCapabilities* \
                        GetCapabilities() const; \
    };

#define HGS_DECLARE_SURFACE(pi_ClassName)   HGS_DECLARE_SURFACE_DLL(_HDLLNone, pi_ClassName)

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the declaration of the child classes
//-----------------------------------------------------------------------------
#define HGS_DECLARE_SURFACECAPABILITIES() \
    public: \
        virtual const HGSSurfaceImplementationCreator* GetCreator() const;

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_BEGIN_SURFACECAPABILITIES_REGISTRATION(pi_ClassName) \
HFC_IMPLEMENT_SINGLETON(pi_ClassName##Capabilities) \
HFC_IMPLEMENT_SINGLETON(pi_ClassName##Creator) \
HGSSurfaceImplementation* \
    pi_ClassName##Creator::Create( const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor) const \
                            {  return new pi_ClassName(pi_rpDescriptor); }; \
HCLASS_ID pi_ClassName##Creator::GetSurfaceImplementationID() const \
                    { return pi_ClassName::CLASS_ID; }; \
const HGSSurfaceCapabilities* \
            pi_ClassName##Creator::GetCapabilities() const { return pi_ClassName##Capabilities::GetInstance();}; \
const HGSSurfaceImplementationCreator* pi_ClassName::GetCreator() const { return pi_ClassName##Creator::GetInstance(); }; \
pi_ClassName##Capabilities::pi_ClassName##Capabilities() \
: HGSSurfaceCapabilities() \
{

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_END_SURFACECAPABILITIES_REGISTRATION() \
}

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_REGISTER_SURFACECAPABILITY(pi_Attribute) \
Add(new HGSSurfaceCapability(pi_Attribute));


//-----------------------------------------------------------------------------
// Declare the graphic tool
//-----------------------------------------------------------------------------
#define HGS_DECLARE_GRAPHICTOOL_DLL(HDLL, pi_ClassName) \
    class pi_ClassName##Capabilities : public HGSGraphicToolCapabilities \
    { \
        HFC_DECLARE_SINGLETON_DLL(HDLL, pi_ClassName##Capabilities) \
        public: \
                        pi_ClassName##Capabilities(); \
            virtual     ~pi_ClassName##Capabilities() {}; \
    }; \
    class pi_ClassName##Creator : public HGSGraphicToolImplementationCreator \
    { \
        HFC_DECLARE_SINGLETON_DLL(HDLL, pi_ClassName##Creator) \
        public: \
                        pi_ClassName##Creator() : HGSGraphicToolImplementationCreator() {}; \
            virtual     ~pi_ClassName##Creator() {}; \
            virtual HGSGraphicToolImplementation* \
                        Create(const HGSGraphicToolAttributes* pi_pAttributes, \
                               HGSSurfaceImplementation*       pi_pSurfaceImplementation) const; \
            virtual HCLASS_ID \
                        GetSurfaceImplementationID() const; \
            virtual HCLASS_ID \
                        GetGraphicToolID() const; \
            virtual const HGSGraphicToolCapabilities* \
                        GetCapabilities() const; \
    };

#define HGS_DECLARE_GRAPHICTOOL(pi_ClassName)   HGS_DECLARE_GRAPHICTOOL_DLL(_HDLLNone, pi_ClassName)

//-----------------------------------------------------------------------------
// Declare the graphic tools
// To insert in the declaration of the child classes
//-----------------------------------------------------------------------------
#define HGS_DECLARE_GRAPHICCAPABILITIES() \
    public: \
        virtual const HGSGraphicToolImplementationCreator* GetCreator() const;

//-----------------------------------------------------------------------------
// Declare the graphic tools
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_BEGIN_GRAPHICCAPABILITIES_REGISTRATION(pi_ClassName, pi_Ancestor, pi_Surface) \
HFC_IMPLEMENT_SINGLETON(pi_ClassName##Capabilities) \
HFC_IMPLEMENT_SINGLETON(pi_ClassName##Creator) \
HGSGraphicToolImplementation* \
    pi_ClassName##Creator::Create(const HGSGraphicToolAttributes* pi_pAttributes, \
                                  HGSSurfaceImplementation*       pi_pSurfaceImplementation) const \
                        {  return new pi_ClassName(pi_pAttributes, pi_pSurfaceImplementation); }; \
HCLASS_ID pi_ClassName##Creator::GetSurfaceImplementationID() const \
                    { return pi_Surface::CLASS_ID; }; \
                    HCLASS_ID pi_ClassName##Creator::GetGraphicToolID() const { return pi_Ancestor::CLASS_ID; }; \
  const HGSGraphicToolImplementationCreator* pi_ClassName::GetCreator() const { return pi_ClassName##Creator::GetInstance(); }; \
const HGSGraphicToolCapabilities* \
            pi_ClassName##Creator::GetCapabilities() const { return pi_ClassName##Capabilities::GetInstance();}; \
pi_ClassName##Capabilities::pi_ClassName##Capabilities() \
: HGSGraphicToolCapabilities() \
{

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_END_GRAPHICCAPABILITIES_REGISTRATION() \
}

//-----------------------------------------------------------------------------
// Declare the capabilities
// To insert in the .cpp file associated to child classes
//-----------------------------------------------------------------------------
#define HGS_REGISTER_GRAPHICCAPABILITY(pi_Attribute) \
Add(new HGSGraphicToolCapability(pi_Attribute));



//-----------------------------------------------------------------------------
// REGISTRATION OF A SURFACE, TO BE CALLED BY THE APPLICATION
//-----------------------------------------------------------------------------
#define HGS_REGISTER_SURFACE(pi_ClassName, pi_Priority) \
static struct pi_ClassName##CreatorRegister \
{ \
    pi_ClassName##CreatorRegister() \
    { \
        HGSSurfaceImplementationCreator* pCreator = pi_ClassName##Creator::GetInstance(); \
        pCreator->SetPriority(pi_Priority); \
        HGSSurfaceImplementationFactory::GetInstance()->Register(pCreator); \
    } \
} g_##pi_ClassName##CreatorRegister;

#define HOST_REGISTER_SURFACE(pi_ClassName, pi_Priority) \
    HGSSurfaceImplementationCreator* pCreator = pi_ClassName##Creator::GetInstance(); \
    pCreator->SetPriority(pi_Priority); \
    HGSSurfaceImplementationFactory::GetInstance()->Register(pCreator); \

//-----------------------------------------------------------------------------
// REGISTRATION OF A GRAPHIC TOOL
//-----------------------------------------------------------------------------
#define HGS_REGISTER_GRAPHICTOOL(pi_ClassName) \
static struct pi_ClassName##GraphicToolRegister \
{ \
    pi_ClassName##GraphicToolRegister() \
    { \
        HGSGraphicToolImplementationFactory::GetInstance()->Register(pi_ClassName##Creator::GetInstance()); \
    } \
} g_##pi_ClassName##CreatorRegister;

#define HOST_REGISTER_GRAPHICTOOL(pi_ClassName) \
    HGSGraphicToolImplementationFactory::GetInstance()->Register(pi_ClassName##Creator::GetInstance()); 

