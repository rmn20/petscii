#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <vector>
#include <string>

#include "raylib.h"

#include "itemList/listitem.h"
#include "itemList/textitem.h"

class ItemList {
	private:
		//Для работы мыши
		Vector2 prevMousePos = {-1, -1};
		int itemPressed = -1;
		
		//Размеры
		Vector2 size = {1, 1};
		int fontHeight = 1;
		float elementsHeight = 0;
		
		//Индекс выделенного элемента, верхний элемент на экране, скролл
		int index = 0, topIndex = 0;
		float yScroll = 0;
		
		//Центрирование
		bool vCenter = false;
		
		std::vector<ListItem*> items;
		
		//Обновить размеры элементов в соответствии с размером области списка и высотой шрифта
		void updateElementsSize();
		//Дейстие по выделению элемента
		void itemSelected();
		
		//Скролл на -1 или +1
		void scroll(int dIndex);
		//Скролл к выделенному элементу
		void scrollToCurrentIndex();
		//Ограничить скролл в соответствии с размером области и высотой элементов списка
		void limitScroll();
		//Отцентрировать элементы по Y
		void centralize();
		
	public:
		ItemList() {}
		ItemList(const ItemList &ItemList) = delete;
		void operator=(const ItemList &ItemList) = delete;
		~ItemList() {removeAll();}
		
		//Установить размер области списка и высоту шрифта
		void setSize(Vector2 size, int fontHeight);
		void setVCenter(bool vCenter) {this->vCenter = vCenter;}
		
		//Удалить все элементы из списка
		void removeAll();
		//Добавить пустой элемент-разделитель
		void addVoid();
		//Добавить элемент в конец списка
		void add(ListItem* item);
		
		void render(Vector2 listPos);
		
		//Попадает ли vec в область списка (с позицией pos)
		bool isInBox(Vector2 listPos, Vector2 vec);
		//Попадает ли vec в область элементов списка (с позицией pos)
		bool isPointsElements(Vector2 listPos, Vector2 vec);
		//Чтение действий мыши
		void mouseUpdate(Vector2 listPos, Vector2 mousePos);
		//Чтение нажатий клавиш
		void keysUpdate();
		
		Vector2 getSize() {return size;}
		float getElementsHeight() {return elementsHeight;}
		ListItem* getSelectedItem() {return index>0?items[index]:nullptr;}
};

#endif