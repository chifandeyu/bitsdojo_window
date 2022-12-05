#include "include/bitsdojo_window_windows/bitsdojo_window_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include "./bitsdojo_window_api.h"

#define ID_TIMER_FULL_SCREEN 2    
const UINT interval = 2000; //ms

const char kChannelName[] = "bitsdojo/window";
const auto bdwAPI = bitsdojo_window_api();

std::unique_ptr<flutter::MethodChannel<>> bitsdojo_window_channel;

namespace
{
    enum WINDOW_STATE
    {
        STATE_NORMAL = 0,
        STATE_MAXIMIZED,
        STATE_MINIMIZED,
        STATE_FULLSCREEN_ENTERED,
    };

    class BitsdojoWindowPlugin : public flutter::Plugin
    {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

        BitsdojoWindowPlugin(
            flutter::PluginRegistrarWindows *registrar,
            std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel);

        virtual ~BitsdojoWindowPlugin();

    private:
        // Called when a method is called on this plugin's channel from Dart.
        void HandleMethodCall(
            const flutter::MethodCall<flutter::EncodableValue> &method_call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
        
        // Called for top-level WindowProc delegation.
        std::optional<LRESULT> HandleWindowProc(HWND hwnd, UINT message,
            WPARAM wparam, LPARAM lparam);

        // The ID of the WindowProc delegate registration.
        int window_proc_id_ = -1;

        // Event for the window notification
        void _EmitEvent(std::string eventName);

        void _HideCursor();

        void _ShowCursor();

        void _ResetShowCursor(HWND hwnd);

        // The registrar for this plugin.
        flutter::PluginRegistrarWindows *registrar_;

        // The cannel to send menu item activations on.
        std::unique_ptr<flutter::MethodChannel<>> channel_;
        BOOL _bHideCursor = FALSE;
        WINDOW_STATE _lastState = STATE_NORMAL;
    };

    // static
    void BitsdojoWindowPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarWindows *registrar)
    {
        auto channel =
            std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), kChannelName,
                &flutter::StandardMethodCodec::GetInstance());

        auto *channel_pointer = channel.get();

        auto plugin = std::make_unique<BitsdojoWindowPlugin>(registrar, std::move(channel));

        channel_pointer->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto &call, auto result) {
                plugin_pointer->HandleMethodCall(call, std::move(result));
            });

        registrar->AddPlugin(std::move(plugin));
    }

    BitsdojoWindowPlugin::BitsdojoWindowPlugin(
        flutter::PluginRegistrarWindows *registrar,
        std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel) : registrar_(registrar), channel_(std::move(channel))
    {
        window_proc_id_ = registrar_->RegisterTopLevelWindowProcDelegate(
            [this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
                return this->HandleWindowProc(hwnd, message, wparam, lparam);
        });
    }

    BitsdojoWindowPlugin::~BitsdojoWindowPlugin()
    {
        std::cout << "==== ~BitsdojoWindowPlugin()" << std::endl;
        registrar_->UnregisterTopLevelWindowProcDelegate(window_proc_id_);
    }

    void BitsdojoWindowPlugin::HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
    {        
        if (method_call.method_name().compare("dragAppWindow") == 0)
        {
            bool callResult = bdwAPI->privateAPI->dragAppWindow();
            if (callResult) {
                result->Success();
            } else {
                result->Error("ERROR_DRAG_APP_WINDOW_FAILED","Could not drag app window");
            }
        }
        else
        {
            result->NotImplemented();
        }
    }

    std::optional<LRESULT> BitsdojoWindowPlugin::HandleWindowProc(HWND hwnd,
        UINT message,
        WPARAM wparam,
        LPARAM lparam)
    {
        std::optional<LRESULT> result;
        switch (message) {
        case WM_SIZE:
        {
            //LONG_PTR gwlStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (_lastState != STATE_MAXIMIZED &&
                wparam == SIZE_MAXIMIZED) {
                _EmitEvent("maximize");
                _lastState = STATE_MAXIMIZED;
            }
            else if (wparam == SIZE_MINIMIZED) {
                _EmitEvent("minimize");
                _lastState = STATE_MINIMIZED;
            }
            else if (wparam == SIZE_RESTORED) {
                if (_lastState == STATE_MAXIMIZED) {
                    _EmitEvent("unmaximize");
                    _lastState = STATE_NORMAL;
                }
                else if (_lastState == STATE_MINIMIZED) {
                    _EmitEvent("restore");
                    _lastState = STATE_NORMAL;
                }
            }
        }
          break;
        case WM_SHOWFULLSCREEN_FLUTTER_WINDOW:
        {
            bool isFullScreen = (bool)wparam;
            if (isFullScreen) {
                _EmitEvent("enter-full-screen");
                _lastState = STATE_FULLSCREEN_ENTERED;
                //重置鼠标光标显示
                _ResetShowCursor(hwnd);
                //std::cout << "====== enter-full-screen Reset ShowCursor" << std::endl;
            }
            else {
                _EmitEvent("leave-full-screen");
                _lastState = STATE_NORMAL;
                //取消全屏立即显示鼠标光标，停止计时
                KillTimer(hwnd, ID_TIMER_FULL_SCREEN);
                _ShowCursor();
            }
        }
           break;
        case WM_MOUSEMOVE_FLUTTER_WINDOW:
        {
            if (_lastState == STATE_FULLSCREEN_ENTERED) {
                //std::cout << "==== mouse move Reset ShowCursor" << std::endl;
                _ResetShowCursor(hwnd);
            }
        }
          break;
        case WM_TIMER:
        {
            if (wparam == ID_TIMER_FULL_SCREEN) {
                KillTimer(hwnd, ID_TIMER_FULL_SCREEN);
                if (_lastState == STATE_FULLSCREEN_ENTERED) {
                    //to hide mouse pointer
                    _HideCursor();
                }
                else {
                    _ShowCursor();
                }
            }
        }
          break;
	    }
        return result;
    }

    void BitsdojoWindowPlugin::_EmitEvent(std::string eventName)
    {
        flutter::EncodableMap args = flutter::EncodableMap();
        args[flutter::EncodableValue("eventName")] =
            flutter::EncodableValue(eventName);
        //send event to MethodChannelBitsdojoWindow
        channel_->InvokeMethod("onEvent",
            std::make_unique<flutter::EncodableValue>(args));
    }

    void BitsdojoWindowPlugin::_HideCursor()
    {
        if (!_bHideCursor) {
            while (::ShowCursor(FALSE) >= 0)
            {
                ::ShowCursor(FALSE); //隐藏光标
            }
            _bHideCursor = TRUE;
            _EmitEvent("hide-cursor");
            //std::cout << "==== Hide Cursor" << std::endl;
        }
    }

    void BitsdojoWindowPlugin::_ShowCursor()
    {
        if (_bHideCursor) {
            while (ShowCursor(TRUE) < 0) {
                ShowCursor(TRUE); //显示光标
            }
            _bHideCursor = FALSE;
            _EmitEvent("show-cursor");
            //std::cout << "==== Show Cursor" << std::endl;
        }
    }

    void BitsdojoWindowPlugin::_ResetShowCursor(HWND hwnd)
    {
        KillTimer(hwnd, ID_TIMER_FULL_SCREEN);
        SetTimer(hwnd, ID_TIMER_FULL_SCREEN, interval, NULL);
        _ShowCursor();
    }

} // namespace

void BitsdojoWindowPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
    BitsdojoWindowPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
