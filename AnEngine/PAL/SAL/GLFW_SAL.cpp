// Parent Header
#include "GLFW_SAL.hpp"



namespace SAL::GLFW
{
	bool CanClose(const ptr<Window> _window)
	{
		return glfwWindowShouldClose(_window);
	}

	ptr<Window> MakeWindow(int _width, int _height, ptr<const char> _title, ptr<Monitor> _fullscreenCast, ptr<Window> _windowToShareWith)
	{
		return glfwCreateWindow(_width, _height, _title, _fullscreenCast, _windowToShareWith);
	}

	void DestroyWindow(ptr<Window> _window)
	{
		glfwDestroyWindow(_window);
	}

	bool Initalize()
	{
		return glfwInit() ? true : false;
	}

	void PollEvents()
	{
		glfwPollEvents();

		return;
	}

	void Terminate()
	{
		glfwTerminate();
	}


	// Vulkan Related

	CStrArray GetRequiredVulkanAppExtensions(uint32& _extensionCount)
	{
		return glfwGetRequiredInstanceExtensions(getAddress(_extensionCount));
	}

	void WaitForEvents()
	{
		glfwWaitEvents();
	}

	// OS Related

	Where<OSAL::IsWindows, OSAL::PlatformTypes::
		OS_WindowHandle> GetOS_WindowHandle(const ptr<Window> _window)
	{
		return glfwGetWin32Window(_window);
	}

	void GetFramebufferSize(const ptr<Window> _window, int& _width, int& _height)
	{
		glfwGetFramebufferSize(_window, &_width, &_height);
	}

	// TODO: use glfw callback func type instead of making it..
	void SetWidnowSizeCallback(const ptr<Window> _window, void(*_callback)(ptr<Window>, int, int))
	{
		glfwSetWindowSizeCallback(_window, _callback);
	}
}