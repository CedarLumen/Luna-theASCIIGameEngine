#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <Windows.h>
#define PI 3.14159265358979323846
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define MAX_ELEMENTS 1000
#define MAX_SPRITES 1000

char SCREEN_CHAR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
struct RGBA {
	unsigned char r; unsigned char g; unsigned char b; unsigned char a;
};
RGBA SCREEN_COLOR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
struct Pixel { char c; RGBA color; };
template<typename T>
struct Dot2D { T x; T y; };

int SPRITE_COUNT = 0;
struct Sprite {
	std::string name = "default";
	unsigned int id = SPRITE_COUNT;
	bool isVisible = 1;
	Dot2D<int> position = { 0, 0 };
	Dot2D<int> collisionBox = { 16, 16 };
	Dot2D<int> pivot = { 0, 0 };
	Dot2D<int> collisionPivot = { 0, 0 };
	float rotation = 0.0f;
	int width = 16;
	int height = 16;
	Pixel* pixels = new Pixel[width * height];
	Sprite() {
		SPRITE_COUNT++;
	}
	~Sprite() {
		delete[] pixels;
	}
	void setPixel(int x, int y, char c, RGBA color) {
		if (x < 0 || x >= width || y < 0 || y >= height) {
			std::cerr << "Error: Pixel coordinates out of bounds" << std::endl;
			return;
		}
		pixels[y * width + x] = { c, color };
	}
	void setPixel(int index, char c, RGBA color) {
		if (index < 0 || index >= width * height) {
			std::cerr << "Error: Pixel index out of bounds" << std::endl;
			return;
		}
		pixels[index] = { c, color };
	}
	void blitToScreen() {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int screenX = position.x + x;
				int screenY = position.y + y;
				if (screenX < 0 || screenX >= SCREEN_WIDTH || screenY < 0 || screenY >= SCREEN_HEIGHT) {
					continue; // Skip pixels outside the screen bounds
				}
				Pixel& pixel = pixels[y * width + x];
				if (pixel.color.a == 0) {
					continue; // Skip transparent pixels
				}
				SCREEN_CHAR_BUFFER[screenY][screenX] = pixel.c;
				SCREEN_COLOR_BUFFER[screenY][screenX] = pixel.color;
			}
		}
	}
};

HANDLE hConsole;

void loadPicture(const std::string& filename, Sprite& sprite) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}
	std::string line;
	size_t index = 0;
	const size_t total = static_cast<size_t>(sprite.width) * sprite.height;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		if (line[0] == '#') continue;
		std::istringstream iss(line);
		char c;
		int r, g, b, a;
		if (!(iss >> c >> r >> g >> b >> a)) {
			std::cerr << "Error: Invalid line format in file " << filename << std::endl;
			continue;
		}
		if (r < 0 || r > 255 || g < 0 || g > 255 ||
			b < 0 || b > 255 || a < 0 || a > 255) {
			std::cerr << "Error: Color value out of range in file " << filename << std::endl;
			continue;
		}
		if (index >= total) {
			std::cerr << "Error: Too many pixels in file " << filename << std::endl;
			break;
		}
		sprite.pixels[index] = {
			c,
			{
				static_cast<unsigned char>(r),
				static_cast<unsigned char>(g),
				static_cast<unsigned char>(b),
				static_cast<unsigned char>(a)
			}
		};
		++index;
	}
	if (index < total) {
		std::cerr << "Warning: Not enough pixels in file " << filename
			<< ", loaded " << index << " / " << total << std::endl;
	}
}

void clearScreen() {
	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			SCREEN_CHAR_BUFFER[y][x] = ' ';
			SCREEN_COLOR_BUFFER[y][x] = { 0, 0, 0, 0 };
		}
	}
}

// ============================================================
// 真彩色渲染：整帧拼成一个字符串，只调用一次 WriteConsoleA
// 要点：
//   1. 帧首 \033[H 让光标归位（不清屏，不清屏就不会闪）
//   2. 行尾 \033[0m 重置颜色 + \033[K 擦掉该行残留
//   3. 最后一行绝不输出 \n，避免整屏向上滚动
// ============================================================
void renderScreen() {
	static std::string out;
	out.clear();
	out.reserve(SCREEN_HEIGHT * SCREEN_WIDTH * 24 + 64);

	out += "\033[H";

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		int lr = -1, lg = -1, lb = -1;
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			RGBA c = SCREEN_COLOR_BUFFER[y][x];
			char ch = SCREEN_CHAR_BUFFER[y][x];
			if (c.a == 0) {
				out += ' ';
				continue;
			}
			if (c.r != lr || c.g != lg || c.b != lb) {
				out += "\033[38;2;";
				out += std::to_string(c.r); out += ';';
				out += std::to_string(c.g); out += ';';
				out += std::to_string(c.b); out += 'm';
				lr = c.r; lg = c.g; lb = c.b;
			}
			out += ch;
		}
		out += "\033[0m\033[K";
		if (y + 1 < SCREEN_HEIGHT) out += '\n';
	}


	DWORD written = 0;
	WriteConsoleA(hConsole, out.data(), (DWORD)out.size(), &written, NULL);
}


void initConsole() {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


	DWORD mode = 0;
	GetConsoleMode(hConsole, &mode);
	SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);


	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);


	DWORD w = 0;
	const char* reset = "\033[2J\033[H";
	WriteConsoleA(hConsole, reset, (DWORD)strlen(reset), &w, NULL);
}

Sprite sprites[MAX_SPRITES];
int spriteCount = 0;
int addSprite(std::string name) {
	if (spriteCount >= MAX_SPRITES) {
		std::cerr << "Error: Maximum number of sprites reached" << std::endl;
		return -1;
	}
	sprites[spriteCount].name = name;
	return spriteCount++;
}
int removeSprite(int id) {
	if (id < 0 || id >= spriteCount) {
		std::cerr << "Error: Invalid sprite ID" << std::endl;
		return -1;
	}
	for (int i = id; i < spriteCount - 1; ++i) {
		sprites[i] = sprites[i + 1];
	}
	spriteCount--;
	return 0;
}
int blitSprite(int id) {
	if (id < 0 || id >= spriteCount) {
		std::cerr << "Error: Invalid sprite ID: " << id << std::endl;
		return -1;
	}
	if (sprites[id].isVisible) {
		sprites[id].blitToScreen();
	}
	return 0;
}

int main() {
	initConsole();

	int iddog = addSprite("dog");
	loadPicture("dog.pix", sprites[iddog]);
	int idcat = addSprite("cat");
	loadPicture("cat.pix", sprites[idcat]);

	while (true) {
		system("cls");
		clearScreen();
		blitSprite(iddog);
		blitSprite(idcat);
		renderScreen();                   
		Sleep(16);                               
	}

	return 0;
}