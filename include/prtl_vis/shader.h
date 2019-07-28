#pragma once

#include <GL/glew.h>

GLuint LoadShadersFromFiles(const char* vertex_file_path, const char* fragment_file_path);
GLuint LoadShadersFromString(const char* vertex_file, const char* fragment_file, const char* name);