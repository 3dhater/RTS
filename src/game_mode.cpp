#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"

extern SpriteCache *		g_spriteCachePtr;
extern yyVideoDriverAPI*	g_videoDriver;
extern v2f					g_rawInputCursorPosition;
extern yyInputContext*		g_inputContex;
extern v2f  g_gameCursorPosition;
extern v2f*	g_spriteCameraPosition;
extern v2f  g_screenHalfSize;
extern Map* g_map;
extern yyGUIFont*			g_defaultFont;
extern v2f					g_cameraLimits;
extern f32					g_screenRectRadius;

void GameStep(f32 dt)
{
	
}