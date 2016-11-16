#include "mvImage.h"

// mvImageData here

mvImageData::mvImageData(std::string fileName) : _fileName(fileName) {

   // Data read from the header of the BMP file
   unsigned char header[54]; // Each BMP file begins by a 54-bytes header
   unsigned int dataPos;     // Position in the file where the actual data begins
   unsigned int width, height;
   unsigned int imageSize;   // = width*height*3
   // Actual RGB data
   unsigned char * data;

   std::cout << "Reading: " << fileName << std::endl;
   
   FILE * file = fopen(_fileName.c_str(),"rb");
   if (!file)
     throw std::runtime_error("Image could not be opened: " + fileName);

   if ( fread(header, 1, 54, file)!=54 ) // If not 54 bytes read : problem
     throw std::runtime_error(fileName + " is not a correct BMP file.");

   if ( header[0]!='B' || header[1]!='M' )
     throw std::runtime_error(fileName + " is not a correct BMP file.");

   // Read ints from the byte array
   dataPos    = *(int*)&(header[0x0A]);
   imageSize  = *(int*)&(header[0x22]);
   width      = *(int*)&(header[0x12]);
   height     = *(int*)&(header[0x16]);

   // Some BMP files are misformatted, guess missing information
   // 3 : one byte for each Red, Green and Blue component
   if (imageSize==0)    imageSize=width*height*3; 
   if (dataPos==0)      dataPos=54; // The BMP header is done that way

   // Create a buffer
   data = new unsigned char [imageSize];

   // Read the actual data from the file into the buffer
   fread(data,1,imageSize,file);

   //Everything is in memory now, the file can be closed
   fclose(file);

   // Create one OpenGL texture
   glGenTextures(1, &_gTextureID);

   // "Bind" the newly created texture : all future texture functions
   // will modify this texture
   glBindTexture(GL_TEXTURE_2D, _gTextureID);

   // Give the image to OpenGL
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                GL_BGR, GL_UNSIGNED_BYTE, data);

   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 }


// mvImageShape here

void mvImageShape::create() {};

void mvImageShape::draw(const mvImageData* img) {

  glPushMatrix();

  glBegin(GL_QUADS); // Begin drawing the color cube with 6 quads
  // Top face (y = 1.0f) 
  // Define vertices in counter-clockwise (CCW) order with normal pointing out
  glColor3f(0.0f, 0.5f, 0.0f);     // Green                                    
  glVertex3f( 1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f,  1.0f);
  glVertex3f( 1.0f, 1.0f,  1.0f);
  
  // Bottom face (y = -1.0f)                                                   
  glColor3f(0.5f, 0.25f, 0.0f);     // Orange                                  
  glVertex3f( 1.0f, -1.0f,  1.0f);
  glVertex3f(-1.0f, -1.0f,  1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f( 1.0f, -1.0f, -1.0f);

  // Front face  (z = 1.0f)                                                    
  glColor3f(0.5f, 0.0f, 0.0f);     // Red                                      
  glVertex3f( 1.0f,  1.0f, 1.0f);
  glVertex3f(-1.0f,  1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f( 1.0f, -1.0f, 1.0f);

  // Back face (z = -1.0f)                                                     
  glColor3f(0.5f, 0.5f, 0.0f);     // Yellow                                   
  glVertex3f( 1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f,  1.0f, -1.0f);
  glVertex3f( 1.0f,  1.0f, -1.0f);

  // Left face (x = -1.0f)                                                     
  glColor3f(0.0f, 0.0f, 0.5f);     // Blue                                     
  glVertex3f(-1.0f,  1.0f,  1.0f);
  glVertex3f(-1.0f,  1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f,  1.0f);

  // Right face (x = 1.0f)                                                     
  glColor3f(0.5f, 0.0f, 0.5f);     // Magenta                                  
  glVertex3f(1.0f,  1.0f, -1.0f);
  glVertex3f(1.0f,  1.0f,  1.0f);
  glVertex3f(1.0f, -1.0f,  1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);

  glEnd();  // End of drawing color-cube                                       
  glPopMatrix();

}

void mvImageShapeRectangle::setTransform(MinVR::VRMatrix4 transform) {

  _transform = transform;

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
    
  // Move rectangle to proper location, orientation.
  for (int i = 0; i < 4; i++ ) {
    MinVR::VRVector3 tmp = MinVR::VRVector3(_vertices[0 + 8*i],
                                            _vertices[1 + 8*i],
                                            _vertices[2 + 8*i]);
    MinVR::VRVector3 out = _transform * tmp;
    _vertices[0 + 8*i] = out.x;
    _vertices[1 + 8*i] = out.y;
    _vertices[2 + 8*i] = out.z;
  }
}

void mvImageShapeRectangle::create() {};

void mvImageShapeRectangle::draw(const mvImageData* img) {

  glPushMatrix();
  glMultMatrixd(_transform.m);
  glBegin(GL_QUADS); 
  // Define vertices in counter-clockwise (CCW) order with normal pointing out
  glColor3f(0.0f, 0.5f, 0.0f);     // Green
  for (int i = 0; i < 4; i++) {
    glVertex3f(_vertices[0 + 8*i], _vertices[1 + 8*i], _vertices[2 + 8*i]);
    glColor3f(_vertices[3 + 8*i], _vertices[4 + 8*i], _vertices[5 + 8*i]);
    glTexCoord2f(_vertices[6 + 8*i], _vertices[7 + 8*i]);
  }
  glEnd();  // End of drawing quad
  glPopMatrix();
  
}


// mvImage here
void mvImage::draw() {

  imageShape->draw(imageData);  
 
}

// mvImages here

// Loop through each of the image objects in the list and draw them.
void mvImages::draw() {
  for (imageMap::iterator it = images.begin();
       it != images.end(); it++) {
    it->second->draw();
  }
};

// Loop through each of the image objects in the list and create them.
// This involves initializing the vertex buffers and positions and so on.
void mvImages::create() {
  for (imageMap::iterator it = images.begin();
       it != images.end(); it++) {
    it->second->create();
  }
};

std::string mvImages::addImage(mvImage* image) {
  // generate a name
  std::string name = "newname";

  return addImage(name, image);
}

// Adds an image object to the collection to be drawn.
std::string mvImages::addImage(const std::string name, mvImage* image) {
  images[name] = image;
  return name;
}

int mvImages::delImage(const std::string name) {
  delete images[name];
  return images.erase(name);
}

mvImage* mvImages::getImage(const std::string name) {
  return images[name];
}

