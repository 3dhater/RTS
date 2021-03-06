﻿#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"
#include "Game.h"
#include "Player.h"
#include "Struct.h"
#include "StructBase.h"

Game*		g_game = nullptr;

void EnterGameMenu()
{
	g_game->m_isPause = true;
	if(g_game->m_GUIMenuBG)
		g_game->m_GUIMenuBG->SetVisible(true);
	if(g_game->m_GUIContinueButton)
		g_game->m_GUIContinueButton->SetVisible(true);
	if(g_game->m_GUIExitButton)
		g_game->m_GUIExitButton->SetVisible(true);
}
void LeaveGameMenu()
{
	g_game->m_isPause = false;
	if (g_game->m_GUIMenuBG)
		g_game->m_GUIMenuBG->SetVisible(false);
	if(g_game->m_GUIContinueButton)
		g_game->m_GUIContinueButton->SetVisible(false);
	if(g_game->m_GUIExitButton)
		g_game->m_GUIExitButton->SetVisible(false);
	if(g_game->m_GUIPlayPauseButton)
		g_game->m_GUIPlayPauseButton->m_isChecked = false;
}

void PlayPauseButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	if (g_game->m_isPause)
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
void TowerButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	g_game->LeaveCreateMode();
	g_game->EnterCreateMode(new GameStructureTower);
}
void HomeButton_onRelease(yyGUIElement* elem, s32 m_id)
{
	g_game->ToHome();
}

void updateInputContext() // call before all callbacks
{
	g_game->m_inputContext->m_isLMBDbl = false;
	g_game->m_inputContext->m_isLMBDown = false;
	g_game->m_inputContext->m_isRMBDown = false;
	g_game->m_inputContext->m_isLMBUp = false;
	g_game->m_inputContext->m_mouseDelta.x = 0.f;
	g_game->m_inputContext->m_mouseDelta.y = 0.f;
	g_game->m_inputContext->m_cursorCoordsForGUI = g_game->m_inputContext->m_cursorCoords;

	memset(g_game->m_inputContext->m_key_pressed, 0, sizeof(u8) * 256);
	memset(g_game->m_inputContext->m_key_hit, 0, sizeof(u8) * 256);
}

void window_callbackKeyboard(yyWindow*, bool isPress, u32 key, char16_t character)
{
	if (isPress)
	{
		if (key < 256)
		{
			g_game->m_inputContext->m_key_hold[key] = 1;
			g_game->m_inputContext->m_key_hit[key] = 1;
		}
	}
	else
	{
		if (key < 256)
		{
			g_game->m_inputContext->m_key_hold[key] = 0;
			g_game->m_inputContext->m_key_pressed[key] = 1;
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
		g_game->m_rawInputCursorPosition.x = (f32)x;
		g_game->m_rawInputCursorPosition.y = (f32)y;
	}
}
void window_onCLose(yyWindow* window)
{
	yyQuit(); // change yySystemState - set yySystemState::Quit
}

void window_callbackMouse(yyWindow* w, s32 wheel, s32 x, s32 y, u32 click)
{
	g_game->m_inputContext->m_cursorCoords.x = (f32)x;
	g_game->m_inputContext->m_cursorCoords.y = (f32)y;

	g_game->m_inputContext->m_mouseDelta.x = (f32)x - g_game->m_inputContext->m_cursorCoordsOld.x;
	g_game->m_inputContext->m_mouseDelta.y = (f32)y - g_game->m_inputContext->m_cursorCoordsOld.y;

	g_game->m_inputContext->m_cursorCoordsOld = g_game->m_inputContext->m_cursorCoords;

	if (click & yyWindow_mouseClickMask_LMB_DOWN)
	{
		g_game->m_inputContext->m_isLMBDown = true;
		g_game->m_inputContext->m_isLMBHold = true;
	}
	if (click & yyWindow_mouseClickMask_LMB_UP)
	{
		g_game->m_inputContext->m_isLMBHold = false;
		g_game->m_inputContext->m_isLMBUp = true;
	}
	if (click & yyWindow_mouseClickMask_LMB_DOUBLE)
	{
		g_game->m_inputContext->m_isLMBDbl = true;
	}

	if (click & yyWindow_mouseClickMask_RMB_DOWN)
	{
		g_game->m_inputContext->m_isRMBDown = true;
		g_game->m_inputContext->m_isRMBHold = true;
	}
	if (click & yyWindow_mouseClickMask_RMB_UP)
	{
		g_game->m_inputContext->m_isRMBHold = false;
	}

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

Game::Game()
{
	m_createModeStructure = 0;

	m_player1 = 0;
	m_player2 = 0;

	m_selectedStruct = 0;

//	m_structureTest = 0;
//	m_structureBase = 0;
	m_activeCursor = 0;

	m_canPlaceStructureOnMap = false;
		 
	m_mapGenSizeX = 10;
	m_mapGenSizeY = 10;
	m_isPause = false;
	m_spriteCache = 0;
	m_cursor_arrow = 0;
	m_map = 0;
	m_useImgui = false;
	m_isEditorMode = false;
	m_inputContext = 0;
	m_engineContext = 0;
	m_window = 0;
	m_gpu = nullptr;
	m_spriteGridWhite = nullptr;
	m_spriteGridRed = nullptr;
	m_spriteGridBlue = nullptr;
	m_spriteGridBlack = 0;
	m_spriteGridGreen = 0;

	m_GUIContinueButton = 0;
	m_GUIExitButton = 0;
	m_GUIMenuBG = 0;
	m_GUIPlayPauseButton = 0;

	g_game = this;
}

Game::~Game()
{
//	if (m_structureTest) delete m_structureTest;
//	if (m_structureBase) delete m_structureBase;
	if (m_player1) delete m_player1;
	if (m_player2) delete m_player2;


	if (m_useImgui)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	if (m_spriteCache) delete m_spriteCache;
	if (m_cursor_arrow) delete m_cursor_arrow;
	if (m_map) delete m_map;
	if (m_spriteGridWhite) yyDestroy(m_spriteGridWhite);
	if (m_spriteGridRed) yyDestroy(m_spriteGridRed);
	if (m_spriteGridBlue) yyDestroy(m_spriteGridBlue);
	if (m_spriteGridBlack) yyDestroy(m_spriteGridBlack);
	if (m_spriteGridGreen) yyDestroy(m_spriteGridGreen);
	if (m_window) yyDestroy(m_window);
	if (m_engineContext) yyDestroy(m_engineContext);
	if (m_inputContext) yyDestroy(m_inputContext);
}

bool Game::Init(std::vector<yyStringA>& cmdLineArr)
{
	for (auto s : cmdLineArr)
	{
		if (strcmp(s.data(), "editor") == 0)
			m_isEditorMode = true;
	}

	const char * videoDriverType = "opengl.yyvd"; // for example read name from .ini
	wchar_t videoDriverTypeBuffer[50];
	memset(videoDriverTypeBuffer, 0, 50 * sizeof(wchar_t));
	GetPrivateProfileString(L"video", L"driver", L"opengl.yyvd", videoDriverTypeBuffer, 50, L"../params.ini");

	m_mainTargetSize.x = (f32)GetPrivateProfileInt(L"screen", L"x", 800, L"../params.ini");
	m_mainTargetSize.y = (f32)GetPrivateProfileInt(L"screen", L"y", 600, L"../params.ini");
	
	int isFullscreen = GetPrivateProfileInt(L"screen", L"fullscreen", 0, L"../params.ini");

	yyStringA videoDriverTypeStr = videoDriverTypeBuffer;
	if (!videoDriverTypeStr.size())
		videoDriverTypeStr = videoDriverType;
	if (!videoDriverTypeStr.size())
		videoDriverTypeStr = videoDriverType;

	if (m_isEditorMode)
	{
		videoDriverTypeStr = videoDriverType;
	}

	m_inputContext = yyCreate<yyInputContext>();
	m_engineContext = yyCreate<yyEngineContext>();
	m_engineContext->init(m_inputContext);

	yyLogSetErrorOutput(log_onError);
	yyLogSetInfoOutput(log_onInfo);
	yyLogSetWarningOutput(log_onError);

	m_window = yyCreate<yyWindow>();
	if (!m_window->init((s32)m_mainTargetSize.x, (s32)m_mainTargetSize.y, 0))
	{
		YY_PRINT_FAILED;
		return false;
	}

	yySetMainWindow(m_window);

	m_window->m_onClose = window_onCLose;
	m_window->m_onMouseButton = window_callbackMouse;
	m_window->m_onKeyboard = window_callbackKeyboard;
	m_window->m_onRawInput = OnRawInput;

	if (!yyInitVideoDriver(videoDriverTypeStr.c_str(), m_window))
	{
		yyLogWriteWarning("Can't load video driver : %s\n", videoDriverTypeStr.c_str());
		for (auto & entry : yyFS::directory_iterator(yyFS::current_path()))
		{
			auto path = entry.path();
			if (path.has_extension())
			{
				auto ex = path.extension();
				if (ex == ".yyvd")
				{
					yyLogWriteWarning("Trying to load video driver : %s\n", path.generic_string().c_str());
					if (yyInitVideoDriver(path.generic_string().c_str(), m_window))
						goto vidOk;
					else
						yyLogWriteWarning("Can't load video driver : %s\n", path.generic_string().c_str());
				}
			}
		}
		YY_PRINT_FAILED;
		return false;
	}
vidOk:

	m_gpu = yyGetVideoDriverAPI();
	m_gpu->SetClearColor(0.f, 0.f, 0.f, 1.f);
	m_useImgui = m_isEditorMode;
	if (m_useImgui)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(m_window->m_hWnd);
		gl3wInit();
		const char* glsl_version = "#version 330";
		ImGui_ImplOpenGL3_Init(glsl_version);
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	}

	yyStringA title = "3Dhater RTS - ";
	title += m_gpu->GetVideoDriverName();
	m_window->SetTitle(title.data());

	m_spriteGridWhite = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_white.png", false, false, true), 6);
	m_spriteGridRed = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_red.png", false, false, true), 6);
	m_spriteGridBlue = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_blue.png", false, false, true), 6);
	m_spriteGridBlack = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_black.png", false, false, true), 6);
	m_spriteGridGreen = yyCreateSprite(v4f(0.f, 0.f, 10.f, 10.f), yyGetTextureResource("../res/editor/grid_green.png", false, false, true), 6);

	m_spriteCameraPosition = m_gpu->GetSpriteCameraPosition();
	auto spriteCameraScale = m_gpu->GetSpriteCameraScale();

	f32 aspect = (f32)m_window->m_currentSize.x / (f32)m_window->m_currentSize.y;
	spriteCameraScale->x = 1.f;
	spriteCameraScale->y = 1.f;

	m_map = new Map;

	v2f curCoordsOnPressRMB;
	m_screenHalfSize = v2f(m_window->m_currentSize.x, m_window->m_currentSize.y) * 0.5f;
	m_screenRectRadius = m_screenHalfSize.distance(v2f()) * 2.f;

	m_cursor_arrow = new GameCursor;
	m_cursor_arrow->init("../res/textures/gui/cursors1.png", v2f(60.f, 61.f), v2i(17, 11), v2i(75, 70));
	m_activeCursor = m_cursor_arrow;
	
	m_gameCursorLimits.x = f32(m_window->m_currentSize.x - 20);
	m_gameCursorLimits.y = f32(m_window->m_currentSize.y - 20);
	m_gameCursorPosition.x = m_screenHalfSize.x;
	m_gameCursorPosition.y = m_screenHalfSize.y;

	m_spriteCache = new SpriteCache;

	m_defaultFont = yyGUILoadFont("../res/fonts/Noto/notosans.txt");

	if (isFullscreen == 1)
	{
		m_window->ToFullscreenMode();
		m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, m_mainTargetSize);

		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = m_window->m_currentSize.x;
		rc.bottom = m_window->m_currentSize.y;
		ClipCursor(&rc);
	}
	m_gpu->UseBlend(false);

	m_GUIMenuBG = yyGUICreatePictureBox(v4f(0.f, 0.f, 512.f, 512.f), yyGetTextureResource("../res/gui/menubg.png", false, false, true), -1);
	m_GUIMenuBG->SetVisible(false);
	m_GUIMenuBG->IgnoreInput(true);

	m_GUIPlayPauseButton = yyGUICreateButton(v4f(0.f, 0.f, 32.f, 32.f), yyGetTextureResource("../res/gui/pause1.png", false, false, true), -1);
	m_GUIPlayPauseButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/pause2.png", false, false, true));
	m_GUIPlayPauseButton->m_useAsCheckbox = true;
	//m_GUIPlayPauseButton->m_onRelease = PlayPauseButton_onRelease;
	m_GUIPlayPauseButton->m_onClick = PlayPauseButton_onRelease;

	m_GUIContinueButton = yyGUICreateButton(v4f(0.f, 0.f, 250.f, 40.f), yyGetTextureResource("../res/gui/continue1.png", false, false, true), -1);
	m_GUIContinueButton->SetMouseHoverTexture(yyGetTextureResource("../res/gui/continue2.png", false, false, true));
	m_GUIContinueButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/continue3.png", false, false, true));
	m_GUIContinueButton->SetOffset(v2f(50.f, 50.f));
	m_GUIContinueButton->SetVisible(false);
	m_GUIContinueButton->m_onRelease = ContinueButton_onRelease;

	m_GUIExitButton = yyGUICreateButton(v4f(0.f, 0.f, 250.f, 40.f), yyGetTextureResource("../res/gui/exit1.png", false, false, true), -1);
	m_GUIExitButton->SetMouseHoverTexture(yyGetTextureResource("../res/gui/exit2.png", false, false, true));
	m_GUIExitButton->SetMouseClickTexture(yyGetTextureResource("../res/gui/exit3.png", false, false, true));
	m_GUIExitButton->SetOffset(v2f(50.f, 100.f));
	m_GUIExitButton->SetVisible(false);
	m_GUIExitButton->m_onRelease = ExitButton_onRelease;


	
	
	//m_structureTest = new GameStructure;
	//m_structureTest->init(7,20, 7,5, "../res/structs/test.png", "../res/structs/testBG.png");

//	m_structureBase = new GameStructure;
//	m_structureBase->init(16, 16, 16, 10, "../res/structs/base.png", "../res/structs/baseBG.png");

	m_player1 = new GamePlayer;
	m_player2 = new GamePlayer;

	if (!m_isEditorMode)
	{
		ShowCursor(FALSE);
		m_map->InitFromFile("../res/maps/1.rtsmap");
		
		auto toolBG = yyGUICreatePictureBox(v4f(0.f, m_window->m_currentSize.y - 160.f, m_window->m_currentSize.x, m_window->m_currentSize.y), yyGetTextureResource("../res/gui/toolbg.png", false, false, true), -1);
		toolBG->IgnoreInput(true);
	
		auto homeButton = yyGUICreateButton(v4f(0.f, m_window->m_currentSize.y - 160.f, 32.f, m_window->m_currentSize.y - 160.f + 32.f), yyGetTextureResource("../res/gui/home.png", false, false, true), -1);
		homeButton->m_onRelease = HomeButton_onRelease;
		
		auto towerButton = yyGUICreateButton(
			v4f(
				80.f, m_window->m_currentSize.y - 150.f, 
				80.f + 64.f, m_window->m_currentSize.y - 150.f + 64.f), 
			yyGetTextureResource("../res/structs/towerGUI.png", false, false, true), -1);
		towerButton->SetMouseHoverTexture(yyGetTextureResource("../res/structs/towerGUI2.png", false, false, true));
		towerButton->SetVisible(true);
		towerButton->m_onRelease = TowerButton_onRelease;

		m_player2->m_isAI = true;

		GameStructure * strct = new GameStructureBase;
		m_player1->AddStructure(strct, m_map->m_player1Position, 1000);
		m_map->AddStructure(strct, m_map->m_player1Position);

		strct = new GameStructureBase;
		m_player2->AddStructure(strct, m_map->m_player2Position, 1000);
		m_map->AddStructure(strct, m_map->m_player2Position);
	}

	return true;
}

void Game::EnterCreateMode(GameStructure* newStructure)
{
	m_createModeStructure = newStructure;
	m_createModeStructure->Init();
}
void Game::LeaveCreateMode()
{
	if (m_createModeStructure)
	{
		delete m_createModeStructure;
		m_createModeStructure = 0;
	}
}

v2f Game::GetCursorPositionInWorld()
{
	return m_gameCursorPositionWorld;
	//return m_gameCursorPosition + *m_spriteCameraPosition - m_screenHalfSize;
}

void Game::DrawGround()
{
	for (u16 i = 0, sz = m_map->m_bgSpritePositions.size(); i < sz; ++i)
	{
		auto & spritePos = m_map->m_bgSpritePositions[i];
		if ((m_spriteCameraPosition->distance(spritePos) - m_map->m_bgSpriteRadius) <= m_screenRectRadius)
		{
			m_map->m_bgSprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
			m_map->m_bgSprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
			m_gpu->DrawSprite(m_map->m_bgSprite);
		}
	}
}

void Game::SortRenderSprites()
{
	m_renderSprites.clear();

	for (auto & o : m_player1->m_structs)
	{
		s32 CellIndexX = 0;
		s32 CellIndexY = 0;
		m_map->GetCellInds(CellIndexX, CellIndexY, o.m_data->m_position);

		if ((CellIndexX + o.m_data->m_fullSizeX) - m_map->m_screenCellLT.x < 0) continue;
		if (CellIndexY - m_map->m_screenCellLT.y < 0) continue;

		if (CellIndexX > m_map->m_screenCellRB.x) continue;
		if ((CellIndexY - o.m_data->m_fullSizeY) > m_map->m_screenCellRB.y) continue;

		m_renderSprites.push_back(o.m_data);
	}

	for (auto & o : m_player2->m_structs)
	{
		s32 CellIndexX = 0;
		s32 CellIndexY = 0;
		m_map->GetCellInds(CellIndexX, CellIndexY, o.m_data->m_position);

		if ((CellIndexX + o.m_data->m_fullSizeX) - m_map->m_screenCellLT.x < 0) continue;
		if (CellIndexY - m_map->m_screenCellLT.y < 0) continue;

		if (CellIndexX > m_map->m_screenCellRB.x) continue;
		if ((CellIndexY - o.m_data->m_fullSizeY) > m_map->m_screenCellRB.y) continue;

		m_renderSprites.push_back(o.m_data);
	}

	struct _pred
	{
		bool operator() (GameSpriteBase* a, GameSpriteBase* b) const
		{
			return a->m_position.y > b->m_position.y;
		}
	};

	m_renderSprites.sort_insertion(_pred());

}
void Game::DrawVisibleSprites()
{
	SortRenderSprites();
	/*for (u16 i = 0; i < m_renderSprites.m_size; ++i)
	{
		auto & rn = m_renderSprites.m_data[i];
		if (rn.m_spriteBG)
		{
			rn.m_spriteBG->m_objectBase.m_globalMatrix[3].x = rn.m_position.x;
			rn.m_spriteBG->m_objectBase.m_globalMatrix[3].y = rn.m_position.y;
			m_gpu->DrawSprite(rn.m_spriteBG);
		}
	}*/
	for (u16 i = 0; i < m_renderSprites.m_size; ++i)
	{
		m_renderSprites.m_data[i]->Draw();
		/*auto & rn = m_renderSprites.m_data[i];
		rn.m_sprite->m_objectBase.m_globalMatrix[3].x = rn.m_position.x;
		rn.m_sprite->m_objectBase.m_globalMatrix[3].y = rn.m_position.y;
		m_gpu->DrawSprite(rn.m_sprite);*/
	}
}
void Game::DrawGroundSprites()
{
	for (auto & sp : m_map->m_bgObjects)
	{
#ifdef YY_DEBUG
		sp.m_data->m_gui_text->SetVisible(false);
#endif

		auto sprite = sp.m_data->m_spritePtr;
		auto & spritePos = sp.m_data->m_spritePosition;
		f32 dist = m_spriteCameraPosition->distance(sp.m_data->m_spritePosition);
		if ((dist - sp.m_data->m_radius) <= m_screenRectRadius)
		{
			sprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
			sprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
			m_gpu->DrawSprite(sprite);

#ifdef YY_DEBUG
			if (m_inputContext->isKeyHold(yyKey::K_LCTRL))
			{
				sp.m_data->m_gui_text->m_offset.x = spritePos.x + m_worldToScreenOffset.x;
				sp.m_data->m_gui_text->m_offset.y = spritePos.y + m_worldToScreenOffset.y;
				sp.m_data->m_gui_text->SetVisible(true);
			}
#endif
		}
	}
}

void Game::CameraSetPosition(const v2f& p)
{
	*m_spriteCameraPosition = p;
}

void Game::Run()
{
	auto spriteCameraScale = m_gpu->GetSpriteCameraScale();

	f32 deltaTime = 0.f;
	bool run = true;
	while (run)
	{
		//v2f cameraMoveVector;

		static u64 t1 = 0;
		u64 t2 = yyGetTime();
		f32 m_tick = f32(t2 - t1);
		t1 = t2;
		deltaTime = m_tick / 1000.f;

		updateInputContext();
		m_inputContext->m_cursorCoordsForGUI = m_gameCursorPosition;

#ifdef YY_PLATFORM_WINDOWS
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			GetMessage(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#else
#error For windows
#endif

		auto screenCenterRoundX = std::floor(m_screenHalfSize.x);
		auto screenCenterRoundY = std::floor(m_screenHalfSize.y);
		m_inputContext->m_mouseDelta.x = m_rawInputCursorPosition.x - m_cursorOld.x;
		m_inputContext->m_mouseDelta.y = m_rawInputCursorPosition.y - m_cursorOld.y;
		m_cursorOld.x = m_rawInputCursorPosition.x;
		m_cursorOld.y = m_rawInputCursorPosition.y;

		yyGUIUpdate(deltaTime);


		switch (*m_engineContext->m_state)
		{
		default:
			run = false;
			break;
		case yySystemState::Run:
		{
			m_gameCursorPosition.x += m_inputContext->m_mouseDelta.x * 1.9f;
			m_gameCursorPosition.y += m_inputContext->m_mouseDelta.y * 1.9f;

			f32  spriteCameraMoveSpeed = 1000.f;
			f32  spriteCameraScaleSpeed = 1.f;

			bool findCell = false;

			if (m_inputContext->isKeyHold(yyKey::K_CTRL) || m_inputContext->isKeyHold(yyKey::K_LCTRL)
				|| m_inputContext->isKeyHold(yyKey::K_RCTRL))
			{
				spriteCameraMoveSpeed = 100.f;
				if (m_inputContext->isKeyHold(yyKey::K_SHIFT) || m_inputContext->isKeyHold(yyKey::K_LSHIFT)
					|| m_inputContext->isKeyHold(yyKey::K_RSHIFT))
				{
					spriteCameraMoveSpeed *= deltaTime;
				}

				if (m_inputContext->isKeyHold(yyKey::K_LEFT))
					m_gameCursorPosition.x -= spriteCameraMoveSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_RIGHT))
					m_gameCursorPosition.x += spriteCameraMoveSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_UP))
					m_gameCursorPosition.y -= spriteCameraMoveSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_DOWN))
					m_gameCursorPosition.y += spriteCameraMoveSpeed * deltaTime;
			}
			else
			{
				if (m_inputContext->isKeyHold(yyKey::K_LEFT))
				{
					findCell = true;
					m_spriteCameraPosition->x -= spriteCameraMoveSpeed * deltaTime;
				}
				if (m_inputContext->isKeyHold(yyKey::K_RIGHT))
				{
					findCell = true;
					m_spriteCameraPosition->x += spriteCameraMoveSpeed * deltaTime;
				}
				if (m_inputContext->isKeyHold(yyKey::K_UP))
				{
					findCell = true;
					m_spriteCameraPosition->y -= spriteCameraMoveSpeed * deltaTime;
				}
				if (m_inputContext->isKeyHold(yyKey::K_DOWN))
				{
					findCell = true;
					m_spriteCameraPosition->y += spriteCameraMoveSpeed * deltaTime;
				}
				if (m_inputContext->isKeyHold(yyKey::K_NUM_7))
					spriteCameraScale->x -= spriteCameraScaleSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_NUM_9))
					spriteCameraScale->x += spriteCameraScaleSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_NUM_1))
					spriteCameraScale->y -= spriteCameraScaleSpeed * deltaTime;
				if (m_inputContext->isKeyHold(yyKey::K_NUM_3))
					spriteCameraScale->y += spriteCameraScaleSpeed * deltaTime;
			}

			if (m_gameCursorPosition.x < 0.f) m_gameCursorPosition.x = 0.f;
			if (m_gameCursorPosition.y < 0.f) m_gameCursorPosition.y = 0.f;
			if (m_gameCursorPosition.x > m_gameCursorLimits.x) m_gameCursorPosition.x = m_gameCursorLimits.x;
			if (m_gameCursorPosition.y > m_gameCursorLimits.y) m_gameCursorPosition.y = m_gameCursorLimits.y;

			const f32 scroll_zone = 2.f;
			if (m_gameCursorPosition.x < scroll_zone)
			{
				m_spriteCameraPosition->x -= (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (m_gameCursorPosition.x > m_gameCursorLimits.x - scroll_zone)
			{
				m_spriteCameraPosition->x += (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (m_gameCursorPosition.y < scroll_zone)
			{
				m_spriteCameraPosition->y -= (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}
			if (m_gameCursorPosition.y > m_gameCursorLimits.y - scroll_zone)
			{
				m_spriteCameraPosition->y += (spriteCameraMoveSpeed*2.f) * deltaTime;
				findCell = true;
			}


			if (m_spriteCameraPosition->x > m_cameraLimits.x) m_spriteCameraPosition->x = m_cameraLimits.x;
			if (m_spriteCameraPosition->y > m_cameraLimits.y) m_spriteCameraPosition->y = m_cameraLimits.y;
			if (m_spriteCameraPosition->x < 0.f) m_spriteCameraPosition->x = 0.f;
			if (m_spriteCameraPosition->y < 0.f) m_spriteCameraPosition->y = 0.f;

			m_spriteCameraPosition->x = std::floor(m_spriteCameraPosition->x);
			m_spriteCameraPosition->y = std::floor(m_spriteCameraPosition->y);
			m_gameCursorPosition.x = std::floor(m_gameCursorPosition.x);
			m_gameCursorPosition.y = std::floor(m_gameCursorPosition.y);

			if (findCell)
			{
				m_map->FindCellPosition();
			}

			if (m_inputContext->isKeyHit(yyKey::K_ESCAPE))
			{
				if (m_isPause)
				{
					LeaveGameMenu();
				}
				else
				{
					EnterGameMenu();
				}
			}

			if (m_inputContext->isKeyHold(yyKey::K_F12))
			{
				m_window->ToFullscreenMode();
				m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, m_mainTargetSize);
				//screenHalfSize = v2f(m_window->m_currentSize.x, m_window->m_currentSize.y) * 0.5f;
			}
			if (m_inputContext->isKeyHold(yyKey::K_F11))
			{
				m_window->ToWindowMode();
				m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, m_mainTargetSize);
				//	screenHalfSize = v2f(m_window->m_currentSize.x, m_window->m_currentSize.y) * 0.5f;
			}
			if (m_inputContext->isKeyHold(yyKey::K_F10))
				m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, v2f(m_window->m_currentSize.x / 2, m_window->m_currentSize.y / 2));
			if (m_inputContext->isKeyHold(yyKey::K_F9))
				m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, v2f(m_window->m_currentSize.x / 4, m_window->m_currentSize.y / 4));
			if (m_inputContext->isKeyHold(yyKey::K_F8))
				m_gpu->UpdateMainRenderTarget(m_window->m_currentSize, v2f(m_window->m_currentSize.x / 8, m_window->m_currentSize.y / 8));

			m_gpu->BeginDraw();
			m_gpu->ClearAll();


			m_worldToScreenOffset.x = -m_spriteCameraPosition->x + m_screenHalfSize.x;
			m_worldToScreenOffset.y = -m_spriteCameraPosition->y + m_screenHalfSize.y;

			m_gameCursorPositionWorld = m_gameCursorPosition + *m_spriteCameraPosition - m_screenHalfSize;

			if (m_isEditorMode)
				EditorStep(deltaTime);
			else
				GameStep(deltaTime);


			yyGUIDrawAll();

			v2f spriteCameraPositionSave = *m_spriteCameraPosition;
			m_spriteCameraPosition->set(0.f, 0.f);
			if (m_activeCursor)
			{
				m_activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].x = m_gameCursorPosition.x - m_screenHalfSize.x;
				m_activeCursor->m_sprite->m_objectBase.m_globalMatrix[3].y = m_gameCursorPosition.y - m_screenHalfSize.y;
				m_gpu->DrawSprite(m_activeCursor->m_sprite);
			}

			*m_spriteCameraPosition = spriteCameraPositionSave;


			m_gpu->EndDraw();
			if (m_useImgui)
			{
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}
			m_gpu->SwapBuffers();

			if (!m_inputContext->isKeyHold(yyKey::K_LCTRL))
			{
				//			ShowCursor(FALSE);
				m_inputContext->m_cursorCoordsOld.set(screenCenterRoundX, screenCenterRoundY);
				if (m_inputContext->m_mouseDelta.x != 0.f) m_cursorOld.x -= m_inputContext->m_mouseDelta.x;
				if (m_inputContext->m_mouseDelta.y != 0.f) m_cursorOld.y -= m_inputContext->m_mouseDelta.y;
				yySetCursorPosition(screenCenterRoundX, screenCenterRoundY, m_window);
			}
			else
			{
				//			ShowCursor(TRUE);
			}
		}break;
		}

		m_rawInputCursorPosition.x = 0.f;
		m_rawInputCursorPosition.y = 0.f;
	}
}

void Game::ToHome()
{
	LeaveCreateMode();
	CameraSetPosition(m_map->m_player1Position);
	m_map->FindCellPosition();
	m_selectedStruct = m_player1->GetBase();
}