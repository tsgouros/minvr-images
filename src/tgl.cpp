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
    // Create and compile our GLSL program from the shaders
    mvShaders shaders = mvShaders("StandardShading.vertexshader", "",
                                  "StandardShading.fragmentshader");

    // Switch to axes.  These use the default shader, which you get
    // by initializing the shader object with no args.
    mvShaders axisShaders = mvShaders();

    mvShapeObj* suzanne = new mvShapeObj(shaders.getProgram());
    mvShapeAxes* axes = new mvShapeAxes(axisShaders.getProgram());
    
    _shapeList.push_back((mvShape*)suzanne);
    _shapeList.push_back((mvShape*)axes);

    // Load all the shapes.
    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) (*it)->load();

  };

  ~VRApp() {

    // TBD: We can't delete these in the mvShape object because more than one
    // shape  might be using them.
    //    glDeleteProgram(_programID);
    //glDeleteProgram(_axisProgramID);

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

    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) (*it)->draw(control);

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
