#ifndef PALETTE_H
#define PALETTE_H
#include <string>

#include "raylib.h"

//класс палитры
//размер палитры ограничен файлом проекта

class Palette {
	private:
		//Палитра в ОЗУ и во VRAM
		Image img = {0};
		Texture2D tex = {0};
	public:
		std::string name;
		
		Palette(std::string name);
		Palette(const Palette &palette) = delete;
		void operator=(const Palette &palette) = delete;
		~Palette();
		
		Color getColor(uint32_t id);
		
		//Отрисовка палитры
		void render(Rectangle rect);
		
		uint32_t getSize() {return img.width * img.height;}
		int getWidth() {return img.width;}
		int getHeight() {return img.height;}
};

#endif