#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spob/spob.h>
#include <array>

#include <json.hpp>

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

int drawTime = 0, drawCount = 0;

void writeFps(int value) {
	if (drawCount != 0) {
		std::cout << 1000.0 / (drawTime / drawCount) << " fps, drawCount: " << drawCount << std::endl;
		drawTime = 0;
		drawCount = 0;
	}
	glutTimerFunc(1000, writeFps, 100);
}

//-----------------------------------------------------------------------------
void display() {
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glClearColor(0.6, 0.6, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	sceneDrawer->drawAll(w, h);

	glutSwapBuffers();

	drawTime += glutGet(GLUT_ELAPSED_TIME) - timeSinceStart;
	drawCount++;
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
		cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 0.5*(y - l_starty));
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

	if (key == '+' || key == '=') ++(*sceneDrawer);
	if (key == '-') --(*sceneDrawer);

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void init() {
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
	std::string filename = "scene.json";
	if (argc > 1)
		filename = std::string(argv[1]);

	scene::json js;
	std::ifstream fin(filename);
	fin >> js;
	fin.close();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
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
	glutTimerFunc(1000, writeFps, 100);

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
	sceneDrawer = new SceneDrawer(scene::parseScene(js), cam_rotate_around, cam_spheric_pos);
	glutMainLoop();
}