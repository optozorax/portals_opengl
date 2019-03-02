#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

//-----------------------------------------------------------------------------
struct Fragment
{
	int begin;
	std::vector<glm::vec3> vertices;
};

struct TexFragment
{
	int begin;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> tex_coords;	
};

void drawFragment(const Fragment& f);
void drawFragments(const std::vector<Fragment>& f);

void drawFragment(const TexFragment& f);
void drawFragments(const std::vector<TexFragment>& f);

class Fragmentator
{
public:
	static std::vector<Fragment> fragmentize(const std::vector<glm::vec4>& polygon);

	static std::vector<TexFragment> fragmentize(const std::vector<glm::vec4>& polygon, const std::vector<glm::vec4>& tex_coords);
private:
	static std::vector<Fragment> fragments;
	static std::vector<TexFragment> texFragments;

	static void __stdcall tessBegin1(GLenum which);
	static void __stdcall tessEnd1();
	static void __stdcall tessVertex1(const GLvoid *data);
	static void __stdcall tessError1(GLenum errorCode);

	static void __stdcall tessBegin2(GLenum which);
	static void __stdcall tessEnd2();
	static void __stdcall tessVertex2(const GLvoid *data);
	static void __stdcall tessError2(GLenum errorCode);
};