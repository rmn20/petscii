#ifndef MAIN_H
#define MAIN_H

#include <memory>

#include "screen.h"
#include "editor.h"

#include "itemList/itemlist.h"
#include "itemList/textitem.h"

//Менеджер экранов
class Main {
	private:
	bool run = true;
	
	std::shared_ptr<Screen> scr;
	std::shared_ptr<Screen> nextScr;
	
	Main();
	~Main();
	public:
	static Main& get() {
		 static Main s;
		 return s;
	}
	
	void setScreen(std::shared_ptr<Screen> scr);
	std::shared_ptr<Screen> getScreen();
	void stop();
	void tick();
};

#endif