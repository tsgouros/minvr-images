#ifndef SHADER_HPP
#define SHADER_HPP

#include "vecTypes.h"
#include "texture.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
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
// create a mvShaderSet object with the vertex, fragment, and geometry
// (if any) shaders to go with them.  If your shaders depend on the
// number of lights, there is a constructor for the mvShaderSet object
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
//
// A properly initialized mvShaderSet object, attached to some lights,
// is ready to go, and can be given to the mvShape object for use.
// Note that multiple shaders might depend on the same lights and
// multiple objects will likely use the same set of shaders.
// Therefore, these objects should be managed separately, in different
// lists for example.  This will facilitate memory management, and not
// cleaning up some shader while another shape is still using it.
//
//
// mvLights is a class for managing a list of lights to go with some
// shader.  The lights are communicated with the shader in two blocks
// of data, one for the light positions and the other for the light
// colors.  Since the shaders depend on the specific number of lights
// in the list, we think of this list as an integral part of the
// shader's data, even if a few different shaders might refer to the
// same list.
//
// We anticipate the load() and draw() methods of this class will be
// invoked in the load() and draw() methods of the shaders that depend
// on them.  There may be several shaders that use the same lights, so
// this may result in multiple loads of the same data.  Optimizing
// that redundancy out of the system is an exercise left for the
// reader.
//
class mvLights {
 private:

  std::vector<MVec3> _positions;
  std::vector<MVec3> _colors;

  GLuint _lightPositionID;
  std::string _lightPositionName;
  
  GLuint _lightColorID;
  std::string _lightColorName;

  void setupDefaultNames() {
    // The default names of things in the shaders, put here for easy
    // comparison or editing.  If you're mucking around with the
    // shaders, don't forget that these are names of arrays inside the
    // shader, and that the size of the arrays is set with 'XX', see
    // the shader constructors below.
    _lightPositionName = std::string("LightPosition_worldspace");
    _lightColorName = std::string("LightColor");
  }
  
 public:
  mvLights() { setupDefaultNames(); };
  mvLights(MVec3 position, MVec3 color) {
    setupDefaultNames();
    addLight(position, color);
  }

  GLuint getPositionID() { return _lightPositionID; };
  GLuint getColorID() { return _lightColorID; };

  int getNumLights() { return _positions.size(); };

  // We have mutators and accessors for all the pieces...
  std::vector<MVec3> getPositions() { return _positions; };
  void setPositions(std::vector<MVec3> positions) { _positions = positions; };

  std::vector<MVec3> getColors() { return _colors; };
  void setColors(std::vector<MVec3> colors) { _colors = colors; };

  // ... and also for individual lights.
  void setPosition(int i, MVec3 position) { _positions[i] = position; };
  MVec3 getPosition(int i) { return _positions[i]; };
  
  void setColor(int i, MVec3 color) { _colors[i] = color; };
  MVec3 getColor(int i) { return _colors[i]; };

  // Use this to add lights.  Since the shaders are compiled and
  // linked after the number of lights is set, this is pretty much a
  // one-way street.  Add lights, but don't subtract them.  If you
  // want to extinguish one, just move it far away, or dial its
  // intensity way down.
  int addLight(MVec3 position, MVec3 color) {
    _positions.push_back(position);
    _colors.push_back(color);
    return _positions.size();
  };

  // Load these lights for use with this program.  This should be
  // called inside the shader that uses it.
  void load(GLuint programID);

  // Draw these lights.  This is mostly just for updating the position
  // and color if they have changed since the last scene render.
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

  // This is the guts of the constructor, separated out for the
  // convenience of multiple constructors.
  GLuint init(mvShaderType type, const char** shaderLines);
  
 public:
  // There are two varieties of constructors, so you can specify the
  // file that the shader code is in, or just hand it an array of
  // strings containing the shader.  
  mvShader(mvShaderType type, const std::string fileName);

  // This is a variant of the first constructor, where the shaders
  // need to know the number of lights.  It expects the shader code to
  // use 'XX' wherever the number of lights appears, and will edit on
  // the fly, accordingly.
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

// mvShaderSet is a class to manage the collection of two or three
// shaders that make up an OpenGL "program".  The geometry shader is
// optional.  The compiling and linking of the shaders is done during
// the object construction, so you can't swap the shaders in and out
// after creation.
class mvShaderSet {
 private:
  mvShader* _vertShader;
  mvShader* _geomShader;
  mvShader* _fragShader;
  
  GLuint _programID;
  std::string _linkLog;

  mvLights* _lights;
  bool _lightsLoaded;

  void attachAndLinkShaders();
  
 public:
  mvShaderSet();
  mvShaderSet(const std::string vertShader,
              const std::string geomShader,
              const std::string fragShader,
              mvLights* lights);

  ~mvShaderSet();
  
  GLuint getProgramID() { return _programID; };
  std::string getLinkLog() { return _linkLog; };

  // These are for making sure that anything that has changed in the
  // shader set will be properly accounted for at the next render.
  // Mostly this would be changes in location for one or more of the
  // lights, but anything else could be changed, too.
  void load();
  void draw();
};


// This class packages all the buffers and array objects that make a
// shader work, and provides the object with a relatively simple and
// well-labeled interface.  Basically the object just has to provide a
// vertex array, normals, texture coordinates, and colors on the
// load(), and model, view, and projection matrices on the draw().
// The shaders (and their accompanying lights) can be shared among a
// large number of shader contexts, though the calling program must
// keep track of creating and destroying those objects when necessary.
//
// In a management sense, this class is closely linked with some
// specific shaders object.  It uses the same names in shader, for
// example.  But there is one shaderContext for each mvShape object in
// the scene, and a much smaller number of mvShaderSet objects, possibly
// just one.
class mvShaderContext {
private:
  mvShaderSet* _shaderSet;
  mvTexture* _texture;

  // One of GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
  // GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS,
  // GL_QUAD_STRIP, and GL_POLYGON.  See below.
  GLenum _mode;
  
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

  // Is this the VAO?
  GLuint _arrayID;
  
  // These IDs point to actual data.
  GLuint _vertexBufferID;
	GLuint _uvBufferID;
	GLuint _normalBufferID;
  GLuint _colorBufferID;
  int _vertexBufferSize;
  
  // These matrices may appear in the shaders.
  GLuint _projMatrixID;
  std::string  _projMatrixName;

	GLuint _viewMatrixID;
  std::string  _viewMatrixName;

	GLuint _modelMatrixID;
  std::string  _modelMatrixName;

	GLuint _inverseModelMatrixID;
  std::string  _inverseModelMatrixName;

  // These are the default names of variables in the shaders.  Placed
  // here so they're all in one place, for easy comparison to the shader
  // you'll use.
  void setupDefaultNames() {
    _vertexAttribName = std::string("vertexPosition_modelspace");
    _uvAttribName = std::string("vertexUV");
    _normalAttribName = std::string("vertexNormal_modelspace");
    _colorAttribName = std::string("vertexInputColor");
    _projMatrixName = std::string("P");
    _viewMatrixName = std::string("V");
    _modelMatrixName = std::string("M");
    _inverseModelMatrixName = std::string("invM");
  }

public:
  mvShaderContext(mvShaderSet* shaderSet) {
    _mode = GL_TRIANGLES; // this is the default
    _shaderSet = shaderSet;
    _texture = NULL;
    setupDefaultNames();
  }
  mvShaderContext(mvShaderSet* shaderSet, mvTexture* texture) {
    _mode = GL_TRIANGLES; // this is the default
    _shaderSet = shaderSet;
    _texture = texture;
    setupDefaultNames();
  }    
  
  // Should we delete the texture here?
  ~mvShaderContext() {
    if (_texture) delete _texture;
    // Cleanup VBO and shader
    if (glIsBuffer(_vertexBufferID)) glDeleteBuffers(1, &_vertexBufferID);
    if (glIsBuffer(_uvBufferID))     glDeleteBuffers(1, &_uvBufferID);
    if (glIsBuffer(_normalBufferID)) glDeleteBuffers(1, &_normalBufferID);
    if (glIsBuffer(_colorBufferID))  glDeleteBuffers(1, &_colorBufferID);
    if (glIsVertexArray(_arrayID))   glDeleteVertexArrays(1, &_arrayID);
  };
  
  void load(const std::vector<MVec3> &vertices,
            const std::vector<MVec2> &uvs,
            const std::vector<MVec3> &normals,
            const std::vector<MVec3> &colors);
  void draw(const MMat4 &modelMatrix,
            const MMat4 &viewMatrix,
            const MMat4 &projectionMatrix);

  // A description of all the OpenGL drawing modes.
  //
  // GL_POINTS -- Treats each vertex as a single point. Vertex n
  //           defines point n. N points are drawn.
  //
  // GL_LINES -- Treats each pair of vertices as an independent line
  //           segment. Vertices 2 ⁢ n - 1 and 2 ⁢ n define line n. N 2
  //           lines are drawn.
  //
  // GL_LINE_STRIP -- Draws a connected group of line segments from
  //           the first vertex to the last. Vertices n and n + 1
  //           define line n. N - 1 lines are drawn.
  //
  // GL_LINE_LOOP -- Draws a connected group of line segments from the
  //           first vertex to the last, then back to the
  //           first. Vertices n and n + 1 define line n. The last
  //           line, however, is defined by vertices N and 1. N lines
  //           are drawn.
  //
  // GL_TRIANGLES -- Treats each triplet of vertices as an independent
  //           triangle. Vertices 3 ⁢ n - 2 , 3 ⁢ n - 1 , and 3 ⁢ n
  //           define triangle n. N 3 triangles are drawn.
  //
  // GL_TRIANGLE_STRIP -- Draws a connected group of triangles. One
  //           triangle is defined for each vertex presented after the
  //           first two vertices. For odd n, vertices n, n + 1 , and
  //           n + 2 define triangle n. For even n, vertices n + 1 ,
  //           n, and n + 2 define triangle n. N - 2 triangles are
  //           drawn.
  //
  // GL_TRIANGLE_FAN -- Draws a connected group of triangles. One
  //           triangle is defined for each vertex presented after the
  //           first two vertices. Vertices 1 , n + 1 , and n + 2
  //           define triangle n. N - 2 triangles are drawn.
  // 
  // GL_QUADS -- Treats each group of four vertices as an independent
  //           quadrilateral. Vertices 4 ⁢ n - 3 , 4 ⁢ n - 2 , 4 ⁢ n - 1,
  //           and 4 ⁢ n define quadrilateral n. N 4 quadrilaterals are
  //           drawn.
  //
  // GL_QUAD_STRIP -- Draws a connected group of quadrilaterals. One
  //           quadrilateral is defined for each pair of vertices
  //           presented after the first pair. Vertices 2 ⁢ n - 1 , 2 ⁢
  //           n , 2 ⁢ n + 2 , and 2 ⁢ n + 1 define quadrilateral n. N 2
  //           - 1 quadrilaterals are drawn. Note that the order in
  //           which vertices are used to construct a quadrilateral
  //           from strip data is different from that used with
  //           independent data.
  //
  // GL_POLYGON -- Draws a single, convex polygon. Vertices 1 through
  //           N define this polygon.



};

  
#endif
