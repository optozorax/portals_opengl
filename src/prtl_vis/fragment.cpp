#include <GL/glew.h>

#include <prtl_vis/fragment.h>
#include <stdexcept>

std::vector<Fragment> Fragmentator::fragments;
std::vector<TexFragment> Fragmentator::texFragments;

//-----------------------------------------------------------------------------
void __stdcall  Fragmentator::tessBegin1(GLenum which) {
	fragments.push_back({int(which), {}});
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessEnd1() {
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessVertex1(const GLvoid *data) {
	const GLdouble* ptr = (GLdouble*)data;
	fragments.back().vertices.emplace_back(ptr[0], ptr[1], ptr[2]);
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessError1(GLenum errorCode) {
	throw std::logic_error("Error with polygons tesselation. Your polygons is wrong.");
}

//-----------------------------------------------------------------------------
void __stdcall  Fragmentator::tessBegin2(GLenum which) {
	texFragments.push_back({int(which), {}, {}});
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessEnd2() {
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessVertex2(const GLvoid *data) {
	const GLdouble* ptr = (GLdouble*)data;
	texFragments.back().vertices.emplace_back(ptr[0], ptr[1], ptr[2]);
	texFragments.back().tex_coords.emplace_back(ptr[3], ptr[4]);
}

//-----------------------------------------------------------------------------
void __stdcall Fragmentator::tessError2(GLenum errorCode) {
	throw std::exception();
}

//-----------------------------------------------------------------------------
std::vector<Fragment> Fragmentator::fragmentize(const std::vector<glm::vec4>& polygon) {
	// http://www.songho.ca/opengl/gl_tessellation.html
	fragments.clear();
	GLuint id = glGenLists(1);
	if (!id) throw std::exception();

	GLUtesselator *tess = gluNewTess();
	if (!tess) throw std::exception();

	std::vector<glm::dvec4> p;
	for (auto& i : polygon)
		p.push_back(i);

	gluTessCallback(tess, GLU_TESS_BEGIN, (void (__stdcall *)())tessBegin1);
	gluTessCallback(tess, GLU_TESS_END, (void (__stdcall *)())tessEnd1);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void))tessError1);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void (__stdcall *)())tessVertex1);

	glNewList(id, GL_COMPILE);
	glColor3f(1, 1, 1);
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);
	for (auto& i : p) {
		gluTessVertex(tess, &i[0], &i[0]);
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	glEndList();

	gluDeleteTess(tess);

	return fragments;
}

//-----------------------------------------------------------------------------
std::vector<TexFragment> Fragmentator::fragmentize(const std::vector<glm::vec4>& polygon, const std::vector<glm::vec4>& tex_coords) {
	texFragments.clear();
	GLuint id = glGenLists(1);
	if (!id) throw std::exception();

	GLUtesselator *tess = gluNewTess();
	if (!tess) throw std::exception();

	std::vector<std::vector<GLdouble>> temp;
	for (int i = 0; i < polygon.size(); i++) {
		auto p = polygon[i];
		auto t = tex_coords[i];
		temp.push_back({p.x, p.y, p.z, t.x, t.y});
	}	

	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall *)())tessBegin2);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall *)())tessEnd2);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void))tessError2);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall *)())tessVertex2);

	glNewList(id, GL_COMPILE);
	glColor3f(1, 1, 1);
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);
	for (auto& i : temp) {
		gluTessVertex(tess, &i[0], &i[0]);
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	glEndList();

	gluDeleteTess(tess);

	return texFragments;
}

//-----------------------------------------------------------------------------
void drawFragment(const Fragment& f) {
	glBegin(f.begin);
	for (const auto& i : f.vertices)
		glVertex3f(i.x, i.y, i.z);
	glEnd();
}

//-----------------------------------------------------------------------------
void drawFragments(const std::vector<Fragment>& f) {
	for (const auto& i : f)
		drawFragment(i);
}

//-----------------------------------------------------------------------------
void drawFragment(const TexFragment& f) {
	glBegin(f.begin);
	for (int i = 0; i < f.vertices.size(); i++) {
		auto p = f.vertices[i];
		auto t = f.tex_coords[i];
		glTexCoord2f(t.x, t.y); glVertex3f(p.x, p.y, p.z);
	}	
	glEnd();
}

//-----------------------------------------------------------------------------
void drawFragments(const std::vector<TexFragment>& f) {
	for (const auto& i : f)
		drawFragment(i);
}