/*----------------------------------------------------------*/ 
/* IndexStream.cpp											*/ 
/* Point Cloud Index stream Implementation file				*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#undef _VERBOSE

#include "PointoolsVortexAPIInternal.h"

#include <ptcloud2/indexstream.h>
#include <pt/timer.h>
#include <iostream>

#include <ptcloud2/scene.h>

#include <wildmagic/math/Wm5Plane3.h>
#include <wildmagic/math/Wm5ApprPlaneFit3.h>

#include <ptcmdppe/cmdprogress.h>
#include <utility/ptstr.h>

#include <ptcloud2/PointStreamFilter.h>

using namespace pcloud;
using namespace pt;

#ifdef POINTOOLS_API_INCLUDE
#undef debugAssertM
#define debugAssertM(a,b) //if(!a) std::cout << "assertion failed!!"  << std::endl 
#endif

#define IMAGE_BASE		10000
#define TRUNCATE_MULTIPLIER _truncationMultiplier

#define CLOUD_KEY(g,c) g * 1000 + c


// Pip Option Begin
pcloud::PointStreamFilter *pcloud::IndexStream::globalPointStreamFilter;
// Pip Option End


//----------------------------------------------------------------------------
// IndexStream constructor
//----------------------------------------------------------------------------
IndexStream::IndexStream()
{	
	_cloud = 0;
	_image = 0;
	_pager = 0;
	_groupidx = 0;
	_writeblock = 0;
	_readblock = 0;
	_normalpager = 0;
	_normalreadblock = 0;
	_rescaleIntensities = false;
	_truncationMultiplier = 10000;
	_clouds = 0;
	_group = 0;
	_pointBytes = 0;
	_cloudSpec = 0;

	memset(_intensityHist, 0, sizeof(_intensityHist));
															// Default to a normal (non root) stream for use without PointStreamFilter
	setRootStream(false);
}
IndexStream::~IndexStream()
{
	try
	{
		if (_pager) delete _pager;
		if (_normalpager) delete _normalpager;	
		if (_writeblock) delete _writeblock;
		if (_readblock) delete _readblock;
		if (_normalreadblock) delete _normalreadblock;

		for (int i = 0; i < _groups.size(); i++)
		{
			if (CloudGroup* group = _groups[i])
			{
				for (int j = 0; j < group->clouds.size(); j++)
				{
					if (CloudInfo* info = group->clouds[j])
						delete info;
				}
				group->clouds.clear();
				delete group;
			}
		}
		_groups.clear();
	}
	catch (...) {}
}
IndexStream::CloudGroup *IndexStream::setGroup(int g)
{
	_groupidx = g;
	_group = _groups[g];
	_clouds = &_group->clouds;
	return _group;
}
int IndexStream::addGroup(bool combine, float tolerance, bool gen_normals, float normal_quality, const wchar_t*name)
{
	if(applyPointStreamFilter())
//		return globalPointStreamFilter->addGroup(combine, tolerance, gen_normals, normal_quality, name);
		return globalPointStreamFilter->addGroup(combine, tolerance, false,       normal_quality, name);

	_group = new CloudGroup;
	_group->combine = combine;
	_group->normal_quality = normal_quality;
	_group->generate_normals = gen_normals;
	_group->tolerance = tolerance;
	_group->xbounds.clear();

	if (_truncationMultiplier == 0)
	{
		if (tolerance > 0)
		{
			_truncationMultiplier = tolerance * 1e6;
			/* remove inaccuraccy */ 
			_truncationMultiplier /= 10;
			_truncationMultiplier *= 10;
		}
		else
		{
			_truncationMultiplier = 100000;
		}
	}

	if (name) ptstr::copy(_group->name, name, 64);
	else _group->name[0] = '\0';

	_clouds = &_group->clouds;
	_groups.push_back(_group);
	
	_groupidx = _groups.size() - 1;
	return _groupidx;
}


bool IndexStream::applyPointStreamFilter(void) const
{
	return getRootStream() && globalPointStreamFilter && (globalPointStreamFilter->isEmpty() == false);
}

//----------------------------------------------------------------------------
// start write stream
//----------------------------------------------------------------------------
bool IndexStream::startStream(const wchar_t *filename)
{
	setAddedPointCount(0);

	if(applyPointStreamFilter())
		return globalPointStreamFilter->startStream(filename);

	/*create file*/ 
	_filePath.setPath(filename);
	_filePath.setExtension(L"pod_tmp");

	_pager = new ptds::BlockPager(_filePath.path());

	return _pager->fileValid();
}
//----------------------------------------------------------------------------
// finish writing stream
//----------------------------------------------------------------------------
bool IndexStream::closeStream()
{
	if (_image)
	{
		bool res = writeImage(_image);
		delete _image;
		_image = 0;
		if (!res)
			return false;
	}

	/*center cloud bounding boxes and add this offset to the registration matrix	*/ 
	/*shift large part of coords to matrix to prevent lage coords from hitting gl*/ 
	int i;
	for (i=0; i<_groups.size(); i++)
	{
		setGroup(i);
		CloudGroup *gr = const_cast<CloudGroup*>(group());
		gr->xbounds.clear();

		for (int j=0; j<_clouds->size(); j++)
		{
			CloudInfo *ci = (*_clouds)[j];
			if (ci->numPoints < 2) continue;

		//	if (ci->bounds.center().length() > 1000)
			if (!ci->truncation.is_zero())
			{
				vector3d c = ci->bounds.center();
				
				ci->bounds.translateBy(-c);

				ci->xbounds.translateBy(vector3(
					ci->truncation.x * TRUNCATE_MULTIPLIER, 
					ci->truncation.y * TRUNCATE_MULTIPLIER, 
					ci->truncation.z * TRUNCATE_MULTIPLIER));

				ci->matrix.translate(vector4d(
					(double)c.x + (double)(ci->truncation.x * TRUNCATE_MULTIPLIER), 
					(double)c.y + (double)(ci->truncation.y * TRUNCATE_MULTIPLIER), 
					(double)c.z + (double)(ci->truncation.z * TRUNCATE_MULTIPLIER), 0)
					);
				ci->offset = c;
			}
			else ci->offset.set(0,0,0);

			gr->xbounds.expandBy(ci->xbounds);
		}
	}
	if (_groups.size()) setGroup(0);

	/* process intensity histo and take off upper 10% of values */ 
	unsigned int mx=0;
	int mxh = 0;
	/*check for mis interpreted intensity */ 
	_intensityBounds.makeEmpty();
	
	for (i=0;i<255;i++)
	{
		if (_intensityHist[i] > mx)
		{
			mx = _intensityHist[i];
			mxh = i;
		}
			
	}
	
	int val;
	mx *= 0.05;
	
	for (i=0; i<255; i++)
	{
		val = i*255-32768;
		if (_intensityHist[i] > mx) _intensityBounds.expand(&val);
	}
	
	/* this can fail if intensities are actually 0-255, this shold fix it mostly */ 
	if (_intensityBounds.size(0) <= 1)
	{
		int upper_bound = _intensityBounds.lower(0) + 255;
		_intensityBounds.expand( &upper_bound );
	}

	/* update Metadata */ 
	//_metaData.audit.import_settings.channels_imported  =
	_metaData.audit.import_date = time(NULL);
	_metaData.audit.import_settings.compression_tolerance = group()->tolerance;
	_metaData.audit.import_settings.import_options = 0;

	if (group()->generate_normals) 
		_metaData.audit.import_settings.import_options |= MetaImportSettings::GenerateNormals;
	
	if (group()->combine)
		_metaData.audit.import_settings.import_options |= MetaImportSettings::CombineClouds;

	if (_rescaleIntensities)
		_metaData.audit.import_settings.import_options |= MetaImportSettings::ScaleIntensities;

	uint spec = 0;

	for (i=0; i< group()->clouds.size(); i++)
	{
		spec |= group()->clouds[i]->spec;

		if ( group()->clouds[i]->spec & PCLOUD_CLOUD_INTENSITY)
			spec |= 1 >> pcloud::PCloud_Intensity;

		if ( group()->clouds[i]->spec & PCLOUD_CLOUD_RGB)
			spec |= 1 >> pcloud::PCloud_RGB;
	}

	_metaData.audit.import_settings.channels_imported |= spec;
	
	/* dump structure, calling close() before deleting allows the number of write block errors to be checked */ 
	bool res = true;
	if (_writeblock)
	{
		_writeblock->close();
		if (_writeblock->numErrors())
			res = false;
	}
	delete _writeblock;

	_writeblock = 0;
	
	return true;
}
//------------------------------------------------------------------------------------------------
// Add a calibration scan image to the pod
//------------------------------------------------------------------------------------------------
// this is undistorted here and resaved
//------------------------------------------------------------------------------------------------
/* add an image to the last cloud / scan position */ 
void	IndexStream::addCalibratedImageToCloud(const char *filename, const mmatrix4d &extrinsic, 
	double cx, double cy, double fx, double fy, 
	double k1, double k2, double k3, double k4, 
	double p1, double p2)
{
	assert(_cloud);

	if (!_cloud) return;

	Calibration calib;
	calib.k1 = k1;
	calib.k2 = k2;
	calib.k3 = k3;
	calib.k4 = k4;
	calib.p1 = p1;
	calib.p2 = p2;
	calib.setIntrinsicParameters(fx, fy, cx, cy);
	calib.extrinsic = extrinsic;

	ScanImage scanimage( pt::String(filename), calib);

	char file[260];
	char newfile[260];
	int l = strlen(filename);

	strncpy(file, filename, 260);

	/* get the extension */ 
	char ext[8];
	strncpy(ext, &filename[l-4], 4);

	/* chop off extension */ 
	file[l-5] = '\0';

	/* append the word undistorted at the end */ 
	sprintf(newfile, "%s_undistorted.%s", file, ext);
	scanimage.undistort(newfile);

	_cloud->images.push_back(scanimage);
}
/* add an image to the last cloud / scan position */ 
void	IndexStream::addCallibratedImageToCloud(const ScanImage &si)
{
	assert(_cloud);
	if (!_cloud) return;

	char file[260];
	char newfile[260];
	int l = si.filepath().length();

	strncpy(file, si.filepath().c_str(), 260);

	/* get the extension */ 
	char ext[8];
	strncpy(ext, &file[l-3], 3);
	ext[3] = '\0';

	/* chop off extension */ 
	file[l-4] = '\0';

	/* append the word undistorted at the end */ 
	sprintf(newfile, "%s_undistorted.%s", file, ext);
	const_cast<ScanImage*>(&si)->undistort(newfile);

	_cloud->images.push_back(si);
}
//------------------------------------------------------------------------------
// add cloud
//------------------------------------------------------------------------------
bool IndexStream::addCloud(uint cloud_spec, const mmatrix4d *mat, uint ibound, uint jbound, const wchar_t*name)
{
	if(applyPointStreamFilter())
	{
		globalPointStreamFilter->addCloud(cloud_spec, mat, ibound, jbound, name);
		return true;
	}

	assert( _pager );
	assert( _clouds );

	if (!_pager || !_clouds ) return false;

	_cloud = new CloudInfo;
	_cloud->bounds.clear();
	_cloud->ibound = ibound;
	_cloud->jbound = jbound;
	_cloud->spec = cloud_spec;
	_cloud->numPoints = 0;
	_cloud->_orderedpos = 0;
	_cloud->truncation.zero();
	
	if (name) ptstr::copy(_cloud->name, name, 128);
	else _cloud->name[0] = L'\0';

	if (mat) memcpy(&_cloud->matrix, mat, sizeof(mmatrix4d)); 
	else _cloud->matrix = mmatrix4d::identity();

	if (_image)
	{
		bool res = writeImage(_image);
		delete _image;
		_image = 0;
		if (!res)
			return false;
	}
	if (_writeblock) 
	{
		_writeblock->close();
		int errors = _writeblock->numErrors();
		delete _writeblock;
		if (errors)
			return false;
	}
	_writeblock = _pager->newWriteBlock(PCLOUD_STREAM_BLOCKSIZE, CLOUD_KEY(_groupidx, _clouds->size()), "Cloud", true);
	if (!_writeblock)
		return false;
	
	//std::cout << "+c";
	_cloud->bytesPerPoint = sizeof(pt::vector3);
	_cloud->bytesPerPoint += cloud_spec & PCLOUD_CLOUD_INTENSITY ? sizeof(short) : 0;
	_cloud->bytesPerPoint += cloud_spec & PCLOUD_CLOUD_RGB ? 3 : 0;
	_cloud->bytesPerPoint += cloud_spec & PCLOUD_CLOUD_NORMAL ? sizeof(pt::vector3) : 0;
	 
	if (ibound >0 && jbound >0 && (ibound * jbound < 64000000) )
	{
		_image = new PointsImage(ibound,jbound,_clouds->size(), 
			cloud_spec & PCLOUD_CLOUD_ROW_MAJOR ? true : false);
	}

	_clouds->push_back(_cloud);

	return (_writeblock->numErrors() == 0);
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IndexStream::restartCloudPass()
{
	if(applyPointStreamFilter())
	{
		globalPointStreamFilter->restartCloudPass();
		return;
	}

	assert( _writeblock );
	assert( _cloud );
	if ( !_writeblock || !_cloud ) return;

	/* reposition the writeblock back to the start */ 
	_writeblock->startWriteThroughPass();
	_cloud->_orderedpos=0;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IndexStream::addPassPointColour(const ubyte *rgb)
{
	if(applyPointStreamFilter())
	{
	//	globalPointStreamFilter->addPassPointColour(geomd, rgb);
	//	return;
	}

	assert( _writeblock );
	assert( _cloud );
	if ( !_writeblock || !_cloud ) return;


	debugAssertM(_cloud->spec & PCLOUD_CLOUD_RGB, "Cloud must have RGB Spec");

	/* bypass geometry*/ 
	_writeblock->writeThroughAdvance(sizeof(pt::vector3));
	
	if (_cloud->spec & PCLOUD_CLOUD_INTENSITY)
	{
		_writeblock->writeThroughAdvance(sizeof(short));
	}
	/* create static array sp write through can get size */ 
	ubyte col[3] = {rgb[0], rgb[1], rgb[2]};
	_writeblock->writeThrough(col);

	if (_cloud->spec & PCLOUD_CLOUD_NORMAL)
	{
		_writeblock->writeThroughAdvance(sizeof(pt::vector3s));
	}
	if (_cloud->spec & PCLOUD_CLOUD_ORDERED)
	{
		_writeblock->writeThroughAdvance(sizeof(_cloud->_orderedpos));
	}
	_cloud->_orderedpos++;
}	
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IndexStream::addPassPointIntensity(const short *intensity)
{
	if(applyPointStreamFilter())
	{
	//	globalPointStreamFilter->addPassPointIntensity(geomd, intensity);
	//	return;
	}

	assert( _writeblock );
	assert( _cloud );
	if ( !_writeblock || !_cloud ) return;

	debugAssertM(_cloud->spec & PCLOUD_CLOUD_INTENSITY, "Cloud must have Intensity Spec");

	/* bypass geometry*/ 
	_writeblock->writeThroughAdvance(sizeof(pt::vector3));
	
	/* create static array sp write through can get size */ 
	_writeblock->writeThrough(*intensity);

	if (_cloud->spec & PCLOUD_CLOUD_RGB)
	{
		_writeblock->writeThroughAdvance(sizeof(ubyte)*3);
	}
	if (_cloud->spec & PCLOUD_CLOUD_NORMAL)
	{
		_writeblock->writeThroughAdvance(sizeof(pt::vector3s));
	}
	if (_cloud->spec & PCLOUD_CLOUD_ORDERED)
	{
		_writeblock->writeThroughAdvance(sizeof(_cloud->_orderedpos));
	}
	_cloud->_orderedpos++;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
__int64 IndexStream::writeStreamPosition()
{
	if(applyPointStreamFilter())
		return globalPointStreamFilter->writeStreamPosition();

	return _writeblock->tracker()->position();
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IndexStream::addNull()
{ 
	if(applyPointStreamFilter())
	{
		globalPointStreamFilter->addNull();
		return;
	}


	_cloud->_orderedpos++;
}
//------------------------------------------------------------------------------
// add Point
//------------------------------------------------------------------------------
bool IndexStream::addPoint(const pt::vector3d &geomd, const ubyte * const rgb, 
						   const short * const intensity, const pt::vector3s * const normal, const ubyte * const category)
{
	if(applyPointStreamFilter())
	{
		setAddedPointCount(getAddedPointCount() + 1);
		return globalPointStreamFilter->addPoint(geomd, rgb, intensity, normal);
	} 

	assert( _writeblock );
	assert( _cloud );
	assert( _clouds && _clouds->size() );
	if ( !_writeblock || !_cloud ) return false;

	static pt::vector3d geomdn;
	
	if (geomd.is_zero())
	{
		addNull();
		return true;
	}
	/* sanity check */ 
	if (geomd.is_nan() || 
		geomd.x > 1e50 || geomd.x < -1e50 ||
		geomd.y > 1e50 || geomd.y < -1e50 ||
		geomd.z > 1e50 || geomd.z < -1e50)
	{
		return true;
	}
	if (!_cloud->numPoints)
	{
		/* check size of values for truncation*/ 
		_cloud->truncation.x = geomd.x / TRUNCATE_MULTIPLIER;
		_cloud->truncation.y = geomd.y / TRUNCATE_MULTIPLIER;
		_cloud->truncation.z = geomd.z / TRUNCATE_MULTIPLIER;
	}
	/* remove any large component */ 
	geomdn.x = geomd.x - _cloud->truncation.x * TRUNCATE_MULTIPLIER;
	geomdn.y = geomd.y - _cloud->truncation.y * TRUNCATE_MULTIPLIER;
	geomdn.z = geomd.z - _cloud->truncation.z * TRUNCATE_MULTIPLIER;
	pt::vector3 geom(geomdn);

	_cloud->bounds.expand(geomdn);
	pt::vector3 xgeom;
	_cloud->matrix.vec3_multiply_mat4f(geom, xgeom);
	_group->xbounds.expand(xgeom);
	_cloud->xbounds.expand(xgeom);

	if (_image) _image->set(_cloud->_orderedpos, _cloud->numPoints);
	_cloud->numPoints++;
	_cloud->_orderedpos++;

	/*geometry into buffer*/ 
	if (!_writeblock->write(geom) || _writeblock->numErrors())
		return false;
	 
	if (_cloud->spec & PCLOUD_CLOUD_INTENSITY)
	{
		debugAssertM(intensity, "Intensity specified but not provided");

		int inteni;
		short intens;

		if(intensity)
		{
			inteni = *intensity;
			intens = *intensity;
		}
		else
		{
			inteni = 100;
			intens = 100;
		}

		_writeblock->write(intens);

		_intensityBounds.expand(&inteni);
		inteni += 32768;
		inteni /= 256;
		
		++_intensityHist[inteni];
	}
	if (_cloud->spec & PCLOUD_CLOUD_RGB)
	{
		debugAssertM(rgb, "RGB specified but not provided");
		if (!rgb)
		{
			ubyte blank[] = {0,0,0};
			if (!_writeblock->write(blank, 3)) return false;
		}
		else if (!_writeblock->write(rgb, 3)) return false;
	}
	if (_cloud->spec & PCLOUD_CLOUD_NORMAL)
	{
		debugAssertM(normal, "Normal specified but not provided");

		if (!normal)
		{
			pt::vector3s blank(0,0,1);
			if (!_writeblock->write(blank)) return false;
		}
		else if (!_writeblock->write(*normal)) return false;
	}
	if (_cloud->spec & PCLOUD_CLOUD_CLASSIFICATION)
	{
		debugAssertM(category, "Classification specified but not provided");
		if (!category)
		{
			ubyte blank;
			if (!_writeblock->write(blank)) return false;
		}
		else if (!_writeblock->write(*category)) return false;
	}
	if (_cloud->spec & PCLOUD_CLOUD_ORDERED)
	{
		if (!_writeblock->write(_cloud->_orderedpos-1)) return false;
	}

	setAddedPointCount(getAddedPointCount() + 1);

	if (_writeblock->numErrors())
		return false;

	return true;
} 
//------------------------------------------------------------------------------
// start read stream
//------------------------------------------------------------------------------
bool IndexStream::startReadStream()
{
	//std::cout << "Starting read stream" << std::endl;
	return true;	
}
//------------------------------------------------------------------------------
// start read stream
//------------------------------------------------------------------------------
void IndexStream::closeReadStream()
{
	//std::cout << "Closing read stream" << std::endl;
	try
	{
		if (_readblock) delete _readblock;
		if (_normalreadblock) delete _normalreadblock;
		delete _pager;
		if (_normalpager) delete _normalpager;
	}
	catch(...) {}
	_readblock = 0;
	_normalreadblock = 0;
	_pager = 0;
	_normalpager = 0;
}
//------------------------------------------------------------------------------
// read Point
//------------------------------------------------------------------------------
int IndexStream::readPoint(pt::vector3 &geom, ubyte *rgb, short *intensity, pt::vector3s *normal, ubyte *category)
{
	assert( _clouds->size() );
	assert( _readblock );

	if (!_readblock) return 0;
	if (!_clouds ) return 0;
	if (!_cloud ) return 0;

	CloudInfo *ci = _cloud;
	int pos = -1;

	/*geometry into buffer*/ 
	if (!_readblock->read(geom) || _readblock->numErrors())
	{
		return Scene::ReadPntStreamFailure;
	}

	/*correction for normalized coords*/ 
	geom.x -= ci->offset.x;
	geom.y -= ci->offset.y;
	geom.z -= ci->offset.z;

	double intd;

	if (ci->spec & PCLOUD_CLOUD_INTENSITY)
	{
		if (intensity)
		{
			_readblock->read(*intensity);
			if (_rescaleIntensities)
			{
				int inteni = *intensity;
				_intensityBounds.normalizedValue(&inteni, &intd, -32767, 32767);
				if (intd > 32767) intd = 32767;
				if (intd < -32767) intd = -32767;
				
				/* reduce contrast */ 
				intd *= 0.5;
				*intensity = intd;
			}
		}
		else _readblock->advance(sizeof(short)); 
	}
	if (ci->spec & PCLOUD_CLOUD_RGB)
	{
		if (rgb) _readblock->read(rgb, 3);
		else  _readblock->advance(3);
	}
	if (ci->spec & PCLOUD_CLOUD_NORMAL)
	{
		if (_normalreadblock)
		{
			if (normal) _normalreadblock->read(*normal);
			else  _normalreadblock->advance(sizeof(pt::vector3s));
		}
		else if (normal) _readblock->read(*normal);
		else _readblock->advance(sizeof(pt::vector3s)); 
	}
	if (ci->spec & PCLOUD_CLOUD_CLASSIFICATION)
	{
		if (category) _readblock->read(*category);
		else _readblock->advance(1);
	}
	if (ci->spec & PCLOUD_CLOUD_ORDERED)
	{
		unsigned int p;
		_readblock->read(p);
		pos = p;
	}

	if (_readblock->numErrors() || (_normalreadblock && _normalreadblock->numErrors()))
	{
		return Scene::ReadPntStreamFailure;
	}

	return pos;
}
//------------------------------------------------------------------------------
// read Cloud
//------------------------------------------------------------------------------
const IndexStream::CloudInfo *IndexStream::readCloud(int idx)
{
	if (idx > _clouds->size()) return 0;

	if (_readblock) delete _readblock;
	_readblock = _pager->newReadBlock(CLOUD_KEY(_groupidx, idx), "Cloud", true);
	if (_normalpager) 
	{
		if (_normalreadblock) delete _normalreadblock;
		_normalreadblock = _normalpager->newReadBlock(CLOUD_KEY(_groupidx, idx), "Normal", true);
	}

	/*move file pointer*/ 
	_cloud = (*_clouds)[idx];

	if ((_readblock && _readblock->numErrors())
		|| (_normalreadblock && _normalreadblock->numErrors()))
	{
		return 0;
	}

	return _cloud;
}
bool IndexStream::readImage(PointsImage *image, int cloud_idx)
{
	assert( _pager );
	assert( image );

	if (!_pager || !image) return false;

	if (_pager->readBlock(cloud_idx + IMAGE_BASE, image->data(), "CloudImage"))
	{
#ifdef _VERBOSE
		std::cout << "Reading Image(" << image->ibound() << "x" << image->jbound() 
			<<  "  " << image->validPoints() << " points )" << std::endl;
#endif

		return true;
	}
	return false;
}
//------------------------------------------------------------------------------
// Write scan order information
//------------------------------------------------------------------------------
bool IndexStream::writeImage(PointsImage *image)
{
	int errors = 0;

	assert( _image );
	if (!_image || !image) return false;

#ifdef _VERBOSE
	std::cout << "Writing Image(" << image->ibound() << "x" << image->jbound() <<  "  " 
		<< image->validPoints() << " points )" << std::endl;
#endif

	if (_writeblock) 
	{
		_writeblock->close();
		errors =_writeblock->numErrors();
		delete _writeblock;
		if (errors)
			return false;
	}
	_writeblock = _pager->newWriteBlock(PCLOUD_STREAM_BLOCKSIZE, image->cloud_idx() + IMAGE_BASE, "CloudImage", true);
	if (!_writeblock)
		return false;

	int bytes = image->ibound() * image->jbound() * sizeof(int);
	_writeblock->write(_image->data(), bytes, "CloudImage");
	errors = _writeblock->numErrors();
	delete _writeblock;
	_writeblock = 0;

	return (errors == 0);
} 
//------------------------------------------------------------------------------
// build normals for ordered cloud
//------------------------------------------------------------------------------
bool IndexStream::_buildNormals(int cloud_idx, pt::vector3s* _normals, bool transform, float quality)
{
	assert( _clouds );

	if (!_clouds) return false;

	struct Normal
	{
		void set(float x, float y, float z)
		{
			_x = (short)(32768 * x);
			_y = (short)(32768 * y);
			_z = (short)(32768 * z);
		}	
		short _x;
		short _y;
		short _z;
	};
	Normal *normals = reinterpret_cast<Normal*>(_normals);
	int nh_size = quality * 10;
	if (nh_size < 2) nh_size = 2;
	if (nh_size > 13) nh_size = 13;

	CloudInfo *ci = (*_clouds)[cloud_idx];

	if (!ci->ibound || !ci->jbound || !ci->numPoints) return false;

	/*read image*/ 
	PointsImage image(ci->ibound, ci->jbound, cloud_idx);
	if (!readImage(&image, cloud_idx)) return false;

	pt::vector3* points=0;
	try
	{
		points = new pt::vector3[ci->numPoints];
	}
	catch(...)
	{
		return false;
	}
	/*read these points in*/ 
	readCloud(cloud_idx);
	
	uint i;
	for (i=0; i<ci->numPoints; i++)
		readPoint(points[i],0,0,0);

	/*load data*/ 
	/*allocate channel*/ 
	int cpm[150];
	Wm5::Vector3f _v[150];
	Wm5::Vector3f _n, _o, _n1, _origin(0,0,0);

	unsigned int errors =0 ;
#ifdef _VERBOSE
	std::cout << "Calculating Normals (" << ci->ibound << "x" << ci->jbound << ")...";
#endif
	int count = 0;
	int sparse = 0;

	mmatrix4d mat = ci->matrix;
	mat(3,0) = 0;
	mat(3,1) = 0;
	mat(3,2) = 0;
	mat.invert();
	mat.transpose();

	float threshold = 0.1f;

	vector3 zvec(0,0,0);
	vector3 cen;

	//mat.vec3_multiply_mat4f(zvec);

	for (i=0; i<ci->ibound * ci->jbound; i++)
	{
		int p = image.get(i);

		if (p>=0)
		{
			int c =  image.getNeighbourhood(i, cpm, nh_size);
			if (c>3)
			{
				// validate points
				//crude check to see if all the points are the same
				//this is required to avoid exception occuring in Wm5::method  
				//also invalidate points with z distance

				int valid_points = 0;
				cen = points[p];

				vector3 diff(points[p]);
				diff *= c;

				float za, zb;

				for (int j=0; j<c; j++)
				{
					za = cen.z;
					zb = points[cpm[j]].z;

					if (fabs(za - zb) < threshold)
					{
						memcpy(&_v[valid_points++], &points[cpm[j]], sizeof(float)*3);

						diff.x -= _v[j].X();
						diff.y -= _v[j].Y();
						diff.z -= _v[j].Z();
					}
				}
				if (valid_points > 3 && !diff.is_zero())
				{
					count++;
					Wm5::Plane3f plane = Wm5::OrthogonalPlaneFit3(valid_points, _v);
					_n = plane.Normal;
					_o = plane.Constant * plane.Normal;

					_origin = -_o;
					_origin.Normalize();
					_n.Normalize();
					
					/*check side for flip*/ 
					float dot = _n.Dot(_origin);
					if ( dot < 0)
					{
						_n.X() = -_n.X();
						_n.Y() = -_n.Y();
						_n.Z() = -_n.Z();
					}
					if (transform) 
					{
						mat.vec3_multiply_mat4f(_n);
						_n.Normalize();
					}
				}
				else
				{
					/*point toward scanner by default*/ 
					_n = Wm5::Vector3f(zvec.x, zvec.y, zvec.z);
					sparse++;
				}
			}
			else
			{
                _n = Wm5::Vector3f(zvec.x, zvec.y, zvec.z);
				sparse++;
			}
			normals[p].set(_n.X(), _n.Y(), _n.Z()); 
		}
	}	
	////post process - use neighbour normal for zero normals
	//for (i=0; i<ci->ibound * ci->jbound; i++)
	//{
	//	int p = image.get(i);
	//	if (p>=0 && normals[p].is_zero())
	//	{
	//		int c =  image.getNeighbourhood(i, cpm, nh_size);
	//		for (int j=0; j<c; j++)
	//		{
	//			if (cpm[j] >= 0) points[cpm[j]]
	//		}
	//	}	
	//}
#ifdef _VERBOSE
	std::cout << "done "<< std::endl;
	std::cout << "    " << count << " normals generated" << std::endl;
	std::cout << "    " << sparse << " points have insufficient neighbourhood" << std::endl;
	std::cout << "    " << ci->numPoints - sparse - count << " points not present in image" << std::endl;
#endif
	delete [] points;
	
	return true;
}
void IndexStream::generateNormals()
{
	for (int g=0; g<numGroups(); g++)
	{
		IndexStream::CloudGroup *group = setGroup(g);

		if (group->generate_normals)
			buildNormals(group->combine, group->normal_quality);
	}
}
//------------------------------------------------------------------------------
// 
// 
//------------------------------------------------------------------------------
void IndexStream::setRootStream(bool isRoot)
{
	_rootStream = isRoot;
}
//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool IndexStream::getRootStream(void) const
{
	return _rootStream;
}

//------------------------------------------------------------------------------
// BuildNormals - must only call after writing is done
//------------------------------------------------------------------------------
void IndexStream::buildNormals(bool transform, float quality)
{
	if(applyPointStreamFilter())
		return globalPointStreamFilter->buildNormals(transform, quality);

	assert( _clouds );
	assert( _pager );
	if (!_clouds || !_pager ) return;

	if (! _clouds ) return;
	/*build all normal channels to page file*/ 
	if (!_normalpager)
	{
		wchar_t filepath[PT_PATH_SIZE-4];
		ptstr::copy(filepath, _pager->filepath(), PT_PATH_SIZE-4);
		wcscat(filepath, L".nml");

		_normalpager = new ptds::BlockPager(filepath);
	}
	ptapp::CmdProgress progress("Building Normals...", 0, _clouds->size());

	for (int i=0; i<_clouds->size(); i++)
	{
		CloudInfo *ci = (*_clouds)[i];
		if (!ci->hasNormals() && ci->ibound && ci->jbound && ci->numPoints > 1)
		{
			/*build channels and record normal pointer in _cloud_info*/ 

			pt::vector3s *normals = 0;
			
			try {
				normals = new pt::vector3s[ci->numPoints];
			}
			catch(...)
			{
				return;
			}
			if (_buildNormals(i, normals, transform, quality))
			{
				ptds::WriteBlock *wb = _normalpager->newWriteBlock(sizeof(pt::vector3s) * ci->numPoints,CLOUD_KEY(_groupidx, i), 0, true);
				wb->write(normals, sizeof(pt::vector3s) * ci->numPoints);
				delete wb;
				ci->spec |= PCLOUD_CLOUD_NORMAL;
			}
			delete [] normals;
		}
		progress.inc(1);
	}
}

unsigned __int64 IndexStream::getNumCloudPoints() const
{
	__int64 numPoints = 0;

	if(applyPointStreamFilter())
	{
		std::vector<IndexStream*> streams;

		globalPointStreamFilter->getIndexStreams(streams);

		for (int i=0; i<streams.size(); i++)
		{
			int numClouds = streams[i]->numClouds();

			for (int j=0; j<numClouds; j++)
			{
				numPoints += streams[i]->cloudInfo(j)->numPoints;
			}
		}
	}
	else if (_clouds)
	{
		for (int i=0; i<_clouds->size(); i++)
		{
			numPoints += cloudInfo(i)->numPoints;
		}
	}
	return numPoints;
}