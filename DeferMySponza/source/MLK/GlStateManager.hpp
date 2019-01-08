#pragma once

#include <functional>
#include <unordered_map>

namespace MLK
{
    /// <summary>
    /// Enum to specify which draw pass you wish to do, the necessary openGl state is attached to the drawpass value.
    /// </summary>
    enum DrawPass
    {
        NoPass,
        GBufferPass,
		AmbientPass,
		FullScreenPass,
        LightStencilPass,
        LightShadingPass,
		ShadowMapPass,
        SSRPass,
        SMAAEdge,
        SMAABlend,
        SMAAResolve
    };

	/// <summary>
    /// Implementation to help manage witching of OpenGl variables. The different passes require multiple
    /// GL variables to be set, this groups them and ensures they aren't switched unnecessarily.
    /// </summary>
	class GlStateManager
	{
	public:
        GlStateManager();
		~GlStateManager();

        void setState(DrawPass pass);

	private:
        void setGBufferPass();
        void setFullScreenPass();
		void setAmbientPass();
        void setLightStencilPass();
        void setLightShadingPass();
		void setShadowMapPass();
        void setSMAAEdge();
        void setSMAABlend();
        void setSMAAResolve();
        void setSSRPass();

        DrawPass m_currentPass = DrawPass::NoPass;
        std::unordered_map<DrawPass, std::function<void()>> m_passFunctions;
	};
}