/*
colcycle - color cycling image viewer
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "app.h"

static int init(int argc, char **argv);
static void cleanup(void);
static void display(void);
static void idle(void);
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void toggle_fullscreen(void);
static unsigned int create_program(const char *vsdr, const char *psdr);
static unsigned int create_shader(unsigned int type, const char *sdr);
static unsigned int next_pow2(unsigned int x);
static int enable_vsync(void);

static int tex_xsz, tex_ysz;
static unsigned int img_tex, pal_tex, prog;
static unsigned char pal[256 * 3];
static int pal_valid;

static float verts[] = {
	-1, -1, 1, -1, 1, 1,
	-1, -1, 1, 1, -1, 1
};
static unsigned int vbo;

static const char *vsdr =
	"uniform mat4 xform;\n"
	"uniform vec2 uvscale;\n"
	"attribute vec4 attr_vertex;\n"
	"varying vec2 uv;\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = xform * attr_vertex;\n"
	"\tuv = (attr_vertex.xy * vec2(0.5, -0.5) + 0.5) * uvscale;\n"
	"}\n";

static const char *psdr =
	"uniform sampler2D img_tex;\n"
	"uniform sampler1D pal_tex;\n"
	"varying vec2 uv;\n"
	"void main()\n"
	"{\n"
	"\tfloat cidx = texture2D(img_tex, uv).x;\n"
	"\tvec3 color = texture1D(pal_tex, cidx).xyz;\n"
	"\tgl_FragColor.xyz = color;\n"
	"\tgl_FragColor.a = 1.0;\n"
	"}\n";

int main(int argc, char **argv)
{
	const char *env = getenv("FULLSCREEN");

	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("colcycle/GL");

	if(env && atoi(env)) {
		toggle_fullscreen();
	}

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);

	if(init(argc, argv) == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}

void app_quit(void)
{
	exit(0);
}

unsigned long get_msec(void)
{
	return glutGet(GLUT_ELAPSED_TIME);
}

void set_palette(int idx, int r, int g, int b)
{
	unsigned char *pptr = pal + idx * 3;
	pptr[0] = r;
	pptr[1] = g;
	pptr[2] = b;
	pal_valid = 0;
}

static int init(int argc, char **argv)
{
	int loc;

	fbwidth = 640;
	fbheight = 480;
	fbpixels = malloc(fbwidth * fbheight);

	if(app_init(argc, argv) == -1) {
		return -1;
	}

	glewInit();

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

	tex_xsz = next_pow2(fbwidth);
	tex_ysz = next_pow2(fbheight);

	glGenTextures(1, &img_tex);
	glBindTexture(GL_TEXTURE_2D, img_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, tex_xsz, tex_ysz, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fbwidth, fbheight, GL_LUMINANCE, GL_UNSIGNED_BYTE, fbpixels);

	glGenTextures(1, &pal_tex);
	glBindTexture(GL_TEXTURE_1D, pal_tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pal);

	if(!(prog = create_program(vsdr, psdr))) {
		return -1;
	}
	glBindAttribLocation(prog, 0, "attr_vertex");
	glLinkProgram(prog);
	glUseProgram(prog);
	if((loc = glGetUniformLocation(prog, "img_tex")) >= 0) {
		glUniform1i(loc, 0);
	}
	if((loc = glGetUniformLocation(prog, "pal_tex")) >= 0) {
		glUniform1i(loc, 1);
	}
	if((loc = glGetUniformLocation(prog, "uvscale")) >= 0) {
		glUniform2f(loc, (float)fbwidth / (float)tex_xsz, (float)fbheight / (float)tex_ysz);
	}

	if(enable_vsync() == -1) {
		fprintf(stderr, "failed to enable vsync\n");
	}
	return 0;
}

static void cleanup(void)
{
	app_cleanup();
}

static void display(void)
{
	time_msec = get_msec();

	*(uint32_t*)fbpixels = 0xbadf00d;
	app_draw();

	if(*(uint32_t*)fbpixels != 0xbadf00d) {
		/* update texture data if the framebuffer changed */
		glBindTexture(GL_TEXTURE_2D, img_tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fbwidth, fbheight, GL_LUMINANCE, GL_UNSIGNED_BYTE, fbpixels);
	}
	if(!pal_valid) {
		/* ipdate the palette texture */
		glBindTexture(GL_TEXTURE_1D, pal_tex);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGB, GL_UNSIGNED_BYTE, pal);
		pal_valid = 1;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, pal_tex);
	glActiveTexture(GL_TEXTURE0);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	int loc;
	float aspect = (float)x / (float)y;
	float fbaspect = (float)fbwidth / (float)fbheight;
	float xform[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	glViewport(0, 0, x, y);

	if(aspect > fbaspect) {
		xform[0] = fbaspect / aspect;
	} else if(fbaspect > aspect) {
		xform[5] = aspect / fbaspect;
	}

	glUseProgram(prog);
	if((loc = glGetUniformLocation(prog, "xform")) >= 0) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, xform);
	}
}

static void keydown(unsigned char key, int x, int y)
{
	if(key >= '1' && key <= '9') {
		int scale = key - '0';
		glutReshapeWindow(fbwidth * scale, fbheight * scale);
		return;
	}
	if(key == 'f' || key == 'F') {
		toggle_fullscreen();
		return;
	}
	app_keyboard(key, 1);
}

static void toggle_fullscreen(void)
{
	static int fullscr, win_width, win_height;

	fullscr = !fullscr;
	if(fullscr) {
		win_width = glutGet(GLUT_WINDOW_WIDTH);
		win_height = glutGet(GLUT_WINDOW_HEIGHT);
		glutFullScreen();
	} else {
		glutReshapeWindow(win_width, win_height);
	}
}

static unsigned int create_program(const char *vsdr, const char *psdr)
{
	unsigned int vs, ps, prog;
	int status;

	if(!(vs = create_shader(GL_VERTEX_SHADER, vsdr))) {
		return 0;
	}
	if(!(ps = create_shader(GL_FRAGMENT_SHADER, psdr))) {
		glDeleteShader(vs);
		return 0;
	}

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, ps);
	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if(!status) {
		fprintf(stderr, "failed to link shader program\n");
		glDeleteProgram(prog);
		prog = 0;
	}
	return prog;
}

static unsigned int create_shader(unsigned int type, const char *src)
{
	unsigned int sdr;
	int status, info_len;

	sdr = glCreateShader(type);
	glShaderSource(sdr, 1, &src, 0);
	glCompileShader(sdr);

	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);
	if(!status) {
		fprintf(stderr, "failed to compile %s shader\n", type == GL_VERTEX_SHADER ? "vertex" : "pixel");
	}

	glGetShaderiv(sdr, GL_INFO_LOG_LENGTH, &info_len);
	if(info_len) {
		char *buf = alloca(info_len + 1);
		glGetShaderInfoLog(sdr, info_len, 0, buf);
		buf[info_len] = 0;
		if(buf[0]) {
			fprintf(stderr, "compiler output:\n%s\n", buf);
		}
	}

	if(!status) {
		glDeleteShader(sdr);
		sdr = 0;
	}
	return sdr;
}

static unsigned int next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

#ifdef __unix__
#include <GL/glxew.h>
static int enable_vsync(void)
{
	if(GLXEW_EXT_swap_control) {
		Display *dpy = glXGetCurrentDisplay();
		Window win = glXGetCurrentDrawable();
		glXSwapIntervalEXT(dpy, win, 1);
		return 0;
	}
	if(GLXEW_MESA_swap_control) {
		glXSwapIntervalMESA(1);
		return 0;
	}
	if(GLXEW_SGI_swap_control) {
		glXSwapIntervalSGI(1);
		return 0;
	}
	return -1;
}
#endif	/* __unix__ */
#ifdef WIN32
#include <GL/wglew.h>
static int enable_vsync(void)
{
	if(WGLEW_EXT_swap_control) {
		wglSwapIntervalEXT(1);
		return 0;
	}
	return -1;
}
#endif	/* WIN32 */
