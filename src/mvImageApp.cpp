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

void
mvImageApp::setPerspective(float fov, float aspect, float near, float far) {
	float f;
	float mat[16];

	f = 1.0f / tanf(fov / 2.0f);

	mat[0] = f / aspect;
	mat[1] = 0.0f;
	mat[2] = 0.0f;
	mat[3] = 0.0f;

	mat[4] = 0.0f;
	mat[5] = f;
	mat[6] = 0.0f;
	mat[7] = 0.0f;

	mat[8] = 0.0f;
	mat[9] = 0.0f;
	mat[10] = (far + near) / (near  - far);
	mat[11] = -1.0f;

	mat[12] = 0.0f;
	mat[13] = 0.0f;
	mat[14] = (2.0f * far * near) / (near - far);
	mat[15] = 0.0f;

	glMultMatrixf(mat);
}

void
mvImageApp::lookAt(float eyeX, float eyeY, float eyeZ,
                   float centerX, float centerY, float centerZ,
                   float upX, float upY, float upZ) {

  VRVector3 forward, side, up;
  float m[4][4];

  
  forward = VRVector3(centerX - eyeX, centerY - eyeY, centerZ - eyeZ);
  forward = forward.normalize();

  up = VRVector3(upX, upY, upZ);
  side = forward.cross(up);

  side = side.normalize();
  
  up = side.cross(forward);

  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];
  m[3][0] = 0.0;
  
  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];
  m[3][1] = 0.0;
  
  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];
  m[3][2] = 0.0;

  m[0][3] = m[1][3] = m[2][3] = 0.0; m[3][3] = 1.0;
  
  glMultMatrixf(&m[0][0]);
  glTranslated(-eyeX, -eyeY, -eyeZ);
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
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

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

      // In a VR program, we want the camera (Projection and View
      // matrices) to be controlled by head tracking.  So, we switch
      // to having the keyboard and mouse control the Model matrix.
      // Guideline: In VR, put all of the "scene navigation" into the
      // Model matrix and leave the Projection and View matrices for
      // head tracking.
      VRMatrix4 M = VRMatrix4::translation(VRVector3(0.0, 0.0, -_radius)) *
        VRMatrix4::rotationX(_vertAngle) *
        VRMatrix4::rotationY(_horizAngle);

      VRMatrix4 V = renderState->getValue("ViewMatrix", "/");
      glLoadMatrixd((V * M).m);
    } else {
      // If the DisplayGraph does not contain a node that sets the
      // ProjectionMatrix and ViewMatrix, then we must be in a
      // non-headtracked simple desktop mode.  We can just set the
      // projection and modelview matrices.
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      setPerspective(M_PI / 4.0f, 1.f, 0.1f, 100.0f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      float cameraPos[3];
      cameraPos[0] = _radius * cos(_horizAngle) * cos(_vertAngle);
      cameraPos[1] = -_radius * sin(_vertAngle);
      cameraPos[2] = _radius * sin(_horizAngle) * cos(_vertAngle);

      float cameraAim[3];
      cameraAim[0] = cos(_horizAngle) * sin(_vertAngle);
      cameraAim[1] = cos(_vertAngle);
      cameraAim[2] = sin(_horizAngle) * sin(_vertAngle);

      float targetPos[3] = {0.0f, 0.0f, 0.0f};
      lookAt (cameraPos[0], cameraPos[1], cameraPos[2],
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

