#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <string>

typedef enum {
  VERTEX   = 0,
  GEOMETRY = 1,
  FRAGMENT = 2 } mvShaderType;

class mvShader {
 private:
  GLuint _shaderID;
  mvShaderType _shaderType;  
  std::string _shaderCode;
  std::string _compilationLog;

  GLuint init(mvShaderType type, const char** shaderLines);
  
 public:
  mvShader(mvShaderType type, const std::string fileName);
  mvShader(mvShaderType type, const char** shaderLines);
  ~mvShader();

  GLuint getShaderID() { return _shaderID; };
  mvShaderType getShaderType() { return _shaderType; };

  std::string getCode() { return _shaderCode; };
  std::string getCompileLog() { return _compilationLog; };  
};

class mvShaders {
 private:
  mvShader* _vertShader;
  mvShader* _fragShader;
  
  GLuint _programID;
  std::string _linkLog;

 public:
  mvShaders(const char** vertShader,
            const char** fragShader);
  mvShaders(const std::string vertFilePath,
            const std::string fragFilePath);

  GLuint getProgram() { return _programID; };
  std::string getLinkLog() { return _linkLog; };
};
#endif
