#ifndef POINTOOLS_SCENE_CLASS_MANAGER_H
#define POINTOOLS_SCENE_CLASS_MANAGER_H

#include <pt/SceneGraph.h>
#include <utility/ptstr.h>

namespace ptgl { class Viewport; }
namespace pt
{
class ParameterMap;

enum SceneAction
{
	SceneActionClear				=0x01,
	SceneActionUnload				=0x02,
	SceneActionLoad					=0x03,
	SceneActionMerge				=0x04,
	SceneActionHide					=0x05,
	SceneActionShow					=0x06,
	SceneActionExport				=0x07,
	SceneActionCompleteIteration	=0x08
};
class CCLASSES_API SceneClassInfo
{
public:
	SceneClassInfo() 
		: m_infolevel(0), m_objectInfo(0), m_objectInfoTail(0), m_objectInfoDirty(true), m_objectInfoUpdated(true) {};
	~SceneClassInfo()
	{
		clear();	
	};
	
	class ObjectInfo
	{
	public:
		ObjectInfo(const wchar_t* _name, int _key, void *_obj, short _imgID, bool _prop, const pt::DisplayInfo *_di)
			: key (_key), obj((Object*)_obj), img(_imgID), isproperty(_prop), dinfo(_di), level(0), next(0) 
		{ ptstr::copy(name, _name, 255); }
		~ObjectInfo() { if (next) delete next; }
		wchar_t			name[256];
		Object*			obj;
		int				key;
		const pt::DisplayInfo	*dinfo;

		short			img;
		bool			isproperty;
		unsigned char	level;

		ObjectInfo *next;
	};
	int addImage(const char* file)
	{ 
		m_images.push_back(file); 
		return m_images.size()-1; 
	}
	void dirtyInfo()		{ m_objectInfoDirty = true;	}
	void undirtyInfo()		{ m_objectInfoDirty = false; }
	bool isDirty() const	{ return m_objectInfoDirty; }

	void setUpdated()		{ m_objectInfoUpdated = true; }
	void unsetUpdated()		{ m_objectInfoUpdated = false; }
	bool isUpdated() const	{ return m_objectInfoUpdated; }

	void pushInfoLevel()	{ m_infolevel++; }
	void popInfoLevel()		{ m_infolevel--; }
	
	void clear();

	void addInfo(const wchar_t* _name, int _key, void *_obj, short _imgID, bool _prop, const pt::DisplayInfo *_di);

	int				numImages() const	{ return m_images.size(); }
	const char*		image(int i) const	{ return m_images[i]; }

	const ObjectInfo*	popObjectInfo()	const	{ const_cast<SceneClassInfo*>(this)->m_objectInfoUpdated = false; return m_objectInfo; }
	const ObjectInfo*	objectInfo() const		{ return m_objectInfo; }

private:
	std::vector<const char*> m_images;

	void addInfo(ObjectInfo* info) 
	{
		if (m_objectInfoTail) m_objectInfoTail->next = info; 
		m_objectInfoTail = info; 
		if (!m_objectInfo) m_objectInfo = info; 
		info->level = m_infolevel;

		m_objectInfoUpdated = true;
		dirtyInfo();
	}

	int					m_infolevel;
	ObjectInfo*			m_objectInfo;
	ObjectInfo*			m_objectInfoTail;
	/* track object tree state. Dirty flag is for external use only */ 
	bool				m_objectInfoDirty;
	/* track update state, cleared by popObjectInfo */ 
	bool				m_objectInfoUpdated;
};
//-------------------------------------------------------------------------
// Scene Class Manager, class level functions for derived scene classes
//-------------------------------------------------------------------------
// Override to manage Scene classes
//-------------------------------------------------------------------------
class CCLASSES_API SceneClassManager : public Group3D
{
public:
	SceneClassManager(const wchar_t*id, const wchar_t *typeDesc);
	virtual ~SceneClassManager() { };
	
	typedef int OBJKEY;
	typedef std::vector<OBJKEY> OBJKEYS;

	static int loadSceneClassManagers();

	static bool registerImportHandler(const ptds::FileType &ftype, const wchar_t *typeDesc);
	static bool registerExportHandler(const ptds::FileType &ftype, const wchar_t *typeDesc);
	static bool registerInfoImage(const ptds::FilePath &path);

	static int numImportFileTypes(const wchar_t *typeDesc=0);
	static int numExportFileTypes(const wchar_t *typeDesc=0);
	static const ptds::FileType &importFileType(int i, const wchar_t *typeDesc=0);
	static const ptds::FileType &exportFileType(int i, const wchar_t *typeDesc=0);

	/** number of Currently registered SceneClassManagers */ 
	static int numClassManagers();

	/** get all the SceneClassManagers */ 
	static int getClassManagers(SceneClassManager** managers);
	
	/** get a SceneClassManager from the classes Type Description */ 
	static SceneClassManager *manager(const wchar_t *typeDesc);

	/** import a scene from persistant store - check file type always, return 0 if not supported */ 
	virtual Scene3D	*importScene(const ptds::FilePath &path)=0;

	/** export a scene to persistant store - check file type always, return 0 if not supported */ 
	virtual bool exportScene(const ptds::FilePath &path) { return false; };

	/** string describing data type of scene - should be human readable */ 
	virtual const wchar_t *typeDescriptor() const	{ return L"_SceneClassManager"; }

	/** string describing underlaying scenegraph object class - should be human readable */ 
	const char *objectClass() const			{ return "SceneClassManager"; }

	/** string describing class Name - should be human readable */ 
	virtual const char *className() const			{ return "SceneClassManager"; }
	
	/** indicates Managers responsibility to manage memory for scenes */ 
	virtual bool managesSceneMemory() const		{ return true; }

	/** remove scene from underlaying database, free memory: manager deletes memory*/ 
	virtual void removeScene(Scene3D *scene, bool freeMemory) {};

	/** parameter map that stores settings for this class */ 
	virtual ParameterMap *settingsParameterMap() { return 0; }

	/** script used to build settings interface */ 
	virtual const char* settingsScript() const { return 0; }; 

	/** script used to build settings interface */ 
	virtual void settingsUpdate() {}; 

	const SceneClassInfo*		infoTree() const { return &m_infoTree; }

	virtual int action(SceneAction a, OBJKEYS keys) { return 0; };

	/** support for object reference resolving */
	virtual Object3D* findObjectByGuid( const Guid &guid );
	virtual Scene3D* findSceneByName( const pt::String &name );	
	virtual Scene3D* findSceneByFilepath( const ptds::FilePath &path);
	virtual pt::Object3D* findObjectByKey( const pt::ObjectKey &key );	
	
protected:
	/** add scene loaded outside the project into the project graph */ 
	void addSceneToProject(Scene3D *scene);

	SceneClassInfo				m_infoTree;
};
}

#endif