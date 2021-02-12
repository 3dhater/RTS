#include "RTS.h"
#include "Game.h"

extern Game*		g_game;

Map::Map()
{
	m_bgSprite = 0;
	m_cells = 0;
	m_cellsX = m_cellsY = 0;
	m_firstCellIndexY = m_firstCellIndexX = 0;
	m_cellsLeftY = m_cellsLeftX = 0;
}

Map::~Map()
{
	Destroy();
}

MapCell * Map::GetCellUnderCursor(const v2f& gameCursorPosition)
{
	f32 m = 1.f / GAME_MAP_GRID_SIZE;
	auto pos = gameCursorPosition + *g_game->m_spriteCameraPosition - g_game->m_screenHalfSize;
	if (pos.x < 0.f)
		pos.x = 0.f;

	if (pos.y < 0.f)
		pos.y = 0.f;

	if (pos.x != 0.f)
		pos.x -= (f32)((int)pos.x % (int)GAME_MAP_GRID_SIZE);
	if (pos.y != 0.f)
		pos.y -= (f32)((int)pos.y % (int)GAME_MAP_GRID_SIZE);

	auto CellIndexX = s32((pos.x) * m);
	auto CellIndexY = s32((pos.y) * m);
	if (CellIndexX < 0) CellIndexX = 0;
	if (CellIndexY < 0) CellIndexY = 0;

	if (CellIndexX >= m_cellsX)CellIndexX = m_cellsX-1;
	if (CellIndexY >= m_cellsY)CellIndexY = m_cellsY - 1;

	return &m_cells[CellIndexY][CellIndexX];
}

void Map::FindCellPosition()
{
	m_cellPosition.x = g_game->m_spriteCameraPosition->x - g_game->m_screenHalfSize.x;
	if (m_cellPosition.x < 0.f)
		m_cellPosition.x = 0.f;

	m_cellPosition.y = g_game->m_spriteCameraPosition->y - g_game->m_screenHalfSize.y;
	if (m_cellPosition.y < 0.f)
		m_cellPosition.y = 0.f;

	if (m_cellPosition.x != 0.f)
		m_cellPosition.x -= (f32)((int)m_cellPosition.x % (int)GAME_MAP_GRID_SIZE);
	if (m_cellPosition.y != 0.f)
		m_cellPosition.y -= (f32)((int)m_cellPosition.y % (int)GAME_MAP_GRID_SIZE);
//	printf("[%f][%f]\n", m_cellPosition.x, m_cellPosition.y);
//	printf("[%f][%f][%f] - [%i][%i]\n", 
//		m_cellPosition.x + m_mapHalfSizeX, m_cellPosition.y + m_mapHalfSizeY, m_mapSizeX,
//		s32((m_cellPosition.x + m_mapHalfSizeX) * 0.1f), s32((m_cellPosition.y + m_mapHalfSizeY) * 0.1f));
	m_firstCellIndexX = s32((m_cellPosition.x) * 0.1f);
	m_firstCellIndexY = s32((m_cellPosition.y) * 0.1f);
	if (m_firstCellIndexX < 0) m_firstCellIndexX = 0;
	if (m_firstCellIndexY < 0) m_firstCellIndexY = 0;


	s32 cellsScreenX = (s32(g_game->m_screenHalfSize.x + g_game->m_screenHalfSize.x) / (int)GAME_MAP_GRID_SIZE);
	s32 cellsScreenY = (s32(g_game->m_screenHalfSize.y + g_game->m_screenHalfSize.y) / (int)GAME_MAP_GRID_SIZE);
	
	m_cellsLeftY = (m_cellsY - m_firstCellIndexY);
	m_cellsLeftX = (m_cellsX - m_firstCellIndexX);

	if (m_cellsLeftX > cellsScreenX)m_cellsLeftX = cellsScreenX;
	if (m_cellsLeftY > cellsScreenY)m_cellsLeftY = cellsScreenY;

	//printf("[%i][%i]\t[%i][%i]\n", m_firstCellIndexY, m_firstCellIndexX, m_cellsLeftY, m_cellsLeftX);
}

void Map::Generate()
{
	Destroy();
	float spriteSize = 512.f;
	float spriteSizeHalf = spriteSize * 0.5f;
	m_bgSprite = yyCreateSprite(v4f(0.f, 0.f, spriteSize, spriteSize), yyGetTextureResource("../res/textures/bg/grass.dds", false, false, true), false);

	m_bgSpriteRadius = v2f(spriteSize * 0.5f, spriteSize * 0.5f).distance(v2f());

	m_mapSizeX = spriteSize * g_game->m_mapGenSizeX;
	m_mapSizeY = spriteSize * g_game->m_mapGenSizeY;
	m_mapHalfSizeX = m_mapSizeX * 0.5f;
	m_mapHalfSizeY = m_mapSizeY * 0.5f;

	for (int y = 0; y < g_game->m_mapGenSizeY; ++y) 
	{
		for (int x = 0; x < g_game->m_mapGenSizeX; ++x)
		{
			v2f pos = v2f(spriteSize * (f32)x + (0.f* (f32)x), spriteSize * (f32)y + (0.f* (f32)y));
			//pos.x -= m_mapHalfSizeX;
			//pos.y -= m_mapHalfSizeY;
			m_bgSpritePositions.push_back(pos);
		}
	}

	m_leftTop.x = m_bgSpritePositions[0].x;
	m_leftTop.y = m_bgSpritePositions[0].y;
	m_rightBottom.x = m_bgSpritePositions[m_bgSpritePositions.size() - 1].x + spriteSize;
	m_rightBottom.y = m_bgSpritePositions[m_bgSpritePositions.size() - 1].y + spriteSize;

	printf("LT:[%f][%f] RB:[%f][%f]\n", m_leftTop.x, m_leftTop.y, m_rightBottom.x, m_rightBottom.y);

	m_cellsX = (m_mapSizeX / GAME_MAP_GRID_SIZE) + 1;
	m_cellsY = (m_mapSizeY / GAME_MAP_GRID_SIZE) + 1;
	m_cells = new MapCell*[m_cellsY];
	for (u32 i = 0; i < m_cellsY; ++i)
	{
		m_cells[i] = new MapCell[m_cellsX];
	
		if (i == 0 || i == m_cellsY-1)
		{
			for (u32 o = 0; o < m_cellsX; ++o)
			{
				m_cells[i][o].m_flags |= MapCell::flag_wall;
			}
		}
		else
		{
			for (u32 o = 0; o < m_cellsX; ++o)
			{
				if (o == 0 || o == m_cellsX-1)
				{
					m_cells[i][o].m_flags |= MapCell::flag_wall;
				}
			}
		}
	}
	m_cells[10][10].m_flags |= MapCell::flag_wall;
	m_cells[10][11].m_flags |= MapCell::flag_wall;
	m_cells[10][12].m_flags |= MapCell::flag_wall;
	
	m_cells[11][10].m_flags |= MapCell::flag_wall;
	//m_cells[11][11].m_flags |= MapCell::flag_wall;
	m_cells[11][12].m_flags |= MapCell::flag_wall;

	m_cells[12][10].m_flags |= MapCell::flag_wall;
	m_cells[12][11].m_flags |= MapCell::flag_wall;
	m_cells[12][12].m_flags |= MapCell::flag_wall;
	FindCellPosition();
}

void Map::Destroy()
{
	if (m_cells)
	{
		for (u32 i = 0; i < m_cellsY; ++i)
		{
			delete[] m_cells[i];
		}
		delete[] m_cells;
	}
	if (m_bgSprite)
	{
		if (m_bgSprite->m_texture)
		{
			g_game->m_gpu->UnloadTexture(m_bgSprite->m_texture);
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
