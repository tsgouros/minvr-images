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

enum mvDrawBuffer { UNSET, BOTH, LEFT, RIGHT };
enum mvLayerType {LAYER, IMAGE, SHAPE, GROUP };

// A class to hold some image data, probably retrieved from a file somewhere
// that might have been in any of a variety of different formats.
class mvImageData {
 protected:
  std::string _fileName;

 public:
 mvImageData(const std::string fileName) : _fileName(fileName) {};
  virtual ~mvImageData() {};
  
  std::string getFileName() { return _fileName; };

};

// An object to hold the shape data for the object on which the
// texture image will be displayed.  Mostly this is just going to be a
// rectangle, but we leave it separated like this so we can experiment
// with different shapes later.
class mvImageShape {
 protected:
  mvPoint _center;
  MinVR::VRDoubleArray _dims;
  std::string _shape;

 public:
  mvPoint getCenter() { return _center; };
  void setCenter(mvPoint center) { _center = center; };

  virtual void setDims(MinVR::VRDoubleArray dims) {_dims = dims; };
  void setShape(std::string shape) { _shape = shape; };

  std::string getShape() { return _shape; };
  MinVR::VRDoubleArray getDims() { return _dims; };
  
  // The draw() function here has imageData as an input and draws each
  // polygon of the shape after first checking with the imageData
  // object for the appropriate mipmap.
  virtual void draw(const mvImageData* img);
  
};


// A class to hold an image object.  This is a 3D object located in
// space somewhere, textured with an input image read from a file.
// The image data is kept in a separate object, mvImageData.
class mvImage {
 protected:
  double _alpha;
  double _scale;
  bool _visible;
  mvPoint _center;
  double meterspp;

  std::string _name;
  int _group;
  mvDrawBuffer _drawBuffer;
  mvLayerType _type;

  double gamma, gmin, gmax;
  MinVR::VRMatrix4 transform;
  mvImageData* imageData;
  mvImageShape* imageShape;
  
 public:
  mvImage(const std::string fileName) {
   imageData = new mvImageData(fileName);
   imageShape = new mvImageShape();
 };
  virtual ~mvImage() {};

  mvImageData* getImage() { return imageData; };
  std::string getFileName() { return imageData->getFileName(); };
  void setImage(mvImageData* imgData) { imageData = imgData; };
  
  mvImageShape* getShape() { return imageShape; };
  void setImage(mvImageShape* shapeData) { imageShape = shapeData; };
  
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
  void draw();
  std::string addImage(mvImage* image);
  std::string addImage(const std::string name, mvImage* image);
  int delImage(const std::string name);
  mvImage* getImage(const std::string name);
};

#endif
