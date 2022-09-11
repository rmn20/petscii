#include "palette.h"

Palette::Palette(std::string name) {
	this->name = name;
	
	img = LoadImage(name.data());
	tex = LoadTextureFromImage(img);
}

Palette::~Palette() {
	UnloadImage(img);
	UnloadTexture(tex);
}

Color Palette::getColor(uint32_t id) {
	if(id < getSize()) return GetImageColor(img, id % img.width, id / img.width);
	else if(id == getSize()) return (Color) {0}; //Последний цвет в палитре - прозрачный
	else return RED;
}

void Palette::render(Rectangle rect) {
	DrawTexturePro(tex, 
		(Rectangle) {0, 0, (float) tex.width, (float) tex.height}, 
		rect, (Vector2) {0}, 0, WHITE);
}