#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
bool coll = false;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};

typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL){
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL){
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
  
bool fired = GL_FALSE;

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao){
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height){
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	  GLfloat fov = 90.0f;

	  // sets the viewport of openGL renderer
	  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	  // set the projection matrix as perspective
	  
    /* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	  
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

float camera_rotation_angle = 90;

VAO* createPolygon (float radius, int sides, GLfloat red, GLfloat green, GLfloat blue){
  // declare vertex and color data for polygon
  GLfloat vertex_buffer_data [sides * 9];
  GLfloat color_buffer_data [sides * 12];

  // initialize color array with all black
  for (int i = 0; i < sides * 12; i++)
  {
    color_buffer_data[i] = 0;
  }
  // initialize first vertex to (0, 0, 0)
  vertex_buffer_data[0] = 0.0;
  vertex_buffer_data[1] = 0.0;
  vertex_buffer_data[2] = 0.0;

  int base = 0;
  double baseAngle = 2 * M_PI/sides;

  for(int i = 1; i <= sides; i++)
  {

    vertex_buffer_data[base + 0] = 0.0;
    vertex_buffer_data[base + 1] = 0.0;
    vertex_buffer_data[base + 2] = 0.0;
    vertex_buffer_data[base + 3] = radius * cos(baseAngle * i);
    vertex_buffer_data[base + 4] = radius * sin(baseAngle * i);
    vertex_buffer_data[base + 5] = 0.0;
    vertex_buffer_data[base + 6] = radius * cos(baseAngle * (i + 1));
    vertex_buffer_data[base + 7] = radius * sin(baseAngle * (i + 1));
    vertex_buffer_data[base + 8] = 0.0;

    base += 9;
  
  }
  return create3DObject(GL_TRIANGLES, sides * 3, vertex_buffer_data, red, blue, green, GL_FILL);
}
  

class CoordinateVector {

  public:
    double x, y, z;
    // constructor (as __init__ in python)
    CoordinateVector(double x = 0, double y = 0, double z = 0){
      CoordinateVector::x = x;
      CoordinateVector::y = y;
      CoordinateVector::z = z;
    }

    // '+' defined for vectors - addition of vectors
    CoordinateVector operator + (CoordinateVector &one){
      return CoordinateVector(x + one.x, y + one.y, z + one.z);
    }

    // '-' defined for vectors - subtraction of vectors
    CoordinateVector operator - (CoordinateVector &one){
      return CoordinateVector(x - one.x, y - one.y, z - one.z);
    }

    // '*' defined for vectors and scalars - multiplication of a scalar with vectors
    CoordinateVector operator * (float m){
      return CoordinateVector(x * m, y * m, z * m);
    }

    // get glm vector for CoordinateVector
    glm::vec3 getGlmVector(){
      return glm::vec3(x, y, z);
    }
};

class GameObject{
  
  VAO *drawable;
  
  int sides;
  float radius;

  float rotationAngle;
  float rotationValue;

  CoordinateVector position;
  CoordinateVector velocity;
  CoordinateVector acceleration;
  
  public:
    GLfloat red, green, blue;
    int score, value;
    GameObject(float circleRadius = 1, int numberOfSides = 3){
      
      radius = circleRadius;
      sides = numberOfSides;

      score = 0;
      value = 0;

      rotationAngle = 180.0 / sides;
      rotationValue = 0;
      
      position = CoordinateVector(0, 0, 0);
      velocity = CoordinateVector(0, 0, 0);
      acceleration = CoordinateVector(0, 0, 0);
      
      drawable = NULL;
      setColors();  // will be initialized with default colors if no arguments are passed
      createDrawable (radius, sides); // for first time creation of drawable
    }

    // get radius of the circumcircle
    float getRadius(){
      return radius;
    }

    float getRotationAngle(){
      return rotationAngle;
    }

    void setRotationAngle(float angle){
      rotationAngle = angle;
    }

    float getRotationValue(){
      return rotationValue;
    }

    void setRotationValue(float angle){
      rotationValue = angle;
    }

    // get position
    CoordinateVector getPosition(){
      return position;
    }

    // set position
    void setPosition(double x = 0, double y = 0, double z = 0){
      position = CoordinateVector(x, y, z);
    }

    // set position from CoordinateVector
    void setPosition(CoordinateVector one){
      position = one;
    }

    // get velocity
    CoordinateVector getVelocity(){
      return velocity;
    }

    // set velocity
    void setVelocity(double x = 0, double y = 0, double z = 0){
      velocity = CoordinateVector(x, y, z);
    }

    // set velocity from CoordinateVector
    void setVelocity(CoordinateVector one){
      velocity = one;
    }

    // get acceleration
    CoordinateVector getAcceleration(){
      return acceleration;
    }

    // set acceleration
    void setAcceleration(double x = 0, double y = 0, double z = 0){
      acceleration = CoordinateVector(x, y, z);
    }

    // will set radius and sides of the object when called for the first time
    void createDrawable(float circleRadius, int numberOfSides){
      drawable = createPolygon (circleRadius, numberOfSides, red, green, blue); // with default or first time values passed into
    }

    // create the object ready to be drawn
    void updateDrawable(){
      drawable = createPolygon (radius, sides, red, green, blue); // with updated values
    }


    // set the colors of object
    void setColors(GLfloat r = 0, GLfloat g = 0, GLfloat b = 0){
      red = r;
      green = g;
      blue = b;
      if(drawable) { // if drawable exists then update it with new colors
        updateDrawable();
      }
    }

    // get the object ready to be drawn
    VAO* getDrawable(){
      return drawable;
    }

    // change velocity of the object
    void changeVelocity(CoordinateVector change){
      velocity = velocity + change; // the '+' used here is defined in CoordinateVector Class
    }

    // change acceleration of the object
    void changeAcceleration(CoordinateVector change){
      acceleration = acceleration + change; // the '+' used here is defined in CoordinateVector Class
    }

    // update state of the object: keep updating velocity till we have acceleration and position till we have velocity
    void updateState(){
      //cout<<"position : "<<position.x<<" "<<position.y<<" "<<position.z<<endl;
      //mcout<<"velocity : "<<velocity.x<<" "<<velocity.y<<" "<<velocity.z<<endl;
      velocity = velocity + acceleration; 
      position = position + velocity;
      if(position.x <= -4){
        velocity.x = -1 * velocity.x;
      }
    }
};

class LineObject {
  
  public:
    VAO* drawable;
    GLfloat vertex_buffer_data[6] = {0, 0, 0, 0, 0, 0};
    GLfloat color_buffer_data[6] = {0, 0, 0, 0, 0, 0};
    
    LineObject(GLfloat vertex_buffer_data[], GLfloat color_buffer_data[]){
      for(int i = 0; i < 6; i++){
        LineObject::vertex_buffer_data[i] = vertex_buffer_data[i];
        LineObject::color_buffer_data[i] = color_buffer_data[i];
      }
      createDrawable();
    }
    
    VAO* getDrawable(){
      return drawable;
    }

    void createDrawable(){
      drawable = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
    }

    void updateDrawable(){
      drawable = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
    }

    void setVertexBufferData(GLfloat vertex_buffer_data[]){
      for(int i = 0; i < 6; i++){
        LineObject::vertex_buffer_data[i] = vertex_buffer_data[i];
      }
    }

    CoordinateVector getFirstVertex(){
      return CoordinateVector(vertex_buffer_data[0], vertex_buffer_data[1], vertex_buffer_data[2]);
    }

    CoordinateVector getSecondVertex(){
      return CoordinateVector(vertex_buffer_data[3], vertex_buffer_data[4], vertex_buffer_data[5]);
    }
};

map<string, GameObject> allGameObjects;  // dictionary containing all game objects

bool CheckCollision(GameObject one, GameObject two) {
  double currentDistance = sqrt( pow((one.getPosition().x - two.getPosition().x), 2) + pow((one.getPosition().y - two.getPosition().y), 2));  // length of line joining the centres

  double actualDistance = one.getRadius() + two.getRadius();  // sum of radii of circumcircles of both VAO Objects 
    
  // collision if distance between their centres is less than the sum
  return currentDistance <= actualDistance; 
}

void increaseSpeed(){
  
  double x = allGameObjects["player"].getPosition().x - 0.1;
  double y = allGameObjects["player"].getPosition().y - 0.1;
  double z = allGameObjects["player"].getPosition().z;

  allGameObjects["player"].setPosition(x, y, z);
}

int first = 1;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods){
    
    // Function is called first on GLFW_PRESS.
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                // do something ..
                break;
            case GLFW_KEY_S:
                // do something ..
                if(first){
                  fired = GL_TRUE;
                  allGameObjects["player"].setVelocity( 
                    allGameObjects["cannon4"].getPosition().x * 0.1 - 0.1 * allGameObjects["player"].getPosition().x, 
                    allGameObjects["cannon4"].getPosition().y * 0.1 - 0.1 * allGameObjects["player"].getPosition().y, 
                    0);
                  allGameObjects["player"].setAcceleration(-0.00005 * allGameObjects["player"].getVelocity().x , -0.0006, 0);
                  first = 0;
                }
                break;
            case GLFW_KEY_R:
                // replay
                first = 1;
                fired = GL_FALSE;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key){
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods){
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE){
                if(first){
                  fired = GL_TRUE;
                  allGameObjects["player"].setVelocity( 
                    allGameObjects["cannon4"].getPosition().x * 0.1 - 0.1 * allGameObjects["player"].getPosition().x, 
                    allGameObjects["cannon4"].getPosition().y * 0.1 - 0.1 * allGameObjects["player"].getPosition().y, 
                    0);
                  allGameObjects["player"].setAcceleration(-0.00005 * allGameObjects["player"].getVelocity().x , -0.0006, 0);
                  first = 0;
                }
              }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
              //allGameObjects["player"].setVelocity(allGameObjects["player"].getVelocity() * -1);
            }
            break;
        default:
            break;
    }
}

void drawPolygon (GameObject polygon){
  
  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;  // MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  //cout << "position : " << polygon.getPosition().x << " " << polygon.getPosition().y << " " << polygon.getPosition().z << endl;
  glm::mat4 translateMatrix = glm::translate (polygon.getPosition().getGlmVector());        // glTranslatef
  glm::mat4 rotateMatrix = glm::rotate((float)((polygon.getRotationAngle() + polygon.getRotationValue()) * M_PI/180.0f), glm::vec3(0, 0, 1));  // rotate about vector (1,0,0)
  
  //Matrices.model *= translateMatrix;
  Matrices.model *= translateMatrix * rotateMatrix;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(polygon.getDrawable());
}

void drawPolygonLine (VAO* line){
  
  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;  // MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  //cout << "position : " << polygon.getPosition().x << " " << polygon.getPosition().y << " " << polygon.getPosition().z << endl;
  //glm::mat4 translateMatrix = glm::translate (polygon.getPosition().getGlmVector());        // glTranslatef
  //glm::mat4 rotateMatrix = glm::rotate((float)((polygon.getRotationAngle() + polygon.getRotationValue()) * M_PI/180.0f), glm::vec3(0, 0, 1));  // rotate about vector (1,0,0)
  
  //Matrices.model *= translateMatrix;
  //Matrices.model *= translateMatrix * rotateMatrix;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(line);
}


void createGround(){
  
  int sides = 200;
  allGameObjects["ground1"] = GameObject(1.2, sides);
  allGameObjects["ground1"].setPosition(-3.2, -3.2, 0);
  allGameObjects["ground1"].setVelocity(0, 0, 0);
  allGameObjects["ground1"].setAcceleration(0, 0, 0);
  allGameObjects["ground1"].setColors(0.3, 0.1, 0);

  allGameObjects["ground2"] = GameObject(1.2, sides);
  allGameObjects["ground2"].setPosition(-1.6, -3.2, 0);
  allGameObjects["ground2"].setVelocity(0, 0, 0);
  allGameObjects["ground2"].setAcceleration(0, 0, 0);
  allGameObjects["ground2"].setColors(0.3, 0.1, 0);

  allGameObjects["ground3"] = GameObject(1.2, sides);
  allGameObjects["ground3"].setPosition(0, -3.2, 0);
  allGameObjects["ground3"].setVelocity(0, 0, 0);
  allGameObjects["ground3"].setAcceleration(0, 0, 0);
  allGameObjects["ground3"].setColors(0.3, 0.1, 0);

  allGameObjects["ground4"] = GameObject(1.2, sides);
  allGameObjects["ground4"].setPosition(1.6, -3.2, 0);
  allGameObjects["ground4"].setVelocity(0, 0, 0);
  allGameObjects["ground4"].setAcceleration(0, 0, 0);
  allGameObjects["ground4"].setColors(0.3, 0.1, 0);

  allGameObjects["ground5"] = GameObject(1.2, sides);
  allGameObjects["ground5"].setPosition(3.2, -3.2, 0);
  allGameObjects["ground5"].setVelocity(0, 0, 0);
  allGameObjects["ground5"].setAcceleration(0, 0, 0);
  allGameObjects["ground5"].setColors(0.3, 0.1, 0);
}

void createTree(){

  // one tree
  allGameObjects["tree14"] = GameObject(0.1, 4);
  allGameObjects["tree14"].setPosition(3.2, -2.0, 0);
  allGameObjects["tree14"].setColors(0.3, 0.1, 0);
  allGameObjects["tree14"].setRotationAngle(45);
  allGameObjects["tree14"].value = 10;

  allGameObjects["tree11"] = GameObject(0.3, 3);
  allGameObjects["tree11"].setPosition(3.2, -1.8, 0);
  allGameObjects["tree11"].setColors(0.3, 0.4, 0.1);
  allGameObjects["tree11"].setRotationAngle(90);
  allGameObjects["tree11"].value = 10;

  allGameObjects["tree12"] = GameObject(0.3, 3);
  allGameObjects["tree12"].setPosition(3.2, -1.6, 0);
  allGameObjects["tree12"].setColors(0.3, 0.8, 0.1);
  allGameObjects["tree12"].setRotationAngle(90);
  allGameObjects["tree12"].value = 10;

  allGameObjects["tree13"] = GameObject(0.3, 3);
  allGameObjects["tree13"].setPosition(3.2, -1.4, 0);
  allGameObjects["tree13"].setColors(0.2, 0.8, 0.1);
  allGameObjects["tree13"].setRotationAngle(90);
  allGameObjects["tree13"].value = 10;

  // next tree
  allGameObjects["tree24"] = GameObject(0.1, 4);
  allGameObjects["tree24"].setPosition(1.6, -2.0, 0);
  allGameObjects["tree24"].setColors(0.3, 0.1, 0);
  allGameObjects["tree24"].setRotationAngle(45);
  allGameObjects["tree24"].value = 10;

  allGameObjects["tree21"] = GameObject(0.3, 3);
  allGameObjects["tree21"].setPosition(1.6, -1.8, 0);
  allGameObjects["tree21"].setColors(0.4, 0.4, 0.1);
  allGameObjects["tree21"].setRotationAngle(90);
  allGameObjects["tree21"].value = 10;

  allGameObjects["tree22"] = GameObject(0.2, 3);
  allGameObjects["tree22"].setPosition(1.6, -1.6, 0);
  allGameObjects["tree22"].setColors(0.4, 0.8, 0.1);
  allGameObjects["tree22"].setRotationAngle(90);
  allGameObjects["tree22"].value = 10;
}

void createCannon(){

  allGameObjects["cannon0"] = GameObject(0.3, 4); // ground touching cannon
  allGameObjects["cannon0"].setPosition(-2.8, -2.2, 0);
  allGameObjects["cannon0"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon0"].setRotationAngle(45);

  allGameObjects["cannon1"] = GameObject(0.3, 4);
  allGameObjects["cannon1"].setPosition(-2.8, -2.0, 0);
  allGameObjects["cannon1"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon1"].setRotationAngle(45);

  allGameObjects["cannon2"] = GameObject(0.3, 4);
  allGameObjects["cannon2"].setPosition(-2.8, -1.8, 0);
  allGameObjects["cannon2"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon2"].setRotationAngle(45);

  allGameObjects["cannon3"] = GameObject(0.3, 4);
  allGameObjects["cannon3"].setPosition(-2.8, -1.6, 0);
  allGameObjects["cannon3"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon3"].setRotationAngle(45);

  allGameObjects["cannon5"] = GameObject(0.3, 4);
  allGameObjects["cannon5"].setPosition(-2.8, -1.4, 0);
  allGameObjects["cannon5"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon5"].setRotationAngle(45);

  allGameObjects["cannon6"] = GameObject(0.3, 4);
  allGameObjects["cannon6"].setPosition(-2.8, -1.2, 0);
  allGameObjects["cannon6"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon6"].setRotationAngle(45);

  allGameObjects["cannon4"] = GameObject(0.3, 4); // uppermost cannnon
  allGameObjects["cannon4"].setPosition(-2.8, -1.0, 0);
  allGameObjects["cannon4"].setColors(0.3, 0.2, 0.1);
  allGameObjects["cannon4"].setRotationAngle(45);
}

void createAllObjects(){

  createGround();
  createCannon();
  createTree();
  
  allGameObjects["player"] = GameObject(0.2, 5);
  
  allGameObjects["player"].setPosition(
    allGameObjects["cannon3"].getPosition().x, 
    allGameObjects["cannon3"].getPosition().y + 0.4, 
    allGameObjects["cannon3"].getPosition().z
    );

  allGameObjects["player"].setVelocity(0, 0, 0);
  allGameObjects["player"].setAcceleration(0, 0, 0);

  // pigs
  allGameObjects["pig1"] = GameObject(0.17, 6);
  allGameObjects["pig1"].setPosition(0.745, -2.1, 0);
  allGameObjects["pig1"].setColors(0.2, 0.9, 0);
  allGameObjects["pig1"].value = 50;

  allGameObjects["pig2"] = GameObject(0.17, 7);
  allGameObjects["pig2"].setPosition(2.9, -1.9, 0);
  allGameObjects["pig2"].setColors(40, 2, 0);
  allGameObjects["pig2"].value = 50;

  allGameObjects["pig3"] = GameObject(0.17, 7);
  allGameObjects["pig3"].setPosition(3.2, -1.0, 0);
  allGameObjects["pig3"].setColors(40, 2, 0);
  allGameObjects["pig3"].value = 50;

  //goals
  allGameObjects["goal1"] = GameObject(0.18, 200);
  allGameObjects["goal1"].setPosition(1.5, 0, 0);
  allGameObjects["goal1"].setColors(0.5, 0.2, 0.1);
  allGameObjects["goal1"].value = 100;

  allGameObjects["goal2"] = GameObject(0.2, 200);
  allGameObjects["goal2"].setPosition(-0.0, 0.8, 0);
  allGameObjects["goal2"].setColors(0.5, 0.2, 0.1);
  allGameObjects["goal2"].value = 100;

  allGameObjects["goal3"] = GameObject(0.18, 200);
  allGameObjects["goal3"].setPosition(0.5, -1.0, 0);
  allGameObjects["goal3"].setColors(0.5, 0.2, 0.1);
  allGameObjects["goal3"].value = 100;
}

void setCamera(){
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), target, up); // Fixed camera for 2D (ortho) in XY plane
}

double distance(double x, double y, double u, double v){
  return sqrt(pow(u - x, 2) + pow(v - y, 2));
}

void drawAllObjects (){
  
  setCamera();
  
  double s = .0;

  if(!fired){
    GLfloat vbd[]={
      allGameObjects["cannon4"].getPosition().x, allGameObjects["cannon4"].getPosition().y + 0.1, 0,
      allGameObjects["player"].getPosition().x, allGameObjects["player"].getPosition().y, 0
    };
    
    GLfloat color_buffer_data[]={
      2, 3, 0,
      2.6, 3.5, 0
    };

    LineObject line1(vbd, color_buffer_data);
    drawPolygonLine(line1.getDrawable());
    s = distance(vbd[0], vbd[1], vbd[3], vbd[4]);
  }

  // draw the power line
  if(s > 3.0){
    s = 3.0;
  }

  GLfloat vertex_buffer_data0[]={
      -3.0, 3.5, 0,
      s - 3.0, 3.5, 0
    };
    
    GLfloat color_buffer_data[]={
      0.1, 0.9, 0.4,
      0.6, 0.5, 0.4
    };

    LineObject powerline(vertex_buffer_data0, color_buffer_data);
    drawPolygonLine(powerline.getDrawable());

  // draw all models one by one
  for(map<string, GameObject> :: iterator it = allGameObjects.begin(); it != allGameObjects.end(); it++){
    drawPolygon(it->second);
  }

  if(!fired){
    GLfloat vertex_buffer_data2[] = {
      allGameObjects["cannon4"].getPosition().x, allGameObjects["cannon4"].getPosition().y, 0,
      allGameObjects["player"].getPosition().x, allGameObjects["player"].getPosition().y, 0
    };
    
    GLfloat color_buffer_data[] = {
      2, 3, 0,
      2.6, 3.5, 0
    };

    LineObject line2(vertex_buffer_data2, color_buffer_data);
    drawPolygonLine (line2.getDrawable());
  }

}

// update all objects by usual values
void updateAllObjects(){
  for(map<string, GameObject> :: iterator it = allGameObjects.begin() ; it != allGameObjects.end(); it++){
    it->second.updateState();
  }
}

void CollisionResponse(GameObject &one, GameObject &two) {
  
  //cout<<"striked\n";
  CoordinateVector ra = one.getPosition();
  CoordinateVector rb = two.getPosition();
  CoordinateVector vai = one.getVelocity();
  CoordinateVector vbi = two.getVelocity();
  CoordinateVector vaf;
  CoordinateVector vbf;

  //cout<<"two ini : "<<vbi.x<<" "<<vbi.y<<endl;
  double e = 0.5;
  double ma = one.getRadius();
  double mb = two.getRadius();
  double Ia = one.getRadius();
  double Ib = two.getRadius();

  double k = 1/(ma*ma)+ 2/(ma*mb) +1/(mb*mb) - ra.x*ra.x/(ma*Ia) - rb.x*rb.x/(ma*Ib)  - ra.y*ra.y/(ma*Ia)
    - ra.y*ra.y/(mb*Ia) - ra.x*ra.x/(mb*Ia) - rb.x*rb.x/(mb*Ib) - rb.y*rb.y/(ma*Ib)
    - rb.y*rb.y/(mb*Ib) + ra.y*ra.y*rb.x*rb.x/(Ia*Ib) + ra.x*ra.x*rb.y*rb.y/(Ia*Ib) - 2*ra.x*ra.y*rb.x*rb.y/(Ia*Ib);

  double Jx = (e+1)/k * (vai.x - vbi.x) * ( 1/ma - ra.x*ra.x/Ia + 1/mb - rb.x*rb.x/Ib)
     - (e+1)/k * (vai.y - vbi.y) * (ra.x*ra.y / Ia + rb.x*rb.y / Ib);

  double Jy = - (e+1)/k * (vai.x - vbi.x) * (ra.x*ra.y / Ia + rb.x*rb.y / Ib)
     + (e+1)/k  * (vai.y - vbi.y) * ( 1/ma - ra.y*ra.y/Ia + 1/mb - rb.y*rb.y/Ib);

  vaf.x = vai.x - Jx/ma;
  vaf.y = vai.y - Jy/ma;
  vbf.x = vbi.x - Jx/mb;
  vbf.y = vbi.y - Jy/mb;

  //cout<<"one : "<<vaf.x<<" "<<vaf.y<<endl;
  one.setVelocity(vaf.x, vaf.y , 0);
}

int c = 0;

// handle all collisions
void handleCollisions(){
  
  if(fired){
    for(map<string, GameObject> :: iterator it = allGameObjects.begin(); it != allGameObjects.end(); it++){
      
      bool collided = CheckCollision(allGameObjects["player"], it->second);
      
      if(collided && 
        it->first != "player" && it->first != "cannon0" && it->first != "cannon1" && it->first != "cannon2" && it->first != "cannon3" && it->first != "cannon4" && it->first != "cannon5" && it->first != "cannon6"){
        
        // ground
        if(it->first == "ground5" || it->first == "ground4" || it->first == "ground3" || it->first == "ground2" || it->first == "ground1"){
          //CollisionResponse(allGameObjects["player"], it->second);
          allGameObjects["player"].setVelocity(allGameObjects["player"].getVelocity() * -0.9);
          allGameObjects["player"].setRotationValue(0);
          c++;
          if(allGameObjects["player"].getPosition().y <= -2.0){
            allGameObjects["player"].setPosition(allGameObjects["player"].getPosition().x, -2.0, 0);
            allGameObjects["player"].setAcceleration(0, 0, 0);
            allGameObjects["player"].setVelocity(0, 0, 0);
            allGameObjects["player"].setRotationValue(0);
          }
        }

        // not ground
        else {

          it->second.score++;
          it->second.setPosition(it->second.getPosition().x + 0.005, it->second.getPosition().y + 0.005, 0);
        
          if(it->first == "goal1" || it->first == "goal2" || it->first == "goal3" || it->second.score > 25) {
            allGameObjects["player"].score += it->second.value;
            cout<<"score : "<<allGameObjects["player"].score<<endl;
            allGameObjects.erase(it);
          }

          it->second.setColors(it->second.red, it->second.green, 0.2);
          //CollisionResponse(allGameObjects["player"], it->second);
          allGameObjects["player"].setVelocity(allGameObjects["player"].getVelocity() * -0.8);
          allGameObjects["player"].setRotationValue(allGameObjects["player"].getRotationValue() + 5);

        }
      }
    }
  }

}


/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height){
  // Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
//void glBegin(GLenum mode);
//void glEnd();
//void glVertex2d(GLfloat x, GLfloat y);

int main (int argc, char** argv){
	int width = 600;
	int height = 600;

  GLFWwindow* window = initGLFW(width, height);

  /* Objects should be created before any other gl function and shaders */
  createAllObjects ();

  /* Initialize the OpenGL rendering properties */
	initGL (window, width, height);
  
  double xcoor, ycoor;
  double last_update_time = glfwGetTime(), current_time;

  GLfloat line_data[6];
  
  line_data[0] = allGameObjects["cannon4"].getPosition().x;
  line_data[1] = allGameObjects["cannon4"].getPosition().y + 0.1;
  line_data[2] = 0;
  line_data[3] = allGameObjects["player"].getPosition().x;
  line_data[4] = allGameObjects["player"].getPosition().y;
  line_data[5] = 0;

  /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {
    
    line_data[3] = allGameObjects["player"].getPosition().x;
    line_data[4] = allGameObjects["player"].getPosition().y;

    handleCollisions();

    //cout<<"initially "<<allGameObjects["player"].getPosition().x<<" "<<allGameObjects["player"].getPosition().y<<endl;
    // get cursor position
    glfwGetCursorPos(window, &xcoor, &ycoor);
    //cout<<"mouse "<<xcoor<<" "<<ycoor<<" "<<endl;
    if(!fired){
      allGameObjects["player"].setPosition(xcoor/75.0 - 4, 4.0 - 1.0 * ycoor/75.0, 0.0);
    }
    //cout<<"finally "<<allGameObjects["player"].getPosition().x<<" "<<allGameObjects["player"].getPosition().y<<endl<<endl;
    // first need to update states of all objects
    updateAllObjects();

    // then draw all
    drawAllObjects();

    //mcout<<line_data[3]<<endl;
    //if(allGameObjects["player"].getVelocity().x == 2.22222e-06 || allGameObjects["player"].getVelocity().y == 4.44444e-05)
    allGameObjects["player"].setRotationValue(allGameObjects["player"].getRotationValue() + 5);

    // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

    // Poll for Keyboard and mouse events
    glfwPollEvents();

    // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    current_time = glfwGetTime(); // Time in seconds
    if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
      // do something every 0.5 seconds ..
        last_update_time = current_time;
      }
  }

  // exit functions
  glfwTerminate();
  exit(EXIT_SUCCESS);
}