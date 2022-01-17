// header files
#include <math.h>
#include "C:\freeglut\include\GL\freeglut.h"

// macros
#define PI 3.14159f
#define DEG_TO_RAD(x) (((x) * PI) / (180.0f))
#define INITIAL_ANGLE_SECOND_HAND 30
#define INITIAL_ANGLE_MINUTE_HAND 0.0f
#define INITIAL_ANGLE_HOUR_HAND 330.0f

// global variable declarations
int second_hand_angle;
float minute_hand_angle, hour_hand_angle;

// entry-point function
int main(int argc, char *argv[])
{
	// local function declarations
	void initialize(void);
	void resize(int, int);
	void display(void);
	void TimerFunction(int);
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
	glutTimerFunc(1000, TimerFunction, 1);
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

	second_hand_angle = INITIAL_ANGLE_SECOND_HAND;
	minute_hand_angle = INITIAL_ANGLE_MINUTE_HAND;
	hour_hand_angle = INITIAL_ANGLE_HOUR_HAND;
}

void resize(int width, int height)
{
	// code
	glViewport(0, 0, width, height);
}

void TimerFunction(int value)
{
	// code
	second_hand_angle = (second_hand_angle - 5);
	if (second_hand_angle < -(360 - INITIAL_ANGLE_SECOND_HAND - 5))
		second_hand_angle = INITIAL_ANGLE_SECOND_HAND;

	minute_hand_angle = minute_hand_angle - (5.0f / 60.0f);
	if (minute_hand_angle < -(360.0f - INITIAL_ANGLE_MINUTE_HAND - 0.1f))
		minute_hand_angle = INITIAL_ANGLE_MINUTE_HAND;
	
	hour_hand_angle = hour_hand_angle - (5.0f / (60.0f * 12.0f));
	if (hour_hand_angle < -(360.0f - INITIAL_ANGLE_MINUTE_HAND - 0.1f))
		hour_hand_angle = INITIAL_ANGLE_MINUTE_HAND;

	glutPostRedisplay();
	glutTimerFunc(1000, TimerFunction, 1);
}

void display(void)
{
	// variable declarations
	int kvd_i;
	float kvd_f;

	// code
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
		
		// points separated by 30 degrees (1 hour/5 minutes)
		for (kvd_i = 0; kvd_i < 360; kvd_i = kvd_i + 30)
		{
			glColor3f(sinf(DEG_TO_RAD(kvd_i)), cosf(DEG_TO_RAD(kvd_i)), cosf(DEG_TO_RAD(kvd_i)) * sinf(DEG_TO_RAD(kvd_i)));
			glVertex3f(0.5f * cosf(DEG_TO_RAD(kvd_i)), 0.5f * sinf(DEG_TO_RAD(kvd_i)), 0.0f);
		}

	glEnd();
	
	glPointSize(2.0f);
	glBegin(GL_POINTS);
		
		// points separated by 5 degrees (1 second)
		for (kvd_i = 0; kvd_i < 360; kvd_i = kvd_i + 5)
		{
			if (kvd_i % 30 != 0)
			{
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.5f * cosf(DEG_TO_RAD(kvd_i)), 0.5f * sinf(DEG_TO_RAD(kvd_i)), 0.0f);
			}
		}

	glEnd();

	glLineWidth(1.4f);
	glBegin(GL_LINES);

		// second hand
		glColor3f(0.0f, 0.0f, 0.0f);

		glVertex3f(0.4f * cosf(DEG_TO_RAD(second_hand_angle)), 0.4f * sinf(DEG_TO_RAD(second_hand_angle)), 0.0f);
		glVertex3f(-0.08f * cosf(DEG_TO_RAD(second_hand_angle)), -0.08f * sinf(DEG_TO_RAD(second_hand_angle)), 0.0f);
	
	glEnd();

	glLineWidth(1.0f);
	glBegin(GL_LINES);

		// minute hand
		glColor3f(0.8f, 0.5f, 0.0f);

		glVertex3f(0.45f * cosf(DEG_TO_RAD(minute_hand_angle)), 0.45f * sinf(DEG_TO_RAD(minute_hand_angle)), 0.0f);
		glVertex3f(0.045 * cosf(DEG_TO_RAD(minute_hand_angle + 45.0f)), 0.045 * sinf(DEG_TO_RAD(minute_hand_angle + 45.0f)), 0.0f);

		glVertex3f(0.045 * cosf(DEG_TO_RAD(minute_hand_angle + 45.0f)), 0.045 * sinf(DEG_TO_RAD(minute_hand_angle + 45.0f)), 0.0f);
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(minute_hand_angle)), -0.1f * sinf(DEG_TO_RAD(minute_hand_angle)), 0.0f);

		glVertex3f(-0.1f * cosf(DEG_TO_RAD(minute_hand_angle)), -0.1f * sinf(DEG_TO_RAD(minute_hand_angle)), 0.0f);
		glVertex3f(0.045 * cosf(DEG_TO_RAD(minute_hand_angle - 45.0f)), 0.045 * sinf(DEG_TO_RAD(minute_hand_angle - 45.0f)), 0.0f);
		
		glVertex3f(0.045 * cosf(DEG_TO_RAD(minute_hand_angle - 45.0f)), 0.045 * sinf(DEG_TO_RAD(minute_hand_angle - 45.0f)), 0.0f);
		glVertex3f(0.45f * cosf(DEG_TO_RAD(minute_hand_angle)), 0.45f * sinf(DEG_TO_RAD(minute_hand_angle)), 0.0f);

		// hour hand
		glColor3f(0.0f, 0.5f, 0.5f);

		glVertex3f(0.35f * cosf(DEG_TO_RAD(hour_hand_angle)), 0.35f * sinf(DEG_TO_RAD(hour_hand_angle)), 0.0f);
		glVertex3f(0.035 * cosf(DEG_TO_RAD(hour_hand_angle + 45.0f)), 0.035 * sinf(DEG_TO_RAD(hour_hand_angle + 45.0f)), 0.0f);

		glVertex3f(0.035 * cosf(DEG_TO_RAD(hour_hand_angle + 45.0f)), 0.035 * sinf(DEG_TO_RAD(hour_hand_angle + 45.0f)), 0.0f);
		glVertex3f(-0.1f * cosf(DEG_TO_RAD(hour_hand_angle)), -0.1f * sinf(DEG_TO_RAD(hour_hand_angle)), 0.0f);

		glVertex3f(-0.1f * cosf(DEG_TO_RAD(hour_hand_angle)), -0.1f * sinf(DEG_TO_RAD(hour_hand_angle)), 0.0f);
		glVertex3f(0.035 * cosf(DEG_TO_RAD(hour_hand_angle - 45.0f)), 0.035 * sinf(DEG_TO_RAD(hour_hand_angle - 45.0f)), 0.0f);

		glVertex3f(0.035 * cosf(DEG_TO_RAD(hour_hand_angle - 45.0f)), 0.035 * sinf(DEG_TO_RAD(hour_hand_angle - 45.0f)), 0.0f);
		glVertex3f(0.35f * cosf(DEG_TO_RAD(hour_hand_angle)), 0.35f * sinf(DEG_TO_RAD(hour_hand_angle)), 0.0f);

	glEnd();

	glColor3f(0.75, 0.50, 0.0f);
	glBegin(GL_LINES);
		
		// circle
		for (kvd_f = 0.0f; kvd_f < 360.0f; kvd_f = kvd_f + 0.01f)
		{
			glVertex3f(0.53f * cosf(DEG_TO_RAD(kvd_f)), 0.53f * sinf(DEG_TO_RAD(kvd_f)), 0.0f);
			glVertex3f(0.56f * cosf(DEG_TO_RAD(kvd_f)), 0.56f * sinf(DEG_TO_RAD(kvd_f)), 0.0f);
		}

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
