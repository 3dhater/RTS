#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"
#include "math\math.h"
#include "containers\list.h"
#include "Game.h"
#include "Player.h"

extern SpriteCache *		g_spriteCachePtr;
extern yyVideoDriverAPI*	g_videoDriver;
extern v2f					g_rawInputCursorPosition;
extern yyInputContext*		g_inputContex;
extern v2f  g_gameCursorPosition;
extern v2f*	g_spriteCameraPosition;
extern v2f  g_screenHalfSize;
extern Map* g_map;
extern yyGUIFont*			g_defaultFont;
extern v2f					g_cameraLimits;
extern f32					g_screenRectRadius;

void Game::GameStep(f32 dt)
{
	if (m_inputContext->isKeyHit(yyKey::K_H))
	{
		ToHome();
	}

	v2f cursorInWorld = GetCursorPositionInWorld();
	auto cellUnderCursor = m_map->GetCell(m_gameCursorPosition);

	if (m_inputContext->m_isLMBDown && !m_createModeStructure)
	{
		m_selectedStruct = 0;
		for (auto strct : m_player1->m_structs)
		{
		/*	printf("Click %f %f [%f %f %f %f]\n", cursorInWorld.x, cursorInWorld.y,
				strct.m_data->m_rect.x, strct.m_data->m_rect.y, strct.m_data->m_rect.z,
				strct.m_data->m_rect.w);*/
			if (math::pointInRect(cursorInWorld.x, cursorInWorld.y, strct.m_data->m_rect))
			{
				if (cellUnderCursor->m_flags & MapCell::flag_p1_view)
				{
					m_selectedStruct = strct.m_data;
				}
				break;
			}
		}

		if (!m_selectedStruct)
		{
			for (auto strct : m_player2->m_structs)
			{
				if (math::pointInRect(cursorInWorld.x, cursorInWorld.y, strct.m_data->m_rect))
				{
					if (cellUnderCursor->m_flags & MapCell::flag_p1_view)
					{
						m_selectedStruct = strct.m_data;
					}
					break;
				}
			}
		}
	}

	if (m_inputContext->m_isLMBDown && m_createModeStructure && m_canPlaceStructureOnMap)
	{
		m_player1->AddStructure(m_createModeStructure, m_createModeStructure->m_position, 500);
		m_map->AddStructure(m_createModeStructure, m_createModeStructure->m_position);
		m_map->SetGridFlag(m_createModeStructure->m_position, 100.f, MapCell::flag_p1_view);
		m_map->SetGridFlag(m_createModeStructure->m_position, 150.f, MapCell::flag_p1_buildZone);
		m_createModeStructure = 0;
		this->LeaveCreateMode();
	}

	f32 offsetX = -m_spriteCameraPosition->x + m_screenHalfSize.x;
	f32 offsetY = -m_spriteCameraPosition->y + m_screenHalfSize.y;

	m_gpu->UseDepth(false);
	m_gpu->UseBlend(false);

	DrawGround();
	DrawGroundSprites();
	for (auto strct : m_player1->m_structs)
	{
		strct.m_data->Update(dt);
	}
	for (auto strct : m_player2->m_structs)
	{
		strct.m_data->Update(dt);
	}
	DrawVisibleSprites();

	s32 cix = m_map->m_screenCellLT.x;
	s32 ciy = m_map->m_screenCellLT.y;

	auto _x = m_map->m_cellsLeftX + 1;
	auto _y = m_map->m_cellsLeftY + 2;

	if (_x >= m_map->m_cellsX) --_x;
	if (_y >= m_map->m_cellsY) --_y;

	for (u32 y = 0; y < _y; ++y)
	{
		for (u32 x = 0; x < _x; ++x)
		{
			if ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_p1_view) == 0)
			{
				auto cellPos = m_map->GetCellPosition(cix, ciy);

				m_spriteGridBlack->m_objectBase.m_globalMatrix[3].x = cellPos.x;
				m_spriteGridBlack->m_objectBase.m_globalMatrix[3].y = cellPos.y;
				m_gpu->DrawSprite(m_spriteGridBlack);
			}
			++cix;
		}
		++ciy;
		if (ciy >= m_map->m_cellsX)
			break;
		cix = m_map->m_screenCellLT.x;
	}

	if (m_createModeStructure)
	{
		auto pos = m_map->GetSnapPosition(m_gameCursorPosition + *m_spriteCameraPosition - m_screenHalfSize);
		s32 CellIndexX = 0;
		s32 CellIndexY = 0;
		m_map->GetCellInds(CellIndexX, CellIndexY, pos);
		
		// определение выхода за пределы карты
		s32 cellsLeftX = (m_map->m_cellsX - CellIndexX);
		s32 cellsLeftY = (m_map->m_cellsY - CellIndexY);

		if (cellsLeftX <= m_createModeStructure->m_siteSizeX)
		{
			pos.x -= (m_createModeStructure->m_siteSizeX - cellsLeftX+1) * GAME_MAP_GRID_SIZE;
			// нужено будет делать повторно GetCellInds
		}
		if (cellsLeftY <= 0)
		{
			pos.y += (cellsLeftY-1) * GAME_MAP_GRID_SIZE;
		}
		else if (cellsLeftY > m_map->m_cellsY - m_createModeStructure->m_siteSizeY)
		{
			pos.y += (cellsLeftY - (m_map->m_cellsY - m_createModeStructure->m_siteSizeY)) * GAME_MAP_GRID_SIZE;
		}

	//	pos = m_map->GetSnapPosition(pos);
		m_createModeStructure->SetPosition(pos);
		m_createModeStructure->Draw();
		
		m_canPlaceStructureOnMap = false;
		
		m_map->GetCellInds(CellIndexX, CellIndexY, pos);

		bool good = true;
		s32 cix = CellIndexX;
		s32 ciy = CellIndexY;
		for (u32 y = 0; y < m_createModeStructure->m_siteSizeY; ++y)
		{
			for (u32 x = 0; x < m_createModeStructure->m_siteSizeX; ++x)
			{
				auto cellPos = m_map->GetCellPosition(cix, ciy);
				if ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_structure)
					|| (m_map->m_cells[ciy][cix].m_flags & MapCell::flag_wall)
					|| ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_p1_view) == 0)
					|| ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_p1_buildZone) == 0)
					)
				{
					good = false;
					m_spriteGridRed->m_objectBase.m_globalMatrix[3].x = cellPos.x;
					m_spriteGridRed->m_objectBase.m_globalMatrix[3].y = cellPos.y;
					m_gpu->DrawSprite(m_spriteGridRed);
				}
				else
				{
					m_spriteGridWhite->m_objectBase.m_globalMatrix[3].x = cellPos.x;
					m_spriteGridWhite->m_objectBase.m_globalMatrix[3].y = cellPos.y;
					m_gpu->DrawSprite(m_spriteGridWhite);
				}
				++cix;
			}
			--ciy;
			if (ciy < 0)
			{
				break;
			}
			cix = CellIndexX;
		}

		
		if (good)
		{
			m_canPlaceStructureOnMap = true;

			cix = CellIndexX;
			ciy = CellIndexY;
			for (u32 y = 0; y < m_createModeStructure->m_siteSizeY; ++y)
			{
				for (u32 x = 0; x < m_createModeStructure->m_siteSizeX; ++x)
				{
					auto cellPos = m_map->GetCellPosition(cix, ciy);
					m_spriteGridGreen->m_objectBase.m_globalMatrix[3].x = cellPos.x;
					m_spriteGridGreen->m_objectBase.m_globalMatrix[3].y = cellPos.y;
					m_gpu->DrawSprite(m_spriteGridGreen);
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

	}

	if (m_inputContext->isKeyHold(yyKey::K_LALT) )
	{
		auto beginPos = m_map->m_cellPosition;
		auto currPos = beginPos;
		//	int ii = 0;
		for (int y = 0; y < m_map->m_cellsLeftY; ++y)
		{
			s32 indexY = y + m_map->m_screenCellLT.y;

			for (int x = 0; x < m_map->m_cellsLeftX; ++x)
			{
				s32 indexX = x + m_map->m_screenCellLT.x;

				/*	if(ii == 0)
				{
				ii = 1;
				printf("[%f][%f]\n", beginPos.x, beginPos.y);
				}*/
				yySprite * sprite = m_spriteGridWhite;
				if (m_map->m_cells[indexY][indexX].m_flags & MapCell::flag_clear)
				{
					sprite = m_spriteGridWhite;
				}
				else if (m_map->m_cells[indexY][indexX].m_flags & MapCell::flag_structure)
				{
					sprite = m_spriteGridBlue;
				}
				else if (m_map->m_cells[indexY][indexX].m_flags & MapCell::flag_wall)
				{
					sprite = m_spriteGridRed;
				}
				sprite->m_objectBase.m_globalMatrix[3].x = currPos.x;
				sprite->m_objectBase.m_globalMatrix[3].y = currPos.y;
				m_gpu->DrawSprite(sprite);
				currPos.x += GAME_MAP_GRID_SIZE;
			}
			currPos.y += GAME_MAP_GRID_SIZE;
			currPos.x = beginPos.x;
		}
	}

	if (m_selectedStruct)
	{
		m_gpu->DrawLine2D(
			v3f(m_selectedStruct->m_rect.x + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.y + m_worldToScreenOffset.y, 0.f),
			v3f(m_selectedStruct->m_rect.z + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(m_selectedStruct->m_rect.x + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.y + m_worldToScreenOffset.y, 0.f),
			v3f(m_selectedStruct->m_rect.x + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.w + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(m_selectedStruct->m_rect.x + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.w + m_worldToScreenOffset.y, 0.f),
			v3f(m_selectedStruct->m_rect.z + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.w + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(m_selectedStruct->m_rect.z + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.w + m_worldToScreenOffset.y, 0.f),
			v3f(m_selectedStruct->m_rect.z + m_worldToScreenOffset.x,
				m_selectedStruct->m_rect.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
	}
}