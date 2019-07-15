#include <memory>
#include <iostream>

#include <GL/glew.h>

#include <prtl_vis/shader.h>
#include <prtl_vis/framebuffer.h>

std::vector<GLuint> FrameBuffer::f_stack(1, 0);

//-----------------------------------------------------------------------------
FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height) {
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);

    glGenTextures(1, &c);
    glBindTexture(GL_TEXTURE_2D, c);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /*glGenTextures(1, &d);
    glBindTexture(GL_TEXTURE_2D, d);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, c, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, d, 0);

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
    glDrawBuffers(2, DrawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::exception();*/

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &d);
    glBindTexture(GL_TEXTURE_2D, d);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,  GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, c, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, d, 0);

    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_STENCIL_ATTACHMENT};
    glDrawBuffers(2, DrawBuffers);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus == GL_FRAMEBUFFER_UNSUPPORTED)
        std::cout << "implementation is not supported by OpenGL driver " << fboStatus << std::endl;
    if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
        throw std::exception();

    activate();
    disable();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &f);
	glDeleteTextures(1, &c);
	glDeleteTextures(1, &d);
}

//-----------------------------------------------------------------------------
void FrameBuffer::activate(bool isClear) const {
    f_stack.push_back(f);
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, f);
	glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	if (isClear) {
        // Clear the screen
        //glClearColor(0.3, 0.3, 0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

//-----------------------------------------------------------------------------
void FrameBuffer::disable(bool isClear) const {
    f_stack.pop_back();
	// Render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, f_stack.back());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
	glViewport(0, 0, width, height);
	// Clear the screen

	if (isClear) {
		//glClearColor(0.3, 0.3, 0.3, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

//-----------------------------------------------------------------------------
GLuint FrameBuffer::getFrameBuffer(void) const {
	return f;
}

//-----------------------------------------------------------------------------
GLuint FrameBuffer::getColorTexture(void) const {
	return c;
}

//-----------------------------------------------------------------------------
GLuint FrameBuffer::getDepthTexture(void) const {
	return d;
}

//-----------------------------------------------------------------------------
int FrameBuffer::getWidth(void) const {
	return width;
}

//-----------------------------------------------------------------------------
int FrameBuffer::getHeight(void) const {
	return height;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<FrameBuffer>> FrameBufferGetter::f_stack;
int FrameBufferGetter::pos(0);
bool FrameBufferGetter::isMustClear(false);

//-----------------------------------------------------------------------------
const FrameBuffer& FrameBufferGetter::get(int w, int h, bool isClear) {
	if (pos == f_stack.size())
		f_stack.emplace_back(new FrameBuffer(w, h));
	const FrameBuffer& result(*f_stack[pos]);
	if (isClear) {
		result.activate();
		result.disable();
	}
	pos++;
	return result;
}

//-----------------------------------------------------------------------------
void FrameBufferGetter::unget(void) {
	pos--;
	if (pos == 0 && isMustClear) {
		clear();
	}
}

//-----------------------------------------------------------------------------
void FrameBufferGetter::clear(void) {
	if (pos == 0) {
		f_stack.clear();
		pos = 0;
		isMustClear = false;
	} else {
		isMustClear = true;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
FrameBufferMerger::FrameBufferMerger() {
	program = LoadShaders("merge.vertex.glsl", "merge.fragment.glsl" );
	c1ID = glGetUniformLocation(program, "Color1");
	d1ID = glGetUniformLocation(program, "Depth1");
	c2ID = glGetUniformLocation(program, "Color2");
	d2ID = glGetUniformLocation(program, "Depth2");
}

//-----------------------------------------------------------------------------
void FrameBufferMerger::merge(const FrameBuffer& f1, const FrameBuffer& f2) {
    static FrameBufferMerger merger;
	glUseProgram(merger.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, f1.getColorTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, f1.getDepthTexture());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, f2.getColorTexture());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, f2.getDepthTexture());

		glUniform1i(merger.c1ID, 0);
		glUniform1i(merger.d1ID, 1);
		glUniform1i(merger.c2ID, 2);
		glUniform1i(merger.d2ID, 3);

		ScreenFiller::fill();
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
FrameBufferDrawer::FrameBufferDrawer() {
	program = LoadShaders("draw.vertex.glsl", "draw.fragment.glsl" );
	cID = glGetUniformLocation(program, "Color");
	dID = glGetUniformLocation(program, "Depth");
}

//-----------------------------------------------------------------------------
void FrameBufferDrawer::draw(const FrameBuffer& f1) {
	static FrameBufferDrawer drawer;
	/*glUseProgram(drawer.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, f1.getColorTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, f1.getDepthTexture());

		glUniform1i(drawer.cID, 0);
		glUniform1i(drawer.dID, 1);

		ScreenFiller::fill();
	glUseProgram(0);*/
	glBindFramebuffer(GL_READ_FRAMEBUFFER, f1.getFrameBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FrameBuffer::f_stack.back());
	glBlitFramebuffer(0, 0, f1.getWidth(), f1.getHeight(),
	                  0, 0, f1.getWidth(), f1.getHeight(),
	                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
ScreenFiller::ScreenFiller() {
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
}

//-----------------------------------------------------------------------------
void ScreenFiller::fill(void) {
	static ScreenFiller filler;

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, filler.quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
PolygonFramebufferDrawer::PolygonFramebufferDrawer() {
	program = LoadShaders("drawpoly.vertex.glsl", "drawpoly.fragment.glsl" );
	cID = glGetUniformLocation(program, "Color");
	dID = glGetUniformLocation(program, "Depth");
}

//-----------------------------------------------------------------------------
void PolygonFramebufferDrawer::draw(const FrameBuffer& f1, const std::vector<Fragment>& fragments) {
	static PolygonFramebufferDrawer drawer;
	glUseProgram(drawer.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, f1.getColorTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, f1.getDepthTexture());

		glUniform1i(drawer.cID, 0);
		glUniform1i(drawer.dID, 1);

		/*glBegin(GL_POLYGON);
		for (auto& i : poly)
			glVertex3f(i.x, i.y, i.z);
		glEnd();*/
		drawFragments(fragments);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
}
