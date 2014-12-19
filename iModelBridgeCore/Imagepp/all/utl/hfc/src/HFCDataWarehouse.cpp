//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCDataWarehouse.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCDataWarehouse.h>
//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCDataWarehouse::~HFCDataWarehouse()
    {
    DataContainer::iterator DataItr = m_DataContainer.begin();

    while (DataItr != m_DataContainer.end())
        {
        delete (*DataItr).second;
        DataItr++;
        }
    }