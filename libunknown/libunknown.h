#pragma once

#ifndef LIBUNKNOWN_HEADER_LIBU_HPP_
#define LIBUNKNOWN_HEADER_LIBU_HPP_

#define WIN32_LEAN_AND_MEAN  // 只导入常用的 Win32 API
#include <Windows.h>
#include <shellapi.h>

#undef MessageBox

#include <cstdint>
#include <string>
#include <cstring>
#include <atomic>
#include <thread>

#define LIBUNKNOWN_RETURN_IF_EQUAL(x, val, ret) do { if ((val) == (x)) { return (ret); } } while (false)
#define LIBUNKNOWN_RETURN_IF_NOT_EQUAL(x, val, ret) do { if ((val) != (x)) { return (ret); } } while (false)
#define LIBUNKNOWN_RETURN_VOID_IF_EQUAL(x, val) do { if ((val) == (x)) { return; } } while (false)
#define LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(x, val) do { if ((val) != (x)) { return; } } while (false)

namespace libunknown
{
    // 文本编码转换
    std::wstring ToWideChar(UINT code_page, const std::string& str)
    {
        size_t len = ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, nullptr, 0);
        if (!len)
        {
            return L"";
        }
        auto wstr_c = new wchar_t[len + 1];
        if (!wstr_c)
        {
            return L"";
        }
        std::memset(wstr_c, 0, len + 1);
        ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, wstr_c, len);
        std::wstring wstr(wstr_c);
        delete[] wstr_c;
        return wstr;
    }

    std::string WideCharTo(UINT code_page, const std::wstring& wstr)
    {
        size_t len = ::WideCharToMultiByte(code_page, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (!len)
        {
            return "";
        }
        auto str_c = new char[len + 1];
        if (!str_c)
        {
            return "";
        }
        std::memset(str_c, 0, len + 1);
        ::WideCharToMultiByte(code_page, 0, wstr.c_str(), -1, str_c, len, nullptr, nullptr);
        std::string str(str_c);
        delete[] str_c;
        return str;
    }

    std::wstring ANSIToWideChar(const std::string& ansi)
    {
        return ToWideChar(CP_ACP, ansi);
    }

    std::string WideCharToANSI(const std::wstring& wstr)
    {
        return WideCharTo(CP_ACP, wstr);
    }

    std::wstring UTF8ToWideChar(const std::string& utf8)
    {
        return ToWideChar(CP_UTF8, utf8);
    }

    std::string WideCharToUTF8(const std::wstring& wstr)
    {
        return WideCharTo(CP_UTF8, wstr);
    }

    // 显示消息对话框
    class MessageBox
    {
    public:
        enum class AlignX
        {
            Left,
            Middle,
            Right
        };

        enum class AlignY
        {
            Top,
            Middle,
            Bottom
        };

        MessageBox(
            const std::wstring& text, UINT options,
            HINSTANCE hInstance = nullptr, LPCWSTR lpszIcon = nullptr,
            DWORD_PTR dwContextHelpId = 0,
            MSGBOXCALLBACK lpfnMsgBoxCallback = nullptr,
            DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
        )
            : shown(true), return_value(-1)
        {
            // 结构体大小
            this->params.cbSize = sizeof(this->params);
            // 所有者窗口句柄
            this->params.hwndOwner = nullptr;
            // 包含自定义图标资源的示例
            this->params.hInstance = nullptr;
            // 显示文本
            this->params.lpszText = text.c_str();
            // 临时标题
            auto temp_caption = L"__libunknown_temp_caption_" + std::to_wstring(MessageBox::id);
            MessageBox::id++; // 递增临时标题编号
            auto temp_caption_c_str = temp_caption.c_str();
            this->params.lpszCaption = temp_caption_c_str;
            // 类型
            this->params.dwStyle = options;
            // 自定义图标资源的标识符
            this->params.lpszIcon = lpszIcon;
            // 帮助上下文
            this->params.dwContextHelpId = dwContextHelpId;
            // 帮助事件回调函数
            this->params.lpfnMsgBoxCallback = lpfnMsgBoxCallback;
            // 语言
            this->params.dwLanguageId = dwLanguageId;

            // 将被阻塞的线程
            this->thread = std::thread(&MessageBox::MessageBoxThreadEntryPoint, this);
            this->thread.detach();

            // 寻找对话框
            while (this->handle == nullptr)
            {
                this->handle = ::FindWindowW(nullptr, temp_caption_c_str);
            }

            this->Hide();
        }

        void SetOwner(HWND owner)
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);

#ifdef _WIN64
            ::SetWindowLongPtrW(this->handle, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(owner));
#else
            ::SetWindowLongW(this->handle, GWL_HWNDPARENT, reinterpret_cast<LONG>(owner));
#endif // _WIN64
        }

        void SetCaption(const std::wstring& caption)
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);

            ::SetWindowTextW(this->handle, caption.c_str());
        }

        void SetPosition(long x, long y, AlignX align_x, AlignY align_y)
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);

            // 获取高度宽度
            RECT r;
            ::GetWindowRect(this->handle, &r);
            auto w = r.right - r.left;
            auto h = r.bottom - r.top;

            // 计算对齐后的位置
            switch (align_x)
            {
            case AlignX::Left:
                break;
            case AlignX::Middle:
                x -= w / 2;
                break;
            case AlignX::Right:
                x -= w;
                break;
            }

            switch (align_y)
            {
            case AlignY::Top:
                break;
            case AlignY::Middle:
                y -= h / 2;
                break;
            case AlignY::Bottom:
                y -= h;
                break;
            }

            ::MoveWindow(this->handle, x, y, w, h, TRUE);
        }

        void Show()
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);

            if (this->shown)
            {
                return;
            }

            ::ShowWindow(this->handle, SW_SHOW);
            this->shown = true;
        }

        void Hide()
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);

            if (!this->shown)
            {
                return;
            }

            ::ShowWindow(this->handle, SW_HIDE);
            this->shown = false;
        }

        void Close()
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);
            ::SendMessageW(this->handle, WM_CLOSE, 0, 0);
        }

        HWND GetHandle()
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_IF_NOT_EQUAL(this->return_value, -1, nullptr);
            return this->handle;
        }

        int GetReturnValue()
        {
            return this->return_value;
        }

        ~MessageBox()
        {
            // 如果对话框已经关闭就直接返回
            LIBUNKNOWN_RETURN_VOID_IF_NOT_EQUAL(this->return_value, -1);
            ::SendMessageW(this->handle, WM_CLOSE, 0, 0);
        }

    private:
        bool shown;
        MSGBOXPARAMSW params;
        static std::atomic<std::uint32_t> id;
        std::thread thread;
        HWND handle;
        std::atomic<int> return_value;

        void MessageBoxThreadEntryPoint()
        {
            this->return_value = ::MessageBoxIndirectW(&this->params);
        }
    };
    std::atomic<std::uint32_t> MessageBox::id = 0;

    // 显示关于对话框
    void AboutBox(HWND owner, LPCWSTR title, LPCWSTR text1, LPCWSTR text2, HICON icon)
    {
        auto len_title = wcslen(title);
        auto len_text1 = wcslen(text1);
        auto app_text = new WCHAR[len_title + len_text1 + 2];
        wcscpy_s(app_text, len_title + 1, title);
        app_text[len_title] = L'#';
        wcscpy_s(app_text + len_title + 1, len_text1 + 1, text1);
        ShellAboutW(owner, app_text, text2, nullptr);
        delete[] app_text;
    }

    enum HARDERROR_RESPONSE_OPTION {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem,
        OptionOkNoWait,
        OptionCancelTryContinue
    };

    typedef LONG(WINAPI* ZwRaiseHardError_t)(LONG, ULONG, ULONG, PULONG_PTR, HARDERROR_RESPONSE_OPTION, PULONG);

    struct UNICODE_STRING {
        USHORT Length, MaximumLength;
        PWCH Buffer;
    };

    // 蓝屏
    void BSOD(long error_code)
    {
        // 提升到 Shutdown 权限
        while (true)
        {
            Sleep(10);
            HANDLE hToken;
            if (!OpenProcessToken(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken
            ))
            {
                continue;
            }
            LUID PrivilegeLUID;
            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &PrivilegeLUID);
            TOKEN_PRIVILEGES NewPrivilege;
            NewPrivilege.PrivilegeCount = 1;
            NewPrivilege.Privileges[0].Luid = PrivilegeLUID;
            NewPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (!AdjustTokenPrivileges(
                hToken, FALSE, &NewPrivilege, NULL, NULL, NULL
            ))
            {
                continue;
            }
            if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
            {
                continue;
            }
            break;
        }
        
        // 抛出硬件错误
        auto hDll = ::GetModuleHandle(TEXT("ntdll.dll"));
        auto ZwRaiseHardError = (ZwRaiseHardError_t)::GetProcAddress(hDll, "ZwRaiseHardError");
        UNICODE_STRING str = { 0, 0, NULL };
        ULONG x, args[] = { 0, 0, (ULONG)&str };
        while (true)
        {
            ZwRaiseHardError(error_code, 3, 4, args, OptionShutdownSystem, &x);
            Sleep(1000);
        }
    }
}

#endif // !LIBUNKNOWN_HEADER_LIBU_HPP_

