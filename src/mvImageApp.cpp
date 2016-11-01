#include "mvImage.h"
#include "mvImageApp.h"

mvImageApp::mvImageApp(int argc, char** argv) :
  _vrMain(NULL), _quit(false) {

  _vrMain = new MinVR::VRMain();

  _images = mvImage::mvImages();

  mvImage::mvImage* img1 = new mvImage::mvImage("trash.png");
  
  _images.addImage(std::string("first"), img1);

  // glGenVertexArraysAPPLE(1, &_vertexArrayID);
  // glBindVertexArray(_vertexArrayID);
  
  std::string configFile = argv[1];
  _vrMain->initialize(argc, argv, configFile);
  _vrMain->addEventHandler(this);
  _vrMain->addRenderHandler(this);

  _horizAngle = 0.0;
  _vertAngle = 0.0;
  _radius = 15.0;
  _incAngle = -0.1f;
}

mvImageApp::~mvImageApp() {
  _vrMain->shutdown();
  delete _vrMain;
}

void mvImageApp::onVREvent(const std::string &eventName, VRDataIndex *eventData) {

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

void mvImageApp::onVRRenderContext(VRDataIndex *renderState, 
				   MinVR::VRDisplayNode *callingNode) {
  if (!renderState->exists("IsConsole", "/")) {
  }
}

// Callback for rendering, inherited from VRRenderHandler                      
void mvImageApp::onVRRenderScene(VRDataIndex *renderState,
                                 MinVR::VRDisplayNode *callingNode) {
  if (renderState->exists("IsConsole", "/")) {
    MinVR::VRConsoleNode *console = dynamic_cast<MinVR::VRConsoleNode*>(callingNode);
    console->println("Console output...");
  } else {

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
    glClearColor(0.0, 0.0, 0.0, 1.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glClear(GL_DEPTH_BUFFER_BIT);                                

    if (renderState->exists("ProjectionMatrix", "/")) {
      // This is the typical case where the MinVR DisplayGraph contains      
      // an OffAxisProjectionNode or similar node, which sets the            
      // ProjectionMatrix and ViewMatrix based on head tracking data.        

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      VRMatrix4 P = renderState->getValue("ProjectionMatrix", "/");
      glLoadMatrixd(P.m);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // In the original adaptee.cpp program, keyboard and mouse
      // commands were used to adjust the camera.  Now that we are
      // creating a VR program, we want the camera (Projection and
      // View matrices) to be controlled by head tracking.  So, we
      // switch to having the keyboard and mouse control the Model
      // matrix.  Guideline: In VR, put all of the "scene navigation"
      // into the Model matrix and leave the Projection and View
      // matrices for head tracking.
      VRMatrix4 M = VRMatrix4::translation(VRVector3(0.0, 0.0, -_radius)) *
        VRMatrix4::rotationX(_vertAngle) *
        VRMatrix4::rotationY(_horizAngle);

      VRMatrix4 V = renderState->getValue("ViewMatrix", "/");
      glLoadMatrixd((V*M).m);
    } else {
      // If the DisplayGraph does not contain a node that sets the
      // ProjectionMatrix and ViewMatrix, then we must be in a
      // non-headtracked simple desktop mode.  We can just set the
      // projection and modelview matrices the same way as in
      // adaptee.cpp.
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(1.6*45.f, 1.f, 0.1f, 100.0f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // gluPerspective(30f,[self bounds].size.width/(GLfloat)[self bounds].size.width,1.0f,1000.0f);

      // glMultMatrix( GLKMatrix4MakePerspective(30f,[self bounds].size.width/(GLfloat)[self bounds].size.height,1.0f,1000.0f ).m ); // << .m is the GLfloat* you are accessing
      
      double cameraPos[3];
      cameraPos[0] = _radius * cos(_horizAngle) * cos(_vertAngle);
      cameraPos[1] = -_radius * sin(_vertAngle);
      cameraPos[2] = _radius * sin(_horizAngle) * cos(_vertAngle);

      double cameraAim[3];
      cameraAim[0] = cos(_horizAngle) * sin(_vertAngle);
      cameraAim[1] = cos(_vertAngle);
      cameraAim[2] = sin(_horizAngle) * sin(_vertAngle);

      double targetPos[3] = {0.0f, 0.0f, 0.0f};
      gluLookAt (cameraPos[0], cameraPos[1], cameraPos[2],
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

  mvImage::mvImage m = mvImage::mvImage("trash.png");

  mvImageApp app(argc, argv);
  app.run();

  exit(0);  
}

