//----------------------------------------------------------------------------
//
// MetaDataGrid.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "MetaDataGrid.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CMyPropertiesCtrl

CMetaDataGrid::CMetaDataGrid() : m_id(1), m_metaHandle(0)
{ 
	SetCustomDrawManager(new GWCLightColorDrawManager());
	SetSecondColumnWidth(180);

	SetRefColor(RGB(125,156,197));
	m_hStatusBmp = ::LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_STATUS));
}

CMetaDataGrid::~CMetaDataGrid()
{
}

BEGIN_MESSAGE_MAP(CMetaDataGrid, GWCPropertiesCtrl)
	//{{AFX_MSG_MAP(CMetaDataGrid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyPropertiesCtrl message handlers

void CMetaDataGrid::Populate( PThandle h)
{
	Clear();

	if (!h) return;

	m_metaHandle = h;

	wchar_t buffer[ PT_MAX_META_STR_LEN ];
	
	static PTint numClouds=0;
	PTuint64 numPoints=0;
	static PTint numPointsMillions =0;
	PTuint sceneSpec=0;
	PTdouble lower[]  ={0,0,0};
	PTdouble upper[] = {0,0,0};

	ptGetMetaData( h, buffer, numClouds, numPoints, sceneSpec, lower, upper );

	// META
	GWCDeepIterator catIter = AddRootCategory(m_id++, _T("Meta"));

	//		Num Clouds
	GWCDeepIterator scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Name"),GWCPropertyValue::STLSTRING);
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));
	(*scanIter)->GetValue()->SetStringValue( buffer );

	SelectIterItem(scanIter);

	//		Num Points
	scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Num Clouds"),GWCPropertyValue::INT, &numClouds );
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));


	//		Num Points
	numPointsMillions = static_cast<PTint>(numPoints / 1e6);
	scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Num Points (m)"),GWCPropertyValue::INT, &numPointsMillions );
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	//		Lower
	swprintf_s( buffer, PT_MAX_META_STR_LEN, L"%0.3f, %0.3f, %0.3f", lower[0], lower[1], lower[2] );

	scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Lower Bound"),GWCPropertyValue::STLSTRING, NULL, NULL);
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));
	(*scanIter)->GetValue()->SetStringValue( buffer );

	//		Upper
	swprintf_s( buffer, PT_MAX_META_STR_LEN, L"%0.3f, %0.3f, %0.3f", upper[0], upper[1], upper[2] );

	scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Upper Bound"),GWCPropertyValue::STLSTRING, NULL, NULL);
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));
	(*scanIter)->GetValue()->SetStringValue( buffer );

	// INSTRUMENT
	catIter = AddRootCategory(m_id++, _T("Instrument"));

	//		Scanner
	scanIter = AddPropertyUnderCategory(catIter,m_id++,_T("Scanner"),GWCPropertyValue::STLSTRING, NULL, NULL);
	(*scanIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));
	SelectIterItem(scanIter);

	//			Manufacturer
	GWCDeepIterator propIter = AddPropertyUnderCategory(scanIter,m_id++,_T("Manufacturer"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.Manufacturer", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Model
	propIter = AddPropertyUnderCategory(scanIter,m_id++,_T("Model"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.Model", buffer );	
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Serial
	propIter = AddPropertyUnderCategory(scanIter,m_id++,_T("Serial"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.Serial", buffer );	
	(*propIter)->GetValue()->SetStringValue(buffer);

	//		Camera
	GWCDeepIterator camIter  = AddPropertyUnderCategory(catIter,m_id++,_T("Camera"),GWCPropertyValue::STLSTRING, NULL, NULL);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	//			Model
	propIter = AddPropertyUnderCategory(camIter,m_id++,_T("Model"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.CameraModel", buffer );	
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Lens
	propIter = AddPropertyUnderCategory(camIter,m_id++,_T("Lens"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.CameraLens", buffer );	
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Serial
	propIter = AddPropertyUnderCategory(camIter,m_id++,_T("Serial"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Instrument.CameraSerial", buffer );	
	(*propIter)->GetValue()->SetStringValue(buffer);


	// SURVEY
	catIter = AddRootCategory(m_id++, _T("Survey"));

	//			Company
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("Company"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.Company", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Operator
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("Operator"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.Operator", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			ProjectCode
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("ProjectCode"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.ProjectCode", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			ProjectName
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("ProjectName"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.ProjectName", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			GeoReference
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("GeoReference"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.GeoReference", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);


	//			Location
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("SiteLat"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.SiteLat", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Location
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("SiteLong"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.SiteLong", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);


	//			Date of Capture
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("DateOfCapture"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Survey.DateOfCapture", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	// DESCRIPTION
	catIter = AddRootCategory(m_id++, _T("Description"));

	//			Descrption
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("Description"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Description.Description", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Keywords
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("Keywords"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Description.Keywords", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Category
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("Category"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Description.Category", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);


	// AUDIT
	catIter = AddRootCategory(m_id++, _T("Audit"));

	//			CreatorApp
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("CreatorApp"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.CreatorApp", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			DateCreated
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("DateCreated"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.DateCreated", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Compression Tolerance
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("CompressionTolerance"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.CompressionTolerance", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			Compression Tolerance
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("CombinedOnImport"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.CombinedOnImport", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			ChannelsInSource
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("ChannelsInSource"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.ChannelsInSource", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			ChannelsImported
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("ChannelsImported"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.ChannelsImported", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);

	//			SpatialFiltering
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("SpatialFiltering"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.SpatialFiltering", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);


	//			OriginalNumScans
	propIter = AddPropertyUnderCategory(catIter,m_id++,_T("OriginalNumScans"),GWCPropertyValue::STLSTRING);
	(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

	ptGetMetaTag( h, L"Audit.OriginalNumScans", buffer );
	(*propIter)->GetValue()->SetStringValue(buffer);
	

	//			OriginalFilePaths - maybe several
	while ( buffer[0] != 0 ) 
	{
		if (ptGetMetaTag( h, L"Audit.ScanPaths", buffer ))
		{
			propIter = AddPropertyUnderCategory(catIter,m_id++,_T("ScanPath"),GWCPropertyValue::STLSTRING);
			(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

			(*propIter)->GetValue()->SetStringValue(buffer);
		}
	}

	// USER META DATA
	int numSections = ptNumUserMetaSections( h );

	catIter = AddRootCategory(m_id++, _T("User Meta Data"));	
	
	for (int i=0; i<numSections;i++)
	{
		const wchar_t *secName = ptUserMetaSectionName( h, i );		//get the name of the user meta data section

		GWCDeepIterator secIter = AddPropertyUnderCategory(catIter,m_id++,secName,GWCPropertyValue::STLSTRING);
		(*secIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));

		int numTags = ptNumUserMetaTagsInSection( h, i );	// get the num of metatags in this section

		wchar_t name[ PT_MAX_META_STR_LEN ];
		wchar_t value[ PT_MAX_META_STR_LEN ];

		for (int j=0; j<numTags;j++)
		{
			ptGetUserMetaTagByIndex( h, i, j, name, value );	// get this metatag

			propIter = AddPropertyUnderCategory(secIter,m_id++,name,GWCPropertyValue::STLSTRING);
			(*propIter)->SetFeel(GetRegisteredFeel(GWCFEEL_EDIT));
			(*propIter)->GetValue()->SetStringValue(value);
		}
	}
}	

void CMetaDataGrid::PreSubclassWindow()
{
	// Very important to keep this call to the parent as the first line
	GWCPropertiesCtrl::PreSubclassWindow();

	// Can only be done after the window handle exists, so it can't be put in the constructor
	AllowToolTips();
}

void CMetaDataGrid::OnEnableItem(GWCDeepIterator iter, bool enable, bool direct)
{
	int propertyId = (*iter)->GetID();
}

void CMetaDataGrid::OnPropertyChanged(GWCDeepIterator iter)
{
	GWCPropertyItem *par = *(iter.GetParentIterItem());
	GWCPropertyItem *item = (*iter);

	CString metaTag = item->GetName();
	CString value = item->GetValue()->GetStringValue();

	// if its three levels deep shorten to two
	if (iter.GetParentIterItem().HasParent())
	{
		metaTag = par->GetName() + item->GetName();
		iter = iter.GetParentIterItem();
	}	
	par = *(iter.GetParentIterItem());

	metaTag = par->GetName() + L"." + metaTag;
	
	ptSetMetaTag( m_metaHandle, metaTag.GetBuffer(), value.GetBuffer() );
}
