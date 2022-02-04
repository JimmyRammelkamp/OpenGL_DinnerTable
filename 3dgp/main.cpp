#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D models
C3dglModel camera;
C3dglModel vase;
C3dglModel table;
C3dglModel livingRoom;
C3dglModel apple;
C3dglModel lamp;

// Textures
GLuint idTexWood;
GLuint idTexCloth;
GLuint idTexNone;

float vertices[] = {
			-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,
			-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,
			-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 };
float normals[] = {
	0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
	-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
unsigned indices[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };


// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

//GLSL Program
C3dglProgram Program;

// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15;		// Tilt Angle
float angleRot = 0.1f;		// Camera orbiting angle
vec3 cam(0);				// Camera movement values

//controls
bool lamp1, lamp2 = false;

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	//// setup lighting
	//glEnable(GL_LIGHTING);									// --- DEPRECATED
	//glEnable(GL_LIGHT0);									// --- DEPRECATED

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert.shader")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag.shader")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!apple.load("models\\Apple.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!livingRoom.load("models\\LivingRoomObj\\LivingRoom.obj")) return false;
	livingRoom.loadMaterials("models\\LivingRoomObj\\");
	

	// Load Textures
	C3dglBitmap bm;

	bm.Load("models/oak.bmp", GL_RGBA);
	if (!bm.GetBits()) return false;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	bm.Load("models/cloth.bmp", GL_RGBA);
	if (!bm.GetBits()) return false;
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexCloth);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);


	// Send the texture info to the shaders
	Program.SendUniform("texture0", 0);
	//Program.SendUniform("textureNormal", 1);
	//Program.SendUniform("textureAO", 2);



	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(0.0, 5.0, 10.0), //position
		vec3(0.0, 5.0, 0.0),  //point its looking at
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift+AD or arrow key to auto-orbit" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void renderScene(mat4 matrixView, float time)
{
	mat4 m;

	
	Program.SendUniform("lightAmbient1.color", 0.1, 0.1, 0.1);
	Program.SendUniform("lightAmbient2.color", 0, 0, 0);

	Program.SendUniform("materialAmbient", 0.1, 0.1, 0.1);

	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 0.2, 0.2, 0.2);


	Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f); 
	
	//pointlight/lightbulb 1
	Program.SendUniform("lightPoint1.position", -2.35f, 3.9f, -1.12f);
	if (lamp1)
	{
		Program.SendUniform("lightPoint1.diffuse", 0.5, 0.2, 0.2);
		Program.SendUniform("lightPoint1.specular", 1.0, 0.0, 0.0);
	}
	else
	{
		Program.SendUniform("lightPoint1.diffuse", 0.0, 0.0, 0.0);
		Program.SendUniform("lightPoint1.specular", 0.0, 0.0, 0.0);
	}
	

	//pointlight/lightbulb 2
	Program.SendUniform("lightPoint2.position", 1.95f, 3.9f, 1.36f);
	if (lamp2)
	{
		Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5);
		Program.SendUniform("lightPoint2.specular", 0.4, 0.4, 0.4);
	}
	else
	{
		Program.SendUniform("lightPoint2.diffuse", 0.0, 0.0, 0.0);
		Program.SendUniform("lightPoint2.specular", 0.0, 0.0, 0.0);
	}

	Program.SendUniform("materialSpecular", 0.5, 0.5, 0.5);
	Program.SendUniform("shininess", 1.0);
	
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// camera
	m = matrixView;
	m = translate(m, vec3(0.0f, 5.0f, 1.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	//camera.render(m);

	// vase
	Program.SendUniform("materialDiffuse", 0.1f, 0.0f, 2.0f);
	Program.SendUniform("materialSpecular", 1.0, 1.0, 1.0);
	Program.SendUniform("shininess", 10.0);
	m = matrixView;
	m = translate(m, vec3(0.0f, 3.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.08f, 0.08f, 0.08f));
	vase.render(m);

	//Program.SendUniform("material", 0.6f, 0.2f, 0.1f); //Brown

	//table
	Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("materialSpecular", 0.5, 0.5, 0.5);
	Program.SendUniform("shininess", 1.0);
	glBindTexture(GL_TEXTURE_2D, idTexWood);

	m = matrixView;
	m = translate(m, vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f)); 
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));  //0.004 used by jarek maybe change scale and don't put shininess on table
	table.render(1,m);
	
	glBindTexture(GL_TEXTURE_2D, idTexCloth);

	//Chairs
	m = matrixView;
	m = translate(m, vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	table.render(0,m);

	m = matrixView;
	m = translate(m, vec3(1.0f, .0f, 0.0f));
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	table.render(0,m);

	m = matrixView;
	m = translate(m, vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	table.render(0,m);

	m = matrixView;
	m = translate(m, vec3(-1.0f, 0.0f, 0.0f));
	m = rotate(m, radians(270.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	table.render(0, m);


	


	//Living Room
	m = matrixView;
	m = translate(m, vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(.03f, .03f, .03f));
	livingRoom.render(m);

	glBindTexture(GL_TEXTURE_2D, idTexNone);

	//Lamp1
	Program.SendUniform("materialDiffuse", 1.0f, 0.95f, 0.0f);
	Program.SendUniform("materialSpecular", 1.0f, 0.95f, 0.0f);
	m = matrixView;
	m = translate(m, vec3(-1.9f, 3.05f,-0.8f));
	m = rotate(m, radians(-36.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	lamp.render(m);

	//Lamp2
	Program.SendUniform("materialDiffuse", 1.0f, 0.95f, 0.0f);
	Program.SendUniform("materialSpecular", 1.0f, 0.95f, 0.0f);

	m = matrixView;
	m = translate(m, vec3(2.0f, 3.05f, 0.8f));
	m = rotate(m, radians(85.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.015f, 0.015f, 0.015f));
	lamp.render(m);

	//Apple
	Program.SendUniform("materialDiffuse", 2.0f, 0.1f, 0.0f);
	Program.SendUniform("materialSpecular", 1.0f, 1.0f,1.0f);
	Program.SendUniform("shininess", 10.0);
	m = matrixView;
	m = translate(m, vec3(-1.5f, 4.0f, 0.0f));
	m = rotate(m, radians(time*100), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.01f, 0.01f, 0.01f));
	apple.render(m);

	// Pyramid
	Program.SendUniform("materialDiffuse", 1.0f, 0.1f, 0.1f);
	Program.SendUniform("materialSpecular", 0.1f, 0.1f, 0.1f);
	Program.SendUniform("shininess", 1.0);

	// Pendulum mechanics
	static float prevT = 0;		// previous timestamp
	static float alpha = 0;		// current pendulum angle
	static float delta = 0.7f;		// angle increase rate (Hooke's law)
	delta -= alpha * (time - prevT) * 0.04;
	alpha += delta;
	prevT = time;


	m = matrixView;
	m = translate(m, vec3(-1.5f, 3.5f, 0.0f));
	m = rotate(m, radians(time*100), vec3(0.0f, 1.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(1.0f, 0.0f, 0.0f));
	m = scale(m, vec3(.065f, .065f, .065f));

	Program.SendUniform("matrixModelView", m);

	// Get Attribute Locations
	GLuint attribVertex = Program.GetAttribLocation("aVertex");
	GLuint attribNormal = Program.GetAttribLocation("aNormal");

	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);

	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw triangles – using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);

	// teapot
	Program.SendUniform("materialDiffuse", 0.0f, 2.0f, 0.0f);
	Program.SendUniform("materialSpecular", 1, 1, 1);
	Program.SendUniform("shininess", 20.0);
	
	m = matrixView;
	m = translate(m, vec3(1.5f, 3.3f, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(.2f, .2f, .2f));
	Program.SendUniform("matrixModelView", m);
	glutSolidTeapot(2.0);

	

	//Bulb
	Program.SendUniform("materialDiffuse", 1.0f, 0.1f, 0.1f);
	Program.SendUniform("shininess", 1.0);
	if(lamp1) Program.SendUniform("lightAmbient2.color", 2.0, 0.2, 0.2);
	else Program.SendUniform("lightAmbient2.color", 0, 0, 0);
	
	m = matrixView;
	m = translate(m, vec3(-2.35f, 3.9f, -1.12f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	//Bulb2
	Program.SendUniform("materialDiffuse", 1.0f, 1.0f, 1.0f);
	Program.SendUniform("shininess", 1.0);
	if (lamp2) Program.SendUniform("lightAmbient2.color", 1.5, 1.5, 1.5);
	else Program.SendUniform("lightAmbient2.color", 0, 0, 0);
	
	m = matrixView;
	m = translate(m, vec3(1.95f, 3.9f, 1.36f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	
	
}

void onRender()
{
	// this global variable controls the animation
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
	matrixView = m;

	// setup View Matrix
	Program.SendUniform("matrixView", matrixView);



	// render the scene objects
	renderScene(matrixView, time);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(60.f), ratio, 0.02f, 1000.f);

	//// Setup the Projection Matrix
	//glMatrixMode(GL_PROJECTION);							// --- DEPRECATED
	//glLoadIdentity();										// --- DEPRECATED
	//glMultMatrixf((GLfloat*)&matrixProjection);				// --- DEPRECATED

	// Setup the Projection Matrix
	Program.SendUniform("matrixProjection", matrixProjection);

}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': cam.z = std::max(cam.z * 1.05f, 0.01f); break;
	case 's': cam.z = std::min(cam.z * 1.05f, -0.01f); break;
	case 'a': cam.x = std::max(cam.x * 1.05f, 0.01f); angleRot = 0.1f; break;
	case 'd': cam.x = std::min(cam.x * 1.05f, -0.01f); angleRot = -0.1f; break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1': lamp1 = !lamp1; break;
	case '2': lamp2 = !lamp2; break;
	}
	// speed limit
	cam.x = std::max(-1.15f, std::min(1.15f, cam.x));
	cam.y = std::max(-1.15f, std::min(1.15f, cam.y));
	cam.z = std::max(-1.15f, std::min(1.15f, cam.z));
	// stop orbiting
	if ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) == 0) angleRot = 0;
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': cam.z = 0; break;
	case 'a':
	case 'd': cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

