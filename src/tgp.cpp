#include <iostream>
#include <vector>
#include <list>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.h>
#include <common/texture.h>
#include <common/controls.h>
#include <common/objloader.h>

#include "mvShape.h"

class VRApp {
public:
  GLFWwindow* _window;

  VRControl control;

  std::list<mvShape*> _shapeList;
  std::list<mvShaders*> _shaderList;
  std::list<mvLights*> _lightList;

  mvShapeFactory _shapeFactory;

  GLuint _lightPositionID;
  GLuint _lightColorID;
  
  // Put all the GLFW setup business here.
  void setupWin() {
    // Initialise GLFW
    if( !glfwInit() ) {
      throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    _window = glfwCreateWindow( 1024, 768, "tgl", NULL, NULL);
    if( _window == NULL ){
      throw std::runtime_error("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible." );
      glfwTerminate();
    for (std::list<mvShaders*>::iterator it = _shaderList.begin();
         it != _shaderList.end(); it++) {
      glDeleteProgram((*it)->getProgramID());
      delete *it;
    }

    }
    glfwMakeContextCurrent(_window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(_window, 1024/2, 768/2);
  };

  
  VRApp() {
    control = VRControl();
    setupWin();

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("Failed to initialize GLEW");
      glfwTerminate();
    }
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 
    // Cull triangles whose normal is not towards the camera.
    glEnable(GL_CULL_FACE);


    //////////////////////////////////////////////////////////
    // Make some lights
    mvLights* lights = new mvLights();

    lights->addLight(glm::vec3(4.0, 4.0, 4.0), glm::vec3(1.0, 1.0, 1.0));
    lights->addLight(glm::vec3(-4.0, 4.0, 4.0), glm::vec3(0.0, 0.1, 0.1));

    _lightList.push_back(lights);
      
    //////////////////////////////////////////////////////////
    // Create and compile our GLSL program from the shaders
    mvShaders* shaders = new mvShaders("../src/StandardShading.vertexshader", "",
                                       "../src/StandardShading.fragmentshader",
                                       lights);
    _shaderList.push_back(shaders);
    
    // Switch to axes.  These use the default shader, which you get
    // by initializing the shader object with no args.
    mvShaders* axisShaders = new mvShaders();
    _shaderList.push_back(axisShaders);
    
    //////////////////////////////////////////////////////////
    // Create objects to display
    mvShape* axes = _shapeFactory.createShape(shapeAXES, axisShaders);
    _shapeList.push_back(axes);
    
    mvShape* suzanne = _shapeFactory.createShape(shapeOBJ, shaders);

    //    ((mvShapeObj*)suzanne)->setObjFile("suzanne.obj");
    ((mvShapeObj*)suzanne)->setObjFile("../data/office-test.obj");
    
    // Load the texture
    int w, h;
    GLuint textureBufferID = loadPNG("../data/office-test.png", &w, &h);
    //    GLuint textureBufferID = loadDDS("uvmap.DDS");
    suzanne->setTextureID(textureBufferID);

    suzanne->setPosition(MVec3(0.0, 0.0, -18.0));

    mvShape* rect = _shapeFactory.createShape(shapeRECT, shaders);
    // Load the texture
    int width, height;
    textureBufferID = loadPNG("/Users/tomfool/Desktop/on-the-roof.png",
                              &width, &height);
    rect->setTextureID(textureBufferID);
    rect->setDimensions(2.5, 2.5 * ((float)height / (float)width));

    mvShape* rect2 = _shapeFactory.createShape(shapeRECT, shaders);
    // Load the texture
    textureBufferID = loadPNG("e.png", &width, &height);
    rect2->setTextureID(textureBufferID);
    rect2->setDimensions(1.5, 1.5 * ((float)height / (float)width));
    rect2->setPosition(glm::vec3(0.5, 1.0, 0.7));
    rect2->setRotation(glm::vec3(.7,.5,.3));

    _shapeList.push_back(suzanne);
    _shapeList.push_back(rect);
    _shapeList.push_back(rect2);


    // Execute "load()" for all the shaders.  This basically registers
    // them, registers their lighting data, and gets them ready to
    // operate.
    for (std::list<mvShaders*>::iterator it = _shaderList.begin();
         it != _shaderList.end(); it++)         
      (*it)->load();
    
    // Load all the shapes.  Initializes whatever needs to be
    // initialized, etc.    
    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) (*it)->load();

  };

  ~VRApp() {

    for (std::list<mvLights*>::iterator it = _lightList.begin();
         it != _lightList.end(); it++) {
      delete *it;
    }

    for (std::list<mvShaders*>::iterator it = _shaderList.begin();
         it != _shaderList.end(); it++) {
      glDeleteProgram((*it)->getProgramID());
      delete *it;
    }

    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) {
      delete *it;
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
  };

  void checkEvents() {
    control.handleEvents(_window);
    control.computeMatricesFromInputs(_window);
		glfwPollEvents();
  }
  
  void draw() {

    // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MMat4 ViewMatrix = control.getViewMatrix();
    MMat4 ProjectionMatrix = control.getProjectionMatrix();

    // Have any lights moved or changed color?  They get adjusted inside
    // this draw call if they have.
    for (std::list<mvShaders*>::iterator it = _shaderList.begin();
         it != _shaderList.end(); it++)         
      (*it)->draw();

    // Now draw the objects.
    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) {
      (*it)->draw(ViewMatrix, ProjectionMatrix);
    }
    
		// Swap buffers
		glfwSwapBuffers(_window);
  };

};

int main( void )
{
  VRApp app = VRApp();
  
	do{
    app.checkEvents();
    
    app.draw();
    

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(app._window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(app._window) == 0 );

	return 0;
}
