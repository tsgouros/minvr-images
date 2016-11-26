#include "mvImage.h"
#include "mvImageApp.h"




void mvImageApp::graphicsInit(MinVR::VRDataIndex* index) {

  std::string vertexShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/vertex"));
  std::string fragmentShaderFile =
    index->dereferenceEnvVars(index->getValue("/mvImage/shaders/fragment"));

  std::cout << "initialize graphics using shaders: " << vertexShaderFile << " and " << fragmentShaderFile << std::endl;

  std::cout << "create program object to hold the shaders" << std::endl;
  std::cout << "also link the program, check for errors" << std::endl;
  _gProgram = glCreateProgram();
  shaderAttach(_gProgram, GL_VERTEX_SHADER, vertexShaderFile);
  shaderAttach(_gProgram, GL_FRAGMENT_SHADER, fragmentShaderFile);
  
  std::cout << "the shaders are linked, now get uniform locations" << std::endl;

  // These are for the vertex shader.
  _gProgramCameraPositionLocation = glGetUniformLocation(_gProgram,
                                                         "cameraPosition");
  _gProgramLightPositionLocation = glGetUniformLocation(_gProgram, "lightPosition");
  _gProgramModelMatrix = glGetUniformLocation(_gProgram, "modelMatrix");
  _gProgramViewMatrix = glGetUniformLocation(_gProgram, "viewMatrix");
  _gProgramProjMatrix = glGetUniformLocation(_gProgram, "projMatrix");
  
  // The lightColor is part of the fragment shader.
  _gProgramLightColorLocation = glGetUniformLocation(_gProgram, "lightColor");

  glBindAttribLocation(_gProgram, 1, "vertices");
  glBindAttribLocation(_gProgram, 2, "colors");

  GLuint vboIDs[4];
  glGenBuffers(4, vboIDs);

  glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
  //  glBufferData(GL_ARRAY_BUFFER, sizeof(

  
  std::cout << "linking gl shaders to a program" << std::endl;
  glLinkProgram(_gProgram);

  
  std::cout << "set up red/green/blue lights" << std::endl;
	_gLightColor[0] = 1.0f; _gLightColor[1] = 0.0f; _gLightColor[2] = 0.0f;
	_gLightColor[3] = 0.0f; _gLightColor[4] = 1.0f; _gLightColor[5] = 0.0f;
	_gLightColor[6] = 0.0f; _gLightColor[7] = 0.0f; _gLightColor[8] = 1.0f;

  std::cout << "do the first cycle to initialize positions" << std::endl;
	_gLightRotation = 0.0f;

  std::cout << "Time for the real graphics initialization..." << std::endl;

  glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);

  // Create all the shapes.  This will go through each shape and load
  // its vertices into a vertex buffer and record where they are.
  // Then the draw commands just references those buffers.
  std::cout << "create shapes" << std::endl;
  
  for (imageMap::iterator it = _images.begin();
       it != _images.end(); it++) {
    std::cout << "creating: " << it->first << std::endl;
    it->second->create();
  }
}

// Reads a shader file into a string.  Just a locus for the error
// checking and that kind of thing.
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
	GLuint shader;
	GLint length, result;

	// get shader source 
  std::string shaderSource = shaderRead(pathName);
  if (shaderSource.size() == 0) return 0;

	// create shader object, set the source, and compile 
	shader = glCreateShader(type);
  length = shaderSource.size();

  const GLchar * source[] = {shaderSource.c_str()};
  glShaderSource(shader, 1, source, &length);
	glCompileShader(shader);

  // make sure the compilation was successful.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {

    char *log;

		// get the shader info log 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    log = (char*)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);
    
		// print an error message and the info log 
    std::cerr << "ERR: Unable to compile " << pathName << ": " << log << "<<" << std::endl;
		free(log);
    // throw std::runtime_error("Unable to compile " + pathName + ": " +
    //                          std::string(log));
    
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
	} else {
    throw std::runtime_error("shader attachment failed: " + pathName);
  }
}


mvImageApp::mvImageApp(int argc, char** argv) :
  _lastMilliSeconds(0), _quit(false), _needInitialization(true) {

  _vrMain = new MinVR::VRMain();

  _shapeFactory.registerMvImageShape("rectangle", createMvImageShapeRectangle);
  _shapeFactory.registerMvImageShape("box", createMvImageShapeBox);
  _shapeFactory.registerMvImageShape("sphere", createMvImageShapeSphere);
  
  // The first argument is an initialization file for the images.  Remove that
  // from the argument list and pass the rest to MinVR.
  if (argc < 3) {
    throw std::runtime_error("Need two config files, for mvImage and MinVR.");
  }
  std::string MVIConfigFile = std::string(argv[1]);
  std::string MinVRConfigFile = std::string(argv[2]);

  std::cout << "pre-initialize..." << std::endl;
  _vrMain->initialize(argc, argv, MinVRConfigFile);

  // std::cout << "First CGLGetCurrentContext()" << CGLGetCurrentContext() << std::endl;
  
  _vrMain->addEventHandler(this);
  _vrMain->addRenderHandler(this);

  // These angles control the desktop viewing position.
  _horizAngle = 0.0;
  _vertAngle = 0.0;
  _radius = 15.0;
  _incAngle = -0.1f;

  // Read the appplication-specific config file.
  _vrMain->getConfig()->processXMLFile(MVIConfigFile);
  std::cout << _vrMain->getConfig()->printStructure() << std::endl;

  // Parse the image data out of the image config information and add the
  // image objects defined in there.
  MinVR::VRContainer imgs = _vrMain->getConfig()->getValue("/mvImage/images");
  for (MinVR::VRContainer::iterator it = imgs.begin(); it != imgs.end(); it++) {

    addImage("/mvImage/images/" + *it, _vrMain->getConfig());
    
  }

  // Add any miscellaneous objects that appear regardless of config data
  // (axes, light proxies, whatever).

  // This loop adds three little light proxies to sit at the locations
  // defined by the lightPosition array.  So they will appear like
  // little lamps.
  char name[10];
  for(int i = 0; i < NUM_LIGHTS; ++i) {
    
    mvImageShapeSphere* s = new mvImageShapeSphere(0.4, 2, 2);
    sprintf(name, "sphere%d", i);
    addImage(std::string(name), NULL, (mvImageShape*)s);
    
  }
}

mvImageApp::~mvImageApp() {
  _vrMain->shutdown();

  for (imageMap::iterator it = _images.begin();
       it != _images.end(); it++) {
    delete it->second;
  }
  
  delete _vrMain;
}


// Adds an image object to the collection to be drawn.
std::string mvImageApp::addImage(const std::string name,
                                 MinVR::VRDataIndex* index) {

  std::cout << "adding a new image with the name: " << name << std::endl;
  _images[name] = new mvImage(name, index,
                              _shapeFactory.createMvImageShape(name, index));
  return name;
}

// We do envision mvImage objects with null imageData.  Presumably
// there is a color specified via the imageShape.
std::string mvImageApp::addImage(const std::string name,
                                 mvImageData* image, mvImageShape* shape) {

  std::cout << "adding a new image (" << shape->getShape() << ") with the name: " << name << std::endl;
  
  _images[name] = new mvImage(image, shape);
  return name;
}

int mvImageApp::delImage(const std::string name) {
  delete _images[name];
  return _images.erase(name);
}

mvImage* mvImageApp::getImage(const std::string name) {
  return _images[name];
}


MinVR::VRMatrix4 mvImageApp::perspective(float fov,
                                         float aspect,
                                         float near,
                                         float far) {

  MinVR::VRMatrix4 out;
  
  float f;
	f = 1.0f / tanf(fov / 2.0f);

	out[0][0] = f / aspect;
	out[1][1] = f;
	out[2][2] = (far + near) / (near  - far);
	out[2][3] = -1.0f;
	out[3][2] = (2.0f * far * near) / (near - far);

	return out;
}

MinVR::VRMatrix4 mvImageApp::lookat(MinVR::VRVector3 cameraPos,
                                    MinVR::VRVector3 targetPos,
                                    MinVR::VRVector3 cameraUp) {

  // Normalize and straighten out the input directions.
  MinVR::VRVector3 forward = (targetPos - cameraPos).normalize();
  MinVR::VRVector3 up = cameraUp.normalize();
  MinVR::VRVector3 side = (forward.cross(up)).normalize();
  up = side.cross(forward);

  // Insert them into a view matrix.
  MinVR::VRMatrix4 out;
  out[0][0] = side.x;
  out[1][0] = side.y;
  out[2][0] = side.z;
  
  out[0][1] = up.x;
  out[1][1] = up.y;
  out[2][1] = up.z;
  
  out[0][2] = -forward.x;
  out[1][2] = -forward.y;
  out[2][2] = -forward.z;

  out[3][0] = -side.dot(cameraPos);
  out[3][1] = -up.dot(cameraPos);
  out[3][2] = -forward.dot(cameraPos);
  
  return out;
}

void mvImageApp::onVREvent(const std::string &eventName, MinVR::VRDataIndex *eventData) {

  std::cout << "Event: " << eventName << std::endl;                    
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
  std::cout << "Got a CGLGetCurrentContext(): " << CGLGetCurrentContext() << std::endl;
  // If this is the first time through, do the graphics initialization.
  if (_needInitialization) {
    graphicsInit(_vrMain->getConfig());
    _needInitialization = false;
  }
  
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // For a little animation effect, calculate the number of seconds
    // since the program started.
    float secondsElapsed = getElapsedSeconds();
    
    _gLightRotation += (M_PI / 4.0f) * secondsElapsed;
    //    std::cout << "rotation: " << _gLightRotation << std::endl;
    for(int i = 0; i < NUM_LIGHTS; ++i) {
      const float radius = 1.75f;
      float r = (((M_PI * 2.0f) / (float)NUM_LIGHTS) * (float)i) + _gLightRotation;

      _gLightPosition[i * 3 + 0] = cosf(r) * radius;
      _gLightPosition[i * 3 + 1] = cosf(r) * sinf(r);
      _gLightPosition[i * 3 + 2] = sinf(r) * radius;
    }

    // We are ready to execute our shader program...
    std::cout << "using program: " << _gProgram << std::endl;
    glUseProgram(_gProgram);
    
    if (renderState->exists("ProjectionMatrix", "/")) {
      // This is the typical case where the MinVR DisplayGraph contains      
      // an OffAxisProjectionNode or similar node, which sets the            
      // ProjectionMatrix and ViewMatrix based on head tracking data.        

      std::cout << "projection matrix exists" << std::endl;
      
      MinVR::VRMatrix4 projMat = renderState->getValue("ProjectionMatrix", "/");

      // Load it for access by the shader.
      glUniformMatrix4fv(_gProgramProjMatrix, 1, GL_FALSE, projMat.getFloat());

      // In a VR program, we want the camera (Projection and View
      // matrices) to be controlled by head tracking.  
      // Guideline: In VR, put all of the "scene navigation" into the
      // Model matrix and leave the Projection and View matrices for
      // head tracking.

      MinVR::VRMatrix4 viewMat = renderState->getValue("ViewMatrix", "/");

      // Load it for access by the shader.
      glUniformMatrix4fv(_gProgramViewMatrix, 1, GL_FALSE, viewMat.getFloat());

    } else {

      std::cout << "no projection matrix" << std::endl;
      
      // If the DisplayGraph does not contain a node that sets the
      // ProjectionMatrix and ViewMatrix, then we must be in a
      // non-headtracked simple desktop mode.  We can just set the
      // projection and view matrices.

      // Make a projection matrix with the perspective function above.
      MinVR::VRMatrix4 perspMat = perspective(M_PI / 4.0f, 1.f, 0.1f, 100.0f);

      // Load it for access by the shader.
      glUniformMatrix4fv(_gProgramProjMatrix, 1, GL_FALSE, perspMat.getFloat());

      // Just checking...
      GLint count;
      glGetProgramiv(_gProgram, GL_ACTIVE_UNIFORMS, &count);
      std::cout << "**Active (in use by a shader) Uniforms: " << count << std::endl;

      GLint params[4];
      glGetActiveUniformsiv(_gProgram, 1, &_gProgramProjMatrix,
                            GL_UNIFORM_TYPE, params);
      std::cout << "params: " << params[0] << std::endl;
      
      
      // Construct a view matrix, using lookat.
      _gCameraPos.x = _radius * cos(_horizAngle) * cos(_vertAngle);
      _gCameraPos.y = -_radius * sin(_vertAngle);
      _gCameraPos.z = _radius * sin(_horizAngle) * cos(_vertAngle);

      _gCameraUp.x = cos(_horizAngle) * sin(_vertAngle);
      _gCameraUp.y = cos(_vertAngle);
      _gCameraUp.z = sin(_horizAngle) * sin(_vertAngle);

      MinVR::VRMatrix4 viewMat = lookat(_gCameraPos,
                                        MinVR::VRVector3(0.0f, 0.0f, 0.0f),
                                        _gCameraUp);

      // Load it for access by the shader.
      glUniform3fv(_gProgramCameraPositionLocation, 1, _gCameraPos.getFloat());
      glUniformMatrix4fv(_gProgramViewMatrix, 1, GL_FALSE, viewMat.getFloat());

    }

    glUniform3fv(_gProgramLightPositionLocation, NUM_LIGHTS, _gLightPosition);
    glUniform3fv(_gProgramLightColorLocation, NUM_LIGHTS, _gLightColor);

    // Render something for each light.
    char name[10];
    for(int i = 0; i < NUM_LIGHTS; ++i) {
      // Render a shape with the light's color and position.
      sprintf(name, "sphere%d", i);
      getImage(std::string(name))->getShape()->setPosition(_gLightPosition[i*3+0],
                                                           _gLightPosition[i*3+1],
                                                           _gLightPosition[i*3+2]);
 
    }

    // Let's see the whole configuration.
    std::cout << "APP:*****" << std::endl << (*this) << "*******" << std::endl;


    // Debug stuff.
    GLint i;
    GLint count;

    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 16; // maximum name length
    GLchar vname[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    // glGetProgramiv(_gProgram, GL_ACTIVE_ATTRIBUTES, &count);
    // printf("Active Attributes: %d\n", count);

    // for (i = 0; i < count; i++) {
    //   glGetActiveAttrib(_gProgram, (GLuint)i, bufSize, &length, &size, &type, vname);

    //   printf("Attribute #%d Type: %u Name: %s\n", i, type, vname);
    // }

    glGetProgramiv(_gProgram, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    for (i = 0; i < count; i++) {
      glGetActiveUniform(_gProgram, (GLuint)i, bufSize, &length, &size, &type, vname);

      printf("Uniform #%d Type: %u Name: %s\n", i, type, vname);
    }


    
    // GLint numActiveAttribs = 0;
    // GLint numActiveUniforms = 0;
    // glGetProgramInterfaceiv(prog, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numActiveAttribs);
    // glGetProgramInterfaceiv(prog, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numActiveUniforms);

    // std::vector<GLchar> nameData(256);
    // std::vector<GLenum> properties;
    // properties.push_back(GL_NAME_LENGTH​);
    // properties.push_back(GL_TYPE​);
    // properties.push_back(GL_ARRAY_SIZE​);
    // std::vector<GLint> values(properties.size());
    // for(int attrib = 0; attrib < numActiveAttribs; ++attrib) {
    //   glGetProgramResourceiv(prog, GL_PROGRAM_INPUT, attrib, properties.size(),
    //                          &properties[0], values.size(), NULL, &values[0]);

    //   nameData.resize(properties[0]); //The length of the name.
    //   glGetProgramResourceName(prog, GL_PROGRAM_INPUT, attrib, nameData.size(), NULL, &nameData[0]);
    //   std::string name((char*)&nameData[0], nameData.size() - 1);
    // }





   
   
    // Draw objects here.
    std::cout << "draw objects" << std::endl;
    for (imageMap::iterator it = _images.begin();
         it != _images.end(); it++) {
      std::cout << "drawing: " << it->first << std::endl;

      // Retrieve the model matrix from the object about to be drawn.
      MinVR::VRMatrix4 model = it->second->getShape()->getModelMatrix();

      // Load it for access by the shader.
      glUniformMatrix4fv(_gProgramModelMatrix, 1, GL_FALSE, model.getFloat());

      // Draw it.
      it->second->draw();
    }
    
    std::cout << "Draw some axes because why not." << std::endl;
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


    // glPopMatrix();
  }
}

std::string mvImageApp::print() const {
  std::stringstream out;

  out << "Program: " << _gProgram << std::endl;
  out << "Camera: (" << _gCameraPos[0] << ","  << _gCameraPos[1] << ","  << _gCameraPos[2] << ")" << std::endl;
  out << "Light 1: (" << _gLightPosition[0] << ","  << _gLightPosition[1] << ","  << _gLightPosition[2] << ")" << std::endl;
  out << "  color: (" << _gLightColor[0] << ","  << _gLightColor[1] << ","  << _gLightColor[2] << ")" << std::endl;
  out << "Light 2: (" << _gLightPosition[3] << ","  << _gLightPosition[4] << ","  << _gLightPosition[5] << ")" << std::endl;  
  out << "  color: (" << _gLightColor[3] << ","  << _gLightColor[4] << ","  << _gLightColor[5] << ")" << std::endl;
  out << "Light 3: (" << _gLightPosition[6] << ","  << _gLightPosition[7] << ","  << _gLightPosition[8] << ")" << std::endl;  
  out << "  color: (" << _gLightColor[6] << ","  << _gLightColor[7] << ","  << _gLightColor[8] << ")" << std::endl;
  
  for (imageMap::const_iterator it = _images.begin(); it != _images.end(); it++) {

    out << it->first << std::endl;
    out << it->second->print();

  }
  
  return out.str();
}



std::ostream & operator<<(std::ostream &os, const mvImageApp& app) {
  return os << app.print();
}



int main(int argc, char **argv) {

  mvImageApp app(argc, argv);

  // Let's see the whole configuration.  It is not complete, since
  // graphicsInit() waits for the first render to execute.
  std::cout << "IMAGES:*****" << std::endl << app << "*******" << std::endl;
 
  app.run();

  exit(0);  
}

