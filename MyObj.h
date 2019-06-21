#pragma once

#include <vgl.h>
#include <vec.h>

struct MyObjVertex
{
	vec4 position;
	vec4 color;
	vec3 normal;
};

struct MyTriangle
{
	int ind[3];
	vec3 normal;
};



class MyObj
{
public:
	int NumVertices;
	GLuint vao;
	GLuint buffer;
	bool bInitialized;
	MyObjVertex * Vertices;


	MyObj(void);
	~MyObj(void);
	void Draw(GLuint program);
	void SetPositionAndOtherAttributes(GLuint program);
	void Init(char * filename);
};

