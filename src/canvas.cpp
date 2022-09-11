#include <cstring>

#include "canvas.h"

Canvas::~Canvas() {
	unload();
}

void Canvas::set(std::shared_ptr<Charset> charset, std::shared_ptr<Palette> palette, uint32_t w, uint32_t h) {
	this->charset = charset;
	this->palette = palette;
	
	canvasW = w; canvasH = h;
	bckColor = 0;
	
	chars = new uint32_t[w * h];
	charsColors = new uint32_t[w * h * 2];
	
	std::memset(chars, 0, sizeof(uint32_t) * w * h);
	//Цвет символа должен быть 0, а цвет фона - прозрачным
	for(size_t i=0; i<w * h; i++) {
		charsColors[i * 2] = palette->getSize();
		charsColors[i * 2 + 1] = 0;
	}
	
	//Обновление текстуры во VRAM
	if(getRenderWidth() != w * charset->charW || getRenderHeight() != h * charset->charH) {
		renderCanvas = LoadRenderTexture(w * charset->charW, h * charset->charH);
	}
}

void Canvas::unload() {
	delete[] chars;
	delete[] charsColors;
	
	UnloadRenderTexture(renderCanvas);
}

void Canvas::setBackgroundColor(uint32_t col) {
	bckColor = col;
}

void Canvas::setChar(size_t x, size_t y, uint32_t ch, uint32_t bck, uint32_t col) {
	if(x >= canvasW || y >= canvasH) return;
	
	chars[x + y*canvasW] = ch;
	
	charsColors[(x + y*canvasW) * 2] = bck;
	charsColors[(x + y*canvasW) * 2 + 1] = col;
}

uint32_t* Canvas::getCharColors(size_t x, size_t y) {
	if(x >= canvasW || y >= canvasH) return charsColors;
	
	return charsColors + (x + y*canvasW) * 2;
}
uint32_t Canvas::getChar(size_t x, size_t y) {
	if(x >= canvasW || y >= canvasH) return 0;
	
	return chars[x + y*canvasW];
}

void Canvas::repaintChar(size_t x, size_t y) {
	if(x >= canvasW || y >= canvasH) return;
	
	//Rect символа
	Rectangle rect = (Rectangle) {
		(float) (x * charset->charW),
		(float) (getRenderHeight() - (y + 1) * charset->charH),
		(float) charset->charW,
		(float) charset->charH
	};
	
	BeginTextureMode(renderCanvas); //Отрисовка на текстуре холста
	DrawRectangleRec(rect, palette->getColor(bckColor)); //Цвет холста
	
	//Указатель на символ в памяти
	size_t p = x + y*canvasW;
	
	charset->renderChar(
		rect, 
		chars[p], 
		palette->getColor(charsColors[p * 2]), 
		palette->getColor(charsColors[p * 2 + 1]),
		true
	);
	
	EndTextureMode();
}

void Canvas::repaint() {
	BeginTextureMode(renderCanvas); //Отрисовка на текстуре холста
	ClearBackground(palette->getColor(bckColor)); //Цвет холста
	
	for(size_t x=0; x<canvasW; x++) {
		for(size_t y=0; y<canvasH; y++) {
			
			//Rect символа
			//Нет, я не хочу вызывать здесь repaintChar, так как будет происходить переключение буфера отрисовки 100500 раз
			
			Rectangle rect = (Rectangle) {
				(float) (x * charset->charW),
				(float) (getRenderHeight() - (y + 1) * charset->charH),
				(float) charset->charW,
				(float) charset->charH
			};
			
			//Указатель на символ в памяти
			size_t p = x + y*canvasW;
			
			charset->renderChar(
				rect, 
				chars[p], 
				palette->getColor(charsColors[p * 2]), 
				palette->getColor(charsColors[p * 2 + 1]),
				true
			);
		}
	}
	
	EndTextureMode();
}

void Canvas::render(Vector2 pos, float scale) {
	DrawTextureEx(renderCanvas.texture, pos, 0, scale, WHITE);
}

void Canvas::resetUndo() {
	undoStorage.reset(bckColor, chars, charsColors, canvasW, canvasH, charset->name, palette->name);
}

void Canvas::storeUndo() {
	undoStorage.store(bckColor, chars, charsColors, canvasW, canvasH, charset->name, palette->name);
}

void Canvas::undo() {
	if(!canUndo()) return;
	
	undoStorage.undo();
	
	loadFromUndoStep(undoStorage.getStep());
}

void Canvas::redo() {
	if(!canRedo()) return;
	
	undoStorage.redo();
	
	loadFromUndoStep(undoStorage.getStep());
}

void Canvas::loadFromUndoStep(UndoStep* step) {
	bckColor = step->bckColor;
	
	if(canvasW * canvasH != step->w * step->h) {
		delete[] chars;
		delete[] charsColors;
		
		chars = new uint32_t[step->w * step->h];
		charsColors = new uint32_t[step->w * step->h * 2];
		
		canvasW = step->w;
		canvasH = step->h;
	}
	
	memcpy(chars, step->chars, canvasW * canvasH * sizeof(uint32_t));
	memcpy(charsColors, step->charsColors, canvasW * canvasH * sizeof(uint32_t) * 2);
	
	//todo load/unload charset, palette
	
	if(getRenderWidth() != canvasW * charset->charW || getRenderHeight() != canvasH * charset->charH) {
		UnloadRenderTexture(renderCanvas);
		renderCanvas = LoadRenderTexture(canvasW * charset->charW, canvasH * charset->charH);
	}
}

//todo endianess fix
//todo palette and charset loading

void Canvas::load(std::string fileName) {
	
	unsigned int size = 0;
	
	uint8_t* data = (uint8_t*) LoadFileData(fileName.c_str(), &size);
	uint8_t* rdata = data;
	
	size_t targetSize = 256 * sizeof(char) * 2 + sizeof(bckColor) + sizeof(canvasW) + sizeof(canvasH);
	if(size < targetSize) {
		UnloadFileData(data);
		return;
	}
	
	std::string charsetName = std::string((char*) (rdata + 256 * sizeof(char)));
	rdata += 256 * sizeof(char);
	
	std::string paletteName = std::string((char*) (rdata + 256 * sizeof(char)));
	rdata += 256 * sizeof(char);
	
	uint32_t bckColor = *((uint32_t*) rdata);
	rdata += sizeof(bckColor);
	
	uint32_t canvasW = *((uint32_t*) rdata);
	rdata += sizeof(canvasW);
	
	uint32_t canvasH = *((uint32_t*) rdata);
	rdata += sizeof(canvasH);
	
	targetSize += sizeof(uint32_t) * (canvasW * canvasH);
	targetSize += sizeof(uint32_t) * (canvasW * canvasH) * 2;
	if(size < targetSize) {
		UnloadFileData(data);
		return;
	}
	
	set(charset, palette, canvasW, canvasH);
	setBackgroundColor(bckColor);
	
	memcpy(chars, rdata, sizeof(uint32_t) * (canvasW * canvasH));
	rdata += sizeof(uint32_t) * (canvasW * canvasH);
	
	memcpy(charsColors, rdata, sizeof(uint32_t) * (canvasW * canvasH) * 2);
	rdata += sizeof(uint32_t) * (canvasW * canvasH) * 2;
	
	UnloadFileData(data);
	
	repaint();
	resetUndo();
}

void Canvas::save(std::string fileName) {
	
	size_t size = 
		2 * (256) * sizeof(char) +
		sizeof(bckColor) +
		sizeof(canvasW) +
		sizeof(canvasH) +
		sizeof(uint32_t) * (canvasW * canvasH * 3);
	
	uint8_t* file = new uint8_t[size];
	uint8_t* wfile = file;
	
	charset->name.resize(256);
	memcpy(wfile, charset->name.c_str(), 256 * sizeof(char));
	wfile += 256 * sizeof(char);
	
	palette->name.resize(256);
	memcpy(wfile, palette->name.c_str(), 256 * sizeof(char));
	wfile += 256 * sizeof(char);
	
	memcpy(wfile, &bckColor, sizeof(bckColor));
	wfile += sizeof(bckColor);
	
	memcpy(wfile, &canvasW, sizeof(canvasW));
	wfile += sizeof(canvasW);
	
	memcpy(wfile, &canvasH, sizeof(canvasH));
	wfile += sizeof(canvasH);
	
	memcpy(wfile, chars, sizeof(uint32_t) * (canvasW * canvasH));
	wfile += sizeof(uint32_t) * (canvasW * canvasH);
	
	memcpy(wfile, charsColors, sizeof(uint32_t) * (canvasW * canvasH) * 2);
	wfile += sizeof(uint32_t) * (canvasW * canvasH) * 2;
	
	SaveFileData(fileName.c_str(), file, size);
	
	free(file);
}

void Canvas::saveImage(std::string fileName) {
	//Сохранение содержимого холста
	Image tmp = LoadImageFromTexture(renderCanvas.texture);
	ExportImage(tmp, fileName.c_str());
	UnloadImage(tmp);
}