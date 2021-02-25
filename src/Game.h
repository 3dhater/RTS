#ifndef __RTS_GAME_H_
#define __RTS_GAME_H_

class GameSpriteBase;
class GamePlayer;
class Game
{
	bool m_isEditorMode;
	bool m_useImgui;
	yyEngineContext* m_engineContext;
	yyWindow* m_window;

	yySprite* m_spriteGridWhite;
	yySprite* m_spriteGridRed;
	yySprite* m_spriteGridBlue;
	yySprite* m_spriteGridBlack;
	yySprite* m_spriteGridGreen;

//	GameStructure* m_structureTest;

	f32					m_screenRectRadius;

	GameCursor * m_cursor_arrow;
	GameCursor * m_activeCursor;
	v2f m_gameCursorLimits;
	v2f	m_gameCursorPosition;
	v2f m_cursorOld;

	//GameStructure* m_structureBase;
	
	v2f m_mainTargetSize;

	v2f  m_worldToScreenOffset;

public:
	Game();
	~Game();
	bool Init(std::vector<yyStringA>& cmdLine);
	void Run();
	void EditorStep(f32 dt);
	void GameStep(f32 dt);

	void DrawGround();
	void DrawGroundSprites();
	void DrawVisibleSprites();

	void CameraSetPosition(const v2f&);
	void ToHome();
	v2f GetCursorPositionInWorld();
	
	v2f	m_gameCursorPositionWorld;

	yyInputContext* m_inputContext;
	v2f					m_rawInputCursorPosition;
	yyVideoDriverAPI*	m_gpu;

	yyGUIButton* m_GUIPlayPauseButton;
	yyGUIButton* m_GUIContinueButton;
	yyGUIButton* m_GUIExitButton;
	yyGUIPictureBox* m_GUIMenuBG;
	yyGUIPictureBox* m_GUIToolBG;
	yyGUIFont* m_defaultFont;
	
	bool m_canPlaceStructureOnMap;
	GameStructure* m_createModeStructure;
	void EnterCreateMode(GameStructure* newStructure);
	void LeaveCreateMode();

	bool m_isPause;
	SpriteCache* m_spriteCache;
	v2f* m_spriteCameraPosition;
	v2f					m_screenHalfSize;
	v2f					m_cameraLimits;

	GamePlayer* m_player1;
	GamePlayer* m_player2;

	Map* m_map;

	int m_mapGenSizeX;
	int m_mapGenSizeY;

	GameStructure* m_selectedStruct;
	yyArraySimple<GameSpriteBase*> m_renderSprites;
	void SortRenderSprites();
};

#endif