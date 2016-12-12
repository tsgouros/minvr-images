#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>

// A class for managing a list of lights to go with some shader.  The
// lights are communicated with the shader in two big blocks of data,
// one for the light positions and the other for the colors.
//
// We anticipate the load() and draw() methods of this class will be
// invoked just before the load() and draw() methods of the shapes
// that depend on them.


// Multiple shader instances will depend on the same lights, so ther
class mvLights {
 private:

  std::vector<glm::vec3> _positions;
  std::vector<glm::vec3> _colors;

  GLuint _lightPositionID;
  GLuint _lightColorID;

 public:
  mvLights(glm::vec3 position, glm::vec3 color) {};

  GLuint getPositionID() { return _lightPositionID; }
  GLuint getColorID() { return _lightColorID; }
  
  std::vector<glm::vec3> getPositions() { return _positions; };
  void setPositions(std::vector<glm::vec3> positions) { _positions = positions; };

  std::vector<glm::vec3> getColors() { return _colors; };
  void setColor(std::vector<glm::vec3> colors) { _colors = colors; };

  void setPosition(int i, glm::vec3 position) { _positions[i] = position; };
  void setColor(int i, glm::vec3 color) { _colors[i] = color; };

  int addLight(glm::vec3 position, glm::vec3 color) {
    _positions.push_back(position);
    _colors.push_back(color);
    return _positions.size();
  };

  void load(GLuint programID) {
    glUseProgram(programID);
      };


  
};
      



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

  std::vector<mvLights*> _lights;

  void attachAndLinkShaders();
  
 public:
  mvShaders();
  mvShaders(const std::string vertShader,
            const std::string geomShader,
            const std::string fragShader);

  GLuint getProgramID() { return _programID; };
  std::string getLinkLog() { return _linkLog; };

  uint addLight(glm::vec3 newLightPosition, glm::vec3 newLightColor);
  bool modLight(uint lightIndex, glm::vec3 newLightPosition, glm::vec3 newLightColor);
  
};
#endif
