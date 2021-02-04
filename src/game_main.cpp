#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "yy.h"
#include "yy_window.h"

#include "yy_ptr.h"

#include "yy_async.h"
#include "yy_resource.h"
#include "yy_material.h"

#include "strings/string.h"

#include <cstdio>
#include <vector>
#include <string>

#include "yy_fs.h"
#include "yy_image.h"
#include "yy_model.h"
#include "yy_gui.h"
#include "yy_input.h"

#include "scene/common.h"
#include "scene/camera.h"
#include "scene/sprite.h"

int g_mapGenSizeX = 10;
int g_mapGenSizeY = 10;
yyVideoDriverAPI* g_videoDriver = nullptr;
v2f g_windowSize;

yyGUIFont* g_defaultFont = 0;

s32 g_selectedListItemBGObject = -1;
v2f g_rawInputCursorPosition;
struct SpriteCacheNode{
	yyStringA m_path;
	yySprite* m_sprite;
};
struct SpriteCache
{
	SpriteCache() {}
	~SpriteCache() 
	{
		for (u16 i = 0, sz = m_cache.size(); i < sz; ++i) 
		{
			yyDestroy(m_cache[i].m_sprite);
		}
	}
	yyArraySmall<SpriteCacheNode> m_cache;
};
SpriteCache * g_spriteCachePtr = 0;

v2f GetSpriteSize(yySprite* sprite) {
	assert(sprite);
	v2i size;
	yyGetTextureSize(sprite->m_texture, &size);
	return v2f((f32)size.x, (f32)size.y);
}

yySprite* GetSprite(const char* file) {
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

struct MapSprite
{
	MapSprite() {
		m_spritePtr = 0;
		m_gui_text = 0;
	}
	~MapSprite() {
		if (m_gui_text)
			yyGUIDeleteElement(m_gui_text);
	}

	yySprite* m_spritePtr;

	yyGUIText* m_gui_text;


	f32 m_radius;
	yyStringA m_name;
	v2f m_spritePosition;
};

class Map{
public:
	Map() {
		m_bgSprite = 0;
	}
	~Map() {
		Destroy();
	}

	f32 m_mapSizeX;
	f32 m_mapSizeY;
	f32 m_mapHalfSizeX;
	f32 m_mapHalfSizeY;

	void Generate() {
		Destroy();
		float spriteSize = 512.f;
		m_bgSprite = yyCreateSprite(v4f(0.f, 0.f, spriteSize, spriteSize), yyGetTextureResource("../res/textures/bg/grass.dds", false, false, true), true);
		
		m_bgSpriteRadius = v2f(spriteSize * 0.5f, spriteSize * 0.5f).distance(v2f());

		m_mapSizeX = spriteSize * g_mapGenSizeX;
		m_mapSizeY = spriteSize * g_mapGenSizeY;
		m_mapHalfSizeX = m_mapSizeX * 0.5f;
		m_mapHalfSizeY = m_mapSizeY * 0.5f;

		for (int y = 0; y < g_mapGenSizeY; ++y){
			for (int x = 0; x < g_mapGenSizeX; ++x){
				v2f pos = v2f(spriteSize * (f32)x + (0.f* (f32)x), spriteSize * (f32)y + (0.f* (f32)y));
				pos.x -= m_mapHalfSizeX;
				pos.y -= m_mapHalfSizeY;
				m_bgSpritePositions.push_back( pos );
			}
		}
	}
	void Destroy() {
		if (m_bgSprite)
		{
			if (m_bgSprite->m_texture)
			{
				g_videoDriver->UnloadTexture(m_bgSprite->m_texture);
				m_bgSprite->m_texture = 0;
			}

			yyDestroy(m_bgSprite);
			m_bgSprite = 0;
		}
		m_bgSpritePositions.clear();

		{
			auto curr = m_bgObjects.head();
			if (curr)
			{
				auto end = curr->m_left;
				while(true)
				{
					delete curr->m_data;

					if (curr == end)
						break;
					curr = curr->m_right;
				}
			}
		}
	}
	yySprite* m_bgSprite;
	f32 m_bgSpriteRadius;

	yyArraySmall<v2f> m_bgSpritePositions;

	yyListFast<MapSprite*> m_bgObjects;
};

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

void asyncLoadEventHandler(u32 userIndex, void* data)
{
	switch (userIndex)
	{
	default:
		break;
	//case 1: g_img1 = (yyImage*)data; printf("1\n"); break;
	//case 2: g_img2 = (yyImage*)data; printf("2\n"); break;
	case 1:
	{
	}break;
	case 2:
	{
	}break;
	}
}

void pictureBox_load_onClick(yyGUIElement* elem, s32 m_id)
{
	yyLoadImageAsync("../res/grass.dds",1);
	yyLoadImageAsync("../res/grass.png",2);
}

yyInputContext* g_inputContex = nullptr;
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
}

void window_callbackKeyboard(yyWindow*, bool isPress, u32 key, char16_t character)
{
	if(isPress)
	{
		if(key < 256)
			g_inputContex->m_key_hold[key] = 1;
	}
	else
	{
		if(key < 256)
			g_inputContex->m_key_hold[key] = 0;
	}
}

class GameCursor {
public:
	GameCursor() {
		m_sprite = 0;
	}
	~GameCursor() {
		if (m_sprite) yyDestroy(m_sprite);
	}
	void init(const char* file, v2f size, v2i lt, v2i rb) {
		yyResource* texture = yyGetTextureResource(file, false, false, true);
		m_sprite = yyCreateSprite(v4f(0.f, 0.f, size.x, size.y), texture, false);
		m_sprite->SetMainFrame(lt.x, lt.y, rb.x, rb.y);
	}

	yySprite* m_sprite;
};

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
	//yyStringA cmdLine = lpCmdLine;
	//std::vector<yyStringA> cmdLineArr;
	//util::stringGetWords(&cmdLineArr, cmdLine);
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	{
		FILE * f = fopen("log.txt", "w");
		fwrite(" ", 1, 1, f);
		fclose(f);
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

	// I don't want to use stack memory, so for class\struct I will create new objects using heap
	// use yyPtr if you want auto destroy objects
	
	yyPtr<yyInputContext> inputContext = yyCreate<yyInputContext>();
	g_inputContex = inputContext.m_data;

	yyPtr<yyEngineContext> engineContext = yyCreate<yyEngineContext>();
	engineContext.m_data->init(inputContext.m_data);
	auto p_engineContext = engineContext.m_data;


	yySetAsyncLoadEventHandler(asyncLoadEventHandler);

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
	bool useImgui = strcmp(videoDriverTypeStr.data(), "opengl.yyvd") != -1;

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
	
	//yySprite* spriteLevel = yyCreateSprite(v4f(0.f, 0.f, 1160.f, 224.f), g_videoDriver->CreateTextureFromFile("../res/GA3E/level1_ground.png", false, false, true), false);
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
	auto spriteCameraPosition = g_videoDriver->GetSpriteCameraPosition();
	auto spriteCameraScale = g_videoDriver->GetSpriteCameraScale();


	f32 aspect = (f32)p_window->m_currentSize.x / (f32)p_window->m_currentSize.y;
	spriteCameraScale->x = 1.f;
	spriteCameraScale->y = 1.f;

	

	bool isNewMapWindow = false;
	Map* map = new Map;

	v2f curCoordsOnPressRMB;
	v2f screenHalfSize = v2f(p_window->m_currentSize.x, p_window->m_currentSize.y) * 0.5f;
	f32 screenRectRadius = screenHalfSize.distance(v2f());

	v2f cameraLimits;
	
	GameCursor cursor_arrow;
	cursor_arrow.init("../res/textures/gui/cursors1.png", v2f(60.f, 61.f), v2i(17, 11), v2i(75, 70));
	GameCursor * activeCursor = &cursor_arrow;
	v2f gameCursorPosition;
	v2f gameCursorLimits;
	gameCursorLimits.x = p_window->m_currentSize.x - 20;
	gameCursorLimits.y = p_window->m_currentSize.y - 20;
	gameCursorPosition.x = screenHalfSize.x;
	gameCursorPosition.y = screenHalfSize.y;
	v2f cursorOld;

	SpriteCache* spriteCache = new SpriteCache;
	g_spriteCachePtr = spriteCache;

	MapSprite* currentMapSprite = 0;

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

	f32 deltaTime = 0.f;
	bool run = true;
	while( run )
	{
		v2f cameraMoveVector;

		static u64 t1 = 0;
		u64 t2 = yyGetTime();
		f32 m_tick = f32(t2 - t1);
		t1 = t2;
		deltaTime = m_tick / 1000.f;

		updateInputContext();

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
		auto screenCenterRoundX = std::floor(screenHalfSize.x);
		auto screenCenterRoundY = std::floor(screenHalfSize.y);

		g_inputContex->m_mouseDelta.x = g_rawInputCursorPosition.x - cursorOld.x;
		g_inputContex->m_mouseDelta.y = g_rawInputCursorPosition.y - cursorOld.y;
		cursorOld.x = g_rawInputCursorPosition.x;
		cursorOld.y = g_rawInputCursorPosition.y;

		if (useImgui)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		//printf("%f %f\n", spriteCameraPosition->x, spriteCameraPosition->y);
		//printf("%f %f - %f %f\n", gameCursorPosition.x, gameCursorPosition.y,
		//	gameCursorLimits.x, gameCursorLimits.y);
		//printf("%f %f - %f %f\n", g_inputContex->m_cursorCoords.x, g_inputContex->m_cursorCoords.y, 
		//	g_inputContex->m_mouseDelta.x, g_inputContex->m_mouseDelta.y);

		yyGUIUpdate(deltaTime);
		yyUpdateAsyncLoader();

		switch(*p_engineContext->m_state)
		{
		default:
			run = false;
			break;
		case yySystemState::Run:
		{
			if (g_inputContex->m_isRMBDown)
			{
				curCoordsOnPressRMB = g_inputContex->m_cursorCoords;
			}
			if (g_inputContex->m_isRMBHold)
			{
				cameraMoveVector = g_inputContex->m_cursorCoords - curCoordsOnPressRMB;

				cameraMoveVector *= deltaTime;
			}

			gameCursorPosition.x += g_inputContex->m_mouseDelta.x * 1.9;
			gameCursorPosition.y += g_inputContex->m_mouseDelta.y * 1.9;


			

			

			f32  spriteCameraMoveSpeed = 1000.f;
			f32  spriteCameraScaleSpeed = 1.f;

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
					gameCursorPosition.x -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_RIGHT))
					gameCursorPosition.x += spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_UP))
					gameCursorPosition.y -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_DOWN))
					gameCursorPosition.y += spriteCameraMoveSpeed * deltaTime;
			}
			else
			{
				if (g_inputContex->isKeyHold(yyKey::K_LEFT))
					spriteCameraPosition->x -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_RIGHT))
					spriteCameraPosition->x += spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_UP))
					spriteCameraPosition->y -= spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_DOWN))
					spriteCameraPosition->y += spriteCameraMoveSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_7))
					spriteCameraScale->x -= spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_9))
					spriteCameraScale->x += spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_1))
					spriteCameraScale->y -= spriteCameraScaleSpeed * deltaTime;
				if (g_inputContex->isKeyHold(yyKey::K_NUM_3))
					spriteCameraScale->y += spriteCameraScaleSpeed * deltaTime;
			}

			if (gameCursorPosition.x < 0.f) gameCursorPosition.x = 0.f;
			if (gameCursorPosition.y < 0.f) gameCursorPosition.y = 0.f;
			if (gameCursorPosition.x > gameCursorLimits.x) gameCursorPosition.x = gameCursorLimits.x;
			if (gameCursorPosition.y > gameCursorLimits.y) gameCursorPosition.y = gameCursorLimits.y;
			
			const f32 scroll_zone = 25.f;
			if (gameCursorPosition.x < scroll_zone) spriteCameraPosition->x -= (spriteCameraMoveSpeed*2.f) * deltaTime;
			if (gameCursorPosition.x > gameCursorLimits.x - scroll_zone) spriteCameraPosition->x += (spriteCameraMoveSpeed*2.f) * deltaTime;
			if (gameCursorPosition.y < scroll_zone) spriteCameraPosition->y -= (spriteCameraMoveSpeed*2.f) * deltaTime;
			if (gameCursorPosition.y > gameCursorLimits.y - scroll_zone) spriteCameraPosition->y += (spriteCameraMoveSpeed*2.f) * deltaTime;

			spriteCameraPosition->x += cameraMoveVector.x;
			spriteCameraPosition->y += cameraMoveVector.y;

			if (spriteCameraPosition->x > cameraLimits.x) spriteCameraPosition->x = cameraLimits.x;
			if (spriteCameraPosition->y > cameraLimits.y) spriteCameraPosition->y = cameraLimits.y;
			if (spriteCameraPosition->x < -cameraLimits.x) spriteCameraPosition->x = -cameraLimits.x;
			if (spriteCameraPosition->y < -cameraLimits.y) spriteCameraPosition->y = -cameraLimits.y;

			spriteCameraPosition->x = std::floor(spriteCameraPosition->x);
			spriteCameraPosition->y = std::floor(spriteCameraPosition->y);
			gameCursorPosition.x = std::floor(gameCursorPosition.x);
			gameCursorPosition.y = std::floor(gameCursorPosition.y);

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

			if (currentMapSprite)
			{
				if (g_inputContex->isKeyHold(yyKey::K_LSHIFT))
				{
					currentMapSprite->m_spritePosition.x = gameCursorPosition.x + spriteCameraPosition->x - screenHalfSize.x;
					currentMapSprite->m_spritePosition.y = gameCursorPosition.y + spriteCameraPosition->y - screenHalfSize.y;
				}
				
				if (g_inputContex->m_isLMBDown)
				{
					currentMapSprite = 0;
				}

				if (g_inputContex->isKeyHold(yyKey::K_DELETE) && currentMapSprite)
				{
					auto find_result = map->m_bgObjects.find_by_value(currentMapSprite);
					if (find_result)
					{
						map->m_bgObjects.erase_node(find_result);
					}
					currentMapSprite = 0;
				}
			}

			if (useImgui)
			{
				if (ImGui::Begin("Main menu", 0, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGuiTabBarFlags mainemnuTab_bar_flags = ImGuiTabBarFlags_None;
					if (ImGui::BeginTabBar("menutab", mainemnuTab_bar_flags))
					{
						if (ImGui::BeginTabItem("File"))
						{
							if (ImGui::Button("New Map"))
							{
								isNewMapWindow = true;
							}
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("BGObjects"))
						{
							ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
							if (ImGui::BeginChild("AddedBGObjects", ImVec2(ImGui::GetWindowContentRegionWidth(), 200), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove))
							{
								int n = 0;
								for (auto & sp : map->m_bgObjects)
								{
									if (ImGui::Selectable(sp.m_data->m_name.data(), g_selectedListItemBGObject == n))
									{
										g_selectedListItemBGObject = n;
										auto findSprite = map->m_bgObjects.find(n);
										if (findSprite)
										{
											currentMapSprite = findSprite->m_data;
										}
									}
									++n;
								}
								ImGui::EndChild();
							}
							ImGui::PopStyleColor();
							if (ImGui::Button("Add"))
							{
								auto path = yyOpenFileDialog("Select Sprite", "Select", "png bmp tga dds", "Supported files");
								if (path)
								{
									yyStringA stra;
									stra = path->to_stringA();
									auto newSprite = GetSprite(stra.data());
									if (newSprite)
									{
										static u32 nameCounter = 0;
										MapSprite* newMapSprite = new MapSprite;
										currentMapSprite = newMapSprite;
										yyFS::path fp = stra.data();
										auto fileName = fp.filename();
										util::stringPopBackBefore(fileName.string_type, '.');
										fileName.string_type.pop_back();
										newMapSprite->m_name = fileName.string_type.data();
										newMapSprite->m_name += "_";
										newMapSprite->m_name += nameCounter++;

										yyStringW wstr;
										wstr += newMapSprite->m_name.data();

										newMapSprite->m_gui_text = yyGUICreateText(v2f(), g_defaultFont, wstr.data());
										

										newMapSprite->m_spritePtr = newSprite;
										v2i textureSize;
										yyGetTextureSize(newSprite->m_texture, &textureSize);
										newMapSprite->m_radius = v2f((f32)textureSize.x, (f32)textureSize.y).distance(v2f());
										
										map->m_bgObjects.push_back(newMapSprite);

									}
									yyDestroy(path);
								}
							}
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}
					ImGui::End();
				}

				if (isNewMapWindow)
				{
					if (ImGui::Begin("New Map", 0, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("Set map size");
						ImGui::SliderInt("X segments", &g_mapGenSizeX, 3, 100);
						ImGui::SliderInt("Y segments", &g_mapGenSizeY, 3, 100);
						if (ImGui::Button("Create"))
						{
							map->Generate();
							isNewMapWindow = false;
							cameraLimits.x = map->m_mapHalfSizeX;
							cameraLimits.y = map->m_mapHalfSizeY;
						}
						if (ImGui::Button("Close"))
						{
							isNewMapWindow = false;
						}
						ImGui::End();
					}
				}
			}
			g_videoDriver->BeginDraw();
			g_videoDriver->ClearAll();

			g_videoDriver->UseDepth(false);
			g_videoDriver->UseBlend(false);
			
			for (u16 i = 0, sz = map->m_bgSpritePositions.size(); i < sz; ++i)
			{
				auto & spritePos = map->m_bgSpritePositions[i];

				f32 dist = spriteCameraPosition->distance(spritePos);
				
				if ((dist - map->m_bgSpriteRadius) <= screenRectRadius)
				{
					map->m_bgSprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
					map->m_bgSprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
					g_videoDriver->DrawSprite(map->m_bgSprite);
				}

			}

			f32 offsetX = -spriteCameraPosition->x + screenHalfSize.x;
			f32 offsetY = -spriteCameraPosition->y + screenHalfSize.y;

			{
				for (auto & sp : map->m_bgObjects)
				{
					sp.m_data->m_gui_text->m_visible = false;

					auto sprite = sp.m_data->m_spritePtr;
					auto & spritePos = sp.m_data->m_spritePosition;
					f32 dist = spriteCameraPosition->distance(sp.m_data->m_spritePosition);
					if ((dist - sp.m_data->m_radius) <= screenRectRadius)
					{
						sprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
						sprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
						g_videoDriver->DrawSprite(sprite);

						if (g_inputContex->isKeyHold(yyKey::K_LCTRL))
						{
							sp.m_data->m_gui_text->m_offset.x = spritePos.x + offsetX;
							sp.m_data->m_gui_text->m_offset.y = spritePos.y + offsetY;
							sp.m_data->m_gui_text->m_visible = true;
						}
					}
				}
				
			}

			if (currentMapSprite)
			{
				auto spriteSize = GetSpriteSize(currentMapSprite->m_spritePtr);
				auto spriteSizeHalf = spriteSize * 0.5f;
				
				g_videoDriver->DrawLine2D(
					v3f(currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
					v3f(currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
					ColorLime
					);
				g_videoDriver->DrawLine2D(
					v3f(currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
					v3f(currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
					ColorLime
				);
				g_videoDriver->DrawLine2D(
					v3f(currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
					v3f(currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
					ColorLime
				);
				g_videoDriver->DrawLine2D(
					v3f(currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
					v3f(currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
						currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
					ColorLime
				);
			}

			/*g_videoDriver->DrawSprite(spriteLevel);
			spriteHero->m_objectBase.UpdateBase();
			spriteHero->Update(deltaTime);
			g_videoDriver->DrawSprite(spriteHero);*/

			v2f spriteCameraPositionSave = *spriteCameraPosition;
			spriteCameraPosition->set(0.f, 0.f);
			activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].x = gameCursorPosition.x - screenHalfSize.x;
			activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].y = gameCursorPosition.y - screenHalfSize.y;
			g_videoDriver->DrawSprite(activeCursor->m_sprite);

			*spriteCameraPosition = spriteCameraPositionSave;


			yyGUIDrawAll();
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

	if (map)
		delete map;

	if (useImgui)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	return 0;
}