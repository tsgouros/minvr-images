#include "mvImage.h"
#include "mvImageApp.h"




void mvImageApp::graphicsInit(MinVR::VRDataIndex* index) {
	GLint result;

  std::string vertexShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/vertex"));
  std::string fragmentShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/fragment"));

  std::cout << "initialize graphics using shaders: " << vertexShaderFile << " and " << fragmentShaderFile << std::endl;

  std::cout << "create program object and attach shaders" << std::endl;

  std::cout << "link the program and make sure there were no errors" << std::endl;

  std::cout << "get uniform locations" << std::endl;
	// _gProgramCameraPositionLocation = glGetUniformLocation(_gProgram, "cameraPosition");
	// _gProgramLightPositionLocation = glGetUniformLocation(_gProgram, "lightPosition");
	// _gProgramLightColorLocation = glGetUniformLocation(_gProgram, "lightColor");

  std::cout << "set up red/green/blue lights" << std::endl;
	_gLightColor[0] = 1.0f; _gLightColor[1] = 0.0f; _gLightColor[2] = 0.0f;
	_gLightColor[3] = 0.0f; _gLightColor[4] = 1.0f; _gLightColor[5] = 0.0f;
	_gLightColor[6] = 0.0f; _gLightColor[7] = 0.0f; _gLightColor[8] = 1.0f;

  std::cout << "do the first cycle to initialize positions" << std::endl;
	_gLightRotation = 0.0f;
  //	sceneCycle();

  std::cout << "setup camera" << std::endl;
	// _gCameraPosition[0] = 0.0f;
	// _gCameraPosition[1] = 10.0f;
	// _gCameraPosition[2] = 0.0f;
	// glLoadIdentity();
	// glTranslatef(-_gCameraPosition[0], -_gCameraPosition[1], -_gCameraPosition[2]);

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


    _images.addImage("/mvImage/images/" + *it, _vrMain->getConfig());
    
  }

  // Add some miscellaneous objects here.
  char name[10];
  for(int i = 0; i < NUM_LIGHTS; ++i) {
    
    mvImageShapeSphere* s = new mvImageShapeSphere(0.4, 2, 2);
    sprintf(name, "sphere%d", i);
    _images.addImage(std::string(name), NULL, (mvImageShape*)s);
    
  }

  // Create all the shapes.  This will go through each shape and load
  // its vertices into a vertex buffer and record where they are.
  // Then the draw commands can just reference those buffers.
  std::cout << "create shapes" << std::endl;
	_images.create();
  
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

  std::cout << "on render context" << std::endl;

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

  std::cout << "on render scene" << std::endl;
  
  if (renderState->exists("IsConsole", "/")) {
    MinVR::VRConsoleNode *console =
      dynamic_cast<MinVR::VRConsoleNode*>(callingNode);
    console->println("Console output...");
  } else {

    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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

      std::cout << "projection matrix exists" << std::endl;
      
      // glMatrixMode(GL_PROJECTION);
      // glLoadIdentity();
      // MinVR::VRMatrix4 P = renderState->getValue("ProjectionMatrix", "/");
      // glLoadMatrixd(P.m);

      // glMatrixMode(GL_MODELVIEW);
      // glLoadIdentity();

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
      //      glLoadMatrixd((V * M).m);

    } else {

      std::cout << "no projection matrix" << std::endl;
      
      // If the DisplayGraph does not contain a node that sets the
      // ProjectionMatrix and ViewMatrix, then we must be in a
      // non-headtracked simple desktop mode.  We can just set the
      // projection and modelview matrices.
      // glMatrixMode(GL_PROJECTION);
      // glLoadIdentity();
      // setPerspective(M_PI / 4.0f, 1.f, 0.1f, 100.0f);
      // glMatrixMode(GL_MODELVIEW);
      // glLoadIdentity();

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

    // glUseProgram(_gProgram);
    // glUniform3fv(_gProgramCameraPositionLocation, 1, _gCameraPosition);
    // glUniform3fv(_gProgramLightPositionLocation, NUM_LIGHTS, _gLightPosition);
    // glUniform3fv(_gProgramLightColorLocation, NUM_LIGHTS, _gLightColor);

    // Render something for each light.
    char name[10];
    for(int i = 0; i < NUM_LIGHTS; ++i) {
      // Render a shape with the light's color and position.
      // glPushMatrix();
      // glTranslatef(_gLightPosition[i * 3 + 0], _gLightPosition[i * 3 + 1], _gLightPosition[i * 3 + 2]);
      // glColor3fv(_gLightColor + (i * 3));

      sprintf(name, "sphere%d", i);
      _images.getImage(std::string(name))->setPosition(_gLightPosition[i * 3 + 0],
                                                       _gLightPosition[i * 3 + 1],
                                                       _gLightPosition[i * 3 + 2]);
 
    }
   
    // Draw objects here.
    _images.draw();
    
    std::cout << "Draw some axes because why not." << std::endl;
    // glBegin(GL_LINES);
    // glColor3f(1.0, 0.0, 0.0);
    // glVertex3f(8.0, 0.0, 0.0);
    // glVertex3f(0.0, 0.0, 0.0);

    // glColor3f(0.0, 1.0, 0.0);
    // glVertex3f(0.0, 8.0, 0.0);
    // glVertex3f(0.0, 0.0, 0.0);

    // glColor3f(0.0, 0.0, 1.0);
    // glVertex3f(0.0, 0.0, 8.0);
    // glVertex3f(0.0, 0.0, 0.0);
    // glEnd();


    // glPopMatrix();
  }
}

int main(int argc, char **argv) {

  mvImageApp app(argc, argv);
  app.run();

  exit(0);  
}

