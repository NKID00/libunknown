// 测试程序

#include "libunknown/libunknown.h"  // 包含 libunknown

#define WIN32_LEAN_AND_MEAN  // 只导入常用的 Win32 API
#include <Windows.h>

#include <iostream>
#include <locale>
#include <string>

int main()
{
    std::wcout.imbue(std::locale(std::locale(), "", LC_CTYPE));
    std::wcout << L"(M)essageBox: 显示帮助对话框" << std::endl;
    std::wcout << L"(A)boutBox: 显示关于对话框" << std::endl;
    std::wcout << L"(B)SOD: 蓝屏" << std::endl;
    std::wcout << L"(E)xit: 退出" << std::endl;

    while (true)
    {
        std::wcout << L">>> ";
        std::string s;
        std::cin >> s;
        switch (s[0])
        {
        case 'M':
        case 'm':
        {
            auto message_box = libunknown::MessageBox(L"👉文本内容👈", MB_OK | MB_ICONINFORMATION);
            message_box.SetCaption(L"👉标题👈");
            message_box.Show();
            std::cin >> s;
            break;
        }
        case 'A':
        case 'a':
        {
            libunknown::AboutBox(nullptr, L"👉标题👈", L"👉文本1👈", L"👉文本2👈", nullptr);
            break;
        }
        case 'B':
        case 'b':
        {
            libunknown::BSOD(0x233);
            break;
        }
        case 'E':
        case 'e':
        {
            return 0;
        }
        }
    }

    return 0;
}
