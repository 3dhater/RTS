#include "RTS.h"
#include "Game.h"
#include "Struct.h"
#include "Player.h"

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

v2f Map::GetSnapPosition(const v2f& position)
{
	auto pos = position;// +*g_game->m_spriteCameraPosition - g_game->m_screenHalfSize;
	if (pos.x < 0.f)
		pos.x = 0.f;

	if (pos.y < 0.f)
		pos.y = 0.f;

	if (pos.x != 0.f)
		pos.x -= (f32)((int)std::floor(pos.x) % (int)GAME_MAP_GRID_SIZE);
	if (pos.y != 0.f)
		pos.y -= (f32)((int)std::floor(pos.y) % (int)GAME_MAP_GRID_SIZE);
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

MapCell * Map::GetCell(const v2f& coord)
{
	auto world_position = coord +*g_game->m_spriteCameraPosition - g_game->m_screenHalfSize;
	auto pos = GetSnapPosition(world_position);

	s32 CellIndexX = 0;
	s32 CellIndexY = 0;

	GetCellInds(CellIndexX, CellIndexY, pos);

	if (CellIndexX >= m_cellsX)CellIndexX = m_cellsX - 1;
	if (CellIndexY >= m_cellsY)CellIndexY = m_cellsY - 1;

	return &m_cells[CellIndexY][CellIndexX];
}

void Map::SetGridFlag(const v2f& _position, f32 radius, u32 flag)
{
	v2f position = _position;

	auto pos = GetSnapPosition(position);
	s32 CellIndexX = 0;
	s32 CellIndexY = 0;
	GetCellInds(CellIndexX, CellIndexY, pos);

	s32 gridRadius = (s32)radius / (s32)GAME_MAP_GRID_SIZE;

	s32 cix = CellIndexX;
	s32 ciy = CellIndexY;

	for (s32 y = 0; y < gridRadius; ++y)
	{
		for (s32 x = 0; x < gridRadius; ++x)
		{
			m_cells[ciy][cix].m_flags |= flag;

			++cix;
			if (cix == m_cellsX) break;
		}
		++ciy;
		if (ciy == m_cellsY) break;
		cix = CellIndexX;
	}

	cix = CellIndexX;
	ciy = CellIndexY;
	for (s32 y = 0; y < gridRadius; ++y)
	{
		for (s32 x = 0; x < gridRadius; ++x)
		{
			m_cells[ciy][cix].m_flags |= flag;

			++cix;
			if (cix == m_cellsX) break;
		}
		--ciy;
		if (ciy < 0) break;
		cix = CellIndexX;
	}

	cix = CellIndexX;
	ciy = CellIndexY;
	for (s32 y = 0; y < gridRadius; ++y)
	{
		for (s32 x = 0; x < gridRadius; ++x)
		{
			m_cells[ciy][cix].m_flags |= flag;

			--cix;
			if (cix < 0) break;
		}
		--ciy;
		if (ciy < 0) break;
		cix = CellIndexX;
	}

	cix = CellIndexX;
	ciy = CellIndexY;
	for (s32 y = 0; y < gridRadius; ++y)
	{
		for (s32 x = 0; x < gridRadius; ++x)
		{
			m_cells[ciy][cix].m_flags |= flag;

			--cix;
			if (cix == m_cellsX) break;
		}
		++ciy;
		if (ciy == m_cellsY) break;
		cix = CellIndexX;
	}
}
void Map::FindCellPosition()
{
	auto world_position = v2f() + *g_game->m_spriteCameraPosition - g_game->m_screenHalfSize;
	m_cellPosition = GetSnapPosition(world_position);

	GetCellInds(m_screenCellLT.x, m_screenCellLT.y, m_cellPosition);
	
	m_cellsLeftX = (m_cellsX - m_screenCellLT.x);
	m_cellsLeftY = (m_cellsY - m_screenCellLT.y);

	m_screenCellRB.x = m_screenCellLT.x + m_cellsScreenX;
	m_screenCellRB.y = m_screenCellLT.y + m_cellsScreenY;

	if (m_cellsLeftX > m_cellsScreenX)m_cellsLeftX = m_cellsScreenX;
	if (m_cellsLeftY > m_cellsScreenY)m_cellsLeftY = m_cellsScreenY;

	//printf("[%i][%i]\t[%i][%i]\n", m_firstCellIndexY, m_firstCellIndexX, m_cellsLeftY, m_cellsLeftX);
}

MapBGSprite* Map::GetNewMapBGSprite(const wchar_t* name, const char* fn)
{
	static u32 nameCounter = 0;

	auto newSprite = GetSprite(fn, 8);
	if (!newSprite)
		return nullptr;

	MapBGSprite* newMapSprite = new MapBGSprite;

#ifdef YY_DEBUG
	newMapSprite->m_name = name;
	newMapSprite->m_name += "_";
	newMapSprite->m_name += nameCounter++;
	yyStringW wstr;
	wstr += newMapSprite->m_name.data();
	newMapSprite->m_gui_text = yyGUICreateText(v2f(), g_game->m_defaultFont, wstr.data());
#endif

	newMapSprite->m_spritePtr = newSprite;

	v2i textureSize;
	yyGetTextureSize(newSprite->m_texture, &textureSize);
	newMapSprite->m_radius = v2f((f32)textureSize.x, (f32)textureSize.y).distance(v2f());

	m_bgObjects.push_back(newMapSprite);
	return newMapSprite;
}
void Map::InitFromFile(const char* fn)
{
	FILE * f = fopen(fn, "rb");

	fread(&g_game->m_mapGenSizeX, sizeof(s32), 1, f);
	fread(&g_game->m_mapGenSizeY, sizeof(s32), 1, f);

	Generate();

	fread(&m_player1Position, sizeof(v2f), 1, f);
	fread(&m_player2Position, sizeof(v2f), 1, f);

	m_player1Position = this->GetSnapPosition(m_player1Position);
	m_player2Position = this->GetSnapPosition(m_player2Position);

	u32 bgObjectsCount = 0;
	fread(&bgObjectsCount, sizeof(u32), 1, f);
	for (u32 i = 0; i < bgObjectsCount; ++i)
	{
		f32 radius = 0.f;
		fread(&radius, sizeof(f32), 1, f);

		v2f spritePosition;
		fread(&spritePosition, sizeof(v2f), 1, f);

		std::string newString;
		for (u32 o = 0; o < 1000; ++o)
		{
			u8 chr = 0;
			fread(&chr, 1, 1, f);

			if (chr != 0)
				newString += (char)chr;
			else
				break;
		}

		auto newBGObject = GetNewMapBGSprite(L"sprite", newString.data());
		newBGObject->m_radius = radius;
		newBGObject->m_spritePosition = spritePosition;
	}

	for (int y = 0; y < m_cellsY; ++y)
	{
		for (int x = 0; x < m_cellsX; ++x)
		{
			fread(&m_cells[y][x], sizeof(MapCell), 1, f);
		}
	}

	fclose(f);

	g_game->CameraSetPosition(m_player1Position);
	this->FindCellPosition();
	this->SetGridFlag(m_player1Position, 500.f, MapCell::flag_p1_view);
	this->SetGridFlag(m_player1Position, 250.f, MapCell::flag_p1_buildZone);
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

	g_game->m_cameraLimits.x = m_mapSizeX;
	g_game->m_cameraLimits.y = m_mapSizeY;
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
	/*{
		for (auto & o : m_structs)
		{
			delete o.m_data;
		}
		m_structs.clear();
	}*/
}

//void Map::AddStructure(GameStructure * strct, const v2f& position, GamePlayer* player, s32 maxHealth)
//{
//	//player->m_structureBase
//	player->m_structs.push_back(strct);
//
//	strct->m_position = position;
//	strct->m_player = player;
//	strct->m_rect.x = position.x;
//	strct->m_rect.y = position.y - (strct->m_fullSizeY * GAME_MAP_GRID_SIZE);
//	strct->m_rect.z = position.x + (strct->m_fullSizeX * GAME_MAP_GRID_SIZE);
//	strct->m_rect.w = position.y;
//	strct->m_healthMax = maxHealth;
//	strct->m_healthCurr = maxHealth;
//	
//	strct->init();
//
//	m_structs.push_back(strct);
//}

void Map::AddStructure(GameStructure * strct, const v2f& pos)
{
	s32 CellIndexX = 0;
	s32 CellIndexY = 0;
	GetCellInds(CellIndexX, CellIndexY, pos);
	
	//++CellIndexX;

	auto cix = CellIndexX;
	auto ciy = CellIndexY;

	for (u32 y = 0; y < strct->m_siteSizeY; ++y)
	{
		for (u32 x = 0; x < strct->m_siteSizeX; ++x)
		{
			m_cells[ciy][cix].m_flags |= MapCell::flag_structure;
			++cix;
		}
		--ciy;
		if (ciy < 0)
		{
			break;
		}
		cix = CellIndexX;
	}
}