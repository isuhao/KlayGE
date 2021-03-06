// RenderFactory.cpp
// KlayGE 渲染工厂类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 增加了MakeSamplerStateObject (2008.9.21)
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Query.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Fence.hpp>
#include <KFL/Hash.hpp>

#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	RenderFactory::~RenderFactory()
	{
		for (auto& rs : rs_pool_)
		{
			rs.second.reset();
		}
		for (auto& ss : ss_pool_)
		{
			ss.second.reset();
		}

		re_.reset();
	}

	RenderEngine& RenderFactory::RenderEngineInstance()
	{
		if (!re_)
		{
			re_ = this->DoMakeRenderEngine();
		}

		return *re_;
	}
	
	void RenderFactory::Suspend()
	{
		if (re_)
		{
			re_->Suspend();
		}
		this->DoSuspend();
	}
	
	void RenderFactory::Resume()
	{
		this->DoResume();
		if (re_)
		{
			re_->Resume();
		}
	}

	TexturePtr RenderFactory::MakeTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		TexturePtr ret = this->MakeDelayCreationTexture1D(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
		ret->CreateHWResource(init_data);
		return ret;
	}

	TexturePtr RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		TexturePtr ret = this->MakeDelayCreationTexture2D(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
		ret->CreateHWResource(init_data);
		return ret;
	}

	TexturePtr RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		TexturePtr ret = this->MakeDelayCreationTexture3D(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
		ret->CreateHWResource(init_data);
		return ret;
	}

	TexturePtr RenderFactory::MakeTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint, ElementInitData const * init_data)
	{
		TexturePtr ret = this->MakeDelayCreationTextureCube(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
		ret->CreateHWResource(init_data);
		return ret;
	}

	GraphicsBufferPtr RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt)
	{
		GraphicsBufferPtr ret = this->MakeDelayCreationVertexBuffer(usage, access_hint, size_in_byte, fmt);
		ret->CreateHWResource(init_data);
		return ret;
	}

	GraphicsBufferPtr RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt)
	{
		GraphicsBufferPtr ret = this->MakeDelayCreationIndexBuffer(usage, access_hint, size_in_byte, fmt);
		ret->CreateHWResource(init_data);
		return ret;
	}

	GraphicsBufferPtr RenderFactory::MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt)
	{
		GraphicsBufferPtr ret = this->MakeDelayCreationConstantBuffer(usage, access_hint, size_in_byte, fmt);
		ret->CreateHWResource(init_data);
		return ret;
	}

	RenderStateObjectPtr RenderFactory::MakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
		BlendStateDesc const & bs_desc)
	{
		RenderStateObjectPtr ret;

		char const * rs_desc_begin = reinterpret_cast<char const *>(&rs_desc);
		char const * rs_desc_end = rs_desc_begin + sizeof(rs_desc);
		char const * dss_desc_begin = reinterpret_cast<char const *>(&dss_desc);
		char const * dss_desc_end = dss_desc_begin + sizeof(dss_desc);
		char const * bs_desc_begin = reinterpret_cast<char const *>(&bs_desc);
		char const * bs_desc_end = bs_desc_begin + sizeof(bs_desc);

		size_t seed = HashRange(rs_desc_begin, rs_desc_end);
		HashRange(seed, dss_desc_begin, dss_desc_end);
		HashRange(seed, bs_desc_begin, bs_desc_end);

		auto iter = rs_pool_.find(seed);
		if (iter == rs_pool_.end())
		{
			ret = this->DoMakeRenderStateObject(rs_desc, dss_desc, bs_desc);
			rs_pool_.emplace(seed, ret);
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}

	SamplerStateObjectPtr RenderFactory::MakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		SamplerStateObjectPtr ret;

		char const * desc_begin = reinterpret_cast<char const *>(&desc);
		char const * desc_end = desc_begin + sizeof(desc);

		size_t seed = HashRange(desc_begin, desc_end);

		auto iter = ss_pool_.find(seed);
		if (iter == ss_pool_.end())
		{
			ret = this->DoMakeSamplerStateObject(desc);
			ss_pool_.emplace(seed, ret);
		}
		else
		{
			ret = iter->second;
		}

		return ret;
	}
}
