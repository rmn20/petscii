#ifndef PROJECTCREATOR_H
#define PROJECTCREATOR_H

#include <memory>

#include "raylib.h"

#include "main.h"
#include "screen.h"
#include "canvas.h"
#include "itemList/itemlist.h"
#include "itemList/textitem.h"

class ProjectCreator: public Screen {
	private:
		std::shared_ptr<Screen> prevScr;
		Canvas* canvas;
		
		ItemList list;
		
		int consoleStep = -1; //Ввод с консоли должен работать только после отрисовки хотя бы 1 кадра на экране
		
	public:
	
		ProjectCreator(
			std::shared_ptr<Screen> prevScr, 
			Canvas* canvas
		):
			prevScr(prevScr),
			canvas(canvas)
		{}
		
		ProjectCreator(const ProjectCreator &ProjectCreator) = delete;
		void operator=(const ProjectCreator &ProjectCreator) = delete;
		~ProjectCreator();
		
		void show();
		void tick();
};
#endif