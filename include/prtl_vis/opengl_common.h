#pragma once

#include <glm/glm.hpp>
#include <spob/spob.h>
#include <vector>
#include <prtl_vis/plane.h>
#include <prtl_vis/scene_reader.h>

//-----------------------------------------------------------------------------
class SceneDrawer
{
public:
	SceneDrawer(const scene::Scene& scene, glm::vec3& cam_rotate_around, glm::vec3& cam_spheric_pos);

	void drawAll(int width, int height);

	SceneDrawer& operator++(void);
	SceneDrawer& operator--(void);
private:
	struct PortalToDraw
	{
		std::vector<glm::vec4> polygon;
		glm::mat4 teleport;
		Plane plane;
		bool isInvert;
		glm::vec3 color;
	};

	struct ColoredPolygonToDraw {
	    std::vector<glm::vec4> polygon;
		glm::vec3 color;
	};

	struct TexturedPolygonToDraw {
	    std::vector<glm::vec4> polygon;
	    std::vector<glm::vec4> texCoords;
	    GLuint texture;
	};

	struct Frame
	{
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
bool isPolygonOrientedClockwise(const std::vector<glm::vec4>& polygon);