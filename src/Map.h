#ifndef __RTS_MAPH_
#define __RTS_MAPH_

#include <Windows.h>

class GameStructure;
class GamePlayer;
struct MapBGSprite
{
	MapBGSprite() {
		m_spritePtr = 0;
		m_gui_text = 0;
	}
	~MapBGSprite() {
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
		flag_clear =		BIT(0),
		flag_wall =			BIT(1),
		flag_structure = BIT(2),

		// каждый игрок должен уметь строить где-то, но не сразу везде.
		// зона должна быть доступна или закрываться при постройке или 
		//  при уничтожении здания.
		// Видимость карты так-же строится на основе ячеек карты. У каждого
		//  игрока своя видимость.
		flag_p1_buildZone = BIT(3),
		flag_p1_view =	BIT(4),
		flag_p2_buildZone = BIT(5),
		flag_p2_view = BIT(6)

	};
};
#pragma pack(pop)



class Map
{
public:
	Map();
	~Map();
	void Generate();
	void Destroy();
	void InitFromFile(const char*);

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
	
	MapCell* GetCell(const v2f& coord);
	v2f GetSnapPosition(const v2f& pos);
	v2f GetCellPosition(s32 x, s32 y);

	void SetGridFlag(const v2f& position, f32 radius, u32 flag);
	
	yySprite*	m_bgSprite;
	f32			m_bgSpriteRadius;

	yyArraySmall<v2f>		m_bgSpritePositions;
	yyListFast<MapBGSprite*>	m_bgObjects;
	MapBGSprite* GetNewMapBGSprite(const wchar_t* name, const char* fn);
	
	void AddStructure(GameStructure * strct, const v2f& position);
	


	v2f m_player1Position;
	v2f m_player2Position;
};

#endif