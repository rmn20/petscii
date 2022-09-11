#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <functional>
#include <memory>

#include "raylib.h"

#include "main.h"
#include "screen.h"
#include "itemList/itemlist.h"
#include "itemList/textitem.h"

class FilePicker: public Screen {
	private:
		std::function<void(FilePicker* picker, std::string)> onSelected = nullptr;
		
		std::string path; //текущая директория
		std::string selFilePath; //путь к выбранному файлу
		
		bool saveHere; //Добавить кнопку "сохранить здесь"
		ItemList list;
		
		int consoleStep = -1; //Ввод с консоли должен работать только после отрисовки хотя бы 1 кадра на экране
		
		void setOverwriteDialog(); //Диалог с вопросом о перезаписе файла
		void setNewFileDialog(); //Диалог создания нового файла
		
	public:
		std::shared_ptr<Screen> prevScr;
		std::string formatFilter = "";
	
		FilePicker(
			std::shared_ptr<Screen> prevScr, 
			std::function<void(FilePicker* picker, std::string)> onSelected, 
			bool saveHere, 
			std::string path,
			std::string formatFilter
		);
		
		FilePicker(const FilePicker &FilePicker) = delete;
		void operator=(const FilePicker &FilePicker) = delete;
		~FilePicker();
		
		void tick();
		
		//смена текущей директории
		void setDirectory(std::string path);
};

class File: public TextItem {
	private:
		FilePicker* picker;
		std::string path;
		
	public:
		File(std::string text, FilePicker* picker, std::string path): 
			TextItem(text),
			picker(picker),
			path(path)
		{}
		
		virtual void onEnter() {
			if(DirectoryExists(path.c_str())) picker->setDirectory(path);
			else if(onEnterf != nullptr) onEnterf(this);
		}
		
		std::string getPath() {return path;}
};

#endif