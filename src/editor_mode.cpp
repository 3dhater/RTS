#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"
#include "Game.h"


s32 g_selectedListItemBGObject = -1;
MapBGSprite*			g_currentMapSprite = 0;

bool g_isNewMapWindow = false;
bool g_isEditWalls = false;
bool g_isAddTestStruct = false;

void Game::EditorStep(f32 dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (g_currentMapSprite)
	{
		if (m_inputContext->isKeyHold(yyKey::K_LSHIFT))
		{
			g_currentMapSprite->m_spritePosition = GetCursorPositionInWorld();
		}

		if (m_inputContext->m_isLMBDown)
		{
			g_currentMapSprite = 0;
		}

		if (m_inputContext->isKeyHold(yyKey::K_DELETE) && g_currentMapSprite)
		{
			auto find_result = m_map->m_bgObjects.find_by_value(g_currentMapSprite);
			if (find_result)
			{
				m_map->m_bgObjects.erase_node(find_result);
			}
			g_currentMapSprite = 0;
		}
	}

	if (ImGui::Begin("Main menu", 0, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGuiTabBarFlags mainemnuTab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("menutab", mainemnuTab_bar_flags))
		{
			if (ImGui::BeginTabItem("File"))
			{
				if (ImGui::Button("New Map"))
				{
					g_isNewMapWindow = true;
				}
				if (m_map->m_bgSpritePositions.size())
				{
					if (!g_isEditWalls)
					{
						if (ImGui::Button("Edit walls"))
						{
							g_isEditWalls = true;
						}
					}
					else
					{
						if (ImGui::Button("End edit walls"))
						{
							g_isEditWalls = false;
						}
					}
				}
				if (ImGui::Button("Add test structure"))
				{
					g_isAddTestStruct = true;
				}
				if (ImGui::Button("Set player 1 position"))
				{
					m_map->m_player1Position = GetCursorPositionInWorld();
				}
				if (ImGui::Button("Set player 2 position"))
				{
					m_map->m_player2Position = GetCursorPositionInWorld();
				}
				if (ImGui::Button("Save"))
				{
					auto str = yySaveFileDialog("Save", "Save", "rtsmap");
					if (str)
					{
						yyFS::path p = str->data();
						if (!p.has_extension())
						{
							p.string_type += ".rtsmap";
						}

						FILE * f = fopen(p.string_type.to_stringA().data(), "wb");

						yyLogWriteInfo("Save: m_mapGenSizeX %i\n", m_mapGenSizeX);
						fwrite(&m_mapGenSizeX, sizeof(s32), 1, f);

						yyLogWriteInfo("Save: m_mapGenSizeY %i\n", m_mapGenSizeY);
						fwrite(&m_mapGenSizeY, sizeof(s32), 1, f);

						yyLogWriteInfo("Save: m_player1Position %f %f\n", m_map->m_player1Position.x, m_map->m_player1Position.y);
						fwrite(&m_map->m_player1Position.x, sizeof(v2f), 1, f);
						
						yyLogWriteInfo("Save: m_player2Position %f %f\n", m_map->m_player2Position.x, m_map->m_player2Position.y);
						fwrite(&m_map->m_player2Position.x, sizeof(v2f), 1, f);
						
						u32 bgObjectsCount = m_map->m_bgObjects.find_size();
						fwrite(&bgObjectsCount, sizeof(u32), 1, f);
						for (auto & sp : m_map->m_bgObjects)
						{
							fwrite(&sp.m_data->m_radius, sizeof(f32), 1, f);
							fwrite(&sp.m_data->m_spritePosition, sizeof(v2f), 1, f);

							auto str2 = sp.m_data->m_spritePtr->m_texture->m_file.to_string();
							auto relPath = yyGetRelativePath((wchar_t*)str2.data());
							if (relPath)
							{
								auto stra = relPath->to_stringA();
								fwrite(stra.data(), stra.size(), 1, f);
								u8 zero = 0;
								fwrite(&zero, 1, 1, f);
								yyDestroy(relPath);
							}
							else
							{
								yyLogWriteWarning("No rel path %s\n", sp.m_data->m_spritePtr->m_texture->m_file.data());
							}
						}
						
						for (int y = 0; y < m_map->m_cellsY; ++y)
						{
							for (int x = 0; x < m_map->m_cellsX; ++x)
							{
								fwrite(&m_map->m_cells[y][x], sizeof(MapCell), 1, f);
							}
						}


						fclose(f);
						yyDestroy(str);
					}
				}
				if (ImGui::Button("Load"))
				{
					auto str = yyOpenFileDialog("Load", "Load", "rtsmap", "RTS map file");
					if (str)
					{
						m_map->InitFromFile(str->to_stringA().data());
						yyDestroy(str);
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("BGObjects"))
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
				if (ImGui::BeginChild("AddedBGObjects", ImVec2(ImGui::GetWindowContentRegionWidth(), 200), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove))
				{
					int n = 0;
					for (auto & sp : m_map->m_bgObjects)
					{
						if (ImGui::Selectable(sp.m_data->m_name.data(), g_selectedListItemBGObject == n))
						{
							g_selectedListItemBGObject = n;
							auto findSprite = m_map->m_bgObjects.find(n);
							if (findSprite)
							{
								g_currentMapSprite = findSprite->m_data;
							}
						}
						++n;
					}
					ImGui::EndChild();
				}
				ImGui::PopStyleColor();
				if (ImGui::Button("Add"))
				{
					auto path = yyOpenFileDialog("Select Sprite", "Select", "png bmp tga dds", "Supported files");
					if (path)
					{
						yyStringA stra;
						stra = path->to_stringA();

						yyFS::path fp = stra.data();
						auto fileName = fp.filename();
						g_currentMapSprite = m_map->GetNewMapBGSprite(fileName.string_type.data(), stra.data());

						yyDestroy(path);
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	if (g_isNewMapWindow)
	{
		if (ImGui::Begin("New Map", 0, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Set map size");
			ImGui::SliderInt("X segments", &m_mapGenSizeX, 3, 100);
			ImGui::SliderInt("Y segments", &m_mapGenSizeY, 3, 100);
			if (ImGui::Button("Create"))
			{
				m_map->Generate();
				g_isNewMapWindow = false;
				
			}
			if (ImGui::Button("Close"))
			{
				g_isNewMapWindow = false;
			}
			ImGui::End();
		}
	}
	
	m_gpu->UseDepth(false);
	m_gpu->UseBlend(false);

	DrawGround();

	{
		DrawGroundSprites();
		DrawVisibleSprites();


		v2f ppos = m_map->m_player1Position;
		ppos.x += m_worldToScreenOffset.x;
		ppos.y += m_worldToScreenOffset.y;
		m_gpu->DrawLine2D(v3f(ppos.x - 10.f, ppos.y, 0.f), v3f(ppos.x + 10.f, ppos.y, 0.f), ColorRed);
		m_gpu->DrawLine2D(v3f(ppos.x, ppos.y - 10.f, 0.f), v3f(ppos.x, ppos.y + 10.f, 0.f), ColorRed);

		ppos = m_map->m_player2Position;
		ppos.x += m_worldToScreenOffset.x;
		ppos.y += m_worldToScreenOffset.y;
		m_gpu->DrawLine2D(v3f(ppos.x - 10.f, ppos.y, 0.f), v3f(ppos.x + 10.f, ppos.y, 0.f), ColorBlue);
		m_gpu->DrawLine2D(v3f(ppos.x, ppos.y - 10.f, 0.f), v3f(ppos.x, ppos.y + 10.f, 0.f), ColorBlue);
	}
	
	if (m_inputContext->m_isLMBHold && g_isEditWalls)
	{
		auto cell = m_map->GetCell(m_gameCursorPosition);
		if (cell)
		{
			cell->m_flags |= cell->flag_wall;
		}
	}
	if (m_inputContext->m_isRMBHold && g_isEditWalls)
	{
		auto cell = m_map->GetCell(m_gameCursorPosition);
		if (cell)
		{
			if(cell->m_flags & cell->flag_wall)
				cell->m_flags ^= cell->flag_wall;
		}
	}
	if (m_inputContext->isKeyHold(yyKey::K_LALT) || g_isEditWalls)
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

	//if (g_isAddTestStruct)
	//{
	//	auto spr = m_structureTest->m_sprite;

	//	auto pos = m_map->GetSnapPosition(m_gameCursorPosition);
	//	//static v2f old_pos = pos;
	//	
	//	s32 CellIndexX = 0;
	//	s32 CellIndexY = 0;
	//	m_map->GetCellInds(CellIndexX, CellIndexY, pos);
	//	
	//	// определение выхода за пределы карты
	//	s32 cellsLeftX = (m_map->m_cellsX - CellIndexX);
	//	s32 cellsLeftY = (m_map->m_cellsY - CellIndexY);


	//	if (cellsLeftX <= m_structureTest->m_siteSizeX)
	//	{
	//		pos.x -= (m_structureTest->m_siteSizeX - cellsLeftX+1) * GAME_MAP_GRID_SIZE;
	//		// нужено будет делать повторно GetCellInds
	//	}
	//	if (cellsLeftY <= 0)
	//	{
	//		pos.y += (cellsLeftY-1) * GAME_MAP_GRID_SIZE;
	//	}
	//	else if (cellsLeftY > m_map->m_cellsY - m_structureTest->m_siteSizeY)
	//	{
	//		pos.y += (cellsLeftY - (m_map->m_cellsY - m_structureTest->m_siteSizeY)) * GAME_MAP_GRID_SIZE;
	//	}

	//	spr->m_objectBase.m_globalMatrix[3].x = pos.x;
	//	spr->m_objectBase.m_globalMatrix[3].y = pos.y;
	//	m_gpu->DrawSprite(spr);

	//	m_spriteGridBlue->m_objectBase.m_globalMatrix[3].x = pos.x;
	//	m_spriteGridBlue->m_objectBase.m_globalMatrix[3].y = pos.y;
	//	m_gpu->DrawSprite(m_spriteGridBlue);


	//	m_map->GetCellInds(CellIndexX, CellIndexY, pos);


	//	s32 cix = CellIndexX;
	//	s32 ciy = CellIndexY;
	//	bool good = true;
	//	for (u32 y = 0; y <= m_structureTest->m_siteSizeY; ++y)
	//	{
	//		for (u32 x = 0; x <= m_structureTest->m_siteSizeX; ++x)
	//		{
	//			if ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_structure)
	//				|| (m_map->m_cells[ciy][cix].m_flags & MapCell::flag_wall))
	//			{
	//				good = false;

	//				auto cellPos = m_map->GetCellPosition(cix, ciy);

	//				m_spriteGridRed->m_objectBase.m_globalMatrix[3].x = cellPos.x;
	//				m_spriteGridRed->m_objectBase.m_globalMatrix[3].y = cellPos.y;
	//				m_gpu->DrawSprite(m_spriteGridRed);
	//			}
	//			++cix;
	//		}
	//		--ciy;
	//		if (ciy < 0)
	//		{
	//			break;
	//		}
	//		cix = CellIndexX;
	//	}
	//	if (good)
	//	{
	//		if (m_inputContext->m_isLMBDown)
	//		{
	//			cix = CellIndexX;
	//			ciy = CellIndexY;

	//			GameStructureNode* newStruct = new GameStructureNode;
	//			newStruct->m_struct = m_structureTest;
	//			newStruct->m_position = pos;
	//			m_map->AddStructure(newStruct);

	//			for (u32 y = 0; y <= m_structureTest->m_siteSizeY; ++y)
	//			{
	//				for (u32 x = 0; x <= m_structureTest->m_siteSizeX; ++x)
	//				{
	//					m_map->m_cells[ciy][cix].m_flags |= MapCell::flag_structure;
	//					++cix;
	//				}
	//				--ciy;
	//				if (ciy < 0)
	//				{
	//					break;
	//				}
	//				cix = CellIndexX;
	//			}
	//		}
	//	}
	//}


	if (g_currentMapSprite)
	{
		auto spriteSize = GetSpriteSize(g_currentMapSprite->m_spritePtr);
		auto spriteSizeHalf = spriteSize * 0.5f;

		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + m_worldToScreenOffset.x,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + m_worldToScreenOffset.y, 0.f),
			ColorLime
		);
	}
}