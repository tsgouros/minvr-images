#include "mvImage.h"
#include "mvImageApp.h"


void mvImageApp::graphicsInit(MinVR::VRDataIndex* index) {
	GLint result;

  std::string vertexShader =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/vertex"));
  std::string fragmentShader =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/fragment"));
  
	// create program object and attach shaders 
	gProgram = glCreateProgram();
	shaderAttach(gProgram, GL_VERTEX_SHADER, vertexShader);
	shaderAttach(gProgram, GL_FRAGMENT_SHADER, fragmentShader);

	// link the program and make sure that there were no errors 
	glLinkProgram(gProgram);
	glGetProgramiv(gProgram, GL_LINK_STATUS, &result);
	if(result == GL_FALSE) {
		GLint length;
		char *log;

		// get the program info log 
		glGetProgramiv(gProgram, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(gProgram, length, &result, log);

		// print an error message and the info log 
    std::cerr << "Program linking failed: " << std::string(log) << std::endl;
		free(log);

		// delete the program 
		glDeleteProgram(gProgram);
		gProgram = 0;
	}

  // get uniform locations 
	gProgramCameraPositionLocation = glGetUniformLocation(gProgram, "cameraPosition");
	gProgramLightPositionLocation = glGetUniformLocation(gProgram, "lightPosition");
	gProgramLightColorLocation = glGetUniformLocation(gProgram, "lightColor");

	// set up red/green/blue lights 
	gLightColor[0] = 1.0f; gLightColor[1] = 0.0f; gLightColor[2] = 0.0f;
	gLightColor[3] = 0.0f; gLightColor[4] = 1.0f; gLightColor[5] = 0.0f;
	gLightColor[6] = 0.0f; gLightColor[7] = 0.0f; gLightColor[8] = 1.0f;

	// create cylinder 
	_images.create();

	// do the first cycle to initialize positions 
	gLightRotation = 0.0f;
  //	sceneCycle();

	// setup camera 
	gCameraPosition[0] = 0.0f;
	gCameraPosition[1] = 0.0f;
	gCameraPosition[2] = 4.0f;
	glLoadIdentity();
	glTranslatef(-gCameraPosition[0], -gCameraPosition[1], -gCameraPosition[2]);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearDepth(1.0f);
  glClearColor(0.0, 0.0, 0.0, 1.f);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  //glClear(GL_DEPTH_BUFFER_BIT);                                
  
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
  _vrMain(NULL), _quit(false) {

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

// Callback for rendering, inherited from VRRenderHandler                      
void mvImageApp::onVRRenderScene(MinVR::VRDataIndex *renderState,
                                 MinVR::VRDisplayNode *callingNode) {
  if (renderState->exists("IsConsole", "/")) {
    MinVR::VRConsoleNode *console =
      dynamic_cast<MinVR::VRConsoleNode*>(callingNode);
    console->println("Console output...");
  } else {


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

    // Draw objects here.
    _images.draw();
  }
}

int main(int argc, char **argv) {

  mvImageApp app(argc, argv);
  app.run();

  exit(0);  
}

