#include <fstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spob/spob.h>
#include <array>

#include <prtl_vis/opengl_common.h>
#include <prtl_vis/plane.h>
#include <prtl_vis/shader.h>
#include <prtl_vis/framebuffer.h>

SceneDrawer sceneDrawer;

//-----------------------------------------------------------------------------
double cam_alpha = glm::radians(30.0);
double cam_beta = glm::radians(30.0);
double cam_distance = 3;
const int depthMax = 3;
double pi = _SPOB_PI;
int w = 800, h = 600;

glm::vec3 cam_pos;

//-----------------------------------------------------------------------------
void display() {
    glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	sceneDrawer.drawAll(w, h);

	glutSwapBuffers();
}

//-----------------------------------------------------------------------------
void update_cam(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, double(w)/h, 1.0, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glm::vec3 pos1(sin(pi/2 - cam_beta) * cos(cam_alpha),
			  sin(pi/2 - cam_beta) * sin(cam_alpha),
			  cos(pi/2 - cam_beta));
	pos1 *= cam_distance;
	pos1 += cam_pos;
	gluLookAt(pos1.x, pos1.y, pos1.z,
			  cam_pos.x, cam_pos.y, cam_pos.z,
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
		cam_alpha = glm::radians(glm::degrees(cam_alpha) - 0.5*(x - l_startx));
		cam_beta = glm::radians(glm::degrees(cam_beta) + 0.5*(y - l_starty));
		l_startx = x;
		l_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (r_moving) {
		cam_pos.x -= 0.1*(x-r_startx);
		cam_pos.z -= 0.1*(y-r_starty);
		r_startx = x;
		r_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (m_moving) {
		cam_pos.y += 0.1*(y-m_starty);
		m_starty = y;
		update_cam();
		glutPostRedisplay();
	}
}

//-----------------------------------------------------------------------------
void wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_distance += 0.5;
	else cam_distance -= 0.5;
}

//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y) {
	if (key == 'a') cam_alpha = glm::radians(glm::degrees(cam_alpha) + 3.0);
	if (key == 'o') cam_alpha = glm::radians(glm::degrees(cam_alpha) - 3.0);

	if (key == 'e') cam_beta = glm::radians(glm::degrees(cam_beta) + 3.0);
	if (key == 'u') cam_beta = glm::radians(glm::degrees(cam_beta) - 3.0);

	if (key == '{') wheel(0, 1, 0, 0);
	if (key == '}') wheel(0, -1, 0, 0);

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.0, 1.0, 1.0);

	auto& polygons = sceneDrawer.polygons;
	auto& portals = sceneDrawer.portals;

	glm::vec3 portalColor0(1, 0.5, 0.15); // orange
	glm::vec3 portalColor1(0.1, 0.55, 1); // blue

	/*// init portals
	std::vector<vec2> pPortal;
	pPortal.push_back({-1, -1});
	pPortal.push_back({-1, 1});
	pPortal.push_back({1, 1});
	pPortal.push_back({1, -1});

	crd3 cPortal1;
	cPortal1.i = vec3(1, 0, 0);
	cPortal1.k = vec3(0, 1, 0);
	cPortal1.j = vec3(0, 0, 1);

	cPortal1.pos = -cPortal1.k * 2.5;

	crd3 cPortal2 = cPortal1;
	cPortal2.pos = cPortal1.k * 2.5;

	addPortal(pPortal, cPortal1, cPortal2, portalColor0, portalColor1);*/

	//-------------------------------------------------------------------------
	// Добавляем портал и его контур
	std::vector<spob::vec2> pPoly;
	pPoly.push_back({0, 0});
	pPoly.push_back({-0.3, 0.7});
	pPoly.push_back({0.3, 0.7});

	spob::plane3 cPoly;
	cPoly.pos = spob::vec3(0.5, 0.5, 0.5);
	cPoly.i = spob::vec3(1, 0, 0);
	cPoly.j = spob::vec3(0, 1, 0);
	cPoly.k = spob::vec3(0, 0, 1);

	polygons.push_back(PolygonToDraw{{}, {}, false, 0, glm::vec3(0.5, 0.5, 0.5)});
	polygons.back().polygon.push_back(getVec(cPoly.from(pPoly[0])));
	polygons.back().polygon.push_back(getVec(cPoly.from(pPoly[1])));
	polygons.back().polygon.push_back(getVec(cPoly.from(pPoly[2])));

	cam_pos = getVec(cPoly.from((pPoly[0] + pPoly[1] + pPoly[2])/3.0));

	double angle = _SPOB_PI/6;
	double xl = 0.5 / cos(angle);

	std::vector<glm::vec4> pPortal1;
	pPortal1.push_back({xl, 0, 0, 1});
	pPortal1.push_back({0, 0, 0, 1});
	pPortal1.push_back({0, 1, 0, 1});
	pPortal1.push_back({xl, 1, 0, 1});

	std::vector<glm::vec4> pPortal2;
	pPortal2.push_back({0, 1, 0, 1});
	pPortal2.push_back({xl, 1, 0, 1});
	pPortal2.push_back({xl, 0, 0, 1});
	pPortal2.push_back({0, 0, 0, 1});

	spob::crd3 cPortal1;
	cPortal1.pos = spob::vec3(0, 0, 0);
	cPortal1.i = cPoly.i;
	cPortal1.j = cPoly.k;
	cPortal1.k = cPoly.j;
	cPortal1 = spob::rotate(cPortal1, spob::vec3(angle, 0, 0));

	spob::crd3 cPortal2 = cPortal1;
	cPortal2.pos = cPortal1.pos + cPortal1.i * xl;
	cPortal2 = spob::rotate(cPortal2, spob::vec3(-angle * 2, 0, 0));

	spob::crd3 cPortal11 = cPortal1;
	cPortal11.pos.y += 1.5;
	spob::crd3 cPortal21 = cPortal2;
	cPortal21.pos.y += 1.5;

	auto p1 = makeDrawPortal(pPortal1, cPortal1, cPortal11, portalColor0, portalColor1);
	auto p2 = makeDrawPortal(pPortal1, cPortal2, cPortal21, portalColor0, portalColor1);
	portals.push_back(p1.first); portals.push_back(p1.second);
	portals.push_back(p2.first); portals.push_back(p2.second);

	polygons.push_back({{}, {}, false, 0, glm::vec3(0.5, 0, 0)});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0, 1});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0, 1});

	polygons.push_back({{}, {}, false, 0, glm::vec3(0, 0.5, 0)});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0, 1});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0, 1});

	polygons.push_back({{}, {}, false, 0, glm::vec3(0, 0, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0, 1});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0, 1});

	polygons.push_back({{}, {}, false, 0, glm::vec3(0, 0.5, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0, 1});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0, 1});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0, 1});

	polygons.push_back({{}, {}, false, 0, glm::vec3(0.5, 0, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0, 1});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0, 1});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0, 1});

	polygons.push_back({{}, {}, false, 0, glm::vec3(0.5, 0.5, 0)});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0, 1});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0, 1});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0, 1});

	update_cam();
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
	glewExperimental = true; // Needed for core profile
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
