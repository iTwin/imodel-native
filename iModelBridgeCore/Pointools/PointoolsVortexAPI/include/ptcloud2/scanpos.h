/*----------------------------------------------------------*/ 
/* PointCloud.h												*/ 
/* Point Cloud Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTCLOUD_SCANPOSITION_H
#define POINTCLOUD_SCANPOSITION_H 1

#include <ptcloud2/pointcloud.h>
#include <ptcloud2/pcimage.h>

namespace pcloud
{	
	class ScanPosition : public pt::Object3D
	{
	public:
		ScanPosition(const mmatrix4d &mat, const wchar_t *name) : Object3D(name)
		{
			_registration = mat;
			this->m_registration.matrix(mat);
		}
		void addCloud(PointCloud *cloud)
		{
			_clouds.push_back(cloud);	
		};
		void addImage(const ScanImage &image)
		{
			_images.push_back(image);
		}

		int numClouds() const { return static_cast<int>(_clouds.size()); }
		PointCloud *cloud(int i) { return _clouds[i]; }
		bool hasCloud(PointCloud *cloud)
		{
			for (unsigned int i=0; i<_clouds.size(); i++)
				if (_clouds[i] == cloud) return true;
			return false;
		}
		void world2ProjectSpace(const Object3D *project)
		{
			const mmatrix4d &prj = project->registration().invMatrix();
			m_registration.matrix(_registration.concat(prj));
		}
		void getLocation(pt::vector3d &loc) const 
		{
			loc.set(static_cast<float>(_registration(3, 0)), static_cast<float>(_registration(3, 1)), static_cast<float>(_registration(3, 2)));
		}

		int numImages() const { return (int)_images.size(); }
		const ScanImage &image(int i) const { return _images[i]; }
		ScanImage &image(int i) { return _images[i]; }

	protected:
		void _computeBounds() { localBounds().clearBounds(); projectBounds().clearBounds(); };
	private:
		mmatrix4d					_registration;
		std::vector<PointCloud *>	_clouds;
		std::vector<ScanImage>		_images;
	};
}
#endif