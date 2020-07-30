
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <core/Texture2D.hpp>
#include <rendering/ARenderer.hpp>
#include <rendering/deferred/DfrLightCaster.hpp>
#include <core/VertexBuffer.hpp>
#include <rendering/postprocess/HdrTonemap.hpp>

namespace blaze
{
/**
 * @brief Deferred rendering specialization of ARenderer
 *
 * This class will be able to setup a PBR/IBL deferred renderer.
 */
class DfrRenderer final : public ARenderer
{
private:
	constexpr static std::string_view vMRTShaderFileName = "shaders/deferred/vMRT.vert.spv";
	constexpr static std::string_view fMRTShaderFileName = "shaders/deferred/fMRT.frag.spv";

	constexpr static std::string_view vLightingShaderFileName = "shaders/deferred/vLighting.vert.spv";
	constexpr static std::string_view fLightingShaderFileName = "shaders/deferred/fLighting.frag.spv";

	constexpr static std::string_view vDirLightingShaderFileName = "shaders/deferred/vDirLighting.vert.spv";
	constexpr static std::string_view fDirLightingShaderFileName = "shaders/deferred/fDirLighting.frag.spv";

	constexpr static std::string_view vTransparencyShaderFileName = "shaders/deferred/vTransparency.vert.spv";
	constexpr static std::string_view fTransparencyShaderFileName = "shaders/deferred/fTransparency.frag.spv";

	struct Settings
	{
		int enableIBL{1};
		enum : int
		{
			POSITION = 0x0,
			NORMAL = 0x1,
			ALBEDO = 0x2,
			OMR = 0x3,
			EMISSION = 0x4,
			RENDER = 0x5,
		} viewRT{RENDER};
	} settings;

	using CameraUBOV = UBOVector<Camera::UBlock>;
	using SettingsUBOV = UBOVector<Settings>;

	Texture2D depthBuffer;
	spirv::RenderPass mrtRenderPass;
	spirv::RenderPass lightingRenderPass;

	struct MRTAttachment
	{
		Texture2D position;
		Texture2D normal;
		Texture2D albedo;
		Texture2D omr;
		Texture2D emission;

		constexpr static uint32_t attachmentCount = 5;

		bool valid() const
		{
			return position.valid() && normal.valid() && albedo.valid() && omr.valid() && emission.valid();
		}
	} mrtAttachment;
	Texture2D outputAttachment;

	spirv::SetSingleton mrtAttachmentSet;

	spirv::Shader mrtShader;
	spirv::Pipeline mrtPipeline;

	spirv::Shader pointLightShader;
	spirv::Pipeline pointLightPipeline;

	spirv::Shader dirLightShader;
	spirv::Pipeline dirLightPipeline;

	spirv::Shader forwardShader;
	spirv::Pipeline forwardPipeline;

	vkw::Framebuffer mrtFramebuffer;
	vkw::Framebuffer lightingFramebuffer;

	CameraUBOV cameraUBOs;
	SettingsUBOV settingsUBOs;
	spirv::SetVector cameraSets;

	IndexedVertexBuffer<Vertex> lightVolume;
	IndexedVertexBuffer<Vertex> lightQuad;

	std::unique_ptr<DfrLightCaster> lightCaster;

	spirv::SetSingleton environmentSet;

	// Post processing

	spirv::RenderPass postProcessRenderPass;
	vkw::FramebufferVector postProcessFramebuffers;

	HDRTonemap hdrTonemap;

public:
	/**
	 * @fn DfrRenderer()
	 *
	 * @brief Default empty constructor.
	 */
	DfrRenderer() noexcept
	{
	}

	/**
	 * @brief Actual constructor
	 *
	 * @param window The GLFW window pointer.
	 * @param enableValidationLayers Enable for Debugging.
	 */
	DfrRenderer(GLFWwindow* window, bool enableValidationLayers = true) noexcept;

	/**
	 * @name Constructors.
	 *
	 * @brief Pointer use only.
	 * @{
	 */
	DfrRenderer(DfrRenderer&& other) = delete;
	DfrRenderer& operator=(DfrRenderer&& other) = delete;
	DfrRenderer(const DfrRenderer& other) = delete;
	DfrRenderer& operator=(const DfrRenderer& other) = delete;
	/**
	 * @}
	 */

	// Inherited via ARenderer
	virtual const spirv::Shader* get_shader() const override;
	virtual ALightCaster* get_lightCaster() override;
	virtual void drawSettings() override;

protected:
	virtual void update(uint32_t frame) override;
	virtual void recordCommands(uint32_t frame) override;
	virtual void recreateSwapchainDependents() override;

private:
	spirv::RenderPass createMRTRenderpass();
	spirv::RenderPass createLightingRenderpass();
	Texture2D createDepthBuffer() const;
	Texture2D createOutputAttachment() const;

	spirv::Shader createMRTShader();
	spirv::Pipeline createMRTPipeline();
	MRTAttachment createMRTAttachment();
	spirv::SetSingleton createMRTSet();

	spirv::Shader createPointLightingShader();
	spirv::Pipeline createPointLightingPipeline();

	spirv::Shader createDirLightingShader();
	spirv::Pipeline createDirLightingPipeline();

	vkw::Framebuffer createRenderFramebuffer();
	vkw::Framebuffer createLightingFramebuffer();

	spirv::SetVector createCameraSets();
	CameraUBOV createCameraUBOs();
	SettingsUBOV createSettingsUBOs();

	// Transparency
	spirv::Shader createForwardShader();
	spirv::Pipeline createForwardPipeline();

	// Post process
	spirv::RenderPass createPostProcessRenderPass();
	vkw::FramebufferVector createPostProcessFramebuffers();

	// Inherited via ARenderer
	virtual spirv::SetSingleton* get_environmentSet() override;
};
} // namespace blaze
