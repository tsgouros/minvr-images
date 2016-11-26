#ifndef CONTROLS_HPP
#define CONTROLS_HPP

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class VRControl {
 protected:
  glm::mat4 ViewMatrix;
  glm::mat4 ProjectionMatrix;

 public:
  VRControl();
  
  // Initial position : on +Z
  glm::vec3 position; 
  // Initial horizontal angle : toward -Z
  float horizontalAngle;
  // Initial vertical angle : none
  float verticalAngle;
  
  // Initial Field of View
  float initialFoV;
  float speed; 
  float mouseSpeed;
  
  void computeMatricesFromInputs(GLFWwindow* window);
  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();
};
#endif
