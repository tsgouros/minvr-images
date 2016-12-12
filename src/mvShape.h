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
// there are accompanying mvShaders and mvLights classes to manage
// those aspects of constructing a scene.  The mvShaders object is
// linked from mvShape largely for access to the programID.
//
// The shaders themselves are not included in this object because many
// objects will use the same shader.  The shader code will have to be
// specifically referenced in the load() and draw() methods here.  I
// have not come up with a clever way to avoid this, only clunky ones.
// So the program that calls this should keep three separate lists:
// the objects in the scenes, the shaders used to render them, and the
// lights used by the shaders.
//
// This class imposes a small number of restrictions on the shader
// code itself, mostly the names of things.  These are specified in
// the setupDefaultName() method.  Setting things up for the number of
// lights is also something that needs to be configured carefully.
// See the shader.h file for more about that.
//
class mvShape {
protected:

  mvShapeType _type;

  // The shaders to be used by this shape.
  mvShaders* _shaders;
  
  GLuint _arrayID;

  // The IDs refer to the OpenGL attribute "names" and the strings
  // that accompany them are the actual names used in the shaders.
  GLuint _vertexAttribID;
  std::string _vertexAttribName;
  
  GLuint _uvAttribID;
  std::string _uvAttribName;
  
  GLuint _normalAttribID;
  std::string _normalAttribName;
  
  GLuint _colorAttribID;
  std::string _colorAttribName;
  
  GLuint _textureAttribID;
  std::string _textureAttribName;

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
  std::string  _mvpMatrixName;

  GLuint _projMatrixID;
  std::string  _projMatrixName;

	GLuint _viewMatrixID;
  std::string  _viewMatrixName;

	GLuint _modelMatrixID;
  std::string  _modelMatrixName;

  glm::vec3 _position;
  glm::vec3 _scale;
  glm::quat _rotQuaternion;
  
  // This is the matrix that controls the shape's position,
  // orientation, and scale, generated by the above vectors.
  glm::mat4 _modelMatrix;
  bool _modelMatrixNeedsReset;

  static void printMat(std::string name, glm::mat4 mat);
  virtual std::string print() const;
  friend std::ostream & operator<<(std::ostream &os, const mvShape& iShape);

  // This is just a central place to put all the names that will be
  // relevant to your shaders.
  void setupDefaultNames();
  
public:
  mvShape(mvShapeType type, mvShaders* shaders);
  virtual ~mvShape();
  
  virtual void load() = 0;
  virtual void draw(VRControl control) = 0;

  // These are here to specialize in the subclasses.
  virtual void setDimensions(GLfloat a) {};
  virtual void setDimensions(GLfloat a, GLfloat b) {};
  virtual void setDimensions(GLfloat a, GLfloat b, GLfloat c) {};
  
  GLuint getProgramID() { return _shaders->getProgramID(); };

  void setTextureID(GLuint textureID) { _textureBufferID = textureID; };
  GLuint getTextureID() { return _textureBufferID; };
  
  // These are here so there is some default shader behavior beside
  // the default blackness.
  const char** defaultVertexShader;
  const char** defaultFragmentShader;

  mvShapeType getType() { return _type; };

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
  void setRotation(glm::vec3 pitchYawRoll) {
    _rotQuaternion = glm::quat(pitchYawRoll);      
    _modelMatrixNeedsReset = true;
  };
  
  glm::vec3 getPosition() { return _position; };
  glm::vec3 getScale() { return _scale; };
  glm::quat getRotQuaternion() { return _rotQuaternion; };
  glm::vec3 getPitchYawRoll() { return glm::eulerAngles(_rotQuaternion); };
  
  glm::mat4 getModelMatrix();


};

class mvShapeRect : public mvShape {
protected:

  GLfloat _width, _height;
  
  // Some of these should move into the parent class.  Also the destructor.
  GLuint _lightPositionID;
  GLuint _lightColorID;

  void initVertices();

  std::string print() const;
  friend std::ostream & operator<<(std::ostream &os, const mvShapeRect& iShape);

  
public:
 mvShapeRect(mvShaders* shaders) : mvShape(shapeRECT, shaders) {
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
  GLuint _lightPositionID;
  GLuint _lightColorID;

  std::string _objFileName;
  
  std::string print() const;
  friend std::ostream & operator<<(std::ostream &os, const mvShapeObj& iShape);
  
public:
 mvShapeObj(mvShaders* shaders) : mvShape(shapeOBJ, shaders) {};

  void load();
  void draw(VRControl control);

  std::string getObjFile() { return _objFileName; };
  void setObjFile(std::string objFileName) { _objFileName = objFileName; };
};

class mvShapeAxes : public mvShape {
 private:
  
  static const int nCoordsComponents = 3;
  static const int nColorComponents = 3;
  static const int nLines = 3;
  static const int nVerticesPerLine = 2;
  static const int nFaces = 6;
  static const int nVerticesPerFace = 3;

  void expandAxesColors();
  void expandAxesVertices();

  std::string print() const;
  friend std::ostream & operator<<(std::ostream &os, const mvShapeAxes& iShape);
  
 public:
 mvShapeAxes(mvShaders* shaders) : mvShape(shapeAXES, shaders) {
    expandAxesVertices();
    expandAxesColors();
  }    
  
  void load();
  void draw(VRControl control);

};  

class mvShapeFactory {
 public:
  template<typename T>
    static mvShape* create(mvShaders* shaders) {
    T* out = new T(shaders);
    return (mvShape*)out;
  }

  typedef mvShape* (*createMvShapeCallback)(mvShaders* shaders);

  mvShape* createShape(mvShapeType type, mvShaders* shaders);

  bool registerShape(mvShapeType type, createMvShapeCallback creator);

  mvShapeFactory() {

    // Fill factory.
    registerShape(shapeOBJ, create<mvShapeObj>);
    registerShape(shapeAXES, create<mvShapeAxes>);
    registerShape(shapeRECT, create<mvShapeRect>);
    
  }
  
 private:
  typedef std::map<mvShapeType, createMvShapeCallback> callbackMap;
  callbackMap _callbacks;

  std::string print() const;
  friend std::ostream & operator<<(std::ostream &os, const mvShapeFactory& iShape);

};

/// TBD: Deleting the shader programs needs to be addressed. Also the
/// detach* thing

// Next, 
