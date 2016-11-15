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
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
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
  float _horizAngle, _vertAngle, _radius, _incAngle;

  // These are the objects that will be drawn.
  mvImages _images;

  GLuint gProgram;
  GLuint gProgramCameraPositionLocation;
  GLuint gProgramLightPositionLocation;
  GLuint gProgramLightColorLocation;

  float gCameraPosition[3];
#define NUM_LIGHTS 3
  float gLightPosition[NUM_LIGHTS * 3];
  float gLightColor[NUM_LIGHTS * 3];
  float gLightRotation;
  
  //GLuint _vertexArrayID;

  static void setPerspective(float fov, float aspect, float near, float far);
  static void lookAt(float eyeX, float eyeY, float eyeZ,
                     float centerX, float centerY, float centerZ,
                     float upX, float upY, float upZ);
  void graphicsInit(MinVR::VRDataIndex* index);
  
  std::string shaderRead(const std::string pathName);
  GLuint shaderCompile(const GLenum type, const std::string pathName);
  void shaderAttach(const GLuint program,
                    const GLenum type, const std::string pathName);

 public:
  mvImageApp(int argc, char** argv);
  virtual ~mvImageApp();

  virtual void onVREvent(const std::string &eventName,
                         MinVR::VRDataIndex *eventData);
  virtual void onVRRenderContext(MinVR::VRDataIndex *renderState, 
                                 MinVR::VRDisplayNode *callingNode);
  void onVRRenderScene(MinVR::VRDataIndex *renderState, 
                       MinVR::VRDisplayNode *callingNode);

  void run() { while (!_quit) { _vrMain->mainloop(); } };

};

