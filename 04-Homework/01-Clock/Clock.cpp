// header files
#include <math.h>
#include "C:\freeglut\include\GL\freeglut.h"

// macros
#define PI 3.14159f
#define DEG_TO_RAD(x) ((x * PI) / (180.0f))

// entry-point function
int main(int argc, char *argv[])
{
	// local function declarations
	void initialize(void);
	void resize(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void uninitialize(void);

	// code
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Kaivalya V. Deshpande: Coloured Petzold Clock");

	initialize();

	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return 0;
}

void initialize(void)
{
	// code
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void resize(int width, int height)
{
	// code
	glViewport(0, 0, width, height);
}

void display(void)
{
	// variable declarations
	int kvd_i;

	// code
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (kvd_i = 0; kvd_i < 360; kvd_i = kvd_i + 30)
	{
		glColor3f(sinf(DEG_TO_RAD(kvd_i)), cosf(DEG_TO_RAD(kvd_i)), cosf(DEG_TO_RAD(kvd_i)) * sinf(DEG_TO_RAD(kvd_i)));
		glVertex3f(0.5f * cosf(DEG_TO_RAD(kvd_i)), 0.5f * sinf(DEG_TO_RAD(kvd_i)), 0.0f);
	}

	glEnd();

	glLineWidth(1.4f);
	glBegin(GL_LINES);
		
		// second hand -- hardcoded to 52s
		glColor3f(0.0f, 0.0f, 0.0f);

		glVertex3f(0.4f * cosf(DEG_TO_RAD(135)), 0.4f * sinf(DEG_TO_RAD(135)), 0.0f);
		glVertex3f(-0.08f * cosf(DEG_TO_RAD(135)), -0.08f * sinf(DEG_TO_RAD(135)), 0.0f);
	
	glEnd();

	glLineWidth(1.0f);
	glBegin(GL_LINES);

		// minute hand -- hardcoded to 14m
		glColor3f(0.8f, 0.5f, 0.0f);

		glVertex3f(0.45f * cosf(DEG_TO_RAD(0)), 0.0f, 0.0f);
		glVertex3f(0.0f, 0.035f, 0.0f);

		glVertex3f(0.0f, 0.035f, 0.0f);
		glVertex3f(-0.1 * cosf(DEG_TO_RAD(0)), 0.0f, 0.0f);
		
		glVertex3f(-0.1 * cosf(DEG_TO_RAD(0)), 0.0f, 0.0f);
		glVertex3f(0.0f, -0.035f, 0.0f);
		
		glVertex3f(0.0f, -0.035f, 0.0f);
		glVertex3f(0.45f * cosf(DEG_TO_RAD(0)), 0.0f, 0.0f);

		// hour hand -- hardcoded to 4h
		glColor3f(0.0f, 0.5f, 0.5f);

		glVertex3f(0.35f * cosf(DEG_TO_RAD(330)), 0.35f * sinf(DEG_TO_RAD(330)), 0.0f);
		glVertex3f(0.035f * cosf(DEG_TO_RAD(60)), 0.035f * sinf(DEG_TO_RAD(60)), 0.0f);
		
		glVertex3f(0.035f * cosf(DEG_TO_RAD(60)), 0.035f * sinf(DEG_TO_RAD(60)), 0.0f);
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(330)), -0.1f * sinf(DEG_TO_RAD(330)), 0.0f);
		
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(330)), -0.1f * sinf(DEG_TO_RAD(330)), 0.0f);
		glVertex3f(-0.035f * cosf(DEG_TO_RAD(60)), -0.035f * sinf(DEG_TO_RAD(60)), 0.0f);
		
		glVertex3f(-0.035f * cosf(DEG_TO_RAD(60)), -0.035f * sinf(DEG_TO_RAD(60)), 0.0f);
		glVertex3f(0.35f * cosf(DEG_TO_RAD(330)), 0.35f * sinf(DEG_TO_RAD(330)), 0.0f);

	glEnd();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	// code
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	// code
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

void uninitialize(void)
{
	// code
}
