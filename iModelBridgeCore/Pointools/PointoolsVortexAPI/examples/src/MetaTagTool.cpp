/******************************************************************************

Pointools Vortex API Examples

ShaderTool.h

Demonstrates a number of shading options available in Vortex

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include "MetaTagTool.h"


static const char *s_metaTagNames [] = 
{
	"Instrument.Manufacturer",
		"Instrument.Model",
		"Instrument.Serial",
		"Instrument.CameraModel",
		"Instrument.CameraLens",
		"Instrument.CameraSerial",
		"Survey.Company",
		"Survey.Operator",
		"Survey.ProjectCode",
		"Survey.ProjectName",
		"Survey.GeoReference",
		"Survey.SiteLat",
		"Survey.SiteLong",
		"Survey.DateOfCapture",
		"Description.Description",
		"Description.Keywords",
		"Description.Category",
		"Audit.CreatorApp",
		"Audit.DateCreated",
		"Audit.CompressionTolerance",
		"Audit.CombinedOnImport",
		"Audit.ChannelsInSource",
		"Audit.ChannelsImported",
		"Audit.SpatialFiltering",
		"Audit.OriginalNumScans"
};

static const wchar_t *s_metaTagNamesW [] = 
{
	L"Instrument.Manufacturer",
	L"Instrument.Model",
	L"Instrument.Serial",
	L"Instrument.CameraModel",
	L"Instrument.CameraLens",
	L"Instrument.CameraSerial",
	L"Survey.Company",
	L"Survey.Operator",
	L"Survey.ProjectCode",
	L"Survey.ProjectName",
	L"Survey.GeoReference",
	L"Survey.SiteLat",
	L"Survey.SiteLong",
	L"Survey.DateOfCapture",
	L"Description.Description",
	L"Description.Keywords",
	L"Description.Category",
	L"Audit.CreatorApp",
	L"Audit.DateCreated",
	L"Audit.CompressionTolerance",
	L"Audit.CombinedOnImport",
	L"Audit.ChannelsInSource",
	L"Audit.ChannelsImported",
	L"Audit.SpatialFiltering",
	L"Audit.OriginalNumScans"
};
#define NUM_META_TAGS 25

//-----------------------------------------------------------------------------
MetaTagTool::MetaTagTool() : Tool(CmdMetaTagShow, CmdMetaTagSave)
//-----------------------------------------------------------------------------
{
	m_metaDataHandle = 0;
	m_listBox = 0;
	m_currentTag = "<None>";
	m_editTag = 0;
}

//-----------------------------------------------------------------------------
void MetaTagTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{
	case CmdMetaTagShow:
		showMetaTag();
		viewRedraw();
		break;

	case CmdMetaTagSet:
		setMetaTag();
		break;

	case CmdMetaTagSave:
		saveMetaTagEdits();
		break;
	}
}


//-----------------------------------------------------------------------------
void MetaTagTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	//// shader Rollout
	GLUI_Rollout *rolloutMeta = new GLUI_Rollout( parent, "Meta Tags", false );
	rolloutMeta->set_w( PANEL_WIDTH );

//		GLUI_StaticText * spacer = new GLUI_StaticText( rolloutMeta, "" );
//		spacer->set_w( PANEL_WIDTH );
	
		GLUI_Panel *tags = new GLUI_Panel( rolloutMeta, "Fixed Tags" );
		tags->set_w( PANEL_WIDTH );

		m_listBox = new GLUI_List( tags,  false, CmdMetaTagShow, &Tool::dispatchCmd );
		m_listBox->set_w( PANEL_WIDTH );
		m_listBox->set_h( 300 );

		m_editTag = new GLUI_TextBox( tags, m_currentTag );
		m_editTag->set_w( PANEL_WIDTH );
		m_editTag->set_h( 100 );

		// list available meta tags
		for (int i=0; i<NUM_META_TAGS; i++)
		{
			m_listBox->add_item(i, s_metaTagNames[i] );
		}

		new GLUI_Button( tags, "Set Tag", CmdMetaTagSet, &Tool::dispatchCmd );
		new GLUI_Button( tags, "Save to File", CmdMetaTagSave, &Tool::dispatchCmd );
}
//-----------------------------------------------------------------------------
// UNICODE to ASCII convert utility
std::string wstrtostr(const std::wstring &wstr)
//-----------------------------------------------------------------------------
{
    std::string strTo;
    char *szTo = new char[wstr.length() + 1];
    szTo[wstr.size()] = '\0';
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    strTo = szTo;
    delete[] szTo;
    return strTo;
}
//-----------------------------------------------------------------------------
// ASCII to UNICODE convert utility
std::wstring strtowstr(const std::string &str)
//-----------------------------------------------------------------------------
{
    // Convert an ASCII string to a Unicode String
    std::wstring wstrTo;
    wchar_t *wszTo = new wchar_t[str.length() + 1];
    wszTo[str.size()] = L'\0';
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszTo, (int)str.length());
    wstrTo = wszTo;
    delete[] wszTo;
    return wstrTo;
}
//-----------------------------------------------------------------------------
void	MetaTagTool::showMetaTag( void )
//-----------------------------------------------------------------------------
{
	// its an example not production code!
	PThandle podHandles[16];

	if (!ptGetSceneHandles( podHandles ))
	{
		m_metaDataHandle = 0;
		m_editTag->set_text( " " );
		return;
	}

	int current = m_listBox->get_current_item();
	
	if (current >= 0 && current < NUM_META_TAGS)
	{
		const wchar_t *tag = s_metaTagNamesW[current];

		// extract tag for the first POD file, others are ignored for this example
		if (!m_metaDataHandle)
			m_metaDataHandle = ptGetMetaDataHandle( podHandles[0] );

		wchar_t buffer[PT_MAX_META_STR_LEN];
		char asciiTag[PT_MAX_META_STR_LEN];

		asciiTag[0] = 0;

		ptGetMetaTag( m_metaDataHandle, tag, buffer );
		
		// convert to ascii for display 
		std::wstring wtag( buffer );
		m_currentTag = wstrtostr( wtag );
		if (!m_currentTag.length())
			m_editTag->set_text( " " );	// otherwise GUI crashes
		else
			m_editTag->set_text( m_currentTag.c_str() );
		m_editTag->redraw();
	}
}

void	MetaTagTool::setMetaTag()
{
	if (!m_metaDataHandle) return;

	int current = m_listBox->get_current_item();
	
	if (current >= 0 && current < NUM_META_TAGS)
	{
		const wchar_t *tag = s_metaTagNamesW[current];

		ptSetMetaTag( m_metaDataHandle, tag, strtowstr(m_editTag->get_text()).c_str() );
	}
}

void	MetaTagTool::saveMetaTagEdits()
{
	if (m_metaDataHandle)
		ptWriteMetaTags( m_metaDataHandle );
}
/*
		PThandle	PTAPI	ptReadPODMeta( const PTstr filepath );
PThandle	PTAPI	ptGetMetaDataHandle( PThandle sceneHandle );
PTres		PTAPI	ptGetMetaData( PThandle metadataHandle, PTstr name, PTint &num_clouds, 
						PTuint64 &num_points, PTuint &scene_spec, PTdouble *lower3, PTdouble *upper3 );
PTres		PTAPI	ptGetMetaTag( PThandle metadataHandle, const PTstr tagName, PTstr value );
PTres		PTAPI	ptSetMetaTag( PThandle metadataHandle, const PTstr tagName, PTstr value );
PTres		PTAPI	ptWriteMetaTags( PThandle metadataHandle );
PTvoid		PTAPI	ptFreeMetaData( PThandle metadataHandle );
*/