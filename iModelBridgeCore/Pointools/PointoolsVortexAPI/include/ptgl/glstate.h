#ifndef POINTOOLS_GL_STATE_OBJECT
#define POINTOOLS_GL_STATE_OBJECT


#include <gl/glew.h>
#include <gl/glext.h>
#include <ptgl/ptgl.h>

#include <map>

namespace ptgl
{
	/*----------------------------------*/ 
	/* State chnage minimisation		*/ 
	/*----------------------------------*/ 
	enum RenderEnableState
	{
		/*ordered least -> most significant for state ordering*/ 
		RS_Lighting			=	0x000001,
		RS_DepthTest		=	0x000002,
		RS_Texture1D		=	0x000004,
		RS_Texture2D		=	0x000008,
		RS_ColorMaterial	=	0x000010,
		RS_VertexProgram	=	0x000020,
		RS_FragmentProgram	=	0x000040,
		RS_PolygonSmooth	=	0x000080,
		RS_PointSmooth		=	0x000100,
		RS_LineSmooth		=	0x000200,
		RS_Blending			=	0x000400,
		RS_LogicOp			=	0x000800,		
		RS_AlphaTest		=	0x001000
	};
	enum RenderClientState
	{
		RS_ClientVertex		=	0x000001,
		RS_ClientNormal		=	0x000002,
		RS_ClientColor		=	0x000004,
		RS_ClientTexture	=	0x000008
	};
	enum RenderParam
	{
		RS_ShadeSmooth		=	0x000001,
		RS_Texture2DClampS	=	0x000002,
		RS_Texture2DClampT	=	0x000004,
		RS_Texture1DClamp	=	0x000008
	};
	#define NUM_ENABLE_STATES 13
	#define NUM_CLIENT_STATES 4

	struct RenderStateSet
	{
		RenderStateSet() : _enablestate(0), _clientstate(0), _paramstate(0) {}
		void reset() {
			_enablestate = 0;
			_clientstate = 0;
			_paramstate = 0;
		}
		inline static const GLenum &glState(const int &rs)
		{
			static GLenum state[] =
			{
				GL_LIGHTING,
				GL_DEPTH_TEST,
				GL_TEXTURE_1D,
				GL_TEXTURE_2D,
				GL_COLOR_MATERIAL,
				GL_VERTEX_PROGRAM_ARB,
				GL_FRAGMENT_PROGRAM_ARB,
				GL_POLYGON_SMOOTH,
				GL_POINT_SMOOTH,
				GL_LINE_SMOOTH,
				GL_BLEND,
				GL_LOGIC_OP,
				GL_ALPHA_TEST,
			};
			return state[rs];
		}
		inline static const GLenum &clientState(const int &cs)
		{
			static GLenum state[] =
			{
				GL_VERTEX_ARRAY,
				GL_NORMAL_ARRAY,
				GL_COLOR_ARRAY,
				GL_TEXTURE_COORD_ARRAY
			};
			return state[cs];
		}
		PTGL_API static unsigned int overrideStateEnable();
		PTGL_API static unsigned int overrideStateDisable();
		PTGL_API static unsigned int overrideParamSet();
		PTGL_API static unsigned int overrideParamUnset();

		PTGL_API static void overrideEnable(RenderEnableState state);
		PTGL_API static void overrideDisable(RenderEnableState state);
		PTGL_API static void overrideSet(RenderParam state);
		PTGL_API static void overrideUnset(RenderParam state);

		PTGL_API static void removeOverrideEnable(RenderEnableState state);
		PTGL_API static void removeOverrideDisable(RenderEnableState state);
		PTGL_API static void removeOverrideSet(RenderParam state);
		PTGL_API static void removeOverrideUnset(RenderParam state);

		PTGL_API static void resetOverrides();
		PTGL_API static void resetEnableOverrides();
		PTGL_API static void resetDisableOverrides();
		PTGL_API static void resetSetOverrides();
		PTGL_API static void resetUnsetOverrides();
		
		static int stateIndex(unsigned int state)
		{
			int i=0;
			while (state != 1) { state /= 2; ++i; }
			return i;
		}
		
		inline bool isEnabled(const RenderEnableState &rs)	const { return _enablestate & rs ? true : false; }
		inline void enable(const RenderEnableState &rs)  { _enablestate |= rs; }
		inline void disable(const RenderEnableState &rs) { _enablestate &= ~rs; }

		inline bool isEnabled(const RenderClientState &rs)	const { return _clientstate & rs ? true : false;; }
		inline void enable(const RenderClientState &rs)  { _clientstate |= rs; }
		inline void disable(const RenderClientState &rs) { _clientstate &= ~rs; }

		inline bool isSet(const RenderParam &rp) const	{ return (_paramstate & rp ) != 0; }
		inline void set(const RenderParam &rp)		{ _paramstate |= rp; }
		inline void unset(const RenderParam &rp)	{ _paramstate &= ~rp; }

		inline const unsigned int &enableState() const { return _enablestate; };
		inline const unsigned int &parameterState() const { return _paramstate; };
			
		PTGL_API void loadGLState();
		PTGL_API void applyGlobal();

		void apply(RenderStateSet *current)
		{
			unsigned int enablestates=0;
			unsigned int disablestates = 0;

			if (current)
			{
				enablestates = ((_enablestate  | overrideStateEnable()) ^ current->enableState()) & ~current->enableState();
				disablestates = (_enablestate ^ current->enableState()) & (~_enablestate | overrideStateDisable()); 
			}
			else
			{
				enablestates = _enablestate | overrideStateEnable();
				disablestates = (~_enablestate) | overrideStateDisable();
			}
			/* enable states */ 
			int i;
			for (i=0;i<NUM_ENABLE_STATES; i++)
				if (enablestates & (1 << i))
					glEnable(glState(i));
			for (i=0;i<NUM_ENABLE_STATES; i++)
				if (disablestates & (1 << i))
					glDisable(glState(i));

			bool enableparam;

			if (_updateParameter(current, RS_ShadeSmooth, enableparam))
				glShadeModel(enableparam ? GL_SMOOTH : GL_FLAT);

			if (_updateParameter(current, RS_Texture2DClampS, enableparam))
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, enableparam ? GL_CLAMP : GL_REPEAT);

			if (_updateParameter(current, RS_Texture2DClampT, enableparam))
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, enableparam ? GL_CLAMP : GL_REPEAT);

			if (_updateParameter(current, RS_Texture1DClamp, enableparam))
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, enableparam ? GL_CLAMP : GL_REPEAT);
			
			if (current)
				memcpy(globalSet(), this, sizeof(RenderStateSet));
		}
		PTGL_API RenderStateSet* globalSet();
		void generateHashCode()
		{
			union HashCode { struct flags { unsigned int st; unsigned int pr; }; flags f; __int64 hash; };
			HashCode hc;
			hc.f.st = _enablestate;
			hc.f.pr = _paramstate;
			
			_hashcode = hc.hash;
		}
		__int64 hashcode() const { return _hashcode; }
	private:
		unsigned int _enablestate;
		unsigned int _paramstate;
		unsigned int _clientstate;

		unsigned __int64 _hashcode;

		inline bool _updateParameter(const RenderStateSet *current, unsigned int param, bool &newstate)
		{
			unsigned int pset = overrideParamSet();
			unsigned int punset = overrideParamUnset();

			if (param & pset)
			{
				newstate = true;
				return (!current || !(current->parameterState() & param));
			}
			if (param & punset)
			{
				newstate = false;
				return (!current || current->parameterState() & param);
			}
			if (!current)
			{
				newstate = param & _paramstate ? true : false;;
				return true;
			}
			else if ((current->parameterState() & param) != (_paramstate & param))
			{
				newstate = !(current->parameterState() & param);
				return true;
			}
			return false;
		}
	};
	PTGL_API const RenderStateSet* currentRenderStateSet();
	PTGL_API void setContext(int context);

	/*----------------------------------*/ 
	/* Local state control and flushing */ 
	/*----------------------------------*/ 
	template<class StateFunc> struct TState
	{
		enum _State { None = 0, Enabled = 1, Disabled = 2, Updated = 4 };
		typedef std::map<GLenum, GLenum> STATEMAP;

		inline static void set(GLenum e, _State s)	
		{ 
			_State &cs = state(e); 
			if (!(cs & s)) cs = s | Updated;
		}
		inline static GLenum &state(GLenum s)
		{ 
			statemap().insert(STATEMAP::value_type(s, None));
			return statemap().find(s)->second;
		}
		inline static void enable(GLenum s)
		{ 
			GLenum &cs = state(s); 
			if (!(cs & Enabled)) cs = Enabled | Updated; 
		}
		inline static void disable(GLenum s)
		{ 
			GLenum &cs = state(s); 
			if (!(cs & Disabled)) cs = Disabled | Updated; 
		}
		static void flush()
		{
			STATEMAP::iterator i = statemap().begin();
			while (i != statemap().end())
			{
				if (i->second & Updated)
				{
					if (i->second & Enabled) StateFunc::enable(i->first);
					else StateFunc::disable(i->first);
				}
				++i;
			}
		}
		static void clear()
		{
			STATEMAP::iterator i = statemap().begin();
			while (i != statemap().end())
			{
				i->second &= ~Updated;
				++i;
			}
		}
		inline static STATEMAP &statemap() { static STATEMAP m; return m; }
	};
	struct GLStateFunc 
	{ 
		inline static void enable(const GLenum &e) 	{ glEnable(e);  }; 
		inline static void disable(const GLenum &e) { glDisable(e); }; 
	};
	struct GLClientStateFunc 
	{ 
		inline static void enable(const GLenum &e) { glEnableClientState(e); }; 
		inline static void disable(const GLenum &e) { glDisableClientState(e); }; 
	};
	typedef TState<GLStateFunc> State;
	typedef TState<GLClientStateFunc> ClientState;
};
#endif