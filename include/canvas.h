#ifndef CANVAS_H
#define CANVAS_H

#include <memory>

#include "raylib.h"
#include "raymath.h"

#include "charset.h"
#include "palette.h"
#include "undoredo.h"

//Класс холста
//bckColor, chars, charsColors, canvasW, canvasH ограничены файлом проекта

class Canvas {
	private:
		std::shared_ptr<Charset> charset;
		std::shared_ptr<Palette> palette;
		
		//Хранилище для отмены
		UndoStorage undoStorage;
		
		//Цвет фона
		uint32_t bckColor;
		
		//Массивы с символами и их цветами
		uint32_t* chars = nullptr;
		uint32_t* charsColors = nullptr;
		
		//Размер холста
		uint32_t canvasW, canvasH;
		
		//Текстура холста во VRAM
		RenderTexture2D renderCanvas = {0};
		
		void loadFromUndoStep(UndoStep* step);
	public:
		
		//Функции
		Canvas() {}
		Canvas(const Canvas &Canvas) = delete;
		void operator=(const Canvas &Canvas) = delete;
		~Canvas();
		
		//Установить набор символов, палитру и размер холста
		void set(std::shared_ptr<Charset> charset, std::shared_ptr<Palette> palette, uint32_t w, uint32_t h);
		//Выгрузить холст из памяти
		void unload();
		
		std::shared_ptr<Charset> getCharset() {return charset;}
		std::shared_ptr<Palette> getPalette() {return palette;}
		
		void setBackgroundColor(uint32_t col);
		uint32_t getBackgroundColor() {return bckColor;}
		
		//Установить символ и цвета на (x,y), получить цвета символа на (x, y), получить символ на (x,y)
		void setChar(size_t x, size_t y, uint32_t ch, uint32_t bck, uint32_t col);
		uint32_t* getCharColors(size_t x, size_t y);
		uint32_t getChar(size_t x, size_t y);
		
		//Перерисовать символ на холсте в GPU
		void repaintChar(size_t x, size_t y);
		//Перерисовать весь холст в GPU
		void repaint();
		//Рендер холста на экране
		void render(Vector2 pos, float scale);
		
		uint32_t getWidth() {return canvasW;}
		uint32_t getHeight() {return canvasH;}
		
		size_t getRenderWidth() {return renderCanvas.texture.width;}
		size_t getRenderHeight() {return renderCanvas.texture.height;}
		
		void resetUndo(); //Удалить все шаги отмены (и сохранить текущее состояние)
		void storeUndo(); //Создать шаг отмены
		bool canUndo() {return undoStorage.canUndo();}
		bool canRedo() {return undoStorage.canRedo();}
		void undo();
		void redo();
		
		void load(std::string file);
		void save(std::string file);
		void saveImage(std::string file);
};

#endif