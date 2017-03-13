# 引言：GobangGame
这是一个使用纯Win32 SDK和C语言实现的五子棋游戏

![五子棋照片](https://github.com/wangying2016/GobangGame/raw/master/Picture/Demo2.png)

在这个游戏中实现的功能

- [x] 绘制一个15 * 15的棋盘

- [x] 绘制5个着重点位置

- [x] 根据鼠标左键的点击，可以绘制出黑色棋子；根据鼠标右键的点击，可以绘制出白色棋子

- [x] 根据鼠标中键的点击，可以初始化棋盘

- [x] 根据鼠标点击的实际位置，可以计算转化为临近点最近的那个点，在这个点上绘制棋子

- [x] 当鼠标位置不在棋盘内，可以显示提示信息

- [x] 五子棋判胜逻辑的实现

这个游戏还是算比较简单的，主要难点在于两点

- 如何将鼠标点击的实际坐标，转化为临近最近的点显示棋子
- 如何实现五子棋的判胜逻辑
# 如何根据鼠标点击的实际坐标，转为临近最近的点呢
这个实现是非常简单的，大家初中肯定都学过勾三股四弦五，就是勾股定理啦。我们只要计算当前实际点到它临近四个点的实际距离，然后拿到距离最短的那个点的逻辑位置就行啦。  

这里的实现比较简单，因为代码大家也是可以拿到的，这里就不再赘述，直接贴上最关键的实现代码。
```C++
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
```
# 如何实现五子棋的判胜逻辑
这是这个游戏的最难的一点，这一点也确实让我思考了很久。  

思路是慢慢展开的，胜利的方式有三种，一种是横着连续5个棋子，一种是竖着连续5个棋子，另一种呢，就是斜着5个棋子。这里就需要对于坐标进行一些计算了。但是这里要记录同一类棋子同一方向的个数...

再按照上述的思考方式来考虑这个问题，就会越来越难了，那么怎么解决这个问题呢？  

我想到了大一下学期学的数据结构，最长路径算法！对了，我们只要能让这个棋盘中，同一类棋子的最长路径为5就可以判定胜利啦！

这样就简化了问题，渐渐地，在这个主要思想的指导下，我又考虑到了一个棋子能判定胜利的方向有8个方向，我们判定胜利都是在这8个方向下遍历判断的。

遍历8个方向的时候，每个方向的棋子个数怎么计算呢？  

这个其实也比较简单，使用**递归**！我们计算以**一个基准点**从**同一方向**开始判定此此基准点的下一个点是不是**同类棋子**，如果是的话，则重复这个函数，如果不是的话，则返回计数。

我们拿到了每个棋子的计数，只要有一个棋子的技术等于了5，我们就可以判定此类棋子代表的那方胜利！很简单吧。  

逻辑分析了这么久，其实还不如一个简单的总结简洁，这里整理如下：

1. 遍历当前棋盘，传入一个基准点参数
2. 遍历8个方向，传入一个方向值
3. 根据方向值计算出下个点的坐标，与基准点比较，判断是否是同类棋子
4. 如果是同类棋子，重复第3步；否则，返回计数
5. 查看返回的计数是否等于5，如果等于5则判定该类棋子胜利

这里附上我觉得最为精华的一个递归函数，其他代码感兴趣的同学可以参看github拿到源代码，就不再赘述：
```C++
// 计算当前方向过去的同类棋子的个数
HRESULT _CountSameDiretionPointsNumber(int chessPoints[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1], POINT point, Win_Direction direction, int *count)
{
	(*count) += 1;
	BOOLEAN bSame = FALSE;
	POINT movedPoint = {point.x, point.y};
	_IsSidewardHasSamePoint(chessPoints, point, direction, &bSame, &movedPoint);
	if (bSame == TRUE) {
		bSame = FALSE;
		POINT movedmovedPoint = { movedPoint.x, movedPoint.y };
		_CountSameDiretionPointsNumber(chessPoints, movedPoint, direction, count);
	}

	return S_OK;
}
```

# 总结
这个游戏的完成，花了我两个晚上。其中第一个晚上，主要用来绘制棋盘，绘制棋子，绘制辅助点，完成大体框架；第二个晚上，主要解决五子棋的判胜逻辑的实现。总的来说，这个游戏中，我觉得还是算法最难。其他的都是多练可以提升的，不过是SDK函数的熟悉而已，而算法才是最练人的。希望自己再接再厉，在学习提升自己的编程熟练度的同时，也要加强算法的学习。

