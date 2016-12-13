#include <iostream>
#include <vector>
#include <list>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vecTypes.h"
#include <common/shader.h>
#include <common/texture.h>
#include <common/controls.h>
#include <common/objloader.h>

#include "mvShape.h"
#include "tinyxml2.h"

class ImageToDisplay {
public:
  std::string fileName;
  float x, y, z;
  float height, width;

  ImageToDisplay(std::string f, float xx, float yy, float zz, float h, float w) :
    fileName(f), x(xx), y(yy), z(zz), height(h), width(w) {};
}; 
  
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

  
  VRApp(std::list<ImageToDisplay> images) {
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

    // Enable depth test for opaque figures
    //glEnable(GL_DEPTH_TEST);
    // Disable depth test and enable blend if there is transparency.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 
    // Cull triangles whose normal is not towards the camera.
    glEnable(GL_CULL_FACE);


    //////////////////////////////////////////////////////////
    // Make some lights
    mvLights* lights = new mvLights();

    lights->addLight(MVec3(14.0, 14.0, 14.0), MVec3(1.0, 1.0, 1.0));
    lights->addLight(MVec3(-4.0, 4.0, 4.0), MVec3(0.0, 0.1, 0.1));

    _lightList.push_back(lights);
      
    //////////////////////////////////////////////////////////
    // Create and compile our GLSL program from the shaders
    mvShaders* shaders = new mvShaders("../src/StandardShading.vertexshader", "",
                                       "../src/PlanktonShading.fragmentshader",
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

    int width, height;
    
    for (std::list<ImageToDisplay>::iterator it = images.begin();
         it != images.end(); it++) {
      _shapeList.push_back(_shapeFactory.createShape(shapeRECT, shaders));

      _shapeList.back()->setTextureID(loadPNG((it->fileName).c_str(),
                                              &width, &height));
      _shapeList.back()->setDimensions(it->width/100.0, it->height/100.0);
      _shapeList.back()->setPosition(it->x/100.0, it->y/100.0, it->z/5000.0);
    }

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

    // Have any lights moved or changed color?  They get adjusted inside
    // this draw call if they have.
    for (std::list<mvShaders*>::iterator it = _shaderList.begin();
         it != _shaderList.end(); it++)         
      (*it)->draw();

    // Now draw the objects.
    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) {
      (*it)->draw(control);
    }
    
		// Swap buffers
		glfwSwapBuffers(_window);
  };

};


int main( int argc, char **argv )
{

  std::list<ImageToDisplay> images;
  
  tinyxml2::XMLDocument doc;
  std::string reportName =
    std::string("../EN_581_cast_7__15-Jun-2016_01-07-21-715.bmp/report.xml");
	//std::string reportName = std::string(argv[1]);
  std::string pathName = reportName.substr(0, reportName.find_last_of("/"));
  std::cout << "opening: " << reportName << std::endl;
  std::cout << "found in:" << pathName << std::endl;

  int d = doc.LoadFile(reportName.c_str()); 
	if (d != tinyxml2::XML_SUCCESS) {
    std::cout << "ouch:" << doc.ErrorName() << std::endl;
  } else {

		tinyxml2::XMLElement* root = doc.FirstChildElement();
    tinyxml2::XMLElement* data = root->FirstChildElement("DATA");

		for (tinyxml2::XMLElement* e = data->FirstChildElement("ROI");
         e != NULL; e = e->NextSiblingElement("ROI")) {

      std::cout << "IMAGE: " << e->FirstChildElement("IMAGE")->GetText() << std::endl;
      std::cout << "X:     " << e->FirstChildElement("X")->GetText() << std::endl;
      std::cout << "Y:     " << e->FirstChildElement("Y")->GetText() << std::endl;
      std::cout << "Z:     " << e->FirstChildElement("DEPTH")->GetText() << std::endl;
      std::cout << "HEIGHT:" << e->FirstChildElement("HEIGHT")->GetText() << std::endl;
      std::cout << "WIDTH: " << e->FirstChildElement("WIDTH")->GetText() << std::endl;
      std::string pngName = pathName + std::string("/") +
        std::string(e->FirstChildElement("IMAGE")->GetText()); 
      images.push_back(ImageToDisplay(pngName,
                                      strtof(e->FirstChildElement("X")->GetText(),NULL),
                                      strtof(e->FirstChildElement("Y")->GetText(),NULL),
                                      strtof(e->FirstChildElement("DEPTH")->GetText(),NULL),
                                      strtof(e->FirstChildElement("HEIGHT")->GetText(),NULL),
                                      strtof(e->FirstChildElement("WIDTH")->GetText(),NULL)));

		}
	}
  
  VRApp app = VRApp(images);
  
	do{
    app.checkEvents();
    
    app.draw();
    

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(app._window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(app._window) == 0 );

	return 0;
}
