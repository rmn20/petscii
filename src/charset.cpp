#include "charset.h"

Charset::Charset(std::string name, size_t charW, size_t charH) {
	this->name = name;
	
	img = LoadImage(name.data());
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	
	//Преобразование чёрно белой текстуры в белую текстуру с прозрачностью
	for(int x=0; x<img.width; x++) {
		for(int y=0; y<img.height; y++) {
			Color c = GetImageColor(img, x, y);
			
			ImageDrawPixel(&img, x, y, (Color) {255, 255, 255, (unsigned char) (255 - c.r)});
		}
	}
	
	//Инверсия прозрачности для корректной отрисовки символов с прозрачным цветов и НЕ прозрачным фоном
	invTex = LoadTextureFromImage(img);
	
	for(int x=0; x<img.width; x++) {
		for(int y=0; y<img.height; y++) {
			Color c = GetImageColor(img, x, y);
			
			ImageDrawPixel(&img, x, y, (Color) {255, 255, 255, (unsigned char) (255 - c.a)});
		}
	}
	
	tex = LoadTextureFromImage(img);
	
	this->charW = charW;
	this->charH = charH;
	
	charsX = img.width / charW;
	charsY = img.height / charH;
	
	charsCount = (uint32_t) (charsX * charsY);
}

Charset::~Charset() {
	UnloadImage(img);
	UnloadTexture(tex);
	UnloadTexture(invTex);
}

void Charset::renderChar(Rectangle rect, uint32_t ch, Color bck, Color col, bool flip) {
	
	//Фон
	if(col.a < 255) {
		DrawTexturePro(invTex, 
			(Rectangle) {
			(float) (ch % charsX) * invTex.width / charsX,
			(float) (ch / charsX) * invTex.height / charsY, 
			(float) invTex.width / charsX,
			(float) invTex.height / charsY * (flip?-1:1)
			}, 
			rect, (Vector2) {0}, 0, bck);
	} else {
		DrawRectangleRec(rect, bck);
	}
	
	//Сам символ
	DrawTexturePro(tex, 
		(Rectangle) {
		(float) (ch % charsX) * tex.width / charsX,
		(float) (ch / charsX) * tex.height / charsY, 
		(float) tex.width / charsX,
		(float) tex.height / charsY * (flip?-1:1)
		}, 
		rect, (Vector2) {0}, 0, col);
}

void Charset::render(Rectangle rect, Color bck, Color col) {
	
	//Фон
	if(col.a < 255) {
		DrawTexturePro(invTex, 
			(Rectangle) {0, 0, (float) invTex.width, (float) invTex.height}, 
			rect, (Vector2) {0}, 0, bck);
	} else {
		DrawRectangleRec(rect, bck);
	}
	
	//Сам символ
	DrawTexturePro(tex, 
		(Rectangle) {0, 0, (float) tex.width, (float) tex.height}, 
		rect, (Vector2) {0}, 0, col);
}