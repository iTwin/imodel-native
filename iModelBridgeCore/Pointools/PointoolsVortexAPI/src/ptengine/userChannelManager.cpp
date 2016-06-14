#include "PointoolsVortexAPIInternal.h"
#include <ptengine/userchannels.h>
#include <ptengine/pointsScene.h>
#include <ptengine/engine.h>
#include <ptedit/pointVisitors.h>
#include <ptedit/editState.h>
#include <pt/datatreeIO.h>

#include <ptds/DataSource.h>

using namespace pcloud;
using namespace pointsengine;

#define BYTES_PER_ELEMENT ((m_multiple * m_bitsize / 8) ? (m_multiple * m_bitsize / 8) : 1)
#define UCH_FILE_HEADER_RESERVE		1024	// bytes reserved in UC file header
#define UCH_FILE_CHANNEL_RESERVE	256		// bytes reserved in UC file per channel
//-----------------------------------------------------------------------------
namespace detail
{
	// Singleton instance
	//-------------------------------------------------------------------------
	UserChannelManager * g_manager=0;
	
	// Datastore type
	//-------------------------------------------------------------------------
	typedef std::map<pt::String, UserChannel*> MapChannels;
 
	// Channels
	//-------------------------------------------------------------------------
	MapChannels g_channels;

	// Observer to POD file close and open
	//-------------------------------------------------------------------------
	struct UserChannelsFileObserver : public pointsengine::FileObserver
	{
		void sceneRemove( pcloud::Scene *scene )
		{	
			MapChannels::iterator i = g_channels.begin();

			while (i != g_channels.end())
			{
				i->second->remFromChannel( scene );
				++i;
			}
		}
		void sceneAdd( pcloud::Scene *scene ) 
		{ 
			MapChannels::iterator i = g_channels.begin();

			while (i != g_channels.end())
			{
				UserChannel *uc = i->second;
				uc->addToChannel( scene );
				++i;
			}
		}
	};
	// File Observer instance
	//-------------------------------------------------------------------------
	UserChannelsFileObserver g_fileObserver;
}
using namespace detail;

//-----------------------------------------------------------------------------
// User Channel Manager : Constructor
//-----------------------------------------------------------------------------
UserChannelManager::UserChannelManager()
{
	_renderOffsetChannel = 0;
	_renderTextureChannel = 0;
	_renderRGBChannel = 0;
	_renderZChannel = 0;
	_renderOffsetBlend = 0;

	/* first time */ 
	thePointsScene().addFileObserver( &g_fileObserver );
}
//-----------------------------------------------------------------------------
// User Channel Manager : Destructor
//-----------------------------------------------------------------------------
UserChannelManager::~UserChannelManager()
{	
	eraseAllChannels();
}
//-----------------------------------------------------------------------------
// User Channel Manager : erase All Channels ie clear
//-----------------------------------------------------------------------------
void UserChannelManager::eraseAllChannels()
{
	MapChannels::iterator i = g_channels.begin();

	while (i != g_channels.end() )
	{
		delete i->second;
		++i;
	}
	g_channels.clear();

	_renderOffsetChannel = 0;
	_renderRGBChannel = 0;
	_renderTextureChannel = 0;
	_renderZChannel = 0;
}
//-----------------------------------------------------------------------------
// Singleton instance
//-----------------------------------------------------------------------------
UserChannelManager* UserChannelManager::instance() 
{ 
	if (!g_manager) g_manager = new UserChannelManager(); 
	return g_manager;
}
//-----------------------------------------------------------------------------
// create a user channel
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::createChannel( const pt::String &name, uint bitsize, 
											   uint multiple, void* defaultVal, uint flags )
{
	try
	{
		UserChannel *channel = new UserChannel( name, bitsize, multiple, defaultVal, (ubyte)flags );
		g_channels.insert( MapChannels::value_type( name, channel ) );
		return channel;
	}
	catch (...) { return 0; }
}

//-----------------------------------------------------------------------------
// copy a user channel
//-----------------------------------------------------------------------------
UserChannel *pointsengine::UserChannelManager::copyChannel(UserChannel *sourceChannel, const pt::String &destName, UserChannelFlags destFlags)
{
	if(sourceChannel == NULL)
	{
		return NULL;
	}

	try
	{
		UserChannel *newChannel = new UserChannel(sourceChannel, destName, destFlags);

		if(newChannel)
		{
			g_channels.insert( MapChannels::value_type(destName, newChannel));
		}

		return newChannel;
	}
	catch (...)
	{ 

	}

	return NULL;
}


//-----------------------------------------------------------------------------
// remove a user channel
//-----------------------------------------------------------------------------
bool UserChannelManager::eraseChannel( UserChannel *channel )
{
	MapChannels::iterator i = g_channels.find( channel->name() );
	if ( i != g_channels.end() )
	{
		if (_renderOffsetChannel == channel)
			_renderOffsetChannel = 0;
		if (_renderRGBChannel == channel)
			_renderRGBChannel = 0;
		if (_renderTextureChannel == channel)
			_renderTextureChannel = 0;
		if (_renderZChannel == channel) 
			_renderZChannel = 0;
	
		delete i->second;
		g_channels.erase( i );
		
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
// retur na channel by its name
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::channelByName( const pt::String &name )
{
	MapChannels::iterator i = g_channels.find( name );
	return i == g_channels.end() ? 0 : i->second;	
}
//=============================================================================
// Rendering
//=============================================================================
// render the channel as a vector offset per point
//-----------------------------------------------------------------------------
bool UserChannelManager::renderAsOffset( UserChannel* channel, float blend )
{
	if (!channel) return false;
	if (channel->multiple() == 3 && channel->bitsPerValue() == 32)
	{
		_renderOffsetBlend = blend;
		_renderOffsetChannel = channel;
		//theRenderEngine().enableUserChannelRender(true);
		return true;
	}	
	else _renderOffsetChannel = 0;

	return false;
}
//-----------------------------------------------------------------------------
// render offset blend in value
//-----------------------------------------------------------------------------
float UserChannelManager::renderOffsetBlendValue() const 
{ 
	return _renderOffsetBlend; 
}
//-----------------------------------------------------------------------------
// render as RGB
//-----------------------------------------------------------------------------
bool UserChannelManager::renderAsRGB( UserChannel* channel )
{
	if (!channel) return false;
	if (channel->multiple() == 3 && channel->bitsPerValue() == 8)
	{
		_renderRGBChannel = channel;
		return true;
	}	
	else _renderRGBChannel =0;
	return false;
}
//-----------------------------------------------------------------------------
// render as a texture coordinate
//-----------------------------------------------------------------------------
bool UserChannelManager::renderAsTexture( UserChannel* channel, int stride )
{
	if (!channel) return false;
	if (channel->multiple() == 1 && channel->bitsPerValue() == 32)
	{
		_renderTextureChannel = channel;
		return true;
	}	
	else _renderTextureChannel = 0;
	return false;
}
//-----------------------------------------------------------------------------
// render as a shift in Z value
//-----------------------------------------------------------------------------
bool UserChannelManager::renderAsZShift( UserChannel* channel, int stride )
{
	if (!channel) return false;
	if (channel->multiple() == 1 && channel->bitsPerValue() == 32)
	{
		_renderZChannel = channel;
		return true;
	}	
	else _renderZChannel = 0;
	return false;
}

//-----------------------------------------------------------------------------
// Getters for rendering methods
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::renderOffsetChannel()
{
	return _renderOffsetChannel;
}
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::renderTextureChannel()
{
	return _renderTextureChannel;
}
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::renderRGBChannel()
{
	return _renderRGBChannel;
}
//-----------------------------------------------------------------------------
UserChannel *UserChannelManager::renderZChannel()
{
	return _renderZChannel;
}
//=============================================================================
// Persistence
//=============================================================================
// Save all channels to file
int UserChannelManager::saveChannelsToFile( const pt::String &filename, int numChannels, UserChannel** channels, ptds::DataSourcePtr *dataSourceMemory)
{
	std::vector<UserChannel*> channelsToWrite;
	// Gather channels to write
	if (numChannels > 0 && channels)
	{
		for (int i=0; i<numChannels; i++)
		{
			channelsToWrite.push_back( channels[i] );
		}
	}
	else
	{
		MapChannels::iterator i = g_channels.begin();	

		while ( i != g_channels.end())
		{
			channelsToWrite.push_back( i->second );
			++i;
		}
	}
	numChannels = static_cast<int>(channelsToWrite.size());
	if (!numChannels) return PTV_FILE_NOTHING_TO_WRITE;

	ptds::FilePath fp(filename.c_wstr());
	ptds::DataSourcePtr fhandle = ptds::dataSourceManager.openForWrite(&fp);	// open for write
	if (!fhandle)
		return PTV_FILE_FAILED_TO_CREATE;

	// create a data tree and save
	pt::datatree::Branch *dtree = new pt::datatree::Branch("UserChannels");
	dtree->addNode("version", (int)2);
	dtree->addNode("num_channels", numChannels);
	pt::datatree::Branch *br_channels = dtree->addBranch("channels");

	for (size_t i=0; i<channelsToWrite.size(); i++)
	{
		UserChannel *channel = channelsToWrite[i];

		// add channel to tree 
		pt::datatree::Branch *br = br_channels->addIndexedBranch();

		if (!channel->writeToBranch( br ))
		{
			ptds::dataSourceManager.close(fhandle);
			delete dtree;
			return PTV_FILE_WRITE_FAILURE;
		}
	}

	pt::datatree::writeBinaryDatatree( dtree, fhandle );
	
	fhandle->close();

	if(dataSourceMemory)
	{
		*dataSourceMemory = fhandle;
	}
	else
	{
		ptds::dataSourceManager.deleteDataSource(fhandle);
	}

	delete dtree;

	return PTV_SUCCESS;
}
//-----------------------------------------------------------------------------
// load channels in, point clouds will be referenced by GUIDs if valid
//-----------------------------------------------------------------------------
int UserChannelManager::loadChannelsFromFile( const pt::String &filename, std::vector<UserChannel*> &channels, ptds::DataSource::Data *sourceBuffer, ptds::DataSource::DataSize sourceBufferSize)
{
	ptds::FilePath fp(filename.c_wstr());
	ptds::DataSourcePtr fhandle = ptds::dataSourceManager.openForRead(&fp, sourceBuffer, sourceBufferSize);	// open for read

	if (!fhandle)
	{
		ptds::dataSourceManager.close(fhandle);
		return PTV_FILE_NOT_ACCESSIBLE;
	}

	int err = PTV_SUCCESS;

	pt::datatree::Branch *root = new pt::datatree::Branch("UserChannels");

	if (pt::datatree::readBinaryDatatree( root, fhandle ))
	{
		int version = 0;
		int num_channels = 0;

		if (	!root->getNode("version", version)
			||	!root->getNode("num_channels", num_channels))
			err = PTV_FILE_READ_FAILURE;
		
		if (version <2) 
		{
			err =  PTV_FILE_VERSION_NOT_HANDLED;
		}
		
		pt::datatree::Branch *br_channels = root->getBranch("channels");
		
		if (br_channels) 
		{
			for (int i=0; i<num_channels; i++)
			{
				pt::datatree::Branch *br = br_channels->getIndexedBranch(i+1);

				if (br)
				{
					try
					{					
						UserChannel *uchannel = UserChannel::createFromBranch( br );	
						if (uchannel)
						{
							g_channels.insert( MapChannels::value_type( uchannel->name(), uchannel ) );	// add channel
							channels.push_back( uchannel );
						}
					}
					catch (std::bad_alloc) 
					{
						ptds::dataSourceManager.close(fhandle);
						return PTV_OUT_OF_MEMORY; 
					}	// memory allocation could cause failure
				}

			}
		}
		else err = PTV_FILE_READ_FAILURE;

	}
	else err = PTV_FILE_READ_FAILURE;

	fhandle->close();
	ptds::dataSourceManager.deleteDataSource(fhandle);

	delete root;

	return err;
}


UserChannel *UserChannelManager::createChannelFromLayers( const pt::String &name, const pcloud::Scene *scene )	// copies the edit layers into a user channel
{
	bool allscenes = scene ? false : true;

	// stop all paging first - no locking on a voxel level
	try
	{
		UserChannel *channel = new UserChannel( name, 8, 1, 0, 0, const_cast<pcloud::Scene *>(scene) );
		g_channels.insert( MapChannels::value_type( name, channel ) );

		for (int s=0; s<thePointsScene().size();s++)
		{
			if (allscenes) 
			{
				scene = thePointsScene()[s];
			}
			else if (s) break;
		
			channel->addToChannel(scene);

			// populate channel with scene  
			for (uint i=0; i<scene->size(); i++)
			{
				const pcloud::PointCloud *cloud = scene->cloud(i);

				// populate this cloud channel
                for (size_t v = 0; v < cloud->voxels().size(); v++)
				{
					pcloud::Voxel *vx = cloud->voxels()[v];
					VoxelChannelData *vdata = channel->voxelChannel( vx );


					if (vdata)
					{
						pcloud::DataChannel *dc = vx->channel(pcloud::PCloud_Filter);
						
						// if no channel just set user0 / user1
						if (dc && vx->layers(1))	// we only want layer data, not selection
						{
							ubyte layerval=0;

							for (unsigned int p=0;p<dc->size(); p++)
							{
								dc->getval(layerval, p);		// get value from channel
								layerval = layerval &~ SELECTED_PNT_BIT;
								vdata->setVal(p, &layerval);		// write to user channel
							}
						}
						// write flags / layers
						vdata->setUser0( vx->layers(0) );
						vdata->setUser1( vx->layers(1) );
					}	
				}
			}
		}
		return channel;
		// 
	}
	catch (...) { return 0; }
}
extern uint ptedit::g_editApplyMode;

//-------------------------------------------------------------------------------------------------
bool		UserChannelManager::applyChannelToLayers( const UserChannel *channel, pcloud::Scene *scene )		// reverse opp, applies the channel data to layers
{
	//read the data from the channel, then propogate flags upwards
	bool retval = false;
	bool allscenes = scene ? false : true;

	bool flagged_only = false;
	
	if (ptedit::g_editApplyMode & ptedit::EditIntentFlagged) 
			flagged_only = true;

	for (int s=0; s<thePointsScene().size();s++)
	{
		if (allscenes)
		{
			scene = thePointsScene()[s];
		}
		else if (s) break;
		
		// must check for exclusion
		if ( ptedit::g_state.isSceneExcluded( scene ) )
		{
			continue;
		}
		
		// populate channel with scene  
		for (uint i=0; i<scene->size(); i++)
		{
			pcloud::PointCloud *cloud = scene->cloud(i);

			// populate this cloud channel
            for (size_t v = 0; v < cloud->voxels().size(); v++)
			{
				pcloud::Voxel *vx = cloud->voxels()[v];
				const VoxelChannelData *vdata = channel->voxelChannel( vx );

				if (flagged_only && !vx->flag( pcloud::Flagged ))
					continue;

				// selection state is not stored and should be cleared from the cloud
				vx->flag( pcloud::PartSelected, false );
				vx->flag( pcloud::WholeSelected, false );

				if (vdata)
				{
					retval = true;
					
					vx->layers(0) = (ubyte) vdata->getUser0();
					vx->layers(1) = (ubyte) vdata->getUser1();

					if (vx->layers(1) && vdata->getData() )
					{
						pcloud::DataChannel *dc = vx->channel(pcloud::PCloud_Filter);

						if (!dc)
						{
							vx->buildEditChannel();
							dc = vx->channel(pcloud::PCloud_Filter);

							if (!dc) continue;	
						}

						dc->allocate(vdata->getNumPoints());
						ubyte layerval=0;

						for (unsigned int p=0;p<vdata->getNumPoints(); p++)
						{
							vdata->getVal(p, &layerval);
							dc->set(p, &layerval);
						}
						vx->numPointsEdited( 0 ); // this must be zero and not all points otherwise further editing will not get processed
					}
					else
					{
						// destroy any existing edit channel if there is no corresponding one being loaded
						vx->destroyEditChannel();
					}
				}					
			}
		}
	}
	// propogate flags
	ptedit::TraverseScene::consolidateFlags();
	ptedit::TraverseScene::consolidateLayers();

	return retval;
}

