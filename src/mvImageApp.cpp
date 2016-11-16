#include "mvImage.h"
#include "mvImageApp.h"

void mvImageApp::fghCircleTable(double **sint,double **cost,const int n) {
  int i;

  /* Table size, the sign of n flips the circle direction */

  const int size = abs(n);

  /* Determine the angle between samples */

  const double angle = 2*M_PI/(double)( ( n == 0 ) ? 1 : n );

  /* Allocate memory for n samples, plus duplicate of first entry at the end */

  *sint = (double *) calloc(sizeof(double), size+1);
  *cost = (double *) calloc(sizeof(double), size+1);

  /* Bail out if memory allocation fails, fgError never returns */

  if (!(*sint) || !(*cost)) {
    free(*sint);
    free(*cost);
    std::cout << "Failed to allocate memory in fghCircleTable" << std::endl;
  }

  /* Compute cos and sin around the circle */

  (*sint)[0] = 0.0;
  (*cost)[0] = 1.0;

  for (i=1; i<size; i++) {
    (*sint)[i] = sin(angle*i);
    (*cost)[i] = cos(angle*i);
  }

  /* Last sample is duplicate of the first */

  (*sint)[size] = (*sint)[0];
  (*cost)[size] = (*cost)[0];
}

/*
 * Draws a solid sphere.  Stolen from freeglut because I didn't want
 * to import the whole thing.
 */
void mvImageApp::makeSolidSphere(GLdouble radius, GLint slices, GLint stacks) {
  int i,j;

  /* Adjust z and radius as stacks are drawn. */

  double z0,z1;
  double r0,r1;

  /* Pre-computed circle */

  double *sint1,*cost1;
  double *sint2,*cost2;

  fghCircleTable(&sint1,&cost1,-slices);
  fghCircleTable(&sint2,&cost2,stacks*2);

  /* The top stack is covered with a triangle fan */

  z0 = 1.0;
  z1 = cost2[(stacks>0)?1:0];
  r0 = 0.0;
  r1 = sint2[(stacks>0)?1:0];

  glBegin(GL_TRIANGLE_FAN);

  glNormal3d(0,0,1);
  glVertex3d(0,0,radius);

  for (j=slices; j>=0; j--) {
    glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
    glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
  }

  glEnd();

  /* Cover each stack with a quad strip, except the top and bottom stacks */

  for( i=1; i<stacks-1; i++ ) {
    z0 = z1; z1 = cost2[i+1];
    r0 = r1; r1 = sint2[i+1];

    glBegin(GL_QUAD_STRIP);

    for(j=0; j<=slices; j++) {
      glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
      glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
      glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
      glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
    }

    glEnd();
  }

  /* The bottom stack is covered with a triangle fan */

  z0 = z1;
  r0 = r1;

  glBegin(GL_TRIANGLE_FAN);

  glNormal3d(0,0,-1);
  glVertex3d(0,0,-radius);

  for (j=0; j<=slices; j++) {
    glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
    glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
  }

  glEnd();

  /* Release sin and cos tables */

  free(sint1);
  free(cost1);
  free(sint2);
  free(cost2);
}


void mvImageApp::graphicsInit(MinVR::VRDataIndex* index) {
	GLint result;

  std::string vertexShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/vertex"));
  std::string fragmentShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/fragment"));
  
	// create program object and attach shaders 
	_gProgram = glCreateProgram();
	shaderAttach(_gProgram, GL_VERTEX_SHADER, vertexShaderFile);
	shaderAttach(_gProgram, GL_FRAGMENT_SHADER, fragmentShaderFile);

	// link the program and make sure that there were no errors 
	glLinkProgram(_gProgram);
	glGetProgramiv(_gProgram, GL_LINK_STATUS, &result);
	if(result == GL_FALSE) {
		GLint length;
		char *log;

		// get the program info log 
		glGetProgramiv(_gProgram, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(_gProgram, length, &result, log);

		// print an error message and the info log 
    std::cerr << "Program linking failed: " << std::string(log) << std::endl;
		free(log);

		// delete the program 
		glDeleteProgram(_gProgram);
		_gProgram = 0;
	}

  // get uniform locations 
	_gProgramCameraPositionLocation = glGetUniformLocation(_gProgram, "cameraPosition");
	_gProgramLightPositionLocation = glGetUniformLocation(_gProgram, "lightPosition");
	_gProgramLightColorLocation = glGetUniformLocation(_gProgram, "lightColor");

	// set up red/green/blue lights 
	_gLightColor[0] = 1.0f; _gLightColor[1] = 0.0f; _gLightColor[2] = 0.0f;
	_gLightColor[3] = 0.0f; _gLightColor[4] = 1.0f; _gLightColor[5] = 0.0f;
	_gLightColor[6] = 0.0f; _gLightColor[7] = 0.0f; _gLightColor[8] = 1.0f;

	// do the first cycle to initialize positions 
	_gLightRotation = 0.0f;
  //	sceneCycle();

	// setup camera 
	// _gCameraPosition[0] = 0.0f;
	// _gCameraPosition[1] = 10.0f;
	// _gCameraPosition[2] = 0.0f;
	// glLoadIdentity();
	// glTranslatef(-_gCameraPosition[0], -_gCameraPosition[1], -_gCameraPosition[2]);

  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 37.0);
  glClearColor(0.0, 0.0, 0.0, 1.f);
  glClearDepth(1.0f);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  //glClear(GL_DEPTH_BUFFER_BIT);                                

	// create shapes 
	_images.create();

}

std::string mvImageApp::shaderRead(std::string pathName) {

  std::string shaderString;
  
#ifdef DEBUG
  std::cout << "Reading from file = " << pathName << std::endl;
#endif
  ifstream file(pathName.c_str());

  if(file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.rdbuf()->str();
    
  } else {

    throw std::runtime_error("Error opening file " + pathName);

  }
  return std::string("");
}


// Returns a shader object containing a shader compiled from the given
// GLSL shader file.
GLuint mvImageApp::shaderCompile(const GLenum type, const std::string pathName) {
  std::string shaderSource;
	GLuint shader;
	GLint length, result;

	// get shader source 
	shaderSource = shaderRead(pathName);
	if (shaderSource.size() == 0) return 0;

	// create shader object, set the source, and compile 
	shader = glCreateShader(type);
  length = shaderSource.size();
  // The cast in the following line is there because of a bug in the gl.h
  // header file (missing comma) on Mac OSX 10.10
  glShaderSource(shader, 1, (const char **)shaderSource.c_str(), &length);
	glCompileShader(shader);

	// make sure the compilation was successful 
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		char *log;

		// get the shader info log 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);

		// print an error message and the info log 
    std::cerr << "ERR: Unable to compile " << pathName << ": " << log << std::endl;
		free(log);
    //    throw std::runtime_error("Unable to compile " + pathName + ": " +
    //                             std::string(log));
    
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

// Compiles and attaches a shader of the given type to the given
// program object.
void mvImageApp::shaderAttach(const GLuint program,
                              const GLenum type,
                              const std::string pathName) {
	// compile the shader
	GLuint shader = shaderCompile(type, pathName);
	if(shader != 0) {
		// attach the shader to the program 
		glAttachShader(program, shader);

		// delete the shader - it won't actually be destroyed until the
		// program that it's attached to has been destroyed.
		glDeleteShader(shader);
	}
}


mvImageApp::mvImageApp(int argc, char** argv) :
  _vrMain(NULL), _lastMilliSeconds(0), _quit(false) {

  _vrMain = new MinVR::VRMain();

  // The first argument is an initialization file for the images.  Remove that
  // from the argument list and pass the rest to MinVR.
  if (argc < 3) {
    throw std::runtime_error("Need two config files, for mvImage and MinVR.");
  }
  std::string MVIConfigFile = std::string(argv[1]);
  std::string MinVRConfigFile = std::string(argv[2]);
      
  _vrMain->initialize(argc, argv, MinVRConfigFile);
  _vrMain->addEventHandler(this);
  _vrMain->addRenderHandler(this);

  _horizAngle = 0.0;
  _vertAngle = 0.0;
  _radius = 15.0;
  _incAngle = -0.1f;

  _vrMain->getConfig()->processXMLFile(MVIConfigFile);
  std::cout << _vrMain->getConfig()->printStructure() << std::endl;
  
  // Do the graphics initialization.
  graphicsInit(_vrMain->getConfig());

  // Parse the image data out of the image config information.
  MinVR::VRContainer imgs = _vrMain->getConfig()->getValue("/mvImage/images");
  for (MinVR::VRContainer::iterator it = imgs.begin(); it != imgs.end(); it++) {

    std::string file =
      _vrMain->getConfig()->getValue((*it) + "/imageFile", "/mvImage/images");

    // Initialize the image.
    mvImage* img = new mvImage(file);

    // Read the image shape and create a corresponding image object.
    std::string shape =
      _vrMain->getConfig()->getValue((*it) + "/shape", "/mvImage/images");

    // Read the image shape's dimensions. (For now we're just doing
    // rectangles.)
    double height =
      _vrMain->getConfig()->getValue((*it) + "/height", "/mvImage/images");
    double width =
      _vrMain->getConfig()->getValue((*it) + "/width", "/mvImage/images");
    
    // Find the shape's position and orientation.
    MinVR::VRMatrix4 transform =
      _vrMain->getConfig()->getValue((*it) + "/transform", "/mvImage/images");

    mvImageShapeRectangle* rect =
      new mvImageShapeRectangle(transform, height, width);
    img->setShape(rect);
    
    _images.addImage(*it, img);
    
    // std::cout << "adding: " << *it << std::endl;
  }  
}

mvImageApp::~mvImageApp() {
  _vrMain->shutdown();
  delete _vrMain;
}

void
mvImageApp::setPerspective(float fov, float aspect, float near, float far) {
	float f;
	float mat[16];

	f = 1.0f / tanf(fov / 2.0f);

	mat[0] = f / aspect;
	mat[1] = 0.0f;
	mat[2] = 0.0f;
	mat[3] = 0.0f;

	mat[4] = 0.0f;
	mat[5] = f;
	mat[6] = 0.0f;
	mat[7] = 0.0f;

	mat[8] = 0.0f;
	mat[9] = 0.0f;
	mat[10] = (far + near) / (near  - far);
	mat[11] = -1.0f;

	mat[12] = 0.0f;
	mat[13] = 0.0f;
	mat[14] = (2.0f * far * near) / (near - far);
	mat[15] = 0.0f;

	glMultMatrixf(mat);
}

void mvImageApp::lookAt(float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ) {

  MinVR::VRVector3 forward, side, up;
  float m[4][4];

  // Normalize and straighten out the input directions.
  forward = MinVR::VRVector3(centerX - eyeX, centerY - eyeY, centerZ - eyeZ).normalize();
  side = forward.cross( MinVR::VRVector3(upX, upY, upZ) ).normalize();
  up = side.cross(forward);

  // Insert them into a view matrix.
  MinVR::VRMatrix4 M;
  M[0][0] = side[0];
  M[1][0] = side[1];
  M[2][0] = side[2];
  
  M[0][1] = up[0];
  M[1][1] = up[1];
  M[2][1] = up[2];
  
  M[0][2] = -forward[0];
  M[1][2] = -forward[1];
  M[2][2] = -forward[2];

  // Add the view matrix to the mix.
  glMultMatrixd( M.m );

  // And translate away from the center.
  glTranslated(-eyeX, -eyeY, -eyeZ);
}

void mvImageApp::onVREvent(const std::string &eventName, MinVR::VRDataIndex *eventData) {

  //std::cout << "Event: " << eventName << std::endl;                    
  if (eventName == "/KbdEsc_Down") {
    _quit = true;
  } else if (eventName == "/MouseBtnLeft_Down") {
    _radius += 5.0 * _incAngle;
  } else if (eventName == "/MouseBtnRight_Down") {
    _radius -= 5.0 * _incAngle;
  } else if ((eventName == "/KbdLeft_Down") || (eventName == "/KbdLeft_Repeat")) {
    _horizAngle -= _incAngle;
  } else if ((eventName == "/KbdRight_Down") || (eventName == "/KbdRight_Repeat")) {
    _horizAngle += _incAngle;
  } else if ((eventName == "/KbdUp_Down") || (eventName == "/KbdUp_Repeat")) {
    _vertAngle -= _incAngle;
  } else if ((eventName == "/KbdDown_Down") || (eventName == "/KbdDown_Repeat")) {
    _vertAngle += _incAngle;
  }

  if (_horizAngle > TWOPI) _horizAngle -= TWOPI;
  if (_horizAngle < 0.0) _horizAngle += TWOPI;

  if (_vertAngle > TWOPI) _vertAngle -= TWOPI;
  if (_vertAngle < 0.0) _vertAngle += TWOPI;
}

void mvImageApp::onVRRenderContext(MinVR::VRDataIndex *renderState, 
				   MinVR::VRDisplayNode *callingNode) {
  if (!renderState->exists("IsConsole", "/")) {
  }
}

float mvImageApp::getElapsedSeconds(void) {

  struct timeval now;
  
  if (_lastMilliSeconds == 0) {
    gettimeofday(&now, NULL);
    _lastMilliSeconds = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    return 0.0f;
  }

  gettimeofday(&now, NULL);
  float out = 0.001f * (((now.tv_sec * 1000) + (now.tv_usec / 1000)) - _lastMilliSeconds);
  _lastMilliSeconds = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
  //  std::cout << "::" << _lastMilliSeconds << ">>" << out << std::endl;

  return out;
}


// Callback for rendering, inherited from VRRenderHandler                      
void mvImageApp::onVRRenderScene(MinVR::VRDataIndex *renderState,
                                 MinVR::VRDisplayNode *callingNode) {
  if (renderState->exists("IsConsole", "/")) {
    MinVR::VRConsoleNode *console =
      dynamic_cast<MinVR::VRConsoleNode*>(callingNode);
    console->println("Console output...");
  } else {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    static unsigned int prevTicks = 0;
    unsigned int ticks;
    float secondsElapsed;
    int i;

    // calculate the number of seconds since the program started.
    secondsElapsed = getElapsedSeconds();
    
    _gLightRotation += (M_PI / 4.0f) * secondsElapsed;
    //    std::cout << "rotation: " << _gLightRotation << std::endl;
    for(i = 0; i < NUM_LIGHTS; ++i) {
      const float radius = 1.75f;
      float r = (((M_PI * 2.0f) / (float)NUM_LIGHTS) * (float)i) + _gLightRotation;

      _gLightPosition[i * 3 + 0] = cosf(r) * radius;
      _gLightPosition[i * 3 + 1] = cosf(r) * sinf(r);
      _gLightPosition[i * 3 + 2] = sinf(r) * radius;
    }


    
    if (renderState->exists("ProjectionMatrix", "/")) {
      // This is the typical case where the MinVR DisplayGraph contains      
      // an OffAxisProjectionNode or similar node, which sets the            
      // ProjectionMatrix and ViewMatrix based on head tracking data.        

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      MinVR::VRMatrix4 P = renderState->getValue("ProjectionMatrix", "/");
      glLoadMatrixd(P.m);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // In a VR program, we want the camera (Projection and View
      // matrices) to be controlled by head tracking.  So, we switch
      // to having the keyboard and mouse control the Model matrix.
      // Guideline: In VR, put all of the "scene navigation" into the
      // Model matrix and leave the Projection and View matrices for
      // head tracking.
      MinVR::VRMatrix4 M =
        MinVR::VRMatrix4::translation(MinVR::VRVector3(0.0, 0.0, -_radius)) *
        MinVR::VRMatrix4::rotationX(_vertAngle) *
        MinVR::VRMatrix4::rotationY(_horizAngle);

      MinVR::VRMatrix4 V = renderState->getValue("ViewMatrix", "/");
      glLoadMatrixd((V * M).m);

    } else {

      // If the DisplayGraph does not contain a node that sets the
      // ProjectionMatrix and ViewMatrix, then we must be in a
      // non-headtracked simple desktop mode.  We can just set the
      // projection and modelview matrices.
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      setPerspective(M_PI / 4.0f, 1.f, 0.1f, 100.0f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      float cameraPos[3];
      cameraPos[0] = _radius * cos(_horizAngle) * cos(_vertAngle);
      cameraPos[1] = -_radius * sin(_vertAngle);
      cameraPos[2] = _radius * sin(_horizAngle) * cos(_vertAngle);

      float cameraAim[3];
      cameraAim[0] = cos(_horizAngle) * sin(_vertAngle);
      cameraAim[1] = cos(_vertAngle);
      cameraAim[2] = sin(_horizAngle) * sin(_vertAngle);

      float targetPos[3] = {0.0f, 0.0f, 0.0f};
      lookAt(cameraPos[0], cameraPos[1], cameraPos[2],
             targetPos[0], targetPos[1], targetPos[2],
             cameraAim[0], cameraAim[1], cameraAim[2]);
    }

    glUseProgram(_gProgram);
    glUniform3fv(_gProgramCameraPositionLocation, 1, _gCameraPosition);
    glUniform3fv(_gProgramLightPositionLocation, NUM_LIGHTS, _gLightPosition);
    glUniform3fv(_gProgramLightColorLocation, NUM_LIGHTS, _gLightColor);

    // Draw objects here.
    _images.draw();
    
    // Draw some axes because why not.
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(8.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 8.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 8.0);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    // Render something for each light.
    for(int i = 0; i < NUM_LIGHTS; ++i) {
      // Render a shape with the light's color and position.
      glPushMatrix();
      glTranslatef(_gLightPosition[i * 3 + 0], _gLightPosition[i * 3 + 1], _gLightPosition[i * 3 + 2]);
      glColor3fv(_gLightColor + (i * 3));

      makeSolidSphere(0.4f, 5, 5);

      glPopMatrix();
    }



    
  }
}

int main(int argc, char **argv) {

  mvImageApp app(argc, argv);
  app.run();

  exit(0);  
}

