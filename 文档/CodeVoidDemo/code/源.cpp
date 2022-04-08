#define WIN32_LEAN_AND_MEAN_
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <fstream>
#include <deque>
#include <vector>
#include <map>

#include "..\\..\\Classes\UvcDDApp.h"
#include "..\\..\\Classes\UvcImage.h"

using std::string;
using std::deque;
using std::vector;
using std::map;
using std::pair;

#define __RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

#define __POSTERR(str) {MessageBox(0, str, "Error", NULL);}
#define UvcDXInit_s(res, str) { if (FAILED(res)) {__POSTERR(str);return false; } }
#define InitDDStruct(dds) {memset(&dds, 0, sizeof(decltype(dds)); dds.dwsize = decltype(dds))}

#define __GAME_STATE_MENU__		0
#define __GAME_STATE_RUN__		1
#define __GAME_STATE_WIN__		2
#define __GAME_STATE_DIE__		3
#define __GAME_STATE_END__		4
#define __GAME_STATE_STGINIT__	5

#define __MENU_NEWGAME__		0
#define __MENU_EXIT__			1
#define __MAX_MENU__			2

#define __DIE_STATE_ALIVE__		0
#define __DIE_STATE_DIE__		1
#define __DIE_STATE_WIN__		2

#define GROUND	0
#define FLY		1

#define __G__	30

// forward Declaration
class UvcRect;
class Role;
class Object;
class TestApp;
class CEnimy;

// UvcRect类：
// 存储了一些用于渲染和判定的矩形
class UvcRect
{
public:
	UvcRect() { rect = { 0 }; return; };
	UvcRect(RECT r) :UvcRect() { rect = r; return; };
	~UvcRect() {};

public:
	RECT rect;
	UINT msg;
};

// Role类：
// 存储了玩家信息，包括贴图和位置
// 以及一部分的物理信息，如速度
// 开枪高度y+36
class Role
{
	// constuctor & desrturctor
public:
	Role();
	explicit Role(int CharaX, int CharaY);
	Role(const Role&) = delete;

public:
	// 读取人物坐标
	inline int GetX() { return mCharax; };
	inline int GetY() { return mCharay; };
	inline POINT GetXY()
	{
		POINT p;
		p.x = mCharax;
		p.y = mCharay;
		return p;
	};

	// 读取待渲染贴图，依据内部变量自动选择。
	UvcImage GetBMP();

	// 判断角色是否在移动
	inline bool IsMoving() { return (mCharavx) ? 1 : 0; };

	// 角色移动操作
	inline void Jump();
	inline void RunLeft() { mCharavx = -mStdvx; mfaceRight = false; };
	inline void RunRight() { mCharavx = mStdvx; mfaceRight = true; };
	inline void StopRun();

	// call when Role is falling
	inline void Fall() { mFall = true; };
	// Call when Role stop Falling.
	inline void StopFall() { mFall = false; mJumpTimes = 2; };
	inline bool IsFalling() { return mFall; };

	// 角色移动的时候的更新函数
	void Move();
	// 设置碰撞箱
	void SetDetectRECT(UINT Width = 65535, UINT Height = 65535);
	// 动画计数
	void Count();

	inline bool IsFaceRight() { return mfaceRight; };
	inline bool IsFireing() { return mFire; };
	inline void Fireing() { if (LastFireTick - GetTickCount64() < 100 || Getvx())return; mFire = true; LastFireTick = GetTickCount64(); return; };
	inline void Fired() { mFire = false; return; };

	inline void Damage(ULONGLONG Dmg);

public:
	// getters for debug and Edge-Detect.
	inline LONG Getvy() { return mCharavy; };
	inline void ForceSetVy(LONG v) { mCharavy = v; return; };
	inline LONG Getvx() { return mCharavx; };
	inline void ForceSetVx(LONG v) { mCharavx = v; return; };
	inline POINT GetCenterPos();
	inline int GetDieState() { return mdie; };
	inline void SetDieState(int state) { mdie = state; return; }
	inline void ForceSetPos(LONG X, LONG Y) { mCharax = X; mCharay = Y; return; };
	inline LONG GetStdVx() { return mStdvx; };

protected:
	void PlotPixel(DDSURFACEDESC2& ddsd, int x, int y, int r, int g = -1, int b = -1, int a = 0);
	static void PlotPixel(UvcImage UImg, int x, int y, int r, int g = -1, int b = -1, int a = 0);

public:
	// 角色贴图
	UvcImage uBmp;
	UvcImage rAnim[5];
	UvcImage lAnim[5];

	RECT detectRect;

public:
	void SetClientSize(int Width, int Height);
	static int mClientWidth;
	static int mClientHeight;
	/*static DDPIXELFORMAT CanvasPF;
	static DDSURFACEDESC2 CanvasDDSD;*/

private:
	int mJumpTimes;

	LONG mCharax;
	LONG mCharay;
	LONG mCharavx;
	LONG mCharavy;
	LONG mStdvx;

	int mAnimIndex;
	int mAnimIndexAddCount;
	int mdie;

	bool mfaceRight;
	bool mFall;

	ULONGLONG LastFireTick;
	bool mFire;

	int mAtk;
	UCHAR mHp;
};

class Object
{
public:
	Object()
	{
		Box = { 0 };
		Attr = 0;
		return;
	};
	Object(const Object& obj) { Box = obj.Box; Attr = obj.Attr; return; };
	virtual ~Object() {};

public:
	RECT Box;
	int Attr;
};

class CEnimy :public Object
{
public:
	CEnimy();
	CEnimy(string Name);
	CEnimy(const CEnimy& src);
	CEnimy(const CEnimy& src, int x, int y);
	virtual ~CEnimy()
	{
		if (LImg != nullptr) delete[] LImg;
		if (RImg != nullptr) delete[] RImg;
	};

public:
	inline void Damage(int Dmg);
	inline LONG GetHP() { return mHP; };
	inline LONG GetATK() { return mATK; };

	void LoadInfo();

	//动画相关函数
public:
	void Move(const TestApp& app);
	UvcImage GetCurImage();
protected:
	void Count();

	// 攻防属性相关
public:
	string mName;
	LONG mHP;
	LONG mATK;
	LONG vx;
	LONG vy;

	// 动画相关变量
public:
	int MaxAnimIndex;
	int mAnimIndex;
	UvcImage* LImg;
	UvcImage* RImg;
	vector<int> mAnimList;
	LONG x;
	LONG y;
	LONG curvx;
	LONG curvy;

	bool mFaceRight;
	bool mFall;
	bool mDeath;

	// AI 相关
public:
	bool mActive;
};

class TestApp :public UvcDDApp
{
public:
	TestApp(HINSTANCE hInstance) :UvcDDApp(hInstance) { mFullScreenState = false; };
	explicit TestApp(HINSTANCE hInstance, int Width = 1920, int Height = 1080) :UvcDDApp(hInstance) { mFullScreenState = false; mClientWidth = Width; mClientHeight = Height; };
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	virtual void Draw(const UvcTimer& Timer)override;
	virtual void Update(const UvcTimer& Timer)override;

public:
	void Pause() { mAppPaused = true; return; };
	void unPause() { mAppPaused = false; return; };
	friend void CEnimy::Move(const TestApp& app);

protected:
	void StageInit();

public:
	//UvcImage uBmp;
	UvcImage Menu[2][2];
	LONG mMaxStage = 0;

public:
	static UvcImage bkg;

private:
	void KeyDownEvent(WPARAM wParam);
	void KeyUpEvent(WPARAM wParam);

	void SetCameraPos(LONG x, LONG y);

private:
	LONG Camerax = 0;
	LONG Cameray = 0;
	int mGameState = 0;
	int mSubStateSlected = 0;
	int mStage = -1;

	bool ShowDebugInfo = false;
};

inline bool IsIn(POINT p, RECT rect);

// Init Class's static member.
int Role::mClientWidth = 800;
int Role::mClientHeight = 600;
UvcImage TestApp::bkg;

// Stage Map
UvcImage Map;
// Menu Bkg.
UvcImage MenuBkg;
// new Charas.(BOBs)
Role Chara(10, 10);
// Chara's AnimList
int Animlist[6] = { 1, 2, 3, 4, 3, 2 };
// MenuItem.
std::string menuItem[2] = { "newGame", "exit" };

// Enimys
map<int, CEnimy> EnimyDict;
vector<CEnimy> EnimyList;
vector<RECT> GoalList;

char debug[100] = {};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance,
	PSTR cmdLine, int showcmd)
{
	using std::ifstream;

	int index = 0;
	string EnimyName;
	RECT GoalRect;

	// buffer String
	char sz[100] = { 0 };

	try {
		// 前置数据载入
		ifstream Config;
		Config.open("rcs\\config.uvcsource");
		if (!Config.is_open()) throw 1;
		int Width = 800, Height = 600;
		Config >> Width >> Height;
		while (Config >> EnimyName >> index)
		{
			EnimyDict.insert(pair<int, CEnimy>(index, CEnimy(EnimyName)));
			EnimyDict[index].LoadInfo();
		}
		Config.close();

		// Init UvcDDApp
		TestApp app(hInstance, Width, Height);
		if (!app.Initialize())
			throw 2;

		//游戏核心数据载入
		Config.open("rcs\\stage.uvcsource");
		if (!Config.is_open()) throw 2;
		Config >> app.mMaxStage;
		for (int i = 0; i < app.mMaxStage; i++)
		{
			Config >> GoalRect.left >> GoalRect.top >> GoalRect.right >> GoalRect.bottom;
			GoalList.push_back(GoalRect);
		}
		Config.close();

		// Load Image.
		{
			Chara.uBmp.loadImage("rcs\\chara\\stay.bmp");

			TestApp::bkg.loadImage("rcs\\bkg.bmp");

			MenuBkg.loadImage("rcs\\Menubkg.bmp");
			Map.loadImage("rcs\\map_long.bmp");
			// Load Role resources
			for (int i = 0; i < 5; i++)
			{
				sprintf_s(sz, "rcs\\chara\\rRun%d.bmp", i);
				Chara.rAnim[i].loadImage(sz);
				sprintf_s(sz, "rcs\\chara\\lRun%d.bmp", i);
				Chara.lAnim[i].loadImage(sz);
			}
			// Load Menu resources
			for (int i = 0; i < __MAX_MENU__; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					if (j)
					{
						sprintf_s(sz, "rcs\\%sSlected.bmp", menuItem[i].c_str());
					}
					else
					{
						sprintf_s(sz, "rcs\\%s.bmp", menuItem[i].c_str());
					}
					app.Menu[i][j].loadImage(sz);
				}
			}
		}
		Chara.SetClientSize(Map.mBitMapInfoHeader.biWidth, Map.mBitMapInfoHeader.biHeight);
		Chara.SetDetectRECT(30, 110);

		return app.Run();
	}
	catch (int a)
	{
		sprintf_s(sz, "%d", a);
		MessageBox(nullptr, sz, "Fatal Error", NULL);
		return a;
	}
	catch (...)
	{
		MessageBox(nullptr, "Failed", "Failed", MB_OK);
		return 1;
	}
	return -1;
}

void TestApp::Draw(const UvcTimer& Timer)
{
	int flag = 0;// 用于部分判定
	int judx = 0, judy = 0;
	RGBInfo col;// for debug
	int step = 0;

	switch (mGameState)
	{
	case __GAME_STATE_MENU__:
	{
		DrawImageToDDSurface(MenuBkg, mDDSD, MenuBkg.GetRect(), 0, 0);
		for (int i = 0; i < __MAX_MENU__; i++)
		{
			DrawImageToDDSurface(Menu[i][(i == mSubStateSlected)], mDDSD, Menu[i][(i == mSubStateSlected)].GetRect(), 1500, (i + 25) * 25);
		}
	}// end menu
	break;
	case __GAME_STATE_RUN__:
	{
		// 摄像机先做一下移动
		if (Chara.GetX() > 960 && Chara.GetX() < (Map.mBitMapInfoHeader.biWidth - 960))
		{
			Camerax = Chara.GetX() - 960;
		}

		if (Chara.GetX() < 960)
		{
			Camerax = 0;
		}

		// 现在准备写一下掉落的判定
		// 思路是对玩家角色下方逐像素进行rgb比对，如果和背景颜色一样说明没有落脚点
		// 就往下掉(0v0)

		// Anim slect
		// 这个是画背景的
		DrawImageToDDSurface(TestApp::bkg, mDDSD, TestApp::bkg.GetRect(), 0, 0);

		// 这里画地图
		DrawImageToDDSurface(Map, mDDSD, Map.GetCameraRECT(Camerax, Cameray, mClientWidth, mClientHeight), 0, 0);

		// 开始掉落比对
		// 使用判定矩形来看看到头没
		// 因为是逐帧操作的所以每次只需要比对Vy行像素
		// 如果y轴方向速度小于零说明正在往上飞，不用管
		if (Chara.Getvy() >= 0)
		{
			// 如果角色正在掉落
			if (Chara.IsFalling())
			{
				for (int j = 0; j < Chara.Getvy(); j++)
				{
					for (int i = Chara.detectRect.left; i < Chara.detectRect.right; i++)
					{
						// 计算要检测的值时一定记住要算相对坐标
						// 要检测的x值范围为 碰撞箱左 -- 碰撞箱右
						judx = Chara.GetX() - Camerax + i;
						// 要检测的y值范围为 碰撞箱下 -- 碰撞箱下+ y轴方向速度
						judy = Chara.GetY() - Cameray + Chara.detectRect.bottom + j;

						// 超出边界没有检测必要
						if (judx > mClientWidth || judx < 0) break;
						if (judy > mClientHeight || judy < 0) break;

						//逐像素比对,有墙就站住
						// 因为都是在主表面上进行检测所以并不用减摄像机算相对坐标
						if (GetPixelRGB(judx, judy) != TestApp::bkg.GetPixelRGB(judx, judy))
						{
							Chara.ForceSetVy(j);
							Chara.StopFall();
						}
					}
					// 这里改过说明之前的循环中已经发现异常，可以退出循环
					if (!Chara.IsFalling()) break;
				}
			}
			// 玩家没在掉落过程中的话每一帧检测一下是否会掉落
			else
			{
				flag = 0;
				for (int i = Chara.detectRect.left; i < Chara.detectRect.right; i++)
				{
					// 碰撞检测起止：
					// x轴： 碰撞箱左右
					judx = Chara.GetX() - Camerax + i;
					judy = Chara.GetY() - Cameray + Chara.detectRect.bottom + 1;
					if (GetPixelRGB(judx, judy) == TestApp::bkg.GetPixelRGB(judx, judy))
					{
						// 此处flag变量用于保存每行中与背景色一致的像素个数
						flag++;
					}
				}
				// 发现人物已经完全离开平台
				if (flag >= (Chara.detectRect.right - Chara.detectRect.left))
				{
					Chara.Fall();
				}
			}
		}
		// 这里进行左右碰撞检测，主要用于人物撞停不会卡进墙里
		if (Chara.Getvx() != 0 && Chara.Getvy() >= 20)
		{
			flag = 0;
			// 改变一下系数，简化后续判定
			if (Chara.Getvx() > 0)	flag = 1;
			if (Chara.Getvx() < 0)	flag = -1;

			// 算出相对坐标
			// 其中x坐标为 玩家中心坐标
			judx = Chara.GetX() - Camerax	// 基准坐标（在摄像机视角下的中心坐标
				+ ((Chara.detectRect.left + Chara.detectRect.right) >> 1)	// 加上这个值后得到角色的x中心
				+ (flag * ((Chara.detectRect.right - Chara.detectRect.left) >> 1)); // 算出碰撞箱相对摄像机的判定起始x
			judy = Chara.GetY() - Cameray;
			for (int i = 0; i < Chara.Getvx(); i++)
			{
				for (int j = judy + Chara.detectRect.top; j < judy + Chara.detectRect.bottom; j++)
				{
					if (GetPixelRGB(judx + i * flag, j) != TestApp::bkg.GetPixelRGB(judx + i * flag, j))
					{
						Chara.ForceSetVx(i * flag);
						break;
					}
				}
				// 不是标准值说明被改过了，前面检测到墙壁了
				if (abs(Chara.Getvx()) != Chara.GetStdVx())	break;
			}
		}
		if (mStage >= GoalList.size())
		{
			Chara.SetDieState(__DIE_STATE_WIN__);
		}
		else
		{
			if (IsIn(Chara.GetCenterPos(), GoalList[mStage]))
			{
				Chara.SetDieState(__DIE_STATE_WIN__);
			}
		}

		if (Chara.GetDieState() == __DIE_STATE_WIN__)
		{
			mGameState = __GAME_STATE_WIN__;
		}
		if (Chara.GetDieState() == __DIE_STATE_DIE__)
		{
			mGameState = __GAME_STATE_DIE__;
		}

		Chara.Move();

		// 正在开火
		if (Chara.IsFireing())
		{
			step = 1;
			if (!Chara.IsFaceRight())step = -1;
			for (int i = Chara.GetX() + (Chara.IsFaceRight() ? Chara.detectRect.right : Chara.detectRect.left) - Camerax; i > 0 && i < mClientWidth; i = i + step)
			{
				PlotPixel(mDDSD, i, Chara.GetY() + 36, 255, 231, 46);
			}
			Chara.Fired();
			for (auto x : EnimyList)
			{
				if (x.Box.top < Chara.GetY() + 36 && x.Box.bottom > Chara.GetY() + 36)
				{
					x.Damage(10);
				}
			}
			for (int i = 0; i < EnimyList.size();)
			{
				if (EnimyList[i].mDeath)
				{
					EnimyList.erase(EnimyList.begin() + i);
				}
				else
				{
					i = i + 1;
				}
			}
		}

		// 这个是画玩家的
		DrawImageToDDSurface(Chara.GetBMP(), mDDSD, Chara.GetBMP().GetRect(), Chara.GetX() - Camerax, Chara.GetY() - Cameray);

		for (auto x : EnimyList)
		{
			DrawImageToDDSurface(x.GetCurImage(), mDDSD, x.GetCurImage().GetRect(), x.x - Camerax, x.y - Cameray);
		}
	}
	break;// end RUN
	case __GAME_STATE_WIN__:
	{
		Chara.ForceSetVx(20);

		Chara.ForceSetPos(Chara.GetX() + 20, Chara.GetY());
		Chara.Count();
		// 这个是画背景的
		DrawImageToDDSurface(TestApp::bkg, mDDSD, TestApp::bkg.GetRect(), 0, 0);

		// 这里画地图
		DrawImageToDDSurface(Map, mDDSD, Map.GetCameraRECT(Camerax, Cameray, mClientWidth, mClientHeight), 0, 0);
		DrawImageToDDSurface(Chara.GetBMP(), mDDSD, Chara.GetBMP().GetRect(), Chara.GetX() - Camerax, Chara.GetY() - Cameray);
		if (Chara.GetX() >= Map.mBitMapInfoHeader.biWidth)
			mGameState = __GAME_STATE_STGINIT__;
	}
	break;
	case __GAME_STATE_DIE__:
	{
	}
	break;
	case __GAME_STATE_STGINIT__:
	{
		Chara.SetDieState(__DIE_STATE_ALIVE__);
		Chara.ForceSetPos(10, 10);
		mGameState = __GAME_STATE_RUN__;
		mStage = mStage + 1;
		StageInit();

		if (mStage >= GoalList.size())
		{
			mStage = 0;
			mGameState = __GAME_STATE_END__;
			break;
		}
	}
	break;
	case __GAME_STATE_END__:
	{
		mGameState = __GAME_STATE_MENU__;
	}
	break;
	default:break;
	}// end Switch
	return;
}

void TestApp::Update(const UvcTimer& Timer)
{
	// TODO: 在此处添加实现代码.

	// To Draw Debug Info
	HDC hdc;
	COLORREF old_fcol, old_bcol;
	int old_tmode;

	memset(&mDDSD, 0, sizeof(mDDSD));
	mDDSD.dwSize = sizeof(mDDSD);
	if (FAILED(mlpDDSurefacePrimary->Lock(NULL, &mDDSD,
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
		NULL)))
	{
		//MessageBox(mhMainWnd, "Lock Falied", "Error", MB_OK);
		//PostQuitMessage(0);
		return;
	}

	Draw(Timer);

	if (FAILED(mlpDDSurefacePrimary->Unlock(NULL)))
	{
		MessageBox(mhMainWnd, "Unlock Falied", "Error", MB_OK);
		PostQuitMessage(0);
		return;
	}

	if (!ShowDebugInfo)	return;
	if (FAILED(mlpDDSurefacePrimary->GetDC(&hdc)))
	{
		__POSTERR("DC get ERR")
			return;
	}
	old_fcol = SetTextColor(hdc, RGB(255, 255, 255));
	old_bcol = SetBkColor(hdc, RGB(0, 0, 0));
	old_tmode = SetBkMode(hdc, TRANSPARENT);

	sprintf_s(debug, "RECT: left:%d right:%d top:%d bottom:%d", Chara.detectRect.left, Chara.detectRect.right, Chara.detectRect.top, Chara.detectRect.bottom);
	TextOut(hdc, 0, 30, debug, strlen(debug));
	sprintf_s(debug, "CharaPos X:%d Y:%d  CharaState:%d CenterPos: x:%d y:%d", Chara.GetX(), Chara.GetY(), Chara.GetDieState(), Chara.GetCenterPos().x, Chara.GetCenterPos().y);
	TextOut(hdc, 0, 45, debug, strlen(debug));
	sprintf_s(debug, "GoalRect:  left:%d right:%d top:%d bottom:%d", GoalList[mStage].left, GoalList[mStage].right, GoalList[mStage].top, GoalList[mStage].bottom);
	TextOut(hdc, 0, 60, debug, strlen(debug));
	sprintf_s(debug, "GameState:%d SubStateSlected:%d Stage:%d", mGameState, mSubStateSlected, mStage);
	TextOut(hdc, 0, 75, debug, strlen(debug));
	sprintf_s(debug, "LeftEnimy: %d", EnimyList.size());
	TextOut(hdc, 0, 90, debug, strlen(debug));

	SetTextColor(hdc, old_fcol);
	SetBkColor(hdc, old_bcol);
	SetBkMode(hdc, old_tmode);
	if (FAILED(mlpDDSurefacePrimary->ReleaseDC(hdc)))
	{
		__POSTERR("DC release ERR")
			return;
	}

	return;
}

void TestApp::StageInit()
{
	using namespace std;
	RECT r;
	char FileName[50];
	int index = 0, x = 0, y = 0;
	string sz;

	sprintf_s(FileName, "rcs\\stage%d.uvcsource", mStage);
	ifstream fin(FileName);
	if (!fin.is_open()) return;

	while (fin >> index)
	{
		fin >> x >> y;
		EnimyList.push_back(CEnimy(EnimyDict[index], x, y));
	}

	fin.close();
}

void TestApp::KeyDownEvent(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_UP:
	case 'W':
	case 'X':
		switch (mGameState)
		{
		case __GAME_STATE_MENU__:
			mSubStateSlected -= 1;
			if (mSubStateSlected < 0)
				mSubStateSlected = 0;
			break;
		case __GAME_STATE_RUN__:
			Chara.Jump();
			Chara.Fall();
			break;
		}

		break;
	case VK_LEFT:
	case 'A':
		Chara.RunLeft();
		break;
	case VK_DOWN:
	case 'S':
		switch (mGameState)
		{
		case __GAME_STATE_MENU__:
			mSubStateSlected += 1;
			if (mSubStateSlected > 1)
				mSubStateSlected = 1;
			break;
		case __GAME_STATE_RUN__:
			break;
		default:break;
		}

		break;
	case VK_RIGHT:
	case 'D':
		Chara.RunRight();
		break;

	case 'Z':
	case VK_RETURN:
		switch (mGameState)
		{
		case __GAME_STATE_MENU__:
			switch (mSubStateSlected)
			{
			case __MENU_NEWGAME__:
				mGameState = __GAME_STATE_STGINIT__;
				mSubStateSlected = 0;
				Chara.Fall();
				break;
			case __MENU_EXIT__:
				PostQuitMessage(0);
				break;
			default:break;
			}
			break;

		case __GAME_STATE_RUN__:
		{
			Chara.Fireing();
		}
		break;

		default:
			break;
		}

		break;// end return.
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;// end ESC
	case VK_F2:
		mAppPaused = !mAppPaused;
		break;// end f2

	case VK_F3:
		ShowDebugInfo = !ShowDebugInfo;
		break;
	default:
		break;
	}
	return;
}// End KeyDown Event

void TestApp::KeyUpEvent(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_UP:
	case 'W':
		break;
	case VK_DOWN:
	case 'S':
		break;
	case VK_LEFT:
	case VK_RIGHT:
	case 'A':
	case 'D':
		Chara.StopRun();
		break;

	default:
		break;
	}
	return;
}

void TestApp::SetCameraPos(LONG x, LONG y)
{
	Camerax = x;
	Cameray = y;
	return;
}

LRESULT TestApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此处添加实现代码.
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			// when the Window is inactive, pause the game.
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;
		break;
	case WM_COMMAND:
	{
	}
	break;
	case WM_KEYDOWN:
		KeyDownEvent(wParam);
		break;
	case WM_KEYUP:
	{
		KeyUpEvent(wParam);
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_MOVE:
	{
		OnResize();
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

Role::Role()
{
	memset(&detectRect, 0, sizeof(RECT));

	mCharax = 0;
	mCharay = 0;
	mCharavx = 0;
	mCharavy = 0;
	mStdvx = 20;

	mAnimIndex = 0;
	mAnimIndexAddCount = 0;
	mJumpTimes = 2;

	mdie = 0;
	mfaceRight = true;
	mFall = false;

	LastFireTick = 0;
	mFire = false;

	mAtk = 10;
	mHp = 100;
	return;
}

Role::Role(int CharaX, int CharaY) :Role()
{
	mCharax = CharaX;
	mCharay = CharaY;
	mCharavx = 0;
	mCharavy = 0;

	mfaceRight = true;
	mFall = false;
	return;
}

UvcImage Role::GetBMP()
{
	// TODO: 在此处插入 return 语句
	if (mfaceRight)
	{
		if (mAnimIndex)
			return rAnim[Animlist[mAnimIndex % 6]];
		else
			return rAnim[0];
	}
	else
	{
		if (mAnimIndex)
			return lAnim[Animlist[mAnimIndex % 6]];
		else
			return lAnim[0];
	}
	return UvcImage();
}

inline void Role::Jump()
{
	if (mJumpTimes <= 0) return;
	mJumpTimes -= 1;
	mCharavy = -160;
	Fall();
	return;
}

void Role::StopRun()
{
	if (mCharavx)
		mCharavx = 0;
	return;
}

void Role::Move()
{
	Count();
	if (mFall)
	{
		if (mCharavy < 20)
			mCharavy = mCharavy + __G__;
	}

	mCharax = mCharax + mCharavx;
	mCharay = mCharay + mCharavy;

	if (abs(mCharavx) != 20)
	{
		mCharavx = 0;
	}

	// edge detect
	if (mCharax + uBmp.mBitMapInfoHeader.biWidth > mClientWidth)
		mCharax = mClientWidth - uBmp.mBitMapInfoHeader.biWidth;
	if (mCharax < 0)
		mCharax = 0;

	if (mCharay + uBmp.mBitMapInfoHeader.biHeight > mClientHeight)
	{
		mCharavy = 0;
		mCharay = mClientHeight - uBmp.mBitMapInfoHeader.biHeight;
		StopFall();
	}
	if (mCharay < 0)
	{
		mCharay = 0;
		mCharavy = 0;
	}

	// 不悬空不会产生y轴速度
	if (!mFall)
	{
		mCharavy = 0;
	}

	return;
}

void Role::SetDetectRECT(UINT Width, UINT Height)
{
	if (uBmp.mBitMapInfoHeader.biHeight <= Height)	Height = uBmp.mBitMapInfoHeader.biHeight;
	if (uBmp.mBitMapInfoHeader.biWidth <= Width)	Width = uBmp.mBitMapInfoHeader.biWidth;
	if (Width == 65535 && Height == 65535)
	{
		detectRect = uBmp.GetRect();
		return;
	}
	detectRect.left = (uBmp.mBitMapInfoHeader.biWidth - Width) / 2;
	detectRect.right = (uBmp.mBitMapInfoHeader.biWidth + Width) / 2;

	detectRect.top = (uBmp.mBitMapInfoHeader.biHeight - Height) / 2;
	detectRect.bottom = (uBmp.mBitMapInfoHeader.biHeight + Height) / 2;
	return;
}

void Role::Count()
{
	if (mCharavx)
	{
		mAnimIndex++;
	}

	if (!IsMoving())
	{
		mAnimIndex = 0;
		mAnimIndexAddCount = 0;
	}
	return;
}

inline void Role::Damage(ULONGLONG Dmg)
{
	if (Dmg >= mHp)
	{
		mdie = __DIE_STATE_DIE__;
		return;
	}
	else
	{
		mHp = mHp - Dmg;
	}
}

inline POINT Role::GetCenterPos()
{
	POINT pos;
	// x坐标中心点计算为角色x坐标加碰撞箱中点，y类似
	pos.x = ((detectRect.left + detectRect.right) >> 1) + mCharax;
	pos.y = ((detectRect.top + detectRect.bottom) >> 1) + mCharay;
	return pos;
}

void Role::PlotPixel(DDSURFACEDESC2& ddsd, int x, int y, int r, int g, int b, int a)
{
	if (ddsd.lpSurface == nullptr)	return;
	if (y <= 0)				return;
	if (x <= 0)				return;
	if (y > ddsd.dwHeight)	return;
	if (x > ddsd.dwWidth)	return;
	if (r < 0)				return;

	if (r == 0 && g == 255 && b == 0) return;
	UINT	pixelFor32Bit = 0;
	UINT* bufferFor32bit = (UINT*)ddsd.lpSurface;

	switch (uBmp.mBitCnt)
	{
		// __RGB 24BIT mode
	case 24:
	{
		pixelFor32Bit = __RGB32BIT(0, r, g, b);
		bufferFor32bit[x + (y * (int)(ddsd.lPitch >> 2))] = pixelFor32Bit;
	}
	break;
	// __RGB32BIT mode
	case 32:
	{
		pixelFor32Bit = __RGB32BIT(a, r, g, b);
		bufferFor32bit[x + (y * (int)(ddsd.lPitch >> 2))] = pixelFor32Bit;
	}
	break;
	default:
		break;
	}
	bufferFor32bit = nullptr;
	return;
}

void Role::PlotPixel(UvcImage UImg, int x, int y, int r, int g, int b, int a)
{
	UINT	pixelFor32Bit = 0;
	UINT* bufferFor32bit = (UINT*)UImg.mBuffer;

	switch (UImg.mBitCnt)
	{
		// __RGB 24BIT mode
	case 24:
	{
		pixelFor32Bit = __RGB32BIT(0, r, g, b);
		bufferFor32bit[x + (y * (int)(UImg.mBitMapInfoHeader.biWidth))] = pixelFor32Bit;
	}
	break;
	// __RGB32BIT mode
	case 32:
	{
		pixelFor32Bit = __RGB32BIT(a, r, g, b);
		bufferFor32bit[x + (y * (int)(UImg.mBitMapInfoHeader.biWidth))] = pixelFor32Bit;
	}
	break;
	default:
		break;
	}
	bufferFor32bit = nullptr;
	return;
}

void Role::SetClientSize(int Width, int Height)
{
	mClientWidth = Width;
	mClientHeight = Height;
	return;
}

CEnimy::CEnimy() :Object()
{
	mName = "";
	mHP = 0;
	mATK = 0;

	MaxAnimIndex = 0;
	mAnimIndex = 0;
	LImg = nullptr;
	RImg = nullptr;
	mAnimList.clear();
	mFaceRight = false;
	mDeath = false;

	vx = 0;
	vy = 0;
	x = 0;
	y = 0;

	mActive = false;
}

CEnimy::CEnimy(string Name) :CEnimy()
{
	mName = Name;

	return;
}

CEnimy::CEnimy(const CEnimy& src) :Object(src)
{
	mName = src.mName;
	mHP = src.mHP;
	mATK = src.mATK;
	MaxAnimIndex = src.MaxAnimIndex;
	LImg = new UvcImage[MaxAnimIndex];
	RImg = new UvcImage[MaxAnimIndex];
	for (int i = 0; i > MaxAnimIndex; i++)
	{
		LImg[i] = src.LImg[i];
		RImg[i] = src.RImg[i];
	}
	mAnimList.clear();

	vx = 0;
	vy = 0;
	x = 0;
	y = 0;
	return;
}

CEnimy::CEnimy(const CEnimy& src, int x, int y) :CEnimy(src)
{
	this->x = x;
	this->y = y;
	return;
}

inline void CEnimy::Damage(int Dmg)
{
	if (Dmg > mHP)
	{
		mDeath = true;
	}
	else
	{
		mHP = mHP - Dmg;
	}
	return;
}

void CEnimy::LoadInfo()
{
	string FileName = "rcs\\";
	FileName += mName;
	FileName += "\\config.uvcsource";
	using namespace std;
	ifstream fin(FileName);

	// 变量前置定义
	char sz[100] = {};
	int index = 0;
	int buffer = 0;

	// 文件格式
	// 第一行：动画帧总数
	// 第二行： 第一个数据：列表关键字数量
	// 第二行后续： 动画列表
	// 第三行：攻击力，生命值，属性值，移动力

	// 读取动画相关数据
	fin >> MaxAnimIndex;

	LImg = new UvcImage[MaxAnimIndex];
	RImg = new UvcImage[MaxAnimIndex];
	for (int i = 0; i < MaxAnimIndex; i++)
	{
		sprintf_s(sz, "rcs\\%s\\r%d", mName.c_str(), i);
		RImg[i].loadImage(sz);
		sprintf_s(sz, "rcs\\%s\\l%d", mName.c_str(), i);
		LImg[i].loadImage(sz);
	}

	Box = LImg[0].GetRect();

	// 读取动画序列关键帧
	fin >> index;
	for (int i = 0; i < index; i++)
	{
		fin >> buffer;
		mAnimList.push_back(buffer);
	}

	// 读取攻击 生命 属性 移动力
	// 因为空中单位会追加一项y轴运动速度，注意这个。
	fin >> mATK >> mHP >> Attr >> vx;
	if (Attr == FLY)
	{
		fin >> vy;
	}

	fin.close();
	return;
}

void CEnimy::Move(const TestApp& app)
{
	// 变量前置声明
	int Judx = 0;
	int Judy = 0;
	int flag = 0;
	int buffer;

	// 离开视野不再活动
	if (x < app.Camerax || x >(app.Camerax + app.mClientWidth))
		mActive = false;
	else
		mActive = true;

	if (mActive == false)
		return;

	// 先进行x轴方向速度检测和碰撞检测
	if (curvx != 0)
	{
		if (curvx > 0) flag = 1;
		if (curvx < 0) flag = -1;
		// i表示x轴要检测的坐标加值
		Judx = (mFaceRight ? x + Box.right : x + Box.left);
		for (int i = 0; i < flag * curvx; i = i + 1)
		{
			// j表示y轴要检测的坐标加值
			for (int j = Box.top; j < Box.bottom; j++)
			{
				Judy = y + j;
				if (app.GetPixelRGB(Judx + (i * flag), Judy) != TestApp::bkg.GetPixelRGB(Judx + (i * flag), Judy))
				{
					curvx = i * flag;
				}
			}
			if (abs(vx) != abs(curvx))
			{
				x = x + curvx;
				curvx = 0;
				break;
			}
		}
	}// 结束x轴的判定
	// 开始y轴判定
	if (Attr != FLY)
	{
		if (mFall)
		{
			for (int j = 0; j < curvy; j++)
			{
				for (int i = Box.left; i < Box.right; i++)
				{
					// 计算要检测的值时一定记住要算相对坐标
					// 要检测的x值范围为 碰撞箱左 -- 碰撞箱右
					Judx = x - app.Camerax + i;
					// 要检测的y值范围为 碰撞箱下 -- 碰撞箱下+ y轴方向速度
					Judy = y - app.Cameray + Box.bottom + j;

					// 超出边界没有检测必要
					if (Judx > Map.mBitMapInfoHeader.biWidth || Judx < 0) break;
					if (Judy > Map.mBitMapInfoHeader.biHeight || Judy < 0) break;

					//逐像素比对,有墙就站住
					// 因为都是在主表面上进行检测所以并不用减摄像机算相对坐标
					if (app.GetPixelRGB(Judx, Judy) != TestApp::bkg.GetPixelRGB(Judx, Judy))
					{
						curvy = j;
						mFall = false;
					}
				}
				// 这里改过说明之前的循环中已经发现异常，可以退出循环
				if (!mFall) break;
			}
		}
		// 不在掉落的时候进行掉落检测
		else
		{
			flag = 0;
			for (int i = Box.left; i < Box.right; i++)
			{
				// 碰撞检测起止：
				// x轴： 碰撞箱左右
				Judx = x - app.Camerax + i;
				Judy = y - app.Cameray + Box.bottom + 1;
				if (app.GetPixelRGB(Judx, Judy) == TestApp::bkg.GetPixelRGB(Judx, Judy))
				{
					// 此处flag变量用于保存每行中与背景色一致的像素个数
					flag++;
				}
			}
			// 发现人物已经完全离开平台
			if (flag >= (Box.right - Box.left))
			{
				mFall = true;
			}
		}// end else
	}// 结束掉落判定

	// 开始坐标运算
	if (mFall)
	{
		if (curvy < 20)
			curvy = curvy + __G__;
	}

	x = x + curvx;
	y = y + curvy;

	if (abs(curvx) != vx)
	{
		curvx = 0;
	}

	// edge detect
	if (x + Box.right > Map.mBitMapInfoHeader.biWidth)
		x = Map.mBitMapInfoHeader.biWidth - Box.right;
	if (x < 0)
		x = 0;

	if (y + Box.bottom > Map.mBitMapInfoHeader.biHeight)
	{
		curvy = 0;
		y = Map.mBitMapInfoHeader.biHeight - Box.bottom;
		mFall = false;
	}
	if (y < 0)
	{
		y = 0;
		curvy = 0;
	}

	// 不悬空不会产生y轴速度
	if (!mFall)
	{
		curvy = 0;
	}

	return;
}

UvcImage CEnimy::GetCurImage()
{
	if (mFaceRight)
	{
		if (curvx != 0)
		{
			return RImg[mAnimList[mAnimIndex]];
		}
		else
		{
			return RImg[0];
		}
	}
	else
	{
		if (curvx != 0)
		{
			return LImg[mAnimList[mAnimIndex]];
		}
		else
		{
			return LImg[0];
		}
	}
	return UvcImage();
}

void CEnimy::Count()
{
	if (vx) mAnimIndex = mAnimIndex + 1;

	if (mAnimIndex > mAnimList.size())
		mAnimIndex = 0;
}

inline bool IsIn(POINT p, RECT rect)
{
	if (p.x >= rect.left && p.x < rect.right && p.y < rect.bottom && p.y >= rect.top) return true;
	return false;
}

/*

				flag = 0;
				// 如果卡墙里了
				for (int i = Chara.detectRect.left; i < Chara.detectRect.right; i++)
				{
					judx = Chara.GetX() + i;
					judy = Chara.GetY() + Chara.detectRect.bottom - 1;
					// 由于这个时候还没有画人物所以可以这么检测：
					if (GetPixelRGB(judx, judy) != Role::bkg.GetPixelRGB(judx, judy))
					{
						flag += 1;// 设置一个判定的特殊值,说明卡墙里了
						break;
					}
				}
				// 如果没卡墙里
				// 进行落地检测
				if (flag <= 3)
switch (mGameState)
	{
	case __GAME_STATE_MENU__:
	{
	}
	break;
	case __GAME_STATE_RUN__:
	{
		Draw(Timer);
		RECT src, dest;
		src.left = Role::mCamerax;
		src.right = Role::mCamerax + Role::mClientx;

		src.top = Role::mCameray;
		src.bottom = Role::mCameray + Role::mClienty;

		dest.left = 0;
		dest.top = 0;
		dest.right = mClientWidth;
		dest.left = mClientHeight;

		if (FAILED(mlpDDSurefacePrimary->Blt(&dest, Role::canvas, &src, DDBLT_WAIT, NULL)))
		{
			__POSTERR("Blt Failed");
		}
	}
	break;
	default:break;
	}
DDSURFACEDESC2 ddsd;
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwHeight = Height; ddsd.dwWidth = Width;

		if (FAILED(app.mlpDD7->CreateSurface(&ddsd, &Role::canvas, NULL)))
		{
			throw 3;
		}
		Role::GetCanvasPF();

abandon.

void static DrawBkgToCanvas();
	void static GetCanvasPF();

void Role::DrawBkgToCanvas()
{
	RECT SrcRect = { 0,0,mClientx, mClienty };
	UCHAR r = 0, g = 0, b = 0;

	for (int j = 0; j < mClienty; j++)
	{
		for (int i = 0; i < mClientx; i++)
		{
			Role::bkg.GetPixelRGB(SrcRect.left + i, SrcRect.top + j, r, g, b);
			//PlotPixel(ddsd, mClientRect.left + x, mClientRect.top + y, r, g, b);
			PlotPixel(Role::canvas, i, j, r, g, b);
		}
	}

	return;
}

void Role::GetCanvasPF()
{
	//memset(&CanvasPF, 0, sizeof(DDPIXELFORMAT));
	//CanvasPF.dwSize = sizeof(DDPIXELFORMAT);
	////CanvasPF.dwSize = sizeof(decltype(CanvasPF));

	//canvas->GetPixelFormat(&CanvasPF);
	return;
}

	void DrawImageToCanvas();
void Role::DrawImageToCanvas()
{
	UvcImage* tag = nullptr;
	if (mfaceRight)
	{
		if (mAnimIndex)
			tag = &rAnim[Animlist[mAnimIndex % 6]];
		else
			tag = &rAnim[0];
	}
	else
	{
		if (mAnimIndex)
			tag = &lAnim[Animlist[mAnimIndex % 6]];
		else
			tag = &lAnim[0];
	}

	RECT SrcRect = tag->GetRect();
	UCHAR r = 0, g = 0, b = 0;

	for (int j = 0; j < SrcRect.bottom - SrcRect.top; j++)
	{
		for (int i = 0; i < SrcRect.right - SrcRect.left; i++)
		{
			tag->GetPixelRGB(SrcRect.left + i, SrcRect.top + j, r, g, b);
			//PlotPixel(ddsd, mClientRect.left + x, mClientRect.top + y, r, g, b);
			PlotPixel(canvas, mCharax + i, mCharay + j, r, g, b);
		}
	}

	return;
}

if (Chara.mCharay + 20 < mClientRect.bottom)
				Chara.mCharavy = 20;

if (Chara.faceRight)
		{
			if (g_AnimIndex)
				DrawImageToDDSurface(Chara.rAnim[Animlist[g_AnimIndex % 6]], mDDSD, Chara.rAnim[Animlist[g_AnimIndex % 6]].GetRect(), Chara.mCharax, Chara.mCharay);
			else
				DrawImageToDDSurface(Chara.rAnim[0], mDDSD, Chara.rAnim[0].GetRect(), Chara.mCharax, Chara.mCharay);
		}
		else
		{
			if (g_AnimIndex)
				DrawImageToDDSurface(Chara.lAnim[Animlist[g_AnimIndex % 6]], mDDSD, Chara.lAnim[Animlist[g_AnimIndex % 6]].GetRect(), Chara.mCharax, Chara.mCharay);
			else
				DrawImageToDDSurface(Chara.lAnim[0], mDDSD, Chara.lAnim[0].GetRect(), Chara.mCharax, Chara.mCharay);
		}

int lpitch32 = (int)(mDDSD.lPitch >> 2);
	UINT* video_buffer = (UINT*)mDDSD.lpSurface;

	auto rd = [](int max = 255) {return rand() % max; };

	//DrawImageToDDSurface(uBmp, mDDSD, uBmp.GetRect(), rd() + 100, rd() + 100);
	//DrawImageToDDSurface(bkg, mDDSD, bkg.GetRect(), 0, 0);
	//DrawImageToDDSurface(uBmp, mDDSD, uBmp.GetRect(), mCharax, mCharay);
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	//char sz[100] = { 0 };
	//sprintf_s(sz, "Chara point:x = %d, y = %d", mCharax, mCharay);
	//__POSTERR(sz);
	//__POSTERR("Next Frame?");

	for (int i = 0; i < 1000; i++)
	{
		video_buffer[rd(mClientWidth) + rd(mClientHeight) * lpitch32] = __RGB32BIT(rd(), rd(), rd(), rd());
	}
	// edge detect
		if (Chara.mCharax + uBmp.mBitMapInfoHeader.biWidth > mClientWidth)
			Chara.mCharax = mClientWidth - uBmp.mBitMapInfoHeader.biWidth;
		if (Chara.mCharax < 0)
			Chara.mCharax = 0;
		if (Chara.mCharay + uBmp.mBitMapInfoHeader.biHeight > mClientHeight)
			Chara.mCharay = mClientHeight - uBmp.mBitMapInfoHeader.biHeight;
		if (Chara.mCharay < 0)
			Chara.mCharay = 0;

*/