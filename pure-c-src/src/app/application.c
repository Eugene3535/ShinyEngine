#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdio.h>
#include <cglm/struct/affine-pre.h>

#include "vulkan_api/context/context.h"
#include "vulkan_api/pipeline/graphics_pipeline.h"
#include "app/application.h"


static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

static bool init_vulkan(VulkanApp* app);
static void recreate_swapchain(VulkanApp* app);
static void update_uniform_buffer(VulkanApp* app, vec3s position, float angle);
static void write_command_buffer(VulkanApp* app, VkCommandBuffer cmd, VkDescriptorSet descriptorSet);
static void draw_frame(VulkanApp* app);


#define FPS_MEASUREMENT

// TODO remove magic numbers
static float lastX = 400;
static float lastY = 300;


// world space positions of our cubes
static const vec3s cubePositions[10] = 
{
    {  0.0f,  0.0f,  0.0f  },
    {  2.0f,  5.0f, -15.0f },
    { -1.5f, -2.2f, -2.5f  },
    { -3.8f, -2.0f, -12.3f },
    {  2.4f, -0.4f, -3.5f  },
    { -1.7f,  3.0f, -7.5f  },
    {  1.3f, -2.0f, -2.5f  },
    {  1.5f,  2.0f, -2.5f  },
    {  1.5f,  0.2f, -1.5f  },
    { -1.3f,  1.0f, -1.5f  }
};


bool VulkanApp_create(const char* title, int width, int height, VulkanApp* app)
{
	if(glfwInit() == GLFW_TRUE)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);

		if(window)
		{
			app->window = window;

			glfwSetWindowUserPointer(window, (void*)app);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
			glfwSetKeyCallback(window, key_callback);
			glfwSetCursorPosCallback(window, mouse_callback);

			glfwGetFramebufferSize(window, &app->width, &app->height);
			
			return init_vulkan(app);
		}
	}

	return false;
}


int VulkanApp_run(VulkanApp* app)
{
	GLFWwindow* window = app->window;
	Camera* camera = &app->camera;

	app->modelViewProjectionMatrix = glms_mat4_identity();
	Camera_init(camera);

	float deltaTime = 0.f;
	float lastFrame = 0.f;

#ifdef FPS_MEASUREMENT
    float timer = 0;
    int fps = 0;
#endif
    
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

#ifdef FPS_MEASUREMENT
        timer += deltaTime;
        ++fps;

        if (timer > 1.f)
        {
            printf("FPS: %i\n", fps);
            timer = 0;
            fps = 0;

        }
#endif

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        	glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			Camera_processKeyboard(camera, FORWARD, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			Camera_processKeyboard(camera, BACKWARD, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			Camera_processKeyboard(camera, LEFT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			Camera_processKeyboard(camera, RIGHT, deltaTime);

		draw_frame(app);

		glfwPollEvents();
	}

	vkDeviceWaitIdle(app->context.device);

	return 0;
}


void VulkanApp_destroy(VulkanApp* app)
{
	VkDevice device = app->context.device;

	BufferHolder_destroy(&app->bufferHolder, device);
	Texture2D_destroy(&app->texture, device);
	SyncManager_destroy(&app->sync, device);
	CommandBufferPool_destroy(&app->commandPool, device);
	DescriptorPool_destroy(&app->descriptorPool, device);
	GraphicsPipeline_destroy(&app->pipeline, device);

	MainView_destroy(&app->view);
	VulkanContext_destroy(&app->context);
	
	glfwDestroyWindow(app->window);
	glfwTerminate();
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	VulkanApp* app = (VulkanApp*)glfwGetWindowUserPointer(window);

	if(app)
	{
		app->width = width;
		app->height = height;
		app->framebufferResized = true;
	}
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	VulkanApp* app = (VulkanApp*)glfwGetWindowUserPointer(window);

	if(app)
	{
		float xpos = (float)xposIn;
		float ypos = (float)yposIn;

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		Camera_processMouseMovement(&app->camera, xoffset, yoffset);
	}
}


static bool init_vulkan(VulkanApp* app)
{
	if(!VulkanContext_create(&app->context))
		return false;

	app->view.context = &app->context;
	
	if(!MainView_createSurface(&app->view, app->window))
		return false;

	VkDevice device = app->context.device;

	{// Pipeline
		Shader shaders[] = 
		{
			Shader_loadFromFile(device, VK_SHADER_STAGE_VERTEX_BIT, "res/shaders/vertex_shader.spv"),
			Shader_loadFromFile(device, VK_SHADER_STAGE_FRAGMENT_BIT, "res/shaders/fragment_shader.spv")
		};

		if(!shaders[0].module || !shaders[1].module)
			return false;

        const VertexInputStateAttributeType attributes[] =
        {
            Float3,
            Float2
        };

        DescriptorSetLayout uniformDescriptors = {0};
        DescriptorSetLayout_addDescriptor(&uniformDescriptors, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

        GraphicsPipelineState pipelineState = {0};

        GraphicsPipelineState_setupShaderStages(&pipelineState, shaders, sizeof(shaders) / sizeof(Shader));
        GraphicsPipelineState_setupVertexInput(&pipelineState, attributes, sizeof(attributes) / sizeof(VertexInputStateAttributeType));
        GraphicsPipelineState_setupInputAssembler(&pipelineState, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        GraphicsPipelineState_setupViewport(&pipelineState);
        GraphicsPipelineState_setupRasterization(&pipelineState, VK_POLYGON_MODE_FILL);
        GraphicsPipelineState_setupMultisampling(&pipelineState);
        GraphicsPipelineState_setupColorBlending(&pipelineState, VK_FALSE);
        GraphicsPipelineState_setupDescriptorSetLayout(&pipelineState, &uniformDescriptors);

        bool result = GraphicsPipeline_create(&app->pipeline, &pipelineState, &app->view);
            
		free(uniformDescriptors.bindings);
		GraphicsPipelineState_release(&pipelineState);
		vkDestroyShaderModule(device, shaders[0].module, VK_NULL_HANDLE);
		vkDestroyShaderModule(device, shaders[1].module, VK_NULL_HANDLE);

		if(!result)
			return false;
	}

	{// Descriptors
		VkDescriptorPoolSize poolSizes = 
		{
			.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_FRAMES_IN_FLIGHT
		};

		if(!DescriptorPool_create(&app->descriptorPool, &poolSizes, 1, device))
			return false;

		VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = 
		{ 
			app->pipeline.descriptorSetLayout, 
			app->pipeline.descriptorSetLayout 
		};

		if(!DescriptorPool_allocateDescriptorSets(&app->descriptorPool, app->descriptorSets, layouts, device))
			return false;	
	}

	if(!CommandBufferPool_create(&app->commandPool, device, app->context.mainQueueFamilyIndex))
        return false;

	if(!SyncManager_create(&app->sync, device))
		return false;

	{
        if(!Texture2D_loadFromFile(&app->texture, "res/textures/container.jpg", &app->context, app->commandPool.handle))
            return false;
                
        const VkDescriptorImageInfo imageInfo = 
        {
            .sampler     = app->texture.sampler,
            .imageView   = app->texture.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

		DescriptorPool_writeCombinedImageSampler(&app->descriptorPool, &imageInfo, app->descriptorSets[0], 0, device);
		DescriptorPool_writeCombinedImageSampler(&app->descriptorPool, &imageInfo, app->descriptorSets[1], 0, device);
    }

	{
	    const float vertices[120] = 
        {
            -0.5f, -0.5f, 0.5f, 0.f, 0.f,
             0.5f, -0.5f, 0.5f, 1.f, 0.f,
             0.5f,  0.5f, 0.5f, 1.f, 1.f,
            -0.5f,  0.5f, 0.5f, 0.f, 1.f,

            -0.5f, -0.5f, -0.5f, 0.f, 0.f,
            -0.5f, -0.5f,  0.5f, 1.f, 0.f,
            -0.5f,  0.5f,  0.5f, 1.f, 1.f,
            -0.5f,  0.5f, -0.5f, 0.f, 1.f,

             0.5f, -0.5f,  0.5f, 0.f, 0.f,
             0.5f, -0.5f, -0.5f, 1.f, 0.f,
             0.5f,  0.5f, -0.5f, 1.f, 1.f,
             0.5f,  0.5f,  0.5f, 0.f, 1.f,

            -0.5f, -0.5f, -0.5f, 0.f, 0.f,
             0.5f, -0.5f, -0.5f, 1.f, 0.f,
             0.5f,  0.5f, -0.5f, 1.f, 1.f,
            -0.5f,  0.5f, -0.5f, 0.f, 1.f,

            -0.5f, 0.5f,  0.5f, 0.f, 0.f,
             0.5f, 0.5f,  0.5f, 1.f, 0.f,
             0.5f, 0.5f, -0.5f, 1.f, 1.f,
            -0.5f, 0.5f, -0.5f, 0.f, 1.f,

            -0.5f, -0.5f, -0.5f, 0.f, 0.f,
             0.5f, -0.5f, -0.5f, 1.f, 0.f,
             0.5f, -0.5f,  0.5f, 1.f, 1.f,
            -0.5f, -0.5f,  0.5f, 0.f, 1.f
        };

        const uint32_t indices[36] = 
        {
            0,  1,  2,  2,  3,  0,   // front
            4,  5,  6,  6,  7,  4,   // left
            8,  9,  10, 10, 11, 8,   // right
            12, 13, 14, 14, 15, 12,  // back
            16, 17, 18, 18, 19, 16,  // top
            20, 21, 22, 22, 23, 20   // bottom
        };

		app->vertices = BufferHolder_allocate(&app->bufferHolder, vertices, 120, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &app->context, app->commandPool.handle);
		app->indices = BufferHolder_allocate(&app->bufferHolder, indices, 36, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &app->context, app->commandPool.handle);

		if(!app->vertices.handle)
			return false;

		if(!app->indices.handle)
			return false;
	}

	return true;
}


void recreate_swapchain(VulkanApp* app)
{
    vkDeviceWaitIdle(app->context.device);
	MainView_recreate(&app->view, true);
}


void update_uniform_buffer(VulkanApp* app, vec3s position, float angle)
{
	vec3s axis = { 1.0f, 0.3f, 0.5f };

    mat4s model = glms_translate(glms_mat4_identity(), position);
    model       = glms_rotate(model, glm_rad(angle), axis);
    mat4s modelView  = Camera_getViewMatrix(&app->camera);
    mat4s projection = glms_perspective(glm_rad(60.f), app->width / (float)app->height, 0.1f, 100.f);
    projection.col[1].y *= -1;

    app->modelViewProjectionMatrix = glms_mat4_mul(glms_mat4_mul(projection, modelView), model);
}


void write_command_buffer(VulkanApp* app, VkCommandBuffer cmd, VkDescriptorSet descriptorSet)
{
    VkDeviceSize offsets[] = {0};
    VkBuffer vertexBuffers[] = {app->vertices.handle};

    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, app->indices.handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(cmd, app->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4s), app->modelViewProjectionMatrix.raw);
    vkCmdDrawIndexed(cmd, app->indices.size, 1, 0, 0, 0);
}


void draw_frame(VulkanApp* app)
{
    uint32_t frame  = app->sync.currentFrame;
    VkDevice device = app->context.device;
    VkQueue  queue  = app->context.queue;

    VkResult result = vkWaitForFences(device, 1, &app->sync.inFlightFences[frame], VK_TRUE, UINT64_MAX);

	if (result != VK_SUCCESS)
    {
#ifdef DEBUG
        printf("failed to wait for fences!\n");
#endif
		return;
    }

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(device, app->view.swapchain, UINT64_MAX, app->sync.imageAvailableSemaphores[frame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain(app);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
#ifdef DEBUG
        printf("failed to acquire swap chain image!\n");
#endif
		return;
    }

    result = vkResetFences(device, 1, &app->sync.inFlightFences[frame]);

	if (result != VK_SUCCESS)
    {
#ifdef DEBUG
        printf("failed to reset fences!\n");
#endif
		return;
    }

    VkCommandBuffer commandBuffer = app->commandPool.commandBuffers[frame];
    VkDescriptorSet descriptorSet = app->descriptorSets[frame];

    result = vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

	if (result != VK_SUCCESS)
    {
#ifdef DEBUG
        printf("failed to reset command buffers!\n");
#endif
		return;
    }

    if(!render_begin(commandBuffer, &app->view, imageIndex))
        return;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline.handle);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline.layout, 0, 1, &descriptorSet, 0, VK_NULL_HANDLE);

    for (uint32_t i = 0; i < 10; ++i)
    {
        const float angle = 20.f * i;
        update_uniform_buffer(app, cubePositions[i], angle);
        write_command_buffer(app, commandBuffer, descriptorSet);
    }

    if(!render_end(commandBuffer, &app->view, imageIndex))
        return;

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo submitInfo = 
	{
		.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext                = VK_NULL_HANDLE,
		.waitSemaphoreCount   = 1,
		.pWaitSemaphores      = app->sync.imageAvailableSemaphores + frame,
		.pWaitDstStageMask    = waitStages,
		.commandBufferCount   = 1,
		.pCommandBuffers      = &app->commandPool.commandBuffers[frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores    = &app->sync.renderFinishedSemaphores[frame]
	};

	result = vkQueueSubmit(queue, 1, &submitInfo, app->sync.inFlightFences[frame]);

    if (result != VK_SUCCESS)
    {
#ifdef DEBUG
        printf("failed to submit draw command buffer!\n");
#endif
		return;
    }

    const VkPresentInfoKHR presentInfo = 
	{
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext              = VK_NULL_HANDLE,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &app->sync.renderFinishedSemaphores[frame],
		.swapchainCount     = 1,
		.pSwapchains        = &app->view.swapchain,
		.pImageIndices      = &imageIndex,
		.pResults           = VK_NULL_HANDLE
	};

    result = vkQueuePresentKHR(queue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebufferResized)
    {
        app->framebufferResized = false;
        recreate_swapchain(app);
    }
    else if (result != VK_SUCCESS)
    {
#ifdef DEBUG
        printf("failed to present swap chain image!!\n");
#endif
		return;
    }

    app->sync.currentFrame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
}