// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.h"
#include "framebuffer.h"

static void drawTorus(int numMajor, int numMinor, float majorRadius, float minorRadius)
{
    static double PI = 3.1415926535897932384626433832795;

    double majorStep = 2.0 * PI / numMajor;
    double minorStep = 2.0 * PI / numMinor;

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (int i = 0; i < numMajor; ++i) {
        double a0 = i * majorStep;
        double a1 = a0 + majorStep;
        GLdouble x0 = cos(a0);
        GLdouble y0 = sin(a0);
        GLdouble x1 = cos(a1);
        GLdouble y1 = sin(a1);

        glBegin(GL_TRIANGLE_STRIP);

            for (int j = 0; j <= numMinor; ++j) {
                double b = j * minorStep;
                GLdouble c = cos(b);
                GLdouble r = minorRadius * c + majorRadius;
                GLdouble z = minorRadius * sin(b);

                glNormal3d(x0 * c, y0 * c, z / minorRadius);
                glTexCoord2d(i / (GLdouble) numMajor, j / (GLdouble) numMinor);
                glVertex3d(x0 * r, y0 * r, z);

                glNormal3d(x1 * c, y1 * c, z / minorRadius);
                glTexCoord2d((i + 1) / (GLdouble) numMajor, j / (GLdouble) numMinor);
                glVertex3d(x1 * r, y1 * r, z);
            }

        glEnd();
    }
}


static void renderMesh(float rotateAngle, float r, float g, float b)
{
    glLoadIdentity();
    glRotatef(rotateAngle, 1.f, 1.f, 1.f);

    glBegin(GL_LINES);
        glColor4ub(255,0,0,255);
        glVertex3f(0,0,0);
        glVertex3f(0.75,0,0);

        glColor4ub(0,0,255,255);
        glVertex3f(0,0,0);
        glVertex3f(0,0,0.75);

        glColor4ub(0,255,0,255);
        glVertex3f(0,0,0);
        glVertex3f(0,0.75,0);
    glEnd();

    glColor4f(r, g, b, 1.0);
    drawTorus(5, 5, 0.5, .2);
    glLoadIdentity();
}


int main()
{
    GLFWwindow* window;

    if(!glfwInit()) {
        fprintf(stderr, "Failed to start glfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //Create Window
    window = glfwCreateWindow(1000, 600, "OpenGL with GLFW", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to make window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

    FrameBuffer f1(1000, 600);
    FrameBuffer f2(1000, 600);
    FrameBuffer f3(1000, 600);
    FrameBuffer f4(1000, 600);

    auto startTime = glfwGetTime();
    int fps = 0;
    while (!glfwWindowShouldClose(window)) {
        double mx, my, t, dt;
        int winWidth, winHeight;
        int fbWidth, fbHeight;

        glfwGetCursorPos(window, &mx, &my);
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.3, 0.3, 0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        #define DRAW_WITH_FRAMEBUFFERS
        #define USE_MERGER
     	//#define FIRST_WAY
     	//#define TEST_AFTER_DRAW

     	#ifdef DRAW_WITH_FRAMEBUFFERS
     		#ifdef USE_MERGER
		        #ifdef FIRST_WAY
			        f1.activate();
			            f3.activate();
			            renderMesh((mx+my)/2.0, 1, 0, 0);
			            f3.disable();

			            f4.activate();
			            renderMesh(sqrt(mx*my), 0, 1, 0);
			            f4.disable();

			            FrameBufferMerger::merge(f3, f4);
					f1.disable();

					f2.activate();
					renderMesh(my, 0, 0, 1);
					f2.disable();

			        FrameBufferMerger::merge(f1, f2);
		        #else
			        f3.activate();
			        renderMesh((mx+my)/2.0, 1, 0, 0);
			        f3.disable(false);

			        f4.activate();
			        renderMesh(sqrt(mx*my), 0, 1, 0);
			        f4.disable();

			        f3.activate(false);
			        FrameBufferMerger::merge(f3, f4);
			        f3.disable(false);

					f2.activate();
					renderMesh(my, 0, 0, 1);
					f2.disable();

			        FrameBufferMerger::merge(f3, f2);
		        #endif
			#else
			    #ifdef TEST_AFTER_DRAW
			        f3.activate();
			        renderMesh((mx+my)/2.0, 1, 0, 0);
			        f3.disable(false);

			        f4.activate();
			        renderMesh(sqrt(mx*my), 0, 1, 0);
			        f4.disable();

			        f3.activate(false);
			        FrameBufferMerger::merge(f3, f4);
			        f3.disable(false);

					FrameBufferDrawer::draw(f3);

					renderMesh(my, 0, 0, 1);
				#else
			        f3.activate();
			        renderMesh((mx+my)/2.0, 1, 0, 0);
			        f3.disable(false);

			        f4.activate();
			        renderMesh(sqrt(mx*my), 0, 1, 0);
			        f4.disable();

			        f3.activate(false);
			        FrameBufferMerger::merge(f3, f4);
			        f3.disable(false);

					f2.activate();
					renderMesh(my, 0, 0, 1);
					f2.disable();

					f3.activate(false);
			        FrameBufferMerger::merge(f3, f2);
			        f3.disable(false);

					FrameBufferDrawer::draw(f3);
				#endif
			#endif
	    #else
	        renderMesh((mx+my)/2.0, 1, 0, 0);
	        renderMesh(sqrt(mx*my), 0, 1, 0);
	        renderMesh(my, 0, 0, 1);
	    #endif

        glfwSwapBuffers(window);
        glfwPollEvents();
        fps++;

        if (glfwGetTime() - startTime > 1) {
            std::cout << "fps: " << fps/(glfwGetTime() - startTime) << std::endl;
            startTime = glfwGetTime();
            fps = 0;
        }
    }

    glfwTerminate();
    return 0;
}
