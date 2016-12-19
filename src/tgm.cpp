#include <iostream>
#include <vector>
#include <list>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

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
#include "MVR.h"

class ImageToDisplay {
public:
  std::string fileName;
  float x, y, z;
  float height, width;

  ImageToDisplay(std::string f, float xx, float yy, float zz, float h, float w) :
    fileName(f), x(xx), y(yy), z(zz), height(h), width(w) {};
}; 
  
class VRApp : public MinVR::VREventHandler, public MinVR::VRRenderHandler {
private:
  MinVR::VRMain* _vrMain;
  bool _quit;
  float _horizAngle, _vertAngle, _radius, _incAngle;
  bool _initialized;

  std::list<ImageToDisplay> _images;
  
public:
  
  std::list<mvShape*> _shapeList;
  std::list<mvShaders*> _shaderList;
  std::list<mvLights*> _lightList;

  mvShapeFactory _shapeFactory;

  GLuint _lightPositionID;
  GLuint _lightColorID;
  

  
  VRApp(int argc, char** argv, std::list<ImageToDisplay> images) :
    _initialized(false), _images(images) {

    _vrMain = new MinVR::VRMain();
    std::string configFile = argv[1];
    _vrMain->initialize(argc, argv, configFile);

    _vrMain->addEventHandler(this);
    _vrMain->addRenderHandler(this);
    _horizAngle = 0.0;
    _vertAngle = 0.0;
		_radius =  15.0;
    _incAngle = -0.1f;

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
    // glfwTerminate();
    _vrMain->shutdown();
    delete _vrMain;
  };

  virtual void onVREvent(const std::string &eventName,
                         MinVR::VRDataIndex *eventData) {
		if (eventName == "/KbdEsc_Down") {
			_quit = true;
		} else if (eventName == "/MouseBtnLeft_Down") {
      _radius += 5.0 * _incAngle;
    } else if (eventName == "/MouseBtnRight_Down") {
      _radius -= 5.0 * _incAngle;
    } else if ((eventName == "/KbdLeft_Down") ||
               (eventName == "/KbdLeft_Repeat")) {
      _horizAngle -= _incAngle;
    } else if ((eventName == "/KbdRight_Down") ||
               (eventName == "/KbdRight_Repeat")) {
      _horizAngle += _incAngle;
    } else if ((eventName == "/KbdUp_Down") ||
               (eventName == "/KbdUp_Repeat")) {
      _vertAngle -= _incAngle;
    } else if ((eventName == "/KbdDown_Down") ||
               (eventName == "/KbdDown_Repeat")) {
      _vertAngle += _incAngle;
    }
      
    if (_horizAngle > 6.283185) _horizAngle -= 6.283185;
    if (_horizAngle < 0.0) _horizAngle += 6.283185;
      
    if (_vertAngle > 6.283185) _vertAngle -= 6.283185;
    if (_vertAngle < 0.0) _vertAngle += 6.283185;
	}

  // void checkEvents() {
  //   control.handleEvents(_window);
  //   control.computeMatricesFromInputs(_window);
	// 	glfwPollEvents();
  // }

  void onVRRenderContext(MinVR::VRDataIndex *renderState,
                         MinVR::VRDisplayNode *callingNode) {

    if (!_initialized) {

      std::cout << "renderContext is here" << std::endl;
      //std::cout << "renderState->.." << (int)renderState->getValue("InitRender") << std::endl;
      std::cout << "renderState->.." << std::endl << renderState->printStructure() << std::endl;

      
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
    
      for (std::list<ImageToDisplay>::iterator it = _images.begin();
           it != _images.end(); it++) {
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

      _initialized = true;
    }
  }

  void draw(MMat4 ViewMatrix, MMat4 ProjectionMatrix) {

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
		//glfwSwapBuffers(_window);
  };

  virtual void onVRRenderScene(MinVR::VRDataIndex *renderState,
                               MinVR::VRDisplayNode *callingNode) {

    if (renderState->exists("IsConsole", "/")) {
      MinVR::VRConsoleNode *console =
        dynamic_cast<MinVR::VRConsoleNode*>(callingNode);
			console->println("Console output...");

		} else {

      MMat4 ProjectionMatrix;
      MMat4 ViewMatrix;
      
      if (renderState->exists("ProjectionMatrix", "/")) {
        MinVR::VRDoubleArray p = renderState->getValue("ProjectionMatrix", "/");
        ProjectionMatrix = MMat4(p[0]);
        MinVR::VRDoubleArray v = renderState->getValue("ViewMatrix", "/");
        ViewMatrix = MMat4(v[0]);

        // Combine that with an ad hoc model matrix.
        ViewMatrix = ViewMatrix *
          glm::translate(MMat4(1.0f), MVec3(0.0,0.0,-_radius)) *
          glm::rotate(MMat4(1.0f), _vertAngle, MVec3(1.0, 0.0, 0.0)) *
          glm::rotate(MMat4(1.0f), _horizAngle, MVec3(0.0, 1.0, 0.0));
        
      } else {
        ProjectionMatrix = MMat4(1.0);

        MVec3 pos = MVec3(_radius * cos(_horizAngle) * cos(_vertAngle),
                          -_radius * sin(_vertAngle),
                          _radius * sin(_horizAngle) * cos(_vertAngle));
        MVec3 up = MVec3(cos(_horizAngle) * sin(_vertAngle),
                         cos(_vertAngle),
                         sin(_horizAngle) * sin(_vertAngle));
        MVec3 target = MVec3(0.0f, 0.0f, 0.0f);
        
        ViewMatrix = glm::lookAt(pos, target, up);

      }

      draw(ViewMatrix, ProjectionMatrix);
    }
  }

  void run() {

    while (!_quit) {
      _vrMain->mainloop();
    }
  }

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
  
  VRApp app(argc, argv, images);

  app.run();

  exit(0);
  
	// do{
  //   app.checkEvents();
    
  //   app.draw();
    

	// } // Check if the ESC key was pressed or the window was closed
	// while( glfwGetKey(app._window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	// 	   glfwWindowShouldClose(app._window) == 0 );

	// return 0;
}
