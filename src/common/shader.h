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
  // There are two constructors, so you can specify the file that the
  // shader code is in, or just hand it an array of strings containing
  // the shader.  Note that if you use the second method, the "#version"
  // line needs to be explicitly terminated with a \n.
  mvShader(mvShaderType type, const std::string fileName);
  mvShader(mvShaderType type, const char** shaderLines);
  ~mvShader();

  // Accessors
  GLuint getShaderID() { return _shaderID; };
  mvShaderType getShaderType() { return _shaderType; };
  std::string getCode() { return _shaderCode; };
  std::string getCompileLog() { return _compilationLog; };  
};

// A class to manage a collection of shaders.  The geometry shader is
// optional.  The compiling and linking of the shaders is done during
// the object construction, so you can't swap the shaders in and out
// after creation.
class mvShaders {
 private:
  mvShader* _vertShader;
  mvShader* _geomShader;
  mvShader* _fragShader;
  
  GLuint _programID;
  std::string _linkLog;

 public:
  mvShaders(const char** vertShader,
            const char** geomShader,
            const char** fragShader);
  mvShaders(const std::string vertFilePath,
            const std::string geomFilePath,
            const std::string fragFilePath);

  GLuint getProgram() { return _programID; };
  std::string getLinkLog() { return _linkLog; };
};
#endif
