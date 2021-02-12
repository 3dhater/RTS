#ifndef __RTS_GAME_H_
#define __RTS_GAME_H_

class Game
{
	bool m_isEditorMode;
	bool m_useImgui;
	yyEngineContext* m_engineContext;
	yyWindow* m_window;

	yySprite* m_spriteGridWhite;
	yySprite* m_spriteGridRed;
	yySprite* m_spriteGridBlue;


	Map* m_map;
	f32					m_screenRectRadius;

	GameCursor * m_cursor_arrow;
	GameCursor * m_activeCursor;
	v2f m_gameCursorLimits;
	v2f					m_gameCursorPosition;
	v2f m_cursorOld;


	yyGUIFont* m_defaultFont;
	
	v2f					m_cameraLimits;
	v2f m_mainTargetSize;

public:
	Game();
	~Game();
	bool Init(std::vector<yyStringA>& cmdLine);
	void Run();
	void EditorStep(f32 dt);

	yyInputContext* m_inputContext;
	v2f					m_rawInputCursorPosition;
	yyVideoDriverAPI*	m_gpu;

	yyGUIButton* m_GUIPlayPauseButton;
	yyGUIButton* m_GUIContinueButton;
	yyGUIButton* m_GUIExitButton;
	yyGUIPictureBox* m_GUIMenuBG;
	bool m_isPause;
	SpriteCache* m_spriteCache;
	v2f* m_spriteCameraPosition;
	v2f					m_screenHalfSize;

	int m_mapGenSizeX;
	int m_mapGenSizeY;
};

#endif