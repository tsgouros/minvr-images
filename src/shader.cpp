#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

void mvLights::load(GLuint programID) {

  // Get a handle for our lighting uniforms.  We are not binding the
  // attribute to a known location, just asking politely for it.  Note
  // that what is going on here is that OpenGL is actually matching
  // the _lightPositionName string to a variable in the shader.
  glUseProgram(programID);
  _lightPositionID = glGetUniformLocation(programID, _lightPositionName.c_str());
  _lightColorID = glGetUniformLocation(programID, _lightColorName.c_str());

}

// Update any changes to the light's position and color.
void mvLights::draw(GLuint programID) {

  glUseProgram(programID);
  glUniform3fv(_lightPositionID, 2, &_positions[0].x);
  glUniform3fv(_lightColorID, 2, &_colors[0].x);
}

// Create a shader and compile it with the OpenGL tools.  This is the
// guts of the mvShader constructor.
GLuint mvShader::init(mvShaderType type, const char** shaderLines) {

  GLuint outID;
  
  // Create the shader, and compile it here.
  switch (type) {
  case VERTEX:
    outID = glCreateShader(GL_VERTEX_SHADER);
    break;

  case GEOMETRY:
    outID = glCreateShader(GL_GEOMETRY_SHADER);
    break;
      
  case FRAGMENT:
    outID = glCreateShader(GL_FRAGMENT_SHADER);
    break;
  };
  
  // Feed in the source.
  glShaderSource(outID, 1, shaderLines, NULL);
  // Compile it.
  glCompileShader(outID);

  // Pick up the error log of the compilation, just in case.
  GLint result = GL_FALSE;
  GLint compilationLogLength;
  glGetShaderiv(outID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(outID, GL_INFO_LOG_LENGTH, &compilationLogLength);

  // Ask if there has been an error, and if so, print from that version.
  if ( compilationLogLength > 0 ){

    _compilationLog = std::string("");
    std::vector<char> shaderErrorMessage(compilationLogLength + 1);
    glGetShaderInfoLog(outID, compilationLogLength, NULL,
                       &shaderErrorMessage[0]);

    for (std::vector<char>::const_iterator it = shaderErrorMessage.begin();
         it != shaderErrorMessage.end(); it++) 
      _compilationLog.push_back(*it);

    std::cout << "compile error: " << _compilationLog << std::endl;
  }

  return outID; 
}

// Read a shader from a given file.
mvShader::mvShader(mvShaderType type, const std::string fileName) :
  _shaderType(type) {

	// Read the shader code from the file
	std::ifstream shaderStream(fileName, std::ios::in);
	if (shaderStream.is_open()) {
		std::string line = "";
    
		while(getline(shaderStream, line)) _shaderCode += "\n" + line;
    
		shaderStream.close();
    
	} else {
    throw std::runtime_error("Cannot open: " + fileName);
	}

	char const * sourcePtr = _shaderCode.c_str();
  _shaderID = init(_shaderType, &sourcePtr );
}

// Read a shader from a file, and edit it to reflect the number of
// lights we will be using.  The shader code should use 'XX' wherever
// it needs the number of lights.
mvShader::mvShader(mvShaderType type, const std::string fileName, int numLights) :
  _shaderType(type) {

#ifdef DEBUG
  std::cout << "shader file: " << fileName << " n: " << numLights << std::endl;
#endif
  
 	// Read the shader code from the file
	std::ifstream shaderStream(fileName, std::ios::in);
	if (shaderStream.is_open()) {
		std::string line = "";
    
		while(getline(shaderStream, line)) _shaderCode += "\n" + line;
    
		shaderStream.close();
    
	} else {
    throw std::runtime_error("Cannot open: " + fileName);
	}

  // Edit the shader source to reflect the input number of lights.  If
  // there is no 'XX' in the shader code, this will cause an ugly
  // error.
  _shaderCode.replace(_shaderCode.find("XX"), 2, to_string(numLights));

	char const * sourcePtr = _shaderCode.c_str();
  _shaderID = init(_shaderType, &sourcePtr );
}

// Just an alternate constructor for an older style input.
mvShader::mvShader(mvShaderType type, const char** shaderLines) :
  _shaderType(type) {

  _shaderID = init(_shaderType, shaderLines);
}

mvShader::~mvShader() {
  glDeleteShader(_shaderID);
}

mvShaderSet::mvShaderSet() : _lightsLoaded(false) {

  // This is sort of hacky, but these two variables are here so that
  // the default constructor for this class provides a trivial shader,
  // so at least something will appear during the experimentation
  // phase of building a 3D application.
  static const char* defaultVertexShader = 
    "#version 120\n"
    "attribute vec3 vertexPosition_modelspace;"
    "attribute vec3 vertexInputColor;"
    "uniform mat4 MVP;"
    "varying vec3 vertexColor;"
    "void main () {"
    "gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);"
    "vertexColor = vertexInputColor;"
    "}";

  static const char* defaultFragmentShader =
    "#version 120\n"
    "varying vec3 vertexColor;"
    "void main () {"
    "gl_FragColor = vec4(vertexColor, 1.0);"
    "}";

  static const char* defaultGeometryShader = NULL;

  // These shaders don't use explicit lights, but we need a light
  // object around to keep things from crashing.
  _lights = new mvLights(MVec3(1,1,1), MVec3(1,1,1));
  
  _programID = glCreateProgram();
  
  _vertShader = new mvShader(VERTEX, &defaultVertexShader);
  _fragShader = new mvShader(FRAGMENT, &defaultFragmentShader);
  if (defaultGeometryShader != NULL) {
    _geomShader = new mvShader(GEOMETRY, &defaultGeometryShader);
  } else {
    _geomShader = NULL;
  }

  attachAndLinkShaders();

}

mvShaderSet::mvShaderSet(const std::string vertShader,
                         const std::string geomShader,
                         const std::string fragShader,
                         mvLights* lights) :
  _lights(lights), _lightsLoaded(false)  {

  // Clear OpenGL errors
  glGetError();

  _programID = glCreateProgram();

#ifdef DEBUG  
  GLint maj, min;
  glGetIntegerv(GL_MAJOR_VERSION, &maj);
  glGetIntegerv(GL_MINOR_VERSION, &min);
  std::cout << "opengl version: " << maj << "." << min << std::endl;

  std::cout << "vertShader:" << vertShader << std::endl;
  if (!geomShader.empty()) {std::cout << "geomShader:" << geomShader << std::endl;}
  std::cout << "fragShader:" << fragShader << std::endl;
  std::cout << "lights: " << lights->getNumLights() << std::endl;
#endif
  
  _vertShader = new mvShader(VERTEX, vertShader, _lights->getNumLights());
  _fragShader = new mvShader(FRAGMENT, fragShader, _lights->getNumLights());
  if (!geomShader.empty())
    _geomShader = new mvShader(GEOMETRY, geomShader, _lights->getNumLights());
  else _geomShader = NULL;
  
  attachAndLinkShaders();

  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cout << "OpenGL error in shader compile: " << error << std::endl;
  }  
}

mvShaderSet::~mvShaderSet() {}

void mvShaderSet::attachAndLinkShaders() {
  
  glAttachShader(_programID, _vertShader->getShaderID());
  glAttachShader(_programID, _fragShader->getShaderID());
  if (_geomShader != NULL) glAttachShader(_programID, _geomShader->getShaderID());

	glLinkProgram(_programID);

	// Check the program
  GLint result = GL_FALSE;
  int linkLogLength;

	glGetProgramiv(_programID, GL_LINK_STATUS, &result);
	glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &linkLogLength);

  if ( linkLogLength > 0 ){

    _linkLog = std::string("");
		std::vector<char> programErrorMessage(linkLogLength + 1);
		glGetProgramInfoLog(_programID, linkLogLength, NULL, &programErrorMessage[0]);

    for (std::vector<char>::const_iterator it = programErrorMessage.begin();
         it != programErrorMessage.end(); it++) 
      _linkLog.push_back(*it);

    std::cout << "link error: " << _linkLog << std::endl;

    std::cout << "vertex code: " << _vertShader->getCode();
    std::cout << "fragment code: " << _fragShader->getCode();
    
	}

  glDetachShader(_programID, _vertShader->getShaderID());
  delete _vertShader;

  if (_geomShader != NULL) {

    glDetachShader(_programID, _geomShader->getShaderID());
    delete _geomShader;
  }

  glDetachShader(_programID, _fragShader->getShaderID());
  delete _fragShader;

}

void mvShaderSet::load() {
  if (!_lightsLoaded) {
    _lights->load(_programID);
    _lightsLoaded = true;
  }
}

void mvShaderSet::draw() {
  _lights->draw(_programID);
}

void mvShaderContext::load(const std::vector<MVec3> &vertices,
                           const std::vector<MVec2> &uvs,
                           const std::vector<MVec3> &normals,
                           const std::vector<MVec3> &colors) {

  _vertexBufferSize = vertices.size();

  GLuint programID = _shaderSet->getProgramID();

  _texture->load(programID);
  _shaderSet->load();
  
    // Arrange the data for the shaders to work on.  "Uniforms" first.
  _projMatrixID = glGetUniformLocation(programID, _projMatrixName.c_str());
  _viewMatrixID = glGetUniformLocation(programID, _viewMatrixName.c_str());
  _modelMatrixID = glGetUniformLocation(programID, _modelMatrixName.c_str());
  _inverseModelMatrixID = glGetUniformLocation(programID,
                                               _inverseModelMatrixName.c_str());

  // Now the vertex data.
  glGenVertexArrays(1, &_arrayID);
  glBindVertexArray(_arrayID);

  // Load it into a VBO
  glGenBuffers(1, &_vertexBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MVec3),
               &vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_uvBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _uvBufferID);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(MVec2),
               &uvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_normalBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(MVec3),
               &normals[0], GL_STATIC_DRAW);

  // Get handles for the various shader inputs.
  _vertexAttribID = glGetAttribLocation(programID, _vertexAttribName.c_str());
  _uvAttribID = glGetAttribLocation(programID, _uvAttribName.c_str());
  _normalAttribID = glGetAttribLocation(programID, _normalAttribName.c_str());

};

void mvShaderContext::draw(const MMat4 &modelMatrix,
                           const MMat4 &viewMatrix,
                           const MMat4 &projectionMatrix) {

  GLuint programID = _shaderSet->getProgramID();

  // Use our shader set.
  glUseProgram(programID);

  _texture->draw(programID);
  _shaderSet->draw();
  
  // Send our transformation to the currently bound shader.
  glUniformMatrix4fv(_projMatrixID, 1, GL_FALSE, &projectionMatrix[0][0]);
  glUniformMatrix4fv(_viewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);
  glUniformMatrix4fv(_modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
  MMat4 invM = glm::transpose(glm::inverse(modelMatrix));
  glUniformMatrix4fv(_inverseModelMatrixID, 1, GL_FALSE, &invM[0][0]);
  
  // GLint countt;
  // glGetProgramiv(_shaders->getProgramID(), GL_ACTIVE_UNIFORMS, &countt);
  // std::cout << "**Active (in use by a shader) Uniforms: " << countt << std::endl;

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(_vertexAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
  glVertexAttribPointer(
                        _vertexAttribID,    // attribute
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(_uvAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _uvBufferID);
  glVertexAttribPointer(
                        _uvAttribID,        // attribute
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // 3rd attribute buffer : normals
  glEnableVertexAttribArray(_normalAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
  glVertexAttribPointer(
                        _normalAttribID,    // attribute
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // Draw the triangles !
  glDrawArrays(_mode, 0, _vertexBufferSize );

  glDisableVertexAttribArray(_vertexAttribID);
  glDisableVertexAttribArray(_uvAttribID);
  glDisableVertexAttribArray(_normalAttribID);

};
