#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#include "MVR.h"

#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
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
// display, and so on.  The overall graphics context is handled here,
// and the mvImage objects are used to handle the objects that appear
// in the scene.
class mvImageApp :  public MinVR::VREventHandler, public MinVR::VRRenderHandler  {
 protected:
  MinVR::VRMain *_vrMain;
  bool _quit, _needInitialization;
  float _horizAngle, _vertAngle, _radius, _incAngle;

  // Where do shape objects come from? 
  mvImageShapeFactory _shapeFactory;
  
  // We store the images in a map, so we can reference them by name or
  // ID string.
  typedef std::map<std::string, mvImage *> imageMap;
  imageMap _images;

  friend std::ostream & operator<<(std::ostream &os, const mvImageApp& imageApp);

  // We can add images either by reference to the input data index or
  // by fiat.
  std::string addImage(const std::string name, MinVR::VRDataIndex* index);
  std::string addImage(const std::string name,
                       mvImageData* image, mvImageShape* shape);
  int delImage(const std::string name);
  mvImage* getImage(const std::string name);

  std::string print() const;
  
  // The OpenGL details.  First, where do we file all the information so the
  // shaders can get at it?
  GLuint _gProgram;
  GLuint _gProgramCameraPositionLocation;
  GLuint _gProgramLightPositionLocation;
  GLuint _gProgramLightColorLocation;
  GLuint _gProgramModelMatrix;
  GLuint _gProgramViewMatrix;
  GLuint _gProgramProjMatrix;

  // Second, what is the information?
  MinVR::VRVector3 _gCameraPos;
  MinVR::VRVector3 _gCameraUp;
#define NUM_LIGHTS 3
  float _gLightPosition[NUM_LIGHTS * 3];
  float _gLightColor[NUM_LIGHTS * 3];
  float _gLightRotation;
  
  void graphicsInit(MinVR::VRDataIndex* index);
  
  std::string shaderRead(const std::string pathName);
  GLuint shaderCompile(const GLenum type, const std::string pathName);
  void shaderAttach(const GLuint program,
                    const GLenum type, const std::string pathName);

  static MinVR::VRMatrix4 perspective(float fov,
                                      float aspect,
                                      float near,
                                      float far);
  static MinVR::VRMatrix4 lookat(MinVR::VRVector3 cameraPos,
                                 MinVR::VRVector3 targetPos,
                                 MinVR::VRVector3 cameraUp);

  // Used for animation features.
  unsigned long int _lastMilliSeconds;
  float getElapsedSeconds(void);
  
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

