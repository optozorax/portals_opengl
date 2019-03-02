#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <prtl_vis/plane.h>

std::vector<Plane> ClipPlane::p_stack;

//-----------------------------------------------------------------------------
void Plane::invert(void) {
    x = -x;
	y = -y;
	z = -z;
	w = -w;
}

//-----------------------------------------------------------------------------
void ClipPlane::activate(const Plane& p) {
	if (!p_stack.empty())
		glDisable(GL_CLIP_PLANE0);

	p_stack.push_back(p);
	GLdouble plane[4] = {p.x, p.y, p.z, p.w};
	glClipPlane(GL_CLIP_PLANE0, plane);
	glEnable(GL_CLIP_PLANE0);
}

//-----------------------------------------------------------------------------
void ClipPlane::disable(void) {
	glDisable(GL_CLIP_PLANE0);
	p_stack.pop_back();

	if (!p_stack.empty()) {
		Plane p = p_stack.back();
		GLdouble plane[4] = {p.x, p.y, p.z, p.w};
    	glClipPlane(GL_CLIP_PLANE0, plane);
    	glEnable(GL_CLIP_PLANE0);
	}
}

//-----------------------------------------------------------------------------
Plane ClipPlane::getCurrentPlane(void) {
	if (p_stack.empty())
		throw std::exception();
	return p_stack.back();
}

//-----------------------------------------------------------------------------
bool isPointBehindPlane(const Plane& plane, const glm::vec4& point) {
	double planeValue = 
		plane.x*point.x + 
		plane.y*point.y + 
		plane.z*point.z + 
		plane.w;
	return planeValue > 0.001;
}

//-----------------------------------------------------------------------------
bool isPolygonBehindPlane(const Plane& plane, const std::vector<glm::vec4>& polygon) {
	bool isBehindPlane = true;
	for (auto& i : polygon)
		isBehindPlane &= !isPointBehindPlane(plane, i);
	return !isBehindPlane;
}

//-----------------------------------------------------------------------------
glm::vec4 getClipPlaneEquation(void) {
	auto plane = ClipPlane::getCurrentPlane();
	glm::dmat4 modelview;
	glGetDoublev(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));

	auto result = plane * glm::inverse(modelview);

	return result;
}