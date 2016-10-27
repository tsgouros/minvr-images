// my first program in C++
#include <iostream>
#include <string>
#include <math.h>

#include "MVR.h"

#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define TWOPI 6.283185

// An object to encapsulate a lot of working information about the
// running program: viewer position, collection of mvImage objects to
// display, and so on.
class mvImageApp :  public MinVR::VREventHandler, public MinVR::VRRenderHandler  {
 protected:
  MinVR::VRMain *_vrMain;
  bool _quit;
  double _horizAngle, _vertAngle, _radius, _incAngle;

 public:
  mvImageApp(int argc, char** argv);
  virtual ~mvImageApp();

  virtual void onVREvent(const std::string &eventName,
                         VRDataIndex *eventData);
  virtual void onVRRenderContext(VRDataIndex *renderState, 
                                 MinVR::VRDisplayNode *callingNode);
  void onVRRenderScene(VRDataIndex *renderState, 
                               MinVR::VRDisplayNode *callingNode);

  void run() { while (!_quit) { _vrMain->mainloop(); } };
};

// A class to hold an image object.  This is a 3D object located in space
// somewhere, textured with an input image read from a file.
class mvImage {
 private:
  int imageData;

 public:
 mvImage(int n) : imageData(n) {};
  virtual ~mvImage() {};

  int getImage() { return imageData; };
};


