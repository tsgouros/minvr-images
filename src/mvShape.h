#include <iostream>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.h>
#include <common/texture.h>
#include <common/controls.h>
#include <common/objloader.h>

typedef enum {
  shapeOBJ = 0,
  shapeAXES = 1
} mvShapeType;
  

class mvShape {
protected:

  mvShapeType _type;
  
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

  GLuint _mvpMatrixID;
	GLuint _viewMatrixID;
	GLuint _modelMatrixID;

  glm::mat4 _modelMatrix;
  
  std::vector<glm::vec3> _vertices;
	std::vector<glm::vec2> _uvs;
	std::vector<glm::vec3> _normals;
	//std::vector<glm::vec3> _colors;

  static void printMat(std::string name, glm::mat4 mat);

  
public:
 mvShape(mvShapeType type) : _type(type) {};

  virtual void load(GLuint programID) = 0;
  virtual void draw(GLuint programID, VRControl control) = 0;

  // These are here so there is some default behavior beside blackness.  
  const char** defaultVertexShader;
  const char** defaultFragmentShader;
};

class mvShapeObj : public mvShape {
private:

  // Some of these should move into the parent class.  Also the destructor.
  GLuint _lightID;

public:
 mvShapeObj() : mvShape(shapeOBJ) {};
  ~mvShapeObj();

  void load(GLuint programID);
  void draw(GLuint programID, VRControl control);
};

class mvShapeAxes : public mvShape {

  static const int nCoordsComponents = 3;
  static const int nColorComponents = 3;
  static const int nLines = 3;
  static const int nVerticesPerLine = 2;
  static const int nFaces = 6;
  static const int nVerticesPerFace = 3;

  float ave[nLines * nVerticesPerLine * nCoordsComponents];
  float ace[nLines * nVerticesPerLine * nColorComponents];

  void expandAxesColors();
  void expandAxesVertices();

 public:
 mvShapeAxes() : mvShape(shapeAXES) {
    expandAxesVertices();
    expandAxesColors();
  }    
  
  void load(GLuint programID);

  void draw(GLuint programID, VRControl control);
};  

