#ifndef __RTS_STRUCT_BASE_
#define __RTS_STRUCT_BASE_

class GameStructureBase : public GameStructure
{
public:
	GameStructureBase();
	virtual ~GameStructureBase();

	virtual void Init();
	virtual void Draw();
	virtual void Update(f32 dt);
};

#endif