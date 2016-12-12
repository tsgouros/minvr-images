#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <string>

#include <glm/glm.hpp>

// HOW TO USE THESE SHADERS AND LIGHTS:
//
// This collection of classes is for managing a bunch of shaders and
// accompanying lights.  It's meant to work with the mvShape classes,
// a set of classes designed to allow the management of a set of
// elementary shapes in a 3D field.  The shapes can have a texture on
// them, and have handles for manipulating their location and
// orientation.
//
// To use them, concoct a list of lights with the mvLight object, and
// create a mvShaders object with the vertex, fragment, and geometry
// (if any) shaders to go with them.  If your shaders depend on the
// number of lights, there is a constructor for the mvShaders object
// that will edit the shader accordingly, see below for instructions.
//
// The shaders that work with these lights have three requirements:
//
// const int NUM_LIGHTS = XX;
// uniform vec3 LightPosition_worldspace[NUM_LIGHTS];
// uniform vec3 LightColor[NUM_LIGHTS];
//
// The 'XX' gets changed to be the actual number of lights just before
// compilation, if you use the mvShader constructor that does the
// edits.  See the setupDefaultNames() method if you want different
// names.
//
// There are other shader requirements, specifically about the names
// of the matrices, that are imposed by the mvShape classes.  See
// those comments for more information.
//
//
// A properly initialized mvShaders object, attached to some lights,
// is ready to go, and can be given to the mvShape object for use.
// Note that multiple shaders might depend on the same lights and
// multiple objects will likely use the same set of shaders.
// Therefore, these objects should be managed separately, in different
// lists for example.  This will facilitate memory management, and not
// cleaning up some shader while another shape is still using it.
//
//
// mvLights is a class for managing a list of lights to go with some
// shader.  The lights are communicated with the shader in two big
// blocks of data, one for the light positions and the other for the
// light colors.  Since the shaders depend on the specific number of
// lights in the list, we think of this list as an integral part of
// the shader's data, even if a few different shaders might refer to
// the same list.
//
// We anticipate the load() and draw() methods of this class will be
// invoked just before the load() and draw() methods of the shapes
// that depend on (the shaders that depend on) them via the shader's
// own load() and draw() methods.  We do not incorporate the load()
// and draw() into the shape methods of the same name because many
// shapes will use the same shaders and we don't really want to get
// into trying to figure out if this shader has already been drawn on
// this cycle or not.
//
class mvLights {
 private:

  std::vector<glm::vec3> _positions;
  std::vector<glm::vec3> _colors;

  GLuint _lightPositionID;
  std::string _lightPositionName;
  
  GLuint _lightColorID;
  std::string _lightColorName;

  // The default names of things in the shaders, put here for easy
  // comparison or editing.  If you're mucking around with the
  // shaders, don't forget that these are names of arrays inside the
  // shader, and that the size of the arrays is set with 'XX', see the
  // shader constructors below.
  void setupDefaultNames() {
    _lightPositionName = std::string("LightPosition_worldspace");
    _lightColorName = std::string("LightColor");
  }
  
 public:
  mvLights() { setupDefaultNames(); };
  mvLights(glm::vec3 position, glm::vec3 color) {
    setupDefaultNames();
    addLight(position, color);
  }

  GLuint getPositionID() { return _lightPositionID; };
  GLuint getColorID() { return _lightColorID; };

  int getNumLights() { return _positions.size(); };

  // We have mutators and accessors for the whole shebang...
  std::vector<glm::vec3> getPositions() { return _positions; };
  void setPositions(std::vector<glm::vec3> positions) { _positions = positions; };

  std::vector<glm::vec3> getColors() { return _colors; };
  void setColors(std::vector<glm::vec3> colors) { _colors = colors; };

  // ... and also for individual lights.
  void setPosition(int i, glm::vec3 position) { _positions[i] = position; };
  glm::vec3 getPosition(int i) { return _positions[i]; };
  
  void setColor(int i, glm::vec3 color) { _colors[i] = color; };
  glm::vec3 getColor(int i) { return _colors[i]; };

  // Use this to add lights.  Since the shaders are compiled and
  // linked after the number of lights is set, this is pretty much a
  // one-way street.  Add lights, but don't subtract them.  If you
  // want to extinguish one, just move it far away, or dial its
  // intensity way down.
  int addLight(glm::vec3 position, glm::vec3 color) {
    _positions.push_back(position);
    _colors.push_back(color);
    return _positions.size();
  };

  // Load these lights for use with this program.  This should be
  // called before the load() method of the shape object.
  void load(GLuint programID);

  // Draw these lights.  This is for updating the position and color
  // if they have changed since the last scene render.
  void draw(GLuint programID);  
};

typedef enum {
  VERTEX   = 0,
  GEOMETRY = 1,
  FRAGMENT = 2 } mvShaderType;

// This class holds and manages just one of the three different kinds
// of shaders.  Basically this is just about file access, and managing
// access to the error log if there is a compile or link problem.
class mvShader {
 private:
  GLuint _shaderID;
  mvShaderType _shaderType;  
  std::string _shaderCode;
  std::string _compilationLog;

  GLuint init(mvShaderType type, const char** shaderLines);
  
 public:
  // There are two varieties of constructors, so you can specify the
  // file that the shader code is in, or just hand it an array of
  // strings containing the shader.  
  mvShader(mvShaderType type, const std::string fileName);
  // This is a variant of the first constructor, where the shaders
  // need to know the number of lights.
  mvShader(mvShaderType type, const std::string fileName, int numLights);
  // This constructor just takes the shader code as a collection of
  // C-style strings.  If you use this method, the "#version" line
  // needs to be explicitly terminated with a \n.
  mvShader(mvShaderType type, const char** shaderLines);
  ~mvShader();

  // Accessors
  GLuint getShaderID() { return _shaderID; };
  mvShaderType getShaderType() { return _shaderType; };
  std::string getCode() { return _shaderCode; };
  std::string getCompileLog() { return _compilationLog; };  
};

// mvShaders is a class to manage the collection of two or three
// shaders that make up an OpenGL "program".  The geometry shader is
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

  mvLights* _lights;

  void attachAndLinkShaders();
  
 public:
  mvShaders();
  mvShaders(const std::string vertShader,
            const std::string geomShader,
            const std::string fragShader,
            mvLights* lights);

  void load() { _lights->load(_programID); };
  void draw() { _lights->draw(_programID); };
  
  GLuint getProgramID() { return _programID; };
  std::string getLinkLog() { return _linkLog; };
};
#endif
