#pragma once

#include <DirectXMath.h>
using namespace DirectX;

extern ID3D11Device* pD3DXDevice;
extern ID3D11DeviceContext* pD3DXDeviceCtx;

#include "ck.h"
#include "FW1/FW1FontWrapper.h"
#include "save_state.hpp"

#define SAFE_RELEASE( p ) if( p ) { p->Release(); p = nullptr; }

static ULONGLONG t0 = 0;
static float accum = 0;

static TScopedHandle<ID3D11Texture2D*, COM_traits> backbuffer_ptr;
static TScopedHandle<ID3D11RenderTargetView*, COM_traits> rtview_ptr;
static TScopedHandle<ID3D11RasterizerState*, COM_traits> rasterizer_state;
static TScopedHandle<ID3D11RasterizerState*, COM_traits> rasterizer_state_ov;

static TScopedHandle<ID3D11VertexShader*, COM_traits> vertex_shader_ptr;
static TScopedHandle<ID3D11PixelShader*, COM_traits> pixel_shader_ptr;
static TScopedHandle<ID3D11InputLayout*, COM_traits> input_layout_ptr;
static TScopedHandle<ID3D11Buffer*, COM_traits> vertex_buffer_ptr;
static TScopedHandle<ID3D11Buffer*, COM_traits> index_buffer_ptr;
static TScopedHandle<ID3D11Buffer*, COM_traits> const_buffer_ptr;

#define STRINGIFY(X) #X

static const char* shader_code = STRINGIFY
(
	cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR0;
};
VS_OUTPUT VS( float4 Pos : POSITION, float4 Color : COLOR )
{
	VS_OUTPUT output = ( VS_OUTPUT )0;
	output.Pos = mul( Pos, World );
	output.Pos = mul( output.Pos, View );
	output.Pos = mul( output.Pos, Projection );
	output.Color = Color;
	return output;
}
float4 PS( VS_OUTPUT input ) : SV_Target
{
	return input.Color;
}
);

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

inline void fix_renderstate()
{
	auto t1 = GetTickCount64();
	float dt = ( t1 - t0 ) * 0.001f;
	accum += dt;
	t0 = t1;

	D3D11_TEXTURE2D_DESC bb;
	ZeroMemory( &bb, sizeof( bb ) );
	backbuffer_ptr->GetDesc( &bb );

	XMMATRIX world = XMMatrixRotationY( accum );
	XMVECTOR eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR at = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMMATRIX view = XMMatrixLookAtLH( eye, at, up );
	XMMATRIX proj = XMMatrixPerspectiveFovLH( XM_PIDIV2, bb.Width / ( FLOAT )bb.Height, 0.01f, 100.0f );

	//float color[4] = { 0, 0, 0, 0 };
	//device_context_ptr->ClearRenderTargetView(*rtview_ptr, color);
	pD3DXDeviceCtx->OMSetRenderTargets( 1, &rtview_ptr, NULL );

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, ( float )bb.Width, ( float )bb.Height, 0.0f, 1.0f };
	pD3DXDeviceCtx->RSSetViewports( 1, &viewport );
	pD3DXDeviceCtx->RSSetState( *rasterizer_state_ov );

	pD3DXDeviceCtx->IASetInputLayout( *input_layout_ptr );
	pD3DXDeviceCtx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT stride = sizeof( SimpleVertex ), offset = 0;
	pD3DXDeviceCtx->IASetVertexBuffers( 0, 1, &vertex_buffer_ptr, &stride, &offset );
	pD3DXDeviceCtx->IASetIndexBuffer( *index_buffer_ptr, DXGI_FORMAT_R16_UINT, 0 );

	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose( world );
	cb.mView = XMMatrixTranspose( view );
	cb.mProjection = XMMatrixTranspose( proj );
	pD3DXDeviceCtx->UpdateSubresource( *const_buffer_ptr, 0, NULL, &cb, 0, 0 );

	pD3DXDeviceCtx->VSSetShader( *vertex_shader_ptr, NULL, 0 );
	pD3DXDeviceCtx->VSSetConstantBuffers( 0, 1, &const_buffer_ptr );
	pD3DXDeviceCtx->PSSetShader( *pixel_shader_ptr, NULL, 0 );
	//pD3DXDeviceCtx->DrawIndexed( 36, 0, 0 );

	pD3DXDeviceCtx->RSSetState( *rasterizer_state );
}

struct COLOR_VERTEX
{
	XMFLOAT3	Position;
	XMFLOAT4	Color;
};

class CRender2D
{
	ID3D11InputLayout* mInputLayout = nullptr;
	ID3D11Buffer* mVertexBuffer = nullptr;
	ID3D11VertexShader* mVS = nullptr;
	ID3D11PixelShader* mPS = nullptr;
	ID3D11BlendState* transparency = nullptr;
	D3D11StateSaver* stateSaver = new D3D11StateSaver();
	IFW1FontWrapper* font;

	bool restoreState = false;
public:
	CRender2D() = default;
	~CRender2D() = default;

	bool Initialize( IFW1FontWrapper* pFontWrapper );
	void BeginScene();
	void EndScene();
	void OnReset();

	void FillRect( float x, float y, float w, float h, ULONG color );
	void OutlineRect( float x1, float y1, float x2, float y2, ULONG color );
	void DrawLine( const XMFLOAT2& point_1, const XMFLOAT2& point_2, ULONG color );
	void DrawHealthBar( float x, float y, float w, float health, float max );
	void DrawHealthBar( float x, float y, float w, float h, float health, float max );

	void RenderText( const wchar_t* txt, float x, float y, ULONG color, bool center, bool shadow = true );
	void RenderText( const char* txt, float x, float y, ULONG color, bool center, bool shadow = true );
};

extern CRender2D render;

