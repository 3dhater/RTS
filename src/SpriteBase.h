#ifndef __RTS_BASE_SPRITE_
#define __RTS_BASE_SPRITE_

class GameSpriteBase
{
public:
	GameSpriteBase() {}
	virtual ~GameSpriteBase() {}

	virtual void Draw() = 0;
	virtual void Update(f32 dt) = 0;

	virtual void SetPosition(const v2f& pos) { m_position = pos; }
	
	// Позиция объекта - левый нижний угол спрайта
	// Думаю правильнее это должно быть его центром, но пока
	//  левый нижний угол проще использовать при установке на карту и т.д.
	// Надо будет переделать.
	// Но пока будет так. И тогда надо будет добавить дополнительное значение
	//  для нахождения центра - m_centerOffset
	v2f m_position;
	v2f m_centerOffset;
};

#endif