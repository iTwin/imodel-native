/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// DOXYGEN LAYOUT CONTROL
// All namespace summaries for the DgnPlatformAPI documentation are placed here.
// The defgroup keyword should only be used in the master list in this file --
//    the larger comments associated with the degroup are place in addtogroup blocks
//    in the appropriate place.  This provides control of the nest structure of the
//    master module list in the Doxygen output.
//
//__PUBLISH_SECTION_START__

#include "ExportMacros.h"

//__PUBLISH_SECTION_END__

// NOTE: The preferred //! doxygen style is not reliable for setting up groups, hence /** method below.
// NOTE: The blank defgroup at the end of each section is required because doxygen puts the last
//       subgroup in the wrong place.  It becomes a child of its grandparent.

/** @cond DGNPLATFORM_SDK_DOCS */
/**  @defgroup DgnPlatformHost DgnPlatform Host */
/** @endcond */
//__PUBLISH_SECTION_START__

/** @cond DGNPLATFORM_SDK_DOCS */
/**
 @defgroup  DocumentManager Document Manager
 @defgroup  DgnPlatformFilesGroup Design Files
 @defgroup  DgnPlatformModelsGroup Models

 @defgroup DgnPlatformElementsGroup Elements
     @{
     @defgroup  ElemHandles              Element Handles
     @defgroup  GeometricQueries         Geometric Queries
     @defgroup  GeometryCollectors       Geometry Collectors
     @defgroup  XAttributes              XAttributes
     @defgroup  ElementCopying           Element Copying
     @defgroup  ElementProperties        Element Properties
         @{
         @defgroup  PropertyContext          Property Context
         @defgroup
         @}
     @defgroup  ElementHandler           Element Handlers
         @{
         @defgroup  DisplayHandler           Displayable Elements
             @{
             @defgroup  Cells                Cell Elements
             @defgroup  SharedCells          Shared Cell Elements
             @defgroup  CurveElements        Curve Elements
             @defgroup  RegionElements       Region Elements
             @defgroup  3DElements           Surface And Solid Elements
             @defgroup  DigitalSignatures    Digital Signature Elements
             @defgroup  MultilineElements    Multi-line Elements
             @defgroup
             @}
        @defgroup  Handler                  Non-Graphic Elements
        @defgroup
        @}
    @defgroup Manipulators             Element Manipulators
    @defgroup
 @}
 @defgroup  DgnECGroup EC Data/Metadata Persistence
     @{
     @defgroup  ECObjectsGroup       ECObjects
     @defgroup
     @}

 @defgroup Views Views
     @{
     @defgroup  Viewports                Viewports
       @{
        @defgroup  Sprites Sprites
        @defgroup
       @}
     @defgroup  ViewContext              ViewContext
     @defgroup  ViewController                 ViewController
     @defgroup  ViewGroup                View Groups
     @defgroup  NamedView                Named Views
     @defgroup  DisplayStyles            Display Styles
     @defgroup
     @}

 @addtogroup Levels
 @{
     @defgroup LevelCache               Level Cache
     @defgroup BitMask                  BitMask
     @defgroup
 @}

 @defgroup TextModule     Text
 @{
    @defgroup DgnFontManagerModule Font Manager
    @defgroup TextStyles           Text Styles
    @defgroup
 @}


 @defgroup AreaPattern              Area Patterns
 @defgroup NamedGroup               Named Groups
 @defgroup LineStyleManagerModule   Line Style Manager
 @defgroup TxnMgr                   Transaction Manager
 @defgroup GeoCoordinate            Geographic Coordination
 @defgroup AuxiliaryCoordinateSystems Auxiliary Coordinate Systems
 @defgroup XMLFragments             XML Fragments

 @defgroup ConfigManagement         Configuration Manager
 @{
    @defgroup ConfigVariableList    Configuration Variable List
    @defgroup
 @}

 @defgroup RegisteredApp RegisteredApps stored in DWG files

 @defgroup DgnDesignHistory Design history

 @defgroup CodeSamples Code Samples
 @{
    @defgroup ElementCopyingCodeSamples ElementCopying Code Samples
    @defgroup DgnDesignHistoryCodeSamples DesignHistory Code Samples
//__PUBLISH_SECTION_END__
    @defgroup DgnDesignHistoryCallbacks DesignHistory Callbacks
//__PUBLISH_SECTION_START__
    @defgroup LevelCacheCodeSamples LevelCache Code Samples
    @defgroup ViewInfoCodeSamples ViewController Code Samples
 @}
 @defgroup IntegrationManager  Integration Manager
*/
/** @endcond */

/** @cond DGNPLATFORM_SDK_DOCS */
/*=================================================================================**//**
@mainpage DgnPlatform Public API Documentation

\image html BentleyLOGO_4C_no-tag.gif

This file describes the public API for writing applications using the DgnPlatform libraries.

@section Namespaces
All of the DgnPlatform APIs are contained within the BentleyApi namespace. There are nested namespaces within the
BentleyApi namespace that hold various parts of the API. Most DgnPlatform APIs are in the BentleyApi::Dgn namespace.

@section headerFiles Header Files
By far the most convenient and efficient way of accessing the classes in the DgnPlatform API is to use:

@code
    #include <DgnPlatform/DgnPlatformAPI.h>
@endcode

in each of your source files. This will include \e all of the individual header files in the API.
@note If you have more than one source file linked into a single .dll, you should <b>probably use precompiled
header files</b> (see Microsoft Visual C++ documentation for details.)

@section DgnPlatformApiOverview Overview

The host application must implement and supply a \ref DgnPlatformHost object to the DgnPlatform library.

The DgnPlatform library is used to read and write \ref DgnPlatformFilesGroup.
A design file is composed of \ref DgnPlatformModelsGroup.
A model contains \ref DgnPlatformElementsGroup.
Use the \ref TxnMgr in order to make changes.

For a list of configuration variables used by DgnPlatform, see \ref ConfigVariableList

* @bsiclass
+===============+===============+===============+===============+===============+======*/
/** @endcond */

#include "DgnPlatform.h"
#include "DgnCoreAPI.h"
#include "DgnHandlersAPI.h"
//__PUBLISH_SECTION_END__
#include  <Bentley/BeAssert.h>
