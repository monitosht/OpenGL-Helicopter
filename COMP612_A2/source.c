/******************************************************************************
 *
 * Computer Graphics Programming 2020 Project Template v1.0 (11/04/2021)
 *
 * Based on: Animation Controller v1.0 (11/04/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene,
 * plus keyboard handling for smooth game-like control of an object such as a32
 * character or vehicle.
 *
 * A simple static lighting setup is provided via initLights(), which is not
 * included in the animationalcontrol.c template. There are no other changes.
 *
 *******************************************************************************
 * 
 * Student Name: Monitosh Thaker
 * Student ID:   17000777
 * 
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <stdio.h>
#include <math.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

// Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				

// Maximum number of particles for the rain particle system
#define MAX_PARTICLES 1000

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Some Simple Definitions of Motion
 ******************************************************************************/

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.

 // Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
 // 
 // You can use any numeric values, as specified in the comments for each axis. However,
 // the MOTION_ definitions offer an easy way to define a "unit" movement without using
 // magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
 //
typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
} keystate_t;

// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;

// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_MOVE_FORWARD	'w'
#define KEY_MOVE_BACKWARD	's'
#define KEY_MOVE_LEFT		'a'
#define KEY_MOVE_RIGHT		'd'
#define KEY_RENDER_FILL		'l'

#define ROT_ON			    'r' 

#define KEY_EXIT			27 // Escape key.

// Define all GLUT special keys used for input (add any new key definitions here).

#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);

void drawOrigin(void);

void drawGround(void);
void drawLake(void);
void drawSky(void);
void drawHeli(void);

void drawTail(void);
void drawTailRotor(void);
void drawLegs(void);
void drawConnection(void);
void drawBlades(void);

void drawHelipad(void);
void drawBuilding(void);

void drawFog(void);

void drawBoat(void);
void updateBoat(void);

void createTreeDisplayList(void);
void drawTree(void);

void drawTreeGroup(GLfloat x, GLfloat y, GLfloat z);
void drawForest(GLfloat x, GLfloat y, GLfloat z);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

int renderFillEnabled = 1;

GLint windowWidth = 1000;
GLint windowHeight = 800;

typedef struct {
	float x;
	float y;
	float z;
}position3D;

GLfloat cameraPosition[] = { 0, 4, 15 };
float camX;
float camY;
float camZ;

GLUquadricObj* skyObj;

position3D heliPos;
float heliHeading = 0.0;
float bodyRadius = 0.2;
float speed = 3.0;

GLUquadricObj* heliBody;
GLUquadricObj* heliTail;
GLUquadricObj* heliTailRotor[2];
GLUquadricObj* tailConnection;
float tailLength;

GLUquadricObj* heliBlades[3];
GLUquadricObj* bladeTop;
GLUquadricObj* rotorConnection;

GLUquadricObj* legs[6];

GLUquadricObj* boat;
position3D boatPos;
int changeBoatDir = 0;
float boatSpeed = 0.025;

int rotStarted = 1;
float rotAngle = 0;
float rotSpeed = 15;

void loadTextures(void);
int texID[2];
char* textureFileNames[2] = 
{
	"textures/grass.ppm",
	"textures/water.ppm"
};

GLuint treeDL;

GLfloat fogColour[4] = { 0.20, 0.35, 0.35, 1 };

/******************************************************************************
 * Particle System Function Prototypes and Variables
 ******************************************************************************/

void createParticle(void);
void spawnParticles(void);

void drawParticles(void);
void updateRain(int i);

typedef struct {
	position3D position;
	float size;
	float alpha;
	float dx;
	float dy;
	int active;
}Particle_t;

Particle_t particle;
Particle_t particle_system[MAX_PARTICLES];

int numParticles;

/******************************************************************************
 * Entry Point (don't put anything except the main function here)
 ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	GLfloat angle = heliHeading - 180;

	camX = heliPos.x - 3 * sin(angle * (3.14 / 180));
	camY = heliPos.y + 0.85;
	camZ = heliPos.z - 3 * cos(angle * (3.14 / 180));
	
	gluLookAt(camX,      camY,      camZ,
			  heliPos.x, heliPos.y, heliPos.z,
			  0,         1,         0);	

	// Allow us to view the scene in a wireframe mode
	renderFillEnabled ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	drawSky();
	drawGround();
	drawLake();

	drawHelipad();
	drawBuilding();
	drawBoat();

	drawForest(5, 0, -20);

	drawHeli();

	drawParticles();
	drawFog();

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
	windowWidth = width;
	windowHeight = h;

	glViewport(0, 0, windowWidth, windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, (float)windowWidth / (float)windowHeight, 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is pressed, we do two things:
			(1) Update motionKeyStates to record that the key is held down. We use
				this later in the keyReleased callback.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. The most recent key always "wins" (e.g. if
				you're holding down KEY_MOVE_LEFT then also pressed KEY_MOVE_RIGHT,
				our object will immediately start moving right).
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_FORWARD;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_BACKWARD;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_LEFT;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_RIGHT;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Rather than using literals (e.g. "t" for spotlight), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
			For example, refer to the existing keys used here (KEY_MOVE_FORWARD,
			KEY_MOVE_LEFT, KEY_EXIT, etc).
		*/
	case KEY_RENDER_FILL:
		renderFillEnabled = !renderFillEnabled;
		break;
	case ROT_ON:
		rotStarted = !rotStarted;
		break;	
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyPressed.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_UP;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_DOWN;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_CLOCKWISE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			Rather than directly using the GLUT constants (e.g. GLUT_KEY_F1), create
			a new SP_KEY_ definition in the "Keyboard Input Handling Setup" section of
			this file. For example, refer to the existing keys used here (SP_KEY_MOVE_UP,
			SP_KEY_TURN_LEFT, etc).
		*/
	}
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is released.
*/
void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is released, we do two things:
			(1) Update motionKeyStates to record that the key is no longer held down;
				we need to know when we get to step (2) below.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. This gets a little complicated to ensure
				the controls work smoothly. When the user releases a key that moves
				in one direction (e.g. KEY_MOVE_RIGHT), we check if its "opposite"
				key (e.g. KEY_MOVE_LEFT) is pressed down. If it is, we begin moving
				in that direction instead. Otherwise, we just stop moving.
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveBackward == KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ? MOTION_FORWARD : MOTION_NONE;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ? MOTION_RIGHT : MOTION_NONE;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ? MOTION_LEFT : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Note: If you only care when your key is first pressed down, you don't have to
			add anything here. You only need to put something in keyReleased if you care
			what happens when the user lets go, like we do with our movement keys above.
			For example: if you wanted a spotlight to come on while you held down "t", you
			would need to set a flag to turn the spotlight on in keyPressed, and update the
			flag to turn it off in keyReleased.
		*/
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ? MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ? MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ? MOTION_CLOCKWISE : MOTION_NONE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ? MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			As per keyReleased, you only need to handle the key here if you want something
			to happen when the user lets go. If you just want something to happen when the
			key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_FOG);

	glClearColor(0.35, 0.45, 0.9, 1.0);

	srand((unsigned)time(0));

	loadTextures();
	createTreeDisplayList();

	initLights();

	heliPos.x = 0.0;
	heliPos.y = 1.0;
	heliPos.z = 5.0;

	particle.active = 0;
	numParticles    = 0;

	skyObj           = gluNewQuadric();

	heliBody         = gluNewQuadric();
	heliTail         = gluNewQuadric();
	heliTailRotor[0] = gluNewQuadric();
	heliTailRotor[1] = gluNewQuadric();
	tailConnection   = gluNewQuadric();

	heliBlades[0]    = gluNewQuadric();
	heliBlades[1]    = gluNewQuadric();
	heliBlades[2]    = gluNewQuadric();
	bladeTop         = gluNewQuadric();
	rotorConnection  = gluNewQuadric();

	for (int i = 0; i < 6; i++)
	{
		legs[i]      = gluNewQuadric();
	}

	boat = gluNewQuadric();

	boatPos.x = 0;
	boatPos.y = 0;
	boatPos.z = 0;
}

/*
	Initialise OpenGL lighting before we begin the render loop.

	Note (advanced): If you're using dynamic lighting (e.g. lights that move around, turn on or
	off, or change colour) you may want to replace this with a drawLights function that gets called
	at the beginning of display() instead of init().
*/
void initLights(void)
{
	GLfloat globalAmbient[] = { 0.2f, 0.2f, 0.2f, 1 };

	GLfloat lightPosition[] = { 5.0, 15.0, 2.0, 0.0 };

	GLfloat ambientLight[]  = { 0.2, 0.2, 0.2, 1 };
	GLfloat diffuseLight[]  = { 0.3, 0.3, 0.3, 1 };
	GLfloat spotlightDiffuse[] = { 1, 1, 1, 1 };
	GLfloat specularLight[] = { 1, 1, 1, 1 };

	GLfloat spotlightDirection[] = { 0.0, -1.0, 0.0 };
	GLfloat exponentLight = 3.0;
	GLfloat theta = 40.0;

	// Configure global ambient lighting.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	// Configure Light 0 (directional light)
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

	// Configure Light 1 (spot light)
	glLightfv(GL_LIGHT1, GL_POSITION,       lightPosition);
	glLightfv(GL_LIGHT1, GL_AMBIENT,        ambientLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,        spotlightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR,       specularLight);

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotlightDirection);
	glLightf( GL_LIGHT1, GL_SPOT_EXPONENT,  exponentLight);
	glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,    theta);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);

	glShadeModel(GL_SMOOTH);
}

void loadTextures(void)
{
	glGenTextures(2, texID);

	for (int j = 0; j < 2; j++)
	{
		GLubyte* imageData;
		int imageWidth, imageHeight;

		FILE* fileID;

		int maxValue;
		int totalPixels;

		char tempChar;
		char headerLine[100];

		int i;

		float RGBscaling;
		int red, blue, green;

		fileID = fopen(textureFileNames[j], "r");

		fscanf(fileID, "%[^\n] ", headerLine);

		if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
		{
			printf("Error, this is not a PPM file.");
			exit(0);
		}

		fscanf(fileID, "%c", &tempChar);

		while (tempChar == '#')
		{
			fscanf(fileID, "%[^\n] ", headerLine);
			fscanf(fileID, "%c", &tempChar);
		}

		ungetc(tempChar, fileID);

		fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

		totalPixels = imageWidth * imageHeight;

		imageData = malloc(3 * sizeof(GLuint) * totalPixels);

		RGBscaling = 255.0 / maxValue;

		if (maxValue == 255)
		{
			for (i = 0; i < totalPixels; i++)
			{
				fscanf(fileID, "%d %d %d", &red, &green, &blue);

				imageData[3 * totalPixels - 3 * i - 3] = red;
				imageData[3 * totalPixels - 3 * i - 2] = green;
				imageData[3 * totalPixels - 3 * i - 1] = blue;
			}
		}
		else
		{
			for (i = 0; i < totalPixels; i++)
			{
				fscanf(fileID, "%d %d %d", &red, &green, &blue);

				imageData[3 * totalPixels - 3 * i - 3] = red * RGBscaling;
				imageData[3 * totalPixels - 3 * i - 2] = green * RGBscaling;
				imageData[3 * totalPixels - 3 * i - 1] = blue * RGBscaling;
			}
		}

		fclose(fileID);

		if (imageData)
		{
			glBindTexture(GL_TEXTURE_2D, texID[j]);

			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, imageData);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			printf("Failed to get texture data from %s\n", textureFileNames[j]);
		}
	}
}

void createTreeDisplayList(void)
{
	treeDL = glGenLists(1);

	glNewList(treeDL, GL_COMPILE);
	drawTree();
	glEndList();
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	/*
		Keyboard motion handler: complete this section to make your "player-controlled"
		object respond to keyboard input.
	*/
	if (keyboardMotion.Yaw != MOTION_NONE) 
	{
		heliHeading += keyboardMotion.Yaw * (speed * 10) * FRAME_TIME_SEC;
	}
	if (keyboardMotion.Surge != MOTION_NONE) 
	{
		heliPos.z -= keyboardMotion.Surge * (speed * cos(heliHeading * 3.14 / 180)) * FRAME_TIME_SEC;
		heliPos.x -= keyboardMotion.Surge * (speed * sin(heliHeading * 3.14 / 180)) * FRAME_TIME_SEC;
	}
	if (keyboardMotion.Sway != MOTION_NONE) 
	{
		float angle = heliHeading + 90.0;
		heliPos.z += keyboardMotion.Sway * (speed * cos(angle * 3.14 / 180)) * FRAME_TIME_SEC;
		heliPos.x += keyboardMotion.Sway * (speed * sin(angle * 3.14 / 180)) * FRAME_TIME_SEC;
	}
	if (keyboardMotion.Heave != MOTION_NONE) 
	{
		heliPos.y += keyboardMotion.Heave * speed * FRAME_TIME_SEC;
	}

	// Limit the helicopters movement to stay within the bounds of the scene

	if (heliPos.y <= 0.15)
		heliPos.y  = 0.15;
	if (heliPos.y >= 20.0)
		heliPos.y  = 20.0;
	
	if (heliPos.x <= -20.0)
		heliPos.x = -20.0;
	if (heliPos.x  >= 20.0)
		heliPos.x   = 20.0;

	if (heliPos.z <= -20.0)
		heliPos.z  = -20.0;
	if (heliPos.z  >= 20.0)
		heliPos.z   = 20.0;

	// Animate rotors

	if (rotStarted)
	{
		rotAngle += rotSpeed;			
	}

	// Particle system animation logic

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		updateRain(i);
	}

	if (numParticles < MAX_PARTICLES) 
	{
		if (numParticles < MAX_PARTICLES)
		{
			spawnParticles();
		}
	}

	// Boat animation logic

	updateBoat();

	// Light movement logic

	GLfloat spotlightPosition[] = { heliPos.x, heliPos.y - bodyRadius, heliPos.z, 1.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, spotlightPosition);
} 

/*
	Draw basic scene elements
*/
void drawSky(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	GLfloat ambient[]  = { 0.5, 0.85, 1.0, 1.0 };
	GLfloat diffuse[]  = { 0.5, 0.85, 1.0, 1.0 };
	GLfloat specular[] = { 1, 1, 1, 1 };
	GLfloat shininess  = 100.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specular);
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glTranslated(0, 0, 0);

	gluSphere(skyObj, 25, 100, 100);;

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawGround(void)
{
	glPushMatrix(); 

	glBindTexture(GL_TEXTURE_2D, texID[0]);

	GLfloat size = 2;
	GLfloat gridsize = 50;

	for (float z = gridsize / 2; z > gridsize / -2; z -= size)
	{
		for (float x = gridsize / 2; x > gridsize / -2; x -= size)
		{
			glBegin(GL_QUADS);

			glNormal3f(0, 1, 0);
			glTexCoord2f(1.0, 1.0);
			glVertex3f(x, 0, z);

			glNormal3f(0, 1, 0);
			glTexCoord2f(1.0, 0.0);
			glVertex3f(x, 0, z - size);

			glNormal3f(0, 1, 0);
			glTexCoord2f(0.0, 0.0);
			glVertex3f(x - size, 0, z - size);

			glNormal3f(0, 1, 0);
			glTexCoord2f(0.0, 1.0);
			glVertex3f(x - size, 0, z);

			glEnd();
		}
	}

	glPopMatrix();
}

void drawLake(void)
{
	glBindTexture(GL_TEXTURE_2D, texID[1]);

	glPushMatrix();

	GLfloat size = 2;
	GLfloat gridsizeZ = 50;

	for (float z = gridsizeZ / 2; z > gridsizeZ / -2; z -= size)
	{
		glBegin(GL_QUADS);

		glNormal3f(0, 1, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(4, 0.01, z);

		glNormal3f(0, 1, 0);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(4, 0.01, z - size);

		glNormal3f(0, 1, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-4, 0.01, z - size);

		glNormal3f(0, 1, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-4, 0.01, z);

		glEnd();
	}

	glPopMatrix();
}

void drawFog(void)
{
	glFogfv(GL_FOG_COLOR, fogColour);

	glFogf(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.055);
}

/*
	Populate the scene with objects
*/
void drawTree(void)
{
	glDisable(GL_TEXTURE_2D);

	GLUquadricObj* base = gluNewQuadric();

	glPushMatrix();

	GLfloat baseColour[] = { 0.55, 0.30, 0.15, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, baseColour);

	glTranslated(0, 0, 0);
	glRotated(270, 1, 0, 0);
	gluCylinder(base, 0.2, 0.2, 2, 10, 10);

	glPushMatrix();

	GLfloat leafColour[] = { 0.05, 0.45, 0.05, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, leafColour);

	glTranslated(0, 0, 0.75);
	glutSolidCone(0.8, 2.4, 40, 40);

	glTranslated(0, 0, 0.6);
	glutSolidCone(0.7, 2, 40, 40);

	glTranslated(0, 0, 0.6);
	glutSolidCone(0.6, 1.6, 40, 40);

	glPopMatrix();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawTreeGroup(GLfloat x, GLfloat y, GLfloat z)
{
	glPushMatrix();

	glTranslated(x, y, z);
	glCallList(treeDL);

	glPushMatrix();
	glTranslated(1, 0, 2);
	glCallList(treeDL);
	glPopMatrix();

	glPushMatrix();
	glTranslated(1, 0, -2);
	glCallList(treeDL);
	glPopMatrix();

	glTranslated(2, 0, 0);
	glCallList(treeDL);

	glPushMatrix();
	glTranslated(1, 0, 2);
	glCallList(treeDL);
	glPopMatrix();

	glPushMatrix();
	glTranslated(1, 0, -2);
	glCallList(treeDL);
	glPopMatrix();

	glTranslated(2, 0, 0);
	glCallList(treeDL);

	glPopMatrix();
}

void drawForest(GLfloat x, GLfloat y, GLfloat z)
{
	//create a forest by drawing multiple trees using the display list
	
	//right side forest

	drawTreeGroup(x, y, z);
	drawTreeGroup(x + 5, y, z + 3);
	drawTreeGroup(x + 10, y, z + 6);
	drawTreeGroup(x + 12, y, z + 10);
	drawTreeGroup(x + 14, y, z + 14);
	drawTreeGroup(x + 14, y, z + 20);
	z *= -1;
	drawTreeGroup(x, y, z);
	drawTreeGroup(x + 5, y, z - 3);
	drawTreeGroup(x + 10, y, z - 6);
	drawTreeGroup(x + 12, y, z - 10);
	drawTreeGroup(x + 14, y, z - 14);
	drawTreeGroup(x + 14, y, z - 20);
	z *= -1;

	//left side forest

	drawTreeGroup(x - 14, y, z);
	drawTreeGroup(x - 19, y, z + 3);
	drawTreeGroup(x - 24, y, z + 6);
	drawTreeGroup(x - 26, y, z + 10);
	drawTreeGroup(x - 28, y, z + 14);
	drawTreeGroup(x - 28, y, z + 20);
	z *= -1;
	drawTreeGroup(x - 14, y, z);
	drawTreeGroup(x - 19, y, z - 3);
	drawTreeGroup(x - 24, y, z - 6);
	drawTreeGroup(x - 26, y, z - 10);
	drawTreeGroup(x - 28, y, z - 14);
	drawTreeGroup(x - 28, y, z - 20);
}

void drawHelipad(void)
{
	glDisable(GL_TEXTURE_2D);

	GLUquadricObj* helipadBase = gluNewQuadric();
	GLUquadricObj* leftH = gluNewQuadric();
	GLUquadricObj* rightH = gluNewQuadric();
	GLUquadricObj* midH = gluNewQuadric();

	glPushMatrix();

	GLfloat ambient[] = { 0.4, 0.4, 0.4, 1 };
	GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
	GLfloat specular[] = { 1, 1, 1, 1 };
	GLfloat shininess = 50;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glTranslated(6, 0.01, 0);
	glScaled(2, 0.01, 3);

	gluCylinder(helipadBase, 0.4, 0.4, 0.5, 20, 20); //draw the base of the helipad

	glPushMatrix();

	GLfloat letterColour[] = { 0.2, 0.0, 0.0, 1 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, letterColour);

	glTranslated(0.15, 0.5, 0.1);
	glScaled(0.4, 1, 0.8);

	gluCylinder(leftH, 0.2, 0.2, 0.4, 20, 20); //draw the left vertical section of the H

	glPopMatrix();

	glPushMatrix();

	glTranslated(-0.15, 0.5, 0.1);
	glScaled(0.4, 1, 0.8);

	gluCylinder(rightH, 0.2, 0.2, 0.4, 20, 20); //draw the right vertical section of the H

	glPopMatrix();

	glPushMatrix();

	glTranslated(0, 0.5, 0.225);
	glScaled(0.2, 1, 0.2);

	gluCylinder(midH, 0.35, 0.35, 0.4, 20, 20); //draw horizontal section of the H

	glPopMatrix();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawBuilding(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	GLfloat ambient[]  = { 0.30, 0.30, 0.35, 1 };
	GLfloat diffuse[]  = { 0.30, 0.30, 0.35, 1 };
	GLfloat specular[] = { 1, 1, 1, 1 };
	GLfloat shininess  = 60.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specular);
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glTranslated(6.0, 0.625, 4.0);
	glScaled(1, 0.5, 1);
	glutSolidCube(2.5);

	glTranslated(0, 1.5, 0);
	glScaled(1, 1, 1);
	glutSolidCube(1.5);


	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawBoat(void)
{
	glDisable(GL_TEXTURE_2D);

	GLUquadricObj* boatTop = gluNewQuadric();

	GLfloat ambient[] = { 0.0, 0.1, 0.4, 1 };
	GLfloat diffuse[] = { 0.0, 0.1, 0.4, 1 };
	GLfloat specular[] = { 1, 1, 1, 1 };
	GLfloat shininess = 80.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glPushMatrix();

	glTranslated(boatPos.x, boatPos.y, boatPos.z);
	glRotated(270, 1, 0, 0);
	glScaled(1.25, 3.0, 1.25);

	gluCylinder(boat, 0.4, 0.4, 0.4, 40, 40); //draw the boat body

	gluQuadricOrientation(boat, GLU_INSIDE);
	gluDisk(boat, 0.0, 0.4, 40, 1); //draw the closing bottom disc of the cylinder

	glPushMatrix();

	glTranslated(0.0, 0.0, 0.4);
	gluQuadricOrientation(boat, GLU_OUTSIDE);
	gluDisk(boat, 0.0, 0.4, 40, 1); //draw the closing top disc of the cylinder

	glPopMatrix();

	glPushMatrix(); //draw the top section of the boat

	GLfloat boatTopColour[] = { 0.8, 0.1, 0.0, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, boatTopColour);

	glTranslated(0.0, 0.0, 0.4);
	glScaled(0.8, 0.35, 1.0);

	gluCylinder(boatTop, 0.25, 0.25, 0.25, 40, 40);

	gluQuadricOrientation(boatTop, GLU_INSIDE);
	gluDisk(boatTop, 0.0, 0.25, 40, 1);

	glPushMatrix();

	glTranslated(0.0, 0.0, 0.25);
	gluQuadricOrientation(boatTop, GLU_OUTSIDE);
	gluDisk(boatTop, 0.0, 0.25, 40, 1);

	glPopMatrix();

	glPopMatrix();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void updateBoat(void)
{
	if (boatPos.z > -20.0 && !changeBoatDir)
	{
		boatPos.z -= boatSpeed; //drive boat forwards in the facing direction
		boatPos.x = sin(boatPos.z * 0.4) * 2.5; //oscillate boat in the path of a sine curve 
	}
	else
	{
		changeBoatDir = 1;
	}

	if (boatPos.z < 20.0 && changeBoatDir)
	{
		boatPos.z += boatSpeed; //drive boat forwards in the facing direction
		boatPos.x = sin(boatPos.z * 0.4) * 2.5; //oscillate boat in the path of a sine curve 
	}
	else
	{
		changeBoatDir = 0;
	}
}

/*
	Draw helicopter and rotors
*/
void drawHeli(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

		GLfloat ambient[] = { 1.0, 0.0, 0.0, 1 };
		GLfloat diffuse[] = { 1.0, 0.0, 0.0, 1 };
		GLfloat specular[] = { 1, 1, 1, 1 };
		GLfloat shininess = 40;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

		glTranslated(heliPos.x, heliPos.y, heliPos.z);
		glRotated(heliHeading, 0.0, 1.0, 0.0);
		glScaled(0.6, 0.5, 0.75);

		gluSphere(heliBody, bodyRadius, 50, 50);	

		drawTail();
		drawConnection();
		drawLegs();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawTail(void)
{
	glDisable(GL_TEXTURE_2D);

	glColor3f(1.0, 0.0, 0.0);

	tailLength = (bodyRadius * 1.75);

	glPushMatrix();

	glTranslated(0.0, bodyRadius/4, bodyRadius + (bodyRadius * 1.5));
	glRotated(-180, 1.0, 0.0, 0.0);
	glScaled(1.0, 1.0, 1.0);

	gluCylinder(heliTail, bodyRadius/5, bodyRadius/2, (bodyRadius * 2), 20, 20);

	drawTailRotor();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawTailRotor(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	GLfloat ambientLight[] = { 0.0, 0.65, 0.65, 1 };
	GLfloat diffuseLight[] = { 0.0, 0.65, 0.65, 1 };
	GLfloat specularLight[] = { 1, 1, 1, 1 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularLight);

	glTranslated(0, 0, bodyRadius * -0.05);
	glScaled(1.0, 1.0, 1.0);
	gluSphere(tailConnection, bodyRadius / 6, 20, 20);

	GLfloat ambientLight2[] = { 0.75, 0.75, 0.0, 1 };
	GLfloat diffuseLight2[] = { 0.75, 0.75, 0.0, 1 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight2);

	glTranslated(0, 0, bodyRadius * -0.1);
	glScaled(1.0, 1.0, 0.2);
	glRotated(rotAngle, 0.0, 0.0, 1.0);

	glRotated(90, 1.0, 0.0, 0.0);
	gluCylinder(heliTailRotor[0], bodyRadius / 12, bodyRadius / 12, bodyRadius * 0.75, 20, 20);

	glRotated(180, 1.0, 0.0, 0.0);
	gluCylinder(heliTailRotor[1], bodyRadius / 12, bodyRadius / 12, bodyRadius * 0.75, 20, 20);

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawConnection(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	glColor3f(0.75, 0.75, 0.0);
	glTranslated(0, bodyRadius * 1.25, 0); 
	glScaled(1.0, 1.0, 1.0);
	glRotated(90, 1.0, 0.0, 0.0);

	glColor3f(0.0, 0.65, 0.65);
	gluCylinder(rotorConnection, bodyRadius / 10, bodyRadius / 10, bodyRadius / 3, 20, 20);

	drawBlades();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawBlades(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	glRotated(270, 1.0, 0.0, 0.0);
	glScaled(1.0, 0.2, 1.0);

	glColor3f(0.75, 0.75, 0.0);
	glRotated(rotAngle, 0.0, 1.0, 0.0);
	gluCylinder(heliBlades[0], bodyRadius / 8, bodyRadius / 8, bodyRadius * 3, 20, 20);

	glColor3f(0.75, 0.75, 0.0);
	glRotated(600, 0.0, 1.0, 0.0);
	gluCylinder(heliBlades[1], bodyRadius / 8, bodyRadius / 8, bodyRadius * 3, 20, 20);

	glColor3f(0.75, 0.75, 0.0);
	glRotated(600, 0.0, 1.0, 0.0);
	gluCylinder(heliBlades[2], bodyRadius / 8, bodyRadius / 8, bodyRadius * 3, 20, 20);

	glColor3f(0.5, 0.5, 0.0);
	glScaled(1.0, 1.0, 1.0);
	gluSphere(bladeTop, bodyRadius / 5, 20, 20);

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

void drawLegs(void)
{
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.0, 0.65, 0.65);

	glScaled(1.0, 0.4, 1.0);	

	glTranslated(bodyRadius/4, -bodyRadius *  2.5, -bodyRadius);
	gluCylinder(legs[0], bodyRadius / 8, bodyRadius / 8, bodyRadius * 2.5, 20, 20);

	glTranslated(-bodyRadius/2, 0, 0);
	gluCylinder(legs[1], bodyRadius / 8, bodyRadius / 8, bodyRadius * 2.5, 20, 20);

	glEnable(GL_TEXTURE_2D);
}

/*
	Particle System Functions
*/
void createParticle(void)
{
	particle.size = (((float)rand() / (float)RAND_MAX) * 0.2) + 0.1;
	particle.alpha = (((float)rand() / (float)RAND_MAX) * 0.6) + 0.4;

	particle.position.x = (((float)rand() / (float)RAND_MAX) * 50.0) - 25.0;
	particle.position.z = (((float)rand() / (float)RAND_MAX) * 50.0) - 25.0;
	particle.position.y = 25.0;

	particle.dy = particle.size;

	particle.active = 1;
}

void spawnParticles(void)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (particle_system[i].active == 0)
		{
			createParticle();
			particle_system[i] = particle;
			numParticles++;
		}
	}
}

void updateRain(int i)
{
	if (particle_system[i].active == 1)
	{
		particle_system[i].position.y -= particle_system[i].dy;

		if (particle_system[i].position.y < 0.0)
		{
			createParticle();
			particle_system[i] = particle;
		}
	}
}

void drawParticles(void)
{
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);

	for (int i = 0; i < numParticles; i++)
	{
		if (particle_system[i].active)
		{
			GLfloat ambientLight[] = { 0.0, 0.0, 1.0, 1.0 };
			GLfloat diffuseLight[] = { 0.0, 0.0, 1.0, 1.0 };
			GLfloat specularLight[] = { 1, 1, 1, 1 };

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientLight);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseLight);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularLight);

			glPushMatrix();

			glTranslated(particle_system[i].position.z, particle_system[i].position.y, particle_system[i].position.x);
			glScaled(0.05, 0.5, 0.05);

			glutSolidSphere(particle_system[i].size, 4, 4);

			glPopMatrix();
		}
	}

	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
}

/*
	Helper Functions
*/
void drawOrigin(void)
{
	glColor3f(0.0f, 1.0f, 1.0f);
	glutWireSphere(0.1, 10, 10);

	glBegin(GL_LINES);

	//x axis -red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.0f, 0.0f, 0.0f);

	//y axis -green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);

	//z axis - blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 2.0f);

	glEnd();
}

/******************************************************************************/