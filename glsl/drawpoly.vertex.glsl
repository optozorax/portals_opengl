#version 110

void main()
{
    gl_Position = ftransform();
 
	// fix of the clipping bug for both Nvidia and ATi
	#ifdef __GLSL_CG_DATA_TYPES
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
	#endif
}

// Source: https://forums.khronos.org/showthread.php/68274-How-to-activate-clip-planes-via-shader?p=331885&viewfull=1#post331885 . Thank you, ehsan2004!