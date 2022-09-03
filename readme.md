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
