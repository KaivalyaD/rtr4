// header files
#include <math.h>
#include "C:\freeglut\include\GL\freeglut.h"

// macros
#define PI 3.14159f
#define DEG_TO_RAD(x) (((x) * PI) / (180.0f))
#define COUNT_EMOJIS 10

// global variable declarations
int current_emoji;

// entry-point function
int main(int argc, char *argv[])
{
	// function prototypes
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
	glutCreateWindow("Kaivalya V. Deshpande: Emojis");

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	current_emoji = 1;
}

void resize(int width, int height)
{
	// code
	glViewport(0, 0, width, height);
}

void display(void)
{
	// function prototypes
	void DrawEmojiOne(void);
	void DrawEmojiTwo(void);
	void DrawEmojiThree(void);
	void DrawEmojiFour(void);
	void DrawEmojiFive(void);
	void DrawEmojiSix(void);
	void DrawEmojiSeven(void);
	void DrawEmojiEight(void);
	void DrawEmojiNine(void);
	void DrawEmojiTen(void);

	// code
	glClear(GL_COLOR_BUFFER_BIT);
	
	switch (current_emoji)
	{
	case 1:
		DrawEmojiOne();
		break;
	case 2:
		DrawEmojiTwo();
		break;
	case 3:
		DrawEmojiThree();
		break;
	case 4:
		DrawEmojiFour();
		break;
	case 5:
		DrawEmojiFive();
		break;
	case 6:
		DrawEmojiSix();
		break;
	case 7:
		DrawEmojiSeven();
		break;
	case 8:
		DrawEmojiEight();
		break;
	case 9:
		DrawEmojiNine();
		break;
	case 10:
		DrawEmojiTen();
		break;
	default:
		break;
	}

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
	case ' ':
		if (current_emoji < COUNT_EMOJIS)
			current_emoji++;
		else
			current_emoji = 1;
		
		glutPostRedisplay();
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

void DrawEmojiOne(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.03f * cosf(DEG_TO_RAD(theta));
		y = 0.06f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.2f, 0.0f);
		glVertex3f(-0.2f + x, 0.2f + y, 0.0f);

		glVertex3f(0.2f, 0.2f, 0.0f);
		glVertex3f(0.2f + x, 0.2f + y, 0.0f);
	}

	glColor3f(1.0f, 0.01f, 0.1f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.5f), 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 2.5f), 0.0f);
	}

	glEnd();
}

void DrawEmojiTwo(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + x, 0.15f + y, 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + x, 0.15f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(theta));
		y = 0.25f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(theta));
		y = 0.25f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.2f), 0.0f);
	}

	glEnd();
}

void DrawEmojiThree(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + x, 0.15f + y, 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + x, 0.15f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);
	}

	glColor3f(1.0f, 0.0f, 0.0f);
	for (theta = 160.0f; theta < 330.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta)) * cosf(DEG_TO_RAD(30.0f));
		y = -0.15f * sinf(DEG_TO_RAD(theta)) * cosf(DEG_TO_RAD(30.0f));

		glVertex3f(-0.2f, -0.26f, 0.0f);
		glVertex3f(-0.2f + x, -0.26f - y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.25f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(theta));
		y = 0.25f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.2f), 0.0f);
	}

	glEnd();
}

void DrawEmojiFour(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = -0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.2f, 0.0f);
		glVertex3f(-0.2f + x, 0.2f + y, 0.0f);

		glVertex3f(0.2f, 0.2f, 0.0f);
		glVertex3f(0.2f + x, 0.2f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = -0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.2f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.2f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.2f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.2f + (y / 1.5f), 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(theta));
		y = 0.25f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(theta));
		y = 0.25f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.2f), 0.0f);
	}

	glEnd();
}

void DrawEmojiFive(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = -0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.2f, 0.0f);
		glVertex3f(-0.2f + x, 0.2f + y, 0.0f);

		glVertex3f(0.2f, 0.2f, 0.0f);
		glVertex3f(0.2f + x, 0.2f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = -0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.2f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.2f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.2f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.2f + (y / 1.5f), 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.28f * cosf(DEG_TO_RAD(theta));
		y = -0.3f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.4f, 0.0f);
		glVertex3f(x, -0.4f - y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.28f * cosf(DEG_TO_RAD(theta));
		y = -0.3f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.4f, 0.0f);
		glVertex3f((x / 1.05f), -0.4f - (y / 1.2f), 0.0f);
	}

	glEnd();
}

void DrawEmojiSix(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.025f * cosf(DEG_TO_RAD(theta));
		y = 0.025f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + x, 0.15f + y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.05f * cosf(DEG_TO_RAD(theta));
		y = 0.05f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + x, 0.15f + y, 0.0f);
	}

	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);

	glVertex3f(-0.3f, 0.2f, 0.0f);
	glVertex3f(-0.1f, 0.2f, 0.0f);

	glEnd();

	glLineWidth(10.0f);
	glBegin(GL_LINES);

	glVertex3f(0.1f, 0.2f, 0.0f);
	glVertex3f(0.2f, 0.3f, 0.0f);

	glVertex3f(0.2f, 0.3f, 0.0f);
	glVertex3f(0.3f, 0.2f, 0.0f);

	glVertex3f(-0.2f, -0.2f, 0.0f);
	glVertex3f(0.2f, -0.2f, 0.0f);

	glEnd();
}

void DrawEmojiSeven(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.09f * cosf(DEG_TO_RAD(theta));
		y = 0.08f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, -0.2f, 0.0f);
		glVertex3f(x, -0.2f - y, 0.0f);
	}

	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);

	glVertex3f(-0.3f, 0.2f, 0.0f);
	glVertex3f(-0.2f, 0.1f, 0.0f);

	glVertex3f(-0.2f, 0.2f, 0.0f);
	glVertex3f(-0.3f, 0.1f, 0.0f);

	glVertex3f(0.2f, 0.2f, 0.0f);
	glVertex3f(0.3f, 0.1f, 0.0f);

	glVertex3f(0.3f, 0.2f, 0.0f);
	glVertex3f(0.2f, 0.1f, 0.0f);

	glEnd();
}

void DrawEmojiEight(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);

	glColor3f(0.0f, 0.0f, 0.0f);
	for (x = -0.2f; x < 0.2f; x = x + 0.1f)
	{
		glVertex3f(x, -0.2f, 0.0f);
		glVertex3f(x + 0.05f, -0.15f, 0.0f);

		glVertex3f(x + 0.05f, -0.15f, 0.0f);
		glVertex3f(x + 0.1f, -0.2f, 0.0f);
	}

	glVertex3f(-0.3f, 0.2f, 0.0f);
	glVertex3f(-0.15f, 0.15f, 0.0f);

	glVertex3f(-0.15f, 0.15f, 0.0f);
	glVertex3f(-0.3f, 0.1f, 0.0f);

	glVertex3f(0.3f, 0.2f, 0.0f);
	glVertex3f(0.15f, 0.15f, 0.0f);

	glVertex3f(0.15f, 0.15f, 0.0f);
	glVertex3f(0.3f, 0.1f, 0.0f);

	glEnd();
}

void DrawEmojiNine(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + x, 0.15f + y, 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + x, 0.15f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);
	}

	glColor3f(1.0f, 0.01f, 0.1f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.5f), 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 2.5f), 0.0f);
	}

	glEnd();
}

void DrawEmojiTen(void)
{
	// variable declarations
	float theta, x, y;

	// code
	glBegin(GL_LINES);

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 360.0f; theta = theta + 0.1f)
	{
		x = 0.5f * cosf(DEG_TO_RAD(theta));
		y = 0.5f * sinf(DEG_TO_RAD(theta));

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(x, y, 0.0f);
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + x, 0.15f + y, 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + x, 0.15f + y, 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.2f, 0.15f, 0.0f);
		glVertex3f(-0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);

		glVertex3f(0.2f, 0.15f, 0.0f);
		glVertex3f(0.2f + (x / 1.1), 0.15f + (y / 1.5f), 0.0f);
	}

	glColor3f(1.0f, 0.01f, 0.1f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f(x, -0.1 - y, 0.0f);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 1.5f), 0.0f);
	}

	glColor3f(0.9f, 0.9f, 0.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.3f * cosf(DEG_TO_RAD(1.0f * theta));
		y = 0.3f * sinf(DEG_TO_RAD(1.0f * theta));

		glVertex3f(0.0f, -0.1f, 0.0f);
		glVertex3f((x / 1.05f), -0.1 - (y / 2.5f), 0.0f);
	}

	glColor3f(0.3f, 0.8f, 1.0f);
	for (theta = 0.0f; theta < 180.0f; theta = theta + 0.1f)
	{
		x = 0.08f * cosf(DEG_TO_RAD(theta));
		y = 0.1f * sinf(DEG_TO_RAD(theta));

		glVertex3f(-0.36f, 0.25f, 0.0f);
		glVertex3f(-0.36f - x, 0.25f - y, 0.0f);
	}

	glEnd();

	glBegin(GL_TRIANGLES);
	
	glVertex3f(-0.36f, 0.45f, 0.0f);
	glVertex3f(-0.44f, 0.25f, 0.0f);
	glVertex3f(-0.28f, 0.25f, 0.0f);

	glEnd();
}
