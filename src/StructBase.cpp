#include "RTS.h"
#include "Struct.h"
#include "Game.h"
extern Game*		g_game;

GameStructureBase::GameStructureBase()
{

}

GameStructureBase::~GameStructureBase()
{

}

void GameStructureBase::Init()
{
	//int full_x, int full_y, int site_x, int site_y, const char* spritePath, const char* spritePathBG

	m_fullSizeX = 16;
	m_fullSizeY = 16;

	m_siteSizeX = 16;
	m_siteSizeY = 16;

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

	m_sprite = GetSprite("../res/structs/base.png", 6);
//	if(yyFS::exists("../res/structs/baseBG.png"))
//		m_spriteBG = GetSprite("../res/structs/baseBG.png", 6);

	m_type = GameStructType::Base;

}

void GameStructureBase::Draw()
{
	m_sprite->m_objectBase.m_globalMatrix[3].x = m_position.x;
	m_sprite->m_objectBase.m_globalMatrix[3].y = m_position.y;
	g_game->m_gpu->DrawSprite(m_sprite);
}

void GameStructureBase::Update(f32 dt)
{

}