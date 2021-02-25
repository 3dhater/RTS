#include "RTS.h"
#include "Struct.h"
#include "Game.h"
#include "math/math.h"

extern Game*		g_game;

GameStructureTower::GameStructureTower()
{
	m_spriteS = 0;
	m_spriteSE = 0;
	m_spriteSW = 0;
	m_spriteW = 0;
	m_spriteE = 0;
	m_spriteN = 0;
	m_spriteNW = 0;
	m_spriteNE = 0;
	m_spriteActive = 0;

	m_angle = math::PI * 0.5f;
}

GameStructureTower::~GameStructureTower()
{

}

void GameStructureTower::Init()
{
	//int full_x, int full_y, int site_x, int site_y, const char* spritePath, const char* spritePathBG

	m_fullSizeX = 4;
	m_fullSizeY = 4;

	m_siteSizeX = 4;
	m_siteSizeY = 4;

	m_field = new u8*[m_siteSizeY];
	for (int y = 0; y < m_siteSizeY; ++y)
	{
		m_field[y] = new u8[m_siteSizeX];
		for (int x = 0; x < m_siteSizeX; ++x)
		{
			m_field[y][x] = MapCell::flag::flag_structure;
		}
	}

	m_rect.x = m_position.x;
	m_rect.y = m_position.y - (m_fullSizeY * GAME_MAP_GRID_SIZE);
	m_rect.z = m_position.x + (m_fullSizeX * GAME_MAP_GRID_SIZE);
	m_rect.w = m_position.y;

	m_sprite = GetSprite("../res/structs/tower.png", 6);
	m_spriteE = GetSprite("../res/structs/towerE.png", 6);
	m_spriteW = GetSprite("../res/structs/towerW.png", 6);
	m_spriteN = GetSprite("../res/structs/towerN.png", 6);
	m_spriteNE = GetSprite("../res/structs/towerNE.png", 6);
	m_spriteNW = GetSprite("../res/structs/towerNW.png", 6);
	m_spriteS = GetSprite("../res/structs/towerS.png", 6);
	m_spriteSE = GetSprite("../res/structs/towerSE.png", 6);
	m_spriteSW = GetSprite("../res/structs/towerSW.png", 6);

	m_spriteActive = m_spriteN;

	m_type = GameStructType::Tower;

	m_centerOffset.set(20.f, -20.f);
}

void GameStructureTower::Draw()
{
	m_sprite->m_objectBase.m_globalMatrix[3].x = m_position.x;
	m_sprite->m_objectBase.m_globalMatrix[3].y = m_position.y;
	g_game->m_gpu->DrawSprite(m_sprite);

	m_spriteActive->m_objectBase.m_globalMatrix[3].x = m_position.x;
	m_spriteActive->m_objectBase.m_globalMatrix[3].y = m_position.y;
	g_game->m_gpu->DrawSprite(m_spriteActive);
}

void GameStructureTower::Update(f32 dt)
{
	f32 new_angle = std::atan2(
		(m_position.y + m_centerOffset.y) - g_game->m_gameCursorPositionWorld.y,
		(m_position.x + m_centerOffset.x) - g_game->m_gameCursorPositionWorld.x) + math::PI;

	f32 a1 = math::get_0_1(math::PIPI, m_angle);
	f32 a2 = math::get_0_1(math::PIPI, new_angle);

	f32 v1 = a1 - a2;
	f32 v2 = a2 - a1;

	if (v1 < 0.f)
	{
		v1 = 1.f - v2;
	}

	if (v2 < 0.f)
	{
		v2 += 1.f;
	}

	if (v1 > v2)
	{
		m_angle += 1.f * dt;
	}
	else if (v1 < v2)
	{
		m_angle -= 1.f * dt;
	}

	if (m_angle > math::PIPI) m_angle = 0.f;
	if (m_angle < 0.f) m_angle = math::PIPI;

	if (m_angle > 5.761351f || m_angle <= 0.336675f)
	{
		m_spriteActive = m_spriteE;
	}
	else if (m_angle > 0.336675f && m_angle <= 1.116494f)
	{
		m_spriteActive = m_spriteSE;
	}
	else if (m_angle > 1.116494f && m_angle <= 2.00615f)
	{
		m_spriteActive = m_spriteS;
	}
	else if (m_angle > 2.00615f && m_angle <= 2.584993f)
	{
		m_spriteActive = m_spriteSW;
	}
	else if (m_angle > 2.584993f && m_angle <= 3.640939f)
	{
		m_spriteActive = m_spriteW;
	}
	else if (m_angle > 3.640939f && m_angle <= 4.055f)
	{
		m_spriteActive = m_spriteNW;
	}
	else if (m_angle > 4.055f && m_angle <= 4.838697f)
	{
		m_spriteActive = m_spriteN;
	}
	else if (m_angle > 4.838697f && m_angle <= 5.761351f)
	{
		m_spriteActive = m_spriteNE;
	}

}