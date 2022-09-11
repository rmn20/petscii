#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>

#include "editor.h"

Editor::Editor() {
	//Инициализация редактора, набора символов, палитры
	menuTex = LoadTexture("res/menu.png");
	
	projectPath = std::string(GetWorkingDirectory());
	
	charset = std::make_shared<Charset>("res/jpetscii.png", 8, 8);
	selectedChar = 1;
	
	palette = std::make_shared<Palette>("res/bedroom.png");
	bckColor = palette->getSize();
	col = 3;
	
	//Инициализация холста
	canvas.set(charset, palette, 24, 16);
	canvas.setBackgroundColor(0);
	canvas.resetUndo();
	canvas.repaint();
}

Editor::~Editor() {
	if(list != nullptr) delete list;
	if(nextList != nullptr) delete nextList;
	
	UnloadTexture(menuTex);
}

void Editor::showMenu() {
	ItemList* list = new ItemList();
	
	TextItem* tmp = new TextItem("New project");
	tmp->onEnterf = [&](ListItem* item) {
		projectName = "";
		
		Main::get().setScreen(
			std::make_shared<ProjectCreator>(
				Main::get().getScreen(), 
				&canvas
			)
		);
		
		setList(nullptr); //Закрываем меню
	};
	list->add(tmp);
	
	tmp = new TextItem("Open project");
	tmp->onEnterf = [&](ListItem* item) {
		setList(nullptr); //Закрываем меню
		
		Main::get().setScreen(
			std::make_shared<FilePicker>(
				Main::get().getScreen(), 
				[&](FilePicker* picker, std::string file) {
					projectPath = std::string(GetDirectoryPath(file.c_str()));
					projectName = std::string(GetFileName(file.c_str()));
					
					canvas.load(file);
					Main::get().setScreen(picker->prevScr);
				}, 
				false, 
				projectPath, 
				".petscii"
			)
		);
	};
	list->add(tmp);
	tmp = new TextItem("Save project");
	tmp->onEnterf = [&](ListItem* item) {
		setList(nullptr); //Закрываем меню
		
		if(projectName.size() == 0) {
			Main::get().setScreen(
				std::make_shared<FilePicker>(
					Main::get().getScreen(), 
					[&](FilePicker* picker, std::string file) {
						projectPath = std::string(GetDirectoryPath(file.c_str()));
						projectName = std::string(GetFileName(file.c_str()));
						
						canvas.save(file);
						Main::get().setScreen(picker->prevScr);
					}, 
					true, 
					projectPath, 
					".petscii"
				)
			);
		} else {
			canvas.save(projectPath + "\\" + projectName);
		}
	};
	list->add(tmp);
	tmp = new TextItem("Save as");
	tmp->onEnterf = [&](ListItem* item) {
		setList(nullptr); //Закрываем меню
		
		Main::get().setScreen(
			std::make_shared<FilePicker>(
				Main::get().getScreen(), 
				[&](FilePicker* picker, std::string file) {
					projectPath = std::string(GetDirectoryPath(file.c_str()));
					projectName = std::string(GetFileName(file.c_str()));
					
					canvas.save(file);
					Main::get().setScreen(picker->prevScr);
				}, 
				true, 
				projectPath, 
				".petscii"
			)
		);
	};
	list->add(tmp);
	
	//list->addVoid();
	
	//list->add(new TextItem("Import image"));
	tmp = new TextItem("Export image");
	tmp->onEnterf = [&](ListItem* item) {
		setList(nullptr); //Закрываем меню
		
		Main::get().setScreen(
			std::make_shared<FilePicker>(
				Main::get().getScreen(), 
				[&](FilePicker* picker, std::string file) {
					canvas.saveImage(file);
					Main::get().setScreen(picker->prevScr);
				}, 
				true, 
				projectPath, 
				".png"
			)
		);
	};
	list->add(tmp);
	list->addVoid();
	
	tmp = new TextItem("Change background color");
	tmp->onEnterf = [&](ListItem* item) {
		setMode(PALETTE_SELECTOR, false, true);
		setList(nullptr); //Закрываем меню
	};
	list->add(tmp);
	
	/*list->add(new TextItem("Change canvas size"));
	list->add(new TextItem("Change charset"));
	list->add(new TextItem("Change palette"));*/
	
	list->addVoid();
	
	tmp = new TextItem("Exit");
	tmp->onEnterf = [&](ListItem* item) {
		Main::get().stop();
	};
	list->add(tmp);
	
	setList(list);
}

void Editor::tick() {
	//Обновление списка
	if(changeList) {
		if(list != nullptr) delete list;
		list = nextList;
		changeList = false;
	}
	
	//Если открыт список
	if(list) {
		Vector2 listSize = list->getSize();
		
		//Обновление размера списка под размер экрана
		if(listSize.x != GetScreenWidth() || listSize.y != GetScreenHeight()) {
			list->setSize({(float) GetScreenWidth(), (float) GetScreenHeight()}, 32);
		}
		
		list->mouseUpdate({0, 0}, GetMousePosition());
		list->keysUpdate();
		
		//Закрытие списка
		if(IsKeyPressed(KEY_ESCAPE) || 
			(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !list->isPointsElements({0, 0},  GetMousePosition()))
		) {
			setList(nullptr);
		}
	} else {
		//Смена режима экрана
		mode = nextMode;
		
		//Обновление экрана холста
		if(mode == CANVAS) {
			updateCanvas();
		}
		
		//Обновление экрана выбора символа + цвета
		if(mode != CANVAS) {
			//Закрывание по отпусканию пробела / нажатию ESC
			if(IsKeyReleased(KEY_SPACE) || (IsKeyReleased(KEY_ESCAPE) && !selectorOpenedBySpace)) {
				setMode(CANVAS, false, false);
			}
			
			//Заканчиваем рисовать (экран выбора можно открыть прямо во время рисовать при помощи пробела)
			if(wasPainting) {
				wasPainting = false;
				canvas.storeUndo();
			}
			
			//Обновление
			if(mode == CHARSET_SELECTOR || mode == CHARSET_PALETTE_COMBINED) updateCharPicker();
			if(mode == PALETTE_SELECTOR || mode == CHARSET_PALETTE_COMBINED) updatePalette();
		}
		
		//Меняем выбранные цвета местами
		if(IsKeyPressed(KEY_W)) {
			uint32_t tmp = bckColor;
			bckColor = col;
			col = tmp;
		}
	}
	
	render();
}

void Editor::updateCanvas() {
	//Открытие экрана выбора символа + цвета по пробелу
	if(IsKeyPressed(KEY_SPACE)) {
		setMode(CHARSET_PALETTE_COMBINED, true, false);
	}
	
	//Кнопка экрана выбора символа + цвета
	float charScale = std::max(1.0f, std::round(GetScreenHeight() / 15.0f / charset->charH));
	float charBtW = charScale * charset->charW;
	float charBtH = charScale * charset->charH;
	
	selectorButton = (Rectangle) {
		charBtW * 0.5f,
		GetScreenHeight() - charBtH * 1.5f,
		charBtW,
		charBtH
	};
	
	if(CheckCollisionPointRec(GetMousePosition(), selectorButton)) {
		
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selectorButtonPressed = true;
			
		} else if(selectorButtonPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			selectorButtonPressed = false;
			setMode(CHARSET_PALETTE_COMBINED, false, false);
			
		}
	}
	
	//Кнопка меню
	float menuButtonScale = std::max(1.0f, std::round(GetScreenHeight() / 15.0f / menuTex.height));
	
	menuButtonRect = (Rectangle) {
		menuTex.width * menuButtonScale * 0.5f,
		menuTex.height * menuButtonScale * 0.5f,
		menuTex.width * menuButtonScale,
		menuTex.height * menuButtonScale
	};
	
	if(CheckCollisionPointRec(GetMousePosition(), menuButtonRect)) {
		
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			menuButtonPressed = true;
			
		} else if(menuButtonPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			menuButtonPressed = false;
			showMenu();
			
		}
	}
	
	if(menuButtonPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		menuButtonPressed = false;
	}
	
	//Зум холста
	if(GetMouseWheelMove() != 0) {
		float pw = pow(1.5, GetMouseWheelMove());
		canvasScale *= pw;
		
		//переводим мыш в нашу систему координат
		float tmp = (GetMouseX() - GetScreenWidth() / 2) / canvasScale / GetScreenHeight() * canvas.getRenderHeight() / canvas.getRenderWidth() * 2;
		canvasPos.x = canvasPos.x - tmp + tmp / pw;
		
		//переводим мыш в нашу систему координат
		tmp = (GetMouseY() - GetScreenHeight() / 2) / canvasScale / GetScreenHeight() * 2;
		canvasPos.y = canvasPos.y - tmp + tmp / pw;
		
		canvasPos.x = std::max(-1.f, std::min(1.f, canvasPos.x));
		canvasPos.y = std::max(-1.f, std::min(1.f, canvasPos.y));
	}
	
	//Перемещение холста
	if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
		Vector2 mouseDelta = GetMouseDelta();
		
		canvasPos.x += mouseDelta.x / canvasScale / GetScreenHeight() * canvas.getRenderHeight() / canvas.getRenderWidth() * 2;
		canvasPos.y += mouseDelta.y / canvasScale / GetScreenHeight() * 2;
		
		canvasPos.x = std::max(-1.f, std::min(1.f, canvasPos.x));
		canvasPos.y = std::max(-1.f, std::min(1.f, canvasPos.y));
	}
	
	//Притягивание к pixel perfect масштабу
	/*{
		float pixelScale = canvasScale * GetScreenHeight() / canvas.getRenderHeight();
		float pixelPerfectScale = round(pixelScale);
		
		if(pixelScale > 0.5 && std::abs(pixelPerfectScale - pixelScale) <= 0.1) {
			canvasScale += (pixelPerfectScale - pixelScale) / pow(2, std::min(std::max(GetFrameTime(), 0.01f), 1.0f) * 60) * canvas.getRenderHeight() / GetScreenHeight();
		}
	}*/
	
	//Притягивание к масштабу, при котором холст влезает к окно
	{
		float minimalScale = std::min(1.0, 1.0 / canvas.getRenderWidth() * canvas.getRenderHeight() / GetScreenHeight() * GetScreenWidth());
		
		if(std::abs(minimalScale - canvasScale) < 0.1) {
			canvasScale += (minimalScale - canvasScale) / pow(2.0, std::min(std::max(GetFrameTime(), 0.01f), 1.0f) * 60);
		}
	}
	
	//Кисть
	bool canPaint = !selectorButtonPressed && !menuButtonPressed;

	float viewCanvasScale = canvasScale * GetScreenHeight() / canvas.getRenderHeight();
	
	Vector2 canvasScrPos = {
		GetScreenWidth() / 2.0f + (canvasPos.x / 2 - 0.5f) * viewCanvasScale * canvas.getRenderWidth(),
		GetScreenHeight() / 2.0f + (canvasPos.y / 2 - 0.5f) * viewCanvasScale * canvas.getRenderHeight()
	};
	
	size_t charX = (size_t) ((GetMouseX() - canvasScrPos.x) / viewCanvasScale) / charset->charW;
	size_t charY = (size_t) ((GetMouseY() - canvasScrPos.y) / viewCanvasScale) / charset->charH;

	if(canPaint && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && charX < canvas.getWidth() && charY < canvas.getHeight()) {
		wasPainting = true;
		canvas.setChar(charX, charY, selectedChar, bckColor, col);
		canvas.repaintChar(charX, charY);
	} else if(wasPainting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		wasPainting = false;
		canvas.storeUndo();
	}
	
	//Undo + redo
	if(IsKeyDown(KEY_LEFT_CONTROL)) {
		if(IsKeyPressed(KEY_Z) && canvas.canUndo()) {
			canvas.undo();
			canvas.repaint();
		} else if(IsKeyPressed(KEY_Y) && canvas.canRedo()) {
			canvas.redo();
			canvas.repaint();
		}
	}

	//Пипетка
	if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && charX < canvas.getWidth() && charY < canvas.getHeight()) {
		selectedChar = canvas.getChar(charX, charY);
		
		uint32_t* charColors = canvas.getCharColors(charX, charY);
		bckColor = *charColors;
		col = *(charColors + 1);
	}
}

void Editor::updateCharPicker() {
	//Размер набора символов
	float scale = std::min(
		GetScreenWidth() / (mode == CHARSET_PALETTE_COMBINED?2:1) / charset->getWidth(), 
		GetScreenHeight() / charset->getHeight()
	);
	
	//Область набора символов
	charPickerRect = {
		GetScreenWidth() / (mode == CHARSET_PALETTE_COMBINED?4:2) - charset->getWidth() * scale / 2,
		GetScreenHeight() / 2 - charset->getHeight() * scale / 2,
		charset->getWidth() * scale,
		charset->getHeight() * scale
	};
	
	//Выбор символа
	if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), charPickerRect)) {
		selectedChar = 
			(int) ((GetMouseX() - charPickerRect.x) / scale / charset->charW) + 
			(int) ((GetMouseY() - charPickerRect.y) / scale / charset->charH) * charset->charsX;
		
		if(!selectorOpenedBySpace) {
			setMode(CANVAS, false, false);
			
			//Оставляем окно, если нажат space
			if(IsKeyDown(KEY_SPACE)) {
				setMode(mode, true, paletteBckColorMode);
			}
		}
	}
}

void Editor::updatePalette() {
	//Размер палитры
	float scale = std::min(
		GetScreenWidth() / (mode == CHARSET_PALETTE_COMBINED?2:1) / palette->getWidth(), 
		GetScreenHeight() / (palette->getHeight() + (paletteBckColorMode?0:1))
	);
	
	//Область палитры
	paletteRect = {
		GetScreenWidth() * (mode == CHARSET_PALETTE_COMBINED?0.75f:0.5f) - palette->getWidth() * scale / 2,
		GetScreenHeight() / 2 - (palette->getHeight() + (paletteBckColorMode?0:1)) * scale / 2,
		palette->getWidth() * scale,
		(palette->getHeight() + (paletteBckColorMode?0:1)) * scale
	};
	
	if((IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (!paletteBckColorMode && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))) 
		&& CheckCollisionPointRec(GetMousePosition(), paletteRect)) {
		
		uint32_t selectedCol = 
			(int) ((GetMouseX() - paletteRect.x) / scale) + 
			(int) ((GetMouseY() - paletteRect.y) / scale) * palette->getWidth();
		
		
		if(!paletteBckColorMode && selectedCol <= palette->getSize()) {
			if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) col = selectedCol;
			else bckColor = selectedCol;
			
		} else if(paletteBckColorMode) {
			canvas.setBackgroundColor(selectedCol);
			canvas.repaint();
			canvas.storeUndo();
		}
		
		if(!selectorOpenedBySpace) {
			setMode(CANVAS, false, paletteBckColorMode);
			
			//Оставляем окно, если нажат space
			if(IsKeyDown(KEY_SPACE)) {
				setMode(mode, true, paletteBckColorMode);
			}
		}
	}
}

void Editor::render() {
	//Rendering
	float viewCanvasScale = canvasScale * GetScreenHeight() / canvas.getRenderHeight();
	
	Vector2 canvasScrPos = {
		GetScreenWidth() / 2.0f + (canvasPos.x / 2 - 0.5f) * viewCanvasScale * canvas.getRenderWidth(),
		GetScreenHeight() / 2.0f + (canvasPos.y / 2 - 0.5f) * viewCanvasScale * canvas.getRenderHeight()
	};
	
	BeginDrawing();
	ClearBackground(RAYWHITE);
	
	canvas.render(canvasScrPos, viewCanvasScale);
	
	//Символ под курсором
	size_t charX = (size_t) ((GetMouseX() - canvasScrPos.x) / viewCanvasScale) / charset->charW;
	size_t charY = (size_t) ((GetMouseY() - canvasScrPos.y) / viewCanvasScale) / charset->charH;
	
	if(list == nullptr && mode == CANVAS && charX < canvas.getWidth() && charY < canvas.getHeight()) {
		charset->renderChar(
			{
				canvasScrPos.x + (charX * charset->charW) * viewCanvasScale,
				canvasScrPos.y + (charY * charset->charH) * viewCanvasScale,
				charset->charW * viewCanvasScale,
				charset->charH * viewCanvasScale
			},
			selectedChar,
			palette->getColor(bckColor == palette->getSize() ? canvas.getBackgroundColor() : bckColor),
			palette->getColor(col == palette->getSize() ? canvas.getBackgroundColor() : col),
			false
		);
	}
	
	//Кнопка экрана выбора и кнопка меню
	if(list == nullptr && mode == CANVAS) {
		charset->renderChar(
			selectorButton,
			selectedChar,
			palette->getColor(bckColor == palette->getSize() ? canvas.getBackgroundColor() : bckColor),
			palette->getColor(col == palette->getSize() ? canvas.getBackgroundColor() : col),
			false
		);
		
		DrawTextureEx(
			menuTex, 
			{menuButtonRect.x, menuButtonRect.y}, 
			0, 
			menuButtonRect.height / menuTex.height, 
			{255, 255, 255, (CheckCollisionPointRec(GetMousePosition(), menuButtonRect)||menuButtonPressed)?255:128}
		);
	}
	
	//Экран выбора
	if(list == nullptr && (mode == CHARSET_SELECTOR || mode == PALETTE_SELECTOR || mode == CHARSET_PALETTE_COMBINED)) {
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) {0, 0, 0, 128});
		
		if(mode == CHARSET_SELECTOR || mode == CHARSET_PALETTE_COMBINED) renderCharPicker();
		if(mode == PALETTE_SELECTOR || mode == CHARSET_PALETTE_COMBINED) renderPalette();
	}
	
	//Список
	if(list != nullptr) {
		DrawRectangle(0, 0, list->getSize().x, list->getElementsHeight(), (Color) {0, 0, 0, 128});
		
		list->render({0, 0});
	}
	
	EndDrawing();
}

void Editor::renderCharPicker() {
	float scale = charPickerRect.width / charset->getWidth();
	Color borderCol = ColorFromHSV(0, 0, 0.5 + std::cos(GetTime() * 3.14) / 2);
	
	charset->render(charPickerRect, palette->getColor(bckColor), palette->getColor(col));
	
	//Выбранный символ
	DrawRectangleLinesEx(
		(Rectangle) {
			charPickerRect.x + (selectedChar % charset->charsX) * charset->charW * scale,
			charPickerRect.y + (selectedChar / charset->charsX) * charset->charH * scale,
			charset->charW * scale,
			charset->charH * scale
		}, 
		1, borderCol
	);
}

void Editor::renderPalette() {
	float scale = paletteRect.width / palette->getWidth();
	Color borderCol = ColorFromHSV(0, 0, 0.5 + std::cos(GetTime() * 3.14) / 2);
	
	//Сама палитра
	palette->render((Rectangle) {paletteRect.x, paletteRect.y, paletteRect.width, paletteRect.height - (paletteBckColorMode?0:scale)});
	
	uint32_t tmp = paletteBckColorMode ? canvas.getBackgroundColor() : bckColor;
	Rectangle bckColorRect = {
		paletteRect.x + (tmp % palette->getWidth()) * scale,
		paletteRect.y + (tmp / palette->getWidth()) * scale,
		scale,
		scale
	};
	
	if(!paletteBckColorMode) {
		//Клетка с прозрачным цветом
		Rectangle transpRect = {
			paletteRect.x,
			paletteRect.y + paletteRect.height - scale,
			scale,
			scale
		};
		
		DrawRectangleLinesEx(transpRect, 1, RED);
		DrawLine(
			transpRect.x, 
			transpRect.y, 
			transpRect.x + transpRect.width,
			transpRect.y + transpRect.height,
			RED
		);
		DrawLine(
			transpRect.x + transpRect.width, 
			transpRect.y, 
			transpRect.x,
			transpRect.y + transpRect.height,
			RED
		);
	}
	
	//Цвет фона
	DrawRectangleLinesEx(bckColorRect, 1, borderCol);
	DrawTextEx(
		GetFontDefault(),
		"B", 
		(Vector2) {
			bckColorRect.x + bckColorRect.width / 2 - MeasureTextEx(GetFontDefault(), "B", bckColorRect.height, 0).x / 2, 
			bckColorRect.y
		},
		bckColorRect.height,
		0,
		borderCol
	);
	
	if(paletteBckColorMode) return;
	
	//Цвет символа
	Rectangle colRect = {
		paletteRect.x + (col % palette->getWidth()) * scale,
		paletteRect.y + (col / palette->getWidth()) * scale,
		scale,
		scale
	};
	
	DrawRectangleLinesEx(colRect, 1, borderCol);
	DrawTextEx(
		GetFontDefault(),
		"F", 
		(Vector2) {
			colRect.x + colRect.width / 2 - MeasureTextEx(GetFontDefault(), "F", colRect.height, 0).x / 2, 
			colRect.y
		},
		colRect.height,
		0,
		borderCol
	);
}