#pragma once

#include <string>
#include <memory>
#include <chrono>

#include <prtl_vis/scene_reader.h>
#include <prtl_vis/opengl_common.h>

namespace prtl_vis
{

	void printText(int x, int y, const std::string& str);

	class PortalsOpenglWindow
	{
	public:
		PortalsOpenglWindow(
			const scene::Scene& scene, 
			int w = 800, int h = 600, 
			std::string title = "PortalViewer"
		);

		void showWindowAndWaitClosing(void);

	private:
		typedef std::chrono::high_resolution_clock hrc;

		std::shared_ptr<SceneDrawer> sceneDrawer;

		int depthMax;
		int w, h;

		glm::vec3 cam_spheric_pos;
		glm::vec3 cam_rotate_around;

		int fps, drawSceneCount1;
		int drawTime, drawCount, drawSceneCount;
		bool drawFps;
		bool drawSceneDrawed;
		bool drawCamPos;
		bool drawDepth;
		bool drawFrame;
		int l_moving, l_startx, l_starty;
		bool isRecording;
		std::vector<std::tuple<spob::vec3, spob::vec3, double, int>> recorded;
		double startTime;

		void writeFps(int value);
		void display(void);
		void update_cam(void);
		void reshape(int w1, int h1);
		void mouse(int button, int state, int x, int y);
		void motion(int x, int y);
		void wheel(int button, int dir, int x, int y);
		void keyboard(unsigned char key, int x, int y);
		void init();
		void menu(int num);
		void createMenu(void);

		float getCurrentTime(void) {
			static hrc::time_point t = hrc::now();
			return std::chrono::duration<double>(hrc::now() - t).count();
		}
		void startStopRecording(void);

		static PortalsOpenglWindow* currentThis;

		static void _writeFps(int value);
		static void _display(void);
		static void _reshape(int w1, int h1);
		static void _mouse(int button, int state, int x, int y);
		static void _motion(int x, int y);
		static void _wheel(int button, int dir, int x, int y);
		static void _keyboard(unsigned char key, int x, int y);
		static void _menu(int num);
		static void _createMenu(void);

		class GlutInitiaziler
		{
		public:
			GlutInitiaziler() {}
		};
	};

}