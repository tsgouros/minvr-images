#include <iostream>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.h>
#include <common/texture.h>
#include <common/controls.h>
#include <common/objloader.h>

void printMat(std::string name, glm::mat4 mat) {
  std::cout << name << std::endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf("%6.2f ", mat[i][j]);
    }
    std::cout << std::endl;
  }
}

class VRApp {
public:
  GLFWwindow* _window;
 	GLuint _VertexArrayID;
  GLuint _programID;
  GLuint _MatrixID;
	GLuint _ViewMatrixID;
	GLuint _ModelMatrixID;
  GLuint _Texture;
  GLuint _TextureID;
  std::vector<glm::vec3> _vertices;
	std::vector<glm::vec2> _uvs;
	std::vector<glm::vec3> _normals;
	GLuint _vertexBuffer;
	GLuint _uvBuffer;
	GLuint _normalBuffer;
  GLuint _LightID;
  VRControl control;
  
  VRApp() {
    control = VRControl();

    // Initialise GLFW
    if( !glfwInit() ) {
      throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    _window = glfwCreateWindow( 1024, 768, "tgl", NULL, NULL);
    if( _window == NULL ){
      throw std::runtime_error("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials." );
      glfwTerminate();
    }
    glfwMakeContextCurrent(_window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("Failed to initialize GLEW");
      glfwTerminate();
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(_window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    // Cull triangles whose normal is not towards the camera.
    glEnable(GL_CULL_FACE);


    glGenVertexArrays(1, &_VertexArrayID);
    glBindVertexArray(_VertexArrayID);

    // Create and compile our GLSL program from the shaders
    _programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

    // Get a handle for our "MVP" uniform
    _MatrixID = glGetUniformLocation(_programID, "MVP");
    _ViewMatrixID = glGetUniformLocation(_programID, "V");
    _ModelMatrixID = glGetUniformLocation(_programID, "M");

    // Load the texture
    _Texture = loadDDS("uvmap.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
    _TextureID  = glGetUniformLocation(_programID, "myTextureSampler");

    // Read our .obj file
    bool res = loadOBJ("suzanne.obj", _vertices, _uvs, _normals);

    // Load it into a VBO
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3), &_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &_uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, _uvs.size() * sizeof(glm::vec2), &_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3), &_normals[0], GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(_programID);
    _LightID = glGetUniformLocation(_programID, "LightPosition_worldspace");
    
  };

  ~VRApp() {

    // Cleanup VBO and shader
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteBuffers(1, &_uvBuffer);
    glDeleteBuffers(1, &_normalBuffer);
    glDeleteProgram(_programID);
    glDeleteTextures(1, &_Texture);
    glDeleteVertexArrays(1, &_VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
  };

  void checkEvents() {
    control.handleEvents(_window);
    control.computeMatricesFromInputs(_window);
		glfwPollEvents();
  }
  
  void draw() {

    // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(_programID);

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
		glm::mat4 ViewMatrix = control.getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // printMat("proj", ProjectionMatrix);
    // printMat("view", ViewMatrix);
    // printMat("model", ModelMatrix);
    // printMat("MVP", MVP);
    
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(_MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(_ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(_ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(_LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(_TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, _vertices.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(_window);
  };

};

int main( void )
{
  VRApp app = VRApp();
  
	do{
    app.checkEvents();
    
    app.draw();
    

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(app._window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(app._window) == 0 );

	return 0;
}
