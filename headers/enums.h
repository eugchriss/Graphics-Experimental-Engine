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
		DEPTH_STENCIL
	};

	enum TargetOp
	{
		CLEAR,
		DONT_CARE,
		STORE
	};

}
