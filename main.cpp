#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MySphere.h"
#include "MyObj.h"
#include "Targa.h"

#include <vec.h>
#include <mat.h>

MyCube cube;
MySphere sphere;
MyObj obj;

GLuint program;
GLuint phong_prog;
GLuint sphere_prog;
GLuint dif_prog;

mat4 g_Mat = mat4(1.0f);

char src_sphere[256]; // texture file의 주소
char src_diffuse[256]; // texture file의 주소
FILE* fp = NULL;

float time = 0;
float g_aspect = 1;

bool bRotate = false;
int drawObj = 0;

float r = 6.0; // 반지름
float theta1 = 1.5*3.141592;
float theta2 = 0;
float eye_x = cos(theta1);
float eye_y = theta2;
float eye_z = sin(theta1);
vec3 pos = normalize(vec3(eye_x, eye_y, eye_z))*-r;

vec2 p1, p2;

float Fresnel = 11.0;
bool shader = false;
bool mouse_btn;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V = mat4(1.0f);
	
	up = normalize(up);
	vec3 n = normalize(at - eye);
	float a = dot(up, n);
	vec3 v = normalize(up-a*n);
	vec3 w = cross(n, v);

	V[0] = vec4(w, dot(-w, eye));
	V[1] = vec4(v, dot(-v, eye));
	V[2] = vec4(-n, dot(n, eye));
	
	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	vec3 center = vec3((l+r)/2, (b+t)/2, -(zNear)/2);
	mat4 T = Translate(-center);
	mat4 S = Scale(2/(r-l), 2/(t-b), -1/(-zNear+zFar));
	mat4 V = S*T;

	return V;
}

mat4 myPerspective(float angle, float aspect, float zNear, float zFar)
{
	float rad = angle*3.141592/180.0f;
	mat4 V(1.0f);
	float h = 2*zFar*tan(rad/2);
	float w = aspect*h;
	mat4 S = Scale(2/w, 2/h, 1/zFar);

	float c = -zNear/zFar;
	
	mat4 Mpt(1.0f);
	Mpt[2] = vec4(0, 0, 1/(c+1), -c/(c+1));
	Mpt[3] = vec4(0, 0, -1, 0);
	

	V = Mpt*S;

	return V;
}

void myInitTexture()
{
	GLuint myTex[2];
	glGenTextures(2, myTex);
	for (int i=0; i<2; i++)
	{
		STGA img;
		if (i==0)
		{
			img.loadTGA(src_sphere);
			glActiveTexture(GL_TEXTURE0); // 0번을 고르고
		}
		else if (i==1)
		{
			img.loadTGA(src_diffuse);
			glActiveTexture(GL_TEXTURE1);
		}
		glBindTexture(GL_TEXTURE_2D, myTex[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
		img.destroy();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}	
}

void myInit()
{
	cube.Init();
	sphere.Init(40,40);
	obj.Init("bunny.obj");

	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	
	phong_prog = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(phong_prog);

	sphere_prog = InitShader("vSphere.glsl", "fSphere.glsl");
	glUseProgram(sphere_prog);
	
	dif_prog = InitShader("vDiffuse.glsl", "fDiffuse.glsl");
	glUseProgram(dif_prog);

	myInitTexture();
		
}

void DrawAxis()
{
	glUseProgram(program);
	GLuint uMat = glGetUniformLocation(program, "uMat");
	GLuint uColor = glGetUniformLocation(program, "uColor");

	mat4 x_a= Translate(0.5,0,0)*Scale(1,0.05,0.05);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*x_a);
	glUniform4f(uColor, 1, 0, 0, 1);
	cube.Draw(program);

	mat4 y_a= Translate(0,0.5,0)*Scale(0.05,1,0.05);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*y_a);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	mat4 z_a= Translate(0,0,0.5)*Scale(0.05,0.05,1);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*z_a);
	glUniform4f(uColor, 0, 0, 1, 1);
	cube.Draw(program);
}

void display()
{
	glClearColor(0,0,0,1);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	vec3 ePos = pos;
	mat4 ModelMat = RotateY(time);
	mat4 ViewMat = myLookAt(ePos, vec3(0,0,0), vec3(0,1,0));
	mat4 ProjMat = myPerspective(45, g_aspect, 0.01, 100.0f);

		
	g_Mat = ProjMat*ViewMat*ModelMat;
	
	//DrawAxis();



	// 1. Define Light Properties
	// 
	vec4 lPos = vec4(2, 2, 0, 1);			 
	vec4 lAmb = vec4(0.5, 0.5, 0.5, 1);		
	vec4 lDif = vec4(1, 1, 1, 1);
	vec4 lSpc = lDif; 

	// 2. Define Material Properties
	//
	vec4 mAmb = vec4(0.3, 0.3, 0.3, 1);		
	vec4 mDif = vec4(0.8, 0.8, 0.8, 1);		
	vec4 mSpc = vec4(0.3, 0.3, 0.3, 1); 
	float mShiny = 50; //1~100;

	// I = lAmb*mAmb + lDif*mDif*(N·L) + lSpc*mSpc*(V·R)^n; 
	vec4 amb = lAmb*mAmb;					
	vec4 dif = lDif*mDif;					
	vec4 spc = lSpc*mSpc; 





	//////////
	glUseProgram(sphere_prog);
	
	sphere.Draw(sphere_prog);
	GLuint uMat = glGetUniformLocation(sphere_prog, "uMat");
	GLuint uTexture = glGetUniformLocation(sphere_prog, "uTexture");

	glUniformMatrix4fv(uMat, 1, true, ProjMat*ViewMat*Scale(10,10,10));
	GLuint uModelMat = glGetUniformLocation(sphere_prog, "uModelMat");
	GLuint uViewMat = glGetUniformLocation(sphere_prog, "uViewMat");
	GLuint uLPos = glGetUniformLocation(sphere_prog, "uLPos"); 
	GLuint uEPos = glGetUniformLocation(sphere_prog, "uEPos"); 
	
	glUniformMatrix4fv(uModelMat, 1, true, ModelMat); 
	glUniformMatrix4fv(uViewMat, 1, true, ViewMat); 
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]); 
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);
	glUniform1i(uTexture, 0);
	


	//////////
	glUseProgram(phong_prog);

	// 3. Send Uniform Variables to the shader
	//
	uModelMat = glGetUniformLocation(phong_prog, "uModelMat");
	uViewMat = glGetUniformLocation(phong_prog, "uViewMat");
	GLuint uProjMat = glGetUniformLocation(phong_prog, "uProjMat"); 
	uLPos = glGetUniformLocation(phong_prog, "uLPos"); 
	GLuint uAmb = glGetUniformLocation(phong_prog, "uAmb"); 
	GLuint uDif = glGetUniformLocation(phong_prog, "uDif"); 
	GLuint uSpc = glGetUniformLocation(phong_prog, "uSpc"); 
	GLuint uShininess = glGetUniformLocation(phong_prog, "uShininess"); 
	uEPos = glGetUniformLocation(phong_prog, "uEPos"); 
	uTexture = glGetUniformLocation(phong_prog, "uTexture");
	GLuint uDiffuse = glGetUniformLocation(phong_prog, "uDiffuse");
	GLuint uFresnel = glGetUniformLocation(phong_prog, "uFresnel");

	glUniformMatrix4fv(uModelMat, 1, true, ModelMat); 
	glUniformMatrix4fv(uViewMat, 1, true, ViewMat); 
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat); 
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]); 
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]); 
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]); 
	glUniform4f(uSpc, spc[0], spc[1], spc[2], spc[3]); 
	glUniform1f(uShininess, mShiny); 
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);
	glUniform1i(uTexture, 0);
	glUniform1i(uDiffuse, 1);
	glUniform1f(uFresnel, Fresnel);

	if (shader == false)
	{
		if (drawObj == 1)
			sphere.Draw(phong_prog);
		else if (drawObj == 2)
			obj.Draw(phong_prog);
	}



	//////////
	glUseProgram(dif_prog);

	// 3. Send Uniform Variables to the shader
	//
	uModelMat = glGetUniformLocation(dif_prog, "uModelMat");
	uViewMat = glGetUniformLocation(dif_prog, "uViewMat");
	uProjMat = glGetUniformLocation(dif_prog, "uProjMat"); 
	uLPos = glGetUniformLocation(dif_prog, "uLPos"); 
	uAmb = glGetUniformLocation(dif_prog, "uAmb"); 
	uDif = glGetUniformLocation(dif_prog, "uDif"); 
	uSpc = glGetUniformLocation(dif_prog, "uSpc"); 
	uShininess = glGetUniformLocation(dif_prog, "uShininess"); 
	uEPos = glGetUniformLocation(dif_prog, "uEPos"); 
	uTexture = glGetUniformLocation(dif_prog, "uTexture");
	uDiffuse = glGetUniformLocation(dif_prog, "uDiffuse");
	uFresnel = glGetUniformLocation(dif_prog, "uFresnel");

	glUniformMatrix4fv(uModelMat, 1, true, ModelMat); 
	glUniformMatrix4fv(uViewMat, 1, true, ViewMat); 
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat); 
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]); 
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]); 
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]); 
	glUniform4f(uSpc, spc[0], spc[1], spc[2], spc[3]); 
	glUniform1f(uShininess, mShiny); 
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);
	glUniform1i(uTexture, 0);
	glUniform1i(uDiffuse, 1);
	glUniform1f(uFresnel, Fresnel);

	if (shader == true)
	{
		if (drawObj == 1)
			sphere.Draw(dif_prog);
		else if (drawObj == 2)
			obj.Draw(dif_prog);
	}



	Sleep(16); // for vSync
	glutSwapBuffers();
}

void idle()
{
	if (bRotate == true)
		time += 1;

	glutPostRedisplay();
}

void keyboard(unsigned char ch, int x, int y)
{
	switch (ch)
	{
		case 'q':
		case 'Q':
				drawObj += 1;
				if (drawObj == 3) drawObj = 0;
				printf("drawObj: %d\n", drawObj);
				break;
		case '1': 
			Fresnel -= 0.5;
			if (Fresnel < 0.5) Fresnel = 0.5;			
			printf("Fresnel Power: %.2f\n", Fresnel);
			break;
		case '2':
			Fresnel += 0.5;
			if (Fresnel > 11.0) Fresnel = 11.0;
			printf("Fresnel Power: %.2f\n", Fresnel);
			break;
		case '3':
				if (!shader)
					shader = true;
				else
					shader = false;
				printf("diffuse light map: %d\n", shader);
				break;
		case VK_SPACE:
				if (!bRotate)
					bRotate = true;
				else
					bRotate = false;
			break;
		default:
			break;
	}
}

void mouse(int btn,int state, int x, int y)
{
	//printf("btn:%d, state:%d, x:%d, y:%d\n", btn, state, x, y);
	
	if (btn == 0) // left btn
	{
		mouse_btn = false;
		if (state == 0) // pressed
			p1 = vec2(x,y);
	}
	else if (btn == 2) // right btn
	{
		mouse_btn = true;
		if (state == 0) // pressed
			p1 = vec2(x,y);
	}

	glutPostRedisplay();
}

void motion(int x, int y)
{
	p2 = vec2(x,y);
	vec2 delta = vec2(p2.x-p1.x, p2.y-p1.y); 
	p1 = p2;

	//printf("delta.x: %.2f, delta.y: %.2f\n", delta.x, delta.y);
	
	if (!mouse_btn) // left btn
	{
		theta1 -= delta.x/300;
		eye_x = cos(theta1);
		eye_z = sin(theta1);

		theta2 += delta.y/300;
		eye_y = theta2;

		pos = vec3(eye_x,eye_y,eye_z);
		pos = normalize(pos)*-r;
	}
	else // right btn
	{
		r -= delta.y/100;

		if (r < 3.0) r = 3.0;
		else if (r > 27.0) r = 27.0;

		pos = vec3(eye_x,eye_y,eye_z);
		pos = normalize(pos)*-r;
	}
	
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	g_aspect = w/float(h);
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	while(true)
	{
		printf("Input Image Group Name: ");
		scanf("%s", &src_sphere); // obj file 이름을 사용자로부터 입력받는다.
		strcpy(src_diffuse, src_sphere);
		strcat(src_sphere, "_spheremap.tga");
		strcat(src_diffuse, "_diffusemap.tga");
		

		fp = fopen(src_sphere, "r");
		if (!fp) // 파일이 없을경우
		{
			printf("File not Found!\n"); // 에러 메세지를 표시하고
			continue; // 다시 입력 받는다.
		}
		else break;
		fp = fopen(src_diffuse, "r");
		if (!fp) // 파일이 없을경우
		{
			printf("File not Found!\n"); // 에러 메세지를 표시하고
			continue; // 다시 입력 받는다.
		}
		else break;
	}


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(800,500);

	glutCreateWindow("Shader Test");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n",	glGetString(GL_VERSION),
									glGetString(GL_SHADING_LANGUAGE_VERSION));

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}


