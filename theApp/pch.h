// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

#ifdef UNICODE
#define txt(x) L ## x
#define ScanTo  swscanf_s
#else
#define txt(x) x
#define ScanTo  sscanf_s
#endif // unicode

#define AddFlag(x, y) (x) |= (y)
#define DelFlag(x, y) (x) ^= (y)
#define IsFlag(x, y)  (bool)(((x) & (y)) == (y))

#define NewLine "\n"

#endif //PCH_H
