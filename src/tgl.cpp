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

// =========== Axis Data ======================================================

//      Y
//      |           Z
//      |         /
//      |       /
//      |     /
//      |   /
//      | /
//      /--------------
//      O              X

GLuint VERTEX_ATTR_COORDS = 1;
GLuint VERTEX_ATTR_COLOR = 2;

static const int nCoordsComponents = 3;
static const int nColorComponents = 3;
static const int nLines = 3;
static const int nVerticesPerLine = 2;
static const int nFaces = 6;
static const int nVerticesPerFace = 3;

float av[12] = { 0.0, 0.0, 0.0,    // origin
               4.0, 0.0, 0.0,    // x-axis
               0.0, 4.0, 0.0,    // y-axis
               0.0, 0.0, 4.0 };  // z-axis

GLubyte avi[6] = { 0, 1,
                  0, 2,
                  0, 3 };

float ac[9] = { 1.0, 0.0, 0.0,    // red   x-axis
               0.0, 1.0, 0.0,    // green y-axis
               0.0, 0.0, 1.0 };  // blue  z-axis

GLubyte aci[6] = { 0, 0,
                  1, 1,
                  2, 2 };

float ave[nLines*nVerticesPerLine*nCoordsComponents];
void expandAxesVertices()
{
    for (int i=0; i<6; i++)
    {
        ave[i*3+0] = av[avi[i]*3+0];
        ave[i*3+1] = av[avi[i]*3+1];
        ave[i*3+2] = av[avi[i]*3+2];
    }
}

float ace[nLines*nVerticesPerLine*nColorComponents];
void expandAxesColors()
{
    for (int i=0; i<6; i++)
    {
        ace[i*3+0] = ac[aci[i]*3+0];
        ace[i*3+1] = ac[aci[i]*3+1];
        ace[i*3+2] = ac[aci[i]*3+2];
    }
}

static const char* axis_vertex_shader =
  "#version 330 core "
  "layout(location = 0) in vec3 aCoords;"
  "layout(location = 1) in vec3 aColor;"
  "uniform mat4 MVP;"
  "out vec3 vColor;"
  "void main () {"
    "gl_Position = MVP * vec4(aCoords, 1.0);"
    "vColor = aColor;"
  "}";

static const char* axis_fragment_shader =
  "#version 330 core\n"
  "in vec3 vColor;"
  "out vec4 fragColor;"
  "void main () {"
    "fragColor = vec4(vColor, 1.0);"
  "}";


class VRApp {
public:
  GLFWwindow* _window;
 	GLuint _VertexArrayID;
  GLuint _programID, _axisProgramID;
  GLuint _axisArrayID, _axisVerticesID, _axisColorID;
  GLuint _MatrixID, _axisMatrixID;
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

  // Put all the GLFW setup business here.
  void setupWin() {
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
      throw std::runtime_error("Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible." );
      glfwTerminate();
    }
    glfwMakeContextCurrent(_window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(_window, 1024/2, 768/2);
  };

  
  VRApp() {
    control = VRControl();
    setupWin();

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("Failed to initialize GLEW");
      glfwTerminate();
    }

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    // Cull triangles whose normal is not towards the camera.
    glEnable(GL_CULL_FACE);

    //////////////////////////////////////////////////////////
    // Create and compile our GLSL program from the shaders
    mvShaders shaders = mvShaders("StandardShading.vertexshader",
                                  "",
                                  "StandardShading.fragmentshader");
    _programID = shaders.getProgram();
    
    // Arrange the data for the shaders to work on.  "Uniforms" first.
    // Get a handle for our "MVP" uniform
    _MatrixID = glGetUniformLocation(_programID, "MVP");
    _ViewMatrixID = glGetUniformLocation(_programID, "V");
    _ModelMatrixID = glGetUniformLocation(_programID, "M");

    // Load the texture
    _Texture = loadDDS("uvmap.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
    _TextureID  = glGetUniformLocation(_programID, "myTextureSampler");

    mvShaders axisShaders = mvShaders(&axis_vertex_shader, NULL,
                                      &axis_fragment_shader);

    _axisProgramID = axisShaders.getProgram();

    glBindAttribLocation(_axisProgramID, VERTEX_ATTR_COORDS, "aCoords");
    glBindAttribLocation(_axisProgramID, VERTEX_ATTR_COLOR, "aColor");

    _axisMatrixID = glGetUniformLocation(_axisProgramID, "MVP");

    
    // Read our .obj file
    bool res = loadOBJ("suzanne.obj", _vertices, _uvs, _normals);

    // Now the vertex data.
    glGenVertexArrays(1, &_VertexArrayID);
    glBindVertexArray(_VertexArrayID);

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

    // Switch to axes
    glUseProgram(_axisProgramID);
    expandAxesVertices();
    expandAxesColors();
    
    glGenVertexArrays(1, &_axisArrayID);
    glBindVertexArray(_axisArrayID);

    glGenBuffers(1, &_axisVerticesID);
    glBindBuffer(GL_ARRAY_BUFFER, _axisVerticesID);  // coordinates
    glBufferData(GL_ARRAY_BUFFER, sizeof(ave), ave, GL_STATIC_DRAW);

    glGenBuffers(1, &_axisColorID);
    glBindBuffer(GL_ARRAY_BUFFER, _axisColorID);  // color
    glBufferData(GL_ARRAY_BUFFER, sizeof(ace), ace, GL_STATIC_DRAW);

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

    // GLint countt;
    // glGetProgramiv(_programID, GL_ACTIVE_UNIFORMS, &countt);
    // std::cout << "**Active (in use by a shader) Uniforms: " << countt << std::endl
      ;
    
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

    GLint vertexAttribCoords = glGetAttribLocation(_axisProgramID, "aCoords");
    GLint vertexAttribColor = glGetAttribLocation(_axisProgramID, "aColor");

    // Use our other shader.
    glUseProgram(_axisProgramID);

    glUniformMatrix4fv(_axisMatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    // Just checking...
    // GLint count;
    // glGetProgramiv(_axisProgramID, GL_ACTIVE_UNIFORMS, &count);
    // std::cout << "**Active (in use by a shader) Uniforms: " << count << std::endl;

    // Enable VAO to set axes data
    glBindVertexArray(_axisArrayID);
    
    glEnableVertexAttribArray(vertexAttribCoords);
    glBindBuffer(GL_ARRAY_BUFFER, _axisVerticesID);  // coordinates
    glVertexAttribPointer(vertexAttribCoords, nCoordsComponents, GL_FLOAT, GL_FALSE, 0, 0);


    glEnableVertexAttribArray(vertexAttribColor);
    glBindBuffer(GL_ARRAY_BUFFER, _axisColorID);  // color
    glVertexAttribPointer(vertexAttribColor, nColorComponents, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Draw axes
    glDrawArrays(GL_LINES, 0, nLines*nVerticesPerLine);

    
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
