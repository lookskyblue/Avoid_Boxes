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
#define STAR_DOWN_SPEED 1
#define MAX_STAR 30
#define FRAME 5
#define MAKE_STAR_SPEED 100
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
	int live_time;

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
	void SetLiveTime(int liveTime) { this->live_time = liveTime; }
	int GetLiveTime() { return live_time; }
};

class Character : public Obj
{
public:
	Character(int x, int y, int speed, int live_time)
		: Obj(x, y, speed)
	{
		SetLiveTime(live_time);
	}

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

	void DownStar(int speed) { SetY(speed + GetY()); }

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
std::chrono::seconds elapsed_time;
TCHAR ElapsedTime[20];
RECT rt = { BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_OFFSET_X + BOARD_X_LEN, BOARD_OFFSET_Y + BOARD_Y_LEN +15 };

void OnCreate(HWND hWnd)
{
	if (avatar == nullptr)
		avatar = new Character(BOARD_X_LEN / 2 + BOARD_OFFSET_X, BOARD_Y_LEN + BOARD_OFFSET_Y - 20, CHAR_SPEED, 0);
	
	stars.reserve(MAX_STAR);

	SetTimer(hWnd, 0, FRAME, NULL);
	SetTimer(hWnd, 1, MAKE_STAR_SPEED, NULL);

	srand(time_t(NULL));

	StartTime = std::chrono::system_clock::now();
}

void OnPaint(HWND hWnd, HBITMAP* MyBitMap)
{
	HDC hdc, hMemDC;
	RECT crt;
	HBITMAP OldBitMap;
	int avatarX, avatarY;

	hdc = GetDC(hWnd);
	GetClientRect(hWnd, &crt);
	
	if (*MyBitMap == nullptr)
	{
		*MyBitMap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}

	hMemDC = CreateCompatibleDC(hdc);
	OldBitMap = (HBITMAP)SelectObject(hMemDC, *MyBitMap);
	avatarX = avatar->GetX();
	avatarY = avatar->GetY();
	FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
	Rectangle(hMemDC, BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_OFFSET_X + BOARD_X_LEN, BOARD_OFFSET_Y + BOARD_Y_LEN);
	Ellipse(hMemDC, avatarX, avatarY, avatarX + CHAR_X_LEN, avatarY + CHAR_Y_LEN);

	EndTime = std::chrono::system_clock::now();
	elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(EndTime - StartTime);
	wsprintf(ElapsedTime, L"Elapsed Time: %d", elapsed_time.count()); // What's the int64 data type..
	TextOut(hMemDC, BOARD_OFFSET_X + 100, BOARD_OFFSET_Y + BOARD_Y_LEN, ElapsedTime, lstrlen(ElapsedTime));

	int x, y;
	for (int i = 0; i < stars.size(); i++)
	{
		if (stars[i].GetLife())
		{
			x = stars[i].GetX();
			y = stars[i].GetY();

			if (y <= BOARD_OFFSET_Y && (BOARD_OFFSET_Y <= y + STAR_Y_LEN))
				Rectangle(hMemDC, x, BOARD_OFFSET_Y, x + STAR_X_LEN, BOARD_OFFSET_Y + (y + STAR_Y_LEN - BOARD_OFFSET_Y));

			else if (y + STAR_Y_LEN >= BOARD_OFFSET_Y + BOARD_Y_LEN)
				Rectangle(hMemDC, x, y, x + STAR_X_LEN, BOARD_OFFSET_Y + BOARD_Y_LEN);

			else if (y > BOARD_OFFSET_Y && (y + STAR_Y_LEN < BOARD_OFFSET_Y + BOARD_Y_LEN))
				Rectangle(hMemDC, x, y, x + STAR_X_LEN, y + STAR_Y_LEN);
		}
	}

	SelectObject(hMemDC, OldBitMap);
	DeleteObject(OldBitMap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hdc);
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

	InvalidateRect(hWnd, &rt, FALSE);
	Collision(hWnd);
}

void OnTimer(HWND hWnd, WPARAM wParam)
{
	MoveStar();
	InvalidateRect(hWnd, &rt, FALSE);
	Collision(hWnd);
}

void ResetGame(HWND hWnd)
{
	if (avatar != nullptr)
	{
		delete avatar;
		avatar = nullptr;
	}

	stars.erase(stars.begin(), stars.end());
}

void Collision(HWND hWnd)
{
	int avatarX = avatar->GetX();
	int avatarY = avatar->GetY();

	for (int i = 0; i < stars.size(); i++)
	{
		if (stars[i].GetLife())
		{
			if ((avatarX < stars[i].GetX() + STAR_X_LEN) &&
				(avatarY < stars[i].GetY() + STAR_Y_LEN) &&
				(avatarX + CHAR_X_LEN > stars[i].GetX()) &&
				(avatarY + CHAR_Y_LEN > stars[i].GetY()))
			{
				KillTimer(hWnd, 0);

				if (MessageBox(hWnd, L"Game Over\r\n Again?", L"Avoid Boxes", MB_YESNO | MB_ICONHAND)
					== IDYES)
				{
					ResetGame(hWnd);
					SendMessageW(hWnd, WM_COMMAND, REGAME, NULL);

						return;
					}

				else
				{
					if (avatar != nullptr)
						delete avatar;

					SendMessageW(hWnd, WM_DESTROY, 0, 0);
				}
			}
		}
	}
}

void MakeStar()
{
	if (stars.size() < MAX_STAR)
	{
		stars.push_back(Star(BOARD_OFFSET_X + rand() % (BOARD_X_LEN), -BOARD_OFFSET_Y, STAR_DOWN_SPEED));

		return;
	}

	else
	{
		for (int i = 0; i < stars.size(); i++)
		{
			if (stars[i].GetLife() == false)
			{
				stars[i].SetAll(BOARD_OFFSET_X + rand() % (BOARD_X_LEN), -BOARD_OFFSET_Y, STAR_DOWN_SPEED, true);

				return;
			}
		}
	}
}

void MoveStar()
{
	int speed = 0;

	for (int i = 0; i < stars.size(); i++)
	{
		if (stars[i].GetLife() == true)
		{
			speed = stars[i].GetSpeed();
			speed += static_cast<int>(elapsed_time.count())/10;
			stars[i].DownStar(speed);
			
			if (BOARD_OFFSET_Y + BOARD_Y_LEN <= stars[i].GetY()) // A star out of board must die
			{
				stars[i].SetLife(false);
				continue;
			}
		}
	}
}