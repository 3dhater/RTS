#ifndef __RTS_PLAYER_
#define __RTS_PLAYER_
#include "Struct.h"
class GamePlayer
{
public:
	GamePlayer();
	~GamePlayer();

	bool m_isAI;

	void AddStructure(GameStructure * strct, const v2f& position, s32 maxHealth);
	yyListFast<GameStructure*> m_structs;

	GameStructure* GetBase();
	//GameStructureNode* m_structureBase;
};

#endif