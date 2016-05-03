/*--------------------------------------------------------------------------*/ 
/*	Plugins Manager template base class interface							*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 18 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_PLUGING_MANAGER_TEMPLATE
#define POINTOOLS_PLUGING_MANAGER_TEMPLATE

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <pt/trace.h>

#include <map>
#include <vector>
#include <string>
#include <pt/typedefs.h>
#include <io.h>
#include <ptappdll/ptapp.h>
#include <utility/ptstr.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef tstring
#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#endif

#define PT_TARGET_SYMBOL "getPTTarget"
#define PT_VERSION_SYMBOL "getVersion"

#ifndef FUNC1ARGSSPEC
#define FUNC1ARGSSPEC void
#define FUNC1ARGS		
#endif

#ifndef FUNC2ARGSSPEC
#define FUNC2ARGSSPEC void
#define FUNC2ARGS		
#endif

#ifndef FUNC3ARGSSPEC
#define FUNC3ARGSSPEC void
#define FUNC3ARGS		
#endif

#ifndef FUNC4ARGSSPEC
#define FUNC4ARGSSPEC void
#define FUNC4ARGS		
#endif

#ifndef FUNC5ARGSSPEC
#define FUNC5ARGSSPEC void
#define FUNC5ARGS		
#endif

#ifndef FUNC6ARGSSPEC
#define FUNC6ARGSSPEC void
#define FUNC6ARGS		
#endif

#ifndef FUNC7ARGSSPEC
#define FUNC7ARGSSPEC void
#define FUNC7ARGS		
#endif

#ifndef FUNC8ARGSSPEC
#define FUNC8ARGSSPEC void
#define FUNC8ARGS		
#endif

#ifndef FUNC9ARGSSPEC
#define FUNC9ARGSSPEC void
#define FUNC9ARGS		
#endif

#ifndef SYM1MARGS   
#define SYM1MARGS " "
#endif
#ifndef SYM2MARGS   
#define SYM2MARGS " "
#endif
#ifndef SYM3MARGS   
#define SYM3MARGS " "
#endif
#ifndef SYM4MARGS   
#define SYM4MARGS " "
#endif
#ifndef SYM5MARGS   
#define SYM5MARGS " "
#endif
#ifndef SYM6MARGS   
#define SYM6MARGS " "
#endif
#ifndef SYM7MARGS   
#define SYM7MARGS " "
#endif
#ifndef SYM8MARGS   
#define SYM8MARGS " "
#endif
#ifndef SYM9MARGS   
#define SYM9MARGS " "
#endif
// note:
// compiler has trouble with this as
// template - so using oldstyle template
// with defines
//

namespace ptapp
{
//to use
// define NUMSYM
// define sysmbol names as 1-NUMSYM
// typedef SYMPTR1 - 10 as pointer type
class PluginsManager
{
public:
	typedef  const char* (*NAMEFUNC)(void);
	typedef  void (*TARGETFUNC)(unsigned char*);
	typedef  void (*VERSIONFUNC)(unsigned char*);

	PluginsManager(const char *identiferSymbol)
	{
		ptstr::copy(m_identiferSymbol, identiferSymbol, 256);
		PTTRACEOUT << "Plugins manager started: id symbol =" << m_identiferSymbol;
	};
	~PluginsManager() 
	{
		unloadPlugins();
	};

	struct Plugin
	{
		FARPROC symbols[NUMSYM];
		char identifier[256];
		char filepath[256];
		byte target[4];
		byte version[4];
		
		Plugin &operator = (const Plugin &p)
		{
			memcpy(this, &p, sizeof(Plugin));
			return *this;
		}
		bool loaded;

		// #define FUNC1IDENTIFIER bool loadData
		// #define FUN1ARGSSPEC	char* filepath
		// #define FUNC1ARGS filepath
#if NUMSYM > 0
		FUNC1IDENTIFIER(FUNC1ARGSSPEC)		{
#ifndef FUNC1VOIDRETURN
			return 
#endif				
			((FUNC1)symbols[0])(FUNC1ARGS);
		}
#endif
#if NUMSYM > 1
		FUNC2IDENTIFIER(FUNC2ARGSSPEC)		{
#ifndef FUNC2VOIDRETURN
			return 
#endif
			((FUNC2)symbols[1])(FUNC2ARGS);
		}
#endif
#if NUMSYM > 2
		FUNC3IDENTIFIER(FUNC3ARGSSPEC)		{
#ifndef FUNC3VOIDRETURN
			return 
#endif	
			((FUNC3)symbols[2])(FUNC3ARGS);
		}
#endif
#if NUMSYM > 3
		FUNC4IDENTIFIER(FUNC4ARGSSPEC)		{
#ifndef FUNC4VOIDRETURN
			return
#endif
			((FUNC4)symbols[3])(FUNC4ARGS);

		}
#endif
#if NUMSYM > 4
		FUNC5IDENTIFIER(FUNC5ARGSSPEC)		{
#ifndef FUNC5VOIDRETURN
			return 
#endif
			((FUNC5)symbols[4])(FUNC5ARGS);
		}
#endif
#if NUMSYM > 5
		FUNC6IDENTIFIER(FUNC6ARGSSPEC)		{
#ifndef FUNC6VOIDRETURN
			return 
#endif
			((FUNC6)symbols[5])(FUNC6ARGS);
		}
#endif
#if NUMSYM > 6
		FUNC7IDENTIFIER(FUNC7ARGSSPEC)		{
#ifndef FUNC7VOIDRETURN
			return 
#endif
			((FUNC7)symbols[6])(FUNC7ARGS);
		}
#endif
#if NUMSYM > 7
		FUNC8IDENTIFIER(FUNC8ARGSSPEC)		{
#ifndef FUNC8VOIDRETURN
			return 
#endif
			((FUNC8)symbols[7])(FUNC8ARGS);
		}
#endif
#if NUMSYM > 8
		FUNC9IDENTIFIER(FUNC9ARGSSPEC)		{
#ifndef FUNC9VOIDRETURN
			return 
#endif
			((FUNC9)symbols[8])(FUNC9ARGS);
		}
#endif

#if defined(_WIN32)
		HMODULE	module;
#endif
#ifdef __APPLE__
		void*	handle;
#endif
	};
	Plugin *operator [] (int index) { return &m_plugins[index]; }
	Plugin *plugin(const char *descriptor)
	{
		for (unsigned int i=0; i<m_plugins.size(); i++)
			if (strcmp(m_plugins[i].identifier, descriptor)==0)
				return &m_plugins[i]; 
		return 0;
	}
	const Plugin *plugin(const char *descriptor) const
	{
		for (unsigned int i=0; i<m_plugins.size(); i++)
			if (strcmp(m_plugins[i].identifier, descriptor)==0)
				return &m_plugins[i]; 
		return 0;
	}
	inline unsigned int size() const { return m_plugins.size(); }

	/*------------------------------------------------------*/ 
	/* unload plugins										*/ 
	/*------------------------------------------------------*/ 
	void unloadPlugins()
	{
		PTTRACE("PluginsManager::unloadPlugins");

		for (uint i=0; i<m_plugins.size(); i++)
		{
#ifdef _WIN32
			::FreeLibrary(m_plugins[i].module);
#endif
		}
		m_plugins.clear();
	}
	/*------------------------------------------------------*/ 
	/* load plugins											*/ 
	/*------------------------------------------------------*/ 
	int loadPlugins(const TCHAR*dir, const TCHAR *ext)
	{
		PTTRACE("PluginsManager::loadPlugins");
		
		int count = 0;
		std::vector<std::tstring> paths;
		scanForPlugins(dir, ext, paths);
		
		for (uint i=0; i<paths.size(); i++)
			if (loadPlugin(paths[i].c_str())) count++;

		return count;
	}
	/*------------------------------------------------------*/ 
	/* scan for plugins in dir								*/ 
	/*------------------------------------------------------*/ 
	bool scanForPlugins(const TCHAR*dir, const TCHAR*ext, std::vector<std::tstring> &paths)
	{
		uint size = paths.size();
		TCHAR fpath[260];
		_tprintf(fpath, _T("%s\\modules\\%s\\*.%s"), ptapp::apppath(), dir, ext);

#if defined(_WIN32)
		WIN32_FIND_DATA ff;
		HANDLE hFind = ::FindFirstFile(fpath, &ff);
		
		if (hFind != INVALID_HANDLE_VALUE)
		{			
			std::tstring fn(_T("modules\\"));
			fn += dir;
			fn += _T("\\");

			paths.push_back(fn + ff.cFileName);	

			while (::FindNextFile(hFind, &ff))
				paths.push_back(fn + ff.cFileName);
		}
		FindClose(hFind);
#endif
		return paths.size() > size ? true : false;
	}
	/*------------------------------------------------------*/ 
	/* scan for plugins in dir								*/ 
	/*------------------------------------------------------*/ 
	bool loadPlugin(const TCHAR *file)
	{
		PTTRACE("PluginsManager::loadPlugin");
		PTTRACEOUT << file;

#if defined(_WIN32)
		HMODULE mod = ::LoadLibrary(file);

		if (!mod)
		{
          PTTRACEOUT << "Plugin Error: load failure";
          return false;
        }

		FARPROC proc = findProcAddress(mod, PT_TARGET_SYMBOL, "Ph");
		TARGETFUNC tfunc = 0;
		
		if (!proc)
        {
            PTTRACEOUT << "Plugin Error: Target symbol not found";
			tfunc = NULL;
			/*this does not cause a fail*/ 
		}
		else tfunc = (TARGETFUNC)proc;

		proc = findProcAddress(mod, PT_VERSION_SYMBOL, "Ph");
		VERSIONFUNC vfunc = 0;
		
		if (!proc)
        {
            PTTRACEOUT << "Plugin Error: Version symbol not found";
			vfunc = NULL;
			/*this does not cause a fail*/ 
		}
		else vfunc = (VERSIONFUNC)proc;

		proc = findProcAddress(mod, m_identiferSymbol, "v");
		if (!proc) 
        {
            /*dont report error*/ 
			PTTRACEOUT << "Plugin Error: Identifier :" << m_identiferSymbol << " symbol not found";
			goto fail;
        }
        {
		NAMEFUNC nfunc = (NAMEFUNC)proc;

		Plugin P;	
		P.module = mod;
		ptstr::copy(P.filepath, file, sizeof(P.filepath));
		ptstr::copy(P.identifier, nfunc(), sizeof(P.identifier));
		memset(P.version, 0, 4);
		memset(P.target, 0, 4);
		if (tfunc) tfunc(P.target);
		if (vfunc) vfunc(P.version);
		P.loaded = true;
		
		if (!extractSymbols(P))
		{
            PTTRACEOUT << "Plugin Error: symbol table extraction failure";		
		    goto fail;
		}
		m_plugins.push_back(P);
		}
		return true;
fail:
        ::FreeLibrary(mod);
        return false;
#endif
#ifdef __APPLE__
		if (getuid() != geteuid())  {
			fprintf(stderr, "%s: plugins disabled in setuid programs\n", name);
		return 0;
		}
		void* handle = dlopen(name, RTLD_NOW);
		if (handle) 
		{
			void* fn = dlsym(handle, identiferSymbol);
			if (!fn) return false;
			NAMEFUNC nfunc = (NAMEFUNC)fn;
			
			fn = dlsym(handle, PT_TARGET_SYMBOL);
			if (!fn) return false;
			TARGETFUNC tfunc = (TARGETFUNC)fn;

			Plugin P;	
			P.handle = handle;
			strcpy(P.filepath, file);
			strcpy(P.identifier, nfunc());
			tfunc(P.target);
			P.loaded = true;

			m_plugins.push_back(P);
			if (!extractSymbols(m_plugins.back()))
			{
			   PTTRACEOUT << "Plugin Error: Symbol extraction Failure in " << file;
				m_plugins.erase(m_plugins.back());
				return false;
			}
		}
#endif
		return true;		
	}
	/*------------------------------------------------------*/ 
	/* retrieve error in case of failure					*/ 
	/*------------------------------------------------------*/ 
	void getLastError(char *pbuff)
	{
#if defined(_WIN32)
		char *buff;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&buff,
			0,
			NULL);
		strcpy(pbuff, buff);
		LocalFree(buff);
#endif
	}
	/*------------------------------------------------------*/ 
	/* check Plugin still exists							*/ 
	/*------------------------------------------------------*/ 
	bool checkPluginExists(int index)
	{
#if defined(_WIN32)
		return (_access(m_plugins[index].filepath, 0) == 0);
#else
		return (access(m_plugins[index].filepath, 0) == 0);
#endif
	}
	/*------------------------------------------------------*/ 
	/* unload Plugin										*/ 
	/*------------------------------------------------------*/ 
	bool unloadPlugin(int index)
	{
#if defined(_WIN32)
		if (::FreeLibrary(m_plugins[index].module))
		{
			m_plugins.erase(m_plugins.begin() + index);
			return true;
		}
		return false;
#endif
	}
protected:

    FARPROC findProcAddress(HMODULE mod, const char*id, const char *args)
    {
        /*try unmangled*/ 
        FARPROC p = ::GetProcAddress(mod, id);
        if (p) return p;
        char proc[256];
        for (int i=0; i<100; i++)
        {
                sprintf(proc, "_Z%d%s%s", i, id, args); 
                p = ::GetProcAddress(mod, proc);
                if (p) return p;
        }
        return 0;
    }
		
	/*------------------------------------------------------*/ 
	/* extract Symbol addresses for loaded plugins			*/ 
	/*------------------------------------------------------*/ 
	bool extractSymbols(Plugin &p)
	{
#if defined(_WIN32)

#if NUMSYM > 0
			p.symbols[0] = findProcAddress(p.module, SYM1, SYM1MARGS);
			if (!p.symbols[0]) return false;
#endif
		
#if NUMSYM > 1
			p.symbols[1] = findProcAddress(p.module, SYM2, SYM2MARGS);		
			if (!p.symbols[1]) return false;
#endif

#if NUMSYM > 2
			p.symbols[2] = findProcAddress(p.module, SYM3, SYM3MARGS);			
			if (!p.symbols[2]) return false;
#endif
#if NUMSYM > 3
			p.symbols[3] = findProcAddress(p.module, SYM4, SYM4MARGS);			
			if (!p.symbols[3]) return false;
#endif
#if NUMSYM > 4
			p.symbols[4] = findProcAddress(p.module, SYM5, SYM5MARGS);
			if (!p.symbols[4]) return false;
#endif

#if NUMSYM > 5
			p.symbols[5] = findProcAddress(p.module, SYM6, SYM6MARGS);
			if (!p.symbols[5]) return false;
#endif

#if NUMSYM > 6
			p.symbols[6] = findProcAddress(p.module, SYM7, SYM7MARGS);
			if (!p.symbols[6]) return false;
#endif

#if NUMSYM > 7
			p.symbols[7] = findProcAddress(p.module, SYM8, SYM8MARGS);
			if (!p.symbols[7]) return false;
#endif

#if NUMSYM > 8
			p.symbols[8] = findProcAddress(p.module, SYM9, SYM9MARGS);
			if (!p.symbols[8]) return false;
#endif
#endif
#ifdef __APPLE__
#if NUMSYM > 0
			p.symbols[0] = dlsym(p.handle, sym1);
			if (!p.symbols[0]) return false;
#endif
		
#if sym2
			p.symbols[1] = dlsym(p.handle, sym2);
			if (!p.symbols[1]) return false;
#endif
#if sym3
			p.symbols[2] = dlsym(p.handle, sym3);
			if (!p.symbols[2]) return false;
#endif
#if sym4
			p.symbols[3] = dlsym(p.handle, sym4);
			if (!p.symbols[3]) return false;
#endif
#if sym5
			p.symbols[4] = dlsym(p.handle, sym5);
			if (!p.symbols[4]) return false;
#endif

#if sym6
			p.symbols[5] = dlsym(p.handle, sym6);
			if (!p.symbols[5]) return false;
#endif

#if sym7
			p.symbols[6] = dlsym(p.handle, sym7);
			if (!p.symbols[6]) return false;
#endif

#if sym8
			p.symbols[7] = dlsym(p.handle, sym8);
			if (!p.symbols[7]) return false;
#endif

#if sym9
			p.symbols[8] = dlsym(p.handle, sym9);
			if (!p.symbols[8]) return false;
#endif
#endif
			return true;
	}
	char m_identiferSymbol[256];
	std::vector<Plugin>	m_plugins;
};
}
//undefine 
#undef SYM1
#undef SYM2
#undef SYM3
#undef SYM4
#undef SYM5
#undef SYM6
#undef SYM7
#undef SYM8
#undef SYM9

#if NUMSYM > 0
#undef FUNC1IDENTIFIER
#undef FUNC1ARGSSPEC
#undef FUNC1
#undef FUNC1ARGS
#endif

#if NUMSYM > 1
#undef FUNC2IDENTIFIER
#undef FUNC2ARGSSPEC
#undef FUNC2
#undef FUNC2ARGS
#endif

#if NUMSYM > 2
#undef FUNC3IDENTIFIER
#undef FUNC3ARGSSPEC
#undef FUNC3
#undef FUNC3ARGS
#endif

#if NUMSYM > 3
#undef FUNC4IDENTIFIER
#undef FUNC4ARGSSPEC
#undef FUNC4
#undef FUNC4ARGS
#endif

#if NUMSYM > 4
#undef FUNC5IDENTIFIER
#undef FUNC5ARGSSPEC
#undef FUNC5
#undef FUNC5ARGS
#endif

#if NUMSYM > 5
#undef FUNC6IDENTIFIER
#undef FUNC6ARGSSPEC
#undef FUNC6
#undef FUNC6ARGS
#endif

#if NUMSYM > 6
#undef FUNC7IDENTIFIER
#undef FUNC7ARGSSPEC
#undef FUNC7
#undef FUNC7ARGS
#endif

#if NUMSYM > 7
#undef FUNC8IDENTIFIER
#undef FUNC8ARGSSPEC
#undef FUNC8
#undef FUNC8ARGS
#endif 

#if NUMSYM > 8
#undef FUNC9IDENTIFIER
#undef FUNC9ARGSSPEC
#undef FUNC9
#undef FUNC9ARGS
#endif

#if NUMSYM > 9
#undef FUNC1IDENTIFIER
#undef FUNC1ARGSSPEC
#undef FUNC1
#undef FUNC1ARGS
#endif

#undef NUMSYM
#endif

#ifdef SYM1MARGS   
#undef SYM1MARGS
#endif
#ifdef SYM2MARGS   
#undef SYM2MARGS
#endif
#ifdef SYM3MARGS   
#undef SYM3MARGS
#endif
#ifdef SYM4MARGS   
#undef SYM4MARGS
#endif
#ifdef SYM5MARGS   
#undef SYM5MARGS
#endif
#ifdef SYM6MARGS   
#undef SYM6MARGS
#endif
#ifdef SYM7MARGS   
#undef SYM7MARGS
#endif
#ifdef SYM8MARGS   
#undef SYM8MARGS
#endif
#ifdef SYM9MARGS   
#undef SYM9MARGS
#endif

#endif