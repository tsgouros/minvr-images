#include "controls.h"

VRControl::VRControl() {
  _position = MVec3( 0, 0, 5 ); 
  _horizontalAngle = 3.14f;  // Initial horizontal angle : toward -Z
  _verticalAngle = 0.0f;     // Initial vertical angle : none
  _initialFoV = 45.0f;
  _speed = 3.0f;             // 3 units / second
  _mouseSpeed = 0.005f;
}
  
// Camera matrix, at _position, looking in _direction, head
// up. (0,-1,0) is upside-down.
MMat4 VRControl::getViewMatrix(){

  // int i, j;
  // std::cout << "pos: ";
  // for (i = 0; i < 3; i++) printf("%6.2f ", _position[i]);
  // std::cout << std::endl << "dir: ";
  // for (i = 0; i < 3; i++) printf("%6.2f ", _direction[i]);
  // std::cout << std::endl << "up : ";
  // MVec3 u = up();
  // for (i = 0; i < 3; i++) printf("%6.2f ", u[i]);

  // MMat4 out = glm::lookAt(_position,
  //                             _position + _direction,
  //                             up());
  
  // std::cout << "lookat:" << std::endl;
  // for (int i = 0; i < 4; i++) {
  //   for (int j = 0; j < 4; j++) {
  //     printf("%6.2f ", out[i][j]);
  //   }
  //   std::cout << std::endl;
  // }

  
	return glm::lookAt(_position,
                     _position + _direction,
                     up());
}

// Projection matrix : 45° Field of View, 4:3 ratio, display range :
// 0.1 unit <-> 100 units
MMat4 VRControl::getProjectionMatrix(){
	return glm::perspective(glm::radians(_initialFoV), 4.0f / 3.0f, 0.1f, 100.0f);
}

void VRControl::handleEvents(GLFWwindow* window) {
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	_deltaTime = float(currentTime - lastTime);
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
  
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
  //std::cout << "x: " << xpos << " y: " << ypos << " ms: " << _mouseSpeed ;
	_horizontalAngle += _mouseSpeed * float(1024/2 - xpos );
	_verticalAngle   += _mouseSpeed * float( 768/2 - ypos );
  //std::cout << " ha: " << _horizontalAngle << " va: " << _verticalAngle << std::endl;

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	_direction = MVec3(
		cos(_verticalAngle) * sin(_horizontalAngle), 
		sin(_verticalAngle),
		cos(_verticalAngle) * cos(_horizontalAngle)
	);

	// Right vector
	_right = MVec3(
		sin(_horizontalAngle - 3.14f/2.0f), 
		0,
		cos(_horizontalAngle - 3.14f/2.0f)
	);
	
	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		_position += _direction * _deltaTime * _speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		_position -= _direction * _deltaTime * _speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		_position += _right * _deltaTime * _speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		_position -= _right * _deltaTime * _speed;
	}

}

void VRControl::computeMatricesFromInputs(GLFWwindow* window){


}
