#pragma once

#include <string>
#include <vector>
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_NONE


// forward decl
struct GLFWwindow;
struct GLFWmonitor;


// consts
namespace glfw
{
	// TODO : add missing
	enum class Hint : int
	{
		  Resizable    = GLFW_RESIZABLE
		, Visible      = GLFW_VISIBLE
		, Decorated    = GLFW_DECORATED
		, Focused      = GLFW_FOCUSED
		, AutoIconify  = GLFW_AUTO_ICONIFY
		, Floating     = GLFW_FLOATING
		, Maximized    = GLFW_MAXIMIZED
		, CenterCursor = GLFW_CENTER_CURSOR

		, TransparentFramebuffer = GLFW_TRANSPARENT_FRAMEBUFFER

		, FocusOnShow    = GLFW_FOCUS_ON_SHOW
		, ScaleToMonitor = GLFW_SCALE_TO_MONITOR

		, RedBits     = GLFW_RED_BITS
		, GreenBits   = GLFW_GREEN_BITS
		, BlueBits    = GLFW_BLUE_BITS
		, AlphaBits   = GLFW_ALPHA_BITS
		, DepthBits   = GLFW_DEPTH_BITS
		, StencilBits = GLFW_STENCIL_BITS

		, Samples = GLFW_SAMPLES

		, RefreshRate = GLFW_REFRESH_RATE

		, Stereo       = GLFW_STEREO
		, SrgbCapable  = GLFW_SRGB_CAPABLE
		, DoubleBuffer = GLFW_DOUBLEBUFFER

		, ClientApi = GLFW_CLIENT_API

		, ContextCreationApi = GLFW_CONTEXT_CREATION_API 

		, ContextVersionMajor    = GLFW_CONTEXT_VERSION_MAJOR
		, ContextVersionMinor    = GLFW_CONTEXT_VERSION_MINOR

		, ContextRobustness      = GLFW_CONTEXT_ROBUSTNESS

		, ContextReleaseBehavior = GLFW_CONTEXT_RELEASE_BEHAVIOR

		, OpenGlForwardCompat    = GLFW_OPENGL_FORWARD_COMPAT
		, OpenGlDebugContext     = GLFW_OPENGL_DEBUG_CONTEXT
		, OpenGlProfile          = GLFW_OPENGL_PROFILE
	};

	enum class Value : int
	{
		  DontCare = GLFW_DONT_CARE

		, True  = GLFW_TRUE
		, False = GLFW_FALSE

		, OpenGlApi   = GLFW_OPENGL_API
		, OpenGlesApi = GLFW_OPENGL_ES_API
		, NoApi       = GLFW_NO_API

		, NativeContextApi = GLFW_NATIVE_CONTEXT_API 
		, EglContextApi    = GLFW_EGL_CONTEXT_API
		, OsMesaContextApi = GLFW_OSMESA_CONTEXT_API

		, NoRobustness        = GLFW_NO_ROBUSTNESS
		, NoResetNotification = GLFW_NO_RESET_NOTIFICATION
		, LoseContextOnReset  = GLFW_LOSE_CONTEXT_ON_RESET

		, AnyReleaseBehavior   = GLFW_ANY_RELEASE_BEHAVIOR
		, ReleaseBehaviorFlush = GLFW_RELEASE_BEHAVIOR_FLUSH
		, ReleaseBehaviorNone  = GLFW_RELEASE_BEHAVIOR_NONE

		, OpenGlAnyProfile    = GLFW_OPENGL_ANY_PROFILE
		, OpenGlCompatProfile = GLFW_OPENGL_COMPAT_PROFILE
		, OpenGlCoreProfile   = GLFW_OPENGL_CORE_PROFILE
	};

	using HintValue  = int;
	using HintInt    = std::pair<Hint, HintValue>;
	using HintString = std::pair<Hint, std::string>;

	struct CreationInfo
	{
		std::string title;

		int width{};
		int height{};

		std::vector<HintInt>    intHints;
		std::vector<HintString> strHints; 
	};
}


// utils
namespace glfw
{
	template<class T>
	struct Size
	{
		T width{};
		T height{};
	};

	template<class T>
	struct Vec2
	{
		T x{};
		T y{};
	};

	template<class T>
	struct Rect
	{
		T left{};
		T top{};
		T right{};
		T bottom{};
	};
}


// window
namespace glfw
{
	// base window class from which you should derive
	// forwards events from callbacks to class methods
	class Window
	{
	private:
		// common window callbacks used to forward events from callbacks to class

		// key input(when physical key was pressed)
		static void dispatch_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

		// text input(idk)
		static void dispatch_char_callback(GLFWwindow* window, unsigned int codepoint);

		// mouse input(mouse move)
		static void dispatch_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

		// mouse input(mouse button press/release)
		static void dispatch_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		// cursor enter/leave(cursor enters/leaves content area of a window)
		static void dispatch_cursor_enter_callback(GLFWwindow* window, int entered);

		// mouse input(scroll event)
		static void dispatch_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

		// drop(paths dropped on the window)
		static void dispatch_drop_callback(GLFWwindow* window, int pathCount, const char* paths[]);


		// window should close(close flag was set)
		static void dispatch_window_close_callback(GLFWwindow* window);

		// resize event
		static void dispatch_window_size_callback(GLFWwindow* window, int width, int height);

		// framebuffer resize
		static void dispatch_framebuffer_size_callback(GLFWwindow* window, int width, int height);

		// window pos(window position changed)
		static void dispatch_window_pos_callback(GLFWwindow* window, int xPos, int yPos);

		// window iconified(minimized)
		static void dispatch_window_iconify_callback(GLFWwindow* window, int iconified);

		// window maximized
		static void dispatch_window_maximize_callback(GLFWwindow* window, int maximized);

		// window focus(input focus)
		static void dispatch_window_focus_callback(GLFWwindow* window, int focused);

		// window damage/refresh
		static void dispatch_window_refresh_callback(GLFWwindow* window);

		// content scale changed
		static void dispatch_window_content_scale_callback(GLFWwindow* window, float xScale, float yScale);


	public:
		// creates empty window
		Window();

		Window(const CreationInfo& info, const Window& contextToShare = {});

		Window(const Window&) = delete;
		Window(Window&&)      = delete;

		virtual ~Window();

		Window& operator = (const Window&) = delete;
		Window& operator = (Window&&)      = delete;


	private:
		void connectDispatchCallbacks();


	public: // utilities
		void hide();

		void show();

		void gainFocus();

		void requestAttention();

		void makeContextCurrent();

		void swapBuffers();

		void iconify();

		void restore();

		void maximize();

		GLFWwindow* getGLFWwindow() const;

		// TODO : decide what to do
		void enterFullscreen(bool enter);


	public: // properties(get/set)
		bool shouldClose() const;

		void shouldClose(bool flag);


		Size<int> size() const;

		void size(int width, int height);


		Vec2<int> position() const;

		void position(int xPos, int yPos);


		float opacity() const;

		void opacity(float newOpacity);


		bool resizable() const;

		void resizable(bool flag);


		bool decorated() const;

		void decorated(bool flag);


		bool floating() const;

		void floating(bool flag);


		bool focusOnShow() const;

		void focusOnShow(bool flag);


		bool autoIconify() const;

		void autoIconify(bool flag);


	public: // properties(get)
		Rect<int> frameSize() const;

		Size<int> framebufferSize() const;

		Vec2<float> contentScale() const;

		bool focused() const;

		bool valid() const;

		bool visible() const;

		bool hovered() const;

		bool transparent() const;

		bool fullscreen() const;


	public: // properties(set)
		void title(const std::string& newTitle);

		void aspectRatio(int width, int height);

		void sizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight);


	public: // properties(TODO)
		void icon();

		void monitor();

		Value clientAPI();



	public: // events
		virtual void keyPressEvent(int key, int scancode, int action, int mods);

		virtual void charEvent(unsigned int codepoint);

		virtual void mouseMoveEvent(double xpos, double ypos);

		virtual void mouseButtonEvent(int button, int action, int mods);

		virtual void mouseEnterEvent(int entered);

		virtual void scrollEvent(double xOffset, double yOffset);

		virtual void dropEvent(int pathCount, const char* paths[]);


		virtual void closeEvent();

		virtual void resizeEvent(int width, int height);

		virtual void framebufferResizeEvent(int width, int height);

		virtual void moveEvent(int xPos, int yPos);

		virtual void minimizeEvent(int minimized);

		virtual void maximizeEvent(int maximized);

		virtual void focusEvent(int focused);

		virtual void refreshEvent();

		virtual void contentScaleEvent(float xScale, float yScale);


	private:
		GLFWwindow* m_window{nullptr};

		Size<int> m_sizeRestore{}; // used to restore window's size after exitin fullscreen mode
		Vec2<int> m_posRestore{}; // used to restore window's pos after exitin fullscreen 
	};
}


// general functions
namespace glfw
{
	bool initialize();

	void terminate();


	// reset window hints
	void set_default_window_hints();


	// event polling
	void poll_events();

	void wait_events();

	void wait_events_timeout(float waitSec);

	void post_empty_event();


	// swap interval
	void set_swap_interval(int interval);

	
	// error processing
	struct Error
	{
		int errorCode{};
		std::string errorString{};
	};

	Error get_error();

	// pointer to void(*)(int, const char*)
	using ErrorCallback = GLFWerrorfun;

	void set_error_callback(ErrorCallback callback);


	// time input
	double get_time();

	void set_time(double t);

	std::uint64_t get_timer_value();

	std::uint64_t get_timer_frequency();
}
