#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <alloca.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif
#include <rbtree.h>
#include <drawtext.h>

#define FONTSZ	18

void init_gl(int argc, char **argv);
float rbvis_width(struct rbnode *tree);
void rbvis_draw(struct rbnode *tree, float x, float y);

void draw_roundbox(float xsz, float ysz, float rad, int segm, const float *col, float border, const float *bcol);
void draw_fillet(float rad, int segm);

struct rbtree *tree;
int num_nodes;
struct dtx_font *font;
char input_buffer[64];
int win_xsz, win_ysz;

int main(int argc, char **argv)
{
	int i;

	if(argv[1]) {
		if(!isdigit(argv[1][0])) {
			fprintf(stderr, "pass a fucking number, not: %s\n", argv[1]);
			return 1;
		}
		num_nodes = atoi(argv[1]);
	}

	if(!(font = dtx_open_font("linux-libertine.ttf", FONTSZ))) {
		fprintf(stderr, "failed to open font\n");
		return 1;
	}

	if(!(tree = rb_create(RB_KEY_INT))) {
		return 1;
	}

	for(i=0; i<num_nodes; i++) {
		rb_inserti(tree, i, 0);
	}

	init_gl(argc, argv);
	return 0;
}

#define NWIDTH	28.0
#define NHEIGHT 24.0
#define PADDING	0
#define DY	(NHEIGHT + NHEIGHT / 2.0)

#define VIS(node)	((struct visinfo*)(node)->data)

void disp(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void draw_rect(float x, float y, float width, float height, const char *text, int red);
void draw_link(float x0, float y0, float x1, float y1, int red);

void init_gl(int argc, char **argv)
{
	glutInitWindowSize(1280, 720);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);

	glutCreateWindow("foo");

	dtx_use_font(font, FONTSZ);

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glutMainLoop();
}

void disp(void)
{
	struct rbnode *root = rb_root(tree);

	glClearColor(0.57, 0.64, 0.59, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(input_buffer[0]) {
		char *prompt = "select node to delete: ";
		char *buf = alloca(strlen(prompt) + strlen(input_buffer));

		glPushMatrix();
		glTranslatef(10, 10, -0.9);
		glColor3f(0, 0, 0);
		sprintf(buf, "%s%s", prompt, input_buffer);
		dtx_string(buf);
		glPopMatrix();
	}

	if(root) {
		rbvis_draw(root, rbvis_width(root->left) + NWIDTH, 550);
	}

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}


float rbvis_width(struct rbnode *tree)
{
	if(!tree)
		return NWIDTH;

	return NWIDTH + rbvis_width(tree->left) + rbvis_width(tree->right) + PADDING * 3.0;
}

void rbvis_draw(struct rbnode *tree, float x, float y)
{
	float leftx, rightx, nexty;
	static const float hxsz = NWIDTH / 2.0;
	static const float hysz = NHEIGHT / 2.0;
	char text[16];

	if(!tree)
		return;

	leftx = x - (tree->left ? rbvis_width(tree->left->right) + NWIDTH : rbvis_width(tree->left) / 2.0);
	rightx = x + (tree->right ? rbvis_width(tree->right->left) + NWIDTH : rbvis_width(tree->right) / 2.0);

	nexty = y - DY;

	sprintf(text, "%d", rb_node_keyi(tree));
	draw_rect(x - hxsz, y - hysz, NWIDTH, NHEIGHT, text, tree->red);

	rbvis_draw(tree->left, leftx, nexty);
	rbvis_draw(tree->right, rightx, nexty);

	if(tree->left)
		draw_link(x, y, leftx, nexty, tree->left->red);
	if(tree->right)
		draw_link(x, y, rightx, nexty, tree->right->red);
}

void draw_rect(float x, float y, float width, float height, const char *text, int red)
{
	float node_col[] = {0.63, 0.71, 0.82, 1.0};
	float bord_col[] = {0, 0, 0, 1};

	if(red) {
		bord_col[0] = 1.0;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x + width / 2.0, y + height / 2.0, 0.0);

	draw_roundbox(width, height, 8, 6, node_col, 1.2, bord_col);

	glColor3f(0.15, 0.15, 0.15);
	glTranslatef(-dtx_string_width(text) / 2.0, -dtx_string_height(text) / 2.0, 0.1);
	dtx_string(text);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void draw_link(float x0, float y0, float x1, float y1, int red)
{
	glPushAttrib(GL_LINE_BIT);
	if(red) {
		glLineWidth(3);
	} else {
		glLineWidth(2);
	}

	glBegin(GL_LINES);
	if(red) {
		glColor3f(0.8, 0.36, 0.3);
	} else {
		glColor3f(0, 0, 0);
	}
	glVertex3f(x0, y0, -0.8);
	glVertex3f(x1, y1, -0.8);
	glEnd();

	glPopAttrib();
}

void draw_roundbox(float xsz, float ysz, float rad, int segm, const float *col, float border, const float *bcol)
{
	float hin_xsz, hin_ysz;

	if(border > 0.0f) {
		glPushMatrix();
		glTranslatef(0, 0, -0.001);
		draw_roundbox(xsz + 2 * border, ysz + 2 * border, rad + border, segm, bcol, 0.0, bcol);
		glPopMatrix();
	}

	/* half inner size */
	hin_xsz = (xsz - 2.0 * rad) / 2.0;
	hin_ysz = (ysz - 2.0 * rad) / 2.0;

	glColor4fv(col);

	glBegin(GL_QUADS);
	/* center */
	glVertex2f(-hin_xsz, -hin_ysz);
	glVertex2f(hin_xsz, -hin_ysz);
	glVertex2f(hin_xsz, hin_ysz);
	glVertex2f(-hin_xsz, hin_ysz);
	/* right */
	glVertex2f(hin_xsz, -hin_ysz);
	glVertex2f(hin_xsz + rad, -hin_ysz);
	glVertex2f(hin_xsz + rad, hin_ysz);
	glVertex2f(hin_xsz, hin_ysz);
	/* top */
	glVertex2f(-hin_xsz, hin_ysz);
	glVertex2f(hin_xsz, hin_ysz);
	glVertex2f(hin_xsz, hin_ysz + rad);
	glVertex2f(-hin_xsz, hin_ysz + rad);
	/* left */
	glVertex2f(-hin_xsz - rad, -hin_ysz);
	glVertex2f(-hin_xsz, -hin_ysz);
	glVertex2f(-hin_xsz, hin_ysz);
	glVertex2f(-hin_xsz - rad, hin_ysz);
	/* bottom */
	glVertex2f(-hin_xsz, -hin_ysz - rad);
	glVertex2f(hin_xsz, -hin_ysz - rad);
	glVertex2f(hin_xsz, -hin_ysz);
	glVertex2f(-hin_xsz, -hin_ysz);
	glEnd();

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glTranslatef(hin_xsz, hin_ysz, 0);
	draw_fillet(rad, segm);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-hin_xsz, hin_ysz, 0);
	glRotatef(90, 0, 0, 1);
	draw_fillet(rad, segm);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-hin_xsz, -hin_ysz, 0);
	glRotatef(180, 0, 0, 1);
	draw_fillet(rad, segm);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hin_xsz, -hin_ysz, 0);
	glRotatef(270, 0, 0, 1);
	draw_fillet(rad, segm);
	glPopMatrix();
}

void draw_fillet(float rad, int segm)
{
	int i;

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0, 0);
	for(i=0; i<segm; i++) {
		float theta = 0.5 * M_PI * (float)i / (float)(segm - 1);
		float x = cos(theta) * rad;
		float y = sin(theta) * rad;
		glVertex2f(x, y);
	}
	glEnd();
}


void reshape(int x, int y)
{
	win_xsz = x;
	win_ysz = y;

	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, 0, y, -1, 1);
}

void keyb(unsigned char key, int x, int y)
{
	static int wposx = -1, wposy;
	static char *inp_next;

	switch(key) {
	case 27:
		exit(0);

	case 'f':
	case 'F':
		if(wposx >= 0) {
			glutPositionWindow(wposx, wposy);
			wposx = -1;
		} else {
			wposx = glutGet(GLUT_WINDOW_X);
			wposy = glutGet(GLUT_WINDOW_Y);
			glutFullScreen();
		}
		break;

	case 'a':
		rb_inserti(tree, num_nodes++, 0);
		glutPostRedisplay();
		break;

	case 'r':
		{
			int x;
			do {
				x = rand() % 1024;
			} while(rb_findi(tree, x));

			rb_inserti(tree, x, 0);
		}
		glutPostRedisplay();
		break;

	case 'd':
		inp_next = input_buffer;
		*inp_next++ = ' ';
		*inp_next = 0;
		glutPostRedisplay();
		break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		if(inp_next) {
			*inp_next++ = key;
			*inp_next = 0;
			glutPostRedisplay();
		}
		break;

	case '\b':
		if(inp_next > input_buffer) {
			*--inp_next = 0;
			glutPostRedisplay();
		}
		break;

	case '\n':
	case '\r':
		if(inp_next) {
			int x;
			char *endp;

			*inp_next = 0;
			inp_next = 0;

			if((x = strtol(input_buffer, &endp, 10)), endp == input_buffer) {
				fprintf(stderr, "invalid input: %s\n", input_buffer);
			} else if(!rb_findi(tree, x)) {
				fprintf(stderr, "%d not found in the tree\n", x);
			} else {
				printf("deleting: %d\n", x);
				rb_deletei(tree, x);
			}
			input_buffer[0] = 0;
			glutPostRedisplay();
		}
		break;

	case 'p':
		{
			struct rbnode *node;

			rb_begin(tree);
			while((node = rb_next(tree))) {
				int key = rb_node_keyi(node);
				printf("%d ", key);
			}
			putchar('\n');
		}
		break;
	}
}

