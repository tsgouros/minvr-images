#include "mvImage.h"

// mvImageData here

// mvImageShape here

void mvImageShape::draw(const mvImageData* img) {

  glPushMatrix();

  glTranslatef(1.5f, 4.0f, 0.0f);  // Move right and into the screen           

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

