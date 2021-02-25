#ifndef __RTS_STRUCT_
#define __RTS_STRUCT_

#include "SpriteBase.h"

enum class GameStructType
{
	Base,
	Tower
};

class GameStructure : public GameSpriteBase
{
public:
	GameStructure()
	{
		m_player = 0;
		m_field = 0;
		m_fullSizeX = 0;
		m_fullSizeY = 0;
		m_sprite = 0;
//		m_spriteBG = 0;
		m_healthMax = 1000;
		m_healthCurr = 1000;
		m_type = GameStructType::Base;
	}
	virtual ~GameStructure()
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
	virtual void Init() = 0;
	
	// bounding box for sprite in cells
	u8 m_fullSizeX;
	u8 m_fullSizeY;

	// building site size in cells
	u8 m_siteSizeX;
	u8 m_siteSizeY;

	u8** m_field;
	yySprite * m_sprite;
	//yySprite * m_spriteBG;

	GamePlayer* m_player;
	v4f m_rect;
	s32 m_healthMax;
	s32 m_healthCurr;

	GameStructType m_type;
};

#include "StructBase.h"
#include "StructTower.h"

#endif