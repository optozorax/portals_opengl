#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <GL/glew.h>
#include <GL/freeglut.h>
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
int depthMax = 3;
double pi = _SPOB_PI;
int w = 800, h = 600;

glm::vec3 cam_spheric_pos;
glm::vec3 cam_rotate_around;

int fps = 0, drawSceneCount1 = 0;
int drawTime = 0, drawCount = 0, drawSceneCount = 0;
bool drawFps = true;
bool drawSceneDrawed = false;
bool drawCamPos = false;
bool drawDepth = false;
bool drawFrame = true;

void update_cam();

//-----------------------------------------------------------------------------
void printText(int x, int y, const std::string& str) {
	int c = std::count(str.begin(), str.end(), '\n');

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glColor3f(0, 0, 0);
	glColor3f(1, 1, 1);
	c--; glRasterPos2i(x, y + 15 * c);
	for (const auto& i : str) {
		if (i == '\n') {
			c--; glRasterPos2f(x, y + 15 * c);
		}
		else
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, i);
	}

	update_cam();
}

//-----------------------------------------------------------------------------
void writeFps(int value) {
	if (drawCount != 0) {
		fps = 1000.0 / (drawTime / drawCount);
		drawSceneCount1 = drawSceneCount/drawCount;
		drawTime = 0;
		drawCount = 0;
		drawSceneCount = 0;
	}
	glutTimerFunc(1000, writeFps, 100);
}

//-----------------------------------------------------------------------------
void display() {
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glClearColor(0.6, 0.6, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	drawSceneCount += sceneDrawer->drawAll(w, h);

	std::stringstream sout;

	if (drawFps) {
		sout << "FPS: " << fps << std::endl;
	}
	if (drawSceneDrawed) {
		sout << "Scene drawed: " << drawSceneCount1 << std::endl;
	}
	if (drawCamPos) {
		sout << "Cam rotate point: (" << std::fixed << std::setprecision(2)
			<< std::setw(6) << cam_rotate_around.x << ", "
			<< std::setw(6) << cam_rotate_around.y << ", "
			<< std::setw(6) << cam_rotate_around.z << ")" << std::endl;
		sout << "Cam spheric pos:  (" << std::fixed << std::setprecision(2)
			<< std::setw(6) << cam_spheric_pos.x << ", "
			<< std::setw(6) << cam_spheric_pos.y << ", "
			<< std::setw(6) << cam_spheric_pos.z << ")" << std::endl;
	}
	if (drawDepth) {
		sout << "Depth: " << sceneDrawer->getMaxDepth() << std::endl;
	}
	if (drawFrame) {
		sout << "Frame: " << sceneDrawer->getCurrentFrame() << "/" << sceneDrawer->getMaxFrame() << std::endl;
	}

	if (!sout.str().empty())
		printText(5, 5, sout.str());

	glutSwapBuffers();

	drawTime += glutGet(GLUT_ELAPSED_TIME) - timeSinceStart;
	drawCount++;
}

//-----------------------------------------------------------------------------
void update_cam(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, double(w)/h, 0.1, 1000.0);

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
	/*if (button == GLUT_RIGHT_BUTTON) {
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
	}*/
}

//-----------------------------------------------------------------------------
void motion(int x, int y) {
	if (l_moving) {
		cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 0.5*(x - l_startx));
		cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 0.5*(y - l_starty));
		l_startx = x;
		l_starty = y;

		if (cam_spheric_pos.y < 0.01) cam_spheric_pos.y = 0.01;
		if (cam_spheric_pos.y > pi-0.01) cam_spheric_pos.y = pi-0.01;

		update_cam();
		glutPostRedisplay();
	}
	/*if (r_moving) {
		cam_rotate_around.x -= 0.01*(x-r_startx);
		cam_rotate_around.y -= 0.01*(y-r_starty);
		r_startx = x;
		r_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (m_moving) {
		cam_rotate_around.z += 0.01*(y-m_starty);
		m_starty = y;
		update_cam();
		glutPostRedisplay();
	}*/
}

//-----------------------------------------------------------------------------
void wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_spheric_pos.z += 0.1;
	else cam_spheric_pos.z -= 0.1;

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y) {
	/*if (key == 'a') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) + 3.0);
	if (key == 'o') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 3.0);

	if (key == 'e') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) + 3.0);
	if (key == 'u') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 3.0);
	if (cam_spheric_pos.y < 0.01) cam_spheric_pos.y = 0.01;
	if (cam_spheric_pos.y > pi - 0.01) cam_spheric_pos.y = pi - 0.01;*/

	if (key == '{') wheel(0, 1, 0, 0);
	if (key == '}') wheel(0, -1, 0, 0);

	if (key == '+' || key == '=') ++(*sceneDrawer);
	if (key == '-') --(*sceneDrawer);

	if (key == 'h') sceneDrawer->setCam(cam_rotate_around, cam_spheric_pos);

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void init() {
}

//-----------------------------------------------------------------------------
void menu(int num) {
	switch (num) {
		case 101: sceneDrawer->setMaxDepth(01); break;
		case 102: sceneDrawer->setMaxDepth(02); break;
		case 103: sceneDrawer->setMaxDepth(03); break;
		case 104: sceneDrawer->setMaxDepth(04); break;
		case 105: sceneDrawer->setMaxDepth(05); break;
		case 106: sceneDrawer->setMaxDepth(06); break;
		case 110: sceneDrawer->setMaxDepth(10); break;
		case 115: sceneDrawer->setMaxDepth(15); break;
		case 120: sceneDrawer->setMaxDepth(20); break;
		case 130: sceneDrawer->setMaxDepth(30); break;
		case 199: sceneDrawer->setMaxDepth(99); break;

		case 200: drawFps = !drawFps;       break;
		case 201: drawCamPos = !drawCamPos; break;
		case 202: drawDepth = !drawDepth;   break;
		case 203: drawFrame = !drawFrame;   break;
		case 204: drawSceneDrawed = !drawSceneDrawed; break;

		case 0: keyboard('h', 0, 0); break;
		case 1: keyboard('+', 0, 0); break;
		case 2: keyboard('-', 0, 0); break;
		case 3: keyboard('{', 0, 0); break;
		case 4: keyboard('}', 0, 0); break;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void createMenu(void) {
	int depthMenu = glutCreateMenu(menu);
	glutAddMenuEntry("1", 101);
	glutAddMenuEntry("2", 102);
	glutAddMenuEntry("3", 103);
	glutAddMenuEntry("4", 104);
	glutAddMenuEntry("5", 105);
	glutAddMenuEntry("6", 106);
	glutAddMenuEntry("10", 110);
	glutAddMenuEntry("15", 115);
	glutAddMenuEntry("20", 120);
	glutAddMenuEntry("30", 130);
	glutAddMenuEntry("99 (Not recommended!)", 199);

	int debugMenu = glutCreateMenu(menu);
	glutAddMenuEntry("FPS", 200);
	glutAddMenuEntry("Cam position", 201);
	glutAddMenuEntry("Depth", 202);
	glutAddMenuEntry("Frame", 203);
	glutAddMenuEntry("Scene drawed count", 204);

	int mainMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Move cam to start position  h", 0);
	glutAddMenuEntry("Next frame                  +", 1);
	glutAddMenuEntry("Previous frame              -", 2);
	glutAddMenuEntry("Zoom in                     {", 3);
	glutAddMenuEntry("Zoom out                    }", 4);
	glutAddSubMenu("Depth", depthMenu);
	glutAddSubMenu("Show debug information", debugMenu);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
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
	glutMouseWheelFunc(wheel);
	glutTimerFunc(1000, writeFps, 100);

	glEnable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);

	// И почему это ломает цвета, когда нет текстур, а?
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);

	init();
	createMenu();
	sceneDrawer = new SceneDrawer(scene::parseScene(js), cam_rotate_around, cam_spheric_pos, 6);
	glutMainLoop();
}