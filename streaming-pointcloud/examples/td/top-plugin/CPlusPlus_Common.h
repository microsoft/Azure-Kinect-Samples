/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

/*******
Derivative Developers: Make sure the virtual function order
stays the same, otherwise changes won't be backwards compatible
********/


#ifndef __CPlusPlus_Common
#define __CPlusPlus_Common


#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
	#include <stdint.h>
	#include "GL_Extensions.h"
	#define DLLEXPORT __declspec (dllexport)
#else
	#include <OpenGL/gltypes.h>
	#define DLLEXPORT
#endif

#include <assert.h>
#include <cmath>
#include <float.h>

#ifndef PyObject_HEAD
	struct _object;
	typedef _object PyObject;
#endif

struct cudaArray;

enum class OP_CPUMemPixelType : int32_t
{
	// 8-bit per color, BGRA pixels. This is preferred for 4 channel 8-bit data
	BGRA8Fixed = 0,
	// 8-bit per color, RGBA pixels. Only use this one if absolutely nesseary.
	RGBA8Fixed,
	// 32-bit float per color, RGBA pixels
	RGBA32Float,

	// A few single and two channel versions of the above
	R8Fixed,
	RG8Fixed,
	R32Float,
	RG32Float,

	R16Fixed = 100,
	RG16Fixed,
	RGBA16Fixed,

	R16Float = 200,
	RG16Float,
	RGBA16Float,
};

class OP_String;

// Used to describe this Plugin so it can be used as a custom OP.
// Can be filled in as part of the Fill*PluginInfo() callback
class OP_CustomOPInfo
{
public:
	// For this plugin to be treated as a Custom OP, all of the below fields
	// must be filled in correctly. Otherwise the .dll can only be used
	// when manually loaded into the C++ TOP

	// The type name of the node, this needs to be unique from all the other
	// TOP plugins loaded on the system. The name must start with an upper case
	// character (A-Z), and the rest should be lower case
	// Only the characters a-z and 0-9 are allowed in the opType.
	// Spaces are not allowed
	OP_String*		opType;

	// The english readable label for the node. This is what is shown in the 
	// OP Create Menu dialog.
	// Spaces and other special characters are allowed.
	// This can be a UTF-8 encoded string for non-english langauge label
	OP_String*		opLabel;

	// This should be three letters (upper or lower case), or numbers, which
	// are used to create an icon for this Custom OP.
	OP_String*		opIcon;

	// The minimum number of wired inputs required for this OP to function.
	int32_t			minInputs = 0;

	// The maximum number of connected inputs allowed for this OP. If this plugin
	// always requires 1 input, then set both min and max to 1.
	int32_t			maxInputs = 0;

	// The name of the author
	OP_String*		authorName;

	// The email of the author
	OP_String*		authorEmail;

	// Major version should be used to differentiate between drastically different
	// versions of this Custom OP. In particular changes that arn't backwards
	// compatible.
	// A project file will compare the major version of OPs saved in it with the
	// major version of the plugin installed on the system, and expect them to be
	// the same.
	int32_t			majorVersion = 0;

	// Minor version is used to denote upgrades to a plugin. It should be increased
	// when new features are added to a plugin that would cause loading up a project
	// with an older version of the plguin to behavior incorrectly. For example
	// if new parameters are added to the plugin.
	// A project file will expect the plugin installed on the system to be greater than
	// or equal to the plugin version the project was created with. Assuming
	// the majorVersion is the same.
	int32_t			minorVersion = 1;

	// If this Custom OP is using CPython objects (PyObject* etc.) obtained via
	// getParPython() calls, this needs to be set to the Python
	// version this plugin is compiled against.
	// 
	// This ensures when TD's Python version is upgraded the plugins will
	// error cleanly. This should be set to PY_VERSION as defined in
	// patchlevel.h from the Python include folder. (E.g, "3.5.1")
	// It should be left unchanged if CPython isn't being used in this plugin.
	OP_String*		pythonVersion;

	int32_t			reserved[98];
};


class OP_NodeInfo
{
public:

	// The full path to the operator
	const char*		opPath;

	// A unique ID representing the operator, no two operators will ever
	// have the same ID in a single TouchDesigner instance.
	uint32_t		opId;

	// This is the handle to the main TouchDesigner window.
	// It's possible this will be 0 the first few times the operator cooks,
	// incase it cooks while TouchDesigner is still loading up
#ifdef _WIN32
	HWND			mainWindowHandle;
#endif

	// The path to where the plugin's binary is located on this machine.
	// UTF8-8 encoded.
	const char*		pluginPath;

	int32_t			reserved[17];
};


class OP_DATInput
{
public:
	const char*		opPath;
	uint32_t		opId;

	int32_t         numRows;
	int32_t         numCols;
	bool            isTable;

	// data, referenced by (row,col), which will be a const char* for the
	// contents of the cell
	// E.g getCell(1,2) will be the contents of the cell located at (1,2)
	// The string will be in UTF-8 encoding.
	const char*
	getCell(int32_t row, int32_t col) const
	{
		return cellData[row * numCols + col];
	}

	const char**	cellData;

	// The number of times this node has cooked
	int64_t			totalCooks;

	int32_t         reserved[18];
};


class OP_TOPInput
{
public:
	const char*		opPath;
	uint32_t		opId;

	int32_t			width;
	int32_t			height;

	// You can use OP_Inputs::getTOPDataInCPUMemory() to download the
	// data from a TOP input into CPU memory easily.

	// The OpenGL Texture index for this TOP.
	// This is only valid when accessed from C++ TOPs.
	// Other C++ OPs will have this value set to 0 (invalid).
	GLuint			textureIndex;

	// The OpenGL Texture target for this TOP.
	// E.g GL_TEXTURE_2D, GL_TEXTURE_CUBE,
	// GL_TEXTURE_2D_ARRAY
	GLenum			textureType;

	// Depth for 3D and 2D_ARRAY textures, undefined
	// for other texture types
	uint32_t		depth;

	// contains the internalFormat for the texture
	// such as GL_RGBA8, GL_RGBA32F, GL_R16
	GLint			pixelFormat;

	int32_t			reserved1;

	// When the TOP_ExecuteMode is CUDA, this will be filled in
	cudaArray*		cudaInput;

	// The number of times this node has cooked
	int64_t			totalCooks;

	int32_t			reserved[14];
};

class OP_String
{
protected:
	OP_String()
	{
	}

	virtual ~OP_String()
	{
	}

public:

	// val is expected to be UTF-8 encoded
	virtual void	setString(const char* val) = 0;


	int32_t			reserved[20];

};


class OP_CHOPInput
{
public:

	const char*		opPath;
	uint32_t		opId;

	int32_t			numChannels;
	int32_t			numSamples;
	double			sampleRate;
	double			startIndex;



	// Retrieve a float array for a specific channel.
	// 'i' ranges from 0 to numChannels-1
	// The returned arrray contains 'numSamples' samples.
	// e.g: getChannelData(1)[10] will refer to the 11th sample in the 2nd channel

	const float*
	getChannelData(int32_t i) const
	{
		return channelData[i];
	}


	// Retrieve the name of a specific channel.
	// 'i' ranges from 0 to numChannels-1
	// For example getChannelName(1) is the name of the 2nd channel

	const char*
	getChannelName(int32_t i) const
	{
		return nameData[i];
	}

	const float**	channelData;
	const char**	nameData;

	// The number of times this node has cooked
	int64_t			totalCooks;

	int32_t			reserved[18];
};


class OP_ObjectInput
{
public:

	const char*		opPath;
	uint32_t		opId;

	// Use these methods to calculate object transforms
	double			worldTransform[4][4];
	double			localTransform[4][4];

	// The number of times this node has cooked
	int64_t			totalCooks;

	int32_t			reserved[18];
};


// The type of data the attribute holds
enum class AttribType : int32_t
{
	// One or more floats
	Float = 0,

	// One or more integers
	Int,
};

// Right now we only support point attributes.
enum class AttribSet : int32_t
{
	Invalid,
	Point = 0,
};

// The type of the primitives, currently only Polygon type
// is supported
enum class PrimitiveType : int32_t
{
	Invalid,
	Polygon = 0,
};


class Vector
{
public:
	Vector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Vector(float xx, float yy, float zz)
	{
		x = xx;
		y = yy;
		z = zz;
	}

	// inplace operators
	inline Vector&
	operator*=(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	inline Vector&
	operator/=(const float scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	inline Vector&
	operator-=(const Vector& trans)
	{
		x -= trans.x;
		y -= trans.y;
		z -= trans.z;
		return *this;
	}

	inline Vector&
	operator+=(const Vector& trans)
	{
		x += trans.x;
		y += trans.y;
		z += trans.z;
		return *this;
	}

	// non-inplace operations:
	inline Vector
	operator*(const float scalar)
	{
		Vector temp(*this);
		temp.x *= scalar;
		temp.y *= scalar;
		temp.z *= scalar;
		return temp;
	}

	inline Vector
	operator/(const float scalar)
	{
		Vector temp(*this);
		temp.x /= scalar;
		temp.y /= scalar;
		temp.z /= scalar;
		return temp;
	}

	inline Vector
	operator-(const Vector& trans)
	{
		Vector temp(*this);
		temp.x -= trans.x;
		temp.y -= trans.y;
		temp.z -= trans.z;
		return temp;
	}

	inline Vector
	operator+(const Vector& trans)
	{
		Vector temp(*this);
		temp.x += trans.x;
		temp.y += trans.y;
		temp.z += trans.z;
		return temp;
	}

	//------
	float
	dot(const Vector &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float
	length()
	{
		return sqrtf(dot(*this));
	}

	inline float
	normalize()
	{
		float dn = x * x + y * y + z * z;
		if (dn > FLT_MIN && dn != 1.0F)
		{
			dn = sqrtf(dn);
			(*this) /= dn;
		}
		return dn;
	}

	float x;
	float y;
	float z;
};

class Position
{
public:
	Position()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Position(float xx, float yy, float zz)
	{
		x = xx;
		y = yy;
		z = zz;
	}

	// in-place operators
	inline Position& operator*=(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	inline Position& operator/=(const float scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	inline Position& operator-=(const Vector& trans)
	{
		x -= trans.x;
		y -= trans.y;
		z -= trans.z;
		return *this;
	}

	inline Position& operator+=(const Vector& trans)
	{
		x += trans.x;
		y += trans.y;
		z += trans.z;
		return *this;
	}

	// non-inplace operators
	inline Position operator*(const float scalar)
	{
		Position temp(*this);
		temp.x *= scalar;
		temp.y *= scalar;
		temp.z *= scalar;
		return temp;
	}

	inline Position operator/(const float scalar)
	{
		Position temp(*this);
		temp.x /= scalar;
		temp.y /= scalar;
		temp.z /= scalar;
		return temp;
	}

	inline Position operator+(const Vector& trans)
	{
		Position temp(*this);
		temp.x += trans.x;
		temp.y += trans.y;
		temp.z += trans.z;
		return temp;
	}

	inline Position operator-(const Vector& trans)
	{
		Position temp(*this);
		temp.x -= trans.x;
		temp.y -= trans.y;
		temp.z -= trans.z;
		return temp;
	}

	float x;
	float y;
	float z;
};


class Color
{
public:
	Color ()
	{
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		a = 1.0f;
	}

	Color (float rr, float gg, float bb, float aa)
	{
		r = rr;
		g = gg;
		b = bb;
		a = aa;
	}

	float r;
	float g;
	float b;
	float a;
};


class TexCoord
{
public:
	TexCoord()
	{
		u = 0.0f;
		v = 0.0f;
		w = 0.0f;
	}

	TexCoord(float uu, float vv, float ww)
	{
		u = uu;
		v = vv;
		w = ww;
	}

	float u;
	float v;
	float w;
};

class BoundingBox
{
public:
	BoundingBox(float minx, float miny, float minz,
		float maxx, float maxy, float maxz) :
		minX(minx), minY(miny), minZ(minz), maxX(maxx), maxY(maxy), maxZ(maxz)
	{
	}

	BoundingBox(const Position& min, const Position& max)
	{
		minX = min.x;
		maxX = max.x;
		minY = min.y;
		maxY = max.y;
		minZ = min.z;
		maxZ = max.z;
	}

	BoundingBox(const Position& center, float x, float y, float z)
	{
		minX = center.x - x;
		maxX = center.x + x;
		minY = center.y - y;
		maxY = center.y + y;
		minZ = center.z - z;
		maxZ = center.z + z;
	}

	// enlarge the bounding box by the input point Position
	void
	enlargeBounds(const Position& pos)
	{
		if (pos.x < minX)
			minX = pos.x;
		if (pos.x > maxX)
			maxX = pos.x;
		if (pos.y < minY)
			minY = pos.y;
		if (pos.y > maxY)
			maxY = pos.y;
		if (pos.z < minZ)
			minZ = pos.z;
		if (pos.z > maxZ)
			maxZ = pos.z;
	}

	// enlarge the bounding box by the input bounding box:
	void
	enlargeBounds(const BoundingBox &box)
	{
		if (box.minX < minX)
			minX = box.minX;
		if (box.maxX > maxX)
			maxX = box.maxX;
		if (box.minY < minY)
			minY = box.minY;
		if (box.maxY > maxY)
			maxY = box.maxY;
		if (box.minZ < minZ)
			minZ = box.minZ;
		if (box.maxZ > maxZ)
			maxZ = box.maxZ;
	}

	// returns the bounding box length in x axis:
	float
	sizeX()
	{
		return maxX - minX;
	}

	// returns the bounding box length in y axis:
	float
	sizeY()
	{
		return maxY - minY;
	}

	// returns the bounding box length in z axis:
	float
	sizeZ()
	{
		return maxZ - minZ;
	}

	bool
	getCenter(Position* pos)
	{
		if (!pos)
			return false;
		pos->x = (minX + maxX) / 2.0f;
		pos->y = (minY + maxY) / 2.0f;
		pos->z = (minZ + maxZ) / 2.0f;
		return true;
	}

	// verifies if the input position (pos) is inside the current bounding box or not:
	bool
	isInside(const Position& pos)
	{
		if (pos.x >= minX && pos.x <= maxX &&
			pos.y >= minY && pos.y <= maxY &&
			pos.z >= minZ && pos.z <= maxZ)
			return true;
		else
			return false;
	}


	float minX;
	float minY;
	float minZ;

	float maxX;
	float maxY;
	float maxZ;

};


class SOP_NormalInfo
{
public:

	SOP_NormalInfo()
	{
		numNormals = 0;
		attribSet = AttribSet::Point;
		normals = nullptr;
	}

	int32_t			numNormals;
	AttribSet	 	attribSet;
	const Vector*	normals;
};

class SOP_ColorInfo
{
public:

	SOP_ColorInfo()
	{
		numColors = 0;
		attribSet = AttribSet::Point;
		colors = nullptr;
	}

	int32_t			numColors;
	AttribSet		attribSet;
	const Color*	colors;
};

class SOP_TextureInfo
{
public:

	SOP_TextureInfo()
	{
		numTextures = 0;
		attribSet = AttribSet::Point;
		textures = nullptr;
		numTextureLayers = 0;
	}

	int32_t			numTextures;
	AttribSet		attribSet;
	const TexCoord*	textures;
	int32_t			numTextureLayers;
};



// CustomAttribInfo, all the required data for each custom attribute
// this info can be queried by calling getCustomAttribute() which accepts
// two types of argument:
// 1) a valid index of a custom attribute
// 2) a valid name of a custom attribute
class SOP_CustomAttribInfo
{
public:

	SOP_CustomAttribInfo()
	{
		name = nullptr;
		numComponents = 0;
		attribType = AttribType::Float;
	}

	SOP_CustomAttribInfo(const char* n, int32_t numComp, const AttribType& type)
	{
		name = n;
		numComponents = numComp;
		attribType = type;
	}

	const char*			name;
	int32_t				numComponents;
	AttribType			attribType;
};

// SOP_CustomAttribData, all the required data for each custom attribute
// this info can be queried by calling getCustomAttribute() which accepts
// a valid name of a custom attribute
class SOP_CustomAttribData : public SOP_CustomAttribInfo
{
public:

	SOP_CustomAttribData()
	{
		name = nullptr;
		numComponents = 0;
		attribType = AttribType::Float;
		floatData = nullptr;
		intData = nullptr;
	}

	SOP_CustomAttribData(const char* n, int32_t numComp, const AttribType& type)
	{
		name = n;
		numComponents = numComp;
		attribType = type;
		floatData = nullptr;
		intData = nullptr;
	}

	const float*		floatData;
	const int32_t*		intData;

};

// SOP_PrimitiveInfo, all the required data for each primitive
// this info can be queried by calling getPrimitive() which accepts
// a valid index of a primitive as an input argument
class SOP_PrimitiveInfo
{
public:

	SOP_PrimitiveInfo()
	{
		pointIndices = nullptr;
		numVertices = 0;
		type = PrimitiveType::Invalid;
		pointIndicesOffset = 0;
	}

	// number of vertices of this prim
	int32_t			numVertices;

	// all the indices of the vertices of the primitive. This array has
	// numVertices entries in it
	const int32_t*	pointIndices;

	// The type of this primitive
	PrimitiveType	type;

	// the offset of the this primitive's point indices in the index array
	// returned from getAllPrimPointIndices()
	int32_t			pointIndicesOffset;

};




class OP_SOPInput
{
public:

	virtual ~OP_SOPInput()
	{
	}



	const char*		opPath;
	uint32_t		opId;


	// Returns the total number of points
	virtual int32_t 		getNumPoints() const = 0;

	// The total number of vertices, across all primitives.
	virtual int32_t			getNumVertices() const = 0;

	// The total number of primitives
	virtual int32_t			getNumPrimitives() const = 0;

	// The total number of custom attributes
	virtual int32_t			getNumCustomAttributes() const = 0;

	// Returns an array of point positions. This array is getNumPoints() long.
	virtual const Position*	getPointPositions() const = 0;

	// Returns an array of normals.
	//
	// Returns nullptr if no normals are present
	virtual const SOP_NormalInfo* 	getNormals() const = 0;

	// Returns an array of colors.
	// Returns nullptr if no colors are present
	virtual const SOP_ColorInfo* 	getColors() const = 0;

	// Returns an array of texture coordinates.
	// If multiple texture coordinate layers are present, they will be placed
	// interleaved back-to-back.
	// E.g layer0 followed by layer1 followed by layer0 etc.
	//
	// Returns nullptr if no texture layers are present
	virtual const SOP_TextureInfo*	getTextures() const = 0;

	// Returns the custom attribute data with an input index
	virtual const SOP_CustomAttribData*	getCustomAttribute(int32_t customAttribIndex) const = 0;

	// Returns the custom attribute data with its name
	virtual const SOP_CustomAttribData*	getCustomAttribute(const char* customAttribName) const = 0;

	// Returns true if the SOP has a normal attribute of the given source
	// attribute 'N'
	virtual bool			hasNormals() const = 0;

	// Returns true if the SOP has a color the given source
	// attribute 'Cd'
	virtual bool			hasColors() const = 0;

	// Returns the SOP_PrimitiveInfo with primIndex
	const SOP_PrimitiveInfo
	getPrimitive(int32_t primIndex) const
	{
		return myPrimsInfo[primIndex];
	}

	// Returns the full list of all the point indices for all primitives.
	// The primitives are stored back to back in this array.
	// This is a faster but harder way to work with primitives than
	// getPrimPointIndices()
	const int32_t*
	getAllPrimPointIndices()
	{
		return myPrimPointIndices;
	}

	SOP_PrimitiveInfo*		myPrimsInfo;
	const int32_t*			myPrimPointIndices;

	// The number of times this node has cooked
	int64_t			totalCooks;

	int32_t			reserved[98];
};



enum class OP_TOPInputDownloadType : int32_t
{
	// The texture data will be downloaded and and available on the next frame.
	// Except for the first time this is used, getTOPDataInCPUMemory()
	// will return the texture data on the CPU from the previous frame.
	// The first getTOPDataInCPUMemory() is called it will be nullptr.
	// ** This mode should be used is most cases for performance reasons **
	Delayed = 0,

	// The texture data will be downloaded immediately and be available
	// this frame. This can cause a large stall though and should be avoided
	// in most cases
	Instant,
};

class OP_TOPInputDownloadOptions
{
public:
	OP_TOPInputDownloadOptions()
	{
		downloadType = OP_TOPInputDownloadType::Delayed;
		verticalFlip = false;
		cpuMemPixelType = OP_CPUMemPixelType::BGRA8Fixed;
	}

	OP_TOPInputDownloadType	downloadType;

	// Set this to true if you want the image vertically flipped in the
	// downloaded data
	bool					verticalFlip;

	// Set this to how you want the pixel data to be give to you in CPU
	// memory. BGRA8Fixed should be used for 4 channel 8-bit data if possible
	OP_CPUMemPixelType		cpuMemPixelType;

};

class OP_TimeInfo
{
public:

	// same as global Python value absTime.frame. Counts up forever
	// since the application started. In rootFPS units.
	int64_t	absFrame;

	// The timeline frame number for this cook
	double	frame;

	// The timeline FPS/rate this node is cooking at.
	// If the component this node is located in has Component Time, it's FPS
	// may be different than the Root FPS
	double	rate;

	// The frame number for the root timeline. Different than frame
	// if the node is in a component that has component time.
	double 	rootFrame;

	// The Root FPS/Rate the file is running at.
	double	rootRate;

	// The number of frames that have elapsed since the last cook occured.
	// This can be more than one if frames were dropped.
	// If this is the first time this node is cooking, this will be 0.0
	// This is in 'rate' units, not 'rootRate' units.
	double	deltaFrames;

	// The number of milliseconds that have elapsed since the last cook.
	// Note that this isn't done via CPU timers, but is instead 
	// simply deltaFrames * milliSecondsPerFrame
	double	deltaMS;



	int32_t	reserved[40];
};


class OP_Inputs
{
public:
	// NOTE: When writting a TOP, none of these functions should
	// be called inside a beginGLCommands()/endGLCommands() section
	// as they may require GL themselves to complete execution.

	// Inputs that are wired into the node. Note that since some inputs
	// may not be connected this number doesn't mean that that the first N
	// inputs are connected. For example on a 3 input node if the 3rd input
	// is only one connected, this will return 1, and getInput*(0) and (1)
	// will return nullptr.
	virtual int32_t		getNumInputs() const = 0;

	// Will return nullptr when the input has nothing connected to it.
	// only valid for C++ TOP operators
	virtual const OP_TOPInput*		getInputTOP(int32_t index) const = 0;
	// Only valid for C++ CHOP operators
	virtual const OP_CHOPInput*		getInputCHOP(int32_t index) const = 0;
	// getInputSOP() declared later on in the class
	// getInputDAT() declared later on in the class

	// these are defined by parameters.
	// may return nullptr when invalid input
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	virtual const OP_DATInput*		getParDAT(const char *name) const = 0;
	virtual const OP_TOPInput*		getParTOP(const char *name) const = 0;
	virtual const OP_CHOPInput*		getParCHOP(const char *name) const = 0;
	virtual const OP_ObjectInput*	getParObject(const char *name) const = 0;
	// getParSOP() declared later on in the class

	// these work on any type of parameter and can be interchanged
	// for menu types, int returns the menu selection index, string returns the item

	// returns the requested value, index may be 0 to 4.
	virtual double		getParDouble(const char* name, int32_t index = 0) const = 0;

	// for multiple values: returns True on success/false otherwise
	virtual bool        getParDouble2(const char* name, double &v0, double &v1) const = 0;
	virtual bool        getParDouble3(const char* name, double &v0, double &v1, double &v2) const = 0;
	virtual bool        getParDouble4(const char* name, double &v0, double &v1, double &v2, double &v3) const = 0;


	// returns the requested value
	virtual int32_t		getParInt(const char* name, int32_t index = 0) const = 0;

	// for multiple values: returns True on success/false otherwise
	virtual bool        getParInt2(const char* name, int32_t &v0, int32_t &v1) const = 0;
	virtual bool        getParInt3(const char* name, int32_t &v0, int32_t &v1, int32_t &v2) const = 0;
	virtual bool        getParInt4(const char* name, int32_t &v0, int32_t &v1, int32_t &v2, int32_t &v3) const = 0;

	// returns the requested value
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	// return value usable for life of parameter
	// The returned string will be in UTF-8 encoding.
	virtual const char*	getParString(const char* name) const = 0;


	// this is similar to getParString, but will return an absolute path if it exists, with
	// slash direction consistent with O/S requirements.
	// to get the original parameter value, use getParString
	// return value usable for life of parameter
	// The returned string will be in UTF-8 encoding.
	virtual const char*	getParFilePath(const char* name) const = 0;

	// returns true on success
	// from_name and to_name must be Object parameters
	virtual bool	getRelativeTransform(const char* from_name, const char* to_name, double matrix[4][4]) const = 0;


	// disable or enable updating of the parameter
	virtual void		 enablePar(const char* name, bool onoff) const = 0;


	// these are defined by paths.
	// may return nullptr when invalid input
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	virtual const OP_DATInput*		getDAT(const char *path) const = 0;
	virtual const OP_TOPInput*		getTOP(const char *path) const = 0;
	virtual const OP_CHOPInput*		getCHOP(const char *path) const = 0;
	virtual const OP_ObjectInput*	getObject(const char *path) const = 0;


	// This function can be used to retrieve the TOPs texture data in CPU
	// memory. You must pass the OP_TOPInput object you get from
	// getParTOP/getInputTOP into this, not a copy you've made
	//
	// Fill in a OP_TOPIputDownloadOptions class with the desired options set
	//
	// Returns the data, which will be valid until the end of execute()
	// Returned value may be nullptr in some cases, such as the first call
	// to this with options->downloadType == OP_TOP_DOWNLOAD_DELAYED.
	virtual void* 					getTOPDataInCPUMemory(const OP_TOPInput *top,
		const OP_TOPInputDownloadOptions *options) const = 0;


	virtual const OP_SOPInput*		getParSOP(const char *name) const = 0;
	// only valid for C++ SOP operators
	virtual const OP_SOPInput*		getInputSOP(int32_t index) const = 0;
	virtual const OP_SOPInput*		getSOP(const char *path) const = 0;

	// only valid for C++ DAT operators
	virtual const OP_DATInput*		getInputDAT(int32_t index) const = 0;

	// To use Python in your Plugin you need to fill the
	// customOPInfo.pythonVersion member in Fill*PluginInfo.
	//
	// The returned object does NOT have it's reference count incremented.
	// So increment it if you want to hold onto the object, and only
	// decement it if you've incremented it.
	virtual PyObject*				getParPython(const char* name) const = 0;


	// Returns a class whose members gives you information about timing
	// such as FPS and delta-time since the last cook.
	// See OP_TimeInfo for more information
	virtual const OP_TimeInfo*		getTimeInfo() const = 0;

};

class OP_InfoCHOPChan
{
public:
	OP_String*		name;
	float			value;

	int32_t			reserved[10];
};


class OP_InfoDATSize
{
public:

	// Set this to the size you want the table to be

	int32_t			rows;
	int32_t			cols;

	// Set this to true if you want to return DAT entries on a column
	// by column basis.
	// Otherwise set to false, and you'll be expected to set them on
	// a row by row basis.
	// DEFAULT : false

	bool			byColumn;

	int32_t			reserved[10];
};


class OP_InfoDATEntries
{
public:

	// This is an array of OP_String* pointers which you are expected to assign
	// values to.
	// e.g values[1]->setString("myColumnName");
	// The string should be in UTF-8 encoding.
	OP_String**			values;

	int32_t			reserved[10];
};


class OP_NumericParameter
{
public:

	OP_NumericParameter(const char* iname = nullptr)
	{
		name = iname;
		label = page = nullptr;

		for (int i = 0; i<4; i++)
		{
			defaultValues[i] = 0.0;

			minSliders[i] = 0.0;
			maxSliders[i] = 1.0;

			minValues[i] = 0.0;
			maxValues[i] = 1.0;

			clampMins[i] = false;
			clampMaxes[i] = false;
		}
	}

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	// Must begin with capital letter, and contain no spaces
	const char*	name;
	const char*	label;
	const char*	page;

	double		defaultValues[4];
	double		minValues[4];
	double		maxValues[4];

	bool		clampMins[4];
	bool		clampMaxes[4];

	double		minSliders[4];
	double		maxSliders[4];

	int32_t		reserved[20];

};


class OP_StringParameter
{
public:

	OP_StringParameter(const char* iname = nullptr)
	{
		name = iname;
		label = page = nullptr;
		defaultValue = nullptr;
	}

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.

	// Must begin with capital letter, and contain no spaces
	const char*	name;
	const char*	label;
	const char*	page;

	// This should be in UTF-8 encoding.
	const char*	defaultValue;

	int32_t		reserved[20];
};


enum class OP_ParAppendResult : int32_t
{
	Success = 0,
	InvalidName,	// invalid or duplicate name
	InvalidSize,	// size out of range
};


class OP_ParameterManager
{

public:

	// Returns PARAMETER_APPEND_SUCCESS on succesful

	virtual OP_ParAppendResult		appendFloat(const OP_NumericParameter &np, int32_t size = 1) = 0;
	virtual OP_ParAppendResult		appendInt(const OP_NumericParameter &np, int32_t size = 1) = 0;

	virtual OP_ParAppendResult		appendXY(const OP_NumericParameter &np) = 0;
	virtual OP_ParAppendResult		appendXYZ(const OP_NumericParameter &np) = 0;

	virtual OP_ParAppendResult		appendUV(const OP_NumericParameter &np) = 0;
	virtual OP_ParAppendResult		appendUVW(const OP_NumericParameter &np) = 0;

	virtual OP_ParAppendResult		appendRGB(const OP_NumericParameter &np) = 0;
	virtual OP_ParAppendResult		appendRGBA(const OP_NumericParameter &np) = 0;

	virtual OP_ParAppendResult		appendToggle(const OP_NumericParameter &np) = 0;
	virtual OP_ParAppendResult		appendPulse(const OP_NumericParameter &np) = 0;

	virtual OP_ParAppendResult		appendString(const OP_StringParameter &sp) = 0;
	virtual OP_ParAppendResult		appendFile(const OP_StringParameter &sp) = 0;
	virtual OP_ParAppendResult		appendFolder(const OP_StringParameter &sp) = 0;

	virtual OP_ParAppendResult		appendDAT(const OP_StringParameter &sp) = 0;
	virtual OP_ParAppendResult		appendCHOP(const OP_StringParameter &sp) = 0;
	virtual OP_ParAppendResult		appendTOP(const OP_StringParameter &sp) = 0;
	virtual OP_ParAppendResult		appendObject(const OP_StringParameter &sp) = 0;
	// appendSOP() located further down in the class


	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	virtual OP_ParAppendResult		appendMenu(const OP_StringParameter &sp,
		int32_t nitems, const char **names,
		const char **labels) = 0;

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	virtual OP_ParAppendResult		appendStringMenu(const OP_StringParameter &sp,
		int32_t nitems, const char **names,
		const char **labels) = 0;

	virtual OP_ParAppendResult		appendSOP(const OP_StringParameter &sp) = 0;

	// To use Python in your Plugin you need to fill the
	// customOPInfo.pythonVersion member in Fill*PluginInfo.
	virtual OP_ParAppendResult		appendPython(const OP_StringParameter &sp) = 0;


};

static_assert(offsetof(OP_CustomOPInfo,	opType) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	opLabel) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	opIcon) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	minInputs) == 24, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	maxInputs) == 28, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	authorName) == 32, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	authorEmail) == 40, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	majorVersion) == 48, "Incorrect Alignment");
static_assert(offsetof(OP_CustomOPInfo,	minorVersion) == 52, "Incorrect Alignment");
static_assert(sizeof(OP_CustomOPInfo) == 456, "Incorrect Size");

static_assert(offsetof(OP_NodeInfo, opPath) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_NodeInfo, opId) == 8, "Incorrect Alignment");
#ifdef _WIN32
	static_assert(offsetof(OP_NodeInfo, mainWindowHandle) == 16, "Incorrect Alignment");
	static_assert(sizeof(OP_NodeInfo) == 104, "Incorrect Size");
#else
	static_assert(sizeof(OP_NodeInfo) == 96, "Incorrect Size");
#endif

static_assert(offsetof(OP_DATInput, opPath) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, opId) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, numRows) == 12, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, numCols) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, isTable) == 20, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, cellData) == 24, "Incorrect Alignment");
static_assert(offsetof(OP_DATInput, totalCooks) == 32, "Incorrect Alignment");
static_assert(sizeof(OP_DATInput) == 112, "Incorrect Size");

static_assert(offsetof(OP_TOPInput, opPath) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, opId) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, width) == 12, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, height) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, textureIndex) == 20, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, textureType) == 24, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, depth) == 28, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, pixelFormat) == 32, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, cudaInput) == 40, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInput, totalCooks) == 48, "Incorrect Alignment");
static_assert(sizeof(OP_TOPInput) == 112, "Incorrect Size");

static_assert(offsetof(OP_CHOPInput, opPath) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, opId) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, numChannels) == 12, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, numSamples) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, sampleRate) == 24, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, startIndex) == 32, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, channelData) == 40, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, nameData) == 48, "Incorrect Alignment");
static_assert(offsetof(OP_CHOPInput, totalCooks) == 56, "Incorrect Alignment");
static_assert(sizeof(OP_CHOPInput) == 136, "Incorrect Size");

static_assert(offsetof(OP_ObjectInput, opPath) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_ObjectInput, opId) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_ObjectInput, worldTransform) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_ObjectInput, localTransform) == 144, "Incorrect Alignment");
static_assert(offsetof(OP_ObjectInput, totalCooks) == 272, "Incorrect Alignment");
static_assert(sizeof(OP_ObjectInput) == 352, "Incorrect Size");

static_assert(offsetof(Position, x) == 0, "Incorrect Alignment");
static_assert(offsetof(Position, y) == 4, "Incorrect Alignment");
static_assert(offsetof(Position, z) == 8, "Incorrect Alignment");
static_assert(sizeof(Position) == 12, "Incorrect Size");

static_assert(offsetof(Vector, x) == 0, "Incorrect Alignment");
static_assert(offsetof(Vector, y) == 4, "Incorrect Alignment");
static_assert(offsetof(Vector, z) == 8, "Incorrect Alignment");
static_assert(sizeof(Vector) == 12, "Incorrect Size");

static_assert(offsetof(Color, r) == 0, "Incorrect Alignment");
static_assert(offsetof(Color, g) == 4, "Incorrect Alignment");
static_assert(offsetof(Color, b) == 8, "Incorrect Alignment");
static_assert(offsetof(Color, a) == 12, "Incorrect Alignment");
static_assert(sizeof(Color) == 16, "Incorrect Size");

static_assert(offsetof(TexCoord, u) == 0, "Incorrect Alignment");
static_assert(offsetof(TexCoord, v) == 4, "Incorrect Alignment");
static_assert(offsetof(TexCoord, w) == 8, "Incorrect Alignment");
static_assert(sizeof(TexCoord) == 12, "Incorrect Size");

static_assert(offsetof(SOP_NormalInfo, numNormals) == 0, "Incorrect Alignment");
static_assert(offsetof(SOP_NormalInfo, attribSet) == 4, "Incorrect Alignment");
static_assert(offsetof(SOP_NormalInfo, normals) == 8, "Incorrect Alignment");
static_assert(sizeof(SOP_NormalInfo) == 16, "Incorrect Size");

static_assert(offsetof(SOP_ColorInfo, numColors) == 0, "Incorrect Alignment");
static_assert(offsetof(SOP_ColorInfo, attribSet) == 4, "Incorrect Alignment");
static_assert(offsetof(SOP_ColorInfo, colors) == 8, "Incorrect Alignment");
static_assert(sizeof(SOP_ColorInfo) == 16, "Incorrect Size");

static_assert(offsetof(SOP_TextureInfo, numTextures) == 0, "Incorrect Alignment");
static_assert(offsetof(SOP_TextureInfo, attribSet) == 4, "Incorrect Alignment");
static_assert(offsetof(SOP_TextureInfo, textures) == 8, "Incorrect Alignment");
static_assert(offsetof(SOP_TextureInfo, numTextureLayers) == 16, "Incorrect Alignment");
static_assert(sizeof(SOP_TextureInfo) == 24, "Incorrect Size");

static_assert(offsetof(SOP_CustomAttribData, name) == 0, "Incorrect Alignment");
static_assert(offsetof(SOP_CustomAttribData, numComponents) == 8, "Incorrect Alignment");
static_assert(offsetof(SOP_CustomAttribData, attribType) == 12, "Incorrect Alignment");
static_assert(offsetof(SOP_CustomAttribData, floatData) == 16, "Incorrect Alignment");
static_assert(offsetof(SOP_CustomAttribData, intData) == 24, "Incorrect Alignment");
static_assert(sizeof(SOP_CustomAttribData) == 32, "Incorrect Size");

static_assert(offsetof(SOP_PrimitiveInfo, numVertices) == 0, "Incorrect Alignment");
static_assert(offsetof(SOP_PrimitiveInfo, pointIndices) == 8, "Incorrect Alignment");
static_assert(offsetof(SOP_PrimitiveInfo, type) == 16, "Incorrect Alignment");
static_assert(offsetof(SOP_PrimitiveInfo, pointIndicesOffset) == 20, "Incorrect Alignment");
static_assert(sizeof(SOP_PrimitiveInfo) == 24, "Incorrect Size");

static_assert(sizeof(OP_SOPInput) == 440, "Incorrect Size");

static_assert(offsetof(OP_TOPInputDownloadOptions, downloadType) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInputDownloadOptions, verticalFlip) == 4, "Incorrect Alignment");
static_assert(offsetof(OP_TOPInputDownloadOptions, cpuMemPixelType) == 8, "Incorrect Alignment");
static_assert(sizeof(OP_TOPInputDownloadOptions) == 12, "Incorrect Size");

static_assert(offsetof(OP_InfoCHOPChan, name) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_InfoCHOPChan, value) == 8, "Incorrect Alignment");
static_assert(sizeof(OP_InfoCHOPChan) == 56, "Incorrect Size");

static_assert(offsetof(OP_InfoDATSize, rows) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_InfoDATSize, cols) == 4, "Incorrect Alignment");
static_assert(offsetof(OP_InfoDATSize, byColumn) == 8, "Incorrect Alignment");
static_assert(sizeof(OP_InfoDATSize) == 52, "Incorrect Size");

static_assert(offsetof(OP_InfoDATEntries, values) == 0, "Incorrect Alignment");
static_assert(sizeof(OP_InfoDATEntries) == 48, "Incorrect Size");

static_assert(offsetof(OP_NumericParameter, name) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, label) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, page) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, defaultValues) == 24, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, minValues) == 56, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, maxValues) == 88, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, clampMins) == 120, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, clampMaxes) == 124, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, minSliders) == 128, "Incorrect Alignment");
static_assert(offsetof(OP_NumericParameter, maxSliders) == 160, "Incorrect Alignment");
static_assert(sizeof(OP_NumericParameter) == 272, "Incorrect Size");

static_assert(offsetof(OP_StringParameter, name) == 0, "Incorrect Alignment");
static_assert(offsetof(OP_StringParameter, label) == 8, "Incorrect Alignment");
static_assert(offsetof(OP_StringParameter, page) == 16, "Incorrect Alignment");
static_assert(offsetof(OP_StringParameter, defaultValue) == 24, "Incorrect Alignment");
static_assert(sizeof(OP_StringParameter) == 112, "Incorrect Size");
static_assert(sizeof(OP_TimeInfo) == 216, "Incorrect Size");
#endif
