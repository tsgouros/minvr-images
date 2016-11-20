#include "mvImage.h"

// mvImageData here

mvImageData::mvImageData(std::string name, MinVR::VRDataIndex* index) {

  _fileName = (std::string)index->getValue(name + "/imageFile");   

  std::cout << "read image data from file: " << _fileName << std::endl;

 }

// mvImageShape here

// This is here just to satisfy the linker.  The definition is in the
// header file, and so is the initialization.  But without this line,
// the linker bombs, go figure.
mvShapeMap mvImageShape::_shapeMap;

// This is a reverse lookup of the shapeID map, to find the shape ID from
// the string representation (such as might be in the XML config file).
void mvImageShape::setShape(std::string shape) {
  for (mvShapeMap::iterator it = _shapeMap.begin(); it != _shapeMap.end(); it++) {
    if (it->second.compare(shape) == 0) {
      setShapeID(it->first);
      return;
    }
  }
  throw std::runtime_error("no such shape in the catalog: " + shape);
}

// Initialize the shape from a config file.
mvImageShape::mvImageShape(std::string name, MinVR::VRDataIndex* index) {

  // Set the default values for the configurable position, rotation, and scale.
  init();
  setShape(index->getValue(name +  "/shape"));

  if (index->exists(name + "/scale")) {
    // Read in a double array.  If scale is actually a scalar, it will
    // come in as a one-element array.  This way we can handle a
    // 3-item scale vector or a scalar.
    MinVR::VRDoubleArray v = index->getValue(name + "/scale");

    if (v.size() > 1) {
      _scalex = v[0]; _scaley = v[1]; _scalez = v[2];
    } else {
      _scalex = v[0]; _scaley = v[0]; _scalez = v[0];
    }      
  }

  if (index->exists(name + "/angle"))
    _angle = (GLdouble)index->getValue(name + "/angle");

  if (index->exists(name + "/rotAxis")) {
    MinVR::VRVector3 v = MinVR::VRVector3(index->getValue(name + "/rotAxis"));
    _rotx = v[0]; _roty = v[1]; _rotz = v[2];
  }
  
  if (index->exists(name + "/position")) {
    MinVR::VRVector3 v = MinVR::VRVector3(index->getValue(name + "/position"));
    _posx = v[0]; _posy = v[1]; _posz = v[2];
  }
}


void mvImageShape::create() {
  
  std::cout << "create default object (should not happen)" << std::endl;

};

void mvImageShape::draw(const mvImageData* img) {

  std::cout << "draw default object (should not happen)" << std::endl;

}



// mvImage here
void mvImage::draw() {

  std::cout << "drawing image " << _name << " on a " << _imageShape->getShape() << " with texture from " << getFileName() << "." << std::endl;
  _imageShape->draw(_imageData);  
 
}

// mvImageShapeFactory here
mvImageShape* mvImageShapeFactory::createMvImageShape(std::string name,
                                                      MinVR::VRDataIndex* index) {

  std::string shapeType = index->getValue(name + "/shape");
  
  callbackMap::const_iterator it = _callbacks.find(shapeType);

  std::cout << "trying to create a: " << shapeType << std::endl;
  std::cout << "with the name of: " << name << std::endl;

  if (it == _callbacks.end()) {
    throw std::runtime_error("unknown shape: " + shapeType);
  } else {
    // Call the create function in the shape map.
    return (it->second)(name, index);
  }
}

bool mvImageShapeFactory::registerMvImageShape(std::string shapeType,
                                              createMvImageShapeCallback creator) {
  std::cout << "registering a callback for: " << shapeType << std::endl;

  // Returns true if type is successfully registered, false if it was
  // already registered.
  return _callbacks.insert(callbackMap::value_type(shapeType, creator)).second;
}


// Rectangle specialization.
void mvImageShapeRectangle::create() {

  std::cout << "load rectangle vertices " << _height << "x" << _width << std::endl;

    // Positions
  _vertices[0]  =  1.0f; _vertices[1]  =  1.0f; _vertices[2]  = 0.0f; // UR
  _vertices[8]  =  1.0f; _vertices[9]  = -1.0f; _vertices[10] = 0.0f; // LR
  _vertices[16] = -1.0f; _vertices[17] = -1.0f; _vertices[18] = 0.0f; // LL
  _vertices[24] = -1.0f; _vertices[25] =  1.0f; _vertices[26] = 0.0f; // UL

  _vertices[0]  *= _width;
  _vertices[8]  *= _width;
  _vertices[16] *= _width;
  _vertices[24] *= _width;

  _vertices[1]  *= _height;
  _vertices[9]  *= _height;
  _vertices[17] *= _height;
  _vertices[25] *= _height;

  // Colors
  _vertices[3]  = 1.0f; _vertices[4]  = 0.0f; _vertices[5]  = 0.0f;
  _vertices[11] = 0.0f; _vertices[12] = 1.0f; _vertices[13] = 0.0f;
  _vertices[19] = 0.0f; _vertices[20] = 0.0f; _vertices[21] = 1.0f;
  _vertices[27] = 1.0f; _vertices[28] = 1.0f; _vertices[29] = 0.0f;

  // Texture coordinates
  _vertices[6]  = 1.0f; _vertices[7]  = 1.0f;
  _vertices[14] = 1.0f; _vertices[15] = 1.0f;
  _vertices[22] = 1.0f; _vertices[23] = 1.0f;
  _vertices[30] = 1.0f; _vertices[31] = 1.0f;
  
};

void mvImageShapeRectangle::draw(const mvImageData* img) {

  std::cout << "draw rectangle" << std::endl;
  
}

mvImageShape* createMvImageShapeRectangle(std::string shapeName,
                                          MinVR::VRDataIndex* index) {
  mvImageShapeRectangle* out = new mvImageShapeRectangle(shapeName, index);
  return (mvImageShape*)out;  
}

// Box specialization.
void mvImageShapeBox::create() {

  std::cout << "load box vertices " << _height << "x" << _width << "x" << _depth << std::endl;

  // Positions
  _vertices[0]  =  1.0f; _vertices[1]  =  1.0f; _vertices[2]  = 0.0f; // UR
  _vertices[8]  =  1.0f; _vertices[9]  = -1.0f; _vertices[10] = 0.0f; // LR
  _vertices[16] = -1.0f; _vertices[17] = -1.0f; _vertices[18] = 0.0f; // LL
  _vertices[24] = -1.0f; _vertices[25] =  1.0f; _vertices[26] = 0.0f; // UL

  _vertices[0]  *= _width;
  _vertices[8]  *= _width;
  _vertices[16] *= _width;
  _vertices[24] *= _width;

  _vertices[1]  *= _height;
  _vertices[9]  *= _height;
  _vertices[17] *= _height;
  _vertices[25] *= _height;

  // Colors
  _vertices[3]  = 1.0f; _vertices[4]  = 0.0f; _vertices[5]  = 0.0f;
  _vertices[11] = 0.0f; _vertices[12] = 1.0f; _vertices[13] = 0.0f;
  _vertices[19] = 0.0f; _vertices[20] = 0.0f; _vertices[21] = 1.0f;
  _vertices[27] = 1.0f; _vertices[28] = 1.0f; _vertices[29] = 0.0f;

  // Texture coordinates
  _vertices[6]  = 1.0f; _vertices[7]  = 1.0f;
  _vertices[14] = 1.0f; _vertices[15] = 1.0f;
  _vertices[22] = 1.0f; _vertices[23] = 1.0f;
  _vertices[30] = 1.0f; _vertices[31] = 1.0f;

};

void mvImageShapeBox::draw(const mvImageData* img) {

  std::cout << "draw box" << std::endl;
  
}

mvImageShape* createMvImageShapeBox(std::string shapeName,
                                    MinVR::VRDataIndex* index) {
  mvImageShapeBox* out = new mvImageShapeBox(shapeName, index);
  return (mvImageShape*)out;
}

// Pre-calculate a bunch of trig values for the sphere generation.
void mvImageShapeSphere::circleTable(double **sint, double **cost, const int n) {
  int i;

  // Table size, the sign of n flips the circle direction.
  const int size = abs(n);

  // Determine the angle between samples.
  const double angle = 2*M_PI/(double)( ( n == 0 ) ? 1 : n );

  // Allocate memory for n samples, plus duplicate of first entry at the end.
  *sint = (double *) calloc(sizeof(double), size + 1);
  *cost = (double *) calloc(sizeof(double), size + 1);

  // Bail out if memory allocation fails.
  if (!(*sint) || !(*cost)) {
    free(*sint);
    free(*cost);
    throw std::runtime_error("Failed to allocate memory in circleTable");
  }

  // Compute cos and sin around the circle.
  (*sint)[0] = 0.0;
  (*cost)[0] = 1.0;

  for (i=1; i<size; i++) {
    (*sint)[i] = sin(angle*i);
    (*cost)[i] = cos(angle*i);
  }

  // Last sample is duplicate of the first, to close the circle.
  (*sint)[size] = (*sint)[0];
  (*cost)[size] = (*cost)[0];
}

// Draws a solid sphere.  Stolen from freeglut.

void mvImageShapeSphere::create() {

  std::cout << "load sphere vertices r=" << _radius << std::endl;

  int i,j;

  // Adjust z and radius as stacks are drawn. 

  double z0,z1;
  double r0,r1;

  // Pre-computed circle 

  double *sint1,*cost1;
  double *sint2,*cost2;

  circleTable(&sint1, &cost1, -_slices);
  circleTable(&sint2, &cost2, _stacks*2);

  // The top stack is covered with a triangle fan.
  z0 = 1.0;
  z1 = cost2[(_stacks > 0) ? 1 : 0];
  r0 = 0.0;
  r1 = sint2[(_stacks > 0) ? 1 : 0];

  std::cout << "draw sphere top" << std::endl;
  
  // glBegin(GL_TRIANGLE_FAN);

  // glNormal3d(0,0,1);
  // glVertex3d(0,0,radius);

  // for (j=slices; j>=0; j--) {
  //   glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
  //   glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
  // }

  // glEnd();

  // Cover each stack with a quad strip, except the top and bottom stacks. 
  for (i = 1; i < _stacks - 1; i++ ) {
    z0 = z1; z1 = cost2[i + 1];
    r0 = r1; r1 = sint2[i + 1];

    std::cout << "draw sphere middle" << std::endl;
    
    // glBegin(GL_QUAD_STRIP);

    // for(j=0; j<=slices; j++) {
    //   glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
    //   glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
    //   glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
    //   glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
    // }

    // glEnd();
  }

  // The bottom stack is covered with a triangle fan 
  z0 = z1;
  r0 = r1;

  std::cout << "draw sphere bottom" << std::endl;
  
  // glBegin(GL_TRIANGLE_FAN);

  // glNormal3d(0,0,-1);
  // glVertex3d(0,0,-radius);

  // for (j=0; j<=slices; j++) {
  //   glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
  //   glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
  // }

  // glEnd();

  // Release sin and cos tables.
  free(sint1);
  free(cost1);
  free(sint2);
  free(cost2);
}

void mvImageShapeSphere::draw(const mvImageData* img) {

  std::cout << "draw sphere" << std::endl;
  
}




mvImageShape* createMvImageShapeSphere(std::string shapeName,
                                       MinVR::VRDataIndex* index) {
  mvImageShapeSphere* out = new mvImageShapeSphere(shapeName, index);
  return (mvImageShape*)out;
}

// mvImages here
mvImages::mvImages() {
  _shapeFactory.registerMvImageShape("rectangle", createMvImageShapeRectangle);
  _shapeFactory.registerMvImageShape("box", createMvImageShapeBox);
  _shapeFactory.registerMvImageShape("sphere", createMvImageShapeSphere);
}

mvImages::~mvImages() {
  for (imageMap::iterator it = _images.begin();
       it != _images.end(); it++) {
    delete it->second;
  }
}

// Loop through each of the image objects in the list and draw them.
void mvImages::draw() {
  std::cout << "draw objects" << std::endl;
  for (imageMap::iterator it = _images.begin();
       it != _images.end(); it++) {
    std::cout << "drawing: " << it->first << std::endl;
    it->second->draw();
  }
};

// Loop through each of the image objects in the list and create them.
// This involves initializing the vertex buffers and positions and so on.
void mvImages::create() {
  std::cout << "create objects" << std::endl;

  for (imageMap::iterator it = _images.begin();
       it != _images.end(); it++) {
    std::cout << "creating: " << it->first << std::endl;
    it->second->create();
  }
};

// Adds an image object to the collection to be drawn.
std::string mvImages::addImage(const std::string name,
                               MinVR::VRDataIndex* index) {

  std::cout << "adding a new image with the name: " << name << std::endl;
  _images[name] = new mvImage(name, index,
                             _shapeFactory.createMvImageShape(name, index));
  return name;
}

// We do envision mvImage objects with null imageData.  Presumably
// there is a color specified via the imageShape.
std::string mvImages::addImage(const std::string name,
                               mvImageData* image, mvImageShape* shape) {

  std::cout << "adding a new image (" << shape->getShape() << ") with the name: " << name << std::endl;
  
  _images[name] = new mvImage(image, shape);
  return name;
}


int mvImages::delImage(const std::string name) {
  delete _images[name];
  return _images.erase(name);
}

mvImage* mvImages::getImage(const std::string name) {
  return _images[name];
}


std::string mvImageData::print() const {
  std::stringstream out;
  
  out <<  "imageData object from filename: " + _fileName << std::endl;

  return out.str();
}

std::ostream & operator<<(std::ostream &os, const mvImageData& iData) {
  return os << iData.print();
}

std::string mvImageShape::print() const {
  std::stringstream out;

  out << "This is a " << _shapeMap[_shapeID] << std::endl;
  out << "position: (" << _posx << ", " << _posy << ", " << _posz << ")" << std::endl;
  if (_angle != 0.0)
    out << "rotation: " << _angle << " radians around (" << _rotx << ", " << _roty << ", " << _rotz << ")" << std::endl;

  if ((_scalex * _scaley * _scalez) != 1.0) 
    out << "scale:    (" << _scalex  << ", " << _scaley << ", " << _scalez << ")" << std::endl;

  return out.str();
}
  
std::ostream & operator<<(std::ostream &os, const mvImageShape& iShape) {
  return os << iShape.print();
}

std::string mvImageShapeRectangle::print() const {
  std::stringstream out;

  out << mvImageShape::print();
  out << "dimensions: " << _height << " (h) X " << _width << " (w)" << std::endl;

  return out.str();
}

std::ostream & operator<<(std::ostream &os, const mvImageShapeRectangle& iShape) {
  return os << iShape.print();
}

std::string mvImageShapeBox::print() const {
  std::stringstream out;

  out << mvImageShape::print();
  out << "dimensions: " << _height << " (h) X " << _width << " (w) X " << _depth << " (d)" << std::endl;

  return out.str();
}

std::ostream & operator<<(std::ostream &os, const mvImageShapeBox& iShape) {
  return os << iShape.print();
}

std::string mvImageShapeSphere::print() const {
  std::stringstream out;

  out << mvImageShape::print();
  out << "radius: " << _radius << std::endl;

  return out.str();
}

std::ostream & operator<<(std::ostream &os, const mvImageShapeSphere& iShape) {
  return os << iShape.print();
}

std::string mvImage::print() const {
  std::stringstream out;

  if (_imageData) out << _imageData->print();


  switch (_imageShape->getShapeID()) {

  case IMG_RECTANGLE:
    out << ((mvImageShapeRectangle*)_imageShape)->print();
      break;

    case IMG_BOX:
      out << ((mvImageShapeBox*)_imageShape)->print();
      break;

    case IMG_SPHERE:
      out << ((mvImageShapeSphere*)_imageShape)->print();
      break;

    default:
      out << _imageShape->print();
  }
  
  return out.str();
}


std::ostream & operator<<(std::ostream &os, const mvImage& image) {
  return os << image.print();
}

std::string mvImages::print() const {
  std::stringstream out;

  for (imageMap::const_iterator it = _images.begin(); it != _images.end(); it++) {

    out << it->first << std::endl;
    out << it->second->print();

  }
  
  return out.str();
}



std::ostream & operator<<(std::ostream &os, const mvImages& _images) {
  return os << _images.print();
}
