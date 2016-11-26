// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

class VRApp {
public:
  GLFWwindow* window;
 	GLuint VertexArrayID;
  GLuint programID;
  GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
  GLuint Texture;
  GLuint TextureID;
  std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint normalBuffer;
  GLuint LightID;


};

int main( void )
{
  VRApp app = VRApp();
  
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	app.window = glfwCreateWindow( 1024, 768, "tgl", NULL, NULL);
	if( app.window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(app.window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(app.window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(app.window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &app.VertexArrayID);
	glBindVertexArray(app.VertexArrayID);

	// Create and compile our GLSL program from the shaders
	app.programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
  app.MatrixID = glGetUniformLocation(app.programID, "MVP");
  app.ViewMatrixID = glGetUniformLocation(app.programID, "V");
  app.ModelMatrixID = glGetUniformLocation(app.programID, "M");

	// Load the texture
	app.Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	app.TextureID  = glGetUniformLocation(app.programID, "myTextureSampler");

	// Read our .obj file
	bool res = loadOBJ("suzanne.obj", app.vertices, app.uvs, app.normals);

	// Load it into a VBO

	glGenBuffers(1, &app.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, app.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, app.vertices.size() * sizeof(glm::vec3), &app.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &app.uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, app.uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, app.uvs.size() * sizeof(glm::vec2), &app.uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &app.normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, app.normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, app.normals.size() * sizeof(glm::vec3), &app.normals[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(app.programID);
	app.LightID = glGetUniformLocation(app.programID, "LightPosition_worldspace");

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(app.programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(app.window);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(app.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(app.ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(app.ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(app.LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, app.Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(app.TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, app.vertexBuffer);
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
		glBindBuffer(GL_ARRAY_BUFFER, app.uvBuffer);
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
		glBindBuffer(GL_ARRAY_BUFFER, app.normalBuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, app.vertices.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(app.window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(app.window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(app.window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &app.vertexBuffer);
	glDeleteBuffers(1, &app.uvBuffer);
	glDeleteBuffers(1, &app.normalBuffer);
	glDeleteProgram(app.programID);
	glDeleteTextures(1, &app.Texture);
	glDeleteVertexArrays(1, &app.VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
