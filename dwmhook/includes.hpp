#pragma once

#include <Windows.h>
#include <cstdio>
#include <Psapi.h>
#include <process.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3d11_1.h>
#include <intrin.h>
#include <d2d1_1.h>
#include <d2d1_2.h>
#include <d2d1_3.h>
#include <d3d11_2.h>
#include <d2d1_3helper.h>
#include <dwrite_3.h>
#include <dcomp.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3dcompiler.h>

using namespace DirectX;

#pragma comment( lib, "dxgi" )
#pragma comment( lib, "d2d1" )
#pragma comment( lib, "d3d11" )
#pragma comment( lib, "dcomp" )
#pragma comment( lib, "dwrite" )
#ifdef _DEBUG
#pragma comment( lib, "minhook-debug.lib" )
#else
#pragma comment( lib, "minhook.lib" )
#endif
#pragma comment( lib, "d3dcompiler.lib" )

#define SAFE_RELEASE( p )	if( p ) { p->Release(); p = nullptr; }
#define LOG_FILE_PATH		"C:\\DWM_GAY.log"

#include "MinHook.h"
#include "save_state.hpp"