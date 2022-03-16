#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#include "Model.h"

// Window dimensions
const float toRadians = 3.14159265f / 180.f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;

Material shinyMaterial;
Material dullMaterial;

Model blackHawk;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];


GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

// Vertex Shader code
static const char* vShader = "Shaders/shader.vert.txt";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag.txt";

void calcAverageNormal(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
							unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
	//	x	  y		z			u	v			nx	 ny	  nz
		-1.f, -1.f, -0.6f,		0.f, 0.f,		0.f, 0.f, 0.f,
		0.f, -1.f, 1.f,			0.5f, 0.f,		0.f, 0.f, 0.f,
		1.f, -1.f, -0.6f,		1.f, 0.f,		0.f, 0.f, 0.f,
		0.f, 1.f, 0.f,			0.5f, 1.f,		0.f, 0.f, 0.f
	};


	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.f, 0.f, -10.f,		0.f, 0.f,		0.f, -1.f, 0.f,
		10.f, 0.f, -10.f,		10.f, 0.f,		0.f, -1.f, 0.f,
		-10.f, 0.f, 10.f,		0.f, 10.f,		0.f, -1.f, 0.f,
		10.f, 0.f, 10.f,		10.f, 10.f,		0.f, -1.f, 0.f
	};

	calcAverageNormal(indices, 12, vertices, 32, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main() 
{
	mainWindow = Window(1366, 768);
	mainWindow.Intitialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), -90.f, 0.f, 5.f, 0.25f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	mainLight = DirectionalLight(1.f, 1.f, 1.f, 
								0.5f, 0.5f,
								0.f, 0.f, -1.f);

	unsigned int pointLightCount = 0;

	pointLights[0] = PointLight(0.f, 0.f, 1.f,
								0.f, 0.1f,
								0.f, 0.f, 0.f,
								0.3f, 0.2f, 0.1f);
	pointLightCount++;

	pointLights[1] = PointLight(0.f, 1.f, 0.f,
								0.0f, 0.1f,
								-4.f, 2.f, 0.f,
								0.3f, 0.1f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;

	spotLights[0] = SpotLight(1.f, 1.f, 1.f,
							0.f, 2.f,
							0.f, 0.f, 0.f,
							0.f, -1.f, 0.f,
							1.f, 0.f, 0.0f,
							20.f);

	spotLightCount++;

	spotLights[1] = SpotLight(1.f, 1.f, 1.f,
							0.1f, 0.1f,
							0.f, -1.5f, 0.f,
							-100.f, -1.f, 0.f,
							1.f, 0.0f, 0.0f,
							20.f);

	spotLightCount++;

	shinyMaterial = Material(1.f, 256);
	dullMaterial = Material(0.3f, 4);

	blackHawk = Model();
	blackHawk.LoadModel("Models/Black Hawk uh-60.obj");

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
				uniformSpecularIntensity = 0 , uniformShininess = 0;

	glm::mat4 projection = glm::perspective(45.f, mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.f);

	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle user input events
		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear window
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight -= -0.3f;
		//spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionLight(&mainLight);
		shaderList[0].setPointLight(pointLights, pointLightCount);
		shaderList[0].setSpotLight(spotLights, spotLightCount);

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		glm::mat4 model(1.f);
		
		model = glm::translate(model, glm::vec3(0.f, 0.f, -2.5f));
		//model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		brickTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, 4.f, -2.5f));
		//model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[1]->RenderMesh();

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, -2.f, 0.f));
		//model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, 10.f, 0.f));
		//model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		blackHawk.RenderModel();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}