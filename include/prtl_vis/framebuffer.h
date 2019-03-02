#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <prtl_vis/fragment.h>

class FrameBufferDrawer;

//-----------------------------------------------------------------------------
class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	void activate(bool isClear = true) const;
	void disable(bool isClear = true) const;

	GLuint getFrameBuffer(void) const;
	GLuint getColorTexture(void) const;
	GLuint getDepthTexture(void) const;

	int getWidth(void) const;
	int getHeight(void) const;
private:
	GLuint f, c, d;
	int width, height;

	static std::vector<GLuint> f_stack;

	friend FrameBufferDrawer;
};

//-----------------------------------------------------------------------------
class FrameBufferGetter {
public:
	static const FrameBuffer& get(int w, int h, bool isClear);
	static void unget(void);
	static void clear(void);
private:
	static std::vector<std::shared_ptr<FrameBuffer>> f_stack;
	static int pos;
	static bool isMustClear;
};

//-----------------------------------------------------------------------------
class FrameBufferMerger
{
public:
	static void merge(const FrameBuffer& f1, const FrameBuffer& f2);
private:
	GLuint program;
	GLuint c1ID, d1ID, c2ID, d2ID;

	FrameBufferMerger();
};

//-----------------------------------------------------------------------------
class FrameBufferDrawer
{
public:
	static void draw(const FrameBuffer& f1);
private:
	GLuint program;
	GLuint cID, dID;

	FrameBufferDrawer();
};

//-----------------------------------------------------------------------------
class ScreenFiller
{
public:
	static void fill(void);
private:
	GLuint quad_vertexbuffer;

	ScreenFiller();
};

//-----------------------------------------------------------------------------
class PolygonFramebufferDrawer
{
public:
	static void draw(const FrameBuffer& f1, const std::vector<Fragment>& fragments);
private:
	GLuint program;
	GLuint cID, dID;

	PolygonFramebufferDrawer();
};