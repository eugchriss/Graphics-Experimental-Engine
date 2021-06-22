#pragma once

namespace gee
{
	enum class TEXTURE_SLOT
	{
		COLOR,
		NORMAL,
		SPECULAR
	};
	enum TargetFormat
	{
		D24_UNORM_S8_UINT
	};
	enum TargetUsage
	{
		COLOR,
		DEPTH,
		SHADER_TEXTURE,
		READ
	};
	enum TargetOp
	{
		CLEAR,
		DONT_CARE,
		STORE
	};
	enum Filter
	{
		NEAREST,
		LINEAR
	};

	enum AddressMode
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};
	enum CompareOp
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_OR_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_OR_EQUAL,
		ALWAYS
	};
	enum BorderColor
	{
		TRANSPARENT_BLACK,
		OPAQUE_BLACK,
		OPAQUE_WHITE
	};

}
