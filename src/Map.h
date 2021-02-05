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
	
	yySprite*	m_bgSprite;
	f32			m_bgSpriteRadius;

	yyArraySmall<v2f>		m_bgSpritePositions;
	yyListFast<MapSprite*>	m_bgObjects;
};

#endif