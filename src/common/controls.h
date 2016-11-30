#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <iostream> 

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class VRControl {
 protected:
  glm::mat4 _ViewMatrix;
  glm::mat4 _ProjectionMatrix;

 public:
  VRControl();
  
  // Initial position : on +Z
  glm::vec3 _position;
  glm::vec3 _direction;
	glm::vec3 _right;
  float _horizontalAngle;
  float _verticalAngle;
    
  // Initial Field of View
  float _initialFoV;
  float _speed; 
  float _mouseSpeed;

  float _deltaTime;
  
  void handleEvents(GLFWwindow* window);
  void computeMatricesFromInputs(GLFWwindow* window);
  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();

  glm::vec3 up() { return glm::cross( _right, _direction ); };
};
#endif
