/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMDrapedLinearElement.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include ".\DTMDrapedLinearElement.h"
#using <mscorlib.dll>
#include "DTMHelpers.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMDrapedLinearElement::DTMDrapedLinearElement (BcDTMDrapedLine* dtmDrapedLine)
    {
    m_dtmDrapedLine = dtmDrapedLine;
    m_dtmDrapedLine->AddRef();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMDrapedLinearElementPointEnumerator::DTMDrapedLinearElementPointEnumerator (BcDTMDrapedLine* dtmDrapedLine)
    {
    m_index = -1;
    m_dtmDrapedLine = dtmDrapedLine;
    m_dtmDrapedLine->AddRef();
    }

DTMDrapedLinearElement::~DTMDrapedLinearElement ()
    {
    this->!DTMDrapedLinearElement();
    }

DTMDrapedLinearElement::!DTMDrapedLinearElement ()
    {
    if (m_dtmDrapedLine) m_dtmDrapedLine->Release();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      1/2008
//=======================================================================================
IEnumerator^ DTMDrapedLinearElement::GetEnumeratorInternal ()
    {
    return GetEnumerator ();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      1/2008
//=======================================================================================
Generic::IEnumerator<DTMDrapedLinearElementPoint^>^ DTMDrapedLinearElement::GetEnumerator ()
    {
    return gcnew DTMDrapedLinearElementPointEnumerator (m_dtmDrapedLine);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
System::Object^ DTMDrapedLinearElementPointEnumerator::CurrentObject::get ()
    {
    return Current;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMDrapedLinearElementPoint^ DTMDrapedLinearElementPointEnumerator::Current::get ()
    {
    BcDTMDrapedLinePointPtr pt = NULL;
    m_dtmDrapedLine->GetPointByIndex (pt, m_index);
    DTMDrapedLinearElementPoint^ secPoint = gcnew DTMDrapedLinearElementPoint (pt.get());
    return secPoint;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
bool DTMDrapedLinearElementPointEnumerator::MoveNext ()
    {
    int nPoint = m_dtmDrapedLine->GetPointCount ();
    if (m_index < nPoint - 1)
        {
        m_index++;
        return true;
        }
    else
        return false;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTMDrapedLinearElementPointEnumerator::Reset ()
    {
    m_index = -1;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMDrapedLinearElementPointEnumerator::~DTMDrapedLinearElementPointEnumerator ()
    {
    this->!DTMDrapedLinearElementPointEnumerator ();
    }

DTMDrapedLinearElementPointEnumerator::!DTMDrapedLinearElementPointEnumerator ()
    {
    if (m_dtmDrapedLine != NULL)
        {
        m_dtmDrapedLine->Release ();
        m_dtmDrapedLine = NULL;
        }
    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE
