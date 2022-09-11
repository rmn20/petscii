#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <iostream>

#include "raylib.h"
#include "itemlist/listitem.h"

class TextItem : public ListItem {
	private:
		std::string text;
	public:
		TextItem(std::string text) {
			this->text = text;
		}
		
		void updateListSize(Vector2 size, int fontHeight) {
			height = fontHeight;
		};
		
		void render(Vector2 pos, Vector2 size, float yScroll, bool selected) {
			DrawText(text.data(), (int) pos.x, (int) (pos.y + y + yScroll), height, selected ? WHITE : ((Color) {255,255,255,128})); 
		};
		
		std::string getText() {return text;}
};

#endif