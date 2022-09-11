#ifndef CHARSET_H
#define CHARSET_H
#include <string>

#include "raylib.h"

//класс набора символов
//charsCount ограничен файлом проекта

class Charset {
	private:
		//Набор символов в ОЗУ, во VRAM и во VRAM с инверсией прозрачности
		Image img = {0};
		Texture2D tex = {0}, invTex = {0};
	public:
		std::string name;
		
		//Размеры символа, кол-во символов по ширине и высоте, общее кол-во символов
		size_t charW, charH;
		size_t charsX, charsY;
		uint32_t charsCount;
		
		Charset(std::string name, size_t charW, size_t charH);
		Charset(const Charset &charset) = delete;
		void operator=(const Charset &charset) = delete;
		~Charset();
		
		//Рендер символа
		void renderChar(Rectangle rect, uint32_t ch, Color bck, Color col, bool flip);
		//Рендер набора символов
		void render(Rectangle rect, Color bck, Color col);
		
		int getWidth() {return img.width;}
		int getHeight() {return img.height;}
};

#endif