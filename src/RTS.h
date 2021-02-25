#ifndef __RTS_H_
#define __RTS_H_

#include "yy.h"
#include "yy_window.h"

#include "yy_ptr.h"

#include "yy_async.h"
#include "yy_resource.h"
#include "yy_material.h"

#include "strings/string.h"

#include <cstdio>
#include <vector>
#include <string>

#include "yy_fs.h"
#include "yy_image.h"
#include "yy_model.h"
#include "yy_gui.h"
#include "yy_input.h"

#include "scene/common.h"
#include "scene/camera.h"
#include "scene/sprite.h"


v2f GetSpriteSize(yySprite* sprite);
yySprite* GetSprite(const char* file, u8 spritePosition = 0);

#include "Map.h"

struct SpriteCacheNode 
{
	yyStringA m_path;
	yySprite* m_sprite;
};

struct SpriteCache
{
	SpriteCache() {}
	~SpriteCache()
	{
		for (u16 i = 0, sz = m_cache.size(); i < sz; ++i)
		{
			yyDestroy(m_cache[i].m_sprite);
		}
	}
	yyArraySmall<SpriteCacheNode> m_cache;
};

class GameCursor {
public:
	GameCursor() {
		m_sprite = 0;
	}
	~GameCursor() {
		if (m_sprite) yyDestroy(m_sprite);
	}
	void init(const char* file, v2f size, v2i lt, v2i rb) {
		yyResource* texture = yyGetTextureResource(file, false, false, true);
		m_sprite = yyCreateSprite(v4f(0.f, 0.f, size.x, size.y), texture, false);
		m_sprite->SetMainFrame(lt.x, lt.y, rb.x, rb.y);
	}

	yySprite* m_sprite;
};


class GameFaction
{
public:
	GameFaction()
	{
		m_color = ColorWhite;
	}
	~GameFaction()
	{

	}

	yyColor m_color;
};

#endif