#ifndef __RTS_MAPH_
#define __RTS_MAPH_

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
struct MapCell
{
	MapCell() 
	{
		m_flags = 0;
	}
	u32 m_flags;

	enum flag
	{
		flag_clear = 1,
		flag_structure = 2
	};
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
	u32 m_cellsX;
	u32 m_cellsY;
	// позицию каждой ячейки можно вычислить зная позицию левой верхней ячейки
	v2f m_cellPosition;
	s32 m_firstCellIndexY;
	s32 m_firstCellIndexX;
	s32 m_cellsLeftY;
	s32 m_cellsLeftX;
	void FindCellPosition();
	
	yySprite*	m_bgSprite;
	f32			m_bgSpriteRadius;

	yyArraySmall<v2f>		m_bgSpritePositions;
	yyListFast<MapSprite*>	m_bgObjects;
};

#endif