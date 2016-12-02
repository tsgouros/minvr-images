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
  
  glShaderSource(outID, 1, shaderLines, NULL);
  glCompileShader(outID);

  // Pick up the error log of the compilation, just in case.
  GLint result = GL_FALSE;
  GLint compilationLogLength;

  glGetShaderiv(outID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(outID, GL_INFO_LOG_LENGTH, &compilationLogLength);

  if ( compilationLogLength > 0 ){

    _compilationLog = std::string("");
    std::vector<char> shaderErrorMessage(compilationLogLength + 1);
    glGetShaderInfoLog(outID, compilationLogLength, NULL,
                       &shaderErrorMessage[0]);

    for (std::vector<char>::const_iterator it = shaderErrorMessage.begin();
         it != shaderErrorMessage.end(); it++) 
      _compilationLog.push_back(*it);

    std::cout << "compile error: " << _compilationLog << std::endl;

    std::cout << "shader code: " << _shaderCode << std::endl;
  }

  return outID; 
}
  
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

mvShader::mvShader(mvShaderType type, const char** shaderLines) :
  _shaderType(type) {

  _shaderID = init(_shaderType, shaderLines);
}

mvShader::~mvShader() {
  glDeleteShader(_shaderID);
}


mvShaders::mvShaders(const char** vertShader,
                     const char** fragShader) {

  _programID = glCreateProgram();
  
  _vertShader = new mvShader(VERTEX, vertShader);
  _fragShader = new mvShader(FRAGMENT, fragShader);

  glAttachShader(_programID, _vertShader->getShaderID());
  glAttachShader(_programID, _fragShader->getShaderID());

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
	}

  glDetachShader(_programID, _vertShader->getShaderID());
  delete _vertShader;

  glDetachShader(_programID, _fragShader->getShaderID());
  delete _fragShader;

}

mvShaders::mvShaders(const std::string vertFilePath,
                     const std::string fragFilePath) {

  _programID = glCreateProgram();
  
  _vertShader = new mvShader(VERTEX, vertFilePath);
  _fragShader = new mvShader(FRAGMENT, fragFilePath);

  glAttachShader(_programID, _vertShader->getShaderID());
  glAttachShader(_programID, _fragShader->getShaderID());

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

  glDetachShader(_programID, _fragShader->getShaderID());
  delete _fragShader;

}


