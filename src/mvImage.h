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
  mvImageData(std::string fileName);
  virtual ~mvImageData() {};
  
  std::string getFileName() const { return _fileName; };

  void useTexture() {
    glBindTexture(GL_TEXTURE_2D, _gTextureID);
  };
};

// An object to hold the shape data for the object on which the
// texture image will be displayed.  Mostly this is just going to be a
// rectangle, but we leave it separated like this so we can experiment
// with different shapes later.
class mvImageShape {
 protected:
  MinVR::VRMatrix4 _transform;
  std::string _shape;
  GLuint _gBufferId;
  
 public:
  mvImageShape() {};
 mvImageShape(MinVR::VRMatrix4 transform) : _transform(transform) {};
  virtual ~mvImageShape() {};
  
  MinVR::VRMatrix4 getTransform() { return _transform; };
  void setTransform(MinVR::VRMatrix4 transform) { _transform = transform; };

  void setShape(std::string shape) { _shape = shape; };
  std::string getShape() { return _shape; };
  
  // The draw() function here has imageData as an input and draws each
  // polygon of the shape after first checking with the imageData
  // object for the appropriate mipmap.
  virtual void draw(const mvImageData* img);

  // Create is used to create the vertex buffers and set the pointers for
  // the shape.  Initializing the vertex arrays can go in there, too, but that
  // probably should have been done in the constructor.
  virtual void create();
};

class mvImageShapeRectangle : public mvImageShape {
 protected:
  double _height, _width;
  mvPoint _normal;
  GLfloat _vertices[32];
  
 public:
 mvImageShapeRectangle(MinVR::VRMatrix4 transform, double height, double width) :
  mvImageShape(transform), _height(height), _width(width) {
    
    setShape("rectangle");
    setTransform(transform);
  };

  void setTransform(MinVR::VRMatrix4 transform);
  void draw(const mvImageData* img);
  void create();
};

// A class to hold an image object.  This is a 3D object located in
// space somewhere, textured with an input image read from a file.
// The image data is kept in a separate object, mvImageData.
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

  double gamma, gmin, gmax;
  // The imageData object contains the image.
  mvImageData* imageData;
  // imageShape contains the shape and the location in space.
  mvImageShape* imageShape;
  
 public:
  mvImage(const std::string fileName) {
   imageData = new mvImageData(fileName);
   imageShape = new mvImageShape();
 };
  virtual ~mvImage() {};

  std::string getFileName() const { return imageData->getFileName(); };

  mvImageData* getImage() { return imageData; };
  void setImage(mvImageData* imgData) { imageData = imgData; };
  
  mvImageShape* getShape() { return imageShape; };
  void setShape(mvImageShape* shapeData) { imageShape = shapeData; };

  void create() { imageShape->create(); };
  
  virtual void draw();
};

// Holds a collection of mvImage objects.
class mvImages {
 protected:
  // We store the images in a map, so we can reference them by name or
  // ID string.
  typedef std::map<std::string, mvImage *> imageMap;
  imageMap images;

 public:
  void create();
  void draw();
  std::string addImage(mvImage* image);
  std::string addImage(const std::string name, mvImage* image);
  int delImage(const std::string name);
  mvImage* getImage(const std::string name);
};

#endif
