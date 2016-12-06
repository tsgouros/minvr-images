#include "mvShape.h"


void mvShape::printMat(std::string name, glm::mat4 mat) {
  std::cout << name << std::endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf("%6.2f ", mat[i][j]);
    }
    std::cout << std::endl;
  }
}

mvShape::~mvShape() {
  // Can we test if these are in use, and delete if so? If yes, move
  // this into parent class.

  // Cleanup VBO and shader
  if (glIsBuffer(_vertexBufferID)) glDeleteBuffers(1, &_vertexBufferID);
  if (glIsBuffer(_uvBufferID)) glDeleteBuffers(1, &_uvBufferID);
  if (glIsBuffer(_normalBufferID)) glDeleteBuffers(1, &_normalBufferID);
  if (glIsBuffer(_colorBufferID)) glDeleteBuffers(1, &_colorBufferID);
  if (glIsTexture(_textureBufferID)) glDeleteTextures(1, &_textureBufferID);
  if (glIsVertexArray(_arrayID)) glDeleteVertexArrays(1, &_arrayID);
}


void mvShapeObj::load() {

  // Arrange the data for the shaders to work on.  "Uniforms" first.
  // Get a handle for our "MVP" uniform
  _mvpMatrixID = glGetUniformLocation(_programID, "MVP");
  _viewMatrixID = glGetUniformLocation(_programID, "V");
  _modelMatrixID = glGetUniformLocation(_programID, "M");

  // Load the texture
  _textureBufferID = loadDDS("uvmap.DDS");
    
  // Get a handle for our "myTextureSampler" uniform
  _textureAttribID  = glGetUniformLocation(_programID, "myTextureSampler");

  //std::cout << "loading mvShapeObj" << std::endl;
  // Read our .obj file
  bool res = loadOBJ("suzanne.obj", _vertices, _uvs, _normals);

  // Now the vertex data.
  glGenVertexArrays(1, &_arrayID);
  glBindVertexArray(_arrayID);

  // Load it into a VBO
  glGenBuffers(1, &_vertexBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
  glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3),
               &_vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_uvBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _uvBufferID);
  glBufferData(GL_ARRAY_BUFFER, _uvs.size() * sizeof(glm::vec2),
               &_uvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_normalBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
  glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3),
               &_normals[0], GL_STATIC_DRAW);

  // Get a handle for our "LightPosition" uniform.  We are not
  // binding the attribute location, just asking politely for it.
  glUseProgram(_programID);
  _lightID = glGetUniformLocation(_programID, "LightPosition_worldspace");

  // Get handles for the various shader inputs.
  _vertexAttribID =
    glGetAttribLocation(_programID, "vertexPosition_modelspace");
  _uvAttribID = glGetAttribLocation(_programID, "vertexUV");
  _normalAttribID = 
    glGetAttribLocation(_programID, "vertexNormal_modelspace");

}

void mvShapeObj::draw(VRControl control) {

  // Use our shader
  glUseProgram(_programID);

  // Compute the MVP matrix from keyboard and mouse input
  glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
  glm::mat4 ViewMatrix = control.getViewMatrix();
  _modelMatrix = glm::mat4(1.0);
  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * _modelMatrix;

  // printMat("proj", ProjectionMatrix);
  // printMat("view", ViewMatrix);
  // printMat("model", _modelMatrix);
  // printMat("MVP", MVP);
    
  // Send our transformation to the currently bound shader, 
  // in the "MVP" uniform
  glUniformMatrix4fv(_mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(_modelMatrixID, 1, GL_FALSE, &_modelMatrix[0][0]);
  glUniformMatrix4fv(_viewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

  glm::vec3 lightPos = glm::vec3(4,4,4);
  glUniform3f(_lightID, lightPos.x, lightPos.y, lightPos.z);

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _textureBufferID);
  // Set our "myTextureSampler" sampler to user Texture Unit 0
  glUniform1i(_textureAttribID, 0);

  // GLint countt;
  // glGetProgramiv(_programID, GL_ACTIVE_UNIFORMS, &countt);
  // std::cout << "**Active (in use by a shader) Uniforms: " << countt << std::endl;

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(_vertexAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
  glVertexAttribPointer(
                        _vertexAttribID,    // attribute
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(_uvAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _uvBufferID);
  glVertexAttribPointer(
                        _uvAttribID,        // attribute
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // 3rd attribute buffer : normals
  glEnableVertexAttribArray(_normalAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
  glVertexAttribPointer(
                        _normalAttribID,    // attribute
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );

  // Draw the triangles !
  glDrawArrays(GL_TRIANGLES, 0, _vertices.size() );

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void mvShapeAxes::expandAxesVertices() {

  float av[12] = { 0.0, 0.0, 0.0,    // origin
                   4.0, 0.0, 0.0,    // x-axis
                   0.0, 4.0, 0.0,    // y-axis
                   0.0, 0.0, 4.0 };  // z-axis
    
  GLubyte avi[6] = { 0, 1,
                     0, 2,
                     0, 3 };

  for (int i=0; i<6; i++) {
    _vertices.push_back(glm::vec3(av[avi[i]*3+0],
                                  av[avi[i]*3+1],
                                  av[avi[i]*3+2]));
  }
}

void mvShapeAxes::expandAxesColors() {

  float ac[9] = { 1.0, 0.0, 0.0,    // red   x-axis
                  0.0, 1.0, 0.0,    // green y-axis
                  0.0, 0.0, 1.0 };  // blue  z-axis

  GLubyte aci[6] = { 0, 0,
                     1, 1,
                     2, 2 };

  for (int i=0; i<6; i++) {
    _colors.push_back(glm::vec3(ac[aci[i]*3+0],
                                ac[aci[i]*3+1],
                                ac[aci[i]*3+2]));
  }
}

void mvShapeAxes::load() {

  _mvpMatrixID = glGetUniformLocation(_programID, "MVP");

  glGenVertexArrays(1, &_arrayID);
  glBindVertexArray(_arrayID);

  glGenBuffers(1, &_vertexBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);  // coordinates
  glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3),
               &_vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &_colorBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _colorBufferID);  // color
  glBufferData(GL_ARRAY_BUFFER, _colors.size() * sizeof(glm::vec3),
               &_colors[0], GL_STATIC_DRAW);

}

void mvShapeAxes::draw(VRControl control) {

  // We have to ask where these attributes are located.
  _vertexAttribID = glGetAttribLocation(_programID, "vertexPosition_modelspace");
  _colorAttribID = glGetAttribLocation(_programID, "vertexInputColor");

  glUseProgram(_programID);

  // Compute the MVP matrix from keyboard and mouse input
  glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
  glm::mat4 ViewMatrix = control.getViewMatrix();
  _modelMatrix = glm::mat4(1.0);
  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * _modelMatrix;
    
  glUniformMatrix4fv(_mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
    
  // Just checking...
  // GLint count;
  // glGetProgramiv(_programID, GL_ACTIVE_UNIFORMS, &count);
  // std::cout << "**Active (in use by a shader) Uniforms: " << count << std::endl;

  // Enable VAO to set axes data
  glBindVertexArray(_arrayID);
    
  glEnableVertexAttribArray(_vertexAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);  // coordinates
  glVertexAttribPointer(_vertexAttribID, nCoordsComponents, GL_FLOAT, GL_FALSE, 0, 0);


  glEnableVertexAttribArray(_colorAttribID);
  glBindBuffer(GL_ARRAY_BUFFER, _colorBufferID);  // color
  glVertexAttribPointer(_colorAttribID, nColorComponents, GL_FLOAT, GL_FALSE, 0, 0);
    
  // Draw axes
  glDrawArrays(GL_LINES, 0, nLines*nVerticesPerLine);

}

mvShape* mvShapeFactory::createMvShape(mvShapeType type, GLuint programID) {

  callbackMap::const_iterator it = _callbacks.find(type);

  if (it == _callbacks.end()) {
    std::stringstream msg;
    msg << "unknown shape: " << (int)(type);
    throw std::runtime_error(msg.str());
  } else {
    return (it->second)(programID);
  }
}

bool mvShapeFactory::registerMvShape(mvShapeType type,
                                     createMvShapeCallback creator) {

  return _callbacks.insert(callbackMap::value_type(type, creator)).second;
}





