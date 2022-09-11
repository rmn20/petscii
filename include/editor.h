#ifndef EDITOR_H
#define EDITOR_H

#include "raylib.h"

#include "main.h"
#include "canvas.h"
#include "charset.h"
#include "palette.h"
#include "screen.h"

#include "itemList/itemlist.h"
#include "filepicker.h"
#include "projectcreator.h"
#include "itemList/textitem.h"

enum EditorMode {CANVAS = 0, CHARSET_SELECTOR, PALETTE_SELECTOR, CHARSET_PALETTE_COMBINED};

class Editor: public Screen {
	private:
		std::shared_ptr<Charset> charset;
		std::shared_ptr<Palette> palette;
		std::string projectPath = "";
		std::string projectName = "";
		
		//Холст
		Canvas canvas;
		//-1 = правый/нижний край холста в центре экрана
		//1  = левый/верхний край холста в центре экрана
		Vector2 canvasPos = {0, 0};
		float canvasScale = 1;
		
		//Рисование
		uint32_t selectedChar = 0;
		uint32_t bckColor = 0, col = 0;
		bool wasPainting = false; //Для undo/redo
		
		//Режимы (холст, экран выбора символа, экран палитры, комбинированный...)
		EditorMode mode = CANVAS;
		EditorMode nextMode = mode; //Смена режима происходит в начале каждого кадра
		
		//Экран выбора символа/экран палитры
		Rectangle charPickerRect;
		Rectangle paletteRect;
		bool selectorOpenedBySpace;
		bool paletteBckColorMode; //Менять не цвета кисти, а цвет фона холста
		
		//Кнопка экрана выбора символа + цветов
		Rectangle selectorButton;
		bool selectorButtonPressed = false;
		
		//Кнопка меню
		Texture menuTex;
		Rectangle menuButtonRect;
		bool menuButtonPressed = false;
		
		//ItemList для меню (и не только)
		ItemList* list = nullptr;
		
		ItemList* nextList; //Смена списка происходит в начале каждого кадра
		bool changeList = false;
		
		//Функции
		//Смена режима экрана
		void setMode(EditorMode nextMode, bool openedBySpace, bool bckColorMode) {
			this->nextMode = nextMode; 
			this->selectorOpenedBySpace = openedBySpace;
			this->paletteBckColorMode = bckColorMode;
		}
		
		//Смена списка
		void setList(ItemList* list) {
			nextList = list; 
			changeList = true;
		}
		
		void showMenu();
		
		void updateCanvas();
		//Работа с экраном выбора символа / экраном палитры
		void updateCharPicker();
		void updatePalette();
		
		void render();
		void renderCharPicker();
		void renderPalette();
		
	public:
		Editor();
		Editor(const Editor &Editor) = delete;
		void operator=(const Editor &Editor) = delete;
		~Editor();
		
		void tick();
};

#endif