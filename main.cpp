#include <fstream>
#include <iostream>
#include <json.hpp>
#include <prtl_vis/portal_window.h>

int main(int argc, char** argv) {
	std::string filename = "debug.json";
	if (argc > 1)
		filename = std::string(argv[1]);

	scene::json js;
	std::ifstream fin(filename);
	fin >> js;
	fin.close();

	prtl_vis::PortalsOpenglWindow window(scene::parseScene(js));
	window.showWindowAndWaitClosing();

	// It also possible
	// {
	// 	std::cout << "1" << std::endl;
	// 	prtl_vis::PortalsOpenglWindow window(scene::parseScene(js));
	// 	window.showWindowAndWaitClosing();
	// 	std::cout << "1e" << std::endl;
	// }
	// {
	// 	std::cout << "2" << std::endl;
	// 	prtl_vis::PortalsOpenglWindow window(scene::parseScene(js));
	// 	window.showWindowAndWaitClosing();
	// 	std::cout << "2e" << std::endl;
	// }
}