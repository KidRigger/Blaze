
#include "Blaze.hpp"

#include <Datatypes.hpp>
#include <glm/glm.hpp>

#include <core/Camera.hpp>
#include <drawables/ModelLoader.hpp>
#include <rendering/FwdRenderer.hpp>
#include <util/Environment.hpp>
#include <util/files.hpp>

namespace blaze
{
// Constants
const int WIDTH = 1280;
const int HEIGHT = 720;
const bool FULLSCREEN = false;

#ifdef VALIDATION_LAYERS_ENABLED
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

bool firstMouse = true;
bool mouseEnabled = false;
double lastX = 0.0;
double lastY = 0.0;

double yaw = -90.0;
double pitch = 0.0;

glm::vec3 cameraFront{0.0f, 0.0f, 1.0f};

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	double sensitivity = mouseEnabled ? 0.05f : 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = static_cast<float>(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
	front.y = static_cast<float>(sin(glm::radians(pitch)));
	front.z = static_cast<float>(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
	cameraFront = glm::normalize(front);
}

void glfwErrorCallback(int error, const char* desc)
{
	std::cerr << "GLFW_ERROR: " << desc << std::endl;
}

struct SettingsOverlay
{
	bool rotate;
	float rotSpeed;
};

void runRefactored()
{
	// Usings
	using namespace std;

	// Variables
	GLFWwindow* window = nullptr;
	unique_ptr<ARenderer> renderer;
	unique_ptr<ModelLoader> modelLoader;

	glfwSetErrorCallback(glfwErrorCallback);

	Camera cam({3.0f, 3.0f, 3.0f}, {-0.5773f, -0.5773f, -0.5773f}, {0.0f, 1.0f, 0.0f}, glm::radians(45.0f),
			   (float)WIDTH / (float)HEIGHT, 1.0f, 30.0f);

	// GLFW Setup
	assert(glfwInit());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// glfwWindowHint(GLFW_CURSOR_HIDDEN, GLFW_TRUE);
	glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
	window =
		glfwCreateWindow(WIDTH, HEIGHT, VERSION.FULL_NAME, FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	assert(window != nullptr);

	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		mouse_callback(window, x, y);
	}

	// glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Renderer creation
	renderer = make_unique<FwdRenderer>(window, enableValidationLayers);
	renderer->set_camera(&cam);
	assert(renderer->complete());

	auto environment = make_unique<util::Environment>(
		renderer.get(), loadImageCube(renderer->get_context(), "assets/PaperMill_Ruins_E/PaperMill_E_3k.hdr", false));
	renderer->setEnvironment(environment.get());

	modelLoader = make_unique<ModelLoader>();
	struct SceneInfo
	{
		string modelName;
		int modelIndex;
	};

	struct LightsInfo
	{
		struct PointLights
		{
			glm::vec3 pos{0};
			float brightness{1};
			bool hasShadow{false};

			bool draw()
			{
				bool edited = false;
				edited |= ImGui::InputFloat3("Position##Position", &pos[0]);
				edited |= ImGui::InputFloat("Brightness##Brightness", &brightness);
				edited |= ImGui::Checkbox("Enable Shadow##Shadow", &hasShadow);
				return edited;
			}
		};

		std::vector<ALightCaster::Handle> pointHandles;
		PointLights editable;
		std::vector<PointLights> lights;

		int toDelete{-1};
		int toAdd{-1};
		int toUpdate{-1};

		void update(ARenderer* renderer)
		{
			if (toDelete >= 0)
			{
				lights.erase(lights.begin() + toDelete);
				renderer->get_lightCaster()->removeLight(pointHandles[toDelete]);
				pointHandles.erase(pointHandles.begin() + toDelete);
			}
			if (toAdd >= 0)
			{
				lights.push_back(editable);
				auto handle = renderer->get_lightCaster()->createPointLight(editable.pos, editable.brightness,
																			editable.hasShadow);
				pointHandles.push_back(handle);

				editable.brightness = 1.0f;
				editable.pos = glm::vec3{0.0f};
				editable.hasShadow = false;
			}
			if (toUpdate >= 0)
			{
				auto h = pointHandles[toUpdate];
				auto& l = lights[toUpdate];
				renderer->get_lightCaster()->setPosition(h, l.pos);
				renderer->get_lightCaster()->setBrightness(h, l.brightness);
				renderer->get_lightCaster()->setShadow(h, l.hasShadow);
			}
			toDelete = -1;
			toAdd = -1;
			toUpdate = -1;
		}
	} lightInfo;

	SceneInfo sceneInfo;
	sceneInfo.modelName = "DamagedHelmet";
	sceneInfo.modelIndex = 0;
	{
		auto& ms = modelLoader->getFileNames();
		for (auto& fn : ms)
		{
			if (fn == sceneInfo.modelName)
			{
				break;
			}
			sceneInfo.modelIndex++;
		}
	}

	int holderKey = 0;
	std::map<int, std::shared_ptr<Model2>> modelHolder;

	auto mod = modelHolder[holderKey++] =
		modelLoader->loadModel(renderer->get_context(), renderer->createMaterialSet(), sceneInfo.modelIndex);
	auto handle = renderer->submit(mod.get());

	// Run
	bool onetime = true;

	double prevTime = glfwGetTime();
	double deltaTime = 0.0;
	double elapsed = 0.0;

	SettingsOverlay settings;
	settings.rotate = false;
	settings.rotSpeed = 1.0f;

	while (!glfwWindowShouldClose(window))
	{
		prevTime = glfwGetTime();
		glfwPollEvents();
		elapsed += deltaTime;

		for (auto& [k, model] : modelHolder)
		{
			if (settings.rotate)
			{
				model->get_root()->rotation = glm::rotate(
					model->get_root()->rotation, settings.rotSpeed * static_cast<float>(deltaTime), glm::vec3(0, 1, 0));
			}
			model->update();
		}

		{
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetCursorPosCallback(window, nullptr);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				mouseEnabled = false;
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}

			GUI::startFrame();
			{
				if (ImGui::Begin("Settings"))
				{
					ImGui::Checkbox("Rotate##Model", &settings.rotate);
					ImGui::SameLine();
					ImGui::PushItemWidth(100);
					ImGui::InputFloat("Speed##Rotation", &settings.rotSpeed, 0.1f, 0.3f, 2);
					ImGui::PopItemWidth();

					bool quit = ImGui::Button("Exit");
					if (quit)
					{
						glfwSetWindowShouldClose(window, GLFW_TRUE);
					}
				}
				ImGui::End();

				if (ImGui::Begin("Camera"))
				{
					glm::vec3 val = cam.get_position();
					if (ImGui::InputFloat3("Position", &val[0]))
					{
						cam.moveTo(val);
					}
					val = cam.get_direction();
					if (ImGui::InputFloat3("Direction", &val[0]))
					{
						cam.lookTo(val);
					}
				}
				ImGui::End();

				if (ImGui::Begin("Scene"))
				{
					if (ImGui::CollapsingHeader("Models##InTheScene"))
					{
						auto& modelNames = modelLoader->getFileNames();
						if (ImGui::BeginCombo("Model##Combo", modelNames[sceneInfo.modelIndex].c_str()))
						{
							int i = 0;
							for (const auto& label : modelNames)
							{
								bool selected = (sceneInfo.modelIndex == i);
								if (ImGui::Selectable(label.c_str(), selected))
								{
									sceneInfo.modelIndex = i;
									sceneInfo.modelName = label;
									handle.destroy();
									auto mod = modelHolder[holderKey++] =
										modelLoader->loadModel(renderer->get_context(), renderer->createMaterialSet(),
															   sceneInfo.modelIndex); // TODO
									handle = renderer->submit(mod.get());
									renderer->waitIdle();
									modelHolder.erase(holderKey - 2);
									// TODO(Improvement) Make this smoother
								}
								if (selected)
								{
									ImGui::SetItemDefaultFocus();
								}
								++i;
							}
							ImGui::EndCombo();
						}
					}

					ImGui::Text("Num Lights %lu", lightInfo.lights.size());
					if (ImGui::CollapsingHeader("Lights"))
					{
						int idx = 0;
						for (auto& light : lightInfo.lights)
						{
							ImGui::PushID(idx);
							if (ImGui::TreeNode("Light##", "light %d", idx))
							{
								bool edited = light.draw();
								if (edited)
								{
									lightInfo.toUpdate = idx;
								}
								if (ImGui::Button("Remove"))
								{
									lightInfo.toDelete = idx;
								}
								ImGui::TreePop();
							}
							ImGui::PopID();
							ImGui::Separator();
							idx++;
						}
						ImGui::Text("New Light");
						ImGui::TreePush("new light");
						{
							ImGui::PushID("LightEditable");
							lightInfo.editable.draw();
							ImGui::PopID();
							if (ImGui::Button("Add"))
							{
								lightInfo.toAdd = 1;
							}
						}
						ImGui::TreePop();
						lightInfo.update(renderer.get());
					}
				}
				ImGui::End();
			}
			GUI::endFrame();

			renderer->render();

			deltaTime = glfwGetTime() - prevTime;
		}
	}
	renderer->waitIdle();
}
} // namespace blaze
