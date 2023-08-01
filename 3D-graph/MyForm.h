#pragma once

namespace Project1 {
	
	using namespace std;
	
	vec2 Vc;
	vec2 V;

	vec2 Vc_work, V_work;
	
	
	mat3 T;// матрица, в которой накапливаются все преобразования
	// первоначально - единичная матрица
	mat3 initT; // матрица начального преобразования
	vector<model> models;

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Сводка для MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: добавьте код конструктора
			//
		}

	protected:
		/// <summary>
		/// Освободить все используемые ресурсы.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::OpenFileDialog^ openFileDialog;
	protected:

    private: float left = 30, right = 100, top = 20, bottom = 50; // расстояния до границ окна
		   float minX = left, maxX; // диапазон изменения координат x
		   float minY = top, maxY; // диапазон изменения координат y
		   float Wcx = left, Wcy; // координаты левого нижнего угла прямоугольника
		   float Wx, Wy; // ширина и высота прямоугольника
		   int numXsect = 5, numYsect = 5;

	private: System::Void rectCalc() {
		maxX = ClientRectangle.Width - right; // диапазон изменения координат x
		maxY = ClientRectangle.Height - bottom; // диапазон изменения координат y
		Wcy = maxY; // координаты левого нижнего угла прямоугольника
		Wx = maxX - left; // ширина прямоугольника
		Wy = maxY - top; // ширина и высота прямоугольника

	}

	private: System::Void worldRectCalc() {
		Vc_work = normalize(T * vec3(Vc, 1.f));
		V_work = mat2(T) * V;
	}
	private:float f(float x) {
		return tan(x);

	}
	private: bool f_exists(float x, float delta) {
		return fabs(2.f * acos(cos(x)) - Math::PI) > delta;
	}


	private:
		/// <summary>
		/// Обязательная переменная конструктора.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Требуемый метод для поддержки конструктора — не изменяйте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->openFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
			this->SuspendLayout();
			// 
			// openFileDialog
			// 
			this->openFileDialog->DefaultExt = L"txt";
			this->openFileDialog->FileName = L"Сделать поле пустым";
			this->openFileDialog->Filter = L"Текстовые файлы (*.txt)|*.txt|Все файлы (*.*)|*.*";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(232, 211);
			this->DoubleBuffered = true;
			this->KeyPreview = true;
			this->Margin = System::Windows::Forms::Padding(2, 3, 2, 3);
			this->MinimumSize = System::Drawing::Size(90, 96);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::MyForm_Paint);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::MyForm_KeyDown);
			this->Resize += gcnew System::EventHandler(this, &MyForm::MyForm_Resize);
			this->ResumeLayout(false);

		}
		
#pragma endregion
	

	private: System::Void MyForm_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
		
		//red line
		System::Drawing::Graphics^ g = e->Graphics;
		g->Clear(Color::Aquamarine);
		Pen^ rectPen = gcnew Pen(Color::Black, 2);
		g->DrawRectangle(rectPen, left, top, Wx, Wy);
		Pen^ pen = gcnew Pen(Color::Blue, 1);
		float deltaX = V_work.x / Wx;
		Pen^ gridPen = gcnew Pen(Color::Black, 1);
		SolidBrush^ drawBrush = gcnew SolidBrush(Color::Black);
		//x
		System::Drawing::Font^ drawFont = gcnew System::Drawing::Font("Arial", 8);
		float gridStep_x = Wx / numXsect; // расстояние между линиями сетки по x

		float grid_dX = V_work.x / numXsect; // расстояние между линиями сетки по x в мировых координата
		//red line
		float tick_x = Vc_work.x; // значение соответствующее первой линии сетки
		for (int i = 0; i <= numXsect; i++) { // цикл по количеству линий
			
			float tmpXCoord_v = Wcx + i * gridStep_x; // координата x i-й вертикальной линии
			// i-я диагональная линия
			// i-я вертикальная линия
			g->DrawLine(gridPen, tmpXCoord_v, Wcy, tmpXCoord_v, minY);
			if (i > 0 && i < numXsect) // если линия не крайняя
				// выводим текст в нижней точке диагональной линии
				g->DrawString(tick_x.ToString("F4"), drawFont, drawBrush, tmpXCoord_v, Wcy);
			tick_x += grid_dX; // вычисляем значение, соответствующее следующей линии

		}
		//y
		float grid_dY = V_work.y / numYsect; // расстояние между линиями сетки по x в мировых координата
		float gridStep_y = Wy / numYsect; // расстояние между линиями сетки по x
		float tick_y = Vc_work.y; // значение соответствующее первой линии сетки
		for (int i = 0; i <= numYsect; i++) { // цикл по количеству линий

			float tmpYCoord_g = Wcy - i * gridStep_y; // координата x i-й вертикальной линии
			// i-я диагональная линия
			// i-я вертикальная линия
			g->DrawLine(gridPen, Wcx, tmpYCoord_g, Wcx+Wx, tmpYCoord_g);
			if (i > 0 && i < numYsect) // если линия не крайняя
				// выводим текст в нижней точке диагональной линии
				g->DrawString(tick_y.ToString("F4"), drawFont, drawBrush, Wcx, tmpYCoord_g);
			tick_y += grid_dY; // вычисляем значение, соответствующее следующей линии

		}

		vec2 start;
		float x, y;
		bool hasStart;
		start.x = Wcx; // для начальной точки первого отрезка устанавливаем координату x
		x = Vc_work.x;
		hasStart = f_exists(x, deltaX);
		if (hasStart) {
			y = f(x); // координата y начальной точки в мировых координатах
			// вычисляем соответствующее значение в координатах экрана
			start.y = Wcy - (y - Vc_work.y) / V_work.y * Wy;
		}
		while (start.x < maxX) {
			float deltaY; // высота точки в прямоугольнике (доля общей высоты)
			float red, green, blue; // компоненты цвета отрезк
			vec2 end;// точка конца отрезка в координатах экрана
			bool hasEnd;

			end.x = start.x + 1.f; // координата x отличается на единицу
			x += deltaX; // координата x конечной точки отрезка в мировых координатах
			y = f(x); // координата y конечной точки в мировых координатах
			// вычисляем соответствующее значение в координатах экрана
			// вычисляем соответствующее значение в координатах экрана
			deltaY = (y - Vc_work.y) / V_work.y;
			end.y = Wcy - deltaY * Wy;
			hasEnd = f_exists(x,deltaX);
			if (hasEnd) {
				y = f(x); // координата y начальной точки в мировых координатах
				// вычисляем соответствующее значение в координатах экрана
				end.y = Wcy - (y - Vc_work.y) / V_work.y * Wy;
			}

			vec2 tmpEnd = end;
			bool visible = hasStart && hasEnd && clip(start, end, minX, minY, maxX, maxY);
			if (visible) { // если отрезок видим
				// после отсечения, start и end - концы видимой части отрезка
				if (deltaY > 1.f) deltaY = 1.f; // нормализуем значение высоты точки
				if (deltaY < 0.f) deltaY = 0.f; // на случай, если отрезок отсекался
				green = 510.f * deltaY; // предварительное вычисление произведения
				if (deltaY < 0.5) { // если точка ниже середины области видимости
					// компонента зеленого уже вычислена
					blue = 255.f - green; // синий дополняет зеленый
					red = 0.f; // красный равен нулю
				}
				else { // если точка не ниже середины
					blue = 0.f; // синий равен нулю
					red = green - 255.f; // вычисляем красный и зеленый
					green = 510.f - green; // с использованием вычисленного произведения
				}
				pen->Color = Color::FromArgb(red, green, blue); // меняем цвет пер
				g->DrawLine(pen, start.x, start.y, end.x, end.y); // отрисовка видимых частей
			}
			// конечная точка неотсеченного отрезка становится начальной точкой следующего
			start = tmpEnd;
			hasStart = hasEnd;

		}
	}
	private: System::Void MyForm_Resize(System::Object^ sender, System::EventArgs^ e) {
		
		rectCalc();
		
		Refresh();
	}
	private: System::Void MyForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		float centerX = Vc_work.x + V_work.x / 2; // координаты центра прямоугольника
		float centerY = Vc_work.y + V_work.y / 2; // в мировой системе координат
		switch (e->KeyCode) {

		case Keys::A:
			T = translate(-V_work.x / Wx, 0.f) * T; // сдвиг графика вправо на один пиксел
			break;
		case Keys::D:
			T = translate(V_work.x / Wx, 0) * T; // сдвиг графика вправо на один пиксел
			break;
		case Keys::S:
			T = translate(0, -V_work.x / Wx) * T; // сдвиг графика вправо на один пиксел
			break;
		case Keys::W:
			T = translate(0, V_work.x / Wx) * T; // сдвиг графика вправо на один пиксел
			break;



		case Keys::Z:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1.1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;
		case Keys::X:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1/1.1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;
		case Keys::T:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1.1,1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;
		case Keys::G:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1 / 1.1,1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;
		case Keys::Y:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1,1.1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;
		case Keys::H:
			T = translate(-centerX, -centerY) * T; // перенос начала координат в центр
			T = scale(1,1 / 1.1) * T; // масштабирование относительно начала координат
			T = translate(centerX, centerY) * T; // возврат позиции начала координат
			break;



		case Keys::D1:
			numXsect += 1;
			break;
		case Keys::D2:
			if (numXsect >= 2) numXsect -= 1;
			break;
		case Keys::D3:
			numYsect += 1;
			break;
		case Keys::D4:
			if (numYsect >= 2) numYsect -= 1;
			break;
		
		case Keys::Escape:
			numXsect = 6;
			numYsect = 6;
			T = initT;
			break;
											

		default:
			break;
		}
		worldRectCalc();
		Refresh();
	}
	private: System::Void MyForm_Load(System::Object^ sender, System::EventArgs^ e) {
		Vc = vec2(-2.f, -2.f);
		V = vec2(4.f, 4.f);
		initT = mat3(1.f);
		T = initT;
		rectCalc();
		worldRectCalc();
			
		

		

	}

};
}
