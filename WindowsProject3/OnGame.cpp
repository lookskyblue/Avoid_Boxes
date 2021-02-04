#include <iostream>
#include <time.h>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include "OnGame.h"

#define CHAR_X_LEN 10
#define CHAR_Y_LEN 10
#define CHAR_SPEED 10
#define STAR_X_LEN 5
#define STAR_Y_LEN 80
#define MAX_STAR 15
#define STAR_SPEED 10
#define BOARD_OFFSET_X 100
#define BOARD_OFFSET_Y 100
#define BOARD_X_LEN 300
#define BOARD_Y_LEN 400
#define REGAME 0

class Obj
{
private:
	int x;
	int y;
	int speed;
public:
	Obj() = default;
	Obj(int x, int y, int speed)
		: x(x)
		, y(y)
		, speed(speed) {}
	virtual ~Obj() = default;

	void SetInfo(int x, int y, int speed)
	{
		this->x = x;
		this->y = y;
		this->speed = speed;
	}

	int GetX() { return x; }
	int GetY() { return y; }
	int GetSpeed() { return speed; }
	void SetY(int y) { this->y = y; }
	void SetX(int x) { this->x = x; }
};

class Character : public Obj
{
public:
	Character(int x, int y, int speed)
		: Obj(x, y, speed)
	{}

	~Character() = default;
};

class Star : public Obj
{
private:
	bool life;

public:
	Star(int x, int y, int speed)
		: Obj(x, y, speed)
		, life(true)
	{}

	void DownStar(int x) { SetY(x); }

	bool GetLife() { return life; }

	void SetLife(bool life) { this->life = life; }

	void SetAll(int x, int y, int speed, bool life)
	{
		SetInfo(x, y, speed);
		SetLife(life);
	}
};

Obj* avatar = nullptr;
std::vector<Star> stars;
std::chrono::system_clock::time_point StartTime;
std::chrono::system_clock::time_point EndTime;
std::chrono::seconds sec;
TCHAR ElapsedTime[20];
RECT rt = { BOARD_OFFSET_X, BOARD_OFFSET_Y - 20, BOARD_OFFSET_X + BOARD_X_LEN, BOARD_OFFSET_Y + BOARD_Y_LEN };

void OnCreate(HWND hWnd)
{
	avatar = new Character(BOARD_X_LEN / 2 + BOARD_OFFSET_X, BOARD_Y_LEN + BOARD_OFFSET_Y - 20, CHAR_SPEED);
	stars.reserve(MAX_STAR);

	for (int i = 0; i < MAX_STAR; i++)
		stars.push_back(Star(BOARD_OFFSET_X + rand() % (BOARD_X_LEN), 0 + BOARD_OFFSET_Y, STAR_SPEED));

	SetTimer(hWnd, 0, 100, NULL);
	srand(time_t(NULL));

	StartTime = std::chrono::system_clock::now();
}

void OnPaint(HDC hdc)
{
	EndTime = std::chrono::system_clock::now();
	sec = std::chrono::duration_cast<std::chrono::seconds>(EndTime - StartTime);

	int avatarX = avatar->GetX();
	int avatarY = avatar->GetY();

	Rectangle(hdc, BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_OFFSET_X + BOARD_X_LEN, BOARD_OFFSET_Y + BOARD_Y_LEN);
	Ellipse(hdc, avatarX, avatarY, avatarX + CHAR_X_LEN, avatarY + CHAR_Y_LEN);

	wsprintf(ElapsedTime, L"Elapsed Time: %d", sec.count()); // sec.count 반환형이 int64 같은데 출력 자료형은 어떻게..? ld, lld도 안돼..
	TextOut(hdc, BOARD_OFFSET_X + 100, BOARD_OFFSET_Y - 20, ElapsedTime, lstrlen(ElapsedTime));

	for (int i = 0; i < MAX_STAR; i++)
	{
		if (stars[i].GetLife())
		{
			int x = stars[i].GetX();
			int y = stars[i].GetY();

			Rectangle(hdc, x, y, x + STAR_X_LEN, y + STAR_Y_LEN);
		}
	}
}

void OnKeyDown(HWND hWnd, WPARAM wParam)
{
	int avatarX = avatar->GetX();
	int avatarSpeed = avatar->GetSpeed();

	switch (wParam)
	{
	case VK_LEFT:
	{
		if (BOARD_OFFSET_X < avatarX - avatarSpeed)
			avatar->SetX(avatarX - avatarSpeed);
		break;
	}

	case VK_RIGHT:
	{
		if (avatarX + CHAR_X_LEN + avatarSpeed < BOARD_OFFSET_X + BOARD_X_LEN)
			avatar->SetX(avatarX + avatarSpeed);
		break;
	}

	default:
		break;
	}

	InvalidateRect(hWnd, &rt, TRUE);
}

void OnTimer(HWND hWnd, WPARAM wParam)
{
	MakeStar();
	MoveStar();
	InvalidateRect(hWnd, &rt, TRUE);
	Collision(hWnd);
}

void ResetGame(HWND hWnd)
{
	if (avatar)
		delete avatar;

	KillTimer(hWnd, 0);
	stars.erase(stars.begin(), stars.end());
}

void Collision(HWND hWnd)
{
	int avatarX = avatar->GetX();
	int avatarY = avatar->GetY();

	for (int i = 0; i < MAX_STAR; i++)
	{
		if (stars[i].GetLife())
		{ // test1dd
			if ((avatarX < stars[i].GetX() + STAR_X_LEN) &&
				(avatarY < stars[i].GetY() + STAR_Y_LEN) &&
				(avatarX + CHAR_X_LEN > stars[i].GetX()) &&
				(avatarY + CHAR_Y_LEN > stars[i].GetY()))
			{
				KillTimer(hWnd, 0);

				if (MessageBox(hWnd, L"Game Over\r\n Again?", L"Avoid Box", MB_YESNO | MB_ICONHAND)
					== IDYES)
				{
					ResetGame(hWnd);
					SendMessageW(hWnd, WM_COMMAND, REGAME, NULL);

					return;
				}

				else
				{
					if (avatar)
						delete avatar;

					exit(0);
				}
			}
		}
	}
}

void MakeStar()
{
	for (int i = 0; i < MAX_STAR; i++)
	{
		if (stars[i].GetLife() == false)
		{
			stars[i].SetAll(rand() % 250 + BOARD_OFFSET_X, 0 + BOARD_OFFSET_Y, STAR_SPEED, true);
		}
	}
}

void MoveStar()
{
	for (int i = 0; i < MAX_STAR; i++)
	{
		if (BOARD_OFFSET_Y + BOARD_Y_LEN < stars[i].GetY())
			stars[i].SetLife(false);

		if (stars[i].GetLife() == true)
		{
			int speed = stars[i].GetSpeed();
			int y = stars[i].GetY();

			stars[i].DownStar(y + speed);
		}
	}
}