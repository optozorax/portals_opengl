#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <prtl_vis/plane.h>
#include <prtl_vis/framebuffer.h>
#include <prtl_vis/opengl_common.h>

//-----------------------------------------------------------------------------
SceneDrawer::SceneDrawer(const scene::Scene& scene, glm::vec3& cam_rotate_around, glm::vec3& cam_spheric_pos) : depthMax(3), frame(0) {
	cam_rotate_around = spob2glm(scene.cam_rotate_around);
	cam_spheric_pos = spob2glm(scene.cam_spheric_pos);
	for (auto& i : scene.frames) {
		frames.emplace_back();
		Frame& f = frames.back();
		for (auto& j : i.portals) {
			auto result = makeDrawPortal(orientPolygonClockwise(j.polygon), j.crd1, j.crd2, j.color1, j.color2);
			f.portals.push_back(result.first);
			f.portals.push_back(result.second);
		}
		for (auto& j : i.colored_polygons) {
			f.colored_polygons.push_back({
				Fragmentator::fragmentize(spob2glm(orientPolygonClockwise(j.polygon), j.crd)),
				spob2glm(j.color)
			});
		}
		/*for (auto& j : i.textured_polygons) {
			f.textured_polygons.push_back({
				Fragmentator::fragmentize(spob2glm(j.polygon, j.crd), spob2glm(j.tex_coords)), 
				GLuint(j.texture)
			});
		}*/
	}
	frame_max = frames.size();
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawAll(int width, int height) {
	// Костыль для того, чтобы FrameBufferGetter очищался, потому что он черт знает почему криво работает в самый первый раз
	static int displayCount = 0;
	if (displayCount == 1) {
		FrameBufferGetter::clear();
		displayCount = 100;
	}
	if (displayCount == 0) displayCount = 1;

	w = width; h = height;

	clockWiseInvert = false;
	const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
	f.activate();
	drawScene(1);
	f.disable();
	FrameBufferDrawer::draw(f);
	FrameBufferGetter::unget();
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawPortal(const PortalToDraw& portal, int depth) {
	if (depth > depthMax) return;

	// Проверка на то, находится ли полигон внутри рисуемой полуплоскости
	if (depth == 1 || isPolygonBehindPlane(ClipPlane::getCurrentPlane(), portal.polygon)) {
		if (isPolygonOrientedClockwise(projectPolygonToScreen(portal.polygon)) ^ (portal.isInvert ^ !clockWiseInvert)) {
			if (portal.isTeleportInvert) clockWiseInvert = !clockWiseInvert;
			// Рисуем портал и сцену с ним
			const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
			f.activate();
			ClipPlane::activate(portal.plane);
				glMatrixMode(GL_MODELVIEW); glPushMatrix();
				glMultMatrixf(glm::value_ptr(portal.teleport));
					drawScene(depth+1);
				glMatrixMode(GL_MODELVIEW); glPopMatrix();
			ClipPlane::disable();
			f.disable();

			PolygonFramebufferDrawer::draw(f, portal.fragments);
			FrameBufferGetter::unget();
			if (portal.isTeleportInvert) clockWiseInvert = !clockWiseInvert;
		} else {
			// Рисуем обратную сторону портала с указанным цветом
			glColor3f(portal.color.x, portal.color.y, portal.color.z);
			drawFragments(portal.fragments);
			/*glBegin(GL_POLYGON);
			for (const auto& i :portal.polygon)
				glVertex3f(i.x, i.y, i.z);
			glEnd();*/
		}
	}
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawScene(int depth) {
	if (depth > depthMax) return;

	const auto& textured_polygons = frames[frame].textured_polygons;
	const auto& colored_polygons = frames[frame].colored_polygons;
	const auto& portals = frames[frame].portals;

	//-------------------------------------------------------------------------
	// Рисуем все порталы
	const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
	const FrameBuffer& f1 = FrameBufferGetter::get(w, h, true);

	for (int i = 0; i < portals.size(); ++i) {
		f1.activate();
		drawPortal(portals[i], depth);
		f1.disable(false);

		f.activate(false);
		FrameBufferMerger::merge(f, f1);
		f.disable(false);
	}

	FrameBufferDrawer::draw(f);

	FrameBufferGetter::unget();
	FrameBufferGetter::unget();

	//-------------------------------------------------------------------------
	// Рисуем все полигоны
	for (auto& i : colored_polygons) {
		glColor3f(i.color.x, i.color.y, i.color.z);
        /*glBegin(GL_TRIANGLE_FAN);
        for (auto& j : i.polygon)
            glVertex3f(j.x, j.y, j.z);
        glEnd();*/
		//drawConcavePolygon(i.polygon);
		drawFragments(i.fragments);
	}

	/*for (auto& i : textured_polygons) {
        glBindTexture(GL_TEXTURE_2D, i.texture);
        glBegin(GL_POLYGON);
        for (auto& j : i.polygon)
            glVertex3f(j.x, j.y, j.z);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
	}*/
}

//-----------------------------------------------------------------------------
SceneDrawer& SceneDrawer::operator++(void) {
	if (frame+1 == frame_max) frame = 0;
	else frame++;
	return *this;
}

//-----------------------------------------------------------------------------
SceneDrawer& SceneDrawer::operator--(void) {
	if (frame == 0) frame = frame_max-1;
	else frame--;
	return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::pair<SceneDrawer::PortalToDraw, SceneDrawer::PortalToDraw> SceneDrawer::makeDrawPortal(const std::vector<spob::vec2>& polygon, const spob::space3& crd1, const spob::space3& crd2, const spob::vec3& clr1, const spob::vec3& clr2) {
	PortalToDraw p1, p2;

	p1.teleport = getFromMatrix(crd2) * getToMatrix(crd1);
	p2.teleport = getFromMatrix(crd1) * getToMatrix(crd2);

	for (auto& i : polygon) {
		p2.polygon.push_back(spob2glm(spob::plane3(crd1).from(i)));
		p1.polygon.push_back(spob2glm(spob::plane3(crd2).from(i)));
	}

	p1.fragments = Fragmentator::fragmentize(p1.polygon);
	p2.fragments = Fragmentator::fragmentize(p2.polygon);

	p1.plane.x = crd1.k.x;
	p1.plane.y = crd1.k.y;
	p1.plane.z = crd1.k.z;
	p1.plane.w = -dot(crd1.pos, crd1.k);

	p2.plane.x = -crd2.k.x;
	p2.plane.y = -crd2.k.y;
	p2.plane.z = -crd2.k.z;
	p2.plane.w = dot(crd2.pos, crd2.k);

	/*std::swap(p1.plane, p2.plane);
	p1.plane.invert();
	p2.plane.invert();*/

	p1.isInvert = true;
	p2.isInvert = false;

	if (crd1.isRight()) p2.polygon = std::vector<glm::vec4>(p2.polygon.rbegin(), p2.polygon.rend());
	if (crd2.isRight()) p1.polygon = std::vector<glm::vec4>(p1.polygon.rbegin(), p1.polygon.rend());

	p1.isTeleportInvert = crd1.isRight() ^ crd2.isRight();
	p2.isTeleportInvert = crd1.isRight() ^ crd2.isRight();

	p1.color = spob2glm(clr1);
	p2.color = spob2glm(clr2);

	return {p1, p2};
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
glm::mat4 getFromMatrix(const spob::crd3& crd) {
	glm::mat4 result;
	result[0] = glm::vec4(crd.i.x, crd.j.x, crd.k.x, -crd.pos.x);
	result[1] = glm::vec4(crd.i.y, crd.j.y, crd.k.y, -crd.pos.y);
	result[2] = glm::vec4(crd.i.z, crd.j.z, crd.k.z, -crd.pos.z);
	result[3] = glm::vec4(0, 0, 0, -1);
	return glm::transpose(result);
}

//-----------------------------------------------------------------------------
glm::mat4 getToMatrix(const spob::crd3& crd) {
	return glm::inverse(getFromMatrix(crd));
}

//-----------------------------------------------------------------------------
glm::vec4 spob2glm(const spob::vec2& vec) {
	return {vec.x, vec.y, 0, 1};
}

//-----------------------------------------------------------------------------
glm::vec4 spob2glm(const spob::vec3& vec) {
	return {vec.x, vec.y, vec.z, 1};
}

//-----------------------------------------------------------------------------
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec3>& mas) {
	std::vector<glm::vec4> result;
	for (const auto& i : mas)
		result.push_back(spob2glm(i));
	return result;
}

//-----------------------------------------------------------------------------
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec2>& mas) {
	std::vector<glm::vec4> result;
	for (const auto& i : mas)
		result.push_back(spob2glm(i));
	return result;
}

//-----------------------------------------------------------------------------
std::vector<glm::vec4> spob2glm(const std::vector<spob::vec2>& mas, const spob::plane3& plane) {
	std::vector<glm::vec4> result;
	for (const auto& i : mas)
		result.push_back(spob2glm(plane.from(i)));
	return result;
}

//-----------------------------------------------------------------------------
glm::vec3 spheric2cartesian(glm::vec3 spheric) {
	auto& alpha = spheric.x;
	auto& beta = spheric.y;
	auto& r = spheric.z;
	return glm::vec3(
		r * sin(beta) * cos(alpha), 
		r * sin(beta) * sin(alpha), 
		r * cos(beta)
	);
}

//-----------------------------------------------------------------------------
glm::vec3 cartesian2spheric(glm::vec3 cartesian) {
	auto& x = cartesian.x;
	auto& y = cartesian.y;
	auto& z = cartesian.z;
	return glm::vec3(
		std::atan2(y, x),
		std::atan2(std::sqrt(x*x + y*y), z),
		std::sqrt(x*x + y*y + z*z)
	);
}

//-----------------------------------------------------------------------------
std::vector<glm::vec4> projectPolygonToScreen(const std::vector<glm::vec4>& polygon) {
	std::array<GLdouble, 16> projection;
	std::array<GLdouble, 16> modelview;
	std::array<GLdouble, 3>  projected;
	std::array<GLint, 4>     viewport;

	glGetDoublev(GL_PROJECTION_MATRIX, projection.data());
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview.data());
	glGetIntegerv(GL_VIEWPORT, viewport.data());

	std::vector<glm::vec4> result;
	for (auto& i : polygon) {
		gluProject(i.x, i.y, i.z,
				   modelview.data(), projection.data(), viewport.data(),
				   &projected[0], &projected[1], &projected[2]);
		result.push_back(glm::vec4(projected[0], projected[1], projected[2], 1.0));
	}

	return result;
}

//-----------------------------------------------------------------------------
bool isPolygonOrientedClockwise(const std::vector<glm::vec4>& polygon) {
	double sum = 0;
	for (int i = 0; i < polygon.size() - 1; i++)
		sum += (polygon[i+1].x-polygon[i].x)*(polygon[i+1].y+polygon[i].y);
	sum += (polygon[0].x-polygon[polygon.size() - 1].x)*(polygon[0].y+polygon[polygon.size() - 1].y);
	return sum > 0;
}

bool isPolygonOrientedClockwise(const std::vector<spob::vec2>& polygon) {
	double sum = 0;
	for (int i = 0; i < polygon.size() - 1; i++)
		sum += (polygon[i + 1].x - polygon[i].x)*(polygon[i + 1].y + polygon[i].y);
	sum += (polygon[0].x - polygon[polygon.size() - 1].x)*(polygon[0].y + polygon[polygon.size() - 1].y);
	return sum > 0;
}

//-----------------------------------------------------------------------------
std::vector<spob::vec2> orientPolygonClockwise(const std::vector<spob::vec2>& polygon) {
	if (isPolygonOrientedClockwise(polygon))
		return polygon;
	else
		return std::vector<spob::vec2>(polygon.rbegin(), polygon.rend());
}