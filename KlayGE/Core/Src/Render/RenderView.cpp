// RenderView.cpp
// KlayGE 渲染视图类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	class NullRenderView : public RenderView
	{
	public:
		void Clear(Color const & /*clr*/)
		{
		}

		void Clear(float /*depth*/)
		{
		}
		
		void Clear(int32_t /*stencil*/)
		{
		}

		void Clear(float /*depth*/, int32_t /*stencil*/)
		{
		}

		void OnAttached(FrameBuffer& /*fb*/, uint32_t /*att*/)
		{
		}

		void OnDetached(FrameBuffer& /*fb*/, uint32_t /*att*/)
		{
		}

		void OnBind(FrameBuffer& /*fb*/, uint32_t /*att*/)
		{
		}

		void OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
		{
		}
	};

	RenderViewPtr RenderView::NullObject()
	{
		static RenderViewPtr obj(new NullRenderView);
		return obj;
	}

	void RenderView::OnBind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
	
	void RenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
}
