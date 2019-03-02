#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <prtl_vis/plane.h>
#include <prtl_vis/framebuffer.h>
#include <prtl_vis/opengl_common.h>

#include <clipper.hpp>

//-----------------------------------------------------------------------------
SceneDrawer::SceneDrawer(const scene::Scene& scene, glm::vec3& cam_rotate_around, glm::vec3& cam_spheric_pos, int maxDepth) : depthMax(maxDepth), frame(0) {
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

		// Считываем текстуры
		if (i.textures.size() != 0) {
			int texN = i.textures.size();
			f.textures.resize(texN, 0);
			f.texture_data.resize(texN, nullptr);
			glGenTextures(texN, &f.textures[0]);
			for (int j = 0; j < i.textures.size(); j++) {
				int width, height, n;
				f.texture_data[j] = stbi_load(i.textures[j].filename.c_str(), &width, &height, &n, 3);
				glBindTexture(GL_TEXTURE_2D, f.textures[j]);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, f.texture_data[j]);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		// Считываем текстурированные полигоны
		for (auto& j : i.textured_polygons) {
			auto poly = spob2glm(j.polygon, j.crd);
			auto tex_coords = spob2glm(j.tex_coords);
			if (!isPolygonOrientedClockwise(poly)) {
				poly = std::vector<glm::vec4>(poly.rbegin(), poly.rend());
				tex_coords = std::vector<glm::vec4>(tex_coords.rbegin(), tex_coords.rend());
			}
			f.textured_polygons.push_back({
				Fragmentator::fragmentize(poly, tex_coords),
				f.textures[j.texture_id]
			});
		}
	}
	frame_max = frames.size();
}

//-----------------------------------------------------------------------------
int SceneDrawer::drawAll(int width, int height) {
	const auto& portals = frames[frame].portals;

	w = width; h = height;

	projectedPortalView.push({ { {0, 0, 0, 0}, {0, h, 0, 0}, {w, h, 0, 0}, {w, 0, 0, 0} } });
	drawSceneCount = 0;
	clockWiseInvert = false;
	const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
	f.activate();
	drawScene(1);
	f.disable();
	FrameBufferDrawer::draw(f);
	FrameBufferGetter::unget();
	projectedPortalView.pop();

	return drawSceneCount;
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawPortal(const PortalToDraw& portal, int depth) {
	if (depth > depthMax) return;

	const auto& portals = frames[frame].portals;

	// Проверка на то, находится ли полигон внутри рисуемой полуплоскости
	bool isBehindPlane = depth == 1;
	if (depth != 1) {
		auto clipPlane = ClipPlane::getCurrentPlane() * currentTeleportMatrix.top(); 
		isBehindPlane = isPolygonBehindPlane(clipPlane, portal.polygon);
	}
	if (isBehindPlane) {
		auto projected = projectPolygonToScreen(portal.polygon);

		auto intersected = intersect(projectedPortalView.top(), projected);
		bool isVisibleOnScreen = intersected.size() != 0;
		if (!isVisibleOnScreen) return;

		projectedPortalView.push(intersected);

		// Определяем, как ориентирован портал. Если по часовой стрелке, то можно рисовать, иначе рисуем обратную сторону портала.
		bool isInvert = portal.isInvert;
		if (clockWiseInvert) isInvert = !isInvert;
		bool isDraw = isPolygonOrientedClockwise(projected);
		if (!isInvert) isDraw = !isDraw;
		if (isDraw) {
			if (portal.isTeleportInvert) clockWiseInvert = !clockWiseInvert;

			// Рисуем портал и сцену с ним
			const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
			f.activate();
			ClipPlane::activate(portal.plane);
				glMatrixMode(GL_MODELVIEW); glPushMatrix();
				glMultMatrixf(glm::value_ptr(portal.teleport));
				currentTeleportMatrix.push(portal.teleport);
					drawScene(depth + 1);
				currentTeleportMatrix.pop();
				glMatrixMode(GL_MODELVIEW); glPopMatrix();

			// Нельзя просто отключить плоскость, необходимо вернуть ту матрицу модельно-видового преобразования, которая была при включении этой плоскости.
			// До того, как был написан код для вовзращения матрицы, этот код был местом серьезного бага
			if (!currentTeleportMatrix.empty()) {
				glPopMatrix();
				ClipPlane::disable();
				glPushMatrix();
				glMultMatrixf(glm::value_ptr(currentTeleportMatrix.top()));
			} else
				ClipPlane::disable();
			f.disable();

			PolygonFramebufferDrawer::draw(f, portal.fragments);
			FrameBufferGetter::unget();

			if (portal.isTeleportInvert) clockWiseInvert = !clockWiseInvert;
		} else {
			// Рисуем обратную сторону портала с указанным цветом
			glColor3f(portal.color.x, portal.color.y, portal.color.z);
			drawFragments(portal.fragments);
		}

		projectedPortalView.pop();
	}
}

//-----------------------------------------------------------------------------
void SceneDrawer::drawScene(int depth) {
	if (depth > depthMax) return;

	drawSceneCount++;

	const auto& textured_polygons = frames[frame].textured_polygons;
	const auto& colored_polygons = frames[frame].colored_polygons;
	const auto& portals = frames[frame].portals;

	//-------------------------------------------------------------------------
	// Рисуем все порталы
	const FrameBuffer& f = FrameBufferGetter::get(w, h, true);
	const FrameBuffer& f1 = FrameBufferGetter::get(w, h, true);

	for (int i = 0; i < portals.size(); ++i) {
		bool isDraw = true;
		if (!currentDrawPortal.empty()) {
			if (i % 2 == 1)
				isDraw = i-1 != currentDrawPortal.top();
			else
				isDraw = i+1 != currentDrawPortal.top();
		}
		if (isDraw) {
			currentDrawPortal.push(i);
			f1.activate();
			drawPortal(portals[i], depth);
			f1.disable(false);
			currentDrawPortal.pop();

			f.activate(false);
			FrameBufferMerger::merge(f, f1);
			f.disable(false);
		}
	}

	FrameBufferDrawer::draw(f);

	FrameBufferGetter::unget();
	FrameBufferGetter::unget();

	//-------------------------------------------------------------------------
	// Рисуем все полигоны
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	for (auto& i : colored_polygons) {
		glColor3f(i.color.x, i.color.y, i.color.z);
		drawFragments(i.fragments);
	}
	glEnable(GL_TEXTURE_2D);

	for (auto& i : textured_polygons) {
		glColor3f(1, 1, 1);
        glBindTexture(GL_TEXTURE_2D, i.texture);
		drawFragments(i.fragments);
        glBindTexture(GL_TEXTURE_2D, 0);
	}
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

	std::swap(p1.plane, p2.plane);
	p1.plane.invert();
	p2.plane.invert();

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
std::vector<std::vector<glm::vec4>> intersect(const std::vector<std::vector<glm::vec4>>& a, const std::vector<glm::vec4>& b) {
	std::vector<std::vector<glm::vec4>> result;

	if (a.empty() || b.empty())
		return result;

	using namespace ClipperLib;
	Path clip;
	Paths subj(a.size());
	Paths solution;

	for (int i = 0; i < a.size(); ++i) {
		for (const auto& j : a[i]) {
			subj[i].push_back(IntPoint(j.x, j.y));
		}
	}
	for (const auto& i : b) {
		clip.push_back(IntPoint(i.x, i.y));
	}

	Clipper c;
	c.AddPaths(subj, ptSubject, true);
	c.AddPath(clip, ptClip, true);
	c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);

	for (auto& i : solution) {
		result.push_back({});
		for (auto& j : i)
			result.back().push_back(glm::vec4(j.X, j.Y, 0, 1));
	}

	return result;
}