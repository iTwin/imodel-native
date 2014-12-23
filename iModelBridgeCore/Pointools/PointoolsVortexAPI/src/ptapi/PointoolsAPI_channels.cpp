#include <pt/os.h>
#define POINTOOLS_API_BUILD_DLL
#include <gl/glew.h>

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsVortexAPI_ResultCodes.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptds/DataSourceMemory.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/PointsExchanger.h>
#include <ptengine/VisibilityEngine.h>
#include <ptengine/userChannels.h>
#include <ptengine/engine.h>

#include <ptcloud2/pod.h>
#include <pt/project.h>

#include <ptgl/gltext.h>

#include <pt/memrw.h>
#include <list>

using namespace pt;
using namespace pcloud;
using namespace pointsengine;

extern int setLastErrorCode( int );


namespace
{
typedef std::map <PThandle, UserChannel *> ChannelMap;
ChannelMap g_channels;
PThandle _lastHandle=6000;
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
UserChannel* findChannel( PThandle h )
{
	ChannelMap::iterator i = g_channels.find( h );
	if (i != g_channels.end())
	{
		return i->second;
	}
	return 0;
}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
UserChannel *_ptGetUserChannel( PThandle h ) { return findChannel(h); }
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PThandle	PTAPI ptCreatePointChannel( PTstr name, PTenum typesize, PTuint multiple, void* default_value, PTuint flags )
{
	UserChannel *channel = UserChannelManager::instance()->createChannel( pt::String(name), typesize * 8, multiple, default_value, (UserChannelFlags)flags);

	if (!channel)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return PT_NULL;
	}

	g_channels.insert( ChannelMap::value_type(++_lastHandle, channel ) );

	return _lastHandle;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PThandle PTAPI ptCopyPointChannel(PThandle channel, PTstr destName, PTuint destFlags)
{
	ChannelMap::iterator i = g_channels.find(channel);
	if (i != g_channels.end())
	{
		UserChannel *newChannel = UserChannelManager::instance()->copyChannel(i->second, pt::String(destName), (UserChannelFlags) destFlags);

		if (!newChannel)
		{
			setLastErrorCode( PTV_OUT_OF_MEMORY );
			return PT_NULL;
		}

		g_channels.insert(ChannelMap::value_type(++_lastHandle, newChannel));

		return _lastHandle;
	}

	setLastErrorCode(PTV_INVALID_PARAMETER);

	return PT_NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PThandle PTAPI ptGetChannelByName( PTstr name )
{
	UserChannel *uc = UserChannelManager::instance()->channelByName( pt::String( name ) );

	if (uc)
	{
		ChannelMap::iterator i = g_channels.begin();
		while (i!=g_channels.end())
		{
			if (i->second == uc)
				return i->first;
			++i;
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTres	PTAPI ptGetChannelInfo(PThandle handle, PTstr name, PTenum& typesize, PTuint& multiple, void *defaultValue, PTuint& flags)
{
	ChannelMap::iterator i = g_channels.find( handle );
	if (i != g_channels.end())
	{
		UserChannel *channel = i->second;

		if(name)
		{
			wcscpy_s(name, 128, channel->name().c_wstr()); 
		}

		typesize = channel->bitsPerValue() / 8;
		multiple = channel->multiple();
		
		if (channel->defaultValue() && defaultValue)
			memcpy(defaultValue, channel->defaultValue(), multiple * typesize);

		flags = channel->flags();

		return setLastErrorCode( PTV_SUCCESS );
	}
	return setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTres	PTAPI ptDeletePointChannel( PThandle channel )
{
	ChannelMap::iterator i = g_channels.find( channel );
	if (i != g_channels.end())
	{
		UserChannelManager::instance()->eraseChannel( i->second );
		g_channels.erase(i);
		return setLastErrorCode( PTV_SUCCESS );
	}
	return setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTres	PTAPI ptDrawPointChannelAs( PThandle channel, PTenum option, PTfloat param1, PTfloat param2 )
{
	ChannelMap::iterator i = g_channels.find( channel );
	if (i == g_channels.end()) return PTV_INVALID_HANDLE;

	switch ( option )
	{
	case PT_CHANNEL_AS_OFFSET:
		return 
			UserChannelManager::instance()->renderAsOffset( i->second, param1 )
			? setLastErrorCode( PTV_SUCCESS ) : setLastErrorCode( PTV_INVALID_VALUE_FOR_PARAMETER );

	case PT_CHANNEL_AS_RAMP:
		return (PTbool)UserChannelManager::instance()->renderAsTexture( i->second, 0 )
			? setLastErrorCode( PTV_SUCCESS ) : setLastErrorCode( PTV_INVALID_VALUE_FOR_PARAMETER );

	case PT_CHANNEL_AS_RGB:
		return (PTbool)UserChannelManager::instance()->renderAsRGB( i->second )
			? setLastErrorCode( PTV_SUCCESS ) : setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );

	case PT_CHANNEL_AS_ZSHIFT:
		return (PTbool)UserChannelManager::instance()->renderAsZShift( i->second, 0 )
			? setLastErrorCode( PTV_SUCCESS ) : setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	}
	return setLastErrorCode( PTV_INVALID_OPTION );
}
//-----------------------------------------------------------------------------
// Create channels from a file
//-----------------------------------------------------------------------------
PTres		PTAPI ptReadChannelsFile( const PTstr filename, PTint &numChannels, PThandle **channelHandles )
{
	static PThandle s_channelHandles[256];

	std::vector<UserChannel*> channels;
	PTres res = UserChannelManager::instance()->loadChannelsFromFile( pt::String( filename ), channels );

	numChannels = channels.size();
	if (numChannels > 256) numChannels = 256;

	if (res != PTV_SUCCESS) 
		return res;

	for (int i=0; i<numChannels; i++)
	{
		s_channelHandles[i] = ++_lastHandle;
		g_channels.insert( ChannelMap::value_type( s_channelHandles[i], channels[i] ) );		
	}
	*channelHandles = s_channelHandles;
	return res;
}

//-----------------------------------------------------------------------------
// Read channels from a file from memory
//-----------------------------------------------------------------------------
PTres PTAPI ptReadChannelsFileFromBuffer(void *buffer, PTuint64 bufferSize, PTint &numChannels, PThandle **channelHandles)
{
	static PThandle s_channelHandles[256];

	if(buffer == NULL || bufferSize == 0)
		return PTV_INVALID_PARAMETER;

	pt::String dummyFilename(PTRMI::URL::PT_PTMY);
	dummyFilename += pt::String(L"://File.uch");

	std::vector<UserChannel*> channels;
	PTres res = UserChannelManager::instance()->loadChannelsFromFile(dummyFilename, channels, static_cast<ptds::DataSource::Data *>(buffer), bufferSize);

	numChannels = channels.size();
	if (numChannels > 256) numChannels = 256;

	if (res != PTV_SUCCESS) 
		return res;

	for (int i=0; i<numChannels; i++)
	{
		s_channelHandles[i] = ++_lastHandle;
		g_channels.insert( ChannelMap::value_type( s_channelHandles[i], channels[i] ) );		
	}
	*channelHandles = s_channelHandles;
	return res;
}

//-----------------------------------------------------------------------------
// Write a file for persisting Channel use
//-----------------------------------------------------------------------------
PTres	PTAPI ptWriteChannelsFile( const PTstr filename, PTint numChannels, const PThandle *channels )
{
	std::vector <UserChannel *> uchannels;

	for (int i=0; i<numChannels; i++)
	{
		UserChannel *uc =  _ptGetUserChannel( channels[i] );
		if (uc) uchannels.push_back(uc);
	}
	if (uchannels.size()) 
	{
		return UserChannelManager::instance()->
			saveChannelsToFile( pt::String( filename ), uchannels.size(), &(uchannels[0]) );	
	}
	else return false;
}

//-----------------------------------------------------------------------------
// Write a file for persisting Channel use
//-----------------------------------------------------------------------------
PTuint64 PTAPI ptWriteChannelsFileToBuffer(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize)
{
	PTuint64 bufferHandle;

	std::vector <UserChannel *> uchannels;

	ptds::DataSourcePtr	dataSource = NULL;

	for (int i=0; i<numChannels; i++)
	{
		UserChannel *uc =  _ptGetUserChannel( channels[i] );

		if (uc)
		{
			uchannels.push_back(uc);
		}
	}

	pt::String dummyFilename(PTRMI::URL::PT_PTMY);
	dummyFilename += pt::String(L"://File.uch");

	if (uchannels.size()) 
	{
		 PTres res = UserChannelManager::instance()->saveChannelsToFile(dummyFilename, uchannels.size(), &(uchannels[0]), &dataSource);	
	}
	else 
	{
		return NULL;
	}

	bufferHandle	= 0;
	buffer			= 0;
	bufferSize		= 0;

	ptds::DataSourceMemory *dataSourceMemory;

	if(dataSourceMemory = dynamic_cast<ptds::DataSourceMemory *>(dataSource))
	{
		ptds::DataSource::Size size;

		buffer = dataSourceMemory->getBuffer(size);

		bufferHandle = reinterpret_cast<PTuint64>(dataSource);

		bufferSize = static_cast<PTuint64>(size);
	}


	return bufferHandle;
}
//-----------------------------------------------------------------------------
// Write a file for persisting Channel use
//-----------------------------------------------------------------------------

PTvoid PTAPI ptReleaseChannelsFileBuffer(PTuint64 bufferHandle)
{
	ptds::DataSourcePtr dataSource = reinterpret_cast<ptds::DataSourcePtr>(bufferHandle);

	if(dataSource)
	{
		ptds::dataSourceManager.deleteDataSource(dataSource);
	}
}

//-----------------------------------------------------------------------------
// Set the folder for OutofCore channels to use - defaults to temp folder
//-----------------------------------------------------------------------------
PTres PTAPI ptSetChannelOOCFolder( const PTstr foldername )
{
	wchar_t filename[MAX_PATH];

	bool res = false;

	/* try to write a file into this folder using the same method as OOCFile */ 
	::GetTempFileName( foldername, L"uch", 0, filename );	// Generate a temporary file name

	ptds::FilePath fp(filename);
	ptds::DataSourcePtr fhandle = ptds::dataSourceManager.openForReadWrite(&fp);	// open for read

	if (fhandle)
		res = true;
	
	ptds::dataSourceManager.close(fhandle);
	DeleteFile(filename);

	if (res)
	{
		OOCFile::setOOCFileFolder( pt::String( foldername ));
		return setLastErrorCode( PTV_SUCCESS );
	}
	return setLastErrorCode( PTV_FILE_NOT_ACCESSIBLE );
}
//-----------------------------------------------------------------------------
// Delete all channels from the user channels manager
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptDeleteAllChannels()
{
	UserChannelManager::instance()->eraseAllChannels();
}