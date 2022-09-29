# DWM Overlay ...-1909 build Windows 10
## Improved from: [First](https://github.com/Rythorndoran/dwm-overlay) | [Second](https://github.com/armasm/dwmhook)

**Pattern:** 
```c++
auto dwRender = FindPattern("d2d1.dll", PBYTE("\x48\x8D\x05\x00\x00\x00\x00\x33\xED\x48\x8D\x71\x08"), "xxx????xxxxxx");
```

**Change overlay font:**
```c++
pFontFactory->CreateFontWrapper(pD3DXDevice, L"Arial", &pFontWrapper);
```
## You can use [Extreme Injector ](https://github.com/master131/ExtremeInjector/releases/tag/v3.7.3)

![injector](https://github.com/rlybasic/DWM_Hook/blob/main/img/inj.jpg)

![type](https://github.com/rlybasic/DWM_Hook/blob/main/img/type.jpg)

## Demonstration

![type](https://github.com/rlybasic/DWM_Hook/blob/main/img/overlay.jpg)

## To update pattern:

```c++
lea     rax, ??_7DrawingContext@@6BIMILRefCount@@@ ; const DrawingContext::`vftable'{for `IMILRefCount'}
xor     ebp, ebp
lea     rsi, [rcx+8]
mov     [rcx], rax
```
