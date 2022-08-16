#include <windows.h>
#include <ctime>
#include "resource.h"
#include <fstream>
#include <sstream>
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE hInst;
LPCTSTR szWindowClass = "FifteenC";
LPCTSTR szTitle = "FIFTEEN";
HBITMAP numBitmap = NULL;
HBITMAP emptyBitmap = NULL;
char buf[10];
int currentScore = 0;

static LOGFONT lf;
static HFONT hf;

POINT emptyCell;
const int matrixSize = 4;
struct Cell
{
    HBITMAP bitmap;
    int number;
};

struct Score
{
    int score;
    char name[10];
};

Score scoreArr[5];

int CALLBACK Rules(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK Scoreboard(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR  CALLBACK Win(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void CellSwap(Cell** matrix, POINT clickedCell);
void CellClicked(HWND hWnd, LPARAM lParam, Cell** matrix);
void Reshuffle(Cell** matrix);
void Win(HWND hWnd, Cell** matrix);
void CheckForWin(HWND hWnd, Cell** matrix);
void CommandHandle(HWND hWnd, Cell** matrix, LPARAM wParam);
void Create(Cell** matrix);
void Destroy(Cell** matrix);
void DrawCells(HDC hdc, HDC hdcMem, Cell** matrix, LPSTR buffer);
void SaveFile();
void LoadScore();
void UpdateScoreboard();

//----boxes----//
int CALLBACK Rules(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;
    }

    return FALSE;
}

int CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;
    }

    return FALSE;
}

int CALLBACK Scoreboard(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    char buffer[22];
    std::stringstream stream;
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_PAINT:
        hdc = BeginPaint(hDlg, &ps);

        hf = CreateFontIndirect(&lf);
        SelectObject(hdc, hf);
        SetBkMode(hdc, TRANSPARENT);
        

        for (int i = 0; i < 5; i++)
        {
            if (scoreArr[i].score == 0)
                break;

            stream.str("");
            stream << i + 1 << ". " << scoreArr[i].score << "  -  " << scoreArr[i].name;

            std::string buffer2 = stream.str();
            strcpy_s(buffer, buffer2.c_str());
            
            bool flag = false;
            for (int i = 0; i < 22; i++)
            {
                if (flag)
                    buffer[i] = ' ';
                if (buffer[i] == '\0')
                    flag = true;
            }

            TextOut(hdc, 20, 100 + i * 50, buffer, 22);
        }

        EndPaint(hDlg, &ps);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;
    }

    return FALSE;
}

int CALLBACK Win(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    char buffer[10];
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_PAINT:
        hdc = BeginPaint(hDlg, &ps);

        hf = CreateFontIndirect(&lf);
        SelectObject(hdc, hf);
        SetBkMode(hdc, TRANSPARENT);

        TextOut(hdc, 125, 55, buffer, wsprintf(buffer, "%3d", currentScore));

        EndPaint(hDlg, &ps);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT1, buf, 10);
            UpdateScoreboard();

            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;
    }

    return FALSE;
}
//-------------//

void LoadScore()
{
    std::ifstream fin("scoreboard.txt");

    if (!fin.is_open())
    {
        return;
    }

    for (int i = 0; i < 5; i++)
    {
        fin >> scoreArr[i].score >> scoreArr[i].name;
    }

    fin.close();
}

void SaveFile()
{
    std::ofstream fout("scoreboard.txt");

    for (int i = 0; i < 5; i++)
    {
        fout << "" << scoreArr[i].score << " " << scoreArr[i].name << "\n";
    }
    fout.close();
}

void CellSwap(Cell** matrix, POINT clickedCell)
{
    currentScore++;
    int temp;
    matrix[emptyCell.x][emptyCell.y].number = matrix[clickedCell.x][clickedCell.y].number;
    matrix[clickedCell.x][clickedCell.y].number = 16;
    emptyCell.x = clickedCell.x;
    emptyCell.y = clickedCell.y;
}

void CellClicked(HWND hWnd, LPARAM lParam, Cell** matrix)
{
    bool isNeedToReDraw;
    isNeedToReDraw = false;
    POINT pt;
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);

    POINT clickedCell;
    clickedCell.x = (pt.x / 100) % 10;
    clickedCell.y = (pt.y / 100) % 10;

    if (emptyCell.x == clickedCell.x + 1 && emptyCell.y == clickedCell.y)
    {
        CellSwap(matrix, clickedCell);
        isNeedToReDraw = true;
    }
    else if (emptyCell.x == clickedCell.x - 1 && emptyCell.y == clickedCell.y)
    {
        CellSwap(matrix, clickedCell);
        isNeedToReDraw = true;
    }
    else if (emptyCell.x == clickedCell.x && emptyCell.y == clickedCell.y + 1)
    {
        CellSwap(matrix, clickedCell);
        isNeedToReDraw = true;
    }
    else if (emptyCell.x == clickedCell.x && emptyCell.y == clickedCell.y - 1)
    {
        CellSwap(matrix, clickedCell);
        isNeedToReDraw = true;
    }

    if (isNeedToReDraw)
    {
        InvalidateRect(hWnd, NULL, TRUE);
        isNeedToReDraw = false;
    }
}

void Reshuffle(Cell** matrix)
{
    currentScore = 0;
    srand(time(NULL));
    int arr[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    for (int i = 15 - 1; i >= 1; i--)
    {
        int j = rand() % (i + 1);

        int tmp = arr[j];
        arr[j] = arr[i];
        arr[i] = tmp;
    }

    int counter = 0;
    int rX = rand() % matrixSize;
    int rY = rand() % matrixSize;

    for (int i = 0; i < matrixSize; i++)
    {
        if (counter == matrixSize * matrixSize)
            break;
        for (int j = 0; j < matrixSize; j++)
        {
            matrix[i][j].number = arr[counter];
            matrix[i][j].bitmap = numBitmap;
            counter++;
        }
    }
    matrix[matrixSize - 1][matrixSize - 1].number = 16;
    matrix[matrixSize - 1][matrixSize - 1].bitmap = emptyBitmap;
    emptyCell.x = matrixSize - 1;
    emptyCell.y = matrixSize - 1;
}

void UpdateScoreboard()
{
    if (scoreArr[4].score < currentScore && scoreArr[4].score != 0)
    {
        return;
    }

    Score currentTS;

    currentTS.score = currentScore;
    for (int i = 0; i < 10; i++)
    {
        currentTS.name[i] = buf[i];
    }

    Score tempS;
    int i = 0;
    for (i; i < 5; i++)
    {
        if (scoreArr[i].score > currentTS.score || scoreArr[i].score == 0)
        {
            tempS.score = scoreArr[i].score;
            for (int k = 0; k < 10; k++)
            {
                tempS.name[k] = scoreArr[i].name[k];
            }

            scoreArr[i].score = currentTS.score;
            for (int k = 0; k < 10; k++)
            {
                scoreArr[i].name[k] = currentTS.name[k];
            }

            currentTS.score = tempS.score;
            for (int k = 0; k < 10; k++)
            {
                currentTS.name[k] = tempS.name[k];
            }
        }
    }

}

void Win(HWND hWnd, Cell** matrix)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, Win);
}

void CheckForWin(HWND hWnd, Cell** matrix)
{
    if (emptyCell.x == matrixSize - 1 && emptyCell.y == matrixSize - 1)
    {
        int prev = 0;
        for (int j = 0; j < matrixSize; j++)
        {
            for (int i = 0; i < matrixSize; i++)
            {
                if (matrix[i][j].number != prev + 1)
                {
                    return;
                }
                prev++;
            }
        }
        Win(hWnd, matrix);
        Reshuffle(matrix);
        InvalidateRect(hWnd, NULL, TRUE);
    }
}

void CommandHandle(HWND hWnd, Cell** matrix, LPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case ID_GAME_SHUFFLE:
        Reshuffle(matrix);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case ID_GAME_SCOREBOARD:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWnd, Scoreboard);
        break;
    case ID_ABOUT_RULES:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Rules);
        break;
    case ID_ABOUT_ABOUT:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, Rules);
        break;
    }
}


void Create(Cell** matrix)
{
    LoadScore();

    numBitmap = (HBITMAP)LoadImage(hInst, "num.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    emptyBitmap = (HBITMAP)LoadImage(hInst, "empty.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    
    if (false) // если сохранение было то открыть и загрузить
    {

    }
    else //если не ьыло сохранения то выполнить решфал
    {
        Reshuffle(matrix);
    }
}

void Destroy(Cell** matrix) 
{
    SaveFile();

    for (int i = 0; i < matrixSize; ++i)
        delete[] matrix[i];
    delete[] matrix;

    DeleteObject(numBitmap);
    DeleteObject(emptyBitmap);
    PostQuitMessage(0);

}

void DrawCells(HDC hdc, HDC hdcMem, Cell** matrix, LPSTR buffer)
{
    BITMAP bitmap;
    HGDIOBJ oldBitmap;
    for (int j = 0; j < matrixSize; j++)
    {
        for (int i = 0; i < matrixSize; i++)
        {
            if (matrix[i][j].number == 16)
            {
                oldBitmap = SelectObject(hdcMem, emptyBitmap);
                GetObject(emptyBitmap, sizeof(bitmap), &bitmap);
                BitBlt(hdc, i * 100, j * 100, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
                SelectObject(hdcMem, oldBitmap);

                continue;
            }
            oldBitmap = SelectObject(hdcMem, numBitmap);
            GetObject(numBitmap, sizeof(bitmap), &bitmap);
            BitBlt(hdc, i * 100, j * 100, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, oldBitmap);
            TextOut(hdc, i * 100 + 10, j * 100 + 20, buffer, wsprintf(buffer, "%3d", matrix[i][j].number));
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = NULL;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    hInst = hInstance;
    hWnd = CreateWindow(szWindowClass,
        szTitle,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        200, 
        200,  
        416,
        455, 
        NULL,  
        NULL,
        hInstance,
        NULL);

    SetClassLong(hWnd, GCL_HICON, (LONG)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));

    if (!hWnd)
    {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HDC hdcMem;
    static short x, y;
    static bool repeater = 1;

    lf.lfPitchAndFamily = VARIABLE_PITCH;
    lf.lfHeight = 60;

    static Cell** matrix = new Cell * [matrixSize];
    if (repeater)
    {
        for (int i = 0; i < matrixSize; ++i)
        {
            matrix[i] = new Cell[matrixSize];
        }
        repeater = false;
    }
    char buffer[10];

    switch (message)
    {
    case WM_CREATE:
        Create(matrix);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        SetTextColor(hdc, RGB(255, 255, 205));
        hf = CreateFontIndirect(&lf);
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, hf);  
        hdcMem = CreateCompatibleDC(hdc);

        DrawCells(hdc, hdcMem, matrix, buffer);

        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
        break;
    case WM_LBUTTONDOWN:
        CellClicked(hWnd, lParam, matrix);
        CheckForWin(hWnd, matrix);
        break;
    case WM_RBUTTONDOWN:
        Win(hWnd, matrix);
        Reshuffle(matrix);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_DESTROY:
        Destroy(matrix);
        break;
    case WM_COMMAND:
        CommandHandle(hWnd, matrix, wParam);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
