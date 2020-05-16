
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <rendering/ARenderer.hpp>
#include <vkwrap/VkWrap.hpp>

#include <spirv/PipelineFactory.hpp>
#include <core/VertexBuffer.hpp>

#include <core/Bindable.hpp>

#include <rendering/FwdLightCaster.hpp>

namespace blaze
{
/**
 * @brief Forward rendering specialization of ARenderer
 *
 * This class is able to setup a PBR/IBL forward renderer.
 */
class FwdRenderer final : public ARenderer
{
private:
	using CameraUBOV = UBOVector<CameraUBlock>;
	Texture2D depthBuffer;
	spirv::RenderPass renderPass;
	vkw::FramebufferVector renderFramebuffers;
	
	spirv::Shader shader;
	spirv::Pipeline pipeline;
	spirv::Shader skyboxShader;
	spirv::Pipeline skyboxPipeline;

	CameraUBOV cameraUBOs;
	spirv::SetVector cameraSets;

	const Bindable* environment;
	IndexedVertexBuffer<Vertex> skyboxCube;

	std::unique_ptr<FwdLightCaster> lightCaster;

public:
	/**
	 * @fn FwdRenderer()
	 *
	 * @brief Default empty constructor.
	 */
	FwdRenderer() noexcept
	{
	}

	/**
	 * @fn FwdRenderer(GLFWwindow* window, bool enableValidationLayers = true)
	 *
	 * @brief Actual constructor
	 *
	 * @param window The GLFW window pointer.
	 * @param enableValidationLayers Enable for Debugging.
	 */
	FwdRenderer(GLFWwindow* window, bool enableValidationLayers = true) noexcept;

	/**
	 * @name Constructors.
	 *
	 * @brief Pointer use only.
	 * @{
	 */
	FwdRenderer(FwdRenderer&& other) = delete;
	FwdRenderer& operator=(FwdRenderer&& other) = delete;
	FwdRenderer(const FwdRenderer& other) = delete;
	FwdRenderer& operator=(const FwdRenderer& other) = delete;
	/**
	 * @}
	 */

	// Inherited via ARenderer
	virtual spirv::SetSingleton createMaterialSet() override;
	virtual const spirv::Shader& get_shader() const override;

	~FwdRenderer()
	{
		clearCommandBuffers();
	}

protected:
	virtual void update(uint32_t frame) override;
	virtual void recordCommands(uint32_t frame) override;
	virtual void recreateSwapchainDependents() override;

private:
	spirv::RenderPass createRenderpass();
	Texture2D createDepthBuffer() const;
	vkw::FramebufferVector createFramebuffers() const;
	spirv::Shader createShader();
	spirv::Pipeline createPipeline();
    spirv::Shader createSkyboxShader();
    spirv::Pipeline createSkyboxPipeline();
	spirv::SetVector createCameraSets();
	CameraUBOV createCameraUBOs();
	spirv::SetVector createLightsDataSet();

	// Inherited via ARenderer
	virtual void setEnvironment(const Bindable* env) override;

	// Inherited via ARenderer
	virtual FwdLightCaster* get_lightCaster() override;
};
} // namespace blaze
