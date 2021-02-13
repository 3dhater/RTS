#ifndef __RTS_MAPH_
#define __RTS_MAPH_

#include <Windows.h>

struct MapSprite
{
	MapSprite() {
		m_spritePtr = 0;
		m_gui_text = 0;
	}
	~MapSprite() {
		if (m_gui_text)
			yyGUIDeleteElement(m_gui_text);
	}

	yySprite* m_spritePtr;

	yyGUIText* m_gui_text;


	f32 m_radius;
	yyStringA m_name;
	v2f m_spritePosition;
};

#define GAME_MAP_GRID_SIZE 10.f
#pragma pack(push,1)
struct MapCell
{
	MapCell() 
	{
		m_flags = 0;
	}
	u8 m_flags;

	enum flag
	{
		flag_clear = 1,
		flag_wall = 2,
		flag_structure  = 4
	};
};
#pragma pack(pop)

enum class GameStructureType
{
	test
};

#pragma pack(push,1)
struct GameStructure
{
	GameStructure()
	{
		m_field = 0;
		m_fullSizeX = 0;
		m_fullSizeY = 0;
		m_sprite = 0;
	}
	~GameStructure()
	{
		if (m_field)
		{
			for (int y = 0; y < m_fullSizeY; ++y)
			{
				delete[] m_field[y];
			}
			delete[] m_field;
		}
	}

	void init(int full_x, int full_y, int site_x, int site_y, const char* spritePath)
	{
		m_fullSizeX = full_x;
		m_fullSizeY = full_y;

		m_siteSizeX = site_x;
		m_siteSizeY = site_y;

		m_field = new u8*[site_y];
		for (int y = 0; y < site_y; ++y)
		{
			m_field[y] = new u8[site_x];
			for (int x = 0; x < site_x; ++x)
			{
				m_field[y][x] = MapCell::flag::flag_structure;
			}
		}

		m_sprite = GetSprite(spritePath, 6);
	}

	// bounding box for sprite in cells
	u8 m_fullSizeX;
	u8 m_fullSizeY;

	// building site size in cells
	u8 m_siteSizeX;
	u8 m_siteSizeY;

	u8** m_field;
	yySprite * m_sprite;
};
#pragma pack(pop)

// GameStructure описывает уникальную структуру при старте игры
// при добавлении зданий, нужно добавлять некие единицы, описывающие это здание
//  тип здания - GameStructure
//  позиция
//  кому оно принадлежит
//  возможно уровень ХП

struct GameStructureNode
{
	GameStructureNode()
	{
		m_struct = 0;
	}
	GameStructure * m_struct;
	v2f m_position;
};
class Map
{
public:
	Map();
	~Map();
	void Generate();
	void Destroy();

	f32 m_mapSizeX;
	f32 m_mapSizeY;
	f32 m_mapHalfSizeX;
	f32 m_mapHalfSizeY;

	v2f m_leftTop;
	v2f m_rightBottom;

	MapCell** m_cells;
	s32 m_cellsX;
	s32 m_cellsY;
	// позицию каждой ячейки можно вычислить зная позицию левой верхней ячейки
	v2f m_cellPosition;
	//s32 m_firstCellIndexY;
	//s32 m_firstCellIndexX;
	v2i m_screenCellLT;
	v2i m_screenCellRB;
	
	s32 m_cellsLeftY;
	s32 m_cellsLeftX;

	s32 m_cellsScreenX;
	s32 m_cellsScreenY;

	void FindCellPosition();
	void GetCellInds(s32& x, s32& y, const v2f& position);
	
	MapCell* GetCellUnderCursor(const v2f& gameCursorPosition);
	v2f GetSnapPosition(const v2f& gameCursorPosition);
	v2f GetCellPosition(s32 x, s32 y);
	
	yySprite*	m_bgSprite;
	f32			m_bgSpriteRadius;

	yyArraySmall<v2f>		m_bgSpritePositions;
	yyListFast<MapSprite*>	m_bgObjects;
	
	void AddStructure(GameStructureNode*);
	yyListFast<GameStructureNode*>	m_structs;

	struct renderNode
	{
		yySprite* m_sprite;
		v2f m_position;
	};
	yyArraySimple<renderNode> m_renderSprites;
	void SortRenderSprites();
};

#endif