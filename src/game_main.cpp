#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"

yyInputContext*		g_inputContex = nullptr;
yyVideoDriverAPI*	g_videoDriver = nullptr;
yyGUIFont*			g_defaultFont = 0;
v2f					g_windowSize;
v2f					g_rawInputCursorPosition;
SpriteCache *		g_spriteCachePtr = 0;
v2f					g_gameCursorPosition;
v2f*				g_spriteCameraPosition = 0;
v2f					g_screenHalfSize;
Map*				g_map = 0;
v2f					g_cameraLimits;
f32					g_screenRectRadius = 0.f;
yySprite*			g_spriteGrid = 0;
yySprite*			g_spriteGridRed = 0;

yyGUIButton* g_GUIPlayPauseButton = 0;
yyGUIButton* g_GUIContinueButton = 0;
yyGUIButton* g_GUIExitButton = 0;
yyGUIPictureBox* g_GUIMenuBG = 0;
bool g_isPause = false;

void EnterGameMenu()
{
	g_isPause = true;
	g_GUIMenuBG->SetVisible(true);
	g_GUIContinueButton->SetVisible(true);
	g_GUIExitButton->SetVisible(true);
}
void LeaveGameMenu()
{
	g_isPause = false;
	g_GUIMenuBG->SetVisible(false);
	g_GUIContinueButton->SetVisible(false);
	g_GUIExitButton->SetVisible(false);
	g_GUIPlayPauseButton->m_isChecked = false;
}

void PlayPauseButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	if (g_isPause)
	{
		LeaveGameMenu();
	}
	else
	{
		EnterGameMenu();
	}
}
void ContinueButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	LeaveGameMenu();
}
void ExitButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	yyQuit();
}

v2f GetSpriteSize(yySprite* sprite) 
{
	assert(sprite);
	v2i size;
	yyGetTextureSize(sprite->m_texture, &size);
	return v2f((f32)size.x, (f32)size.y);
}

yySprite* GetSprite(const char* file) 
{
	for (u16 i = 0, sz = g_spriteCachePtr->m_cache.size(); i < sz; ++i) {
		if (strcmp(g_spriteCachePtr->m_cache[i].m_path.data(), file) == 0) {
			return g_spriteCachePtr->m_cache[i].m_sprite;
		}
	}
	yyResource* texture = yyGetTextureResource(file, false, false, true);
	v2i textureSize;
	yyGetTextureSize(texture, &textureSize);
	yySprite* newSprite = yyCreateSprite(v4f(0.f, 0.f, (f32)textureSize.x, (f32)textureSize.y), texture, true);
	SpriteCacheNode n;
	n.m_path = file;
	n.m_sprite = newSprite;
	g_spriteCachePtr->m_cache.push_back(n);
	return newSprite;
}

// for auto create\delete
struct yyEngineContext
{
	yyEngineContext()
	{
		m_state = nullptr;
	}
	~yyEngineContext()
	{
		yyStop(); // destroy main class, free memory
	}

	void init(yyInputContext* input)
	{
		m_state = yyStart(input); // allocate memory for main class inside yuyu.dll
	}

	yySystemState * m_state;
};

void window_onCLose(yyWindow* window)
{
	yyQuit(); // change yySystemState - set yySystemState::Quit
}

void log_onError(const char* message)
{
	auto l = strlen(message);
	if (l < 1)
		return;

	FILE * f = fopen("log.txt", "a");
	fwrite(message, 1, l, f);
	fclose(f);
}
void log_onInfo(const char* message)
{
	auto l = strlen(message);
	if (l < 1)
		return;

	FILE * f = fopen("log.txt", "a");
	fwrite(message, 1, l, f);
	fclose(f);
}

void window_callbackMouse(yyWindow* w, s32 wheel, s32 x, s32 y, u32 click)
{
	g_inputContex->m_cursorCoords.x = (f32)x;
	g_inputContex->m_cursorCoords.y = (f32)y;

	g_inputContex->m_mouseDelta.x = (f32)x - g_inputContex->m_cursorCoordsOld.x;
	g_inputContex->m_mouseDelta.y = (f32)y - g_inputContex->m_cursorCoordsOld.y;

	g_inputContex->m_cursorCoordsOld = g_inputContex->m_cursorCoords;

	if (click & yyWindow_mouseClickMask_LMB_DOWN)
	{
		g_inputContex->m_isLMBDown = true;
		g_inputContex->m_isLMBHold = true;
	}
	if (click & yyWindow_mouseClickMask_LMB_UP)
	{
		g_inputContex->m_isLMBHold = false;
		g_inputContex->m_isLMBUp = true;
	}
	if (click & yyWindow_mouseClickMask_LMB_DOUBLE)
	{
		g_inputContex->m_isLMBDbl = true;
	}

	if (click & yyWindow_mouseClickMask_RMB_DOWN)
	{
		g_inputContex->m_isRMBDown = true;
		g_inputContex->m_isRMBHold = true;
	}
	if (click & yyWindow_mouseClickMask_RMB_UP)
	{
		g_inputContex->m_isRMBHold = false;
	}
}
void updateInputContext() // call before all callbacks
{
	g_inputContex->m_isLMBDbl = false;
	g_inputContex->m_isLMBDown = false;
	g_inputContex->m_isRMBDown = false;
	g_inputContex->m_isLMBUp = false;
	g_inputContex->m_mouseDelta.x = 0.f;
	g_inputContex->m_mouseDelta.y = 0.f;
	g_inputContex->m_cursorCoordsForGUI = g_inputContex->m_cursorCoords;

	memset(g_inputContex->m_key_pressed, 0, sizeof(u8) * 256);
	memset(g_inputContex->m_key_hit, 0, sizeof(u8) * 256);
}

void window_callbackKeyboard(yyWindow*, bool isPress, u32 key, char16_t character)
{
	if (isPress)
	{
		if (key < 256)
		{
			g_inputContex->m_key_hold[key] = 1;
			g_inputContex->m_key_hit[key] = 1;
		}
	}
	else
	{
		if (key < 256)
		{
			g_inputContex->m_key_hold[key] = 0;
			g_inputContex->m_key_pressed[key] = 1;
		}
	}
}

void OnRawInput(yyWindow* window, bool inForeground, void* ri)
{
	static std::vector<char> m_RawInputMessageData;

	HRAWINPUT hRawInput = (HRAWINPUT)ri;
	UINT dataSize;
	GetRawInputData(
		hRawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

	if (dataSize == 0)
		return;
	if (dataSize > m_RawInputMessageData.size())
		m_RawInputMessageData.resize(dataSize);

	void* dataBuf = &m_RawInputMessageData[0];
	GetRawInputData(
		hRawInput, RID_INPUT, dataBuf, &dataSize, sizeof(RAWINPUTHEADER));

	const RAWINPUT *raw = (const RAWINPUT*)dataBuf;
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		HANDLE deviceHandle = raw->header.hDevice;

		const RAWMOUSE& mouseData = raw->data.mouse;

		USHORT flags = mouseData.usButtonFlags;
		short wheelDelta = (short)mouseData.usButtonData;
		LONG x = mouseData.lLastX, y = mouseData.lLastY;

	//	wprintf(
	//		L"Mouse: Device=0x%08X, Flags=%04x, WheelDelta=%d, X=%d, Y=%d\n",
	//		deviceHandle, flags, wheelDelta, x, y);
		g_rawInputCursorPosition.x = (f32)x;
		g_rawInputCursorPosition.y = (f32)y;
	}
}


int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd)
{
	yyStringA cmdLine = lpCmdLine;
	std::vector<yyStringA> cmdLineArr;
	util::stringGetWords(&cmdLineArr, cmdLine);

	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	{
		FILE * f = fopen("log.txt", "w");
		fwrite(" ", 1, 1, f);
		fclose(f);
	}

	bool isEditorMode = false;
	for (auto s : cmdLineArr)
	{
		//printf("CMD: %s\n", s.data());
		if (strcmp(s.data(), "editor") == 0)
			isEditorMode = true;
	}

	const char * videoDriverType = "opengl.yyvd"; // for example read name from .ini
	
	wchar_t videoDriverTypeBuffer[50];
	memset(videoDriverTypeBuffer, 0, 50 * sizeof(wchar_t));
	GetPrivateProfileString(L"video", L"driver", L"opengl.yyvd", videoDriverTypeBuffer, 50, L"../params.ini");


	v2f mainTargetSize;
	mainTargetSize.x = (f32)GetPrivateProfileInt(L"screen", L"x", 800, L"../params.ini");
	mainTargetSize.y = (f32)GetPrivateProfileInt(L"screen", L"y", 600, L"../params.ini");
	
	int isFullscreen = GetPrivateProfileInt(L"screen", L"fullscreen", 0, L"../params.ini");


	yyStringA videoDriverTypeStr = videoDriverTypeBuffer;
	if (!videoDriverTypeStr.size())
		videoDriverTypeStr = videoDriverType;
	if (!videoDriverTypeStr.size())
		videoDriverTypeStr = videoDriverType;

	if (isEditorMode)
	{
		videoDriverTypeStr = videoDriverType;
	}

	// I don't want to use stack memory, so for class\struct I will create new objects using heap
	// use yyPtr if you want auto destroy objects
	yyPtr<yyInputContext> inputContext = yyCreate<yyInputContext>();
	g_inputContex = inputContext.m_data;

	yyPtr<yyEngineContext> engineContext = yyCreate<yyEngineContext>();
	engineContext.m_data->init(inputContext.m_data);
	auto p_engineContext = engineContext.m_data;


	// set callbacks if you want to read some information 
	yyLogSetErrorOutput(log_onError);
	yyLogSetInfoOutput(log_onInfo);
	yyLogSetWarningOutput(log_onError);

	yyPtr<yyWindow> window = yyCreate<yyWindow>();
	auto p_window = window.m_data;


	if (!p_window->init(mainTargetSize.x, mainTargetSize.y, 0))
	{
		YY_PRINT_FAILED;
		return 1;
	}

	// save pointer
	yySetMainWindow(window.m_data);
	window.m_data = yyGetMainWindow(); 

	p_window->m_onClose = window_onCLose;
	p_window->m_onMouseButton = window_callbackMouse;
	p_window->m_onKeyboard = window_callbackKeyboard;
	p_window->m_onRawInput = OnRawInput;

	// init video driver	
	if( !yyInitVideoDriver(videoDriverTypeStr.c_str(), p_window) )
	{
		yyLogWriteWarning("Can't load video driver : %s\n", videoDriverTypeStr.c_str());

		// if failed, try to init other type
		std::vector<std::string> vidDrivers;
		
		for(auto & entry : yyFS::directory_iterator(yyFS::current_path() ) )
		{

			auto path = entry.path();
			if(path.has_extension())
			{
				auto ex = path.extension();
				if(ex == ".yyvd")
				{
					yyLogWriteWarning("Trying to load video driver : %s\n", path.generic_string().c_str());

					if( yyInitVideoDriver(path.generic_string().c_str(), p_window) )
					{
						goto vidOk;
					}
					else
					{
						yyLogWriteWarning("Can't load video driver : %s\n", path.generic_string().c_str());
					}
				}
			}
		}
		YY_PRINT_FAILED;
		return 1;
	}

vidOk:
	

	g_videoDriver = yyGetVideoDriverAPI();
	g_videoDriver->SetClearColor(0.3f,0.3f,0.74f,1.f);
	bool useImgui = isEditorMode;

	if (useImgui)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(p_window->m_hWnd);
		gl3wInit();
		const char* glsl_version = "#version 330";
		ImGui_ImplOpenGL3_Init(glsl_version);
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	}

	
	yyStringA title = "3Dhater RTS - ";
	title += g_videoDriver->GetVideoDriverName();
	p_window->SetTitle(title.data());

//	auto testTextureGPU = g_videoDriver->CreateTextureFromFile("../res/textures/test.dds", true, false, true);
//	auto gui_pictureBox = yyGUICreatePictureBox(v4f(0.f, 0.f, 100.f, 100.f), testTextureGPU, 1);
	
	yyPtr<yySprite> spriteGrid = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_white.png", false, false, true), false);
	g_spriteGrid = spriteGrid.m_data;
	yyPtr<yySprite> spriteGridRed = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_red.png", false, false, true), false);
	g_spriteGridRed = spriteGridRed.m_data;

	//yySprite* spriteHero = yyCreateSprite(v4f(0.f, 0.f, 50.f, 76.f), g_videoDriver->CreateTextureFromFile("../res/GA3E/hero0.png", false, false, true), true);
	//spriteHero->SetMainFrame(123, 8, 174, 85);
	//spriteHero->m_objectBase.m_localPosition.set(10.f, 20.f, 0.f, 0.f);
	//auto stateIdleRight = spriteHero->AddState("IdleRight");
	//stateIdleRight->m_isAnimation = true;
	////stateIdle->m_invertX = true;
	////stateIdle->m_invertY = true;
	//stateIdleRight->AddAnimationFrame(0, 0, 38, 84);
	//stateIdleRight->AddAnimationFrame(41, 0, 79, 84);
	//stateIdleRight->SetFPS(2.f);
	//auto stateIdleLeft = spriteHero->AddState("IdleLeft");
	//stateIdleLeft->m_isAnimation = true;
	//stateIdleLeft->m_invertX = true;
	//stateIdleLeft->AddAnimationFrame(0, 0, 38, 84);
	//stateIdleLeft->AddAnimationFrame(41, 0, 79, 84);
	//stateIdleLeft->SetFPS(2.f);
	//spriteHero->SetState(stateIdleRight);
	g_spriteCameraPosition = g_videoDriver->GetSpriteCameraPosition();
	auto spriteCameraScale = g_videoDriver->GetSpriteCameraScale();


	f32 aspect = (f32)p_window->m_currentSize.x / (f32)p_window->m_currentSize.y;
	spriteCameraScale->x = 1.f;
	spriteCameraScale->y = 1.f;

	


	g_map = new Map;

	v2f curCoordsOnPressRMB;
	g_screenHalfSize = v2f(p_window->m_currentSize.x, p_window->m_currentSize.y) * 0.5f;
	g_screenRectRadius = g_screenHalfSize.distance(v2f()) * 2.f;

	
	GameCursor cursor_arrow;
	cursor_arrow.init("../res/textures/gui/cursors1.png", v2f(60.f, 61.f), v2i(17, 11), v2i(75, 70));
	GameCursor * activeCursor = &cursor_arrow;
	v2f gameCursorLimits;
	gameCursorLimits.x = p_window->m_currentSize.x - 20;
	gameCursorLimits.y = p_window->m_currentSize.y - 20;
	g_gameCursorPosition.x = g_screenHalfSize.x;
	g_gameCursorPosition.y = g_screenHalfSize.y;
	v2f cursorOld;

	SpriteCache* spriteCache = new SpriteCache;
	g_spriteCachePtr = spriteCache;


	auto noto_font = yyGUILoadFont("../res/fonts/Noto/notosans.txt");
	g_defaultFont = noto_font;

	if (isFullscreen == 1)
	{
		p_window->ToFullscreenMode();
		g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, mainTargetSize);

		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = p_window->m_currentSize.x;
		rc.bottom = p_window->m_currentSize.y;
		ClipCursor(&rc);
	}
//	ShowCursor(FALSE);

	g_videoDriver->UseBlend(false);

	g_GUIMenuBG = yyGUICreatePictureBox(v4f(0.f, 0.f, 512.f, 512.f), yyGetTextureResource("../res/gui/menubg.png", false, false, true), -1);
	g_GUIMenuBG->SetVisible(false);
	g_GUIMenuBG->IgnoreInput(true);

	g_GUIPlayPauseButton = yyGUICreateButton(v4f(0.f, 0.f, 32.f, 32.f), yyGetTextureResource("../res/gui/pause1.png", false, false, true), -1);
	g_GUIPlayPauseButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/pause2.png", false, false, true));
	g_GUIPlayPauseButton->m_useAsCheckbox = true;
	//g_GUIPlayPauseButton->m_onRelease = PlayPauseButton_onRelease;
	g_GUIPlayPauseButton->m_onClick = PlayPauseButton_onRelease;

	g_GUIContinueButton = yyGUICreateButton(v4f(0.f, 0.f, 250.f, 40.f), yyGetTextureResource("../res/gui/continue1.png", false, false, true), -1);
	g_GUIContinueButton->SetMouseHoverTexture(yyGetTextureResource("../res/gui/continue2.png", false, false, true));
	g_GUIContinueButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/continue3.png", false, false, true));
	g_GUIContinueButton->m_offset.set(50.f, 50.f);
	g_GUIContinueButton->SetVisible(false);
	g_GUIContinueButton->m_onRelease = ContinueButton_onRelease;

	g_GUIExitButton = yyGUICreateButton(v4f(0.f, 0.f, 250.f, 40.f), yyGetTextureResource("../res/gui/exit1.png", false, false, true), -1);
	g_GUIExitButton->SetMouseHoverTexture(yyGetTextureResource("../res/gui/exit2.png", false, false, true));
	g_GUIExitButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/exit3.png", false, false, true));
	g_GUIExitButton->m_offset.set(50.f, 100.f);
	g_GUIExitButton->SetVisible(false);
	g_GUIExitButton->m_onRelease = ExitButton_onRelease;

	f32 deltaTime = 0.f;
	bool run = true;
	while( run )
	{
		//v2f cameraMoveVector;

		static u64 t1 = 0;
		u64 t2 = yyGetTime();
		f32 m_tick = f32(t2 - t1);
		t1 = t2;
		deltaTime = m_tick / 1000.f;

		updateInputContext();
		g_inputContex->m_cursorCoordsForGUI = g_gameCursorPosition;

#ifdef YY_PLATFORM_WINDOWS
		MSG msg;
		while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			GetMessage( &msg, NULL, 0, 0 );
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
#else
#error For windows
#endif

		auto screenCenterRoundX = std::floor(g_screenHalfSize.x);
		auto screenCenterRoundY = std::floor(g_screenHalfSize.y);
		g_inputContex->m_mouseDelta.x = g_rawInputCursorPosition.x - cursorOld.x;
		g_inputContex->m_mouseDelta.y = g_rawInputCursorPosition.y - cursorOld.y;
		cursorOld.x = g_rawInputCursorPosition.x;
		cursorOld.y = g_rawInputCursorPosition.y;
		
		yyGUIUpdate(deltaTime);


		//printf("%f %f\n", spriteCameraPosition->x, spriteCameraPosition->y);
		//printf("%f %f - %f %f\n", gameCursorPosition.x, gameCursorPosition.y,
		//	gameCursorLimits.x, gameCursorLimits.y);
		//printf("%f %f - %f %f\n", g_inputContex->m_cursorCoords.x, g_inputContex->m_cursorCoords.y, 
		//	g_inputContex->m_mouseDelta.x, g_inputContex->m_mouseDelta.y);


		switch(*p_engineContext->m_state)
		{
		default:
			run = false;
			break;
		case yySystemState::Run:
		{
			/*if (g_inputContex->m_isRMBDown)
				curCoordsOnPressRMB = g_inputContex->m_cursorCoords;
			if (g_inputContex->m_isRMBHold){
				cameraMoveVector = g_inputContex->m_cursorCoords - curCoordsOnPressRMB;
				cameraMoveVector *= deltaTime;
			}*/

			g_gameCursorPosition.x += g_inputContex->m_mouseDelta.x * 1.9;
			g_gameCursorPosition.y += g_inputContex->m_mouseDelta.y * 1.9;

			f32  spriteCameraMoveSpeed = 1000.f;
			f32  spriteCameraScaleSpeed = 1.f;

			bool findCell = false;

			if (g_inputContex->isKeyHold(yyKey::K_CTRL) || g_inputContex->isKeyHold(yyKey::K_LCTRL)
				|| g_inputContex->isKeyHold(yyKey::K_RCTRL))
			{
				spriteCameraMoveSpeed = 100.f;
				if (g_inputContex->isKeyHold(yyKey::K_SHIFT) || g_inputContex->isKeyHold(yyKey::K_LSHIFT)
					|| g_inputContex->isKeyHold(yyKey::K_RSHIFT))
				{
					spriteCameraMoveSpeed *= deltaTime;
				}

				if (g_inputContex->isKeyHold(yyKey::K_LEFT))
					g_gameCursorPosition.x -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_RIGHT))
					g_gameCursorPosition.x += spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_UP))
					g_gameCursorPosition.y -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_DOWN))
					g_gameCursorPosition.y += spriteCameraMoveSpeed * deltaTime;
			}
			else
			{
				if (g_inputContex->isKeyHold(yyKey::K_LEFT))
				{
					findCell = true;
					g_spriteCameraPosition->x -= spriteCameraMoveSpeed * deltaTime;
				}
				if (g_inputContex->isKeyHold(yyKey::K_RIGHT))
				{
					findCell = true;
					g_spriteCameraPosition->x += spriteCameraMoveSpeed * deltaTime;
				}
				if (g_inputContex->isKeyHold(yyKey::K_UP))
				{
					findCell = true;
					g_spriteCameraPosition->y -= spriteCameraMoveSpeed * deltaTime;
				}
				if (g_inputContex->isKeyHold(yyKey::K_DOWN))
				{
					findCell = true;
					g_spriteCameraPosition->y += spriteCameraMoveSpeed * deltaTime;
				}
				if (g_inputContex->isKeyHold(yyKey::K_NUM_7))
					spriteCameraScale->x -= spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_9))
					spriteCameraScale->x += spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_1))
					spriteCameraScale->y -= spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_3))
					spriteCameraScale->y += spriteCameraScaleSpeed * deltaTime;
			}

			if (g_gameCursorPosition.x < 0.f) g_gameCursorPosition.x = 0.f;
			if (g_gameCursorPosition.y < 0.f) g_gameCursorPosition.y = 0.f;
			if (g_gameCursorPosition.x > gameCursorLimits.x) g_gameCursorPosition.x = gameCursorLimits.x;
			if (g_gameCursorPosition.y > gameCursorLimits.y) g_gameCursorPosition.y = gameCursorLimits.y;

			const f32 scroll_zone = 25.f;
			if (g_gameCursorPosition.x < scroll_zone)
			{
				g_spriteCameraPosition->x -= (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (g_gameCursorPosition.x > gameCursorLimits.x - scroll_zone)
			{
				g_spriteCameraPosition->x += (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (g_gameCursorPosition.y < scroll_zone)
			{
				g_spriteCameraPosition->y -= (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (g_gameCursorPosition.y > gameCursorLimits.y - scroll_zone)
			{
				g_spriteCameraPosition->y += (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}

		
			if (g_spriteCameraPosition->x > g_cameraLimits.x) g_spriteCameraPosition->x = g_cameraLimits.x;
			if (g_spriteCameraPosition->y > g_cameraLimits.y) g_spriteCameraPosition->y = g_cameraLimits.y;
			if (g_spriteCameraPosition->x < 0.f) g_spriteCameraPosition->x = 0.f;
			if (g_spriteCameraPosition->y < 0.f) g_spriteCameraPosition->y = 0.f;

			g_spriteCameraPosition->x = std::floor(g_spriteCameraPosition->x);
			g_spriteCameraPosition->y = std::floor(g_spriteCameraPosition->y);
			g_gameCursorPosition.x = std::floor(g_gameCursorPosition.x);
			g_gameCursorPosition.y = std::floor(g_gameCursorPosition.y);

			if (findCell)
			{
				g_map->FindCellPosition();
			}
			

			if (g_inputContex->isKeyHold(yyKey::K_F12))
			{
				p_window->ToFullscreenMode();
				g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, mainTargetSize);
				//screenHalfSize = v2f(p_window->m_currentSize.x, p_window->m_currentSize.y) * 0.5f;
			}
			if (g_inputContex->isKeyHold(yyKey::K_F11))
			{
				p_window->ToWindowMode();
				g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, mainTargetSize);
			//	screenHalfSize = v2f(p_window->m_currentSize.x, p_window->m_currentSize.y) * 0.5f;
			}
			if (g_inputContex->isKeyHold(yyKey::K_F10))
				g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, v2f(p_window->m_currentSize.x / 2, p_window->m_currentSize.y / 2));
			if (g_inputContex->isKeyHold(yyKey::K_F9))
				g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, v2f(p_window->m_currentSize.x / 4, p_window->m_currentSize.y / 4));
			if (g_inputContex->isKeyHold(yyKey::K_F8))
				g_videoDriver->UpdateMainRenderTarget(p_window->m_currentSize, v2f(p_window->m_currentSize.x / 8, p_window->m_currentSize.y / 8));

			g_videoDriver->BeginDraw();
			g_videoDriver->ClearAll();

			if (isEditorMode)
				EditorStep(deltaTime);
			else
				GameStep(deltaTime);

			

			/*g_videoDriver->DrawSprite(spriteLevel);
			spriteHero->m_objectBase.UpdateBase();
			spriteHero->Update(deltaTime);
			g_videoDriver->DrawSprite(spriteHero);*/

			yyGUIDrawAll();

			v2f spriteCameraPositionSave = *g_spriteCameraPosition;
			g_spriteCameraPosition->set(0.f, 0.f);
			activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].x = g_gameCursorPosition.x - g_screenHalfSize.x;
			activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].y = g_gameCursorPosition.y - g_screenHalfSize.y;
			g_videoDriver->DrawSprite(activeCursor->m_sprite);

			*g_spriteCameraPosition = spriteCameraPositionSave;


			g_videoDriver->EndDraw();
			if (useImgui)
			{
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}
			g_videoDriver->SwapBuffers();

			if (!g_inputContex->isKeyHold(yyKey::K_LCTRL))
			{
	//			ShowCursor(FALSE);
				g_inputContex->m_cursorCoordsOld.set(screenCenterRoundX, screenCenterRoundY);
				if (g_inputContex->m_mouseDelta.x != 0.f) cursorOld.x -= g_inputContex->m_mouseDelta.x;
				if (g_inputContex->m_mouseDelta.y != 0.f) cursorOld.y -= g_inputContex->m_mouseDelta.y;
				yySetCursorPosition(screenCenterRoundX, screenCenterRoundY, window.m_data);
			}
			else
			{
	//			ShowCursor(TRUE);
			}
		}break;
		}

		g_rawInputCursorPosition.x = 0.f;
		g_rawInputCursorPosition.y = 0.f;
	}

	if (spriteCache)
		delete spriteCache;

	if (g_map)
		delete g_map;

	if (useImgui)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	return 0;
}