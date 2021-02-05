#include "imgui.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>

#include "RTS.h"

int g_mapGenSizeX = 10;
int g_mapGenSizeY = 10;
s32 g_selectedListItemBGObject = -1;
MapSprite*			g_currentMapSprite = 0;

bool g_isNewMapWindow = false;

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

void EditorStep(f32 dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (g_currentMapSprite)
	{
		if (g_inputContex->isKeyHold(yyKey::K_LSHIFT))
		{
			g_currentMapSprite->m_spritePosition.x = g_gameCursorPosition.x + g_spriteCameraPosition->x - g_screenHalfSize.x;
			g_currentMapSprite->m_spritePosition.y = g_gameCursorPosition.y + g_spriteCameraPosition->y - g_screenHalfSize.y;
		}

		if (g_inputContex->m_isLMBDown)
		{
			g_currentMapSprite = 0;
		}

		if (g_inputContex->isKeyHold(yyKey::K_DELETE) && g_currentMapSprite)
		{
			auto find_result = g_map->m_bgObjects.find_by_value(g_currentMapSprite);
			if (find_result)
			{
				g_map->m_bgObjects.erase_node(find_result);
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
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("BGObjects"))
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
				if (ImGui::BeginChild("AddedBGObjects", ImVec2(ImGui::GetWindowContentRegionWidth(), 200), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove))
				{
					int n = 0;
					for (auto & sp : g_map->m_bgObjects)
					{
						if (ImGui::Selectable(sp.m_data->m_name.data(), g_selectedListItemBGObject == n))
						{
							g_selectedListItemBGObject = n;
							auto findSprite = g_map->m_bgObjects.find(n);
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

							newMapSprite->m_gui_text = yyGUICreateText(v2f(), g_defaultFont, wstr.data());


							newMapSprite->m_spritePtr = newSprite;
							v2i textureSize;
							yyGetTextureSize(newSprite->m_texture, &textureSize);
							newMapSprite->m_radius = v2f((f32)textureSize.x, (f32)textureSize.y).distance(v2f());

							g_map->m_bgObjects.push_back(newMapSprite);

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
			ImGui::SliderInt("X segments", &g_mapGenSizeX, 3, 100);
			ImGui::SliderInt("Y segments", &g_mapGenSizeY, 3, 100);
			if (ImGui::Button("Create"))
			{
				g_map->Generate();
				g_isNewMapWindow = false;
				g_cameraLimits.x = g_map->m_mapHalfSizeX;
				g_cameraLimits.y = g_map->m_mapHalfSizeY;
			}
			if (ImGui::Button("Close"))
			{
				g_isNewMapWindow = false;
			}
			ImGui::End();
		}
	}
	
	g_videoDriver->UseDepth(false);
	g_videoDriver->UseBlend(false);

	for (u16 i = 0, sz = g_map->m_bgSpritePositions.size(); i < sz; ++i)
	{
		auto & spritePos = g_map->m_bgSpritePositions[i];

		f32 dist = g_spriteCameraPosition->distance(spritePos);

		if ((dist - g_map->m_bgSpriteRadius) <= g_screenRectRadius)
		{
			g_map->m_bgSprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
			g_map->m_bgSprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
			g_videoDriver->DrawSprite(g_map->m_bgSprite);
		}

	}

	f32 offsetX = -g_spriteCameraPosition->x + g_screenHalfSize.x;
	f32 offsetY = -g_spriteCameraPosition->y + g_screenHalfSize.y;

	{
		for (auto & sp : g_map->m_bgObjects)
		{
			sp.m_data->m_gui_text->m_visible = false;

			auto sprite = sp.m_data->m_spritePtr;
			auto & spritePos = sp.m_data->m_spritePosition;
			f32 dist = g_spriteCameraPosition->distance(sp.m_data->m_spritePosition);
			if ((dist - sp.m_data->m_radius) <= g_screenRectRadius)
			{
				sprite->m_objectBase.m_globalMatrix[3].x = spritePos.x;
				sprite->m_objectBase.m_globalMatrix[3].y = spritePos.y;
				g_videoDriver->DrawSprite(sprite);

				if (g_inputContex->isKeyHold(yyKey::K_LCTRL))
				{
					sp.m_data->m_gui_text->m_offset.x = spritePos.x + offsetX;
					sp.m_data->m_gui_text->m_offset.y = spritePos.y + offsetY;
					sp.m_data->m_gui_text->m_visible = true;
				}
			}
		}

	}

	if (g_currentMapSprite)
	{
		auto spriteSize = GetSpriteSize(g_currentMapSprite->m_spritePtr);
		auto spriteSizeHalf = spriteSize * 0.5f;

		g_videoDriver->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		g_videoDriver->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		g_videoDriver->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y - spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
		g_videoDriver->DrawLine2D(
			v3f(g_currentMapSprite->m_spritePosition.x - spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			v3f(g_currentMapSprite->m_spritePosition.x + spriteSizeHalf.x + offsetX,
				g_currentMapSprite->m_spritePosition.y + spriteSizeHalf.y + offsetY, 0.f),
			ColorLime
		);
	}
}