<h1>
    <p align=center>
        libunknown
    </p>
</h1>

> 一些有趣的东西整合成的库。

## 特性

- **方便安全**的文本编码转换函数

- 方便“危险”的蓝屏函数

- 在**任意特定位置**弹出任意消息对话框

- 弹出关于对话框

## 使用

```cpp
#include "libunknown/libunknown.h"
```

具体用法请参见 `test.cpp` 和 `libunknown/libunknown.h`。

## 说明

这玩意主要是用来方便我（NKID00）自己写一些弹很多不同位置的窗的表演程序用的。

这玩意的主体主要源于网课期间我（NKID00）自己编写的 [DongLib](https://github.com/NKID00/toys)，（暂时）去除了播放音频的功能并修改了一些其他的东西。

有问题的话欢迎提交 Issue 或者 PR。

## 已知问题

- 弹出来的消息对话框很丑。

  - 不知道为啥C++（Win32API）调用 MessageBoxW 或者 MessageBoxIndirectW 弹出的消息对话框就会很丑。
  
    - 调用 C#（.NET）的 System.Windows.Forms.MessageBox 弹出的消息对话框就很挺好看。

      - 可能 .NET 底层用了什么别的东西。

- 蓝屏函数只会弹出错误对话框，无法正常蓝屏。

## 版权

版权所有 © NKID00 2020

使用 [MIT License](./LICENSE) 进行许可。

提示：**NKID00 对这个库的使用造成的任何后果不负任何责任。**
