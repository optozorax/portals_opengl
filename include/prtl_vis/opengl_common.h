#pragma once

#include <glm/glm.hpp>
#include <spob/spob.h>
#include <vector>
#include <prtl_vis/plane.h>

//-----------------------------------------------------------------------------
struct PortalToDraw
{
	std::vector<glm::vec4> polygon;
	glm::mat4 teleport;
	Plane plane;
	bool isInvert;
	glm::vec3 color;
};

std::pair<PortalToDraw, PortalToDraw> makeDrawPortal(
	const std::vector<glm::vec4>& polygon, 
	const spob::space3& crd1, const spob::space3& crd2, 
	const glm::vec3& clr1, const glm::vec3& clr2
);

//-----------------------------------------------------------------------------
struct PolygonToDraw {
    std::vector<glm::vec4> polygon;
    std::vector<glm::vec2> texCoords;
    bool isTextured;
    GLuint texture;
	glm::vec3 color;
};

//-----------------------------------------------------------------------------
class SceneDrawer
{
public:
	SceneDrawer() : depthMax(3) {}

	void drawAll(int width, int height);
	void drawScene(int depth);
	void drawPortal(const PortalToDraw& portal, int depth);

	std::vector<PolygonToDraw> polygons;
	std::vector<PortalToDraw> portals;
	int depthMax;
private:
	int w, h;
};

//-----------------------------------------------------------------------------
glm::mat4 getFromMatrix(const spob::crd3& crd);
glm::mat4 getToMatrix(const spob::crd3& crd);
glm::vec4 getVec(const spob::vec2& vec);
glm::vec4 getVec(const spob::vec3& vec);

std::vector<glm::vec4> projectPolygonToScreen(const std::vector<glm::vec4>& polygon);
bool isPolygonOrientedClockwise(const std::vector<glm::vec4>& polygon);