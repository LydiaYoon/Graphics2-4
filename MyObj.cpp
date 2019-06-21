#include "MyObj.h"


MyObj::MyObj(void)
{
	NumVertices = 0;
	bInitialized = false;
	Vertices = NULL;
}


MyObj::~MyObj(void)
{
	if(Vertices != NULL)
		delete [] Vertices;
}


void MyObj::Draw(GLuint program)
{
	if(!bInitialized) return;			// check whether it is initiazed or not. 
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);
	
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

}

void MyObj::SetPositionAndOtherAttributes(GLuint program)
{
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(sizeof(vec4)));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MyObjVertex), BUFFER_OFFSET(sizeof(vec4)+sizeof(vec4)));
	
}

void MyObj::Init(char * filename)
{
	FILE * file = fopen(filename, "r");
	if(file == NULL)
	{
		printf("File not Found!\n");
		return;
	}

	int vNum = 0;
	int fNum = 0;
	while(feof(file) == false)
	{
		char buf[255];
		fgets(buf, 255, file);
		if(buf[0]=='v') vNum ++;
		if(buf[0]=='f') fNum ++;

	}

	printf("vNum=%d, fNum=%d\n",vNum, fNum);

	fclose(file);
	file = fopen(filename, "r");
	vec3 * pos = new vec3[vNum];
	MyTriangle * face = new MyTriangle[fNum];
	int vind = 0;
	int find = 0;
	while(feof(file) == false)
	{
		char buf[255];
		fgets(buf, 255, file);
		if(buf[0]=='v')
		{
			char ch;
			float x,y,z;
			sscanf(buf,"%c %f %f %f",
				&ch,&x,&y,&z);
			pos[vind] = vec3(x,y,z);
			vind++;
		}
		else if(buf[0]=='f')
		{
			char ch;
			int a,b,c;
			sscanf(buf, "%c %d %d %d",
				&ch, &a, &b, &c);
			face[find].ind[0] = a-1;
			face[find].ind[1] = b-1;
			face[find].ind[2] = c-1;
			find ++;
		}
	}
	fclose(file);

	vec3 p_min, p_max;
	p_min = pos[0];
	p_max = pos[0];
	for(int i=0; i<vNum; i++)
	{
		for(int j=0; j<3; j++)
			if(pos[i][j]<p_min[j]) p_min[j] = pos[i][j];
		for(int j=0; j<3; j++)
			if(pos[i][j]>p_max[j]) p_max[j] = pos[i][j];
	}
	vec3 center = (p_max+p_min)*0.5;
	float size = length(p_max-p_min);

	for(int i=0; i<vNum; i++)
	{
		pos[i] = pos[i] - center;
		pos[i] = pos[i]/size*3;
	}



	NumVertices = fNum*3;
	Vertices = new MyObjVertex[NumVertices];
	for(int i=0; i<fNum; i++)
	{
		vec3 a = pos[face[i].ind[1]] -
					pos[face[i].ind[0]];

		vec3 b = pos[face[i].ind[2]] -
					pos[face[i].ind[0]];
		vec3 n = cross(a,b);
		face[i].normal = normalize(n);
	}


	vec3 * nor = new vec3[vNum];
	for(int i=0; i<vNum; i++)
		nor[i] = vec3(0,0,0);

	for(int i=0; i<fNum; i++)
	{
		for(int j=0; j<3; j++)
			nor[face[i].ind[j]] += face[i].normal;
	}

	for(int i=0; i<vNum; i++)
		nor[i] = normalize(nor[i]);

	for(int i=0; i<fNum; i++)
		for(int j=0; j<3; j++)
		{
			Vertices[3*i+j].position = pos[face[i].ind[j]];
			Vertices[3*i+j].color = vec4(1,1,1,1);
			Vertices[3*i+j].normal = nor[face[i].ind[j]];
		}


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyObjVertex)*NumVertices, Vertices, GL_STATIC_DRAW);
	
	bInitialized = true;


	return;
}