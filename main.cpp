#include <GL/glew.h>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <stddef.h>
#include "tiny_obj_loader.h"
#include "MyGui.h"

#define GLM_FORCE_RADIANS
//
struct object_struct{
	unsigned int program;
	unsigned int vao;
	unsigned int vbo[4];
	unsigned int texture;
	glm::mat4 model;
	object_struct(): model(glm::mat4(1.0f)){}
} ;

std::vector<object_struct> objects;
//vertex array object,vertex buffer object and texture(color) for objs
unsigned int program1, gui_shader, shaders[3];
std::vector<int> indicesCount;

Gui gui;
glm::vec4 camera_location, up_direction;
float x_deg, y_deg, z_deg, zoomin;
int x_dir = 0, y_dir = 0, z_dir = 0, zoom_dir = 0;

int show_obj;

std::string filenames[4];
int mouse_state = GLFW_RELEASE;
int now_mesh, now_times, now_algor=16, now_shader=13;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static unsigned int setup_shader(const char *vertex_shader, const char *fragment_shader)
{
	GLuint vs=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const GLchar**)&vertex_shader, nullptr);

	glCompileShader(vs);

	int status, maxLength;
	char *infoLog=nullptr;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE)
	{
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];

		glGetShaderInfoLog(vs, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Vertex Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}

	GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const GLchar**)&fragment_shader, nullptr);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE)
	{
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];

		glGetShaderInfoLog(fs, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Fragment Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}

	unsigned int program=glCreateProgram();
	// Attach our shaders to our program
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if(status==GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);


		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];
		glGetProgramInfoLog(program, maxLength, NULL, infoLog);

		glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Link Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}
	return program;
}

static std::string readfile(const char *filename)
{
	std::ifstream ifs(filename);
	if(!ifs)
		exit(EXIT_FAILURE);
	return std::string( (std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
}

// mini bmp loader written by HSU YOU-LUN
static unsigned char *load_bmp(const char *bmp, unsigned int *width, unsigned int *height, unsigned short int *bits)
{
	unsigned char *result=nullptr;
	FILE *fp = fopen(bmp, "rb");
	if(!fp)
		return nullptr;
	char type[2];
	unsigned int size, offset;
	// check for magic signature
	fread(type, sizeof(type), 1, fp);
	if(type[0]==0x42 || type[1]==0x4d){
		fread(&size, sizeof(size), 1, fp);
		// ignore 2 two-byte reversed fields
		fseek(fp, 4, SEEK_CUR);
		fread(&offset, sizeof(offset), 1, fp);
		// ignore size of bmpinfoheader field
		fseek(fp, 4, SEEK_CUR);
		fread(width, sizeof(*width), 1, fp);
		fread(height, sizeof(*height), 1, fp);
		// ignore planes field
		fseek(fp, 2, SEEK_CUR);
		fread(bits, sizeof(*bits), 1, fp);
		unsigned char *pos = result = new unsigned char[size-offset];
		fseek(fp, offset, SEEK_SET);
		while(size-ftell(fp)>0)
			pos+=fread(pos, 1, size-ftell(fp), fp);
	}
	fclose(fp);
	return result;
}

static int add_obj(const char *filename)
{
	object_struct new_node;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, filename);

	if (!err.empty()||shapes.size()==0)
	{
		std::cerr<<err<<std::endl;
		exit(1);
	}

	glGenVertexArrays(1, &new_node.vao);
	glGenBuffers(4, new_node.vbo);
	glGenTextures(1, &new_node.texture);

	glBindVertexArray(new_node.vao);

	// Upload postion array
	glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.positions.size(),
			shapes[0].mesh.positions.data(), GL_STATIC_DRAW);
	//printf("pos size: %d\n", shapes[0].mesh.positions.size());


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	if(shapes[0].mesh.texcoords.size()>0)
	{

		// Upload texCoord array
		glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.texcoords.size(),
				shapes[0].mesh.texcoords.data(), GL_STATIC_DRAW);
	}


	if(shapes[0].mesh.normals.size()>0)
	{
		GLint n = shapes[0].mesh.normals.size();

		// Upload normal array
		glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*shapes[0].mesh.normals.size(),
				shapes[0].mesh.normals.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_node.vbo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*shapes[0].mesh.indices.size(),
			shapes[0].mesh.indices.data(), GL_STATIC_DRAW);

	indicesCount.push_back(shapes[0].mesh.indices.size());

	glBindVertexArray(0);

	objects.push_back(new_node);

	printf("add obj %s as id:%d\n", filename, objects.size()-1);
	return objects.size()-1;
}

static void releaseObjects()
{
	for(int i=0;i<objects.size();i++){
		glDeleteVertexArrays(1, &objects[i].vao);
		glDeleteTextures(1, &objects[i].texture);
		glDeleteBuffers(4, objects[i].vbo);
	}

	glDeleteVertexArrays(1, &gui.background.vao);
	glDeleteTextures(1, &gui.background.texture);
	glDeleteBuffers(4, gui.background.vbo);

	for(int i=0; i<gui.button_num; ++i)
	{
		glDeleteVertexArrays(1, &gui.buttons[i].vao);
		glDeleteTextures(1, &gui.buttons[i].texture[0]);
		glDeleteTextures(1, &gui.buttons[i].texture[1]);
		glDeleteBuffers(4, gui.buttons[i].vbo);
	}
	//glDeleteProgram(program);
}

static void setUniformMat4(unsigned int program, const std::string &name, const glm::mat4 &mat)
{
	// This line can be ignore. But, if you have multiple shader program
	// You must check if currect binding is the one you want
	glUseProgram(program);
	GLint loc=glGetUniformLocation(program, name.c_str());
	if(loc==-1) return;

	// mat4 of glm is column major, same as opengl
	// we don't need to transpose it. so..GL_FALSE
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

static void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setUniformMat4(program1, "model", objects[0].model);

	glUseProgram(program1);
	glBindVertexArray(objects[0].vao);
	glBindTexture(GL_TEXTURE_2D, objects[0].texture);
	//you should send some data to shader here
	glDrawElements(GL_TRIANGLES, indicesCount[0], GL_UNSIGNED_INT, nullptr);

	//render ui objects, background first
	glDisable(GL_DEPTH_TEST);
	glUseProgram(gui_shader);
	glBindVertexArray(gui.background.vao);
	glBindTexture(GL_TEXTURE_2D, gui.background.texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//then buttons;
	for(int i=0; i<gui.button_num; ++i)
	{
		button b = gui.buttons[i];
		glBindVertexArray(b.vao);
		glBindTexture(GL_TEXTURE_2D, b.texture[b.state]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

static void setup_gui_texture()
{
	unsigned int width, height;
	unsigned short int bits;
	unsigned char *bgr;
	GLenum format;

	glGenTextures(1, &gui.background.texture);
	glBindTexture(GL_TEXTURE_2D, gui.background.texture);

	bgr = load_bmp(gui.background.filename, &width, &height, &bits);
	format = (bits == 24? GL_BGR: GL_BGRA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bgr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	for(int i=0; i<2; ++i)
	{
		for(int j=0; j<gui.button_num; ++j)
		{
			glGenTextures(1, &gui.buttons[j].texture[i]);
			glBindTexture(GL_TEXTURE_2D, gui.buttons[j].texture[i]);

			bgr = load_bmp(gui.buttons[j].filename[i], &width, &height, &bits);
			format = (bits == 24? GL_BGR: GL_BGRA);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bgr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	delete [] bgr;
}

static void setup_gui_render_obj()
{
	glGenVertexArrays(1, &gui.background.vao);
	glGenBuffers(2, gui.background.vbo);

	glBindVertexArray(gui.background.vao);

	float square_pos[] = {0.5, -1.0, 0.1,
						1.0, -1.0, 0.1,
						0.5, 1.0, 0.1,
						1.0, -1.0, 0.1,
						0.5, 1.0, 0.1,
						1.0, 1.0, 0.1};
	float square_uv[] = {0.0, 0.0,
						1.0, 0.0,
						0.0, 1.0,
						1.0, 0.0,
						0.0, 1.0,
						1.0, 1.0};

	glBindBuffer(GL_ARRAY_BUFFER, gui.background.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*18, square_pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, gui.background.vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, square_uv, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	for(int i=0; i<gui.button_num; ++i)
	{
		glGenVertexArrays(1, &gui.buttons[i].vao);
		glGenBuffers(2, gui.buttons[i].vbo);

		glBindVertexArray(gui.buttons[i].vao);

		float l, r, u, d;
		l = gui.buttons[i].obj_border.left/400.0f - 1.0f;
		r = gui.buttons[i].obj_border.right/400.0f - 1.0f;
		u = gui.buttons[i].obj_border.high/300.0f - 1.0f;
		d = gui.buttons[i].obj_border.low/300.0f - 1.0f;

		float button_square_pos[] = {l, d, 0.0,
									r, d, 0.0,
									l, u, 0.0,
									r, d, 0.0,
									l, u, 0.0,
									r, u, 0.0};


		glBindBuffer(GL_ARRAY_BUFFER, gui.buttons[i].vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*18, button_square_pos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, gui.buttons[i].vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, square_uv, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
	}
}

static void mouse_callback(GLFWwindow* window, int click_button, int action, int mods)
{
	button b;
	double xpos, ypos;
	int buttonID;
	glfwGetCursorPos(window, &xpos, &ypos);
	ypos = 960 - ypos;

	if(click_button==GLFW_MOUSE_BUTTON_1)
	{
		if(action==GLFW_PRESS)
			mouse_state = GLFW_PRESS;
		else if(action==GLFW_RELEASE && mouse_state==GLFW_PRESS)
		{
			mouse_state = GLFW_RELEASE;

			buttonID = gui.check_button(xpos/1.6, ypos/1.6);
			printf("at %f %f\n", xpos, ypos);
			if(buttonID!=-1)
			{
				b = gui.buttons[buttonID];
				printf("at %f, %f, click %d button, its state is %d!\n", xpos, ypos, buttonID, b.state);

				//check if the button action dosen't need to change shader
				if(buttonID < 13 || buttonID > 15)
				{

					std::string s = filenames[0];
					if(buttonID<3)
					{
						objects.clear();
						indicesCount.clear();
						s.append(filenames[1]);
						switch(buttonID)
						{
						case 0:
							s += "bunny.obj";
							show_obj = add_obj(s.c_str());
							now_mesh = 0;
							break;
						case 1:
							s += "body.obj";
							show_obj = add_obj(s.c_str());
							now_mesh = 1;
							break;
						case 2:
							s += "trex.obj";
							show_obj = add_obj(s.c_str());
							now_mesh = 2;
							break;
						}

						gui.clear_button_state();
					}
					else if(buttonID < 13)
					{
						objects.clear();
						indicesCount.clear();
						switch(now_algor)
						{
						case 16:
							s += filenames[2];
							break;
						case 17:
							s += filenames[3];
							break;
						}

						switch(now_mesh)
						{
						case 0:
							s += "bunny";
							break;
						case 1:
							s += "body";
							break;
						case 2:
							s += "trex";
							break;
						}

						switch(buttonID)
						{
						case 3:
							s += "10";
							break;
						case 4:
							s += "20";
							break;
						case 5:
							s += "40";
							break;
						case 6:
							s += "80";
							break;
						case 7:
							s += "100";
							break;
						case 8:
							s += "200";
							break;
						case 9:
							s += "400";
							break;
						case 10:
							s += "800";
							break;
						case 11:
							s += "1000";
							break;
						case 12:
							s += "10000";
							break;
						}

						now_times = buttonID;
						s += ".obj";
						printf("%s\n", s.c_str());
						show_obj = add_obj(s.c_str());
					}
					else
					{
						gui.clear_button_state(3);
						now_algor = buttonID;
					}

				}
				else
				{
					//change shader
					if(buttonID==13 && gui.buttons[13].state==0)
						program1 = shaders[0];
					else if(buttonID==14 && gui.buttons[14].state==0)
						program1 = shaders[1];
					else if(buttonID==15 && gui.buttons[15].state==0)
						program1 = shaders[2];

					printf("now program: %d\n", program1);
				}

				gui.clear_button_state(buttonID);
				gui.buttons[buttonID].state = 1;
			}
		}
	}
}

static void rotate()
{
	glm::mat4 x_model(1.0f), y_model(1.0f), z_model(1.0f);

	if(x_dir!=0)
	{
		x_deg = 0.1*x_dir;
	}
	else
		x_deg = 0;
	if(y_dir!=0)
	{
		y_deg = 0.1*y_dir;
	}
	else
		y_deg = 0;
	if(z_dir!=0)
	{
		z_deg = 0.1*z_dir;
	}
	else
		z_deg = 0;

	if(x_dir!=0 || y_deg!=0 || z_deg!=0)
	{
		x_model = glm::rotate(x_model, x_deg, glm::vec3(1,0,0));
		y_model = glm::rotate(y_model, y_deg, glm::vec3(0,1,0));
		z_model = glm::rotate(z_model, z_deg, glm::vec3(0,0,1));

		camera_location = camera_location * x_model * y_model * z_model;
		up_direction = up_direction * x_model * y_model * z_model;
	}
}

static void zoom()
{
	if(zoom_dir != 0)
	{
		glm::vec3 dir_to_center = glm::vec3(-camera_location.x, -camera_location.y, -camera_location.z);

		double z = 0.01 * zoom_dir;
		dir_to_center *= z;

		camera_location.x -= dir_to_center.x;
		camera_location.y -= dir_to_center.y;
		camera_location.z -= dir_to_center.z;
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if(action == GLFW_PRESS)
	{
		switch(key)
		{
		case GLFW_KEY_W:
			x_dir = 1;
			break;
		case GLFW_KEY_S:
			x_dir = -1;
			break;
		case GLFW_KEY_A:
			y_dir = -1;
			break;
		case GLFW_KEY_D:
			y_dir = 1;
			break;
		case GLFW_KEY_LEFT:
			z_dir = 1;
			break;
		case GLFW_KEY_RIGHT:
			z_dir = -1;
			break;
		case GLFW_KEY_MINUS:
			zoom_dir = 1;
			break;
		case GLFW_KEY_EQUAL:
			zoom_dir = -1;
			break;
		}
	}
	else if(action == GLFW_RELEASE)
	{
		switch(key)
		{
		case GLFW_KEY_W:
			x_dir = 0;
			break;
		case GLFW_KEY_S:
			x_dir = 0;
			break;
		case GLFW_KEY_A:
			y_dir = 0;
			break;
		case GLFW_KEY_D:
			y_dir = 0;
			break;
		case GLFW_KEY_LEFT:
			z_dir = 0;
			break;
		case GLFW_KEY_RIGHT:
			z_dir = 0;
			break;
		case GLFW_KEY_MINUS:
			zoom_dir = 0;
			break;
		case GLFW_KEY_EQUAL:
			zoom_dir = 0;
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	// OpenGL 3.3, Mac OS X is reported to have some problem. However I don't have Mac to test
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// For Mac OS X
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(1280, 960, "Mesh Smoothing Demo", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	// This line MUST put below glfwMakeContextCurrent
	glewExperimental = GL_TRUE;
	glewInit();

	// Enable vsync
	glfwSwapInterval(1);

	// Setup input callback
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);

	setup_gui_texture();
	setup_gui_render_obj();

	filenames[0] = "smoothing";
	filenames[1] = "/ori/";
	filenames[2] = "/lap/";
	filenames[3] = "/tau/";

	// load shader program
	shaders[0] = setup_shader(readfile("shaders/vs.txt").c_str(), readfile("shaders/fs.txt").c_str());
	gui_shader = setup_shader(readfile("shaders/guivs.txt").c_str(), readfile("shaders/guifs.txt").c_str());
	shaders[1] = setup_shader(readfile("shaders/nvs.txt").c_str(), readfile("shaders/nfs.txt").c_str());
	shaders[2] = setup_shader(readfile("shaders/fvs.txt").c_str(), readfile("shaders/ffs.txt").c_str());
	printf("we have:%d, %d, %d\n", shaders[0], shaders[1], shaders[2]);
	program1 = shaders[0];

	show_obj = add_obj("smoothing/ori/bunny.obj");
	glm::vec3 light_pos(0.0f, 0.0f, 0.0f);
	camera_location = glm::vec4(0.0f, 0.0f, 2.0f, 1.0f);
	up_direction = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glm::vec3 camera_location_xyz = glm::vec3(camera_location.x, camera_location.y, camera_location.z);
	glm::vec3 up_direction_xyz = glm::vec3(up_direction.x, up_direction.y, up_direction.z);
	setUniformMat4(shaders[0], "p", glm::perspective(glm::radians(45.0f), 640.0f/480, 0.1f, 100.f) *glm::mat4(1.0f));
	setUniformMat4(shaders[0], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), glm::vec3(0, 1, 0))*glm::mat4(1.0f));

	setUniformMat4(shaders[1], "p", glm::perspective(glm::radians(45.0f), 640.0f/480, 0.1f, 100.f) *glm::mat4(1.0f));
	setUniformMat4(shaders[1], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), glm::vec3(0, 1, 0))*glm::mat4(1.0f));

	setUniformMat4(shaders[2], "p", glm::perspective(glm::radians(45.0f), 640.0f/480, 0.1f, 100.f) *glm::mat4(1.0f));
	setUniformMat4(shaders[2], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), glm::vec3(0, 1, 0))*glm::mat4(1.0f));



	glm::mat4 rot;
	glm::mat4 rev;

	float last, start;
	last = start = glfwGetTime();
	float fps = 0;
	while (!glfwWindowShouldClose(window))
	{
		rotate();
		zoom();

		camera_location_xyz = glm::vec3(camera_location.x, camera_location.y, camera_location.z);
		up_direction_xyz = glm::vec3(up_direction.x, up_direction.y, up_direction.z);
		setUniformMat4(shaders[0], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), up_direction_xyz)*glm::mat4(1.0f));
		setUniformMat4(shaders[1], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), up_direction_xyz)*glm::mat4(1.0f));
		setUniformMat4(shaders[2], "v", glm::lookAt(camera_location_xyz, glm::vec3(0.0f), up_direction_xyz)*glm::mat4(1.0f));

		render();

		glfwSwapBuffers(window);
		glfwPollEvents();
		fps++;
		if(glfwGetTime() - last > 1.0)
		{
			std::cout<<(double)fps/(glfwGetTime()-last)<<std::endl;
			fps = 0;
			last = glfwGetTime();
		}
	}

	releaseObjects();
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
