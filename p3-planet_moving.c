#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846//�~�����̒ǉ�
#endif

static int year = 0, day = 0, moonDay = 0;
static float zoom = 30.0;
int samplingTime = 50.0;

static int mouseX = 0, mouseY = 0;
static float cameraX = 0.0, cameraY = 0.0;
static int isDragging = 0;

void myInit(char* progname)
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(progname);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

//���]�O���̕\���p�̌v�Z
void drawEllipse(double semiMajorAxis, double eccentricity) {
    int numSegments = 100; // �Z�O�����g�̐��𑝂₷�Ƒȉ~�����炩�ɂȂ�
    double semiMinorAxis = semiMajorAxis * sqrt(1 - eccentricity * eccentricity);
    double centerX = semiMajorAxis * eccentricity; // �ȉ~�̒��S�����炷����
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < numSegments; ++i) {
        double angle = 2.0 * M_PI * i / numSegments;
        double x = semiMajorAxis * cos(angle) - centerX;
        double z = semiMinorAxis * sin(angle);
        glVertex3d(x, 0.0, z);
    }
    glEnd();
}

void calculateOrbitPosition(double semiMajorAxis, double eccentricity, double angle, double* x, double* z) {
    double orbitRadius = semiMajorAxis * (1 - eccentricity * eccentricity) / (1 + eccentricity * cos(angle));
    *x = orbitRadius * cos(angle);
    *z = orbitRadius * sin(angle);
}


//�y���̊Ɏg���֐�
void drawRing(double innerRadius, double outerRadius, int num_segments) {
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= num_segments; i++) {
        double theta = 2.0 * M_PI * i / num_segments;
        double x = cos(theta);
        double z = sin(theta);
        glVertex3d(innerRadius * x, 0, innerRadius * z);
        glVertex3d(outerRadius * x, 0, outerRadius * z);
    }
    glEnd();
}


//�f�����̕\��
void drawAxes(double length) {
    glBegin(GL_LINES);
    // X �� - Red
    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(length, 0.0, 0.0);
    // Y �� - Green
    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, length, 0.0);
    // Z �� - Blue
    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, length);

    glColor3d(1.0, 1.0, 1.0);

    glEnd();


}

void myDisplay(void)
{

    // �f���̋O���f�[�^��z��Œ�`
    typedef struct {
        double radius;//�f���̔��a
        double yearSpeed;//���]����
        double daySpeed;//���]����
        double semiMajorAxis; // �O�������a
        double eccentricity; // �O�����S��
        double inclination; // �O���ʂ̌X�Ίp
        double axialTilt; // ���]���̌X�Ίp
    } OrbitData;

    OrbitData orbits[] = {
        //[���a*5/100000,���]����,���]����, �O�������a*5,�O�����S��,�O���X�Ίp,���]���X�Ίp,]
           {0.024397 * 5,  0.241,   58.65,     0.387 * 5,  0.2056, 7.0,   0.01}, // �����̋O���f�[�^
           {0.060518 * 5,  0.615,   -243.0187, 0.783 * 5,  0.0067, 3.39,  117.0}, // �����̋O���f�[�^
           {0.060518 * 5,  1.000,   0.997271,  1 * 5,      0.0167, 0.0,   23.4},//�n���̃f�[�^
           {0.033962 * 5,  1.881,   1.02595,   1.524 * 5,  0.0935, 1.850, 25.2},//�ΐ��̃f�[�^
           {0.71492 * 5,   11.862,  0.4135,    5.204 * 5,  0.0489, 1.304, 3.12},//�ؐ��̃f�[�^
           {0.60268 * 5,   29.457,  0.4264,    9.582 * 5,  0.0565, 2.485, 26.7},//�y���̃f�[�^
           {0.25229 * 5,   84.011,  -0.7181,   19.201 * 5, 0.0457, 0.774, 97.8},//�V�����̃f�[�^
           {0.24764 * 5,   164.79,  0.6712,    30.047 * 5, 0.0113, 1.769, 28.3},//�C�����̃f�[�^
           {0.011883 * 5,  247.74,  - 6.3872,  39.445 * 5, 0.2502, 17.089, 120}//�������̃f�[�^
           
    };
    const int numPlanets = sizeof(orbits) / sizeof(orbits[0]);

    glClear(GL_COLOR_BUFFER_BIT);

    /* ���z �i���a��1�ɐݒ�j*/
    glPushMatrix();
    glRotated(7.0, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / 27.275, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.9, 0.0, 0.0); // ��
    glutWireSphere(1.0, 20, 16);   //�i���a��1�ɐݒ�)
    drawAxes(1.2);//�f�����̒���
    glPopMatrix();


    // �e�f���̋O����`��
    for (int i = 0; i < numPlanets; ++i) {
        glPushMatrix();
        glColor3d(0.5, 0.5, 0.5);
        glRotated(orbits[i].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
        drawEllipse(orbits[i].semiMajorAxis, orbits[i].eccentricity);
        glPopMatrix();
    }


    //���]�̑����A���]�̑����A�f���̑傫���͊֐������Őݒ�
    /* ���� */
    double x0, z0;
    double angle0 = -(double)year * M_PI / 180.0 / orbits[0].yearSpeed;
    calculateOrbitPosition(orbits[0].semiMajorAxis, orbits[0].eccentricity, angle0, &x0, &z0);

    glPushMatrix();
    glRotated(orbits[0].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x0, 0.0, z0);
    glRotated(orbits[0].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[0].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.5, 0.5, 0.5);//�O���[
    glutWireSphere(orbits[0].radius, 10, 8);// (�f���̔��a / 100000 * 5)
    drawAxes(0.2);//�f�����̒���
    glPopMatrix();

    /* ���� */
    double x1, z1;
    double angle1 = -(double)year * M_PI / 180.0 / orbits[1].yearSpeed;
    calculateOrbitPosition(orbits[1].semiMajorAxis, orbits[1].eccentricity, angle1, &x1, &z1);

    glPushMatrix();
    glRotated(orbits[1].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x1, 0.0, z1);
    glRotated(orbits[1].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[1].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.8, 0.8, 0.6);//�W�����F
    glutWireSphere(orbits[1].radius, 10, 8);
    drawAxes(0.3);
    glPopMatrix();

    /* �n�� */
    double x2, z2;
    double angle2 = -(double)year * M_PI / 180.0 / orbits[2].yearSpeed;
    calculateOrbitPosition(orbits[2].semiMajorAxis, orbits[2].eccentricity, angle2, &x2, &z2);

    glPushMatrix();
    glRotated(orbits[2].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x2, 0.0, z2);
    glRotated(orbits[2].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[2].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.0, 0.0, 1.0);//��
    glutWireSphere(orbits[2].radius, 10, 8);
    drawAxes(0.6);

    /* �� */
    glPushMatrix();
    glRotated((double)moonDay / 27.32, 0.0, 1.0, 0.0);
    glTranslated(0.5, 0.0, 0.0);
    glColor3d(0.5, 0.5, 0.5);
    glutWireSphere(0.01 * 5, 10, 8);
    glPopMatrix();

    glPopMatrix();

    /* �ΐ� */
    double x3, z3;
    double angle3 = -(double)year * M_PI / 180.0 / orbits[3].yearSpeed;
    calculateOrbitPosition(orbits[3].semiMajorAxis, orbits[3].eccentricity, angle3, &x3, &z3);

    glPushMatrix();
    glRotated(orbits[3].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x3, 0.0, z3);
    glRotated(orbits[3].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[3].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.4, 0.1, 0.0);//�Â߂̐Ԓ��F
    glutWireSphere(orbits[3].radius, 10, 8);
    drawAxes(0.3);
    glPopMatrix();

    /* �ؐ� */
    double x4, z4;
    double angle4 = -(double)year * M_PI / 180.0 / orbits[4].yearSpeed;
    calculateOrbitPosition(orbits[4].semiMajorAxis, orbits[4].eccentricity, angle4, &x4, &z4);

    glPushMatrix();
    glRotated(orbits[4].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x4, 0.0, z4);
    glRotated(orbits[4].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[4].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.9, 0.6, 0.4); // �W�����F
    glutWireSphere(orbits[4].radius, 10, 8);
    drawAxes(4.0);
    glPopMatrix();

    /* �y�� */
    double x5, z5;
    double angle5 = -(double)year * M_PI / 180.0 / orbits[5].yearSpeed;
    calculateOrbitPosition(orbits[5].semiMajorAxis, orbits[5].eccentricity, angle5, &x5, &z5);

    glPushMatrix();
    glRotated(orbits[5].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x5, 0.0, z5);
    glRotated(orbits[5].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[5].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.8, 0.7, 0.5); // �W�������F
    glutWireSphere(orbits[5].radius, 10, 8);
    // �y���̎���Ƀ����O��`��
    glColor3d(0.8, 0.8, 0.7); // �W�����F
    drawRing(orbits[5].radius * 1.2, orbits[5].radius * 2, 100);  // �����O�̓����ƊO���̔��a��ݒ�

    drawAxes(5.0);
    glPopMatrix();

    
    glPopMatrix();

    /* �V���� */
    double x6, z6;
    double angle6 = -(double)year * M_PI / 180.0 / orbits[6].yearSpeed;
    calculateOrbitPosition(orbits[6].semiMajorAxis, orbits[6].eccentricity, angle6, &x6, &z6);

    glPushMatrix();
    glRotated(orbits[6].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x6, 0.0, z6);
    glRotated(orbits[6].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[6].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.0, 1.0, 1.0); // �W���ΐF
    glutWireSphere(orbits[6].radius, 10, 8);
    drawAxes(3.0);
    glPopMatrix();

    /* �C���� */
    double x7, z7;
    double angle7 = -(double)year * M_PI / 180.0 / orbits[7].yearSpeed;
    calculateOrbitPosition(orbits[7].semiMajorAxis, orbits[7].eccentricity, angle7, &x7, &z7);

    glPushMatrix();
    glRotated(orbits[7].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x7, 0.0, z7);
    glRotated(orbits[7].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[7].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.0, 0.0, 0.8); // �Z����
    glutWireSphere(orbits[7].radius, 10, 8);
    drawAxes(3.0);
    glPopMatrix();

    /* ������ */
    double x8, z8;
    double angle8 = -(double)year * M_PI / 180.0 / orbits[8].yearSpeed;
    calculateOrbitPosition(orbits[8].semiMajorAxis, orbits[8].eccentricity, angle8, &x8, &z8);

    glPushMatrix();
    glRotated(orbits[8].inclination, 1.0, 0.0, 0.0); // �O���ʂ��X����
    glTranslated(x8, 0.0, z8);
    glRotated(orbits[8].axialTilt, 0.0, 0.0, 1.0); // ���]���̌X�΂�K�p
    glRotated((double)day / orbits[8].daySpeed, 0.0, 1.0, 0.0); // ���]
    glColor3d(0.7, 0.6, 0.5); // �W�����F
    glutWireSphere(orbits[7].radius, 10, 8);
    drawAxes(3.0);
    glPopMatrix();

    glutSwapBuffers();
}

void myReshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraX, cameraY, zoom, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void myKeyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'd':
        day = (day + 10);
        glutPostRedisplay();
        break;
    case 'D':
        day = (day - 10);
        glutPostRedisplay();
        break;
    case 'y':
        year = (year + 5);
        glutPostRedisplay();
        break;
    case 'Y':
        year = (year - 5);
        glutPostRedisplay();
        break;
    case 27:
        exit(0);
        break;
    default:
        break;
    }
}

void myMouse(int button, int state, int x, int y)
{
    // �}�E�X�z�C�[���ɂ��Y�[���C���E�Y�[���A�E�g
    if (button == 3) // �X�N���[���A�b�v
    {
        zoom -= 1.0;
    }
    else if (button == 4) // �X�N���[���_�E��
    {
        zoom += 1.0;
    }

    // �Y�[���͈͂𐧌�
    if (zoom < 2.0) zoom = 2.0;
    if (zoom > 180.0) zoom = 180.0;

    // ���܂��͉E�̃}�E�X�{�^���������ꂽ���̃h���b�O����
    if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = 1;
            mouseX = x;
            mouseY = y;
        }
        else if (state == GLUT_UP) {
            isDragging = 0;
        }
    }

    // �E�B���h�E�̃��T�C�Y�ƍĕ`����g���K�[
    myReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glutPostRedisplay();
}

void myMotion(int x, int y)
{
    // �h���b�O���̃J�����̈ړ�
    if (isDragging) {
        cameraX += (x - mouseX) * 0.05; // �}�E�X��X�����̈ړ��Ɋ�Â��ăJ���������Ɉړ�
        cameraY -= (y - mouseY) * 0.05; // �}�E�X��Y�����̈ړ��Ɋ�Â��ăJ�������㉺�Ɉړ�

        // ���݂̃}�E�X�ʒu���X�V
        mouseX = x;
        mouseY = y;

        // �E�B���h�E�̃��T�C�Y�ƍĕ`����g���K�[
        myReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        glutPostRedisplay();
    }
}

void myTime(int value)
{
    if (value == 1)
    {
        glutTimerFunc(samplingTime, myTime, 1);
        year = (year + 1);
        day = (day + 5);
        moonDay = (moonDay + 1); // ���̌��]�Ǝ��]��i�߂�
        glutPostRedisplay();
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    myInit(argv[0]);
    glutKeyboardFunc(myKeyboard);
    glutMouseFunc(myMouse);  //�}�E�X�z�C�[���̏����ƃY�[���C���@�\�̒ǉ�
    glutMotionFunc(myMotion);//�J�����̎��_����̒ǉ�
    glutTimerFunc(samplingTime, myTime, 1);//�����Ŏ��]�ƌ��]������@�\�̒ǉ�
    glutReshapeFunc(myReshape);
    glutDisplayFunc(myDisplay);
    glutMainLoop();
    return 0;
}
