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
		m_x = 0;
		m_y = 0;
	}
	~GameStructure()
	{
		if (m_field)
		{
			for (int y = 0; y < m_y; ++y)
			{
				delete[] m_field[y];
			}
			delete[] m_field;
		}
	}

	void init(int _x, int _y)
	{
		m_x = _x;
		m_y = _y;
		m_field = new u8*[_y];
		for (int y = 0; y < m_y; ++y)
		{
			m_field[y] = new u8[_x];
		}
	}

	u8 m_x;
	u8 m_y;
	u8** m_field;
};
#pragma pack(pop)

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
	u32 m_cellsX;
	u32 m_cellsY;
	// позицию каждой ячейки можно вычислить зная позицию левой верхней ячейки
	v2f m_cellPosition;
	s32 m_firstCellIndexY;
	s32 m_firstCellIndexX;
	s32 m_cellsLeftY;
	s32 m_cellsLeftX;
	void FindCellPosition();
	MapCell * GetCellUnderCursor(const v2f& gameCursorPosition);
	
	yySprite*	m_bgSprite;
	f32			m_bgSpriteRadius;

	yyArraySmall<v2f>		m_bgSpritePositions;
	yyListFast<MapSprite*>	m_bgObjects;
};

#endif