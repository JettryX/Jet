#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <math.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

void Render2(float r, float x, float y, float z, int numpoly, float texCenter[], float texRad);
void RenderBonus(float r, float x, float y, float z, int numpoly, float texCenter[], float texRad, float number);
void endingUP(float x1, float x2, float z, float texA[], float texB[], float texC[]);

float pi = 3.14;
int texX;
int texY;
bool textureMode = true;
bool lightMode = !true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}




//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("mytexture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texX=texW, texY=texH, &texCharArray);

	
	GLuint texId;
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}





void Render(OpenGL *ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	//glRotatef(u++, 0, 0, 1);
	// Вверх
 	int z = 0;
	glColor3f(1, 1, 1);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(569.0f / texX, 1 - 224.0f / texY);
	glVertex3i(6, 5, z);
	glTexCoord2f(729.0f / texX, 1 - 224.0f / texY);
	glVertex3i(15, 5, z);
	glTexCoord2f(618.0f / texX, 1 - 176.0f / texY);
	glVertex3i(9, 8, z);
	glTexCoord2f(673.0f / texX, 1 - 190.0f / texY);
	glVertex3i(12, 7, z);
	glTexCoord2f(581.0f / texX, 1 - 88.0f / texY);
	glVertex3i(7, 13, z);
	glTexCoord2f(722.0f / texX, 1 - 88.0f / texY);
	glVertex3i(15, 13, z);
	glEnd();
	glBegin(GL_TRIANGLES);
	glTexCoord2f(674.0f / texX, 1 - 190.0f / texY);
	glVertex3i(12, 7, z);
	glTexCoord2f(729.0f / texX, 1 - 222.0 / texY);
	glVertex3i(15, 5, z);
	glTexCoord2f(779.0f / texX, 1 - 136.0 / texY);
	glVertex3i(18, 10, z);
	glEnd();
	// Низ
	glColor3f(1, 1, 1);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(106.0f/texX , 1 -220.0f/texY);
	glVertex3i(6, 5, z + 3);
	glTexCoord2f(266.0f/texX, 1 - 220.0f/texY);
	glVertex3i(15, 5, z + 3);
	glTexCoord2f(153.0f/texX, 1 - 173.0f/texY);
	glVertex3i(9, 8, z + 3);
	glTexCoord2f(208.0f/texX, 1 - 186.0f/texY);
	glVertex3i(12,7, z + 3);
	glTexCoord2f(119.0f/texX, 1 - 85.0f/texY);
	glVertex3i(7, 13, z + 3);
	glTexCoord2f(256.0f/texX, 1 -85.0f/texY);
	glVertex3i(15, 13, z + 3);
	glEnd();
	glColor3f(1, 1, 1);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(210.0f/texX, 1-186.0f/texY);
	glVertex3i(12, 7, z + 3);
	glTexCoord2f(265.0f/texX, 1-218.0f/texY );
	glVertex3i(15, 5, z + 3);
	glTexCoord2f(315.0f/texX, 1 - 131.0f/texY);
	glVertex3i(18, 10, z + 3);
	glEnd();
	// Закрылки
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	//////////////////////////////
	glTexCoord2f(364.0f / texX, 1 - 275.0f / texY);
	glVertex3i(19, 0, z);
	glTexCoord2f(340.0f / texX, 1 - 275.0f / texY);
	glVertex3i(19, 0, z + 3);
	glTexCoord2f(340.0f / texX, 1 - 205.0f / texY);
	glVertex3i(15, 5, z + 3);
	glTexCoord2f(364.0f / texX, 1 - 205.0f / texY);
	glVertex3i(15, 5, z);
	////////////////////////////
	glTexCoord2f(404.0f / texX, 1 - 200.0f / texY);
	glVertex3i(15, 5, z);
	glTexCoord2f(380.0f / texX, 1 - 200.0f / texY);
	glVertex3i(15, 5, z + 3);
	glTexCoord2f(380.0f / texX, 1 - 130.0f / texY);
	glVertex3i(18, 10, z + 3);
	glTexCoord2f(404.0f / texX, 1 - 130.0f / texY);
	glVertex3i(18, 10, z);
	//////////////////////////////
	glTexCoord2f(364.0f / texX, 1 - 130.0f / texY);
	glVertex3i(18, 10, z);
	glTexCoord2f(339.0f / texX, 1 - 130.0f / texY);
	glVertex3i(18, 10, z + 3);
	glTexCoord2f(339.0f / texX, 1 - 200.0f / texY);
	glVertex3i(12, 7, z + 3);
	glTexCoord2f(364.0f / texX, 1 - 200.0f / texY);
	glVertex3i(12, 7, z);
	//////////////////////////////
	glTexCoord2f(364.0f / texX, 1 - 125.0f / texY);
	glVertex3i(12, 7, z);
	glTexCoord2f(340.0f / texX, 1 - 125.0f / texY);
	glVertex3i(12, 7, z + 3);
	glTexCoord2f(340.0f / texX, 1 - 27.0f / texY);
	glVertex3i(15, 13, z + 3);
	glTexCoord2f(364.0f / texX, 1 - 27.0f / texY);
	glVertex3i(15, 13, z);
	//////////////////////////////
	/////////////////////////////
	glTexCoord2f(45.0f / texX, 1 - 27.0f / texY);
	glVertex3i(7, 13, z);
	glTexCoord2f(70.0f / texX, 1 - 27.0f / texY);
	glVertex3i(7, 13, z + 3);
	glTexCoord2f(70.0f / texX, 1 - 125.0f / texY);
	glVertex3i(9, 8, z + 3);
	glTexCoord2f(45.0f / texX, 1 - 125.0f / texY);
	glVertex3i(9, 8, z);
	////////////////////////////
	glTexCoord2f(46.0f / texX, 1 - 128.0f / texY);
	glVertex3i(9, 8, z);
	glTexCoord2f(20.0f / texX, 1 - 128.0f / texY);
	glVertex3i(9, 8, z + 3);
	glTexCoord2f(20.0f / texX, 1 - 178.0f / texY);
	glVertex3i(6, 5, z + 3);
	glTexCoord2f(46.0f / texX, 1 - 178.0f / texY);
	glVertex3i(6, 5, z);
	///////////////////////////
	glTexCoord2f(46.0f / texX, 1 - 178.0f / texY);
	glVertex3i(6, 5, z);
	glTexCoord2f(20.0f / texX, 1 - 178.0f / texY);
	glVertex3i(6, 5, z + 3);
	glTexCoord2f(20.0f / texX, 1 - 262.0f / texY);
	glVertex3i(0, 0, z + 3);
	glTexCoord2f(46.0f / texX, 1 - 262.0f / texY);
	glVertex3i(0, 0, z);
	glEnd();
	////////////////////////////////////////////
	glColor3f(1, 1, 1);
	float texCA[] = { 653.0f, 88.0f };
	Render2(4 , 11, 13, z, 48, texCA, 72.0f);
	//////////////////
	glColor3f(1, 1, 1);
	float texCB[] = { 189.0f, 84.0f };
	Render2(4, 11, 13, z + 3, 48, texCB, 72.0f);
	//////////////////
	glColor3f(1, 1, 1);
	glBegin(GL_QUAD_STRIP);
	double r = 4;
	float nextpoly = 180.0f / 48;
	for (int i = 0; i<48+1; i++)
	{
		glTexCoord2f((385.0f + nextpoly*i) / texX, 1- 75.0f / texY);
		glVertex3f(r*cos(i*pi / 48) + 11, r*sin(i*pi / 48) + 12.95, z);
		glTexCoord2f((385.0f + nextpoly*i) / texX, 1- 25.0f / texY);
		glVertex3f(r*cos(i*pi / 48) + 11, r*sin(i*pi / 48) + 12.95, z + 3);
	}
	glEnd();
	////////////////////////////////////////////////
	glColor3f(1, 1, 1);
	float texCAbon[] = { 181.0f, 220.0f };
	float texCBbon[] = { 644.0f, 224.0f};
	RenderBonus(17, 9.5, -14, z, 48, texCBbon, 274.0f, 522.0f+4);
	glColor3f(1, 1, 1);
	RenderBonus(17, 9.5, -14, z + 3, 48, texCAbon, 274.0f, 522.0f);
	///////////////////////////////////////////
	glColor3f(1, 1, 1);
	nextpoly = (340 - 5) / (53-52/3);
	//////////////////////////////////////
	glBegin(GL_QUAD_STRIP);
	for (int i = (52 / 3) - 2, j=0; i<(53 - 52 / 3) + 1; i++, j++)
	{
		glTexCoord2f((340.0f - nextpoly*j) / texX, 1 - 366.0f / texY);
		glVertex3f(17 * cos(i*(pi) / 52) + 9.5, 17 * sin(i*(pi) / 52) - 14, z);
		glTexCoord2f((340.0f - nextpoly*j) / texX, 1 - 305.0f / texY);
		glVertex3f(17 * cos(i*(pi) / 52) + 9.5, 17 * sin(i*(pi) / 52) - 14, z + 3);
	}
	glEnd();
	////////////
	float texA[] = { 25.0f/texX, 1- 291.0f/texY };
	float texB[] = { 105.0f/texX, 1- 221.0f/texY};
	float texC[] = { 180.0f/texX, 1- 221.0f/texY };
	float texD[] = { 323.0f / texX, 1 - 291.0f / texY };
	float texE[] = { 264.0f / texX, 1 - 221.0f / texY };
	float texA2[] = { 489.0f / texX, 1 - 294.0f / texY };
	float texB2[] = { 569.0f / texX, 1 - 224.0f / texY };
	float texC2[] = { 644.0f / texX, 1 - 224.0f / texY };
	float texD2[] = { 790.0f / texX, 1 - 294.0f / texY };
	float texE2[] = { 730.0f / texX, 1 - 224.0f / texY };
	glColor3f(1, 1, 1);
	endingUP(0, 6, z+3, texA, texB, texC);
	endingUP(19, 15, z+3, texD, texE, texC);
	glColor3f(1, 1, 1);
	endingUP(0, 6, z , texA2, texB2, texC2);
	endingUP(19, 15, z , texD2, texE2, texC2);
}

void Render2(float r, float x, float y, float z, int numpoly, float texCenter[], float texRad)
{
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(texCenter[0]/texX, 1 - texCenter[1]/texY);
	glVertex3i(x, y, z);
	for (int i = 0; i<(numpoly + 1); i++)
	{	
		glTexCoord2f((texRad/texX * cos(-i*pi / numpoly)+texCenter[0]/texX), 1 - (texRad/texY * sin(-i*pi / numpoly)+texCenter[1]/texY));
		glVertex3f(r*cos(i*pi / numpoly) + x, r*sin(i*pi / numpoly) + y, z);
	}
	glEnd();
}
void RenderBonus(float r, float x, float y, float z, int numpoly, float texCenter[], float texRad, float number)
{
	//glRotatef(u++, 0, 0, 1);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(texCenter[0] / texX, 1 - texCenter[1] / texY);  // 181.0f , 220f
	glVertex3i(10, 5, z);
	for (int i = (numpoly / 3); i<(numpoly + 1) - numpoly / 3; i++)
	{				//274									172						274									522
		glTexCoord2f((texRad / texX * cos(-i*pi / numpoly) + (texCenter[0]-8) / texX), 1 - (texRad / texY * sin(-i*pi / numpoly) + number / texY));
		glVertex3f(r*cos(i*(pi) / numpoly) + x, r*sin(i*(pi) / numpoly) + y, z);
	}
	//glVertex3i(15,13,0);
	glEnd();
}

void endingUP(float x1, float x2, float z, float texA[], float texB[], float texC[])
{
	glBegin(GL_TRIANGLES);
	glTexCoord2fv(texA);
	glVertex3f(x1, 0, z);
	glTexCoord2fv(texC);
	glVertex3f(10, 5, z);
	glTexCoord2fv(texB);
	glVertex3f(x2, 5, z);
	glEnd();
} 
	
	
	//конец рисования квадратика станкина
    
	
	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	/* char c[250];  //максимальная длина сообщения
	sprintf_s(c, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		"G - перемещение в горизонтальной плоскости,\nG+ЛКМ+перемещение по вертикальной линии\n"
		"R - установить камеру и свет в начальное положение\n"
		"F - переместить свет в точку камеры", textureMode, lightMode);
	ogl->message = std::string(c);




}  //конец тела функции */

