#ifndef MVIMAGE_H
#define MVIMAGE_H

#include <iostream>
#include <string>
#include <math.h>
#include <map>

#include "MVR.h"

#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define TWOPI 6.283185

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

 public:
  mvImageData(std::string name, MinVR::VRDataIndex* index);
  virtual ~mvImageData() {};
  
  std::string getFileName() const { return _fileName; };

  void useTexture() {
    std::cout << "use texture from: " << _fileName << std::endl;
  };
};

// An object to hold the shape data for the object on which the
// texture image will be displayed.  Mostly this is just going to be a
// rectangle, but we leave it separated like this so we can experiment
// with different shapes later.  
class mvImageShape {
 protected:
  GLdouble _scale;
  GLdouble _angle;
  GLdouble _rotx, _roty, _rotz;
  GLdouble _transx, _transy, _transz;

  std::string _shape;
  GLuint _gBufferId;
  
 public:
  mvImageShape() { init(); };
  mvImageShape(std::string name, MinVR::VRDataIndex* index); 

  void init() {
    _scale = 1.0;
    _angle = 0.0;
    _rotx = 0.0; _roty = 0.0; _rotz = 1.0;
    _transx = 0.0; _transy = 0.0; _transz = 0.0;
  }
  
  virtual ~mvImageShape() {};
  
  void setShape(std::string shape) { _shape = shape; };
  std::string getShape() { return _shape; };

  void setScale(GLdouble scale) { _scale = scale; };
  GLdouble getScale() { return _scale; };

  void setRotation(GLdouble angle, GLdouble rotx, GLdouble roty, GLdouble rotz) {
    _angle = angle; _rotx = rotx; _roty = roty; _rotz = rotz; };
  GLdouble getAngle() { return _angle; };
  MinVR::VRVector3 getRotAxis() { return MinVR::VRVector3(_rotx, _roty, _rotz); }

  void setPosition(GLdouble x, GLdouble y, GLdouble z) {
    _transx = x; _transy = y; _transz = z; };
  MinVR::VRVector3 getPosition() {
    return MinVR::VRVector3(_transx, _transy, _transz); };
  
  // Create is used to create the vertex buffers and set the pointers for
  // the shape.  Initializing the vertex arrays can go in there, too, but that
  // probably should have been done in the constructor.
  virtual void create();

  // The draw() function here has imageData as an input and draws each
  // polygon of the shape after first checking with the imageData
  // object for the appropriate mipmap.
  virtual void draw(const mvImageData* img);
};

class mvImageShapeRectangle : public mvImageShape {
 protected:
  double _height, _width;
  mvPoint _normal;
  GLfloat _vertices[32];
  
 public:
 mvImageShapeRectangle(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("rectangle");

    _width = index->getValue(name + "/width");
    _height = index->getValue(name + "/height");
    
  };

  void setModelMatrix(MinVR::VRMatrix4 modelMatrix);
  void draw(const mvImageData* img);
  void create();
};

mvImageShape* createMvImageShapeRectangle(std::string shapeName,
                                          MinVR::VRDataIndex* index);

class mvImageShapeBox : public mvImageShape {
 protected:
  double _height, _width, _depth;
  mvPoint _normal;
  GLfloat _vertices[32];
  
 public:
 mvImageShapeBox(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("box");

    _height = index->getValue(name + "/height");
    _width = index->getValue(name + "/width");
    _depth = index->getValue(name + "/depth");
    
  };

  void setModelMatrix(MinVR::VRMatrix4 modelMatrix);
  void draw(const mvImageData* img);
  void create();
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
  
 public:
 mvImageShapeSphere(GLdouble radius, GLint slices=10, GLint stacks=10) :
  _radius(radius), _slices(slices), _stacks(stacks) {
    setShape("sphere");
  };

 mvImageShapeSphere(std::string name, MinVR::VRDataIndex* index) :
  mvImageShape(name, index) {
    setShape("sphere");

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

  void setScale(GLdouble scale) { _imageShape->setScale(scale); };
  GLdouble getScale() { return _imageShape->getScale(); };

  void setRotation(GLdouble angle, GLdouble rotx, GLdouble roty, GLdouble rotz) {
    _imageShape->setRotation(angle, rotx, roty, rotz); };
  GLdouble getAngle() { return _imageShape->getAngle(); };
  MinVR::VRVector3 getRotAxis() { return _imageShape->getRotAxis(); }

  void setPosition(GLdouble x, GLdouble y, GLdouble z) {
    _imageShape->setPosition(x, y, z); };
  MinVR::VRVector3 getPosition() {
    return _imageShape->getPosition(); };
};


// Holds a collection of mvImage objects.
class mvImages {
 protected:
  mvImageShapeFactory _shapeFactory;

  // We store the images in a map, so we can reference them by name or
  // ID string.
  typedef std::map<std::string, mvImage *> imageMap;
  imageMap images;

 public:
  mvImages();
  ~mvImages();
  
  void create();
  void draw();

  // We can add images either by reference to the input data index or
  // by fiat.
  std::string addImage(const std::string name, MinVR::VRDataIndex* index);
  std::string addImage(const std::string name,
                       mvImageData* image, mvImageShape* shape);
  int delImage(const std::string name);
  mvImage* getImage(const std::string name);
};

#endif
