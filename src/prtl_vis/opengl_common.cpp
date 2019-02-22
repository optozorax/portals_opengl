#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <prtl_vis/plane.h>
#include <prtl_vis/framebuffer.h>
#include <prtl_vis/opengl_common.h>

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
		if (isPolygonOrientedClockwise(projectPolygonToScreen(portal.polygon)) ^ !portal.isInvert) {
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

			PolygonFramebufferDrawer::draw(f, portal.polygon);
			FrameBufferGetter::unget();
		} else {
			// Рисуем обратную сторону портала с указанным цветом
			glColor3f(portal.color.x, portal.color.y, portal.color.z);
			glBegin(GL_POLYGON);
			for (const auto& i :portal.polygon)
				glVertex3f(i.x, i.y, i.z);
			glEnd();
		}
	}
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawScene(int depth) {
	if (depth > depthMax) return;

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
	for (auto& i : polygons) {
        if (i.isTextured) {
            glBindTexture(GL_TEXTURE_2D, i.texture);
            glBegin(GL_POLYGON);
            for (auto& j : i.polygon)
                glVertex3f(j.x, j.y, j.z);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
        } else {
            glColor3f(i.color.x, i.color.y, i.color.z);
            glBegin(GL_POLYGON);
            for (auto& j : i.polygon)
                glVertex3f(j.x, j.y, j.z);
            glEnd();
        }
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::pair<PortalToDraw, PortalToDraw> makeDrawPortal(const std::vector<glm::vec4>& polygon, const spob::space3& crd1, const spob::space3& crd2, const glm::vec3& clr1, const glm::vec3& clr2) {
	PortalToDraw p1, p2;

	p1.teleport = getFromMatrix(crd2) * getToMatrix(crd1);
	p2.teleport = getFromMatrix(crd1) * getToMatrix(crd2);

	for (auto& i : polygon) {
		p2.polygon.push_back(getVec(spob::plane3(crd1).from(spob::vec2(i.x, i.y))));
		p1.polygon.push_back(getVec(spob::plane3(crd2).from(spob::vec2(i.x, i.y))));
	}

	p1.plane.x = crd1.k.x;
	p1.plane.y = crd1.k.y;
	p1.plane.z = crd1.k.z;
	p1.plane.w = -dot(crd1.pos, crd1.k);

	p2.plane.x = -crd2.k.x;
	p2.plane.y = -crd2.k.y;
	p2.plane.z = -crd2.k.z;
	p2.plane.w = dot(crd2.pos, crd2.k);

	p1.isInvert = true;
	p2.isInvert = false;

	p1.color = clr1;
	p2.color = clr2;

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
glm::vec4 getVec(const spob::vec2& vec) {
	return glm::vec4(vec.x, vec.y, 0, 1);
}

//-----------------------------------------------------------------------------
glm::vec4 getVec(const spob::vec3& vec) {
	return glm::vec4(vec.x, vec.y, vec.z, 1);
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