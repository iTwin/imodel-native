/*----------------------------------------------------------*/ 
/* PointCloud.h												*/ 
/* Point Cloud Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTCLOUD_DEFINITION_H
#define POINTCLOUD_DEFINITION_H 1

#include <vector>
#include <pt/scenegraph.h>
#include <ptcloud2/pcloud.h>
#include <ptcloud2/voxel.h>

namespace pcloud
{
	typedef int64_t PointCloudGUID;

	class PCLOUD_API UserTransform
	{
	public:
		UserTransform()
		{
			_translation.zero();
			_eulers.zero();
			_scale.set(1,1,1);
			_matrix = mmatrix4d::identity();
			_flags = 0;
		}

		void pushGL();
		void updateMatrix();

		const mmatrix4d &matrix()			const { return _matrix; }
		const mmatrix4d &compiledMatrix()	const { return _matrix; }

		const pt::vector3d	&translation()	const { return _translation; }
		const pt::vector3d	&eulers()		const { return _eulers; }
		const pt::vector3d	&scale()		const { return _scale; }

		void useMatrix( const mmatrix4d &mat ) { if (!(_flags & 0x02)) _flags |= 0x03; _matrix = mat; }
		void useTransform() { _flags &= ~ 0x02; _flags |= 0x01; }

		void setTranslation(const pt::vector3d &v)	{ _translation = v; _flags |= 0x01; }
		void setEulers(const pt::vector3d &v)	{ _eulers = v; _flags |= 0x01; }
		void setScale(const pt::vector3d &v)	{ _scale = v; _flags |= 0x01; }

		void dirtyMatrix()	{ _flags |= 0x01; }
		bool isDirty() const { return _flags & 0x01 ? true : false; }

	private:
		pt::vector3d	_translation;
		pt::vector3d	_eulers;
		pt::vector3d	_scale;
		mmatrix4d		_matrix;
		mmatrix4d		_cmpmatrix;
		unsigned int	_flags;
	};
	class Scene;
	/* Point cloud is root Node*/ 
	class PCLOUD_API PointCloud : public pt::Object3D 
	{
	public:
		PointCloud(const wchar_t *id, const Scene *scene, float compressionTolerance=0);
		~PointCloud();

		void buildFromRaw(int n, float *geom, ubyte *rgb=0, short *intensity=0, short *norms=0);
		bool initialize();

		void setRoot(Node*n);
		
		void randomizeData();
		void generateGuid();
		bool setGuid( PointCloudGUID guid );
		void clearData();

		const Scene *scene() const					{ return _scene; }

		PointCloudGUID guid() const					{ return _guid;	}
		pt::Guid objectGuid() const;

		Node* root()								{ return _root; }
		const Node* root() const					{ return _root; }

		const std::vector<Voxel*> &voxels() const	{ return _voxels; }
		std::vector<Voxel*> &voxels() 				{ return _voxels; }

		void filepointer(int64_t &p) { _filepointer = p; }
		int64_t filepointer() const { return _filepointer; }
		
		uint ibound() { return _ibound; }
		uint jbound() { return _jbound; }

		bool hasChannel(int ch) const { return _channels[ch]; }
		
		int64_t currentMemUseage() const;
		int64_t fullMemUseage() const { return _memused; }
		int64_t numPoints() const { return _numpoints; }

		// returns distance from seek pnt
		double findNearestPoint(const pt::vector3d &seek_pnt, pt::vector3d &nearest, pt::CoordinateSpace cs) const;
		const Voxel* findContainingVoxel(const pt::vector3d &seek_pnt, pt::CoordinateSpace cs) const;
		
		// returns distance along ray ie t
		double findIntersectingPoint(const pt::Rayf &ray, pt::vector3d &nearest, float tolerance, pt::CoordinateSpace cs) const;
		
		const wchar_t *typeDescriptor()	const	{ return L"Point Cloud"; }
		const char *className() const			{ return s_className(); }
		static const char *s_className()		{ return "Point Cloud"; }

		void visitInterface(pt::InterfaceVisitor *visitor) const;
		int setProperty(const char *id, const pt::Variant &v);
		int getProperty(const char *id, pt::Variant &v);

		void pushUserTransformation() const;

		const mmatrix4d &userTransformationMatrix() const { return _userTransform.matrix(); }
		const UserTransform &transform() const			{ return _userTransform; }
		UserTransform &transform()						{ return _userTransform; }

		float compressionTolerance() const				{ return _compressionTolerance; }

		/* additional display information */ 
		bool displayUseGlobalPointSize() const			{ return _pointSize < 0.1f; }
		float displayPointSize() const					{ return _pointSize; }

		const unsigned char* overrideColor() const		{ return _baseColour; }
		void	overrideColor(unsigned char*rgb) 		{ _baseColour[0] = rgb[0]; _baseColour[1] = rgb[1]; _baseColour[2] = rgb[2]; }
		void	enableOverrideColor(bool enable)		{ _baseColour[3] = enable ? 255 : 0; }
		bool	isOverriderColorEnabled() const			{ return _baseColour[3] ? true : false; }

	protected:
		virtual void _computeBounds();
	private:
		friend class PodIO;

		Node *_root;
		std::vector <Voxel*> _voxels;

		PointCloudGUID _guid;
		int64_t _filepointer;

		int64_t _memused;
		int64_t _numpoints;

		float _compressionTolerance;

		uint _ibound;
		uint _jbound;

		UserTransform _userTransform;

		const Scene * _scene;
		bool _channels[MAX_CHANNELS];

		/* additional display info */ 
		float			_pointSize;
		unsigned char	_baseColour[4];
	};
}
#endif