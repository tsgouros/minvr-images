// my first program in C++
#include <iostream>

class mvImage {
 private:
  int imageData;

 public:
 mvImage(int n) : imageData(n) {};
  //  virtual ~mvImage();

  int getImage() { return imageData; };
};

