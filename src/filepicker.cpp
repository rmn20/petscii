#include <iostream>
#include <locale>

#include "filepicker.h"

FilePicker::FilePicker(
	std::shared_ptr<Screen> prevScr, 
	std::function<void(FilePicker* picker, std::string)> onSelected, 
	bool saveHere,
	std::string path,
	std::string formatFilter
):
	prevScr(prevScr),
	onSelected(onSelected),
	saveHere(saveHere),
	formatFilter(formatFilter) 
{
	setDirectory(path);
}

FilePicker::~FilePicker() {
	prevScr = nullptr;
}

//Для проверки формата
bool endsWith(std::string str, std::string substr) {
	if(substr.size() == 0) return true;
	if(substr.size() > str.size()) return false;
	
	std::locale loc;
	
    return std::equal(substr.rbegin(), substr.rend(), str.rbegin());
}

void FilePicker::setDirectory(std::string path) {
	this->path = path;
	
	list.removeAll();
	
	TextItem* item = new TextItem("Cancel");
	item->onEnterf = [&](ListItem* item) {
		Main::get().setScreen(prevScr);
	};
	list.add(item);
	list.addVoid();
	
	//Кнопка для сохранения файла в текущей директории
	if(saveHere) {
		item = new TextItem("Save here");
		item->onEnterf = [&](ListItem* item) {
			setNewFileDialog();
		};
		list.add(item);
		list.addVoid();
	}
	
	//Предыдущая директория
	item = new File((std::string) "..", this, std::string(GetPrevDirectoryPath(path.c_str())));
	list.add(item);
	
	//Читаем список файлов
	int count = 0;
	char** files = GetDirectoryFiles(path.c_str(), &count);
	
	for(int i=0; i<count; i++) {
		std::string name = std::string(files[i]);
		if(name == ".." || name == ".") continue; //raylib why
		
		//Путь к файлу, является ли файл директорией
		std::string nextPath = path + "\\" + name;
		bool isDirectory = DirectoryExists(nextPath.c_str());
		
		//Пропускаем файлы, который не подходят под фильтр форматов
		if(!isDirectory && !endsWith(nextPath, formatFilter)) continue;
		
		item = new File(name, this, nextPath);
		//Загрузка файла, если он не является папкой
		if(!isDirectory) {
			item->onEnterf = [&](ListItem* item) {
				selFilePath = ((File*) list.getSelectedItem())->getPath();
				
				if(saveHere) setOverwriteDialog();
				else if(this->onSelected != nullptr) {
					this->onSelected(this, this->selFilePath);
				}
			};
		}
		list.add(item);
	}
	
    ClearDirectoryFiles();
}

void FilePicker::setOverwriteDialog() {
	std::string fName = ((File*) list.getSelectedItem())->getText();
	selFilePath = ((File*) list.getSelectedItem())->getPath();
	
	list.removeAll();
	
	TextItem* item = new TextItem("Overwrite " + fName + "?");
	item->skip = true;
	list.add(item);
	list.addVoid();
	
	item = new TextItem("Yes");
	item->onEnterf = [&](ListItem* item) {
		if(this->onSelected != nullptr) this->onSelected(this, this->selFilePath);
	};
	list.add(item);
	
	item = new TextItem("No");
	item->onEnterf = [&](ListItem* item) {
		this->setDirectory(this->path);
	};
	list.add(item);
}

void FilePicker::setNewFileDialog() {
	list.removeAll();
	TextItem* item = new TextItem("Please follow the instructions in the console.");
	item->skip = true;
	list.add(item);
	
	consoleStep = 1; //Диалог в консоли
}

void FilePicker::tick() {
	Vector2 listSize = list.getSize();
	
	list.mouseUpdate({0, 0}, GetMousePosition());
	list.keysUpdate();
	
	//Обновление размера списка под размер экрана
	if(listSize.x != GetScreenWidth() || listSize.y != GetScreenHeight()) {
		list.setSize({(float) GetScreenWidth(), (float) GetScreenHeight()}, 32);
	}
	
	//Отрисовка всего
	BeginDrawing();
	ClearBackground(BLACK);
	
	list.render({0, 0});
	
	EndDrawing();
	
	//Сначала нужно отрисовать 1 кадр, и только потом просить ввод с консоли
	if(consoleStep >= 0) {
		if(consoleStep == 2) {
			std::cout << "Enter the file name: ";
			
			std::string fileName;
			std::cin >> fileName;
			
			selFilePath = fileName;
			
			if(formatFilter.size() > 0 && !endsWith(selFilePath, formatFilter)) selFilePath += formatFilter;
			
			if(this->onSelected != nullptr) this->onSelected(this, this->selFilePath);
		} else consoleStep++;
	}
}