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

static const char* axis_vertex_shader =
  "#version 330 core "
  "layout(location = 0) in vec3 vertexPosition_modelspace;"
  "layout(location = 1) in vec3 vertexColor;"
  "uniform mat4 MVP;"
  "out vec3 vColor;"
  "void main () {"
    "gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);"
    "vColor = vertexColor;"
  "}";

static const char* axis_fragment_shader =
  "#version 330 core\n"
  "in vec3 vColor;"
  "out vec4 fragColor;"
  "void main () {"
    "fragColor = vec4(vColor, 1.0);"
  "}";


class shape {
protected:

  GLuint _arrayID;

  // These IDs point to the names/locations of values in shaders.
  GLuint _vertexAttribID;
  GLuint _uvAttribID;
  GLuint _normalAttribID;
  GLuint _colorAttribID;
  GLuint _textureAttribID;

  // These IDs point to actual data.
  GLuint _vertexBufferID;
	GLuint _uvBufferID;
	GLuint _normalBufferID;
  GLuint _colorBufferID;
  GLuint _textureBufferID;

  GLuint _mvpMatrixID;
	GLuint _viewMatrixID;
	GLuint _modelMatrixID;

  glm::mat4 _modelMatrix;
  
  std::vector<glm::vec3> _vertices;
	std::vector<glm::vec2> _uvs;
	std::vector<glm::vec3> _normals;
	//std::vector<glm::vec3> _colors;


public:
  virtual void load(GLuint programID) = 0;
  virtual void draw(GLuint programID, VRControl control) = 0;
};

class shapeObj : public shape {
private:

  // Some of these should move into the parent class.  Also the destructor.
  GLuint _lightID;

public:
  ~shapeObj() {
    // Can we test if these are in use, and delete if so? If yes, move
    // this into parent class.

    // Cleanup VBO and shader
    glDeleteBuffers(1, &_vertexBufferID);
    glDeleteBuffers(1, &_uvBufferID);
    glDeleteBuffers(1, &_normalBufferID);
    glDeleteTextures(1, &_textureBufferID);
    glDeleteVertexArrays(1, &_arrayID);
  }



  void load(GLuint programID) {

    // Arrange the data for the shaders to work on.  "Uniforms" first.
    // Get a handle for our "MVP" uniform
    _mvpMatrixID = glGetUniformLocation(programID, "MVP");
    _viewMatrixID = glGetUniformLocation(programID, "V");
    _modelMatrixID = glGetUniformLocation(programID, "M");

    // Load the texture
    _textureBufferID = loadDDS("uvmap.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
    _textureAttribID  = glGetUniformLocation(programID, "myTextureSampler");

    std::cout << "loading shapeObj" << std::endl;
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
    glUseProgram(programID);
    _lightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Get handles for the various shader inputs.
    _vertexAttribID =
      glGetAttribLocation(programID, "vertexPosition_modelspace");
    _uvAttribID = glGetAttribLocation(programID, "vertexUV");
    _normalAttribID = 
      glGetAttribLocation(programID, "vertexNormal_modelspace");

  }
  
  void draw(GLuint programID, VRControl control) {

		// Use our shader
		glUseProgram(programID);

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
			_uvAttribID,                      // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(_normalAttribID);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
		glVertexAttribPointer(
			_normalAttribID,                  // attribute
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
  }
};  

class shapeAxes : public shape {

  // =========== Axis Data ===================
  //
  //      Y
  //      |           Z
  //      |         /
  //      |       /
  //      |     /
  //      |   /
  //      | /
  //      /--------------
  //      O              X

  static const int nCoordsComponents = 3;
  static const int nColorComponents = 3;
  static const int nLines = 3;
  static const int nVerticesPerLine = 2;
  static const int nFaces = 6;
  static const int nVerticesPerFace = 3;

  float ave[nLines*nVerticesPerLine*nCoordsComponents];
  void expandAxesVertices() {

    float av[12] = { 0.0, 0.0, 0.0,    // origin
                     4.0, 0.0, 0.0,    // x-axis
                     0.0, 4.0, 0.0,    // y-axis
                     0.0, 0.0, 4.0 };  // z-axis
    
    GLubyte avi[6] = { 0, 1,
                       0, 2,
                       0, 3 };

    for (int i=0; i<6; i++) {
      ave[i*3+0] = av[avi[i]*3+0];
      ave[i*3+1] = av[avi[i]*3+1];
      ave[i*3+2] = av[avi[i]*3+2];
    }
  }

  float ace[nLines*nVerticesPerLine*nColorComponents];
  void expandAxesColors() {

    float ac[9] = { 1.0, 0.0, 0.0,    // red   x-axis
                    0.0, 1.0, 0.0,    // green y-axis
                    0.0, 0.0, 1.0 };  // blue  z-axis

    GLubyte aci[6] = { 0, 0,
                       1, 1,
                       2, 2 };

    for (int i=0; i<6; i++) {
      ace[i*3+0] = ac[aci[i]*3+0];
      ace[i*3+1] = ac[aci[i]*3+1];
      ace[i*3+2] = ac[aci[i]*3+2];
    }
  }
  
public:
  shapeAxes() {
    expandAxesVertices();
    expandAxesColors();
  }    
  
  void load(GLuint programID) {

    _mvpMatrixID = glGetUniformLocation(programID, "MVP");

    glGenVertexArrays(1, &_arrayID);
    glBindVertexArray(_arrayID);

    glGenBuffers(1, &_vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);  // coordinates
    glBufferData(GL_ARRAY_BUFFER, sizeof(ave), ave, GL_STATIC_DRAW);

    glGenBuffers(1, &_colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, _colorBufferID);  // color
    glBufferData(GL_ARRAY_BUFFER, sizeof(ace), ace, GL_STATIC_DRAW);

  }
  
  void draw(GLuint programID, VRControl control) {

    // We have to ask where these attributes are located.
    _vertexAttribID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    _colorAttribID = glGetAttribLocation(programID, "vertexColor");

    glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = control.getProjectionMatrix();
		glm::mat4 ViewMatrix = control.getViewMatrix();
    _modelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * _modelMatrix;
    
    glUniformMatrix4fv(_mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    // Just checking...
    // GLint count;
    // glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);
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
};  


class VRApp {
public:
  GLFWwindow* _window;
  GLuint _programID;
  GLuint _axisProgramID;

  VRControl control;

  shapeObj suzanne;
  shapeAxes axes;
  
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
    mvShaders shaders = mvShaders("StandardShading.vertexshader", "",
                                  "StandardShading.fragmentshader");
    _programID = shaders.getProgram();
    
    suzanne.load(_programID);

    // Switch to axes

    mvShaders axisShaders = mvShaders(&axis_vertex_shader, NULL,
                                      &axis_fragment_shader);

    _axisProgramID = axisShaders.getProgram();

    axes.load(_axisProgramID);

  };

  ~VRApp() {

    glDeleteProgram(_programID);
    glDeleteProgram(_axisProgramID);

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

    suzanne.draw(_programID, control);

    // Use our other shader.
    axes.draw(_axisProgramID, control);
    
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
