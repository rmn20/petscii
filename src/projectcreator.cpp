#include <iostream>

#include "projectcreator.h"

ProjectCreator::~ProjectCreator() {
	prevScr = nullptr;
}

//Инициализации экрана при отображении на экране
void ProjectCreator::show() {
	list.removeAll();
	TextItem* item = new TextItem("Please follow the instructions in the console.");
	item->skip = true;
	list.add(item);
	
	consoleStep = 1;
}

void ProjectCreator::tick() {
	Vector2 listSize = list.getSize();
	
	list.mouseUpdate({0, 0}, GetMousePosition());
	list.keysUpdate();
	
	//Обновление размера списка под размер экрана
	if(listSize.x != GetScreenWidth() || listSize.y != GetScreenHeight()) {
		list.setSize({(float) GetScreenWidth(), (float) GetScreenHeight()}, 32);
	}
	
	//Отрисовка списка
	BeginDrawing();
	ClearBackground(BLACK);
	
	list.render({0, 0});
	
	EndDrawing();
	
	//Сначала нужно отрисовать 1 кадр, и только потом просить ввод с консоли
	if(consoleStep >= 0) {
		if(consoleStep == 2) {
			
			int32_t w, h;
			
			while(1) {
				std::cout << "Enter canvas width: ";
				std::cin >> w;
				
				if(w < 1) std::cout << "Canvas width must be greater than 1." << std::endl;
				else break;
			}
			
			while(1) {
				std::cout << "Enter canvas height: ";
				std::cin >> h;
				
				if(h < 1) std::cout << "Canvas height must be greater than 1." << std::endl;
				else break;
			}
			
			canvas->set(canvas->getCharset(), canvas->getPalette(), w, h);
			canvas->setBackgroundColor(0);
			canvas->resetUndo(); //сбрасываем предыдущие шаги
			canvas->repaint();
			
			Main::get().setScreen(prevScr);
			
		} else consoleStep++;
	}
}