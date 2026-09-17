#pragma once
#include "luna_engine.cpp"

int main() {
	initConsole();

	/*
	// 文本精灵示例 // Text sprite example
	int idhud = addSprite("hud");
	sprites[idhud].position = { 2, 1 };
	sprites[idhud].setText(
		"=== Console Sprite Demo ===\n"
		"Dog at (10,10), Cat at (40,20)\n"
		"Press ESC to quit.",
		{ 255, 255, 0, 255 }              // 黄色文本
	);*/

	//======insert your code here========
	int idcard = addSprite("card");

	int idcard0 = addSprite("card0");
	loadPicture("card.pix", sprites[idcard0]);
	int idcard1 = addSprite("card1");
	loadPicture("card1.pix", sprites[idcard1]);
	int idcard2 = addSprite("card2");
	loadPicture("card2.pix", sprites[idcard2]);
	int count = 0;
	//=============endline===============
	while (true) {
		system("cls");// 有时，这会导致屏幕闪烁 //sometimes this causes screen flickering
		clearScreen();
		//======insert your code here========
		if (isKeyDown(VK_LEFT))  sprites[idcard].position.x--;   // 按住持续移动
		if (isKeyDown(VK_RIGHT)) sprites[idcard].position.x++;
		if (isKeyDown(VK_UP))    sprites[idcard].position.y--;
		if (isKeyDown(VK_DOWN))  sprites[idcard].position.y++;

		if (isKeyPressed('A')) { /* 按一次触发一次 */ }
		if (isKeyPressed(VK_SPACE)) { /* 空格 */ }

		if (count % 48 == 0) {
			transportPicture(idcard, idcard0);
		}
		else if (count % 48 == 16) {
			transportPicture(idcard, idcard1);
		}
		else if (count % 48 == 32) {
			transportPicture(idcard, idcard2);
		}
		blitSprite(idcard);
		count++;
		//=============endline===============
		renderScreen();
		Sleep(16);
	}

	return 0;
}