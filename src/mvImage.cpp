#include "mvImage.h"

namespace mvImage {
// mvImageData here

// mvImageShape here

// mvImage here


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
  return images.erase(name);
}

mvImage* mvImages::getImage(const std::string name) {
  return images[name];
}

} // namespace mvImage
