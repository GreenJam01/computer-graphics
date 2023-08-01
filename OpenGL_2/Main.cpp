
#include <iostream>
#include <Windows.h>
#include <glad\glad.h>

#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Shader.h"
#include "../../../cg4/Figure.h"

using namespace glm;
//=============================================================================
 // ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
 //=============================================================================
 std::vector<model> models;
 glm::mat4 T; // матрица, в которой накапливаются все преобразования
 glm::vec3 S, P, u; // координаты точки наблюдения
 glm::mat4 lightM; // модельная матрица для источника света
 glm::mat4 lightM2;
 // точки, в которую направлен вектор наблюдения
 // вектора направления вверх
 float dist; // вспомогательная переменная - расстояние между S и P
 float fovy, aspect; // угол обзора и соотношение сторон окна наблюдения
 float fovy_work, aspect_work; // рабочие переменные для fovy и aspect
 float near_view, far_view; // расстояния до окна наблюдения и до горизонта
 float n, f; // рабочие переменные для near_view и far_view
 float l, r, t, b; // рабочие вспомогательные переменные
 double lastX, lastY; // последняя позиция курсора
 bool click = false;
 glm::vec3 ambient;
 glm::vec3 diffuse;
 glm::vec3 specular;
 // для значений координат левой, правой,
 // нижней и верхней координаты в СКН
 enum projType { Ortho, Frustum, Perspective } pType; // тип трехмерной проекции
 //=============================================================================
 void initWorkPars() { // инициализация рабочих параметров камеры
	 n = near_view;
	 f = far_view;
	 fovy_work = fovy;
	 aspect_work = aspect;
	 float Vy = 2 * near_view * glm::tan(fovy / 2);
	 float Vx = aspect * Vy;
	 l = -Vx / 2;
	 r = -l;
	  b = -Vy / 2;
	  t = -b;
	  dist = glm::length(P - S);
	  T = glm::lookAt(S, P, u);
	 
 }
 void readFromFile(const char* fileName) { // чтение сцены из файла fileName
// объявление и открытие файла
std::ifstream in;
in.open(fileName);
if (in.is_open()) {
// файл успешно открыт
models.clear(); // очищаем имеющийся список рисунков
// временные переменные для чтения из файла
glm::mat4 M = glm::mat4(1.f); // матрица для получения модельной матрицы
 glm::mat4 initM; // матрица для начального преобразования каждого рисунка
 std::vector<glm::mat4> transforms; // стек матриц преобразований
 std::vector<mesh> figure; // список ломаных очередного рисунка
 float thickness = 2; // толщина со значением по умолчанию 2
 // составляющие цвета модели со значениями по умолчанию
 ambient = glm::vec3(0, 0, 0);
 diffuse = glm::vec3(0, 0, 0);
 specular = glm::vec3(0, 0, 0);
 float shininess = 1;

 std::string cmd; // строка для считывания имени команды
 // непосредственно работа с файлом
 std::string str; // строка, в которую считываем строки файла
 std::getline(in, str); // считываем из входного файла первую строку
 while (in) { // если очередная строка считана успешно
 // обрабатываем строку
 if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#')) {
 // прочитанная строка не пуста и не комментарий
 std::stringstream s(str); // строковый поток из строки str
 s >> cmd;
 if (cmd == "camera") { // положение камеры
 float x, y, z;
 s >> x >> y >> z; // координаты точки наблюдения
 S = glm::vec3(x, y, z);
 s >> x >> y >> z; // точка, в которую направлен вектор наблюдения
 P = glm::vec3(x, y, z);
 s >> x >> y >> z; // вектор направления вверх
 u = glm::vec3(x, y, z);
 }
 else if (cmd == "screen") { // положение окна наблюдения
 s >> fovy_work >> aspect >> near_view >> far_view; // параметры команды
 fovy = glm::radians(fovy_work); // перевод угла из градусов в радианты
 }
 else if (cmd == "color") { // цвет линии
	 float r, g, b;
	 s >> r >> g >> b; // считываем три составляющие цвета
	 ambient = glm::vec3(r, g, b) / 255.f;
	 diffuse = ambient;
	 specular = ambient;
 }
 else if (cmd == "ambient") { // способность излучать окружающий свет
	 float r, g, b;
	 s >> r >> g >> b; // считываем три составляющие цвета
	 ambient = glm::vec3(r, g, b);
 }
 else if (cmd == "diffuse") { // способность излучать рассеянный свет
	 float r, g, b;
	 s >> r >> g >> b; // считываем три составляющие цвета
	 diffuse = glm::vec3(r, g, b);
 }
 else if (cmd == "specular") { // // способность излучать блики
	 float r, g, b;
	 s >> r >> g >> b; // считываем три составляющие цвета
	 specular = glm::vec3(r, g, b);
 }
 else if (cmd == "shininess") { // степень глянцевости
	 s >> shininess;
 }

 else if (cmd == "thickness") { // толщина линии
 s >> thickness; // считываем значение толщины
 }

else if (cmd == "mesh") { // набор мешей
std::vector<vertex> vertices; // список точек
int N, K; // количество точек и треугольников
s >> N >> K;
std::string str1; // дополнительная строка для чтения из файла
while (N > 0) { // пока не все точки считали
std::getline(in, str1); // считываем в str1 из входного файла очередную строку
// так как файл корректный, то на конец файла проверять не нужно
if ((str1.find_first_not_of(" \t\r\n") != std::string::npos) && (str1[0] != '#')) {
 // прочитанная строка не пуста и не комментарий
 // значит в ней тройка координат
 float x, y, z; // переменные для считывания координат вершины
 float nx, ny, nz; // переменные для считывания координат нормали
 std::stringstream s1(str1); // еще один строковый поток из строки str1
 s1 >> x >> y >> z;
 s1 >> nx >> ny >> nz;
 // добавляем точку в список
 vertices.push_back({ glm::vec3(x, y, z), glm::vec3(nx, ny, nz) });
 N--; // уменьшаем счетчик после успешного считывания точки
 }
 }
 std::vector<GLuint> indices; // список индексов вершин треугольников
 while (K > 0) { // пока не считали все треугольники
 std::getline(in, str1); // считываем в str1 из входного файла очередную строку
 // так как файл корректный, то на конец файла проверять не нужно
 if ((str1.find_first_not_of(" \t\r\n") != std::string::npos) && (str1[0] != '#')) {
 // прочитанная строка не пуста и не комментарий
 // значит в ней тройка индексов вершин треугольника
 GLuint x; // переменная для считывания
 std::stringstream s1(str1); // еще один строковый поток из строки str1
 for (int i = 0; i < 3; i++) { // три раза
 s1 >> x; // считываем индекс
indices.push_back(x); // добавляем индекс в список indices
}
K--; // уменьшаем счетчик после успешного считывания точки
}
}
// все точки и индексы считаны, генерируем меш и кладем его в список figure
 figure.push_back(mesh(vertices, indices, { ambient, diffuse, specular, shininess }));

}



else if (cmd == "model") { // начало описания нового рисунка
float mVcx, mVcy, mVcz, mVx, mVy, mVz; // параметры команды model
s >> mVcx >> mVcy >> mVcz >> mVx >> mVy >> mVz; // считываем значения переменных
float S = mVx / mVy < 1 ? 2.f / mVy : 2.f / mVx;
// сдвиг точки привязки из начала координат в нужную позицию
// после которого проводим масштабирование
initM = glm::scale(glm::vec3(S)) * glm::translate(glm::vec3(-mVcx, -mVcy, -mVcz));
figure.clear();
}
else if (cmd == "figure") { // формирование новой модели
models.push_back(model(figure, M * initM));
}
else if (cmd == "translate") { // перенос
float Tx, Ty, Tz; // параметры преобразования переноса
s >> Tx >> Ty >> Tz; // считываем параметры
M = glm::translate(glm::vec3(Tx, Ty, Tz)) * M; // добавляем перенос к общему преобразованию
}
else if (cmd == "scale") { // масштабирование
float S; // параметр масштабирования
s >> S; // считываем параметр
M = glm::scale(glm::vec3(S)) * M; // добавляем масштабирование к общему преобразованию
}
else if (cmd == "rotate") { // поворот
float theta; // угол поворота в градусах
float nx, ny, nz; // координаты направляющего вектора оси вращения
s >> theta >> nx >> ny >> nz; // считываем параметры
// добавляем вращение к общему преобразованию
M = glm::rotate(glm::radians(theta), glm::vec3(nx, ny, nz)) * M;
}
else if (cmd == "pushTransform") { // сохранение матрицы в стек
transforms.push_back(M); // сохраняем матрицу в стек
}
else if (cmd == "popTransform") { // откат к матрице из стека
M = transforms.back(); // получаем верхний элемент стека
 transforms.pop_back(); // выкидываем матрицу из стека
 }
 }
 // считываем очередную строку
 std::getline(in, str);
 }
 initWorkPars();
 }
 }

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

}

// матрица вращения относительно оси, заданной вектором n,
// проходящей через точку P
glm::mat4 rotateP(float theta, glm::vec3 n, glm::vec3 P) {
	return glm::translate(P) * glm::rotate(theta, n) * glm::translate(-P);
}
// обработчик панелей прокрутки
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	dist += yoffset;
}


// обработчик положения курсора
void cursorPos_callback(GLFWwindow* window, double xpos, double ypos) {
// вычисляем вектор, задающий ось вращения
glm::vec3 n = glm::vec3(lastY - ypos, lastX - xpos, 0);
// создаем матрицу вращения
glm::mat4 M = glm::rotate(glm::length(n) * 0.002f, n);
// вращаем току (0, 0, -1), на которую смотрит наблюдатель
glm::vec3 P = M * glm::vec4(0, 0, -1, 1);
// добавляем к преобразованиям переход к новой системе координат наблюдателя
 T = glm::lookAt(glm::vec3(0), P, glm::vec3(0, 1, 0)) * T;
 lastX = xpos;
 lastY = ypos;
 }

void cursorPosSave_callback(GLFWwindow* window, double xpos, double ypos) {
	if (click) {

		// вычисляем вектор, задающий ось вращения
		glm::vec3 n = glm::vec3(lastY - ypos, lastX - xpos, 0);
		// создаем матрицу вращения
		glm::mat4 M = rotateP(glm::length(n) * 0.002f, n, glm::vec3(0, 0, -dist));
		// вращаем току (0, 0, -1), на которую смотрит наблюдатель
		glm::vec3 P = M * glm::vec4(0, 0, -1, 1);


		glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // вращение направления вверх
		glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1)); // вращение начала координат
		// переход к СКН в которой начало координат в новой точке, а направление
		// наблюдения - в точку P
		T = glm::lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
	}
	lastX = xpos;
	lastY = ypos;

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if(action == GLFW_PRESS) click = true; 
		else
		if ( action == GLFW_RELEASE)
				click = false;
	}
	

}

bool isOn = false;

// Обработчик нажатия клавиш
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
if (action != GLFW_RELEASE) { // если клавиша нажата
switch (key) { // анализируем обрабатываемую клавишу

case GLFW_KEY_ESCAPE: // если клавиша - Escape
initWorkPars();
break;
case GLFW_KEY_W: {
	if (mode == GLFW_MOD_SHIFT) {
		T = glm::lookAt(glm::vec3(0, 0, -0.1), glm::vec3(0, 0, -0.2), glm::vec3(0, 0.1, 0)) * T;
	}
	else
	T = glm::lookAt(glm::vec3(0, 0, -1), glm::vec3(0, 0, -2), glm::vec3(0, 1, 0)) * T;
	break;
}
case GLFW_KEY_S: {
	if (mode == GLFW_MOD_SHIFT) {
		T = glm::lookAt(glm::vec3(0, 0, 0.1), glm::vec3(0, 0, 0), glm::vec3(0, 0.1, 0)) * T;
	}
	T = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)) * T;
	break;
}
case GLFW_KEY_A: {
	if (mode == GLFW_MOD_SHIFT) {
		T = glm::lookAt(glm::vec3(-0.1, 0, 0), glm::vec3(-0.1, 0, -0.1), glm::vec3(0, 0.1, 0)) * T;

	}
	T = glm::lookAt(glm::vec3(-1, 0, 0), glm::vec3(-1, 0, -1), glm::vec3(0, 1, 0)) * T;
	break;
}
case GLFW_KEY_D: {
	if (mode == GLFW_MOD_SHIFT) {
		T = glm::lookAt(glm::vec3(0.1, 0, 0), glm::vec3(0.1, 0, -0.1), glm::vec3(0, 0.1, 0)) * T;
	}
	T = glm::lookAt(glm::vec3(1, 0, 0), glm::vec3(1, 0, -1), glm::vec3(0, 1, 0)) * T;
	break;
}
 case GLFW_KEY_R: {
 glm::vec3 u_new = glm::mat3(glm::rotate(0.1f, glm::vec3(0, 0, 1))) * glm::vec3(0, 1, 0);
 T = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), u_new) * T;
 break;
 }

 case GLFW_KEY_Y: {
	 glm::vec3 u_new = glm::mat3(glm::rotate(-0.1f, glm::vec3(0, 0, 1))) * glm::vec3(0, 1, 0);
	 T = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), u_new) * T;
	 break;
 }
 case GLFW_KEY_T: {
 if (mode == GLFW_MOD_SHIFT) {
 // матрица вращения относительно точки P
 glm::mat4 M = rotateP(0.1, glm::vec3(1, 0, 0), glm::vec3(0, 0, -dist));
 glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // вращение направления вверх
 glm::vec3 S_new = glm::vec3(M * glm::vec4(0, 0, 0, 1)); // вращение начала координат
 // переход к СКН в которой начало координат в новой точке, а направление
 // наблюдения - в точку P
 T = glm::lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
 }
 else {
 glm::mat4 M = glm::rotate(0.1f, glm::vec3(1, 0, 0)); // матрица вращения относительно Ox
 glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // вращение направления вверх
 // вращение точки, в которую смотрит наблюдатель
 glm::vec3 P_new = glm::vec3(M * glm::vec4(0, 0, -1, 1));
 T = glm::lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
 }
 break;
 }

 case GLFW_KEY_0: {
	 if (!isOn) {
		 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		 isOn = true;
	 }
	 else {
		 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		 isOn = false;
	 }
	 break;
 }

 case GLFW_KEY_G: {
	 if (mode == GLFW_MOD_SHIFT)
	 {
		 glm::mat4 M = rotateP(-0.1, glm::vec3(1, 0, 0), glm::vec3(0, 0, -dist)); // матрица вращения относительно точки P
		 glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // вращение направления вверх
		 glm::vec3 S_new = glm::normalize(M * glm::vec4(0, 0, 0, 1)); // вращение начала координат
		 // переход к СКН в которой начало координат в новой точке, а направление
		 // наблюдения - в точку P
		 T = glm::lookAt(S_new, glm::vec3(0, 0, -dist), u_new) * T;
	 }
	 else {
		 glm::mat4 M = glm::rotate(-0.1f, glm::vec3(1, 0, 0)); // матрица вращения относительно Ox
		 glm::vec3 u_new = glm::mat3(M) * glm::vec3(0, 1, 0); // вращение направления вверх
		 glm::vec3 P_new = glm::normalize(M * glm::vec4(0, 0, -1, 1)); // вращение точки, в которую смотрит наблюдатель
		 T = glm::lookAt(glm::vec3(0, 0, 0), P_new, u_new) * T;
	 }

 case GLFW_KEY_F: {
	 if (mode == GLFW_MOD_SHIFT)
	 {
		 mat4 M = rotateP(0.1, vec3(0, 1, 0), vec3(0, 0, -dist)); // матрица вращения относительно точки P
		 vec3 u_new = mat3(M) * vec3(0, 1, 0); // вращение направления вверх
		 vec3 S_new = normalize(M * vec4(0, 0, 0, 1)); // вращение начала координат
		 // переход к СКН в которой начало координат в новой точке, а направление
		 // наблюдения - в точку P
		 T = lookAt(S_new, vec3(0, 0, -dist), u_new) * T;
	 }
	 else {
		 mat4 M = rotate(0.1f, vec3(0, 1, 0)); // матрица вращения относительно Ox
		 vec3 u_new = mat3(M) * vec3(0, 1, 0); // вращение направления вверх
		 vec3 P_new = normalize(M * vec4(0, 0, -1, 1)); // вращение точки, в которую смотрит наблюдатель
		 T = lookAt(vec3(0, 0, 0), P_new, u_new) * T;
	 }
	 break;

 }

 case GLFW_KEY_H: {
	 if (mode == GLFW_MOD_SHIFT)
	 {
		 mat4 M = rotateP(-0.1, vec3(0, 1, 0), vec3(0, 0, -dist)); // матрица вращения относительно точки P
		 vec3 u_new = mat3(M) * vec3(0, 1, 0); // вращение направления вверх
		 vec3 S_new = normalize(M * vec4(0, 0, 0, 1)); // вращение начала координат
		 // переход к СКН в которой начало координат в новой точке, а направление
		 // наблюдения - в точку P
		 T = lookAt(S_new, vec3(0, 0, -dist), u_new) * T;
	 }
	 else {
		 mat4 M = rotate(-0.1f, vec3(0, 1, 0)); // матрица вращения относительно Ox
		 vec3 u_new = mat3(M) * vec3(0, 1, 0); // вращение направления вверх
		 vec3 P_new = normalize(M * vec4(0, 0, -1, 1)); // вращение точки, в которую смотрит наблюдатель
		 T = lookAt(vec3(0, 0, 0), P_new, u_new) * T;
	 }
	 break;

 }
 case GLFW_KEY_I:
	 if (mode == GLFW_MOD_SHIFT) {
		 t -= 1;
	 }
	 else {
		 t += 1;
	 }
	 break;
 case GLFW_KEY_J:
	 if (mode == GLFW_MOD_SHIFT) {
		 l += 1;
	 }
	 else {
		 l -= 1;
	 }
	 break;
 case GLFW_KEY_K:
	 if (mode == GLFW_MOD_SHIFT) {
		 b -= 1;
	 }
	 else {
		 b += 1;
	 }
	 break;
 case GLFW_KEY_L:
	 if (mode == GLFW_MOD_SHIFT) {
		 r += 1;
	 }
	 else {
		 r -= 1;
	 }
	 break;
 case GLFW_KEY_U:
	 if (mode == GLFW_MOD_SHIFT) {
		 if (n < f - 0.1 && n > 0.1) n += 0.2;
	 }
	 else {
		 if (n > 0.1 && n < f - 0.1) n -= 0.2;
	 }
	 break;
 case GLFW_KEY_O:
	 if (mode == GLFW_MOD_SHIFT) {
		 if (f > n + 0.1)f += 0.2;
	 }
	 else {
		 if (f > n + 0.1)f -= 0.2;
	 }
	 break;

 case GLFW_KEY_B:
	 if (mode == GLFW_MOD_SHIFT) {
		 dist += 0.2;
	 }
	 else {
		 if (dist > 0.1)dist -= 0.2;
	 }
	 break;


 case GLFW_KEY_Z:
	 if (mode == GLFW_MOD_SHIFT) {
		 if (fovy < 3) fovy_work += 0.1;

	 }
	 else {
		 if (fovy > 0.3)fovy_work -= 0.1;
	 }
	 break;
 case GLFW_KEY_X:
	 if (mode == GLFW_MOD_SHIFT) {
		 aspect_work += 0.05;

	 }
	 else {
		 if (aspect_work > 0.1) aspect_work -= 0.05;
	 }
	 break;
 }
 break;

  case GLFW_KEY_4:
	  if (mode == GLFW_MOD_SHIFT) {
		  lightM2 = inverse(T) * translate(vec3(0, 0, -0.3)) * T * lightM2;
	  }
	  else {

		  lightM2 = inverse(T) * translate(vec3(0, 0, 0.3)) * T * lightM2;
	  }
	  break; 


  case GLFW_KEY_5:
	  if (mode == GLFW_MOD_SHIFT) {
		  lightM2 = inverse(T) * translate(vec3(-0.3, 0, 0)) * T * lightM2;
	  }
	  else {

		  lightM2 = inverse(T) * translate(vec3(0.3, 0, 0)) * T * lightM2;
	  }
	  break;

  case GLFW_KEY_6:
	  if (mode == GLFW_MOD_SHIFT) {
		  lightM2 = inverse(T) * translate(vec3(0, -0.3, 0)) * T * lightM2;
	  }
	  else {

		  lightM2 = inverse(T) * translate(vec3(0, 0.3, 0)) * T * lightM2;
	  }
	  break;

  case GLFW_KEY_7:
	  if (mode == GLFW_MOD_SHIFT) {
		  diffuse /= 1.1;
		  specular /= 1.1;
	  }
	  else {
		  diffuse*= 1.1;
		  specular *= 1.1;

	  }
	  break;



 case GLFW_KEY_1:
 pType = Ortho;
 break;
 case GLFW_KEY_2:
	 pType = Frustum;
	 break;
 case GLFW_KEY_3:
 pType = Perspective;
 break;
 case GLFW_KEY_F3: {
	 OPENFILENAME openFileDialog; // диалог открытия файла
	 char fileName[260]; // буфер для имени файла
	 // Инициализация файлового диалога
	 ZeroMemory(&openFileDialog, sizeof(openFileDialog));
	 openFileDialog.lStructSize = sizeof(openFileDialog);
	 openFileDialog.hwndOwner = NULL;
	 openFileDialog.lpstrFile = fileName;
	 openFileDialog.lpstrFile[0] = '\0';
	 openFileDialog.nMaxFile = sizeof(fileName);
	 openFileDialog.lpstrFilter = "Text files (*.txt)\0*.txt\0All files 2007\0*.*\0";
	 openFileDialog.nFilterIndex = 1;
	 openFileDialog.lpstrFileTitle = NULL;
	 openFileDialog.nMaxFileTitle = 0;
	 openFileDialog.lpstrInitialDir = NULL;
	 openFileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	 // Ввывод файлового диалога и получение результата
	 if (GetOpenFileName(&openFileDialog)) {
		 readFromFile(fileName);
		 // если файл выбран успешно
	 }
	 break;
 }
case GLFW_KEY_F5:
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPosCallback(window, cursorPosSave_callback);
		}
		else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, cursorPos_callback);
		}
		 break;


 default:
 break;
 }
 }
 }
int main() {
	glfwInit(); // Инициализация GLFW
	// Проведение начальных установок GLFW
	// Задается минимальная требуемая версия OpenGL.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Номер до десятичной точки
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Номер после десятичной точки
	// Используем только средства указанной версии без совместимости с более ранними
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Создаем окно
	GLFWwindow* window = glfwCreateWindow(800, 600, "Task 7. Chauenov", NULL, NULL);
	if (window == NULL) { // если ссылка на окно не создана
		std::cout << "Вызов glfwCreateWindow закончился неудачей." << std::endl;
		glfwTerminate(); // завершить работу GLFW
		return -1; // завершить программу
	}
	glfwMakeContextCurrent(window); // делаем окно window активным (текущим)
	// Назначение обработчика события Resize
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// Назначение обработчика нажатия клавиш
	
	glfwSetKeyCallback(window, key_callback);
	// назначение обработчика положения курсора
	glfwSetCursorPosCallback(window, cursorPosSave_callback);
	//	прокрутка
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	
	
	// Инициализация GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Не удалось загрузить GLAD" << std::endl;
		glfwTerminate(); // завершить работу GLFW
		return -1; // завершить программу
	}

	// сообщаем диапазон координат в окне
	// (0, 0) - координаты левого нижнего угла, 800x600 - размеры окна в пикселах
	glViewport(0, 0, 800, 600);
	shader vertexShader("Vertex.glsl", GL_VERTEX_SHADER); // вершинный шейдер
	shader fragmentShader("Fragment.glsl", GL_FRAGMENT_SHADER); // фрагментный шейдер

	//GLuint vertexShader; // шейдерный объект - вершинный шейдер
	//vertexShader = glCreateShader(GL_VERTEX_SHADER); // создаем объект
//	//=====================================================================
//// ВЕРШИННЫЙ ШЕЙДЕР
////=====================================================================
//	const char* vertexShaderSource =
//		"#version 330 core\n"
//		"layout (location = 0) in vec3 position;\n"
//		"uniform mat4 clipView;\n"
//		"void main() {\n"
//		" gl_Position = clipView * vec4(position, 1.0);\n"
//		"}\0";
//	//=====================================================================
//	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader); // компилируем шейде
	GLint success; // результат компиляции
	// запрашиваем статус компиляции шейдера в переменную success
	glGetShaderiv(vertexShader.shaderID, GL_COMPILE_STATUS, &success);
	if (!success) { // если компиляция прошла с ошибкой
		GLchar infoLog[512]; // объявим контейнер для сообщения об ошибке
		glGetShaderInfoLog(vertexShader.shaderID, 512, NULL, infoLog); // запрашиваем сообщение
		// выводим сообщение об ошибке на экран
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	////======================================================
	//// ФРАГМЕНТНЫЙ ШЕЙДЕР
	////======================================================
	//const char* fragmentShaderSource =
	//	"#version 330 core\n"
	//	"out vec4 color;\n"
	//	"uniform vec3 pathColor;\n"
	//	"void main() {\n"
	//	" color = vec4(pathColor, 1.0f);\n"
	//	"}\n\0";
	////=======================================================
	//	GLuint fragmentShader; // шейдерный объект - фрагментный шейдер

	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // создаем объект
	//// привязываем исходный код к шейдерному объекту
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//glCompileShader(fragmentShader); // компилируем шейдер
	// запрашиваем статус компиляции шейдера в описанную ранее переменную success
	glGetShaderiv(fragmentShader.shaderID, GL_COMPILE_STATUS, &success);
	if (!success) { // если компиляция прошла с ошибкой
		GLchar infoLog[512]; // объявим контейнер для сообщения об ошибке
		glGetShaderInfoLog(fragmentShader.shaderID, 512, NULL, infoLog); // запрашиваем сообщение
		// выводим сообщение об ошибке на экран
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Шейдерная программа
	//GLuint shaderProgram; // идентификатор шейдерной программы
	//shaderProgram = glCreateProgram(); // создаем программный объект
	//glAttachShader(shaderProgram, vertexShader.shaderID); // присоединяем вершинный шейдер
	//glAttachShader(shaderProgram, fragmentShader.shaderID); // фрагментный шейде
	//glLinkProgram(shaderProgram); // компоновка программ
	//// запрашиваем статус компоновки шейдерной программы в переменную success
	//glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	//if (!success) { // если компоновка прошла с ошибкой
	//	GLchar infoLog[512]; // объявим контейнер для сообщения об ошибке
	//	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); // запрашиваем сообщение
	//	// выводим сообщение об ошибке на экран
	//	std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;

	//}
	// Шейдерная программа
	program shaderProgram(vertexShader, fragmentShader);

	// удаление шейдерных объектов
	vertexShader.clear();
	fragmentShader.clear();
	// запрашиваем у программы shaderProgram расположение переменной pathColor
	// декларируем использование uniform-переменных
	// вершинный шейдер для источника света
	fragmentShader = shader("C:/Users/chaue/Desktop/8 кг/chauenov_7_task7/LightVertex.glsl", GL_VERTEX_SHADER);
	// фрагментный шейдер для источника света
	fragmentShader = shader("C:/Users/chaue/Desktop/8 кг/chauenov_7_task7/LightFragment.glsl", GL_FRAGMENT_SHADER);
	// шейдерная программа для источника света
	program lightShaderProgram(vertexShader, fragmentShader);
	// удаление шейдерных объектов
	vertexShader.clear();
	fragmentShader.clear();
	lightShaderProgram.useUniform("clipView");
	lightShaderProgram.useUniform("pathColor");

	lightShaderProgram.useUniform("clipView2");
	lightShaderProgram.useUniform("pathColor2");

	shaderProgram.useUniform("pathColor");
	shaderProgram.useUniform("clipView");

	shaderProgram.useUniform("modelView");
	shaderProgram.useUniform("modelInv");
	shaderProgram.useUniform("lightPos");
	shaderProgram.useUniform("viewPos");
	shaderProgram.useUniform("light.ambient");
	shaderProgram.useUniform("light.diffuse");
	shaderProgram.useUniform("light.specular");

	//==========================================================
// НАБОР ИСХОДНЫХ ДАННЫХ ДЛЯ ОТРИСОВКИ ИСТОЧНИКА СВЕТА
//==========================================================
	GLfloat lightVertices[] = {
	-0.1f, 0.f, 0.f,
	0.1f, 0.f, 0.f,
	0.f, 0.1f, 0.f,
	0.f, -0.1f, 0.f,
	0.f, 0.f, 0.1f,
	0.f, 0.f, -0.1f,
	0.07071f, 0.07071f, 0.f,
	-0.07071f, -0.07071f, 0.f,
	-0.07071f, 0.07071f, 0.f,
	0.07071f, -0.07071f, 0.f,
	0.07071f, 0.f, 0.07071f,
	-0.07071f, 0.f, -0.07071f,
-0.07071f, 0.f, 0.07071f,
0.07071f, 0.f, -0.07071f,
0.f, 0.07071f, 0.07071f,
0.f, -0.07071f, -0.07071f,
0.f, -0.07071f, 0.07071f,
0.f, 0.07071f, -0.07071f,
0.05774f, 0.05774f, 0.05774f,
-0.05774f, -0.05774f, -0.05774f,
-0.05774f, -0.05774f, 0.05774f,
0.05774f, 0.05774f, -0.05774f,
-0.05774f, 0.05774f, 0.05774f,
0.05774f, -0.05774f, -0.05774f,
0.05774f, -0.05774f, 0.05774f,
-0.05774f, 0.05774f, -0.05774f
	};
	GLuint lightVertexArray; // объект вершинного массива
	// создаем вершинный массив, идентификатор которого присваиваем vertexArray
	glGenVertexArrays(1, &lightVertexArray);
	glBindVertexArray(lightVertexArray); // делаем активным вершинный массив
	GLuint lightVertexBuffer; // идентификатор буферного объекта
	// создаем буферный объект, идентификатор которого присваиваем vertexBuffer
	glGenBuffers(1, &lightVertexBuffer);
	// привязка vertexBuffer к GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, lightVertexBuffer);
	// в буфер, привязанный к GL_ARRAY_BUFFER копируем содержимое vertices
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);
	// описание расположения параметра вершинного шейдера в вершинном буфере
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // включение параметра 0 для шейдера
	glBindBuffer(GL_ARRAY_BUFFER, 0); // отвязка буферного объекта
	glBindVertexArray(0); // отключение вершинного массива
	// перемещение источника света из начала координат в точку (0, 0, 5)
	lightM = glm::translate(glm::vec3(0, 0, 5));
	lightM2 = glm::translate(glm::vec3(7, 5, -4));
//	//==============================================
//// НАБОР ИСХОДНЫХ ДАННЫХ ДЛЯ ОТРИСОВКИ
////==============================================
//	GLfloat vertices[] = {
//	-0.5f, -0.5f, 0.0f,
//	0.5f, -0.5f, 0.0f,
//	0.0f, 0.5f, 0.0f
//	};
//
//	GLuint vertexArray; // объект вершинного массива
//	// создаем вершинный массив, идентификатор которого присваиваем vertexArray
//	glGenVertexArrays(1, &vertexArray);
//	glBindVertexArray(vertexArray); // делаем активным вершинный массив
//
//	GLuint vertexBuffer; // идентификатор буферного объекта
//	// создаем буферный объект, идентификатор которого присваиваем vertexBuffer
//	glGenBuffers(1, &vertexBuffer);
//	// привязка vertexBuffer к GL_ARRAY_BUFFER
//	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
//	// в буфер, привязанный к GL_ARRAY_BUFFER копируем содержимое vertices
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//	// описание расположения параметра вершинного шейдера в вершинном буфере
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0); // включение параметра 0 для шейдера
//	glBindBuffer(GL_ARRAY_BUFFER, 0); // отвязка буферного объекта
//	glBindVertexArray(0); // отключение вершинного массива

	readFromFile("C:/Users/chaue/Desktop/8 кг/chauenov_7_task7/triangle.txt");
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) { // пока окно window не должно закрыться
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // назначаем цвет заливки
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 move = glm::rotate((float)glfwGetTime() * glm::radians(10.0f), glm::vec3(0.0f,  1.0f, 1.0f));
		glm::mat4 lightM1 = move * lightM;



		shaderProgram.use(); // шейдерную программу shaderProgram делаем активной
		//glBindVertexArray(vertexArray); // делаем активным вершинный массив
		//glDrawArrays(GL_TRIANGLES, 0, 3); // отрисовка одного треугольника
		//glBindVertexArray(0); // отключаем вершинный массив
		 glm::mat4 proj; // матрица перехода в пространство отсечения
		switch (pType) {
			case Ortho: // прямоугольная проекция
				proj = glm::ortho(l, r, b, t, n, f);
				break;
			case Frustum: // перспективная проекция с Frustum
				proj = glm::frustum(l, r, b, t, n, f);
				break;
			case Perspective: // перспективная проекция с Perspective
				proj = glm::perspective(fovy_work, aspect_work, n, f);
				break;
 }
 glm::mat4 C = proj * T; // матрица перехода от мировых координат в пространство отсечения
 for (int k = 0; k < models.size(); k++) { // цикл по моделям
 std::vector<mesh> figure = models[k].figure; // список ломаных очередной модели
 glm::mat4 TM = C * models[k].modelM; // матрица общего преобразования модели
 // пересылка пересылка матрицы TM в переменную clipView шейдерной программы
 shaderProgram.setUniform("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
 shaderProgram.setUniform("light.diffuse", diffuse);
 shaderProgram.setUniform("light.specular", specular);
 shaderProgram.setUniform("light1.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
 shaderProgram.setUniform("light1.diffuse", diffuse);
 shaderProgram.setUniform("light1.specular", specular);

 shaderProgram.setUniform("lightPos", glm::vec3(lightM1 * glm::vec4(0, 0, 0, 1)));

 shaderProgram.setUniform("lightPos1", glm::vec3(lightM2* glm::vec4(0, 0, 0, 1)));


 shaderProgram.setUniform("viewPos", glm::vec3(glm::inverse(T) * glm::vec4(0, 0, 0, 1)));
 shaderProgram.setUniform("clipView", TM);
 shaderProgram.setUniform("modelView", models[k].modelM);
 shaderProgram.setUniform("modelInv", glm::transpose(glm::inverse(models[k].modelM)));
 shaderProgram.useUniform("material.ambient");
 shaderProgram.useUniform("material.diffuse");
 shaderProgram.useUniform("material.specular");
 shaderProgram.useUniform("material.shininess");


 for (int i = 0; i < figure.size(); i++) {
	 // пересылка цвета линии в переменную pathColor шейдерной программы
	 shaderProgram.setUniform("material.ambient", figure[i].material.ambient);
	 shaderProgram.setUniform("material.diffuse", figure[i].material.diffuse);
	 shaderProgram.setUniform("material.specular", figure[i].material.specular);
	 shaderProgram.setUniform("material.shininess", figure[i].material.shininess);
	 glBindVertexArray(figure[i].vertexArray); // делаем активным вершинный массив i-й ломаной
	 glDrawElements(GL_TRIANGLES, figure[i].indices.size(), GL_UNSIGNED_INT, 0);
	 glBindVertexArray(0); // отключаем вершинный массив
	 
 }
 }
 lightShaderProgram.use(); // делаем активной программу для источника света
 // матрица перехода в пространство отсечения
 lightShaderProgram.setUniform("clipView", C* lightM1);


 // белый цвет
 lightShaderProgram.setUniform("pathColor", glm::vec3(0.2f, 0.2f, 0.2f) + glm::vec3(0.5f, 0.5f, 0.5f) + glm::vec3(1.f, 1.f, 1.f));

 glBindVertexArray(lightVertexArray); // делаем активным вершинный массив
 glDrawArrays(GL_LINES, 0, 26); // рисуем 13 отрезков (в массиве 26 точек)
 glBindVertexArray(0); // отключаем вершинный массив
 lightShaderProgram.use(); // делаем активной программу для источника света
 lightShaderProgram.setUniform("clipView", C * lightM2);
 // белый цвет
 lightShaderProgram.setUniform("pathColor", glm::vec3(0.2f, 0.2f, 0.2f)+ glm::vec3(0.5f, 0.5f, 0.5f)+ glm::vec3(1.f, 1.f, 1.f));

 glBindVertexArray(lightVertexArray); // делаем активным вершинный массив
 glDrawArrays(GL_LINES, 0, 26); // рисуем 13 отрезков (в массиве 26 точек)
 glBindVertexArray(0); // отключаем вершинный массив

		glfwSwapBuffers(window); // поменять местами буферы изображения
		glfwPollEvents(); // проверить, произошли ли какие-то события
	}
	
}