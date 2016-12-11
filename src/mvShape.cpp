#include "mvShape.h"


void mvShape::printMat(std::string name, glm::mat4 mat) {
  std::cout << name << std::endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf("%6.2f ", mat[j][i]);
    }
    std::cout << std::endl;
  }
}

// These are the default names of variables in the shaders.  Placed
// here so they're all in one place, for easy comparison to the shader
// you'll use.
void mvShape::setupDefaultNames() {

  _vertexAttribName = std::string("vertexPosition_modelspace");
  _uvAttribName = std::string("vertexUV");
  _normalAttribName = std::string("vertexNormal_modelspace");
  _colorAttribName = std::string("vertexInputColor");
  _textureAttribName = std::string("myTextureSampler");
  _mvpMatrixName = std::string("MVP");
  _projMatrixName = std::string("P");
  _viewMatrixName = std::string("V");
  _modelMatrixName = std::string("M");
}

mvShape::mvShape(mvShapeType type, GLuint programID) :
  _type(type), _programID(programID) {

  setupDefaultNames();
  
  // Set all the translations and rotations to zero.
  _scale = glm::vec3(1.0f, 1.0f, 1.0f);
  // _rotQuaternion is initialized to zero rotation by default.
  _position = glm::vec3(0.0f, 0.0f, 0.0f);

  // Calculate the model matrix.  It should come out as an identity
  // matrix, so this is not really necessary.
  _modelMatrixNeedsReset = true;
  _modelMatrix = getModelMatrix();
}

mvShape::~mvShape() {

  // Cleanup VBO and shader
  if (glIsBuffer(_vertexBufferID)) glDeleteBuffers(1, &_vertexBufferID);
  if (glIsBuffer(_uvBufferID)) glDeleteBuffers(1, &_uvBufferID);
  if (glIsBuffer(_normalBufferID)) glDeleteBuffers(1, &_normalBufferID);
  if (glIsBuffer(_colorBufferID)) glDeleteBuffers(1, &_colorBufferID);
  if (glIsVertexArray(_arrayID)) glDeleteVertexArrays(1, &_arrayID);
}

glm::mat4 mvShape::getModelMatrix() {

  if (_modelMatrixNeedsReset) {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), _position);
    glm::mat4 rotationMatrix = glm::mat4_cast(_rotQuaternion);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), _scale);

    _modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    _modelMatrixNeedsReset = false;

    // printMat("scale:", scaleMatrix);
    // printMat("rotat:", rotationMatrix);
    // printMat("trans:", translationMatrix);
    // printMat("model:", _modelMatrix);
  }

  return _modelMatrix;
}

void mvShapeRect::initVertices() {

  // Front side of rectangle.
  _vertices.push_back(glm::vec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3(-_width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f,  _height/2.0f, 0.0f));

  // Back side.
  _vertices.push_back(glm::vec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3(-_width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(glm::vec3( _width/2.0f, -_height/2.0f, 0.0f));

  // Front side texture coordinates.
  _uvs.push_back(glm::vec2(0.0f, 0.0f));
  _uvs.push_back(glm::vec2(1.0f, 1.0f));
  _uvs.push_back(glm::vec2(0.0f, 1.0f));
  _uvs.push_back(glm::vec2(0.0f, 0.0f));
  _uvs.push_back(glm::vec2(1.0f, 0.0f));
  _uvs.push_back(glm::vec2(1.0f, 1.0f));

  // Back side.
  _uvs.push_back(glm::vec2(0.0f, 0.0f));
  _uvs.push_back(glm::vec2(0.0f, 1.0f));
  _uvs.push_back(glm::vec2(1.0f, 1.0f));
  _uvs.push_back(glm::vec2(0.0f, 0.0f));
  _uvs.push_back(glm::vec2(1.0f, 1.0f));
  _uvs.push_back(glm::vec2(1.0f, 0.0f));

  // The normals all point in the same direction.
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
}

void mvShapeRect::load() {

  // Set up the vertex and other arrays.
  initVertices();
  
  // Arrange the data for the shaders to work on.  "Uniforms" first.
  // Get a handle for our "MVP" uniform
  _mvpMatrixID = glGetUniformLocation(_programID, _mvpMatrixName.c_str());
  _viewMatrixID = glGetUniformLocation(_programID, _viewMatrixName.c_str());
  _modelMatrixID = glGetUniformLocation(_programID, _modelMatrixName.c_str());

  // Get a handle for our "myTextureSampler" uniform
  _textureAttribID  = glGetUniformLocation(_programID, _textureAttribName.c_str());

  //std::cout << "loading mvShapeObj" << std::endl;
  // Read our .obj file
  //bool res = loadOBJ("suzanne.obj", _vertices, _uvs, _normals);

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


  // Get handles for the various shader inputs.
  _vertexAttribID =
    glGetAttribLocation(_programID, _vertexAttribName.c_str());
  _uvAttribID = glGetAttribLocation(_programID, _uvAttribName.c_str());
  _normalAttribID = 
    glGetAttribLocation(_programID, _normalAttribName.c_str());

}

void mvShapeRect::draw(VRControl control) {

  // Use our shader
  glUseProgram(_programID);

  glm::vec3 p = getPosition();
  p.x += 0.01;
  setPosition(p);
  // glm::quat q = getRotQuaternion();
  // q.z = 1.0;
  // q.w += 0.01;
  // setRotation(q);

  
  // Compute the MVP matrix from keyboard and mouse input
  glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
  glm::mat4 ViewMatrix = control.getViewMatrix();
  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * getModelMatrix();

  // printMat("proj", ProjectionMatrix);
  // printMat("view", ViewMatrix);
  // printMat("model", _modelMatrix);
  // printMat("MVP", MVP);
    
  // Send our transformation to the currently bound shader, 
  // in the "MVP" uniform
  glUniformMatrix4fv(_mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(_modelMatrixID, 1, GL_FALSE, &_modelMatrix[0][0]);
  glUniformMatrix4fv(_viewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);


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


void mvShapeObj::load() {

  // Arrange the data for the shaders to work on.  "Uniforms" first.
  // Get a handle for our "MVP" uniform
  _mvpMatrixID = glGetUniformLocation(_programID, _mvpMatrixName.c_str());
  _viewMatrixID = glGetUniformLocation(_programID, _viewMatrixName.c_str());
  _modelMatrixID = glGetUniformLocation(_programID, _modelMatrixName.c_str());

  // Get a handle for our "myTextureSampler" uniform
  _textureAttribID  = glGetUniformLocation(_programID, _textureAttribName.c_str());

  //std::cout << "loading mvShapeObj" << std::endl;
  // Read our .obj file
  bool res = loadOBJ(_objFileName.c_str(), _vertices, _uvs, _normals);

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
  // glUseProgram(_programID);
  // _lightPositionID = glGetUniformLocation(_programID, "LightPosition_worldspace");
  // _lightColorID = glGetUniformLocation(_programID, "LightColor");

  // Get handles for the various shader inputs.
  _vertexAttribID =
    glGetAttribLocation(_programID, _vertexAttribName.c_str());
  _uvAttribID = glGetAttribLocation(_programID, _uvAttribName.c_str());
  _normalAttribID = 
    glGetAttribLocation(_programID, _normalAttribName.c_str());

}

void mvShapeObj::draw(VRControl control) {

  // Use our shader
  glUseProgram(_programID);

  // Compute the MVP matrix from keyboard and mouse input
  glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
  glm::mat4 ViewMatrix = control.getViewMatrix();
  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * getModelMatrix();

  // printMat("proj", ProjectionMatrix);
  // printMat("view", ViewMatrix);
  // printMat("model", _modelMatrix);
  // printMat("MVP", MVP);
    
  // Send our transformation to the currently bound shader, 
  // in the "MVP" uniform
  glUniformMatrix4fv(_mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(_modelMatrixID, 1, GL_FALSE, &_modelMatrix[0][0]);
  glUniformMatrix4fv(_viewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

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

  _mvpMatrixID = glGetUniformLocation(_programID, _mvpMatrixName.c_str());

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
  _vertexAttribID = glGetAttribLocation(_programID, _vertexAttribName.c_str());
  _colorAttribID = glGetAttribLocation(_programID, _colorAttribName.c_str());

  glUseProgram(_programID);

  // Compute the MVP matrix from keyboard and mouse input
  glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
  glm::mat4 ViewMatrix = control.getViewMatrix();
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

mvShape* mvShapeFactory::createShape(mvShapeType type, GLuint programID) {

  callbackMap::const_iterator it = _callbacks.find(type);

  if (it == _callbacks.end()) {
    std::stringstream msg;
    msg << "unknown shape: " << (int)(type);
    throw std::runtime_error(msg.str());
  } else {
    return (it->second)(programID);
  }
}

bool mvShapeFactory::registerShape(mvShapeType type,
                                     createMvShapeCallback creator) {

  return _callbacks.insert(callbackMap::value_type(type, creator)).second;
}

std::string mvShape::print() const {
  std::stringstream out;

  std::map<mvShapeType, std::string> map;
  map[shapeOBJ] = "OBJ";
  map[shapeAXES] = "AXES";
  map[shapeRECT] = "RECT";
  
  out << "SHAPE data:" << std::endl;

  out << "_type:            " << _type << " (" << map[_type] << ")" << std::endl;
  out << "_programID:       " << _programID << std::endl;
  out << "_arrayID:         " << _arrayID << std::endl;
  out << "_vertexAttribID:  " << _vertexAttribID;
  out << " (" << _vertexAttribName << ")" << std::endl;
  out << "_uvAttribID:      " << _uvAttribID;
  out << " (" << _uvAttribName << ")" << std::endl;
  out << "_normalAttribID:  " << _normalAttribID;
  out << " (" << _normalAttribName << ")" << std::endl;
  out << "_colorAttribID:   " << _colorAttribID;
  out << " (" << _colorAttribName << ")" << std::endl;
  out << "_textureAttribID: " << _textureAttribID;
  out << " (" << _textureAttribName << ")" << std::endl;
  out << "_vertexBufferID:  " << _vertexBufferID << std::endl;
  out << "_uvBufferID:      " << _uvBufferID << std::endl;
  out << "_normalBufferID:  " << _normalBufferID << std::endl;
  out << "_colorBufferID:   " << _colorBufferID << std::endl;
  out << "_textureBufferID: " << _textureBufferID << std::endl;

  // The actual shape data is stored here.
  out << "_vertices:    N = " << _vertices.size() << std::endl;
  out << "_uvs:         N = " << _uvs.size() << std::endl;
  out << "_normals:     N = " << _normals.size() << std::endl;
  out << "_colors:      N = " << _colors.size() << std::endl;

  // These matrices may appear in the shaders.
  out << "_mvpMatrixID:     " << _mvpMatrixID;
  out << " (" << _mvpMatrixName << ")" << std::endl;
  out << "_projMatrixID:    " << _projMatrixID;
  out << " (" << _projMatrixName << ")" << std::endl;
  out << "_viewMatrixID:    " << _viewMatrixID;
  out << " (" << _viewMatrixName << ")" << std::endl;
  out << "_modelMatrixID:   " << _modelMatrixID;
  out << " (" << _modelMatrixName << ")" << std::endl;

  out << "pos:   " << _position.x << "," << _position.y << "," << _position.z << std::endl;
  out << "scale: " << _scale.x << "," << _scale.y << "," << _scale.z << std::endl;
  out << "quat:  " << _rotQuaternion.x << "," << _rotQuaternion.y << "," << _rotQuaternion.z << "," << _rotQuaternion.w << std::endl;

  return out.str();
}

std::string mvShapeRect::print() const {
  std::stringstream out;

  out << mvShape::print();

  out << std::endl << "_lightPositionID: " << _lightPositionID << std::endl;
  out << std::endl << "_lightColorID: " << _lightColorID << std::endl;
  out << "width: " << _width << "  height: " << _height << std::endl;

  return out.str();
}

std::string mvShapeObj::print() const {
  std::stringstream out;

  out << mvShape::print();
  out << std::endl << "_lightPositionID: " << _lightPositionID << std::endl;
  out << std::endl << "_lightColorID: " << _lightColorID << std::endl;
  out << "OBJ file: " << _objFileName << std::endl;
  
  return out.str();
}

std::string mvShapeAxes::print() const {
  std::stringstream out;

  out << mvShape::print();
  out << "nCoordsComponents:" << nCoordsComponents << std::endl;
  out << "nColorComponents: " << nColorComponents << std::endl;
  out << "nLines:           " << nLines << std::endl;
  out << "nVerticesPerLine: " << nVerticesPerLine << std::endl;
  out << "nFaces:           " << nFaces << std::endl;
  out << "nVerticesPerFace: " << nVerticesPerFace << std::endl;

  return out.str();
}

std::string mvShapeFactory::print() const {
  std::stringstream out;

  for (callbackMap::const_iterator it = _callbacks.begin();
       it != _callbacks.end(); it++) {
    out << "callback for: " << it->first << std::endl;
  }

  return out.str();
}

std::ostream & operator<<(std::ostream &os, const mvShape& iShape) {
  return os << iShape.print();
}
std::ostream & operator<<(std::ostream &os, const mvShapeRect& iShape) {
  return os << iShape.print();
}
std::ostream & operator<<(std::ostream &os, const mvShapeObj& iShape) {
  return os << iShape.print();
}
std::ostream & operator<<(std::ostream &os, const mvShapeAxes& iShape) {
  return os << iShape.print();
}
std::ostream & operator<<(std::ostream &os, const mvShapeFactory& iShape) {
  return os << iShape.print();
}



