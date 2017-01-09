#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <iostream> 

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vecTypes.h"

class VRControl {
 protected:
  MMat4 _ViewMatrix;
  MMat4 _ProjectionMatrix;

 public:
  VRControl();
  
  // Initial position : on +Z
  MVec3 _position;
  MVec3 _direction;
	MVec3 _right;
  float _horizontalAngle;
  float _verticalAngle;
    
  // Initial Field of View
  float _initialFoV;
  float _speed; 
  float _mouseSpeed;

  float _deltaTime;
  
  void handleEvents(GLFWwindow* window);
  void computeMatricesFromInputs(GLFWwindow* window);
  MMat4 getViewMatrix();
  MMat4 getProjectionMatrix();

  MVec3 up() { return glm::cross( _right, _direction ); };
};
#endif
