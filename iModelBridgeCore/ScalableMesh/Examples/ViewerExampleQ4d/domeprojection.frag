#version 130
uniform sampler2D tex;
uniform	int			objectID;
void main()
{
	gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
}
