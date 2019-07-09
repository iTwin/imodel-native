#include <ptengine/podCache.h>
#include <ptengine/voxelLoader.h>
#include <ptcloud2/scene.h>
#include <ptfs/iohelper.h>

using namespace pcloud;
using namespace vortex;

//-------------------------------------------------------------------------------------------------
PODCacheManager::PODCacheManager()
{
	wchar_t tempFolder[MAX_PATH];
	::GetTempPathW(MAX_PATH, tempFolder );

	m_folder = tempFolder;
}
PODCacheManager::~PODCacheManager() {}
//-------------------------------------------------------------------------------------------------
bool PODCacheManager::setCacheFolder( const ptfs::FilePath &folder )
{
	bool retval = false;

	if (::PathIsDirectoryW(folder.path()))
	{
		// attempt to write a file here using same method as cache file writer
		pt::String filename( folder.path());
		filename += pt::String(L"\\cache-test.podcache");

		ptfs::FileHandle h = ptfs::IO::openForWrite(ptfs::FilePath(filename.c_wstr()));
		if (ptfs::IO::validHandle(h))
		{
			int testval=1;
			if (ptfs::IO::writeBytes(h, testval)==sizeof(testval))
			{
				m_folder = folder;			
				retval = true;
			}
			ptfs::IO::close(h);
			ptfs::IO::deleteFile(ptfs::FilePath(filename.c_wstr()));
		}		
	}
	return retval;
}
//-------------------------------------------------------------------------------------------------
ptfs::FilePath PODCacheManager::generateCacheFilename( const pcloud::Scene *pod ) const
{
	pt::String podguid;
	podguid.format("%d.podcache", (int)pod->cloud(0)->numPoints());

	pt::String filename( m_folder.path() );
	filename += pt::String("\\") + podguid;
	return ptfs::FilePath(filename.c_wstr());
}
//-------------------------------------------------------------------------------------------------
// create the cache file
//-------------------------------------------------------------------------------------------------
bool PODCacheManager::createCacheFile( const pcloud::Scene *pod, uint maxBytes )
{
	uint64 fullPnts = pod->fullPointCount();

	//quick version, assume 9 bytes per point
	fullPnts *= 9;

	// approx uniform lod required to get this file size
	float lod = (float)maxBytes / fullPnts;

	if (lod > 0.1) return false;	// not worth having a cache file

	// round lod up to 1/255 boundary
	int byteLod = (lod * 255)+1;
	lod = (float)byteLod/255;

	// create file
	ptfs::FilePath file = generateCacheFilename( pod );

	ptfs::FileHandle h = ptfs::IO::openForWrite( file );
	
	// handle write failure
	if (!ptfs::IO::validHandle(h))
		return false;
	
	// pause points pager

	// use write buffering
	ptfs::Tracker tracker(h);
	ptfs::WriteBlock wb(h, 4194304, &tracker);
	
	const char *filetype ="PODCACHE";

	uint version = 1;
	uint numClouds = pod->numBranchObjects();
	uint dataNumBytes = 0;

	// WRITE HEADER
	int i, j;
	wb.write((void*)filetype, 8);
	wb.write(version);
	wb.write(numClouds);
	wb.insertPlaceholder(1);	// place holder for start of data
	wb.insertDataPlaceholder(2, sizeof(uint));	// place holder for num bytes to read

	uint dataPos = 0;
	uint totalNumPnts = 0;
	uint totalBytes = 0;

	// each cloud
	for (i=0; i<pod->numBranchObjects(); i++)
	{
		const PointCloud *pcloud = pod->cloud(i);
		wb.write(pcloud->guid());
		
		// channels present in this cloud and bytes per point
		uint channels = 0;
		uint bytesPerPnt = 0;

		for (int channelIndex = PCloud_Geometry; channelIndex<PCloud_Classification; channelIndex++)
		{
			if (pcloud->hasChannel(channelIndex))
			{			
				DataChannel *ch = pcloud->voxels()[0]->channel(channelIndex);

				if (ch)
				{
					channels |= (1 << channelIndex);
					bytesPerPnt += ch->typesize() * ch->multiple();
				}
			}
		}
		
		wb.write(channels);

		// voxels
		int numVoxels = pcloud->voxels().size();
		wb.write(numVoxels);

		// write the voxels
		for (j = 0; j<numVoxels; j++)
		{
			Voxel *vox = pcloud->voxels()[j];
			uint numPnts = lod * vox->fullPointCount();

			wb.write(dataPos);	// offset to the data for this voxel
			wb.write(numPnts);	// number of points int he channel data to be written

			dataPos += bytesPerPnt * numPnts;
			totalNumPnts += numPnts;
		}
	}
	wb.commit();

	wb.tracker()->placeReference(1);	// position start of channel data
	wb.tracker()->writePlacedData(2,&dataPos, sizeof(dataPos));	// note num of bytes for channel data
	wb.tracker()->writePlaceHolders();

	// WRITE DATA
	// each cloud
	for (int i=0; i<pod->numBranchObjects(); i++)
	{
		PointCloud *pcloud = const_cast<PointCloud*>(pod->cloud(i));
		int numVoxels = pcloud->voxels().size();

		// write the voxel data
		for (int j = 0; j<numVoxels; j++)
		{
			Voxel *vox = pcloud->voxels()[j];
			uint numPnts = lod * vox->fullPointCount();

			VoxelLoader vload(vox, lod*1.1f, true); // load upto lod

			for (int channelIndex = PCloud_Geometry; channelIndex<=PCloud_Classification; channelIndex++)
			{
				DataChannel *ch = vox->channel(channelIndex);
				if (ch && ch->data())	// should never fail
				{
					uint numBytesToWrite = ch->multiple() * ch->typesize() * numPnts;
					if (numBytesToWrite) wb.write(ch->data(), numBytesToWrite );
					totalBytes += numBytesToWrite;
				}					
			}
		}
	}
 	int numErrors = wb.numErrors();
	
	wb.close();
	ptfs::IO::close(h);

	return numErrors ? false : true;
}
//-------------------------------------------------------------------------------------------------
bool PODCacheManager::hasCacheFile( const pcloud::Scene *pod ) const
{
	ptfs::FilePath file = generateCacheFilename( pod );
	return file.checkExists();
}
//-------------------------------------------------------------------------------------------------
bool PODCacheManager::readCacheFile( const pcloud::Scene *pod )
{
	ptfs::FilePath file = generateCacheFilename( pod );
	ptfs::FileHandle h = 0;

	if (file.checkExists())
	{
		h = ptfs::IO::openForRead(file);

		if (ptfs::IO::validHandle(h))
		{
			ptfs::Tracker tracker(h);
			ptfs::ReadBlock rb(h, &tracker);

			uint version =0;
			uint numClouds = 0;

			char filetype[16];

			rb.read(filetype, 8);
			filetype[8] = 0;
			int i,j;

			// READ HEADER
			// check filetype
			if (strcmp(filetype, "PODCACHE" )==0)
			{
				rb.read( version );

				if (version == 1)
				{
					rb.read( numClouds );
			
					if (numClouds != pod->numBranchObjects())
					{
						goto HandleError;
					}
					ptfs::FilePointer ptrToData;

					// get FilePtr to data start
					rb.read( ptrToData );

					// get size of data to read
					uint numOfBytes = 0;
					rb.read( numOfBytes ); 

					// do sanity check on numBytes
					if (numOfBytes > 500e6) goto HandleError;

					
					ubyte *channelData = 0;
					
					try 
					{
						channelData = new ubyte [numOfBytes];
					}
					catch (std::bad_alloc)
					{
						goto HandleError;
					}
					// store current position
					ptfs::FilePointer fpos = rb.tracker()->position();

					// read in this block into mem in one read					
					ptfs::IO::movePointerTo(h, ptrToData);

					// read in 64k chunks
					int bytes;
					int chunkSize = 65536;
					for (bytes=0; bytes < numOfBytes; bytes+=chunkSize)
					{				
						int numBytesToRead = chunkSize;
						if (bytes + chunkSize > numOfBytes)
							numBytesToRead = numOfBytes % chunkSize;

						if (ptfs::IO::readBytes(h, &channelData[bytes], numBytesToRead ) != numBytesToRead)	// check for truncated read
						{
							delete [] channelData;					
							goto HandleError;
						}
					}
					// move file pointer back into position
					ptfs::IO::movePointerTo(h,fpos);

					// create cache structures;
					for (i=0; i<pod->numBranchObjects(); i++)
					{
						const pcloud::PointCloud *pcloud = pod->cloud(i);
						int64_t guid;
						rb.read(guid);

						if (guid == pcloud->guid())
						{
							uint numVoxels=0;
							uint channels=0;

							rb.read(channels);
							rb.read(numVoxels);

							for (int i=0; i<numVoxels; i++)
							{
								uint numPnts=0;
								uint dataPtr=0;

								rb.read(dataPtr);
								rb.read(numPnts);
								
								Voxel *voxel = pcloud->voxels()[i];		
								if (voxel->lodPointCount() > numPnts)
								{
									continue;
								}

								// read in channel data
								for (int ch = PCloud_Geometry; ch<=PCloud_Classification; ch++)
								{																		
									DataChannel *dc = voxel->channel(ch);
									if (dc)
									{									
										int numChBytes = dc->typesize() * dc->multiple() * numPnts;
										// read the channel data in
										{	
											dc->dump();
											dc->allocate(numPnts);
											memcpy( dc->data(), &channelData[dataPtr], numChBytes ); 
										}
										dataPtr += numChBytes;
									}
								}
								// real nasty, but its the only way for now!								
								const ubyte *clod = &(voxel->lodAsByte());
								ubyte ubyteLod = 255 * (float)numPnts/voxel->fullPointCount();
								*(const_cast<ubyte*>(clod)) = ubyteLod < 1 ? 1 : ubyteLod;
							}
						}
					}				
					delete [] channelData;
				}	//version == 1
			} // valid file
			ptfs::IO::close(h);
		}
		else return false; // unable to open file
	}
	else return false;

	return true;

HandleError:
	ptfs::IO::close(h);

//	rb.close();
	return false;
}
