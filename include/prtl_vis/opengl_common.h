#pragma once

#include <vector>
#include <stack>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <spob/spob.h>

#include <prtl_vis/plane.h>
#include <prtl_vis/fragment.h>
#include <prtl_vis/scene_reader.h>

//-----------------------------------------------------------------------------
class SceneDrawer
{
public:
	SceneDrawer(const scene::Scene& scene, glm::vec3& cam_rotate_around, glm::vec3& cam_spheric_pos, int maxDepth);

	void setCam(glm::vec3& cam_rotate_around, glm::vec3& cam_spheric_pos);

	int drawAll(int width, int height);

	void setMaxDepth(int maxDepth) { depthMax = maxDepth; }
	int getMaxDepth(void) const { return depthMax; }

	int getCurrentFrame(void) const { return frame+1; }
	int getMaxFrame(void) const { return frame_max; }

	SceneDrawer& operator++(void);
	SceneDrawer& operator--(void);
private:
	struct PortalToDraw
	{
		std::vector<glm::vec4> polygon;
		std::vector<Fragment> fragments;
		glm::mat4 teleport;
		Plane plane;
		bool isInvert;
		bool isTeleportInvert;
		glm::vec3 color;
	};

	struct ColoredPolygonToDraw {
	    std::vector<Fragment> fragments;
		glm::vec3 color;
	};

	struct TexturedPolygonToDraw {
	    std::vector<TexFragment> fragments;
	    GLuint texture;
	};

	struct Frame
	{
		std::vector<GLuint> textures;
		std::vector<unsigned char*> texture_data;
		std::vector<ColoredPolygonToDraw> colored_polygons;
		std::vector<TexturedPolygonToDraw> textured_polygons;
		std::vector<PortalToDraw> portals;
	};

	static std::pair<PortalToDraw, PortalToDraw> makeDrawPortal(
		const std::vector<spob::vec2>& polygon, 
		const spob::space3& crd1, const spob::space3& crd2, 
		const spob::vec3& clr1, const spob::vec3& clr2
	);
	void drawScene(int depth);
	void drawPortal(const PortalToDraw& portal, int depth);

	std::vector<Frame> frames;
	int depthMax;
	int w, h;
	int frame;
	int frame_max;
	int drawSceneCount;
	std::stack<int> currentDrawPortal;
	bool clockWiseInvert;
	std::stack<glm::mat4> currentTeleportMatrix;
	std::stack<std::vector<std::vector<glm::vec4>>> projectedPortalView;

	glm::vec3 cam_1, cam_2;
};

//-----------------------------------------------------------------------------
glm::mat4 getFromMatrix(const spob::crd3& crd);
glm::mat4 getToMatrix(const spob::crd3& crd);

glm::vec4 spob2glm(const spob::vec2& vec);
glm::vec4 spob2glm(const spob::vec3& vec);
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec3>& mas);
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec2>& mas);
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec2>& mas, const spob::plane3& plane);

glm::vec3 spheric2cartesian(glm::vec3 cartesian);
glm::vec3 cartesian2spheric(glm::vec3 spheric);

std::vector<glm::vec4> projectPolygonToScreen(const std::vector<glm::vec4>& polygon);
std::vector<std::vector<glm::vec4>> intersect(const std::vector<std::vector<glm::vec4>>& a, const std::vector<glm::vec4>& b);

template<class T>
bool isPolygonOrientedClockwise(const std::vector<T>& polygon);
template<class T>
std::vector<T> orientPolygonClockwise(const std::vector<T>& polygon);

//-----------------------------------------------------------------------------
template<class T>
bool isPolygonOrientedClockwise(const std::vector<T>& polygon) {
	double sum = 0;
	for (int i = 0; i < polygon.size() - 1; i++)
		sum += (polygon[i + 1].x - polygon[i].x)*(polygon[i + 1].y + polygon[i].y);
	sum += (polygon[0].x - polygon[polygon.size() - 1].x)*(polygon[0].y + polygon[polygon.size() - 1].y);
	return sum > 0;
}

//-----------------------------------------------------------------------------
template<class T>
std::vector<T> orientPolygonClockwise(const std::vector<T>& polygon) {
	if (isPolygonOrientedClockwise(polygon))
		return polygon;
	else
		return std::vector<T>(polygon.rbegin(), polygon.rend());
}