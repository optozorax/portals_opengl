#include <fstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spob/spob.h>
#include <array>

#include <prtl_vis/scene_reader.h>
#include <prtl_vis/opengl_common.h>
#include <prtl_vis/plane.h>
#include <prtl_vis/shader.h>
#include <prtl_vis/framebuffer.h>

SceneDrawer* sceneDrawer;

//-----------------------------------------------------------------------------
const int depthMax = 3;
double pi = _SPOB_PI;
int w = 800, h = 600;

glm::vec3 cam_spheric_pos;
glm::vec3 cam_rotate_around;

//-----------------------------------------------------------------------------
void display() {
    glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	sceneDrawer->drawAll(w, h);

	glutSwapBuffers();
}

//-----------------------------------------------------------------------------
void update_cam(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, double(w)/h, 1.0, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glm::vec3 pos1 = cam_rotate_around + spheric2cartesian(cam_spheric_pos);
	gluLookAt(pos1.x, pos1.y, pos1.z,
			  cam_rotate_around.x, cam_rotate_around.y, cam_rotate_around.z,
			  0, 0, 1);
}

//-----------------------------------------------------------------------------
void reshape(int w1, int h1) {
	w = w1; h = h1;
	FrameBufferGetter::clear();
	glViewport(0, 0, w, h);
	update_cam();
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
int r_moving, r_startx, r_starty;
int l_moving, l_startx, l_starty;
int m_moving, m_startx, m_starty;
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			l_moving = 1;
			l_startx = x;
			l_starty = y;
		}
		if (state == GLUT_UP) {
			l_moving = 0;
		}
	}
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			r_moving = 1;
			r_startx = x;
			r_starty = y;
		}
		if (state == GLUT_UP) {
			r_moving = 0;
		}
	}
	if (button == GLUT_MIDDLE_BUTTON) {
		if (state == GLUT_DOWN) {
			m_moving = 1;
			m_startx = x;
			m_starty = y;
		}
		if (state == GLUT_UP) {
			m_moving = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void motion(int x, int y) {
	if (l_moving) {
		cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 0.5*(x - l_startx));
		cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) + 0.5*(y - l_starty));
		l_startx = x;
		l_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (r_moving) {
		cam_rotate_around.x -= 0.1*(x-r_startx);
		cam_rotate_around.z -= 0.1*(y-r_starty);
		r_startx = x;
		r_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (m_moving) {
		cam_rotate_around.y += 0.1*(y-m_starty);
		m_starty = y;
		update_cam();
		glutPostRedisplay();
	}
}

//-----------------------------------------------------------------------------
void wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_spheric_pos.z += 0.5;
	else cam_spheric_pos.z -= 0.5;
}

//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y) {
	if (key == 'a') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) + 3.0);
	if (key == 'o') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 3.0);

	if (key == 'e') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) + 3.0);
	if (key == 'u') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 3.0);

	if (key == '{') wheel(0, 1, 0, 0);
	if (key == '}') wheel(0, -1, 0, 0);

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void init() {
	const spob::vec3 portalColor0(1, 0.5, 0.15); // orange
	const spob::vec3 portalColor1(0.1, 0.55, 1); // blue

	scene::Scene s;

	s.frames.push_back({});
	auto& polygons = s.frames.back().colored_polygons;
	auto& portals = s.frames.back().portals;
	spob::vec3 i(1, 0, 0), j(0, 1, 0), k(0, 0, 1);

	std::vector<spob::vec2> pPoly = {{0, 0}, {-0.3, 0.7}, {0.3, 0.7}};
	spob::plane3 cPoly(i, j, k, {0.5, 0.5, 0.5});
	polygons.push_back({cPoly, pPoly, {0.5, 0.5, 0.5}});

	s.cam_rotate_around = cPoly.from((pPoly[0] + pPoly[1] + pPoly[2]) / 3.0);
	s.cam_spheric_pos = {spob::deg2rad(30.0), spob::deg2rad(30.0), 3};

	double angle = _SPOB_PI/6;
	double xl = 0.5 / cos(angle);

	std::vector<spob::vec2> pPortal1 = {{xl, 0}, {0, 0}, {0, 1}, {xl, 1}};
	std::vector<spob::vec2> pPortal2 = {{0, 1}, {xl, 1}, {xl, 0}, {0, 0}};

	spob::space3 cPortal1(i, k, j, spob::vec3(0));
	cPortal1 = spob::rotate(cPortal1, spob::vec3(angle, 0, 0));

	spob::space3 cPortal2 = cPortal1;
	cPortal2.pos = cPortal1.pos + cPortal1.i * xl;
	cPortal2 = spob::rotate(cPortal2, spob::vec3(-angle * 2, 0, 0));

	spob::space3 cPortal11 = cPortal1;
	cPortal11.pos.y += 1.5;
	spob::space3 cPortal21 = cPortal2;
	cPortal21.pos.y += 1.5;

	portals.push_back({cPortal1, cPortal11, pPortal1, portalColor0, portalColor1});
	portals.push_back({cPortal2, cPortal21, pPortal2, portalColor0, portalColor1});

	double size = 5.0;
	std::vector<spob::vec2> square = {
		{-1, -1}, 
		{-1, 1}, 
		{1, 1}, 
		{1, -1}, 
	};

	i *= size; j *= size; k *= size;
	polygons.push_back({spob::plane3(i, j, k, k),  square, {0.5, 0, 0}});
	polygons.push_back({spob::plane3(i, j, k, -k), square, {0, 0.5, 0}});
	polygons.push_back({spob::plane3(i, k, j, j),  square, {0, 0, 0.5}});
	polygons.push_back({spob::plane3(i, k, j, -j), square, {0.5, 0.5, 0}});
	polygons.push_back({spob::plane3(k, j, i, i),  square, {0.5, 0, 0.5}});
	polygons.push_back({spob::plane3(k, j, i, -i), square, {0, 0.5, 0.5}});

	sceneDrawer = new SceneDrawer(s, cam_rotate_around, cam_spheric_pos);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(w, h);
	glutCreateWindow("Portal viewer");

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		return -1;
	}

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);

	// Включаем сглаживание
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	init();
	glutMainLoop();
}