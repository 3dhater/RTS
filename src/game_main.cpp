#include "RTS.h"
#include "Game.h"

extern Game*		g_game;


v2f GetSpriteSize(yySprite* sprite) 
{
	assert(sprite);
	v2i size;
	yyGetTextureSize(sprite->m_texture, &size);
	return v2f((f32)size.x, (f32)size.y);
}

yySprite* GetSprite(const char* file, u8 pivotPosition ) 
{
	for (u16 i = 0, sz = g_game->m_spriteCache->m_cache.size(); i < sz; ++i) {
		if (strcmp(g_game->m_spriteCache->m_cache[i].m_path.data(), file) == 0) {
			return g_game->m_spriteCache->m_cache[i].m_sprite;
		}
	}
	yyResource* texture = yyGetTextureResource(file, false, false, true);
	v2i textureSize;
	yyGetTextureSize(texture, &textureSize);
	yySprite* newSprite = yyCreateSprite(v4f(0.f, 0.f, (f32)textureSize.x, (f32)textureSize.y), texture, pivotPosition);
	SpriteCacheNode n;
	n.m_path = file;
	n.m_sprite = newSprite;
	g_game->m_spriteCache->m_cache.push_back(n);
	return newSprite;
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

	Game* game = new Game;
	if (game->Init(cmdLineArr))
	{
		game->Run();
	}
	
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
	


	return 0;
}