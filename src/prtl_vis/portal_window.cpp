#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>
#include <array>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spob/spob.h>

#include <prtl_vis/portal_window.h>
#include <prtl_vis/plane.h>
#include <prtl_vis/shader.h>
#include <prtl_vis/framebuffer.h>

namespace prtl_vis
{

//-----------------------------------------------------------------------------
void printText(int x, int y, const std::string& str, int w, int h) {
	int c = std::count(str.begin(), str.end(), '\n');

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(0, 0, 0);
	//glColor3f(1, 1, 1);
	c--; glRasterPos2i(x, y + 15 * c);
	for (const auto& i : str) {
		if (i == '\n') {
			c--; glRasterPos2f(x, y + 15 * c);
		}
		else
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, i);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
PortalsOpenglWindow* PortalsOpenglWindow::currentThis = nullptr;

//-----------------------------------------------------------------------------
PortalsOpenglWindow::PortalsOpenglWindow(const scene::Scene& scene, int w, int h, std::string title) : 
		w(w), h(h),
		depthMax(3),
		fps(0), 
		drawSceneCount1(0), 
		drawTime(0), 
		drawCount(0), 
		drawSceneCount(0), 
		drawFps(true),
		drawSceneDrawed(false),
		drawCamPos(false),
		drawDepth(false),
		drawFrame(true) 
	{
    putenv("MESA_GL_VERSION_OVERRIDE=3.3");
    putenv("MESA_GLSL_VERSION_OVERRIDE=330");

	int argc1 = 0;
	char** argv1 = new char*[1];
	argv1[0] = "";
	delete[] argv1;

	glutInit(&argc1, argv1);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(w, h);
	glutCreateWindow(title.c_str());

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		throw std::logic_error("Failed to initialize GLEW");
	}

	currentThis = this;

	glutDisplayFunc(_display);
	glutReshapeFunc(_reshape);

	glutMouseFunc(_mouse);
	glutMotionFunc(_motion);
	glutKeyboardFunc(_keyboard);
	glutMouseWheelFunc(_wheel);
	glutTimerFunc(1000, _writeFps, 100);

	glEnable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);

	// И почему это ломает цвета, когда нет текстур, а?
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);

	init();
	createMenu();

	sceneDrawer = std::make_shared<SceneDrawer>(scene, cam_rotate_around, cam_spheric_pos, depthMax);
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::showWindowAndWaitClosing(void) {
	FrameBufferGetter::clear();
	glutMainLoop();
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::writeFps(int value) {
	if (drawCount != 0) {
		fps = 1000.0 / (drawTime / drawCount);
		drawSceneCount1 = drawSceneCount/drawCount;
		drawTime = 0;
		drawCount = 0;
		drawSceneCount = 0;
	}
	glutTimerFunc(1000, _writeFps, 100);
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::display() {
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	glClearColor(0.95, 0.95, 0.95, 1.0);
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

	if (!sout.str().empty()) {
		printText(5, 5, sout.str(), w, h);
		update_cam();
	}

	glutSwapBuffers();

	drawTime += glutGet(GLUT_ELAPSED_TIME) - timeSinceStart;
	drawCount++;
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::update_cam(void) {
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
void PortalsOpenglWindow::reshape(int w1, int h1) {
	w = w1; h = h1;
	FrameBufferGetter::clear();
	glViewport(0, 0, w, h);
	update_cam();
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::mouse(int button, int state, int x, int y) {
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
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::motion(int x, int y) {
	if (l_moving) {
		cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 0.5*(x - l_startx));
		cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 0.5*(y - l_starty));
		l_startx = x;
		l_starty = y;

		if (cam_spheric_pos.y < 0.01) cam_spheric_pos.y = 0.01;
		if (cam_spheric_pos.y > _SPOB_PI-0.01) cam_spheric_pos.y = _SPOB_PI-0.01;

		update_cam();
		glutPostRedisplay();
	}
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_spheric_pos.z += 0.01 * abs(dir);
	else cam_spheric_pos.z -= 0.01 * abs(dir);

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::keyboard(unsigned char key, int x, int y) {
	if (key == 'd') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) + 3.0);
	if (key == 'a') cam_spheric_pos.x = glm::radians(glm::degrees(cam_spheric_pos.x) - 3.0);

	if (key == 's') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) + 3.0);
	if (key == 'w') cam_spheric_pos.y = glm::radians(glm::degrees(cam_spheric_pos.y) - 3.0);

	if (cam_spheric_pos.y < 0.01) cam_spheric_pos.y = 0.01;
	if (cam_spheric_pos.y > _SPOB_PI - 0.01) cam_spheric_pos.y = _SPOB_PI - 0.01;

	if (key == '{' || key == '[') wheel(0, 1, 0, 0);
	if (key == '}' || key == ']') wheel(0, -1, 0, 0);

	if (key == '(') wheel(0, 10, 0, 0);
	if (key == ')') wheel(0, -10, 0, 0);

	if (key == '+' || key == '=') ++(*sceneDrawer);
	if (key == '-') --(*sceneDrawer);

	if (key == 'h') sceneDrawer->setCam(cam_rotate_around, cam_spheric_pos);

	if (key == 'l') sceneDrawer->turnLight();

	if (key == 'b') glutLeaveMainLoop();

	update_cam();

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::init(void) {
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::menu(int num) {
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

		case 200: drawFps = !drawFps;	   break;
		case 201: drawCamPos = !drawCamPos; break;
		case 202: drawDepth = !drawDepth;   break;
		case 203: drawFrame = !drawFrame;   break;
		case 204: drawSceneDrawed = !drawSceneDrawed; break;

		case 0: keyboard('h', 0, 0); break;
		case 1: keyboard('+', 0, 0); break;
		case 2: keyboard('-', 0, 0); break;
		case 3: keyboard('{', 0, 0); break;
		case 4: keyboard('}', 0, 0); break;
		case 5: keyboard('l', 0, 0); break;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::createMenu(void) {
	int depthMenu = glutCreateMenu(_menu);
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

	int debugMenu = glutCreateMenu(_menu);
	glutAddMenuEntry("FPS", 200);
	glutAddMenuEntry("Cam position", 201);
	glutAddMenuEntry("Depth", 202);
	glutAddMenuEntry("Frame", 203);
	glutAddMenuEntry("Scene drawed count", 204);

	int mainMenu = glutCreateMenu(_menu);
	glutAddMenuEntry("Move cam to start position  h", 0);
	glutAddMenuEntry("Next frame				  +", 1);
	glutAddMenuEntry("Previous frame			  -", 2);
	glutAddMenuEntry("Zoom in					 {", 3);
	glutAddMenuEntry("Zoom out					}", 4);
	glutAddMenuEntry("Turn light off/on		   l", 5);
	glutAddSubMenu("Depth", depthMenu);
	glutAddSubMenu("Show debug information", debugMenu);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void PortalsOpenglWindow::_writeFps(int value) {
	currentThis->writeFps(value);
}
void PortalsOpenglWindow::_display(void) {
	currentThis->display();
}
void PortalsOpenglWindow::_reshape(int w1, int h1) {
	currentThis->reshape(w1, h1);
}
void PortalsOpenglWindow::_mouse(int button, int state, int x, int y) {
	currentThis->mouse(button, state, x, y);
}
void PortalsOpenglWindow::_motion(int x, int y) {
	currentThis->motion(x, y);
}
void PortalsOpenglWindow::_wheel(int button, int dir, int x, int y) {
	currentThis->wheel(button, dir, x, y);
}
void PortalsOpenglWindow::_keyboard(unsigned char key, int x, int y) {
	currentThis->keyboard(key, x, y);
}
void PortalsOpenglWindow::_menu(int num) {
	currentThis->menu(num);
}
void PortalsOpenglWindow::_createMenu(void) {
	currentThis->createMenu();
}

}