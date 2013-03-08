#include "stdafx.h"
#include "Tetris.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    // Process arguments
    LPWSTR *argv;
    int argc;
    argv = CommandLineToArgvW(lpCmdLine, &argc);
    if (argv != NULL) {
        if (argc == 1 && wcscmp(argv[0], L"/?") == 0) {
            MessageBox(NULL, L"Usage: Tetris.exe <ROW> <COL> <FPS>", L"Tetris", MB_OK | MB_ICONINFORMATION);
            LocalFree(argv);
            return 0;
        } else if (argc >= 3) {
            size_t cCharsConverted;
            CHAR strTmp[256]; 
            wcstombs_s(&cCharsConverted, strTmp, sizeof(strTmp), (const wchar_t *)argv[0], sizeof(strTmp));
            ROW = min(64, max(15, atoi(strTmp)));
            wcstombs_s(&cCharsConverted, strTmp, sizeof(strTmp), (const wchar_t *)argv[1], sizeof(strTmp));
            COL = min(64, max(10, atoi(strTmp)));
            wcstombs_s(&cCharsConverted, strTmp, sizeof(strTmp), (const wchar_t *)argv[1], sizeof(strTmp));
            FPS = min(100, max(10, atoi(strTmp)));
        }
        LocalFree(argv);
    }

    ROW += HIDE_ROW;

    Init();

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TETRIS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TETRIS));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    HCURSOR hCursor = LoadCursorFromFile(L"Res\\Arrow.ani");

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETRIS));
    wcex.hCursor = (hCursor ? hCursor : LoadCursor(NULL, IDC_ARROW));
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;    //MAKEINTRESOURCE(IDC_TETRIS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    int width, height, left, top;

    hInst = hInstance; // Store instance handle in our global variable

    width = COL * IMAGE_SIZE + MAX_SUB_BLOCK * IMAGE_SIZE + MARGIN * 3 + DELTA_WIDTH;
    height = (ROW - HIDE_ROW) * IMAGE_SIZE + MARGIN * 2 + DELTA_HEIGHT;
    left = max(0, GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2);
    top = max(0, GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2);

    hWnd = CreateWindowEx(WS_EX_TOPMOST, szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        left, top, width, height, NULL, NULL, hInstance, NULL);
    
    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd, TIMER_1, 1000 / FPS, NULL);

    hWndMain = hWnd;

    return TRUE;
}

// Mesage handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        CenterInParent(hWndMain, hDlg);
        return (INT_PTR) TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR) TRUE;
        }
        break;
    }
    return (INT_PTR) FALSE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND    - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY    - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_ACTIVATEAPP:
        bIsActive = wParam != 0;
        if (bGameRunning) {
            if (wParam == 0) {
                PauseGame();
            } else {
                ContinueGame();
            }
        }
        break;
    case WM_SIZE:
        if (bGameRunning) {
            if (wParam == SIZE_MINIMIZED) {
                PauseGame();
            } else {
                ContinueGame();
            }
        }
        break;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_KEYDOWN:
        switch (wParam) {
        case P1_MOVETOLEFT:
            MoveToLeft();
            break;
        case P1_MOVETORIGHT:
            MoveToRight();
            break;
        case P1_MOVETODOWN:
            MoveToDown();
            break;
        case P1_TRANSFORM:
            DoTransform();
            break;
        case P1_DROPDOWN:
            DoDropDown();
            break;
        case VK_F1:
            RestartGame();
            break;
        case VK_F2:
            StartGame();
            break;
        case VK_F3:
            if (speed > SPEED_MIN) {
                --speed;
            } else {
                speed = SPEED_MIN;
            }
            UpdateTimer();
            break;
        case VK_F4:
            if (speed < SPEED_MAX) {
                ++speed;
            } else {
                speed = SPEED_MAX;
            }
            UpdateTimer();
            break;
        case VK_F5:
            if (difficulty > DIFFICULTY_MIN) {
                --difficulty;
            } else {
                difficulty = DIFFICULTY_MIN;
            }
            break;
        case VK_F6:
            if (difficulty < DIFFICULTY_MAX) {
                ++difficulty;
            } else {
                difficulty = DIFFICULTY_MAX;
            }
            break;
        case VK_F7:
            bSoundEnable = !bSoundEnable;
            if (!bSoundEnable) {
                PlaySound(NULL, NULL, NULL);
                if (hWndCMI) {
                    MCIWndPause(hWndCMI);
                }
            } else {
                if (hWndCMI) {
                    MCIWndResume(hWndCMI);
                }
            }
            break;
        case VK_F8:
            bShowGridline = !bShowGridline;
            break;
        case VK_F9:
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        case VK_F12:
            DialogBox(hInst, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) About);
            break;
        }
        break;
    case WM_TIMER:
        switch (wParam) {
        case TIMER_1:
            UpdateFrame();
            break;
        case TIMER_2:
            if (bGameRunning && !bGamePaused) {
                if (CanMove(2)) {
                    rowMoved += 1;
                } else {
                    GenNextBlock();
                }
            }
            break;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        //RECT rt;
        //GetClientRect(hWnd, &rt);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ShowGameView(void)
{
    // create a bitmap
    Bitmap bmp(COL * IMAGE_SIZE, ROW * IMAGE_SIZE);
    Graphics bmpGraphics(&bmp);
    bmpGraphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // draw background
    SolidBrush bkBrush(Color(10, 100, 10));
    bmpGraphics.FillRectangle(&bkBrush, 0, 0, COL * IMAGE_SIZE, ROW * IMAGE_SIZE);

    // draw gridline
    if (bShowGridline) {
        Pen pen(Color(0, 128, 128));
        for (int row = 1; row < ROW; ++row) {
            bmpGraphics.DrawLine(&pen, Point(0, row * IMAGE_SIZE), Point(COL * IMAGE_SIZE, row * IMAGE_SIZE));
        }
        for (int col = 1; col < COL; ++col) {
            bmpGraphics.DrawLine(&pen, Point(col * IMAGE_SIZE, 0), Point(col * IMAGE_SIZE, ROW * IMAGE_SIZE));
        }
    }

    // draw image
    Image image(L"Res\\Image.jpg");
    for (int i = HIDE_ROW; i < ROW; i++) {
        for (int j = 0; j < COL; ++j) {
            if (container[i][j] == EMPTY_BLOCK) { // empty block
                continue;
            }
            TextureBrush tBrush(&image, Rect(container[i][j] * IMAGE_SIZE, 0, IMAGE_SIZE, IMAGE_SIZE));
            bmpGraphics.FillRectangle(&tBrush, j * IMAGE_SIZE, (i - HIDE_ROW) * IMAGE_SIZE, IMAGE_SIZE, IMAGE_SIZE);
        }
    }

    for (int i = 0; i < MAX_SUB_BLOCK; ++i) {
        if (curSubBlock[i].row != INVALID_OFFSET) {
            int row = curSubBlock[i].row + rowMoved;
            int col = curSubBlock[i].col + colMoved;
            TextureBrush tBrush(&image, Rect(curBlock * IMAGE_SIZE, 0, IMAGE_SIZE, IMAGE_SIZE));
            bmpGraphics.FillRectangle(&tBrush, col * IMAGE_SIZE, (row - HIDE_ROW) * IMAGE_SIZE, IMAGE_SIZE, IMAGE_SIZE);
        }
    }

    // draw cache bitmap
    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    CachedBitmap cachedBmp(&bmp, &graphics);
    graphics.DrawCachedBitmap(&cachedBmp, MARGIN, MARGIN);
    ReleaseDC(hWndMain, hdc);
}

void ShowGameNext(void)
{
    UpdateBlockPos(nextBlock, nextDirection, nextSubBlock, false);

    int left = COL * IMAGE_SIZE + MARGIN * 2;
    int top = MARGIN;
    int width = MAX_SUB_BLOCK * IMAGE_SIZE;
    int height = MAX_SUB_BLOCK * IMAGE_SIZE;

    // create a bitmap
    Bitmap bmp(width, height);
    Graphics bmpGraphics(&bmp);
    bmpGraphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // draw background
    SolidBrush bkBrush(Color(243, 151, 1));
    bmpGraphics.FillRectangle(&bkBrush, 0, 0, width, height);

    // draw gridline
    if (bShowGridline) {
        Pen pen(Color(128, 128, 128));
        for (int row = 1; row < MAX_SUB_BLOCK; ++row) {
            bmpGraphics.DrawLine(&pen, Point(row * IMAGE_SIZE, 0), Point(row * IMAGE_SIZE, ROW * IMAGE_SIZE));
        }
        for (int col = 1; col < MAX_SUB_BLOCK; ++col) {
            bmpGraphics.DrawLine(&pen, Point(0, col * IMAGE_SIZE), Point(COL * IMAGE_SIZE, col * IMAGE_SIZE));
        }
    }

    Image image(L"Res\\Image.jpg");
    for (int i = 0; i < MAX_SUB_BLOCK; i++) {
        if (nextSubBlock[i].row == INVALID_OFFSET) {
            continue;
        }
        TextureBrush tBrush(&image, Rect(nextBlock * IMAGE_SIZE, 0, IMAGE_SIZE, IMAGE_SIZE));
        bmpGraphics.FillRectangle(&tBrush,
            nextSubBlock[i].col * IMAGE_SIZE, nextSubBlock[i].row * IMAGE_SIZE, IMAGE_SIZE, IMAGE_SIZE);
    }
    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    CachedBitmap cachedBmp(&bmp, &graphics);
    graphics.DrawCachedBitmap(&cachedBmp, left, top);
    ReleaseDC(hWndMain, hdc);
}

void ShowGameStat(void)
{
    // calc position
    int width = MAX_SUB_BLOCK * IMAGE_SIZE;
    int height = MAX_SUB_BLOCK * IMAGE_SIZE;
    int left = MARGIN + COL * IMAGE_SIZE + MARGIN;
    int top = MAX_SUB_BLOCK * IMAGE_SIZE + MARGIN * 2;

    // create a bitmap
    Bitmap bmp(width, height);
    Graphics bmpGraphics(&bmp);
    bmpGraphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // draw background
    SolidBrush bgBrush(Color(159, 79, 151));
    bmpGraphics.FillRectangle(&bgBrush, 0, 0, width, height);

    // init font
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
    PointF pointF(0, 5);
    SolidBrush fontBrush(Color(255, 255, 255, 255));

    // string buffer
    WCHAR buffer[256] = { 0 };

    // draw string
    wsprintf(buffer, L"速度: %06d", speed);
    bmpGraphics.DrawString(buffer, -1, &font, pointF, &fontBrush);
    wsprintf(buffer, L"难度: %06d", difficulty);
    pointF.Y += 30;
    bmpGraphics.DrawString(buffer, -1, &font, pointF, &fontBrush);
    wsprintf(buffer, L"块数: %06d", usedBlockNum);
    pointF.Y += 30;
    bmpGraphics.DrawString(buffer, -1, &font, pointF, &fontBrush);
    wsprintf(buffer, L"行数: %06d", deletedRowNum);
    pointF.Y += 30;
    bmpGraphics.DrawString(buffer, -1, &font, pointF, &fontBrush);
    wsprintf(buffer, L"分数: %06d", totalScore);
    pointF.Y += 30;
    bmpGraphics.DrawString(buffer, -1, &font, pointF, &fontBrush);

    // draw cache bitmap
    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    CachedBitmap cachedBmp(&bmp, &graphics);
    graphics.DrawCachedBitmap(&cachedBmp, left, top);
    ReleaseDC(hWndMain, hdc);
}

void ShowGameHelp(void)
{
    // calc position
    int left = COL * IMAGE_SIZE + MARGIN * 2;
    int top = MAX_SUB_BLOCK * IMAGE_SIZE * 2 + MARGIN * 3;
    int width = MAX_SUB_BLOCK * IMAGE_SIZE;
    int height = ROW * IMAGE_SIZE + MARGIN - top;

    // create a bitmap
    Bitmap bmp(width, height);
    Graphics bmpGraphics(&bmp);
    bmpGraphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // draw background
    SolidBrush bgBrush(Color(3, 171, 234));
    bmpGraphics.FillRectangle(&bgBrush, 0, 0, width, height);

    // init font
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 23, FontStyleBold, UnitPixel);
    PointF pointF(0, 24);
    SolidBrush fontBrush(Color(255, 255, 255, 255));

    // draw string
    bmpGraphics.DrawString(L"F1=重玩", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F2=开始/暂停", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F3=难度↓", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F4=难度↑", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F5=速度↓", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F6=速度↑", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F7=音效开/关", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F8=辅助线", -1, &font, pointF, &fontBrush);
    pointF.Y += 30;
    bmpGraphics.DrawString(L"F9=退出", -1, &font, pointF, &fontBrush);

    // draw cache bitmap
    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    CachedBitmap cachedBmp(&bmp, &graphics);
    graphics.DrawCachedBitmap(&cachedBmp, left, top);
    ReleaseDC(hWndMain, hdc);
}

void ShowGamePaused(void)
{
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 36, FontStyleBold, UnitPixel);
    PointF pointF((REAL)(MARGIN + (COL * IMAGE_SIZE - 36 * 4.5) / 2), (REAL)((MARGIN + ROW * IMAGE_SIZE - 36) / 2));
    SolidBrush txtBrush(Color(255, 255, 0, 0));

    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    graphics.DrawString(L"PAUSED", -1, &font, pointF, &txtBrush);
    ReleaseDC(hWndMain, hdc);
}

void ShowGameOver(void)
{
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 36, FontStyleBold, UnitPixel);
    PointF pointF((REAL)(MARGIN + (COL * IMAGE_SIZE - 36 * 6.6) / 2), (REAL)((MARGIN + ROW * IMAGE_SIZE - 36) / 2));
    SolidBrush txtBrush(Color(255, 255, 0, 0));

    HDC hdc = GetDC(hWndMain);
    Graphics graphics(hdc);
    graphics.DrawString(L"GAME OVER", -1, &font, pointF, &txtBrush);
    ReleaseDC(hWndMain, hdc);
}

void UpdateFrame(void)
{
    if (bGameRunning && bGamePaused) {
        ShowGamePaused();
        return;
    }
    if (++frame >= UINT_MAX) {
        frame = 0;
    }
    ShowGameView();
    if (bGameOver) {
        ShowCursor(TRUE);
        ShowGameOver();
        if (bSoundEnable && hWndCMI) {
            MCIWndStop(hWndCMI);
        }
        if (bGameRunning) {
            bGameRunning = FALSE;
            PlaySoundEx(L"Res\\Sound\\lost.wav", true, false);
        }
        return;
    }
    switch(frame % 3) {
    case 0:
        ShowGameNext();
        break;
    case 1:
        ShowGameStat();
        break;
    case 2:
        ShowGameHelp();
        break;
    }
}

void StartGame(void)
{
    if (bGameRunning) {
        if (bGamePaused) {
            ContinueGame();
        } else {
            PauseGame();
        }
        return;
    }
    bGameRunning = TRUE;
    bGamePaused = FALSE;
    bGameOver = FALSE;
    difficulty = 1;
    speed = 1;
    usedBlockNum = 0;
    deletedRowNum = 0;
    totalScore = 0;
    tmpScore = 0;
    ShowCursor(FALSE);
    srand((int)time(NULL));
    PlaySoundEx(L"Res\\Sound\\ReadyGo.wav", false, false);
    Init();
    GenNextBlock();
    UpdateTimer();
    PlaySoundEx(L"Res\\Sound\\back.wav", true, true);
}

void PauseGame(void)
{
    bGamePaused = TRUE;
    ShowCursor(TRUE);
    if (bSoundEnable && hWndCMI) {
        MCIWndPause(hWndCMI);
    }
}

void ContinueGame(void)
{
    bGamePaused = FALSE;
    ShowCursor(FALSE);
    if (bSoundEnable && hWndCMI) {
        MCIWndResume(hWndCMI);
    }
}

void RestartGame(void)
{
    if (bGameRunning || bGameOver) {
        bGameRunning = FALSE;
        StartGame();
    }
}

void EraseFullLine(void)
{
    int i, j, k, m, n;
    bool bCanErase;
    int lastDelRow = deletedRowNum;
    int topRow = 0;
    for (i = ROW - 1; i >= topRow; --i) {
        bCanErase = TRUE;
        for (j = 0; j < COL; ++j) {
            if (container[i][j] == EMPTY_BLOCK) {
                bCanErase = FALSE;
                break;
            }
        }
        if (bCanErase) {
            bool bIsEmpty = false;
            for (m = i; m >= 1; --m) {
                bIsEmpty = true;
                for (n = 0; n < COL; ++n) {
                    container[m][n] = container[m - 1][n];
                    if (bIsEmpty && container[m - 1][n] != EMPTY_BLOCK) {
                        bIsEmpty = false;
                    }
                }
                if (bIsEmpty) {
                    topRow = m + 1;
                    break;
                }
            }
            if (!bIsEmpty) {
                for (k = 0; k < COL; ++k) {
                    container[0][k] = EMPTY_BLOCK;
                }
            }
            ++i;
            ++deletedRowNum;
        }
    }

    // calc score
    int scoreArr[] = { 0, 100, 300, 500, 800 };
    int row = deletedRowNum - lastDelRow;
    switch (row) {
    case 1:
    case 2:
    case 3:
    case 4:
        tmpScore += scoreArr[row];
        totalScore += scoreArr[row];
        PlaySoundEx(L"Res\\Sound\\fadelayer.wav", true, false);
        break;
    }

    if (tmpScore >= 5000) {
        tmpScore = 0;
        ++speed;
        UpdateTimer();
    }
}

void UpdateBlockPos(int block, int direction, BLOCK blockArr[], bool isCur)
{
    for (int i = 0; i < MAX_SUB_BLOCK; ++i) {
        blockArr[i].row = INVALID_OFFSET;
    }

    //　０１２３－－＞COL
    //０口田田田
    //１田田田田
    //２田田田田
    //３田田田田
    //｜
    //∨
    //ROW
    //POS=(ROW, COL)
    switch (block) {
    case 0:        // 田
        blockArr[0] = SetBlockRefPos(0, 1);
        blockArr[1] = SetBlockRefPos(0, 2);
        blockArr[2] = SetBlockRefPos(1, 1);
        blockArr[3] = SetBlockRefPos(1, 2);
        break;
    case 1:        // 亠
        switch (direction) {
        case 0:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(1, 3);
            break;
        case 1:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(1, 3);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 2:
            blockArr[0] = SetBlockRefPos(1, 1);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(1, 3);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 3:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        }
        break;
    case 2:        // 倒Z
        switch (direction) {
        case 0:
        case 2:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 1:
        case 3:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(0, 3);
            blockArr[2] = SetBlockRefPos(1, 1);
            blockArr[3] = SetBlockRefPos(1, 2);
            break;
        }
        break;
    case 3:        // Z
        switch (direction) {
        case 0:
        case 2:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(2, 1);
            break;
        case 1:
        case 3:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(0, 2);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(1, 3);
            break;
        }
        break;
    case 4:        // L
        switch (direction) {
        case 0:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(0, 2);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 1:
            blockArr[0] = SetBlockRefPos(0, 3);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(1, 3);
            break;
        case 2:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(2, 2);
            blockArr[3] = SetBlockRefPos(2, 3);
            break;
        case 3:
            blockArr[0] = SetBlockRefPos(1, 1);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(1, 3);
            blockArr[3] = SetBlockRefPos(2, 1);
            break;
        }
        break;
    case 5:        // 倒L
        switch (direction) {
        case 0:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(2, 1);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 1:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(1, 3);
            break;
        case 2:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(0, 3);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(2, 2);
            break;
        case 3:
            blockArr[0] = SetBlockRefPos(1, 1);
            blockArr[1] = SetBlockRefPos(1, 2);
            blockArr[2] = SetBlockRefPos(1, 3);
            blockArr[3] = SetBlockRefPos(2, 3);
            break;
        }
        break;
    case 6:        // 一
        switch (direction) {
        case 0:
        case 2:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(2, 1);
            blockArr[3] = SetBlockRefPos(3, 1);
            break;
        case 1:
        case 3:
            blockArr[0] = SetBlockRefPos(1, 0);
            blockArr[1] = SetBlockRefPos(1, 1);
            blockArr[2] = SetBlockRefPos(1, 2);
            blockArr[3] = SetBlockRefPos(1, 3);
            break;
        }
        break;
    case 7:        // 十
        blockArr[0] = SetBlockRefPos(0, 2);
        blockArr[1] = SetBlockRefPos(1, 1);
        blockArr[2] = SetBlockRefPos(1, 2);
        blockArr[3] = SetBlockRefPos(1, 3);
        blockArr[4] = SetBlockRefPos(2, 2);
        break;
    case 8:        // 凵
        switch (direction) {
        case 0:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(0, 2);
            blockArr[2] = SetBlockRefPos(0, 3);
            blockArr[3] = SetBlockRefPos(1, 1);
            blockArr[4] = SetBlockRefPos(1, 3);
            break;
        case 1:
            blockArr[0] = SetBlockRefPos(0, 2);
            blockArr[1] = SetBlockRefPos(0, 3);
            blockArr[2] = SetBlockRefPos(1, 3);
            blockArr[3] = SetBlockRefPos(2, 2);
            blockArr[4] = SetBlockRefPos(2, 3);
            break;
        case 2:
            blockArr[0] = SetBlockRefPos(1, 1);
            blockArr[1] = SetBlockRefPos(1, 3);
            blockArr[2] = SetBlockRefPos(2, 1);
            blockArr[3] = SetBlockRefPos(2, 2);
            blockArr[4] = SetBlockRefPos(2, 3);
            break;
        case 3:
            blockArr[0] = SetBlockRefPos(0, 1);
            blockArr[1] = SetBlockRefPos(0, 2);
            blockArr[2] = SetBlockRefPos(1, 1);
            blockArr[3] = SetBlockRefPos(2, 1);
            blockArr[4] = SetBlockRefPos(2, 2);
            break;
        }
        break;
    }

    if (!isCur) {
        return;
    }

    for (int i = 0; i < MAX_SUB_BLOCK; ++i) {
        if (blockArr[i].row != INVALID_OFFSET) {
            blockArr[i].col += ref.col;
            blockArr[i].row += ref.row;
        }
    }
}

void GenNextBlock(void)
{
    // save block to container
    for (int i = 0; i < MAX_SUB_BLOCK; ++i) {
        if (curSubBlock[i].row != INVALID_OFFSET) {
            int row = curSubBlock[i].row + rowMoved;
            int col = curSubBlock[i].col + colMoved;
            container[row][col] = curBlock;
        }
    }

    // erase the full line before random next block
    EraseFullLine();

    // set cur block
    int blockCount = (difficulty > DIFFICULTY_MIN) ? BLOCK_COUNT : BLOCK_COUNT - 2;
    if (usedBlockNum == 0) {
        curBlock = RandInt(1, 10000) % blockCount;
        curDirection = RandInt(0, 3);
    } else {
        curBlock = nextBlock;
        curDirection = nextDirection;
    }

    // rand next block
    nextBlock = RandInt(1, 10000) % blockCount;
    nextDirection = RandInt(0, 3);

    // reset global val
    ref.row = 0;
    ref.col = COL / 2 - 2;
    rowMoved = 0;
    colMoved = 0;

    UpdateBlockPos(curBlock, curDirection, curSubBlock, true);
    
    // Is game over?
    for (int i = 0; i < MAX_SUB_BLOCK; i++) {
        if (curSubBlock[i].row != INVALID_OFFSET) {
            if (container[curSubBlock[i].row][curSubBlock[i].col] != EMPTY_BLOCK) {
                bGameOver = TRUE;
                break;
            }
        }
    }
    ++usedBlockNum;
}

void Transform(void)
{
    int newDirection = curDirection + 1;
    if (newDirection >= 4) {
        newDirection = 0;
    }
    UpdateBlockPos(curBlock, newDirection, tmpSubBlock, true);
    for(int i = 0; i < MAX_SUB_BLOCK; ++i) {
        if(tmpSubBlock[i].row != INVALID_OFFSET) {
            int row = tmpSubBlock[i].row + rowMoved;
            if (row < 0 || row > ROW - 1) {
                return;
            }
            int col = tmpSubBlock[i].col + colMoved;
            if (col < 0 || col > COL - 1) {
                return;
            }
            if (container[row][col] != EMPTY_BLOCK) {
                return;
            }
        }
    }
    UpdateBlockPos(curBlock, newDirection, curSubBlock, true);
    curDirection = newDirection;
}