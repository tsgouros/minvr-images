#include "mvShape.h"


void mvShape::printMat(std::string name, MMat4 mat) {
  std::cout << name << std::endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf("%6.2f ", mat[j][i]);
    }
    std::cout << std::endl;
  }
}

mvShape::mvShape(mvShapeType type, mvShaderSet* shaders, mvTexture* texture) :
  _type(type), _shaderContext(shaders, texture) {

  // Set all the translations and rotations to zero.
  _scale = MVec3(1.0f, 1.0f, 1.0f);
  // _rotQuaternion is initialized to zero rotation by default.
  _position = MVec3(0.0f, 0.0f, 0.0f);

  // Calculate the model matrix.  It should come out as an identity
  // matrix, so this is not really necessary.
  _modelMatrixNeedsReset = true;
  _modelMatrix = getModelMatrix();
}

mvShape::~mvShape() {}

MMat4 mvShape::getModelMatrix() {

  if (_modelMatrixNeedsReset) {
    MMat4 translationMatrix = glm::translate(MMat4(1.0f), _position);
    MMat4 rotationMatrix = glm::mat4_cast(_rotQuaternion);
    MMat4 scaleMatrix = glm::scale(MMat4(1.0f), _scale);

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
  _vertices.push_back(MVec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(MVec3(-_width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(MVec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f,  _height/2.0f, 0.0f));

  // Back side.
  _vertices.push_back(MVec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(MVec3(-_width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(MVec3(-_width/2.0f, -_height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f,  _height/2.0f, 0.0f));
  _vertices.push_back(MVec3( _width/2.0f, -_height/2.0f, 0.0f));

  // Front side texture coordinates.
  _uvs.push_back(MVec2(0.0f, 1.0f));
  _uvs.push_back(MVec2(1.0f, 0.0f));
  _uvs.push_back(MVec2(0.0f, 0.0f));
  _uvs.push_back(MVec2(0.0f, 1.0f));
  _uvs.push_back(MVec2(1.0f, 1.0f));
  _uvs.push_back(MVec2(1.0f, 0.0f));

  // Back side.
  _uvs.push_back(MVec2(0.0f, 1.0f));
  _uvs.push_back(MVec2(0.0f, 0.0f));
  _uvs.push_back(MVec2(1.0f, 0.0f));
  _uvs.push_back(MVec2(0.0f, 1.0f));
  _uvs.push_back(MVec2(1.0f, 0.0f));
  _uvs.push_back(MVec2(1.0f, 1.0f));

  // The normals all point in the same direction.
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
  _normals.push_back(MVec3(0.0f, 0.0f, 1.0f));
}

void mvShapeRect::load() {

  // Set up the vertex and other arrays.
  initVertices();
  _shaderContext.load(_vertices, _uvs, _normals, _colors);
}

void mvShapeRect::draw(MMat4 ViewMatrix, MMat4 ProjectionMatrix) {

  // printMat("model", getModelMatrix());
  // printMat("view", ViewMatrix);
  // printMat("proj", ProjectionMatrix);
    
  _shaderContext.draw(getModelMatrix(), ViewMatrix, ProjectionMatrix);
  
}


void mvShapeObj::load() {

  //std::cout << "loading mvShapeObj" << std::endl;
  // Read our .obj file
  bool res = loadOBJ(_objFileName.c_str(), _vertices, _uvs, _normals);
  _shaderContext.load(_vertices, _uvs, _normals, _colors);
}

void mvShapeObj::draw(MMat4 ViewMatrix, MMat4 ProjectionMatrix) {

  _shaderContext.draw(getModelMatrix(), ViewMatrix, ProjectionMatrix);

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
    _vertices.push_back(MVec3(av[avi[i]*3+0],
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
    _colors.push_back(MVec3(ac[aci[i]*3+0],
                                ac[aci[i]*3+1],
                                ac[aci[i]*3+2]));
  }
}

void mvShapeAxes::load() {

  // Axes have no texture. _texture->load(_shaderContext.getProgramID());
  _shaderContext.load(_vertices, _uvs, _normals, _colors);
}

  
//   _projMatrixID = glGetUniformLocation(_shaderContext.getProgramID(), _projMatrixName.c_str());

//   glGenVertexArrays(1, &_arrayID);
//   glBindVertexArray(_arrayID);

//   glGenBuffers(1, &_vertexBufferID);
//   glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);  // coordinates
//   glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(MVec3),
//                &_vertices[0], GL_STATIC_DRAW);

//   glGenBuffers(1, &_colorBufferID);
//   glBindBuffer(GL_ARRAY_BUFFER, _colorBufferID);  // color
//   glBufferData(GL_ARRAY_BUFFER, _colors.size() * sizeof(MVec3),
//                &_colors[0], GL_STATIC_DRAW);

// }

void mvShapeAxes::draw(MMat4 ViewMatrix, MMat4 ProjectionMatrix) {

  // No texture for this, so skip it.  _texture->draw(_shaderContext.getProgramID());
  _shaderContext.draw(getModelMatrix(), ViewMatrix, ProjectionMatrix);

}
  
//   // We have to ask where these attributes are located.
//   _vertexAttribID = glGetAttribLocation(_shaders->getProgramID(),
//                                         _vertexAttribName.c_str());
//   _colorAttribID = glGetAttribLocation(_shaders->getProgramID(),
//                                        _colorAttribName.c_str());

//   glUseProgram(_shaders->getProgramID());

//   glUniformMatrix4fv(_projMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
    
//   // Just checking...
//   // GLint count;
//   // glGetProgramiv(_shaderContext.getProgramID(), GL_ACTIVE_UNIFORMS, &count);
//   // std::cout << "**Active (in use by a shader) Uniforms: " << count << std::endl;

//   // Enable VAO to set axes data
//   glBindVertexArray(_arrayID);
    
//   glEnableVertexAttribArray(_vertexAttribID);
//   glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);  // coordinates
//   glVertexAttribPointer(_vertexAttribID, nCoordsComponents, GL_FLOAT, GL_FALSE, 0, 0);


//   glEnableVertexAttribArray(_colorAttribID);
//   glBindBuffer(GL_ARRAY_BUFFER, _colorBufferID);  // color
//   glVertexAttribPointer(_colorAttribID, nColorComponents, GL_FLOAT, GL_FALSE, 0, 0);
    
//   // Draw axes
//   glDrawArrays(GL_LINES, 0, nLines*nVerticesPerLine);

// }

mvShape* mvShapeFactory::createShape(mvShapeType type,
                                     mvShaderSet* shaders,
                                     mvTexture* texture) {

  callbackMap::const_iterator it = _callbacks.find(type);

  if (it == _callbacks.end()) {
    std::stringstream msg;
    msg << "unknown shape: " << (int)(type);
    throw std::runtime_error(msg.str());
  } else {
    return (it->second)(shaders, texture);
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

  // The actual shape data is stored here.
  out << "_vertices:    N = " << _vertices.size() << std::endl;
  out << "_uvs:         N = " << _uvs.size() << std::endl;
  out << "_normals:     N = " << _normals.size() << std::endl;
  out << "_colors:      N = " << _colors.size() << std::endl;

  out << "pos:   " << _position.x << "," << _position.y << "," << _position.z << std::endl;
  out << "scale: " << _scale.x << "," << _scale.y << "," << _scale.z << std::endl;
  out << "quat:  " << _rotQuaternion.x << "," << _rotQuaternion.y << "," << _rotQuaternion.z << "," << _rotQuaternion.w << std::endl;

  return out.str();
}

std::string mvShapeRect::print() const {
  std::stringstream out;

  out << mvShape::print();

  out << std::endl << "lightPositionID: " << _lightPositionID << std::endl;
  out << std::endl << "lightColorID: " << _lightColorID << std::endl;
  out << "width: " << _width << "  height: " << _height << std::endl;

  return out.str();
}

std::string mvShapeObj::print() const {
  std::stringstream out;

  out << mvShape::print();
  out << std::endl << "lightPositionID: " << _lightPositionID << std::endl;
  out << std::endl << "lightColorID: " << _lightColorID << std::endl;
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



