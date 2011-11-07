////////////////////////////////////////////////////////////////////////////////
// \file    Md2.hpp
// \author  Jonathan Dupuy
// \brief   C++ loader/player for MD2 Models. OpenGL commands are dropped. 
//          Textures must be loaded separately. Access to the data is read only.
//          Additionnal information
//          (from http:tfc.duke.free.fr/coding/md2-specs-en.html): 
//          - Maximum number of triangles: 4096
//          - Maximum number of vertices: 2048
//          - Maximum number of texture coordinates: 2048
//          - Maximum number of frames: 512
//          - Maximum number of skins: 32
//          - Number of precalculated normal vectors: 162
//
////////////////////////////////////////////////////////////////////////////////

#ifndef MD2_HPP
#define MD2_HPP

#include <string>       // std::string / std::exception
#include <stdint.h>     // integers


////////////////////////////////////////////////////////////////////////////////
// Exception
class Md2Exception : public std::exception
{
public:
	virtual ~Md2Exception()  throw()  {};
	const char* what() const throw()  {return mMessage.c_str();} 
protected:
	std::string mMessage;
};


////////////////////////////////////////////////////////////////////////////////
// Definition
class Md2
{
public:
	// Md2 vertex format for rendering
	class Vertex
	{
	public:
		float p[3];  // position
		float n[3];  // normal
		float st[2]; // texture coordinates
	};
	// Md2 Skin
	class Skin
	{
	public:
		char name[64]; // name of the skin
	};
	enum AnimationName // Md2 animation
	{
		ANIMATION_STAND = 0,
		ANIMATION_RUN,
		ANIMATION_ATTACK,
		ANIMATION_PAIN_A,
		ANIMATION_PAIN_B,
		ANIMATION_PAIN_C,
		ANIMATION_JUMP,
		ANIMATION_FLIP,
		ANIMATION_SALUTE,
		ANIMATION_FALLBACK,
		ANIMATION_WAVE,
		ANIMATION_POINT,
		ANIMATION_CROUCH_STAND,
		ANIMATION_CROUCH_WALK,
		ANIMATION_CROUCH_ATTACK,
		ANIMATION_CROUCH_PAIN,
		ANIMATION_CROUCH_DEATH,
		ANIMATION_DEATH_FALLBACK,
		ANIMATION_DEATH_FALLFORWARD,
		ANIMATION_FALL_BACKSLOW,
		ANIMATION_BOOM // max
	};

	// Construtors / Destructors
	Md2();
	explicit Md2(const std::string& name) throw(Md2Exception);
	~Md2() throw();

	// Loading
	void Load(const std::string& filename) throw(Md2Exception);

	// Animation manipulation
	void Play();                // play current animation
	void Pause();               // pause current animation
	void NextAnimation();       // play next animation
	void PreviousAnimation();   // play previous animation
	void Update(float dt);      // update the animation sequence

	// Stream data
	void GenVertices(Vertex* vertices)        const; // allocated memory

	// Queries
	const int16_t VertexCount()    const;
	const int16_t SkinCount()      const;
	const int16_t TexCoordCount()  const;
	const int16_t TriangleCount()  const;
	const int16_t FrameCount()     const;
	const int16_t SkinWidth()      const;
	const int16_t SkinHeight()     const;
	bool IsPlaying()               const;

private:
	// Non copyable
	Md2(const Md2& md2);
	Md2& operator=(const Md2& md2);

	// Internal manipulation
	void _Clear();

	// Internal types declaration (defined in Md2.cpp)
	class _TexCoord;
	class _Triangle;
	class _Frame;
	class _Normal;
	class _Animation;

	// Members
	static const _Normal     sNormals[162];    // normal table
	static const _Animation  sAnimations[21];  // animation table
	Skin*        mSkins;                  // skin array
	_TexCoord*   mTexCoords;              // texture coords array
	_Triangle*   mTriangles;              // triangles array
	_Frame*      mFrames;                 // frames array
#if __WORDSIZE==32
	char _reserved[16];
#endif
	int16_t mSkinCnt;         // number of skins
	int16_t mTexCoordCnt;     // number of texcoords
	int16_t mTriangleCnt;     // number of triangles
	int16_t mFrameCnt;        // number of frames
	int16_t mVertexCnt;       // number of vertices
	int16_t mSkinWidth;       // width of the original skin
	int16_t mSkinHeight;      // width of the original skin

	int16_t mActiveAnimation;  // active animation index
	float   mActiveFrame;      // active frame index
	float   mSpeed;            // speed of the animation
	bool    mIsPlaying;        // playing flag
};

#endif

