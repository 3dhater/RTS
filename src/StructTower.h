#ifndef __RTS_STRUCT_TOWER_
#define __RTS_STRUCT_TOWER_

class GameStructureTower : public GameStructure
{
	yySprite * m_spriteS;
	yySprite * m_spriteSE;
	yySprite * m_spriteSW;
	yySprite * m_spriteW;
	yySprite * m_spriteE;
	yySprite * m_spriteN;
	yySprite * m_spriteNW;
	yySprite * m_spriteNE;
	yySprite * m_spriteActive;

	f32 m_angle;

public:
	GameStructureTower();
	virtual ~GameStructureTower();

	virtual void Init();
	virtual void Draw();
	virtual void Update(f32 dt);
};

#endif