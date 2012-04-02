#include "Md2.hpp"
#include <fstream> // std::ifstream
#include <cmath>   // modf

////////////////////////////////////////////////////////////////////////////////
// Internal impl
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Md2::_TexCoord
class Md2::_TexCoord
{
public:
	// Members
	int16_t s, t;    // 2d coordinates
};


////////////////////////////////////////////////////////////////////////////////
// Md2::_Triangle
class Md2::_Triangle
{
public:
	// Members
	uint16_t iPos[3];  // vertex index
	uint16_t iSt[3];   // texcoord index
};


////////////////////////////////////////////////////////////////////////////////
// Md2::_Normal
class Md2::_Normal
{
public:
	// Members
	float x, y, z;    // 3d float vector
};


////////////////////////////////////////////////////////////////////////////////
// Md2::_Frame
class Md2::_Frame
{
public:
	// Internal type
	class Vertex
	{
	public:
		uint8_t x,y,z;       // position of the vertex
		uint8_t n;     // index of the normal
	};

	// Constructors / Destructor
	~_Frame()
	{
		delete[] vertices;
	}

	// Members
	Vertex* vertices;   // vertices of the frame
#if __WORDSIZE==32
private:
	char _reserved[4];
public:
#endif
	float scale[3];         // scale {x,y,z}
	float translation[3];   // translate {x,y,z}
	char name[16];          // name of the frame
};


////////////////////////////////////////////////////////////////////////////////
// Md2::_Animation
class Md2::_Animation
{
public:
	int16_t start;   // first frame index
	int16_t end;     // last frame index
	float fps;        // speed
	int16_t FrameCount() const {return end-start+1;}
};

////////////////////////////////////////////////////////////////////////////////
// File header of an md2 model
class _Md2Header
{
public:
	int32_t ident;           // magic number. must be equal to "IDP2"
	int32_t version;         // md2 version. must be equal to 8

	int32_t skinWidth;       // width of the texture
	int32_t skinHeight;      // height of the texture
	int32_t frameSize;       // size of one frame in bytes

	int32_t skinCnt;         // number of textures
	int32_t vertexCnt;       // number of vertices
	int32_t texCoordCnt;     // number of texture coordinates
	int32_t triangleCnt;     // number of triangles
	int32_t glcmdCnt;        // number of opengl commands
	int32_t frameCnt;        // total number of frames

	int32_t skinOffset;      // offset to skin names (64 bytes each)
	int32_t texCoordOffset;  // offset to s-t texture coordinates
	int32_t triangleOffset;  // offset to triangles
	int32_t frameOffset;     // offset to frame data
	int32_t glcmdOffset;     // offset to opengl commands
	int32_t endOffset;       // offset to end of file
};

////////////////////////////////////////////////////////////////////////////////
// Exceptions
// Unable to read file exception
class _FileNotFoundException : public Md2Exception
{
public:
	explicit _FileNotFoundException(const std::string& filename)
	{
		mMessage = "The file " 
		         + filename 
		         + " was not found.";
	}
};

// Bad ident exception
class _BadIdentException : public Md2Exception
{
public:
	explicit _BadIdentException(const std::string& filename)
	{
		mMessage = "The file "
		         + filename
		         + " has an invalid MD2 ident.";
	}
};

// Bad version exception
class _BadVersionException : public Md2Exception
{
public:
	explicit _BadVersionException(const std::string& filename)
	{
		mMessage = "The file "
		         + filename
		         + " has an invalid MD2 version.";
	}
};

// Bad triangle exception
class _BadTriangleDataException : public Md2Exception
{
public:
	explicit _BadTriangleDataException(const std::string& filename)
	{
		mMessage = "The file "
		         + filename
		         + " has invalid MD2 triangle data.";
	}
};

// Bad frame exception
class _BadFrameDataException : public Md2Exception
{
public:
	explicit _BadFrameDataException(const std::string& filename)
	{
		mMessage = "The file "
		         + filename
		         + " has invalid MD2 frame data.";
	}
};

// Bad vertex exception
class _BadVertexDataException : public Md2Exception
{
public:
	explicit _BadVertexDataException(const std::string& filename)
	{
		mMessage = "The file "
		         + filename
		         + " has invalid MD2 vertex data.";
	}
};


////////////////////////////////////////////////////////////////////////////////
// Normal table
const Md2::_Normal Md2::sNormals[162] =
{
	{ -0.525731f,  0.000000f,  0.850651f }, { -0.442863f,  0.238856f,  0.864188f },
	{ -0.295242f,  0.000000f,  0.955423f }, { -0.309017f,  0.500000f,  0.809017f },
	{ -0.162460f,  0.262866f,  0.951056f }, {  0.000000f,  0.000000f,  1.000000f },
	{  0.000000f,  0.850651f,  0.525731f }, { -0.147621f,  0.716567f,  0.681718f },
	{  0.147621f,  0.716567f,  0.681718f }, {  0.000000f,  0.525731f,  0.850651f },
	{  0.309017f,  0.500000f,  0.809017f }, {  0.525731f,  0.000000f,  0.850651f },
	{  0.295242f,  0.000000f,  0.955423f }, {  0.442863f,  0.238856f,  0.864188f },
	{  0.162460f,  0.262866f,  0.951056f }, { -0.681718f,  0.147621f,  0.716567f },
	{ -0.809017f,  0.309017f,  0.500000f }, { -0.587785f,  0.425325f,  0.688191f },
	{ -0.850651f,  0.525731f,  0.000000f }, { -0.864188f,  0.442863f,  0.238856f },
	{ -0.716567f,  0.681718f,  0.147621f }, { -0.688191f,  0.587785f,  0.425325f },
	{ -0.500000f,  0.809017f,  0.309017f }, { -0.238856f,  0.864188f,  0.442863f },
	{ -0.425325f,  0.688191f,  0.587785f }, { -0.716567f,  0.681718f, -0.147621f },
	{ -0.500000f,  0.809017f, -0.309017f }, { -0.525731f,  0.850651f,  0.000000f },
	{  0.000000f,  0.850651f, -0.525731f }, { -0.238856f,  0.864188f, -0.442863f },
	{  0.000000f,  0.955423f, -0.295242f }, { -0.262866f,  0.951056f, -0.162460f },
	{  0.000000f,  1.000000f,  0.000000f }, {  0.000000f,  0.955423f,  0.295242f },
	{ -0.262866f,  0.951056f,  0.162460f }, {  0.238856f,  0.864188f,  0.442863f },
	{  0.262866f,  0.951056f,  0.162460f }, {  0.500000f,  0.809017f,  0.309017f },
	{  0.238856f,  0.864188f, -0.442863f }, {  0.262866f,  0.951056f, -0.162460f },
	{  0.500000f,  0.809017f, -0.309017f }, {  0.850651f,  0.525731f,  0.000000f },
	{  0.716567f,  0.681718f,  0.147621f }, {  0.716567f,  0.681718f, -0.147621f },
	{  0.525731f,  0.850651f,  0.000000f }, {  0.425325f,  0.688191f,  0.587785f },
	{  0.864188f,  0.442863f,  0.238856f }, {  0.688191f,  0.587785f,  0.425325f },
	{  0.809017f,  0.309017f,  0.500000f }, {  0.681718f,  0.147621f,  0.716567f },
	{  0.587785f,  0.425325f,  0.688191f }, {  0.955423f,  0.295242f,  0.000000f },
	{  1.000000f,  0.000000f,  0.000000f }, {  0.951056f,  0.162460f,  0.262866f },
	{  0.850651f, -0.525731f,  0.000000f }, {  0.955423f, -0.295242f,  0.000000f },
	{  0.864188f, -0.442863f,  0.238856f }, {  0.951056f, -0.162460f,  0.262866f },
	{  0.809017f, -0.309017f,  0.500000f }, {  0.681718f, -0.147621f,  0.716567f },
	{  0.850651f,  0.000000f,  0.525731f }, {  0.864188f,  0.442863f, -0.238856f },
	{  0.809017f,  0.309017f, -0.500000f }, {  0.951056f,  0.162460f, -0.262866f },
	{  0.525731f,  0.000000f, -0.850651f }, {  0.681718f,  0.147621f, -0.716567f },
	{  0.681718f, -0.147621f, -0.716567f }, {  0.850651f,  0.000000f, -0.525731f },
	{  0.809017f, -0.309017f, -0.500000f }, {  0.864188f, -0.442863f, -0.238856f },
	{  0.951056f, -0.162460f, -0.262866f }, {  0.147621f,  0.716567f, -0.681718f },
	{  0.309017f,  0.500000f, -0.809017f }, {  0.425325f,  0.688191f, -0.587785f },
	{  0.442863f,  0.238856f, -0.864188f }, {  0.587785f,  0.425325f, -0.688191f },
	{  0.688191f,  0.587785f, -0.425325f }, { -0.147621f,  0.716567f, -0.681718f },
	{ -0.309017f,  0.500000f, -0.809017f }, {  0.000000f,  0.525731f, -0.850651f },
	{ -0.525731f,  0.000000f, -0.850651f }, { -0.442863f,  0.238856f, -0.864188f },
	{ -0.295242f,  0.000000f, -0.955423f }, { -0.162460f,  0.262866f, -0.951056f },
	{  0.000000f,  0.000000f, -1.000000f }, {  0.295242f,  0.000000f, -0.955423f },
	{  0.162460f,  0.262866f, -0.951056f }, { -0.442863f, -0.238856f, -0.864188f },
	{ -0.309017f, -0.500000f, -0.809017f }, { -0.162460f, -0.262866f, -0.951056f },
	{  0.000000f, -0.850651f, -0.525731f }, { -0.147621f, -0.716567f, -0.681718f },
	{  0.147621f, -0.716567f, -0.681718f }, {  0.000000f, -0.525731f, -0.850651f },
	{  0.309017f, -0.500000f, -0.809017f }, {  0.442863f, -0.238856f, -0.864188f },
	{  0.162460f, -0.262866f, -0.951056f }, {  0.238856f, -0.864188f, -0.442863f },
	{  0.500000f, -0.809017f, -0.309017f }, {  0.425325f, -0.688191f, -0.587785f },
	{  0.716567f, -0.681718f, -0.147621f }, {  0.688191f, -0.587785f, -0.425325f },
	{  0.587785f, -0.425325f, -0.688191f }, {  0.000000f, -0.955423f, -0.295242f },
	{  0.000000f, -1.000000f,  0.000000f }, {  0.262866f, -0.951056f, -0.162460f },
	{  0.000000f, -0.850651f,  0.525731f }, {  0.000000f, -0.955423f,  0.295242f },
	{  0.238856f, -0.864188f,  0.442863f }, {  0.262866f, -0.951056f,  0.162460f },
	{  0.500000f, -0.809017f,  0.309017f }, {  0.716567f, -0.681718f,  0.147621f },
	{  0.525731f, -0.850651f,  0.000000f }, { -0.238856f, -0.864188f, -0.442863f },
	{ -0.500000f, -0.809017f, -0.309017f }, { -0.262866f, -0.951056f, -0.162460f },
	{ -0.850651f, -0.525731f,  0.000000f }, { -0.716567f, -0.681718f, -0.147621f },
	{ -0.716567f, -0.681718f,  0.147621f }, { -0.525731f, -0.850651f,  0.000000f },
	{ -0.500000f, -0.809017f,  0.309017f }, { -0.238856f, -0.864188f,  0.442863f },
	{ -0.262866f, -0.951056f,  0.162460f }, { -0.864188f, -0.442863f,  0.238856f },
	{ -0.809017f, -0.309017f,  0.500000f }, { -0.688191f, -0.587785f,  0.425325f },
	{ -0.681718f, -0.147621f,  0.716567f }, { -0.442863f, -0.238856f,  0.864188f },
	{ -0.587785f, -0.425325f,  0.688191f }, { -0.309017f, -0.500000f,  0.809017f },
	{ -0.147621f, -0.716567f,  0.681718f }, { -0.425325f, -0.688191f,  0.587785f },
	{ -0.162460f, -0.262866f,  0.951056f }, {  0.442863f, -0.238856f,  0.864188f },
	{  0.162460f, -0.262866f,  0.951056f }, {  0.309017f, -0.500000f,  0.809017f },
	{  0.147621f, -0.716567f,  0.681718f }, {  0.000000f, -0.525731f,  0.850651f },
	{  0.425325f, -0.688191f,  0.587785f }, {  0.587785f, -0.425325f,  0.688191f },
	{  0.688191f, -0.587785f,  0.425325f }, { -0.955423f,  0.295242f,  0.000000f },
	{ -0.951056f,  0.162460f,  0.262866f }, { -1.000000f,  0.000000f,  0.000000f },
	{ -0.850651f,  0.000000f,  0.525731f }, { -0.955423f, -0.295242f,  0.000000f },
	{ -0.951056f, -0.162460f,  0.262866f }, { -0.864188f,  0.442863f, -0.238856f },
	{ -0.951056f,  0.162460f, -0.262866f }, { -0.809017f,  0.309017f, -0.500000f },
	{ -0.864188f, -0.442863f, -0.238856f }, { -0.951056f, -0.162460f, -0.262866f },
	{ -0.809017f, -0.309017f, -0.500000f }, { -0.681718f,  0.147621f, -0.716567f },
	{ -0.681718f, -0.147621f, -0.716567f }, { -0.850651f,  0.000000f, -0.525731f },
	{ -0.688191f,  0.587785f, -0.425325f }, { -0.587785f,  0.425325f, -0.688191f },
	{ -0.425325f,  0.688191f, -0.587785f }, { -0.425325f, -0.688191f, -0.587785f },
	{ -0.587785f, -0.425325f, -0.688191f }, { -0.688191f, -0.587785f, -0.425325f }
};


////////////////////////////////////////////////////////////////////////////////
// Animation table
const Md2::_Animation Md2::sAnimations[21] = 
{
	// first, last, fps
	{   0,  39,  9.f },   // STAND
	{  40,  45, 10.f },   // RUN
	{  46,  53, 10.f },   // ATTACK
	{  54,  57,  7.f },   // PAIN_A
	{  58,  61,  7.f },   // PAIN_B
	{  62,  65,  7.f },   // PAIN_C
	{  66,  71,  7.f },   // JUMP
	{  72,  83,  7.f },   // FLIP
	{  84,  94,  7.f },   // SALUTE
	{  95, 111, 10.f },   // FALLBACK
	{ 112, 122,  7.f },   // WAVE
	{ 123, 134,  6.f },   // POINT
	{ 135, 153, 10.f },   // CROUCH_STAND
	{ 154, 159,  7.f },   // CROUCH_WALK
	{ 160, 168, 10.f },   // CROUCH_ATTACK
	{ 169, 172,  7.f },   // CROUCH_PAIN
	{ 173, 177,  5.f },   // CROUCH_DEATH
	{ 178, 183,  7.f },   // DEATH_FALLBACK
	{ 184, 189,  7.f },   // DEATH_FALLFORWARD
	{ 190, 197,  7.f },   // DEATH_FALLBACKSLOW
	{ 198, 198,  5.f }    // BOOM
};


////////////////////////////////////////////////////////////////////////////////
// Default constructor
Md2::Md2():
	mSkins(NULL), mTexCoords(NULL), mTriangles(NULL), mFrames(NULL), 
	mSkinCnt(-1), mTexCoordCnt(-1), mTriangleCnt(-1), mFrameCnt(-1), 
	mVertexCnt(-1),
	mSkinWidth(-1), mSkinHeight(-1),
	mActiveAnimation(0), mActiveFrame(0.0f),
	mSpeed(1.0f), mIsPlaying(true)
{

}


////////////////////////////////////////////////////////////////////////////////
// Overloaded constructor
Md2::Md2(const std::string& filename) throw(Md2Exception): 
	mSkins(NULL), mTexCoords(NULL), mTriangles(NULL), mFrames(NULL), 
	mSkinCnt(-1), mTexCoordCnt(-1), mTriangleCnt(-1), mFrameCnt(-1), 
	mVertexCnt(-1),
	mSkinWidth(-1), mSkinHeight(-1),
	mActiveAnimation(0), mActiveFrame(0.0f),
	mSpeed(1.0f), mIsPlaying(true)
{
	// pass construction to LoadFromFile
	Load(filename);
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
Md2::~Md2() throw()
{
	// clean data
	_Clear();
}


////////////////////////////////////////////////////////////////////////////////
// Load from file
void Md2::Load(const std::string& filename) throw (Md2Exception)
{
	// Clean if necessary
	_Clear();

	// open file
	std::ifstream fileStream( filename.c_str(), 
	                          std::ifstream::binary|std::ifstream::in );
	if(!fileStream)
		throw _FileNotFoundException(filename);

	// read header
	_Md2Header header;
	fileStream.read( reinterpret_cast<char*>(&header), 
	                 sizeof(_Md2Header));

	// check version and ident
	if(header.ident != ('I'|'D'<< 8|'P'<<16|'2'<< 24))
		throw _BadIdentException(filename);
	if(header.version!=8)
		throw _BadVersionException(filename);

	// save information
	mSkinCnt      = header.skinCnt;
	mTexCoordCnt  = header.texCoordCnt;
	mTriangleCnt  = header.triangleCnt;
	mFrameCnt     = header.frameCnt;
	mVertexCnt    = header.vertexCnt;
	mSkinWidth    = header.skinWidth;
	mSkinHeight   = header.skinHeight;

	// allocate and read in data
	if(0<mSkinCnt)
	{
		mSkins = new Skin[mSkinCnt];
		// read skin data
		fileStream.seekg( header.skinOffset, std::fstream::beg);
		fileStream.read(  reinterpret_cast<char*>(mSkins),
		                  sizeof(Md2::Skin)*mSkinCnt);
	}
	if(0<mTexCoordCnt)
	{
		mTexCoords = new _TexCoord[mTexCoordCnt];
		// read texcoord data
		fileStream.seekg( header.texCoordOffset, std::fstream::beg);
		fileStream.read(  reinterpret_cast<char*>(mTexCoords),
		                  sizeof(Md2::_TexCoord)*mTexCoordCnt);
	}
	if(0<mTriangleCnt)
	{
		mTriangles = new _Triangle[mTriangleCnt];
		// read triangles
		fileStream.seekg( header.triangleOffset, std::fstream::beg);
		fileStream.read(  reinterpret_cast<char*>(mTriangles),
		                  sizeof(Md2::_Triangle)*mTriangleCnt);
	}
	else
		throw _BadTriangleDataException(filename);
	if(198==mFrameCnt)
		mFrames = new _Frame[mFrameCnt];
	else
		throw _BadFrameDataException(filename);
	if(0<mVertexCnt)
		for(int32_t i=0; i<mFrameCnt; ++i)
			mFrames[i].vertices = new _Frame::Vertex[mVertexCnt];
	else
		throw _BadVertexDataException(filename);

	// read frames
	fileStream.seekg(header.frameOffset, std::ifstream::beg);
	for(int32_t i=0; i<mFrameCnt;++i)
	{
		// load frame
		fileStream.read(   reinterpret_cast<char*>(&mFrames[i].scale[0]),
		                    sizeof(float)*3);
		fileStream.read(   reinterpret_cast<char*>(&mFrames[i].translation[0]),
		                    sizeof(float)*3);
		fileStream.read(   reinterpret_cast<char*>(&mFrames[i].name[0]),
		                    sizeof(char)*16);
		fileStream.read(   reinterpret_cast<char*>(mFrames[i].vertices),
		                    sizeof(Md2::_Frame::Vertex)*mVertexCnt);
	}

	// done
	fileStream.close();
}


////////////////////////////////////////////////////////////////////////////////
// Play
void Md2::Play()
{
	mIsPlaying = true;
}


////////////////////////////////////////////////////////////////////////////////
// Play
void Md2::Pause()
{
	mIsPlaying = false;
}


////////////////////////////////////////////////////////////////////////////////
// Play next animation
void Md2::NextAnimation()
{
	++mActiveAnimation;
	if(mActiveAnimation >= ANIMATION_BOOM)
		mActiveAnimation = ANIMATION_STAND;
	mActiveFrame = sAnimations[mActiveAnimation].start;
}


////////////////////////////////////////////////////////////////////////////////
// Play next animation
void Md2::PreviousAnimation()
{
	--mActiveAnimation;
	if(mActiveAnimation == -1)
		mActiveAnimation += ANIMATION_BOOM;
	mActiveFrame = sAnimations[mActiveAnimation].start;
}

#include <iostream>
////////////////////////////////////////////////////////////////////////////////
// Update
void Md2::Update(float dt)
{
	// ignore if paused
	if(!mIsPlaying)
		return;

	// increment frame
	mActiveFrame+= mSpeed*dt*sAnimations[mActiveAnimation].fps;

	// loop animation
	if(mActiveFrame >= sAnimations[mActiveAnimation].end)
		mActiveFrame = std::modf(mActiveFrame, &mActiveFrame)
		             + std::fmod(mActiveFrame-sAnimations[mActiveAnimation].start,
		                         sAnimations[mActiveAnimation].FrameCount())
		             + sAnimations[mActiveAnimation].start;
}


////////////////////////////////////////////////////////////////////////////////
// Get Vertices
void Md2::GenVertices(Md2::Vertex* vertices) const
{
	// variables
	Md2::_Frame *frameA, *frameB;
	Md2::_Frame::Vertex *vertA, *vertB;
	Md2::_Normal *normA, *normB;
	Md2::_TexCoord *texCoord;
	uint16_t nextFrame, index;
	float posA[3], posB[3];
	float lerp         = std::modf(mActiveFrame, &posA[0]); // &posA[0] is hack!
	float oneMinusLerp = 1.0f - lerp; 
	int16_t activeFrameIdx = static_cast<int16_t>(mActiveFrame);

	// compute next frame index
	nextFrame = activeFrameIdx == sAnimations[mActiveAnimation].end 
	            ? sAnimations[mActiveAnimation].start 
	            : activeFrameIdx+1;

	// Uncompress the vertices
	for(uint16_t i=0; i<mTriangleCnt; ++i)
		for(uint16_t j=0; j<3; ++j)
		{
			// set pointers
			frameA   = &mFrames[activeFrameIdx];
			frameB   = &mFrames[nextFrame];
			vertA    = &frameA->vertices[mTriangles[i].iPos[j]];
			vertB    = &frameB->vertices[mTriangles[i].iPos[j]];
			normA    = const_cast<Md2::_Normal*>(&sNormals[vertA->n]);
			normB    = const_cast<Md2::_Normal*>(&sNormals[vertB->n]);
			texCoord = &mTexCoords[mTriangles[i].iSt[j]];
			// set index
			index = i*3+j;

			// Uncompress vertex position
			posA[0] = frameA->scale[0] * vertA->x + frameA->translation[0];
			posA[1] = frameA->scale[1] * vertA->y + frameA->translation[1];
			posA[2] = frameA->scale[2] * vertA->z + frameA->translation[2];

			posB[0] = frameB->scale[0] * vertB->x + frameB->translation[0];
			posB[1] = frameB->scale[1] * vertB->y + frameB->translation[1];
			posB[2] = frameB->scale[2] * vertB->z + frameB->translation[2];

			// Interpolate vertex position
			vertices[index].p[0] = oneMinusLerp * posA[0] 
			                     + lerp * posB[0];
			vertices[index].p[1] = oneMinusLerp * posA[1]
			                     + lerp * posB[1];
			vertices[index].p[2] = oneMinusLerp * posA[2] 
			                     + lerp * posB[2];
			
			// compute normal
			vertices[index].n[0] = oneMinusLerp * normA->x 
			                     + lerp * normB->x;
			vertices[index].n[1] = oneMinusLerp * normA->y 
			                     + lerp * normB->y;
			vertices[index].n[2] = oneMinusLerp * normA->z 
			                     + lerp * normB->z;

			// compute texture coords
			vertices[index].st[0] = float(texCoord->s) / mSkinWidth;
			vertices[index].st[1] = 1.0f - float(texCoord->t) / mSkinHeight;
		}
}

////////////////////////////////////////////////////////////////////////////////
// Accessors
const int16_t Md2::SkinCount()      const {return mSkinCnt;}
const int16_t Md2::TexCoordCount()  const {return mTexCoordCnt;}
const int16_t Md2::TriangleCount()  const {return mTriangleCnt;}
const int16_t Md2::FrameCount()     const {return mFrameCnt;}
const int16_t Md2::VertexCount()    const {return mVertexCnt;}
const int16_t Md2::SkinWidth()      const {return mSkinWidth;}
const int16_t Md2::SkinHeight()     const {return mSkinHeight;}
bool Md2::IsPlaying() const {return mIsPlaying;}

////////////////////////////////////////////////////////////////////////////////
// Clean
void Md2::_Clear()
{
	delete[] mSkins;
	delete[] mTexCoords;
	delete[] mTriangles;
	delete[] mFrames;

	mSkins      = NULL;
	mTexCoords  = NULL;
	mTriangles  = NULL;
	mFrames     = NULL;

	mSkinCnt    = mTexCoordCnt
	            = mTriangleCnt
	            = mFrameCnt
	            = mVertexCnt
	            = mSkinWidth
	            = mSkinHeight
	            = -1;

}



