#ifndef MVIMAGE_H
#define MVIMAGE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <map>

#include "MVR.h"

#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>

#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define TWOPI 6.283185

typedef enum {
  IMG_RECTANGLE   = 0,
  IMG_BOX         = 1,
  IMG_SPHERE      = 2,

  NSHAPETYPES
} IMG_SHAPEID;

typedef std::map<IMG_SHAPEID, std::string> mvShapeMap;

class mvPoint {
 public:
  double x;
  double y;
  double z;

  void set(MinVR::VRDoubleArray p) { x = p[0]; y = p[1]; z = p[2]; };
  void set(MinVR::VRIntArray p) {
    x = (double)p[0]; y = (double)p[1]; z = (double)p[2]; };
};

class mvQuat {
 public:
  double x;
  double y;
  double z;
  double w;

  void set(MinVR::VRDoubleArray p) { x = p[0]; y = p[1]; z = p[2]; w = p[3]; };
  void set(MinVR::VRIntArray p) {
    x = (double)p[0]; y = (double)p[1]; z = (double)p[2]; w = (double)p[3]; };
};

enum mvDrawBuffer { UNSET, BOTH, LEFT, RIGHT };
enum mvLayerType {LAYER, IMAGE, SHAPE, GROUP };

// A class to hold some image data, probably retrieved from a file somewhere
// that might have been in any of a variety of different formats.
class mvImageData {
 protected:
  std::string _fileName;

  GLuint _gTextureID;

  friend std::ostream & operator<<(std::ostream &os, const mvImageData& iData);
  
 public:
  mvImageData(std::string name, MinVR::VRDataIndex* index);
  virtual ~mvImageData() {};
  
  std::string getFileName() const { return _fileName; };

  void useTexture() {
    std::cout << "use texture from: " << _fileName << std::endl;
  };

  std::string print() const;
    
};

// An object to hold the shape data for the object on which the
// texture image will be displayed.  Mostly this is just going to be a
// rectangle, but we leave it separated like this so we can experiment
// with different shapes later.  
class mvImageShape {
 protected:
  GLdouble _posx, _posy, _posz;
  GLdouble _angle;
  GLdouble _rotx, _roty, _rotz;
  GLdouble _scalex, _scaley, _scalez;
  GLuint _gBufferId, _gIndexId;
  
  IMG_SHAPEID _shapeID; 

  static mvShapeMap _shapeMap;
  static void _initializeShapeMap() {
    _shapeMap[IMG_RECTANGLE] = "rectangle";
    _shapeMap[IMG_BOX]       = "box";
    _shapeMap[IMG_SPHERE]    = "sphere";
  };
  
  friend std::ostream & operator<<(std::ostream &os, const mvImageShape& iShape);

  struct mvVertex {
    GLfloat x, y, z;
    GLfloat nx, ny, nz;
    GLfloat u, v;
    GLfloat r, g, b, a;
  };
  // These are index values for the glVertexAttribPointer() calls.
  static const GLuint _vptr = 0, _nptr = 1, _tptr = 2, _cptr = 3;
  
 public:
  mvImageShape() { init(); };
  mvImageShape(std::string name, MinVR::VRDataIndex* index); 

  virtual std::string print() const;
  
  void init() {
    _initializeShapeMap();
    _scalex = 1.0; _scaley = 1.0; _scalez = 1.0;
    _angle = 0.0;
    _rotx = 0.0; _roty = 0.0; _rotz = 1.0;
    _posx = 0.0; _posy = 0.0; _posz = 0.0;
  }
  
  virtual ~mvImageShape() {};

  // Here are the basic accessors and mutators.
  void setShape(std::string shape);
  std::string getShape() { return _shapeMap[_shapeID]; };

  void setShapeID(IMG_SHAPEID shapeID) { _shapeID = shapeID; };
  IMG_SHAPEID getShapeID() { return _shapeID; };

  void setScale(GLdouble scalex, GLdouble scaley, GLdouble scalez) {
    _scalex = scalex; _scaley = scaley; _scalez = scalez; };
  MinVR::VRVector3 getScale() {
    return MinVR::VRVector3(_scalex, _scaley, _scalez); };

  void setRotation(GLdouble angle, GLdouble rotx, GLdouble roty, GLdouble rotz) {
    _angle = angle; _rotx = rotx; _roty = roty; _rotz = rotz; };
  GLdouble getAngle() { return _angle; };
  MinVR::VRVector3 getRotAxis() { return MinVR::VRVector3(_rotx, _roty, _rotz); }

  void setPosition(GLdouble x, GLdouble y, GLdouble z) {
    _posx = x; _posy = y; _posz = z; };
  MinVR::VRVector3 getPosition() {
    return MinVR::VRVector3(_posx, _posy, _posz); };
  
  // Create is used to create the vertex buffers and set the pointers for
  // the shape.  Initializing the vertex arrays can go in there, too, but that
  // probably should have been done in the constructor.
  virtual void create();

  // Call this just before draw to submit the model matrix to the
  // vertex shader before drawing.  Composes the scale, rotate, and
  // transpose values into a model matrix, and returns the result.
  // Note that this has to be converted into a matrix of float values
  // in order to be useful to many OpenGL implementations.  Or at
  // least the implementation on my Mac.
  MinVR::VRMatrix4 getModelMatrix();
  
  // The draw() function here has imageData as an input and draws each
  // polygon of the shape after first checking with the imageData
  // object for the appropriate mipmap.
  virtual void draw(const mvImageData* img);

};

class mvImageShapeRectangle : public mvImageShape {
 protected:
  double _height, _width;
  mvPoint _normal;
  mvVertex _vertices[4];
  GLuint _indices[4];

  friend std::ostream & operator<<(std::ostream &os,
                                   const mvImageShapeRectangle& iShape);
  
 public:
 mvImageShapeRectangle(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("rectangle");
    setShapeID(IMG_RECTANGLE);

    _width = index->getValue(name + "/width");
    _height = index->getValue(name + "/height");
    
  };

  void setModelMatrix(MinVR::VRMatrix4 modelMatrix);
  void draw(const mvImageData* img);
  void create();

  std::string print() const;
};

mvImageShape* createMvImageShapeRectangle(std::string shapeName,
                                          MinVR::VRDataIndex* index);

class mvImageShapeBox : public mvImageShape {
 protected:
  double _height, _width, _depth;
  mvPoint _normal;
  GLfloat _vertices[32];

  friend std::ostream & operator<<(std::ostream &os,
                                   const mvImageShapeBox& iShape);
  
 public:
 mvImageShapeBox(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("box");
    setShapeID(IMG_BOX);

    _height = index->getValue(name + "/height");
    _width = index->getValue(name + "/width");
    _depth = index->getValue(name + "/depth");
    
  };

  void setModelMatrix(MinVR::VRMatrix4 modelMatrix);
  void draw(const mvImageData* img);
  void create();

  std::string print() const;
};

mvImageShape* createMvImageShapeBox(std::string shapeName,
                                    MinVR::VRDataIndex* index);

class mvImageShapeSphere : public mvImageShape {
 protected:
  GLdouble _radius;
  GLint _slices, _stacks;
  mvPoint _normal;
  GLfloat _vertices[32];

  void makeSolidSphere(GLdouble radius, GLint slices, GLint stacks);
  void circleTable(double **sint,double **cost,const int n);

  friend std::ostream & operator<<(std::ostream &os,
                                   const mvImageShapeSphere& iShape);
  
 public:
 mvImageShapeSphere(GLdouble radius, GLint slices=10, GLint stacks=10) :
  _radius(radius), _slices(slices), _stacks(stacks) {
    setShape("sphere");
    setShapeID(IMG_SPHERE);    
  };

 mvImageShapeSphere(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("sphere");
    setShapeID(IMG_SPHERE);

    _radius = index->getValue(name + "/radius");
    if (index->exists(name + "/slices")) {
      _slices = index->getValue(name + "/slices");
    } else {
      _slices = 10;
    }
    if (index->exists(name + "/stacks")) {
      _stacks = index->getValue(name + "/stacks");
    } else {
      _stacks = 10;
    }    
  };

  void draw(const mvImageData* img);
  void create();

  std::string print() const;
};

mvImageShape* createMvImageShapeSphere(std::string shapeName,
                                       MinVR::VRDataIndex* index);


class mvImageShapeFactory {

 public:
  typedef mvImageShape* (*createMvImageShapeCallback)(std::string shapeName,
                                                      MinVR::VRDataIndex* index);

  mvImageShape* createMvImageShape(std::string name, MinVR::VRDataIndex* index);

  bool registerMvImageShape(std::string shapeType,
                           createMvImageShapeCallback creator);

 private:
  // A different callback for each shape.
  typedef std::map<std::string, createMvImageShapeCallback> callbackMap;
  // A map to hold them in.
  callbackMap _callbacks;
  
};

// A class to hold an image object.  This is a 3D object located in
// space somewhere, textured with an input image read from a file.
// The image data -- the texture itself -- is kept in a separate
// object, mvImageData.
class mvImage {
  
 protected:

  double _alpha;
  double _scale;
  bool _visible;
  double meterspp;

  std::string _name;
  int _group;
  mvDrawBuffer _drawBuffer;
  mvLayerType _type;

  //  double gamma, gmin, gmax;
  // The imageData object contains the image.
  mvImageData* _imageData;
  // imageShape contains the shape and the location in space.
  mvImageShape* _imageShape;

  friend std::ostream & operator<<(std::ostream &os, const mvImage& image);
  
 public:
  mvImage(const std::string name, MinVR::VRDataIndex* index, mvImageShape* shape) {
    _imageData = new mvImageData(name, index);
    _imageShape = shape;
  };
 mvImage(mvImageData* imageData, mvImageShape* imageShape) :
  _imageData(imageData), _imageShape(imageShape) {};
  virtual ~mvImage() {};

  std::string getFileName() const {
    if (_imageData != NULL)
      return _imageData->getFileName();
    else
      return std::string("");
  };

  mvImageData* getImage() { return _imageData; };
  void setImage(mvImageData* imgData) { _imageData = imgData; };
  
  mvImageShape* getShape() { return _imageShape; };
  void setShape(mvImageShape* shapeData) { _imageShape = shapeData; };

  void create() { _imageShape->create(); };
  
  virtual void draw();

  std::string print() const;
};


#endif
