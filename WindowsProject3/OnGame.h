#pragma once

void OnCreate(HWND hWnd);

void OnPaint(HWND hWnd, HBITMAP* MyBitMap);

void OnKeyDown(HWND hWnd, WPARAM wParam);

void OnTimer(HWND hWnd, WPARAM wParam);

void MakeStar();
void MoveStar();
void Collision(HWND hWnd);
void ResetGame(HWND hWnd);