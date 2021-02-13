#include "RTS.h"
#include "Game.h"

extern Game*		g_game;

Map::Map()
{
	m_bgSprite = 0;
	m_cells = 0;
	m_cellsX = m_cellsY = 0;
	m_cellsLeftY = m_cellsLeftX = 0;
	m_cellsScreenX = m_cellsScreenY = 0;
}

Map::~Map()
{
	Destroy();
}

v2f Map::GetSnapPosition(const v2f& gameCursorPosition)
{
	auto pos = gameCursorPosition + *g_game->m_spriteCameraPosition - g_game->m_screenHalfSize;
	if (pos.x < 0.f)
		pos.x = 0.f;

	if (pos.y < 0.f)
		pos.y = 0.f;

	if (pos.x != 0.f)
		pos.x -= (f32)((int)pos.x % (int)GAME_MAP_GRID_SIZE);
	if (pos.y != 0.f)
		pos.y -= (f32)((int)pos.y % (int)GAME_MAP_GRID_SIZE);
	return pos;
}

void Map::GetCellInds(s32& x, s32& y, const v2f& position)
{
	f32 m = 1.f / GAME_MAP_GRID_SIZE;
	x = s32((position.x) * m);
	y = s32((position.y) * m);
	if (x < 0) x = 0;
	if (y < 0) y = 0;
}

v2f Map::GetCellPosition(s32 x, s32 y)
{
	v2f pos;

	pos.x = (x) * GAME_MAP_GRID_SIZE;
	pos.y = (y) * GAME_MAP_GRID_SIZE;

	return pos;
}

MapCell * Map::GetCellUnderCursor(const v2f& gameCursorPosition)
{
	auto pos = GetSnapPosition(gameCursorPosition);

	s32 CellIndexX = 0;
	s32 CellIndexY = 0;

	GetCellInds(CellIndexX, CellIndexY, pos);

	if (CellIndexX >= m_cellsX)CellIndexX = m_cellsX - 1;
	if (CellIndexY >= m_cellsY)CellIndexY = m_cellsY - 1;

	return &m_cells[CellIndexY][CellIndexX];
}


void Map::FindCellPosition()
{
	m_cellPosition = GetSnapPosition(v2f());

	GetCellInds(m_screenCellLT.x, m_screenCellLT.y, m_cellPosition);
	
	m_cellsLeftX = (m_cellsX - m_screenCellLT.x);
	m_cellsLeftY = (m_cellsY - m_screenCellLT.y);

	m_screenCellRB.x = m_screenCellLT.x + m_cellsScreenX;
	m_screenCellRB.y = m_screenCellLT.y + m_cellsScreenY;

	if (m_cellsLeftX > m_cellsScreenX)m_cellsLeftX = m_cellsScreenX;
	if (m_cellsLeftY > m_cellsScreenY)m_cellsLeftY = m_cellsScreenY;

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
			m_bgSpritePositions.push_back(pos);
		}
	}

	m_leftTop.x = m_bgSpritePositions[0].x;
	m_leftTop.y = m_bgSpritePositions[0].y;
	m_rightBottom.x = m_bgSpritePositions[m_bgSpritePositions.size() - 1].x + spriteSize;
	m_rightBottom.y = m_bgSpritePositions[m_bgSpritePositions.size() - 1].y + spriteSize;

	printf("LT:[%f][%f] RB:[%f][%f]\n", m_leftTop.x, m_leftTop.y, m_rightBottom.x, m_rightBottom.y);

	m_cellsX = s32(m_mapSizeX / GAME_MAP_GRID_SIZE) + 1;
	m_cellsY = s32(m_mapSizeY / GAME_MAP_GRID_SIZE) + 1;
	m_cells = new MapCell*[m_cellsY];
	for (s32 i = 0; i < m_cellsY; ++i)
	{
		m_cells[i] = new MapCell[m_cellsX];
	
		if (i == 0 || i == m_cellsY-1)
		{
			for (s32 o = 0; o < m_cellsX; ++o)
			{
				m_cells[i][o].m_flags |= MapCell::flag_wall;
			}
		}
		else
		{
			for (s32 o = 0; o < m_cellsX; ++o)
			{
				if (o == 0 || o == m_cellsX-1)
				{
					m_cells[i][o].m_flags |= MapCell::flag_wall;
				}
			}
		}
	}
	m_cellsScreenX = (s32(g_game->m_screenHalfSize.x + g_game->m_screenHalfSize.x) / (int)GAME_MAP_GRID_SIZE);
	m_cellsScreenY = (s32(g_game->m_screenHalfSize.y + g_game->m_screenHalfSize.y) / (int)GAME_MAP_GRID_SIZE);

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
		for (s32 i = 0; i < m_cellsY; ++i)
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
		for (auto & o : m_bgObjects)
		{
			delete o.m_data;
		}
		m_bgObjects.clear();
		/*auto curr = m_bgObjects.head();
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
		}*/
	}
	{
		for (auto & o : m_structs)
		{
			delete o.m_data;
		}
		m_structs.clear();
	}
}

void Map::AddStructure(GameStructureNode* node)
{
	m_structs.push_back(node);
}

void Map::SortRenderSprites()
{
	static renderNode rn;
	m_renderSprites.clear();
	for (auto & o : m_structs)
	{
		s32 CellIndexX = 0;
		s32 CellIndexY = 0;
		GetCellInds(CellIndexX, CellIndexY, o.m_data->m_position);

		if ((CellIndexX + o.m_data->m_struct->m_fullSizeX) - m_screenCellLT.x < 0) continue;
		if (CellIndexY - m_screenCellLT.y < 0) continue;
		
		if (CellIndexX > m_screenCellRB.x) continue;
		if ((CellIndexY - o.m_data->m_struct->m_fullSizeY) > m_screenCellRB.y) continue;

		rn.m_sprite = o.m_data->m_struct->m_sprite;
		rn.m_position = o.m_data->m_position;
		m_renderSprites.push_back(rn);
	}

	struct _pred
	{
		bool operator() (const Map::renderNode& a, const Map::renderNode& b) const
		{
			return a.m_position.y > b.m_position.y;
		}
	}_p;

	m_renderSprites.sort_insertion(_p);

}