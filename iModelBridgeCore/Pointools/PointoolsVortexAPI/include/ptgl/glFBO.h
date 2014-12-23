// Frame Buffer Object class
//
// Jun 2009
//
namespace ptgl
{
	class FBO
	{	
	public:
		FBO( pt::String name, GLenum colorFormat, GLenum depthFormat, 
			int redbits, int depthBits, int depthSamples, int coverageSamples)
		{
			m_name = name;
			m_colorFormat = colorFormat;
			m_depthFormat = depthFormat;
			m_redbits = redbits;
			m_depthSamples = depthSamples;
			m_coverageSamples = coverageSamples;


		}
	private:
		pt::String m_name;
		GLenum m_colorFormat;
		GLenum m_depthFormat;
		int m_redbits;
		int m_depthBits;
		int m_depthSamples;
		int m_coverageSamples;
	};
}