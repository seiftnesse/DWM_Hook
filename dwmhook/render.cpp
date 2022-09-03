#include "includes.hpp"
#include "render.hpp"

CRender2D render;

static char D3D11FillShader[] =
"struct VSOut"
"{"
"	float4 Col : COLOR;"
"	float4 Pos : SV_POSITION;"
"};"

"VSOut VS(float4 Col : COLOR, float4 Pos : POSITION)"
"{"
"	VSOut Output;"
"	Output.Pos = Pos;"
"	Output.Col = Col;"
"	return Output;"
"}"

"float4 PS(float4 Col : COLOR) : SV_TARGET"
"{"
"	return Col;"
"}";

bool CRender2D::Initialize( IFW1FontWrapper* pFontWrapper )
{
	HRESULT hr;

	if ( !pD3DXDevice )
		return false;

	if ( !pD3DXDeviceCtx )
		return false;

	if ( pFontWrapper )
		this->font = pFontWrapper;

	ID3DBlob* VS, * PS;
	hr = D3DCompile( D3D11FillShader, sizeof( D3D11FillShader ), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &VS, NULL );
	if ( FAILED( hr ) )
		return false;

	hr = pD3DXDevice->CreateVertexShader( VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &this->mVS );
	if ( FAILED( hr ) )
	{
		SAFE_RELEASE( VS );
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = pD3DXDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), VS->GetBufferPointer(), VS->GetBufferSize(), &this->mInputLayout );
	SAFE_RELEASE( VS );
	if ( FAILED( hr ) )
		return false;

	D3DCompile( D3D11FillShader, sizeof( D3D11FillShader ), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, &PS, NULL );
	if ( FAILED( hr ) )
		return false;

	hr = pD3DXDevice->CreatePixelShader( PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &this->mPS );
	if ( FAILED( hr ) )
	{
		SAFE_RELEASE( PS );
		return false;
	}

	D3D11_BUFFER_DESC bufferDesc{ };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = 4 * sizeof( COLOR_VERTEX );
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	hr = pD3DXDevice->CreateBuffer( &bufferDesc, NULL, &this->mVertexBuffer );
	if ( FAILED( hr ) )
		return false;

	D3D11_BLEND_DESC blendStateDescription{ };
	blendStateDescription.RenderTarget[ 0 ].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[ 0 ].RenderTargetWriteMask = 0x0f;

	hr = pD3DXDevice->CreateBlendState( &blendStateDescription, &this->transparency );
	if ( FAILED( hr ) )
		return false;

	return true;
}

void CRender2D::FillRect( float x, float y, float w, float h, ULONG color )
{
	if ( pD3DXDeviceCtx == NULL )
		return;

	auto a = ( ( color >> 24 ) & 255 );
	auto r = ( ( color >> 16 ) & 255 );
	auto g = ( ( color >> 12 ) & 255 );
	auto b = ( color & 255 );

	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	pD3DXDeviceCtx->RSGetViewports( &viewportNumber, &vp );

	float x0 = x;
	float y0 = y;
	float x1 = x + w;
	float y1 = y + h;

	float xx0 = 2.0f * ( x0 - 0.5f ) / vp.Width - 1.0f;
	float yy0 = 1.0f - 2.0f * ( y0 - 0.5f ) / vp.Height;
	float xx1 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f;
	float yy1 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height;

	COLOR_VERTEX* v = NULL;
	D3D11_MAPPED_SUBRESOURCE mapData;

	if ( FAILED( pD3DXDeviceCtx->Map( this->mVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData ) ) )
		return;

	v = ( COLOR_VERTEX* )mapData.pData;

	v[ 0 ].Position.x = ( float )x0;
	v[ 0 ].Position.y = ( float )y0;
	v[ 0 ].Position.z = 0;
	v[ 0 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 1 ].Position.x = ( float )x1;
	v[ 1 ].Position.y = ( float )y1;
	v[ 1 ].Position.z = 0;
	v[ 1 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 0 ].Position.x = xx0;
	v[ 0 ].Position.y = yy0;
	v[ 0 ].Position.z = 0;
	v[ 0 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 1 ].Position.x = xx1;
	v[ 1 ].Position.y = yy0;
	v[ 1 ].Position.z = 0;
	v[ 1 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 2 ].Position.x = xx0;
	v[ 2 ].Position.y = yy1;
	v[ 2 ].Position.z = 0;
	v[ 2 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 3 ].Position.x = xx1;
	v[ 3 ].Position.y = yy1;
	v[ 3 ].Position.z = 0;
	v[ 3 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	pD3DXDeviceCtx->Unmap( this->mVertexBuffer, NULL );

	UINT Stride = sizeof( COLOR_VERTEX );
	UINT Offset = 0;

	pD3DXDeviceCtx->IASetVertexBuffers( 0, 1, &this->mVertexBuffer, &Stride, &Offset );
	pD3DXDeviceCtx->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	pD3DXDeviceCtx->IASetInputLayout( this->mInputLayout );

	pD3DXDeviceCtx->VSSetShader( this->mVS, 0, 0 );
	pD3DXDeviceCtx->PSSetShader( this->mPS, 0, 0 );
	pD3DXDeviceCtx->GSSetShader( NULL, 0, 0 );
	pD3DXDeviceCtx->Draw( 4, 0 );
}

void CRender2D::OutlineRect( float x, float y, float w, float h, ULONG color )
{
	FillRect( x, ( y + h - 1 ), w, 1, color );
	FillRect( x, y, 1, h, color );
	FillRect( x, y, w, 1, color );
	FillRect( ( x + w - 1 ), y, 1, h, color );
}

void CRender2D::DrawLine( const XMFLOAT2& point_1, const XMFLOAT2& point_2, ULONG color )
{
	if ( pD3DXDeviceCtx == NULL )
		return;

	auto a = ( ( color >> 24 ) & 255 );
	auto r = ( ( color >> 16 ) & 255 );
	auto g = ( ( color >> 12 ) & 255 );
	auto b = ( color & 255 );

	float x1 = point_1.x;
	float x2 = point_2.x;

	float y1 = point_1.y;
	float y2 = point_2.y;

	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	pD3DXDeviceCtx->RSGetViewports( &viewportNumber, &vp );

	float xx0 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f;
	float yy0 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height;
	float xx1 = 2.0f * ( x2 - 0.5f ) / vp.Width - 1.0f;
	float yy1 = 1.0f - 2.0f * ( y2 - 0.5f ) / vp.Height;

	COLOR_VERTEX* v = NULL;

	D3D11_MAPPED_SUBRESOURCE mapData;

	if ( FAILED( pD3DXDeviceCtx->Map( this->mVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData ) ) )
		return;

	v = ( COLOR_VERTEX* )mapData.pData;

	v[ 0 ].Position.x = xx0;
	v[ 0 ].Position.y = yy0;

	v[ 0 ].Position.z = 0;
	v[ 0 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	v[ 1 ].Position.x = xx1;
	v[ 1 ].Position.y = yy1;
	v[ 1 ].Position.z = 0;
	v[ 1 ].Color = XMFLOAT4(
		( ( float )r / 255.0f ),
		( ( float )g / 255.0f ),
		( ( float )b / 255.0f ),
		( ( float )a / 255.0f ) );

	pD3DXDeviceCtx->Unmap( this->mVertexBuffer, NULL );

	UINT Stride = sizeof( COLOR_VERTEX );
	UINT Offset = 0;

	pD3DXDeviceCtx->IASetVertexBuffers( 0, 1, &this->mVertexBuffer, &Stride, &Offset );
	pD3DXDeviceCtx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP );
	pD3DXDeviceCtx->IASetInputLayout( this->mInputLayout );

	pD3DXDeviceCtx->VSSetShader( this->mVS, 0, 0 );
	pD3DXDeviceCtx->PSSetShader( this->mPS, 0, 0 );
	pD3DXDeviceCtx->GSSetShader( NULL, 0, 0 );
	pD3DXDeviceCtx->Draw( 2, 0 );
}

void CRender2D::DrawHealthBar( float x, float y, float w, float health, float max )
{
	this->DrawHealthBar( x, y, w, 2, health, max );
}

void CRender2D::DrawHealthBar( float x, float y, float w, float h, float health, float max )
{
	/*if ( !max )
		return;

	if ( w < 5 )
		return;

	if ( health < 0 )
		health = 0;

	float ratio = health / max;

	ULONG col = ULONG( 0, ( BYTE )( 255 - 255 * ratio ), ( BYTE )( 255 * ratio ), 255 );

	float step = ( w / max );
	float draw = ( step * health );

	FillRect( x, y, w, h, ULONG( 0, 0, 0, 255 ) );
	FillRect( x, y, draw, h, col );*/
}

void CRender2D::RenderText( const wchar_t* txt, float x, float y, ULONG color, bool center, bool shadow )
{
	if ( !this->font )
		return;

	if ( center )
	{
		FW1_RECTF nullRect = { 0.f, 0.f, 0.f, 0.f };
		FW1_RECTF rect = this->font->MeasureString( txt, L"Arial", 17.0f, &nullRect, FW1_NOWORDWRAP );

		auto v = XMFLOAT2{ rect.Right, rect.Bottom };

		x -= v.x / 2.f;
	}

	if ( shadow )
	{
		this->font->DrawString( pD3DXDeviceCtx, txt, 17.0f, x - 1, y, 0xFF000000, FW1_RESTORESTATE );
		this->font->DrawString( pD3DXDeviceCtx, txt, 17.0f, x + 1, y, 0xFF000000, FW1_RESTORESTATE );
		this->font->DrawString( pD3DXDeviceCtx, txt, 17.0f, x, y - 1, 0xFF000000, FW1_RESTORESTATE );
		this->font->DrawString( pD3DXDeviceCtx, txt, 17.0f, x, y + 1, 0xFF000000, FW1_RESTORESTATE );
	}

	this->font->DrawString( pD3DXDeviceCtx, txt, 17.0f, x, y, color, FW1_RESTORESTATE );
}

void CRender2D::RenderText( const char* txt, float x, float y, ULONG color, bool center, bool shadow )
{
	auto cb = MultiByteToWideChar( CP_UTF8, 0, txt, -1, nullptr, NULL );

	wchar_t* wsText = new wchar_t[ cb + sizeof( wchar_t ) ];
	MultiByteToWideChar( CP_UTF8, 0, txt, -1, wsText, cb );

	this->RenderText( wsText, x, y, color, center, shadow );

	delete[] wsText;
}

void CRender2D::BeginScene()
{
	this->restoreState = false;

	if ( SUCCEEDED( this->stateSaver->saveCurrentState( pD3DXDeviceCtx ) ) )
		this->restoreState = true;

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pD3DXDeviceCtx->OMSetBlendState( this->transparency, blendFactor, 0xffffffff );
	pD3DXDeviceCtx->IASetInputLayout( this->mInputLayout );
}

void CRender2D::EndScene()
{
	if ( this->restoreState )
		this->stateSaver->restoreSavedState();
}

void CRender2D::OnReset()
{
	SAFE_RELEASE( this->mVS );
	SAFE_RELEASE( this->mPS );
	SAFE_RELEASE( this->transparency );
	SAFE_RELEASE( this->mInputLayout );
	SAFE_RELEASE( this->mVertexBuffer );

	restoreState = false;
}