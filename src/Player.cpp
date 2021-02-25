#include "RTS.h"
#include "Game.h"
#include "Player.h"
#include "Struct.h"

extern Game*		g_game;

GamePlayer::GamePlayer()
{
}

GamePlayer::~GamePlayer()
{
	for (auto & o : m_structs)
	{
		delete o.m_data;
	}
	m_structs.clear();
}

GameStructure* GamePlayer::GetBase()
{
	for (auto & o : m_structs)
	{
		if(o.m_data->m_type == GameStructType::Base)
			return o.m_data;
	}
	return 0;
}

void GamePlayer::AddStructure(GameStructure * strct, const v2f& position, s32 maxHealth)
{
	m_structs.push_back(strct);
	strct->m_position = position;
	strct->m_player = this;
	
	strct->m_healthMax = maxHealth;
	strct->m_healthCurr = maxHealth;

	strct->Init();
}