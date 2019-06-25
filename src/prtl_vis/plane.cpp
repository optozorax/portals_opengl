#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <prtl_vis/plane.h>

std::vector<ClipPlane::PlaneWithMatrix> ClipPlane::p_stack;

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

	// Запоминаем текущую матрицу модельно-видового преобразования
	glm::dmat4 modelview;
	glGetDoublev(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));

	p_stack.push_back({p, modelview});
	GLdouble plane[4] = {p.x, p.y, p.z, p.w};
	glClipPlane(GL_CLIP_PLANE0, plane);
	glEnable(GL_CLIP_PLANE0);
}

//-----------------------------------------------------------------------------
void ClipPlane::disable(void) {
	glDisable(GL_CLIP_PLANE0);
	p_stack.pop_back();

	if (!p_stack.empty()) {
		glm::dmat4 current_modelview;
		glGetDoublev(GL_MODELVIEW_MATRIX, glm::value_ptr(current_modelview));

		Plane p = p_stack.back().p;
		GLdouble plane[4] = {p.x, p.y, p.z, p.w};
		// Тут есть небольшая хитрость: надо вернуть ту матрицу модельно-видового преобразования, которая была в тот момент, когда мы эту плоскость включали, иначе это будет работать неправильно
		glLoadMatrixd(glm::value_ptr(p_stack.back().modelview));
    	glClipPlane(GL_CLIP_PLANE0, plane);
    	glEnable(GL_CLIP_PLANE0);

    	// Возвращаем текущую матрицу
    	glLoadMatrixd(glm::value_ptr(current_modelview));
	}
}

//-----------------------------------------------------------------------------
Plane ClipPlane::getCurrentPlane(void) {
	if (p_stack.empty())
		throw std::exception();
	return p_stack.back().p;
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