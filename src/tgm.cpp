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
#include "shader.h"
#include "texture.h"
#include "objloader.h"

#include "mvShape.h"
#include "tinyxml2.h"
#include "MVR.h"

// This class holds information about the image of plankton we're going to show.
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
  float _horizAngle, _vertAngle, _stepAngle;
  float _xpos, _ypos, _zpos, _stepDist;
  bool _initialized;

  std::list<ImageToDisplay> _images;
  
public:
  
  std::list<mvShape*> _shapeList;
  mvShapeFactory _shapeFactory;

  // We keep track of these because multiple shapes can use the same
  // shaders and multiple shaders can use the same lights.  So they
  // have to be tracked outside their meager little boundaries,
  // i.e. right here.
  std::list<mvShaderSet*> _shaderList;
  std::list<mvLights*> _lightList;
  
  VRApp(int argc, char** argv, std::list<ImageToDisplay> images) :
    _initialized(false), _quit(false), _images(images) {

    _vrMain = new MinVR::VRMain();
    std::string configFile = argv[1];
    _vrMain->initialize(argc, argv, configFile);

    _vrMain->addEventHandler(this);
    _vrMain->addRenderHandler(this);

    _horizAngle = 0.4; _vertAngle = 0.0; _stepAngle = -0.05f;
    _xpos = -7.0; _ypos = 0.0; _zpos = -18.0; _stepDist = 0.2f;


    _vrMain->getConfig()->addData("/HeadLocation/PosX", _xpos);
    _vrMain->getConfig()->addData("/HeadLocation/PosY", _ypos);
    _vrMain->getConfig()->addData("/HeadLocation/PosZ", _zpos);
    _vrMain->getConfig()->addData("/HeadLocation/HorizAngle", _horizAngle);
    _vrMain->getConfig()->addData("/HeadLocation/VertAngle", _vertAngle);

  };

  ~VRApp() {

    for (std::list<mvLights*>::iterator it = _lightList.begin();
         it != _lightList.end(); it++) {
      delete *it;
    }

    for (std::list<mvShaderSet*>::iterator it = _shaderList.begin();
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


  // Maybe what this should do is to accept the keyboard commands and
  // issue another event with Transform in it?  This would be
  // irrelevant during cave runs, wouldn't it?

  
  virtual void onVREvent(const std::string &eventName,
                         MinVR::VRDataIndex *eventData) {
    // std::cout << "event data" << std::endl << eventData->printStructure() << std::endl;

    // These are here to manipulate the view matrix in the event that
    // we're running this on the desktop.  When running in real VR,
    // the view matrix should be provided in the render state.

    //std::cout << "event name: " << eventName << std::endl;
    
		if (eventName == "/KbdEsc_Down") {
			_quit = true;
		} else if (eventName == "/MouseBtnLeft_Down") {
      _ypos += _stepDist;
    } else if (eventName == "/MouseBtnRight_Down") {
      _ypos -= _stepDist;
    } else if ((eventName == "/KbdLeft_Down") ||
               (eventName == "/KbdLeft_Repeat")) {
      _horizAngle -= _stepAngle;
    } else if ((eventName == "/KbdRight_Down") ||
               (eventName == "/KbdRight_Repeat")) {
      _horizAngle += _stepAngle;
    } else if ((eventName == "/KbdUp_Down") ||
               (eventName == "/KbdUp_Repeat")) {
      _vertAngle -= _stepAngle;
    } else if ((eventName == "/KbdDown_Down") ||
               (eventName == "/KbdDown_Repeat")) {
      _vertAngle += _stepAngle;
    } else if ((eventName == "/KbdA_Down") || (eventName == "/Kbda_Down") ||
               (eventName == "/KbdA_Repeat")) {

      // Just a jump to the left.
      _xpos += _stepDist * cos(_horizAngle);
      _zpos += _stepDist * sin(_horizAngle);

    } else if ((eventName == "/KbdD_Down") || (eventName == "/Kbdd_Down") ||
               (eventName == "/KbdD_Repeat")) {
      // Then a step to the right.
      _xpos -= _stepDist * cos(_horizAngle);
      _zpos -= _stepDist * sin(_horizAngle);
      // (Then pull your knees in tight.)
    } else if ((eventName == "/KbdW_Down") || (eventName == "/Kbdw_Down") ||
               (eventName == "/KbdW_Repeat")) {
      // Move a step forward.
      _xpos += _stepDist * sin(_horizAngle) * cos(_vertAngle);
      _ypos += _stepDist * sin(_vertAngle);
      _zpos += _stepDist * cos(_horizAngle) * cos(_vertAngle);

    } else if ((eventName == "/KbdS_Down") || (eventName == "/Kbds_Down") ||
               (eventName == "/KbdS_Repeat")) {
      // Move a step backward.
      _xpos -= _stepDist * sin(_horizAngle) * cos(_vertAngle);
      _ypos -= _stepDist * sin(_vertAngle);
      _zpos -= _stepDist * cos(_horizAngle) * cos(_vertAngle);

    } else if ((eventName == "/KbdZ_Down") || (eventName == "/Kbdz_Down") ||
               (eventName == "/KbdZ_Repeat")) {
      _ypos += _stepDist;

    } else if ((eventName == "/KbdX_Down") || (eventName == "/Kbdx_Down") ||
               (eventName == "/KbdX_Repeat")) {
      _ypos -= _stepDist;
    }
      
    if (_horizAngle > 6.283185) _horizAngle -= 6.283185;
    if (_horizAngle < 0.0) _horizAngle += 6.283185;
      
    if (_vertAngle > 6.283185) _vertAngle -= 6.283185;
    if (_vertAngle < 0.0) _vertAngle += 6.283185;

    eventData->addData("/HeadLocation/PosX", _xpos);
    eventData->addData("/HeadLocation/PosY", _ypos);
    eventData->addData("/HeadLocation/PosZ", _zpos);
    eventData->addData("/HeadLocation/HorizAngle", _horizAngle);
    eventData->addData("/HeadLocation/VertAngle", _vertAngle);
    
	}

  // void checkEvents() {
  //   control.handleEvents(_window);
  //   control.computeMatricesFromInputs(_window);
	// 	glfwPollEvents();
  // }

  void onVRRenderContext(MinVR::VRDataIndex *renderState,
                         MinVR::VRDisplayNode *callingNode) {

    if ((int)renderState->getValue("/InitRender") == 1) {

      // Initialize GLEW
      glewExperimental = true; // Needed for core profile
      GLenum err = glewInit();
      if (err != GLEW_OK) {
        /* Problem: glewInit failed, something is seriously wrong. */
        throw std::runtime_error( "Error: " +
                                  std::string((char*)glewGetErrorString(err)));
      }

      GLenum error = glGetError();

      if (error != GL_NO_ERROR) {
        // There is a 1280 (bad enum) error that is apparently triggered by
        // glewInit(). It seems not to be a problem.
        std::cout << "OpenGL Error: " << error << std::endl;
      } else std::cout << "No OpenGL problem." << std::endl;
      
      // Dark blue background
      glClearColor(0.1f, 0.0f, 0.4f, 0.0f);

      // Enable depth test for opaque figures
      glEnable(GL_DEPTH_TEST);
      // Disable depth test and enable blend if there is transparency.
      // glDisable(GL_DEPTH_TEST);
      // glEnable(GL_BLEND);
      // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      // Accept fragment if it closer to the camera than the former one
      glDepthFunc(GL_LESS); 
      // Cull triangles whose normal is not towards the camera.
      glEnable(GL_CULL_FACE);

      error = glGetError();

      if (error != GL_NO_ERROR) {
        std::cout << "OpenGL Error: " << error << std::endl;
      } //else std::cout << "all clear on the OpenGL front" << std::endl;
      
     
      //////////////////////////////////////////////////////////
      // Make some lights
      mvLights* lights = new mvLights();

      lights->addLight(MVec3(14.0, 14.0, 14.0), MVec3(1.0, 1.0, 1.0));
      lights->addLight(MVec3(-4.0, 4.0, 4.0), MVec3(0.0, 0.1, 0.1));
      lights->addLight(MVec3(-7.0, 0.0, -18.0), MVec3(1.0, 1.0, 1.0));

      _lightList.push_back(lights);
      
      //////////////////////////////////////////////////////////
      // Create and compile our GLSL program from the shaders
      mvShaderSet* shaders =
        new mvShaderSet("../src/StandardShading.vertexshader",
                        "",
                        "../src/PlanktonShading.fragmentshader",
                        lights);
      _shaderList.push_back(shaders);
    
      // Switch to axes.  These use the default shader, which you get
      // by initializing the shader object with no args.
      mvShaderSet* axisShaders = new mvShaderSet();
      _shaderList.push_back(axisShaders);
    
      //////////////////////////////////////////////////////////
      // Create objects to display
      // mvShape* axes = _shapeFactory.createShape(shapeAXES, axisShaders);
      // _shapeList.push_back(axes);

      int width, height;
    
      for (std::list<ImageToDisplay>::iterator it = _images.begin();
           it != _images.end(); it++) {
        //        if (it->width < 100.0) continue;
        
        // Add the appropriate shader and texture to this object.
        mvTexture* tex = new mvTexture(texturePNG, it->fileName);

        // Create a rectangle with the new texture and our favorite shader.
        _shapeList.push_back(_shapeFactory.createShape(shapeRECT, shaders, tex));

        width = tex->getWidth();
        height = tex->getHeight();

        // Size the object and place it in the scene.
        _shapeList.back()->setDimensions(it->width/100.0, it->height/100.0);
        _shapeList.back()->setPosition(it->x/100.0, it->y/100.0, it->z/5000.0);
      }

      // mvTexture* officeTex = new mvTexture(texturePNG, "../data/office-test.png");
      
      // mvShape* suzanne = _shapeFactory.createShape(shapeOBJ, shaders, officeTex);
      // ((mvShapeObj*)suzanne)->setObjFile("suzanne.obj");
      // //((mvShapeObj*)suzanne)->setObjFile("../data/office-test.obj");
      // suzanne->setPosition(MVec3(0.0, 0.0, -18.0));
      // _shapeList.push_back(suzanne);

      
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

    // Now draw the objects.
    for (std::list<mvShape*>::iterator it = _shapeList.begin();
         it != _shapeList.end(); it++) {
      (*it)->draw(ViewMatrix, ProjectionMatrix);
    }
  };

  virtual void onVRRenderScene(MinVR::VRDataIndex *renderState,
                               MinVR::VRDisplayNode *callingNode) {

    MMat4 ProjectionMatrix;
    MMat4 ViewMatrix;

    // MinVR matrices are stored in row-major order so they will be
    // easy to read as text (which is dumb).  OpenGL uses column-major
    // order, so these must be transposed before sending them to
    // OpenGL.
    // std::cout << renderState->printStructure() << std::endl;
    MinVR::VRDoubleArray p = renderState->getValue("ProjectionMatrix", "/");
    ProjectionMatrix = MMat4(p[0],p[4],p[8],p[12],p[1],p[5],p[9],p[13],
                             p[2],p[6],p[10],p[14],p[3],p[7],p[11],p[15]);

    if (renderState->exists("/IsConsole")) {

      // If we're not on the desktop, this must be real VR, in which case
      // the view matrix should be in the render state.
      MinVR::VRDoubleArray v = renderState->getValue("ViewMatrix", "/");
      ViewMatrix = MMat4(v[0],v[4],v[8],v[12],v[1],v[5],v[9],v[13],
                         v[2],v[6],v[10],v[14],v[3],v[7],v[11],v[15]);
      
    } else {

      // If we're operating from the desktop, use the keyboard-controlled
      // position and orientation values to fake a view matrix.
      double xpos = _vrMain->getConfig()->getValue("/HeadLocation/PosX");
      double ypos = _vrMain->getConfig()->getValue("/HeadLocation/PosY");
      double zpos = _vrMain->getConfig()->getValue("/HeadLocation/PosZ");
      double hangle = _vrMain->getConfig()->getValue("/HeadLocation/HorizAngle");
      double vangle = _vrMain->getConfig()->getValue("/HeadLocation/VertAngle");

      MVec3 pos = MVec3(xpos, ypos, zpos);
      MVec3 up = MVec3(cos(hangle) * sin(vangle),
                       cos(vangle),
                       sin(hangle) * sin(vangle));
      MVec3 dir = MVec3(cos(vangle) * sin(hangle), 
                        sin(vangle),
                        cos(vangle) * cos(hangle));
      // std::cout << "xpos: " << xpos << " ypos: " << ypos << " zpos: " << zpos << " hangle: " << hangle << " vangle: " << vangle << '\r';

      ViewMatrix = glm::lookAt(pos, pos + dir, up);
    }
    
    // mvShape::printMat("view", ViewMatrix);
    // mvShape::printMat("projection", ProjectionMatrix);
    // mvShape::printMat("lookat", LookAtMatrix);

    draw(ViewMatrix, ProjectionMatrix);
  }


  void run() {

    // Print what values get added by whom to the render state.
    // std::list<std::string> values = _vrMain->auditValuesFromAllDisplays();
    // std::cout << "VALUES ADDED:" << std::endl;
    // for (std::list<std::string>::iterator it = values.begin();
    //      it != values.end(); it++)
    //   std::cout << *it << std::endl;
    
    while (!_quit) {

      _vrMain->mainloop();
    }
  }

};

  


int main( int argc, char **argv )
{

  std::list<ImageToDisplay> images;
  
  // std::cout << "argc: " << argc << std::endl;
  // for (int i = 0; i < argc; i++) {
  //   std::cout << "    [" << i << "]: " << std::string(argv[i]) << std::endl;
  // }

  if (argc < 3) 
    throw std::runtime_error(std::string("need a config file and a report: ") +
			     std::string("tgm config.xml report.xml"));

  std::string reportName = std::string(argv[2]);
  std::string pathName = reportName.substr(0, reportName.find_last_of("/"));
  std::cout << "opening: " << reportName << std::endl;
  std::cout << "found in:" << pathName << std::endl;

  tinyxml2::XMLDocument doc;
  int d = doc.LoadFile(reportName.c_str()); 
	if (d != tinyxml2::XML_SUCCESS) {
    std::cout << "ouch:" << doc.ErrorName() << std::endl;
  } else {

		tinyxml2::XMLElement* root = doc.FirstChildElement();
    tinyxml2::XMLElement* data = root->FirstChildElement("DATA");

		for (tinyxml2::XMLElement* e = data->FirstChildElement("ROI");
         e != NULL; e = e->NextSiblingElement("ROI")) {

      // std::cout << "IMAGE: " << e->FirstChildElement("IMAGE")->GetText() << std::endl;
      // std::cout << "X:     " << e->FirstChildElement("X")->GetText() << std::endl;
      // std::cout << "Y:     " << e->FirstChildElement("Y")->GetText() << std::endl;
      // std::cout << "Z:     " << e->FirstChildElement("DEPTH")->GetText() << std::endl;
      // std::cout << "HEIGHT:" << e->FirstChildElement("HEIGHT")->GetText() << std::endl;
      // std::cout << "WIDTH: " << e->FirstChildElement("WIDTH")->GetText() << std::endl;
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



// TODO:
//
// 1. Fix so the image is visible.  This is an issue with the
// projection and view matrices, for which there are no sensible
// default values established. There should be a way for a desktop
// user to manipulate the view around to scan the entire 3D space.
// This should be the default.  It seems like the "LookAtNode" is some
// kind of stab at establishing this, but it doesn't seem to do
// anything except sit there and be confusing. [DONE]
//
// 2. Isolate shader and other OpenGL issues into a shader class.  The
// issue here is that GLFW does not support stereo in the yurt, so we
// need FreeGLUT, which doesn't seem to support the GLSL (shader
// language) version 330.  So probably we need to be able to specify a
// lower level of shader sometime.  Probably if the shader and all the
// vertex buffer manipulation were in the shader class, that would
// work out.  So the mvShape class would establish the vertices,
// normals, textures, and colors using std C++ types, and this other
// shader class would take all that and stuff it into the appropriate
// OpenGL buffers and invoke the shader necessary. [DONE]
//
// 3. Write an alternate shader class that uses an earlier shader
// syntax.  See above. [DONE]
//
// 4. Test with FreeGLUT plugin.  Note that the key names in the
//    events are from the GLFW plugin, and will probably be different
//    for the freeglut plugin. [DONE]
//
// 4.5 Test in yurt!
//
// 5. Object loader doesn't work
//
// 5.5 What happened to the axes?
//
// 6. Put spheres at the lights, so we can locate them better them?
//
// 7. Obj viewer.
//
