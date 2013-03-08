#pragma once

#include "resource.h"

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib,"Gdiplus.lib")

#include <mmsystem.h> // PlaySound
#pragma comment(lib, "winmm.lib")

#include <Vfw.h> // MCIWnd*
#pragma comment(lib, "Vfw32.lib")

#include <time.h> // time
#include <Shellapi.h> // CommandLineToArgvW

// Key setting for player 1
#define P1_MOVETOLEFT	VK_LEFT
#define P1_MOVETORIGHT	VK_RIGHT
#define P1_MOVETODOWN	VK_DOWN
#define P1_TRANSFORM	VK_UP
#define P1_DROPDOWN		VK_SPACE

// Basic setting
#define IMAGE_SIZE		32									// Size of the image
#define BLOCK_COUNT		9									// Block count
#define MAX_SUB_BLOCK	5									// Max sub block in all blocks
#define MARGIN			2									// Margin of the each box
#define DELTA_WIDTH		6									// Delta width of window
#define DELTA_HEIGHT	28									// Delta height of window
#define DIFFICULTY_MIN	1									// Min difficulty
#define DIFFICULTY_MAX	20									// Max difficulty
#define SPEED_MIN		1									// Min speed
#define SPEED_MAX		20									// Max speed
#define SPEED_DELTA		50									// Delta of speed change
#define TIMER_1			1									// Timer use for update frame
#define TIMER_2			2									// Timer use for make block auto down
#define EMPTY_BLOCK		-1									// Empty block flag
#define INVALID_OFFSET	-1									// Invalid offset flag
#define HIDE_ROW		1									// Hide row use for make space for transform

typedef struct {
	int					row;
	int					col;
} BLOCK;

// Global Variables:
int						FPS = 30;							// FPS
int						ROW = 20;							// Row of the container
int						COL = 10;							// Col of the container
unsigned int			frame = 0;							// Frame that had drawn
bool					bIsActive;							// When the window is active windows,this set to TRUE
bool					bSoundEnable = TRUE;				// If TRUE,sound is enable
bool					bShowGridline = TRUE;				// If TRUE,show gridline
bool					bGameRunning = FALSE;				// Is the game running?
bool					bGamePaused = FALSE;				// Is the game paused?
bool					bGameOver = FALSE;					// Show the game is over or not
int						container[64][64];					// Define the max size of container
int						curBlock;							// Current block id(from 0 to BLOCK_COUNT-1)
int						curDirection;						// Direction of the current block(from 0 to 3)
int						nextBlock = -1;						// Next block id(from 0 to BLOCK_COUNT-1)
int						nextDirection;						// Direction of the next block(from 0 to 3)
int						difficulty = 1;						// Current difficulty of game
int						speed = 1;							// Current speed of game
int						usedBlockNum = 0;					// Block number that used 
int						deletedRowNum = 0;					// Row number that deleted
int						totalScore = 0;						// Game total tmpScore
int						tmpScore = 0;						// Score of current level
BLOCK					ref;								// Reference of current block
int						rowMoved;							// Row moved
int						colMoved;							// Col moved
BLOCK					curSubBlock[MAX_SUB_BLOCK];			// curSubBlock[0]~curSubBlock[MAX_SUB_BLOCK] store the each sub block's position of current block
BLOCK					tmpSubBlock[MAX_SUB_BLOCK];			// Used for detect block can tranform
BLOCK					nextSubBlock[MAX_SUB_BLOCK];		// nextSubBlock[0]~nextSubBlock[MAX_SUB_BLOCK] store the reference position of the next block

#define MAX_LOADSTRING	100

HINSTANCE				hInst;
TCHAR					szTitle[MAX_LOADSTRING];
TCHAR					szWindowClass[MAX_LOADSTRING];
HWND					hWndMain;
HWND					hWndCMI;

// Foward declarations of functions included in this code module:
ATOM					MyRegisterClass(HINSTANCE hInstance);
BOOL					InitInstance(HINSTANCE, int);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT	CALLBACK		Setting(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK		About(HWND, UINT, WPARAM, LPARAM);

void					UpdateFrame(void);
void					StartGame(void);
void					PauseGame(void);
void					ContinueGame(void);
void					RestartGame(void);
void					UpdateBlockPos(int, int, BLOCK[], bool);
void					GenNextBlock(void);
bool					CanMove(int);
void					Transform(void);

int RandInt(int min,int max)
{
	return rand() % (max - min + 1) + min;
}

void Init(void)
{
	memset(&container, EMPTY_BLOCK, sizeof(container));
	for (int i = 0; i < MAX_SUB_BLOCK; i++) {
		curSubBlock[i].row = INVALID_OFFSET;
	}
}

void UpdateTimer(void)
{
	SetTimer(hWndMain, TIMER_2, (SPEED_MAX + 1 - speed) * SPEED_DELTA, NULL);
}

void PlaySoundEx(LPCWSTR filename, BOOL async, BOOL loop)
{
	if (!bSoundEnable) {
		return;
	}
	DWORD flag = SND_FILENAME | SND_NODEFAULT;
	if (async) {
		flag |= SND_ASYNC;
	}
	if (loop) {
		if (hWndCMI) {
			MCIWndDestroy(hWndCMI);
		}
		hWndCMI = MCIWndCreate(NULL, NULL, WS_MINIMIZE | MCIWNDF_NOERRORDLG, filename);
		MCIWndSetRepeat(hWndCMI,TRUE);
		MCIWndPlay(hWndCMI);
		return;
	}
	PlaySound(filename, NULL, flag);
}

void CenterInParent(HWND parent, HWND child)
{
	RECT rect1, rect2;
	GetWindowRect(parent, &rect1);
	GetWindowRect(child, &rect2);
	SetWindowPos(child, HWND_TOPMOST, (rect1.right + rect1.left) / 2 - (rect2.right - rect2.left) / 2, (rect1.bottom + rect1.top) / 2 - (rect2.bottom - rect2.top) / 2, 0, 0, SWP_NOSIZE);
}

void MoveToLeft()
{
	if (!bGameRunning || bGamePaused || bGameOver) {
		return;
	}
	if (!CanMove(0)) {
		PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
		return;
	}
	colMoved -= 1;
	PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
}

void MoveToRight()
{
	if (!bGameRunning || bGamePaused || bGameOver) {
		return;
	}
	if (!CanMove(1)) {
		PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
		return;
	}
	colMoved += 1;
	PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
}

void MoveToDown()
{
	if (!bGameRunning || bGamePaused || bGameOver) {
		return;
	}
	if (!CanMove(2)) {
		PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
		return;
	}
	rowMoved += 1;
	PlaySoundEx(L"Res\\Sound\\btn.wav", true, false);
}

void DoTransform()
{
	if (!bGameRunning || bGamePaused || bGameOver) {
		return;
	}
	Transform();
	PlaySoundEx(L"Res\\Sound\\transform.wav", true, false);
}

void DoDropDown()
{
	if (!bGameRunning || bGamePaused || bGameOver) {
		return;
	}
	while (CanMove(2)) {
		rowMoved += 1;
	}
	PlaySoundEx(L"Res\\Sound\\fixup.wav", true, false);
	GenNextBlock();
}

BLOCK SetBlockRefPos(int rowOffset, int colOffset)
{
	BLOCK block;
	block.row = rowOffset;
	block.col = colOffset;
	return block;
}

bool CanMove(int direction)
{
	int deltaRow = 0;
	int deltaCol = 0;
	switch (direction) {
	case 0:		// move to left
		deltaCol = -1;
		break;
	case 1:		// move to right
		deltaCol = 1;
		break;
	case 2:		// move to down
		deltaRow = 1;
		break;
	default:
		return FALSE;
	}
	for (int i = 0; i < MAX_SUB_BLOCK; ++i) {
		if (curSubBlock[i].row != INVALID_OFFSET) {
			int row = curSubBlock[i].row + rowMoved + deltaRow;
			if (row < 0 || row > ROW - 1) {
				return FALSE;
			}
			int col = curSubBlock[i].col + colMoved + deltaCol;
			if (col < 0 || col > COL - 1) {
				return FALSE;
			}
			if (container[row][col] != EMPTY_BLOCK) {
				return FALSE;
			}
		}
	}
	return TRUE;
}