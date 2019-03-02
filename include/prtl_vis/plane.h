#pragma once

#include <vector>
#include <glm/glm.hpp>

//-----------------------------------------------------------------------------
struct Plane : public glm::vec4 {
	Plane() : glm::vec4() {}
	Plane(const glm::vec4& v) : glm::vec4(v) {}
	void invert(void);
};

//-----------------------------------------------------------------------------
class ClipPlane {
public:
	static void activate(const Plane& p);
	static void disable(void);
	static Plane getCurrentPlane(void);
private:
	static std::vector<Plane> p_stack;
};

//-----------------------------------------------------------------------------
glm::vec4 getClipPlaneEquation(void);
bool isPointBehindPlane(const Plane& plane, const glm::vec4& point);
bool isPolygonBehindPlane(const Plane& plane, const std::vector<glm::vec4>& polygon);