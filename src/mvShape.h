#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <common/shader.h>
#include <common/texture.h>
#include <common/controls.h>
#include <common/objloader.h>

typedef enum {
  shapeOBJ = 0,
  shapeAXES = 1,
  shapeRECT = 2
} mvShapeType;
  
// This is a generic class to hold a shape to be drawn.  The actual
// vertex coordinates of the shape are supplied in a sub-class, which
// might hold a line, cylinder, rectangle, OBJ file, whatever.  There
// is a load() method for getting vertices and texture coordinates and
// whatever else ready, and a draw() method for the frame updates.
//
// This is meant to work with modern OpenGL, that uses shaders, and
// the constructor to the class requires an OpenGL ID number for a
// compiled set of shaders that will work with this class.  The
// mvShaders class is available for managing a collection of shaders.
//
// The shaders themselves are not included in this object because many
// objects will use the same shader.  The shader code will have to be
// specifically referenced in the load() and draw() methods here.  I
// have not come up with a clever way to avoid this.
//
class mvShape {
protected:

  mvShapeType _type;

  // The program ID for the shaders to be used by this shape.
  GLuint _programID; 
  
  GLuint _arrayID;

  // These IDs point to the names/locations of values in shaders.
  GLuint _vertexAttribID;
  GLuint _uvAttribID;
  GLuint _normalAttribID;
  GLuint _colorAttribID;
  GLuint _textureAttribID;

  // These IDs point to actual data.
  GLuint _vertexBufferID;
	GLuint _uvBufferID;
	GLuint _normalBufferID;
  GLuint _colorBufferID;
  GLuint _textureBufferID;

  // The actual shape data is stored here.
  std::vector<glm::vec3> _vertices;
	std::vector<glm::vec2> _uvs;
	std::vector<glm::vec3> _normals;
	std::vector<glm::vec3> _colors;

  // These matrices may appear in the shaders.
  GLuint _mvpMatrixID;
  GLuint _projMatrixID;
	GLuint _viewMatrixID;
	GLuint _modelMatrixID;

  glm::vec3 _position;
  glm::vec3 _scale;
  glm::quat _rotQuaternion;
  
  // This is the matrix that controls the shape's position,
  // orientation, and scale, generated by the above vectors.
  glm::mat4 _modelMatrix;
  bool _modelMatrixNeedsReset;

  static void printMat(std::string name, glm::mat4 mat);

  
public:
  mvShape(mvShapeType type, GLuint programID);
  ~mvShape();
  
  virtual void load() = 0;
  virtual void draw(VRControl control) = 0;

  // These are here so there is some default shader behavior beside
  // the default blackness.
  const char** defaultVertexShader;
  const char** defaultFragmentShader;

  // Position, rotation, scale control mutators.
  void setPosition(glm::vec3 position) {
    _position = position;
    _modelMatrixNeedsReset = true;
  };
  void setScale(glm::vec3 scale) {
    _scale = scale;
    _modelMatrixNeedsReset = true;
  };
  void setRotation(glm::quat rotQuaternion) {
    _rotQuaternion = rotQuaternion;
    _modelMatrixNeedsReset = true;
  };
  void setRotation(glm::vec3 pitchRollYaw) {
    _rotQuaternion = glm::quat(pitchRollYaw);      
    _modelMatrixNeedsReset = true;
  };
  
  glm::vec3 getPosition() { return _position; };
  glm::vec3 getScale() { return _scale; };
  glm::quat getRotQuaternion() { return _rotQuaternion; };

  glm::mat4 getModelMatrix();


};

class mvShapeRect : public mvShape {
private:

  GLfloat _width, _height;
  
  // Some of these should move into the parent class.  Also the destructor.
  GLuint _lightID;

  void initVertices();

public:
 mvShapeRect(GLuint programID) : mvShape(shapeRECT, programID) {
    // Set default rectangle dimensions.
    _width = 1.0f;  _height = 1.0f;
  };

  void setWidth(GLfloat width) { _width = width; };
  void setHeight(GLfloat height) { _height = height; };
  void setDimensions(GLfloat width, GLfloat height) {
    _width = width; _height = height; };

  GLfloat getWidth() { return _width; };
  GLfloat getHeight() { return _height; };
  
  void load();
  void draw(VRControl control);
};

class mvShapeObj : public mvShape {
private:

  // Some of these should move into the parent class.  Also the destructor.
  GLuint _lightID;

public:
 mvShapeObj(GLuint programID) : mvShape(shapeOBJ, programID) {};

  void load();
  void draw(VRControl control);
};

class mvShapeAxes : public mvShape {

  static const int nCoordsComponents = 3;
  static const int nColorComponents = 3;
  static const int nLines = 3;
  static const int nVerticesPerLine = 2;
  static const int nFaces = 6;
  static const int nVerticesPerFace = 3;

  void expandAxesColors();
  void expandAxesVertices();

 public:
 mvShapeAxes(GLuint programID) : mvShape(shapeAXES, programID) {
    expandAxesVertices();
    expandAxesColors();
  }    
  
  void load();
  void draw(VRControl control);

};  

class mvShapeFactory {
 public:
  typedef mvShape* (*createMvShapeCallback)(GLuint programID);

  mvShape* createMvShape(mvShapeType type, GLuint programID);

  bool registerMvShape(mvShapeType type, createMvShapeCallback creator);

 private:
  typedef std::map<mvShapeType, createMvShapeCallback> callbackMap;
  callbackMap _callbacks;

};

/// TBD: Deleting the shader programs needs to be addressed. Also the
/// detach* thing

// Next, 
