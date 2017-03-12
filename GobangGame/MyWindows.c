/* -------------------------------------------------------------------
                    MyWindows.c -- 基本窗口模型  
				《Windows 程序设计（SDK）》视频教程                    
--------------------------------------------------------------------*/

#include <windows.h>
#include <math.h>

#define BOARD_CELL_NUM 14
#define FIVE_MARK_POINT_RADIUS 4
#define CHESS_PIECE_RADIUS	13
#define BLACK_FLAG 1
#define WHITE_FLAG 2
#define NULL_FLAG 0
#define WIN_CONDITION 5

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("MyWindows");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("这个程序需要在 Windows NT 才能执行！"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, 
		TEXT("我的五子棋游戏"), 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		610, 
		610,
		NULL, 
		NULL, 
		hInstance, 
		NULL);
	
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

// 绘制黑色实心点
HRESULT _DrawBlackSolidPoint(HDC hdc, int radius, POINT postion)
{
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	return S_OK;
}

// 绘制白色空心点
HRESULT _DrawWhiteHollowPoint(HDC hdc, int radius, POINT postion)
{
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);

	return S_OK;
}

// 获取一小格宽度和高度
HRESULT _GetCellWidthAndHeight(POINT ptLeftTop, int cxClient, int cyClient, int *cxCell, int *cyCell)
{
	*cxCell = (cxClient - ptLeftTop.x * 2) / BOARD_CELL_NUM;
	*cyCell = (cyClient - ptLeftTop.y * 2) / BOARD_CELL_NUM;

	return S_OK;
}

// 将实际坐标转化为逻辑坐标，这里需要进行实际点到棋盘点的转化
HRESULT _ExChangeLogicalPosition(POINT actualPostion, POINT ptLeftTop, int cxClient, int cyClient, POINT *logicalPostion)
{
	// 获得一小格的宽度和高度
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// 检查点击有效性
	if (actualPostion.x < ptLeftTop.x || actualPostion.x > ptLeftTop.x + BOARD_CELL_NUM * cxCell ||
		actualPostion.y < ptLeftTop.y || actualPostion.y > ptLeftTop.y + BOARD_CELL_NUM * cyCell) {
		MessageBox(NULL ,TEXT("请点击棋盘内下棋！"), TEXT("提示"), MB_OK);
		return S_FALSE;
	}
	// 获取相邻四个点
	int xCount = 0, yCount = 0;
	POINT sidePoints[4] = { 0 };
	for (int x = ptLeftTop.x; x <= ptLeftTop.x + BOARD_CELL_NUM * cxCell; x += cxCell, xCount++) {
		if (actualPostion.x >= x && actualPostion.x <= x + cxCell) {
			sidePoints[0].x = x;
			sidePoints[2].x = x;
			sidePoints[1].x = x + cxCell;
			sidePoints[3].x = x + cxCell;
			break;
		}
	}
	for (int y = ptLeftTop.y; y <= ptLeftTop.y + BOARD_CELL_NUM * cyCell; y += cyCell, yCount++) {
		if (actualPostion.y >= y && actualPostion.y <= y + cyCell) {
			sidePoints[0].y = y;
			sidePoints[1].y = y;
			sidePoints[2].y = y + cyCell;
			sidePoints[3].y = y + cyCell;
			break;
		}
	}
	// 计算当前点到四个点到当前点距离
	double lengthCount[4] = { 0 };
	for (int item = 0; item < 4; ++item) {
		lengthCount[item] = pow(abs(sidePoints[item].x - actualPostion.x), 2) + pow(abs(sidePoints[item].y - actualPostion.y), 2);
	}
	// 获取四个距离值中最短的一个
	int shortestIndex = 0;
	for (int item = 0; item < 4; ++item) {
		if (lengthCount[item] < lengthCount[shortestIndex]) {
			shortestIndex = item;
		}
	}
	// 计算逻辑坐标，其中下标为0的点为基准点
	if (1 == shortestIndex) {
		xCount += 1;
	} 
	else if (2 == shortestIndex) {
		yCount += 1;
	}
	else if (3 == shortestIndex) {
		xCount += 1;
		yCount += 1;
	}
	logicalPostion->x = xCount;
	logicalPostion->y = yCount;

	return S_OK;
}

// 将逻辑坐标转化为实际坐标
HRESULT _ExchangeActualPositon(POINT logicalPos, int cxCell, int cyCell, POINT ptLeftTop, POINT *actualPos)
{
	actualPos->x = ptLeftTop.x + logicalPos.x * cxCell;
	actualPos->y = ptLeftTop.y + logicalPos.y * cyCell;

	return S_OK;
}

// 绘制棋盘
HRESULT DrawChessBoard(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
	// 获得一小格的宽度和高度
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// 绘制竖线
	for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
		MoveToEx(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y, NULL);
		LineTo(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y + BOARD_CELL_NUM * cyCell);
	}
	// 绘制灰色的横线
	HPEN hPen, hOldPen;
	hPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
	hOldPen = SelectObject(hdc, hPen);
	for (int row = 0; row < 7; ++row) {
		MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + cyCell + row * 2 * cyCell, NULL);
		LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + cyCell + row * 2 * cyCell);
	}
	SelectObject(hdc, hOldPen);
	// 绘制黑色的横线
	for (int row = 0; row < 8; ++row) {
		MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + row * 2 * cyCell, NULL);
		LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + row * 2 * cyCell);
	}

	return S_OK;
}

// 绘制五个着重点
HRESULT DrawFiveHeavyPoint(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
	// 获得一小格的宽度和高度
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// 将逻辑点转换为实际点
	POINT logicalPoint[5] = { 3, 3, 3, 11, 11, 3, 11, 11, 7, 7 };
	POINT actualPoint[5] = { 0 };
	for (int cPt = 0; cPt < 5; ++cPt) {
		_ExchangeActualPositon(logicalPoint[cPt], cxCell, cyCell, ptLeftTop, &actualPoint[cPt]);
	}
	// 绘制五个黑色实心点
	for (int cPt = 0; cPt < 5; ++cPt) {
		_DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, actualPoint[cPt]);
	}

	return S_OK;
}

// 判定是否胜利
HRESULT IsSomeoneWin(int chessPoints[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1], int *winner)
{
	// 横着连续五个胜利
	int blackHrozCount = 0;
	int whiteHrozCount = 0;
	// 竖着连续五个胜利
	int blackVrclCount = 0;
	int whiteVrclCount = 0;
	// 斜着连续五个胜利

	// 计算连续竖着的胜利
	for (int row = 0; row < BOARD_CELL_NUM + 1; ++row) {
		for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
			
		}
		if (WIN_CONDITION == blackVrclCount || WIN_CONDITION == blackHrozCount) {
			*winner = BLACK_FLAG;
			break;
		}
		if (WIN_CONDITION == whiteVrclCount || WIN_CONDITION == whiteHrozCount) {
			*winner = WHITE_FLAG;
			break;
		}
	}


	return S_OK;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	// 客户区坐标
	static int cxClient, cyClient;
	// 每个方格的宽度和高度
	int cxCell = 0, cyCell = 0;
	// 棋盘左上角坐标
	POINT ptLeftTop;
	ptLeftTop.x = 30;
	ptLeftTop.y = 40;
	// 鼠标点击实际点
	POINT actualPosition;
	// 鼠标点击实际点计算转化来的逻辑点
	POINT logicalPostion;
	// 鼠标点击实际点计算转化来的逻辑点对应的实际点
	POINT changedActualPosition;
	// 记录逻辑位置的数组，其中BLACK_FLAG为黑子，WHITE_FLAG为白子，NULL_FLAG为空白
	static int chessPoints[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1] = { NULL_FLAG };
	// 胜利者
	int winner = NULL_FLAG;
	// 函数返回值
	HRESULT hResult = S_FALSE;

	switch (message)
	{
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;

	case WM_LBUTTONDOWN:	// 黑子
		// 获得实际点
		actualPosition.x = LOWORD(lParam);
		actualPosition.y = HIWORD(lParam);
		// 获得对应的计算过后的逻辑点
		hResult = _ExChangeLogicalPosition(actualPosition, ptLeftTop, cxClient, cyClient, &logicalPostion);
		if (S_FALSE == hResult) {
			return 0;
		}
		// 将逻辑点记录下来
		chessPoints[logicalPostion.x][logicalPostion.y] = BLACK_FLAG;
		// 获得一小格的宽度和高度
		_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
		// 将逻辑点转化为实际点
		POINT changedActualPosition;
		_ExchangeActualPositon(logicalPostion, cxCell, cyCell, ptLeftTop, &changedActualPosition);
		// 绘制实际点
		hdc = GetDC(hwnd);
		_DrawBlackSolidPoint(hdc, CHESS_PIECE_RADIUS, changedActualPosition);
		ReleaseDC(hwnd, hdc);
		// 计算胜利
		IsSomeoneWin(chessPoints, &winner);
		if (BLACK_FLAG == winner) {
			MessageBox(hwnd, TEXT("黑棋获胜！"), TEXT("提示"), NULL);
		}
		if (WHITE_FLAG == winner) {
			MessageBox(hwnd, TEXT("白棋获胜！"), TEXT("提示"), NULL);
		}
		return 0;

	case WM_RBUTTONDOWN:	// 白子
		// 获得实际点
		POINT actualPosition, logicalPostion;
		actualPosition.x = LOWORD(lParam);
		actualPosition.y = HIWORD(lParam);
		// 获得对应的计算过后的逻辑点
		hResult = _ExChangeLogicalPosition(actualPosition, ptLeftTop, cxClient, cyClient, &logicalPostion);
		if (S_FALSE == hResult) {
			return 0;
		}
		// 将逻辑点记录下来
		chessPoints[logicalPostion.x][logicalPostion.y] = WHITE_FLAG;
		// 获得一小格的宽度和高度
		_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
		// 将逻辑点转化为实际点
		_ExchangeActualPositon(logicalPostion, cxCell, cyCell, ptLeftTop, &changedActualPosition);
		// 绘制实际点
		hdc = GetDC(hwnd);
		_DrawWhiteHollowPoint(hdc, CHESS_PIECE_RADIUS, changedActualPosition);
		ReleaseDC(hwnd, hdc);
		// 计算胜利
		IsSomeoneWin(chessPoints, &winner);
		if (BLACK_FLAG == winner) {
			MessageBox(hwnd, TEXT("黑棋获胜！"), TEXT("提示"), NULL);
		}
		if (WHITE_FLAG == winner) {
			MessageBox(hwnd, TEXT("白棋获胜！"), TEXT("提示"), NULL);
		}
		return 0;

	case WM_MBUTTONDOWN:	// 初始化棋盘
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// 初始化棋盘
		DrawChessBoard(hdc, ptLeftTop, cxClient, cyClient);			// 绘制棋盘
		DrawFiveHeavyPoint(hdc, ptLeftTop, cxClient, cyClient);		// 绘制五个着重点

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}