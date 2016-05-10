#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <pt/unicodeconversion.h>
#include <pt/ptmath.h>
#include <Bentley/BeFileName.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/Array.h>
#endif

/*-----------------------------------------------------------------------*/
/* FilePath class														 */
/*-----------------------------------------------------------------------*/

#define PT_MAXPATH 512

#ifndef NO_DATA_SOURCE_SERVER
namespace PTRMI { class DataBuffer; }
#endif



namespace ptds
{


class FilePath
{
public:

	FilePath()
	{
		m_parent = 0;
		m_bAbsolute = true;
        BeFileName tmp(L"(empty)");
        m_path = tmp;
	}

	FilePath(const wchar_t* path)
	{
		m_parent = 0;
		m_bAbsolute = true;
        BeFileName tmp(path);
        m_path = tmp;
	}

	FilePath(const char* path)
	{
		m_parent = 0;
		m_bAbsolute = true;
        BeFileName tmp(path);
        m_path = tmp;
	}

	~FilePath() {}

	inline bool operator == (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) == 0;
	}

	inline bool operator < (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) < 0;
	}

	inline bool operator > (const FilePath &b) const
	{
		return wcscmp(path(), b.path()) > 0;
	}

	inline FilePath& operator = (const FilePath &p)
	{
        m_path = p.m_path;

		m_bAbsolute = p.m_bAbsolute;
		m_parent = p.m_parent;

		return (*this);
	}

	void setPath(const wchar_t* path)
	{
        BeFileName tmp(path);
        m_path = tmp;
	}

	void stripExtension()
	{
        WString dev, dir, name;
        m_path.ParseName(&dev, &dir, &name, nullptr);

        BeFileName pathWithoutExtension(dev.c_str(), dir.c_str(), name.c_str(), nullptr);
        m_path = pathWithoutExtension;
	}

	void stripFilename()
	{
        WString dev, dir;
        m_path.ParseName(&dev, &dir, nullptr, nullptr);

        BeFileName pathWithoutFileName(dev.c_str(), dir.c_str(), nullptr, nullptr);
        m_path = pathWithoutFileName;
	}

	void setFilenameOnly(const wchar_t *fn)
	{
        WString dev, dir, name, ext;
        m_path.ParseName(&dev, &dir, &name, &ext);
        BeFileName pathWitNewName(dev.c_str(), dir.c_str(), fn, ext.c_str());
        m_path = pathWitNewName;
	}

	const wchar_t* filename() const
	{
        return m_path.GetFileNameAndExtension().c_str();
	}

	void directory(wchar_t *dst) const
	{
        WString dev, dir;
        m_path.ParseName(&dev, &dir, nullptr, nullptr);
        BeFileName pathWithoutFileName(dev.c_str(), dir.c_str(), nullptr, nullptr);
        BeStringUtilities::Wcsncpy(dst, PT_MAXPATH, pathWithoutFileName.c_str());
	}

	void fulldirectory(wchar_t *dst) const
	{
		if (m_bAbsolute)
		{
			directory(dst);
			return;
		}
		wchar_t d[PT_MAXPATH];
		dst[0] = '\0';

		const FilePath *par = m_parent;

		std::vector <const FilePath*> ancestors;

		while (par)
		{
			ancestors.push_back(par);
			par = par->parent();
		}
		std::reverse(ancestors.begin(), ancestors.end());
		std::vector <const FilePath*>::iterator it = ancestors.begin();

		while (it != ancestors.end())
		{
			m_parent->directory(d);
            BeFileName dstPath(dst);
            dstPath.AppendToPath(d);
            BeStringUtilities::Wcsncpy(dst, PT_MAXPATH, dstPath.c_str());
			++it;
		}
	}

	const wchar_t* path() const
	{
    return m_path.c_str();
	}

	void fullpath(wchar_t *dst) const
	{
		if (m_bAbsolute)
		{
			BeStringUtilities::Wcsncpy(dst, PT_MAXPATH, m_path.c_str());
			return;
		}
		fulldirectory(dst);
        BeFileName dstPath(dst);
        dstPath.AppendToPath(m_path);
        BeStringUtilities::Wcsncpy(dst, PT_MAXPATH, dstPath.c_str());
	}

	void setExtension(const wchar_t*ext)
	{
        WString dev, dir, name;
        m_path.ParseName(&dev, &dir, &name, nullptr);
        BeFileName pathWitNewExtension(dev.c_str(), dir.c_str(), name.c_str(), nullptr);
        pathWitNewExtension.AppendExtension(ext);
        m_path = pathWitNewExtension;
	}

	const FilePath *parent() const { return m_parent; }

	void setAbsolute()
	{
		m_bAbsolute = true;
	}

	void setRelative()
	{
		m_bAbsolute = false;
	}

	bool isRelative() const
	{
		return !m_bAbsolute;
	}

	bool isEmpty() const
	{
        return m_path.IsEmpty();
	}

	void setParent(const FilePath *f)
	{
		m_parent = f;
	}

	bool checkExists() const
	{
		// Note: Handle structured storage here ?

		wchar_t p[PT_MAXPATH];
		fullpath(p);
		return (_waccess(p,0) == 0) ? true : false;
	}

	static bool checkExists(const std::wstring &p)
	{
		return (_waccess(p.c_str(),0) == 0) ? true : false;
	}

	static bool isSlash(wchar_t c)
	{
		return (c == '\\') || (c == '/');
	}

	static void parseFilename(const std::wstring&  filename, std::wstring& root, std::vector<std::wstring>& path, std::wstring &base, std::wstring &ext)
	{
		std::wstring f = filename;

		root = L"";
		path.clear();
		base = L"";
		ext = L"";

		if (f == L"")
		{
				// Empty filename
			return;
		}

			// See if there is a root/drive spec.
		if ((f.size() >= 2) && (f[1] == ':'))
		{

			if ((f.size() > 2) && isSlash(f[2]))
			{
					// e.g.  c:\foo
				root = f.substr(0, 3);
				f = f.substr(3, f.size() - 3);

			} 
			else 
			{

					// e.g.  c:foo
				root = f.substr(2);
				f = f.substr(2, f.size() - 2);
			}

		}
		else if ((f.size() >= 2) & isSlash(f[0]) && isSlash(f[1]))
		{
				// e.g. //foo
			root = f.substr(0, 2);
			f = f.substr(2, f.size() - 2);
		}
		else if(isSlash(f[0]))
		{
			root = f.substr(0, 1);
			f = f.substr(1, f.size() - 1);
		}

			// Pull the extension off
		{
				// Find the period
			size_t i = f.rfind(L'.');

				// Make sure it is before a slash!
			size_t j = pt::iMax((int)f.rfind(L'/'), (int)f.rfind(L'\\'));
			if ((i != std::wstring::npos) && (i > j))
			{
				ext = f.substr(i + 1, f.size() - i - 1);
				f = f.substr(0, i);
			}
		}

			// Pull the basename off
		{
				// Find the last slash
			size_t i = (size_t)pt::iMax((int)f.rfind(L'/'), (int)f.rfind(L'\\'));

			if (i == std::wstring::npos)
			{
					// There is no slash; the basename is the whole thing
				base = f;
				f    = L"";
			}
			else if ((i != std::wstring::npos) && (i < f.size() - 1))
			{
				base = f.substr(i + 1, f.size() - i - 1);
				f    = f.substr(0, i);

			}
		}

			// Parse what remains into path.
		size_t prev, cur = 0;

		while (cur < f.size())
		{
			prev = cur;

				// Allow either slash
			size_t i = f.find(L'/', prev + 1);
			size_t j = f.find(L'\\', prev + 1);
			if (i == std::wstring::npos)
			{
				i = f.size();
			}

			if (j == std::wstring::npos)
			{
				j = f.size();
			}

			cur = (int)pt::iMin((int)i, (int)j);

			if (cur == std::wstring::npos)
			{
				cur = f.size();
			}

			path.push_back(f.substr(prev, cur - prev));
			++cur;
		}
	}

	/*project directory*/ 
	static const wchar_t* getProjectDirectory()
	{
		return projectDirectory();
	}

	static void setProjectDirectory(const wchar_t *path)
	{
        BeFileName pathWithoutFileName(path);
        WString dev, dir;
        pathWithoutFileName.ParseName(&dev, &dir, nullptr, nullptr);
        BeStringUtilities::Wcsncpy(projectDirectory(), PT_MAXPATH, pathWithoutFileName.c_str());
	}

	static void makeProjectDirectoryCurrent()
	{
#if defined(NEEDS_WORK_VORTEX_DGNDB)
		SetCurrentDirectoryW(projectDirectory());
#endif
	}

	static int maxFilePath()
	{ 
		return PT_MAXPATH; 
	}

#ifdef NEEDS_WORK_VORTEX_DGNDB
	static ptds::FilePath applicationDataFolder()
	{
		LPITEMIDLIST pidl;
		if (SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR)	
		{
			wchar_t p[PT_MAXPATH];
			SHGetPathFromIDListW(pidl,p);
			return ptds::FilePath(p);
		}
	}
#endif

#ifndef NO_DATA_SOURCE_SERVER

	void read	(PTRMI::DataBuffer &buffer);
	void write	(PTRMI::DataBuffer &buffer) const;

#endif

private:

	static wchar_t *projectDirectory()
	{
		static wchar_t dir[PT_MAXPATH] = L"\0";
		return dir;
	}

    BeFileName      m_path;
	bool            m_bAbsolute;
	const FilePath* m_parent;
};

} // End ptfs namespace