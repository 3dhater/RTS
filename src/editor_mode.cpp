#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"
#include "Game.h"


s32 g_selectedListItemBGObject = -1;
MapSprite*			g_currentMapSprite = 0;

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
			g_currentMapSprite->m_spritePosition.x = m_gameCursorPosition.x + m_spriteCameraPosition->x - m_screenHalfSize.x;
			g_currentMapSprite->m_spritePosition.y = m_gameCursorPosition.y + m_spriteCameraPosition->y - m_screenHalfSize.y;
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
						auto newSprite = GetSprite(stra.data());
						if (newSprite)
						{
							static u32 nameCounter = 0;
							MapSprite* newMapSprite = new MapSprite;
							g_currentMapSprite = newMapSprite;
							yyFS::path fp = stra.data();
							auto fileName = fp.filename();
							util::stringPopBackBefore(fileName.string_type, '.');
							fileName.string_type.pop_back();
							newMapSprite->m_name = fileName.string_type.data();
							newMapSprite->m_name += "_";
							newMapSprite->m_name += nameCounter++;

							yyStringW wstr;
							wstr += newMapSprite->m_name.data();

							newMapSprite->m_gui_text = yyGUICreateText(v2f(), m_defaultFont, wstr.data());


							newMapSprite->m_spritePtr = newSprite;
							v2i textureSize;
							yyGetTextureSize(newSprite->m_texture, &textureSize);
							newMapSprite->m_radius = v2f((f32)textureSize.x, (f32)textureSize.y).distance(v2f());

							m_map->m_bgObjects.push_back(newMapSprite);

						}
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
				m_cameraLimits.x = m_map->m_mapSizeX;
				m_cameraLimits.y = m_map->m_mapSizeY;
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

	for (u16 i = 0, sz = m_map->m_bgSpritePositions.size(); i < sz; ++i)
	{
		auto & spritePos = m_map->m_bgSpritePositions[i];

		f32 dist = m_spriteCameraPosition->distance(spritePos);

		if ((dist - m_map->m_bgSpriteRadius) <= m_screenRectRadius)
		{
			m_map->m_bgSprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
			m_map->m_bgSprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
			m_gpu->DrawSprite(m_map->m_bgSprite);
		}

	}

	f32 offsetX = -m_spriteCameraPosition->x + m_screenHalfSize.x;
	f32 offsetY = -m_spriteCameraPosition->y + m_screenHalfSize.y;

	{
		for (auto & sp : m_map->m_bgObjects)
		{
			sp.m_data->m_gui_text->SetVisible(false);

			auto sprite = sp.m_data->m_spritePtr;
			auto & spritePos = sp.m_data->m_spritePosition;
			f32 dist = m_spriteCameraPosition->distance(sp.m_data->m_spritePosition);
			if ((dist - sp.m_data->m_radius) <= m_screenRectRadius)
			{
				sprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
				sprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
				m_gpu->DrawSprite(sprite);

				if (m_inputContext->isKeyHold(yyKey::K_LCTRL))
				{
					sp.m_data->m_gui_text->m_offset.x = spritePos.x + offsetX;
					sp.m_data->m_gui_text->m_offset.y = spritePos.y + offsetY;
					sp.m_data->m_gui_text->SetVisible(true);
				}
			}
		}

		m_map->SortRenderSprites();
		for (u16 i = 0; i < m_map->m_renderSprites.m_size; ++i)
		{
			auto & rn = m_map->m_renderSprites.m_data[i];
			rn.m_sprite->m_objectBase.m_globalMatrix[3].x = rn.m_position.x;
			rn.m_sprite->m_objectBase.m_globalMatrix[3].y = rn.m_position.y;
			m_gpu->DrawSprite(rn.m_sprite);
		}
	}
	
	if (m_inputContext->m_isLMBHold && g_isEditWalls)
	{
		auto cell = m_map->GetCellUnderCursor(m_gameCursorPosition);
		if (cell)
		{
			cell->m_flags |= cell->flag_wall;
		}
	}
	if (m_inputContext->m_isRMBHold && g_isEditWalls)
	{
		auto cell = m_map->GetCellUnderCursor(m_gameCursorPosition);
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

	if (g_isAddTestStruct)
	{
		auto spr = m_structureTest->m_sprite;

		auto pos = m_map->GetSnapPosition(m_gameCursorPosition);
		//static v2f old_pos = pos;
		
		s32 CellIndexX = 0;
		s32 CellIndexY = 0;
		m_map->GetCellInds(CellIndexX, CellIndexY, pos);
		
		// определение выхода за пределы карты
		s32 cellsLeftX = (m_map->m_cellsX - CellIndexX);
		s32 cellsLeftY = (m_map->m_cellsY - CellIndexY);


		if (cellsLeftX <= m_structureTest->m_siteSizeX)
		{
			pos.x -= (m_structureTest->m_siteSizeX - cellsLeftX+1) * GAME_MAP_GRID_SIZE;
			// нужено будет делать повторно GetCellInds
		}
		if (cellsLeftY <= 0)
		{
			pos.y += (cellsLeftY-1) * GAME_MAP_GRID_SIZE;
		}
		else if (cellsLeftY > m_map->m_cellsY - m_structureTest->m_siteSizeY)
		{
			pos.y += (cellsLeftY - (m_map->m_cellsY - m_structureTest->m_siteSizeY)) * GAME_MAP_GRID_SIZE;
		}

		spr->m_objectBase.m_globalMatrix[3].x = pos.x;
		spr->m_objectBase.m_globalMatrix[3].y = pos.y;
		m_gpu->DrawSprite(spr);

		m_spriteGridBlue->m_objectBase.m_globalMatrix[3].x = pos.x;
		m_spriteGridBlue->m_objectBase.m_globalMatrix[3].y = pos.y;
		m_gpu->DrawSprite(m_spriteGridBlue);


		m_map->GetCellInds(CellIndexX, CellIndexY, pos);


		s32 cix = CellIndexX;
		s32 ciy = CellIndexY;
		bool good = true;
		for (u32 y = 0; y <= m_structureTest->m_siteSizeY; ++y)
		{
			for (u32 x = 0; x <= m_structureTest->m_siteSizeX; ++x)
			{
				if ((m_map->m_cells[ciy][cix].m_flags & MapCell::flag_structure)
					|| (m_map->m_cells[ciy][cix].m_flags & MapCell::flag_wall))
				{
					good = false;

					auto cellPos = m_map->GetCellPosition(cix, ciy);

					m_spriteGridRed->m_objectBase.m_globalMatrix[3].x = cellPos.x;
					m_spriteGridRed->m_objectBase.m_globalMatrix[3].y = cellPos.y;
					m_gpu->DrawSprite(m_spriteGridRed);
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
			if (m_inputContext->m_isLMBDown)
			{
				cix = CellIndexX;
				ciy = CellIndexY;

				GameStructureNode* newStruct = new GameStructureNode;
				newStruct->m_struct = m_structureTest;
				newStruct->m_position = pos;
				m_map->AddStructure(newStruct);

				for (u32 y = 0; y <= m_structureTest->m_siteSizeY; ++y)
				{
					for (u32 x = 0; x <= m_structureTest->m_siteSizeX; ++x)
					{
						m_map->m_cells[ciy][cix].m_flags |= MapCell::flag_structure;
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
	}


	if (g_currentMapSprite)
	{
		auto spriteSize = GetSpriteSize(g_currentMapSprite->m_spritePtr);
		auto spriteSizeHalf = spriteSize * 0.5f;

		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		m_gpu->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
	}
}