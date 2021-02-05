#include "RTS.h"

extern int g_mapGenSizeX;
extern int g_mapGenSizeY;
extern yyVideoDriverAPI*	g_videoDriver;

Map::Map()
{
	m_bgSprite = 0;
}

Map::~Map()
{
	Destroy();
}

void Map::Generate()
{
	Destroy();
	float spriteSize = 512.f;
	m_bgSprite = yyCreateSprite(v4f(0.f, 0.f, spriteSize, spriteSize), yyGetTextureResource("../res/textures/bg/grass.dds", false, false, true), true);

	m_bgSpriteRadius = v2f(spriteSize * 0.5f, spriteSize * 0.5f).distance(v2f());

	m_mapSizeX = spriteSize * g_mapGenSizeX;
	m_mapSizeY = spriteSize * g_mapGenSizeY;
	m_mapHalfSizeX = m_mapSizeX * 0.5f;
	m_mapHalfSizeY = m_mapSizeY * 0.5f;

	for (int y = 0; y < g_mapGenSizeY; ++y) {
		for (int x = 0; x < g_mapGenSizeX; ++x) {
			v2f pos = v2f(spriteSize * (f32)x + (0.f* (f32)x), spriteSize * (f32)y + (0.f* (f32)y));
			pos.x -= m_mapHalfSizeX;
			pos.y -= m_mapHalfSizeY;
			m_bgSpritePositions.push_back(pos);
		}
	}
}

void Map::Destroy()
{
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
			while (true)
			{
				delete curr->m_data;

				if (curr == end)
					break;
				curr = curr->m_right;
			}
		}
	}
}
