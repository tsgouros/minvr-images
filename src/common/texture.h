#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <GL/glew.h>


#include <png.h>

typedef enum {
  texturePNG = 0,
  textureDDS = 1,
  textureBMP = 2
} mvTextureType;

class mvTexture {
 private:
  GLfloat _width, _height;

  GLuint _textureAttribID;
  std::string _textureAttribName;

  void setupDefaultNames() {
    _textureAttribName = std::string("mvTextureSampler");
  };

  GLuint _textureBufferID;

  GLuint loadBMP(const std::string imagepath);
  GLuint loadDDS(const std::string imagepath);
  GLuint loadPNG(const std::string imagePath);
  
 public:
  mvTexture(const mvTextureType t, const std::string fileName);
  ~mvTexture() {};

  void load(GLuint programID);
  void draw(GLuint programID);

  void setTextureID(GLuint textureID) { _textureBufferID = textureID; };
  GLuint getTextureID() { return _textureBufferID; };

  GLfloat getWidth() { return _width; };
  GLfloat getHeight() { return _height; };

};
  

  

#endif
