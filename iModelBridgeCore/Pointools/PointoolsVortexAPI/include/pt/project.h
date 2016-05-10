#ifndef POINTOOLS_ROOT_PROJECT_NODE_H
#define POINTOOLS_ROOT_PROJECT_NODE_H

#include <pt/scenegraph.h>

#include <map>

namespace pt
{
//-------------------------------------------------------------------------
// Project, a collection of Scenes | operations on data | root of graph
//-------------------------------------------------------------------------
class CCLASSES_API Project3D : public Group3D
{
public:
	Project3D() : Group3D(L"Project",0) { m_registration.coordinateSpace(ProjectSpace); };
	virtual ~Project3D(){ destroy(); };

	virtual const wchar_t *typeDescriptor() const { return L"Project"; }

//	Scene3D* scene(const wchar_t* id);
	Scene3D* scene(int i) { return m_scenesv[i]; }

	bool isSceneValid(Scene3D *scene);

	bool removeScene(const wchar_t* id);
	bool removeScene(Scene3D* sc);
	Scene3D* addScene(const ptds::FilePath &path);
	void addScene(Scene3D* sc);

	void removeAllScenes();

	double nearestPoint(const vector3 &nr, vector3 &pt);
	void initialize();
	void destroy();

	static Project3D &project();

	bool setProjectSpaceOrigin( const vector3d &origin );
	void optimizeCoordinateSpace();

	int numObjects() const { return static_cast<int>(m_scenesv.size()); }
	const Object3D* object(int i) const { return m_scenesv[i]; }
	Object3D* object(int i) { return m_scenesv[i]; }

#ifdef HAVE_OPENGL
    void drawGL(uint32 drawmode, int millisecs, const ptgl::Viewport *viewport);
#endif

	virtual void diagnostic(Output *output, bool recursive) const;

	void project2WorldSpace(const vector3 &v0, vector3 &v1) const	{
		m_registration.matrix().vec3_multiply_mat4f(v0,v1);	}

	void world2ProjectSpace(const vector3 &v0, vector3 &v1) const	{
		m_registration.invMatrix().vec3_multiply_mat4f(v0,v1);	}

	void project2WorldSpace(const vector3d &v0, vector3d &v1) const	{
		m_registration.matrix().vec3_multiply_mat4(v0,v1);	}

	void world2ProjectSpace(const vector3d &v0, vector3d &v1) const	{
		m_registration.invMatrix().vec3_multiply_mat4(v0,v1);	}

	void project2WorldSpace(const vector3 &v0, vector3d &v1) const	{
		m_registration.matrix().vec3_multiply_mat4(vector3d(v0),v1);	}

	void world2ProjectSpace(const vector3d &v0, vector3 &v1) const	{
		vector3d v1d;
		m_registration.invMatrix().vec3_multiply_mat4(v0,v1d);	
		v1.x = static_cast<float>(v1d.x);v1.y = static_cast<float>(v1d.y);v1.z = static_cast<float>(v1d.z);
	}
private:
	void updateSceneArray();

	typedef std::multimap<std::wstring, Scene3D*> SCENEMAP;
	std::vector<Scene3D*> m_scenesv;
	SCENEMAP m_scenes;
};
}

#endif