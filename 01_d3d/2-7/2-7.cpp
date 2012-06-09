#include <d3d9.h>
#include <d3dx9.h>
//------------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D        = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice  = NULL;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuff = NULL;
LPDIRECT3DINDEXBUFFER9  g_pIndexBuff  = NULL;

//------------------------------------------------------------------------------
struct CUSTOMVERTEX
{
    float x, y, z;
    DWORD color;
};

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

struct CUSTOMINDEX
{
    WORD _0, _1, _2;
};
//------------------------------------------------------------------------------
VOID Cleanup()
{
    if ( g_pIndexBuff )  { g_pIndexBuff->Release();  g_pIndexBuff = NULL; }
    if ( g_pVertexBuff ) { g_pVertexBuff->Release(); g_pVertexBuff = NULL; }
    if ( g_pd3dDevice )  { g_pd3dDevice->Release();  g_pd3dDevice = NULL; }
    if ( g_pD3D )        { g_pD3D->Release();        g_pD3D = NULL; }
}

HRESULT InitD3D( HWND hWnd )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    if ( !g_pD3D ) return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = true;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    if ( FAILED( g_pD3D->CreateDevice( 
                    D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL,
                    hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );

    return S_OK;
}
//------------------------------------------------------------------------------
HRESULT init_vertex_buffer()
{
    CUSTOMVERTEX vertices[] = {
		{ -1,  1,  1 , 0xFFFF0000 },		/// v0
		{  1,  1,  1 , 0xFF00FF00 },		/// v1
		{  1,  1, -1 , 0xFF0000FF },		/// v2
		{ -1,  1, -1 , 0xFFFFFF00 },		/// v3

		{ -1, -1,  1 , 0xFF00FFFF },		/// v4
		{  1, -1,  1 , 0xFFFF00FF },		/// v5
		{  1, -1, -1 , 0xFF000000 },		/// v6
		{ -1, -1, -1 , 0xFFFFFFFF },		/// v7
    };

    if (FAILED( g_pd3dDevice->CreateVertexBuffer( 8*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVertexBuff, NULL )))
        return E_FAIL;

    VOID* pVertices;
    if (FAILED( g_pVertexBuff->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 )))
        return E_FAIL;
    memcpy( pVertices, vertices, sizeof(vertices) );
    g_pVertexBuff->Unlock();

    return S_OK;
}


HRESULT init_index_buffer()
{
    CUSTOMINDEX	indices[] = {
		{ 0, 1, 2 }, { 0, 2, 3 },	/// À­¸é
		{ 4, 6, 5 }, { 4, 7, 6 },	/// ¾Æ·§¸é
		{ 0, 3, 7 }, { 0, 7, 4 },	/// ¿Þ¸é
		{ 1, 5, 6 }, { 1, 6, 2 },	/// ¿À¸¥¸é
		{ 3, 2, 6 }, { 3, 6, 7 },	/// ¾Õ¸é
		{ 0, 4, 5 }, { 0, 5, 1 }	/// µÞ¸é
    };

    if (FAILED( g_pd3dDevice->CreateIndexBuffer( 12 * sizeof(CUSTOMINDEX), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIndexBuff, NULL )))
        return E_FAIL;

    VOID* pIndices;
    if (FAILED( g_pIndexBuff->Lock( 0, sizeof(indices), (void**)&pIndices, 0 )))
        return E_FAIL;
    memcpy( pIndices, indices, sizeof(indices) );
    g_pIndexBuff->Unlock();

    return S_OK;
}

VOID SetupMatrices()
{
    // world
    D3DXMATRIXA16 world_matrix;

    D3DXMatrixIdentity( &world_matrix );

    float angle = GetTickCount() / 500.0f;
    D3DXMatrixRotationY( &world_matrix, angle );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &world_matrix );
    
    // view
    D3DXMATRIXA16 view_matrix;

    D3DXVECTOR3 eye_pos      ( 0.0f, 3.0f, -5.0f );
    D3DXVECTOR3 look_pos     ( 0.0f, 0.0f,  0.0f );
    D3DXVECTOR3 upper_vector ( 0.0f, 1.0f,  0.0f );

    D3DXMatrixLookAtLH( &view_matrix, &eye_pos, &look_pos, &upper_vector );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &view_matrix );

    // projection
    D3DXMATRIXA16 proj_matrix;
    float fov = D3DX_PI / 4;
    float ar  = 1.0f;
    float ncp = 1.0f;
    float fcp = 100.0f;
    D3DXMatrixPerspectiveFovLH( &proj_matrix, fov, ar, ncp, fcp );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &proj_matrix );
}

VOID Render()
{
    if ( !g_pd3dDevice ) return;
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if ( SUCCEEDED( d->BeginScene() ) ) {
        SetupMatrices();

        d->SetStreamSource( 0, g_pVertexBuff, 0, sizeof(CUSTOMVERTEX) );
        d->SetFVF( D3DFVF_CUSTOMVERTEX );
        d->SetIndices( g_pIndexBuff ); // ÀÎµ¦½º ¹öÆÛ ¼±ÅÃ
        d->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 );
        d->EndScene();
    }
    d->Present( NULL, NULL, NULL, NULL );
}
//==============================================================================
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 04", 
                              WS_OVERLAPPEDWINDOW,
                              100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    if (FAILED( InitD3D(hWnd) )) goto MAIN_END;

    if (FAILED( init_vertex_buffer() )) goto MAIN_END;
    if (FAILED( init_index_buffer() ))  goto MAIN_END;

    ShowWindow( hWnd, SW_SHOWDEFAULT );
    UpdateWindow( hWnd );

    MSG msg;
    ZeroMemory( &msg, sizeof(msg) );
    while( msg.message != WM_QUIT ) {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else {
            Render();
        }
    }

MAIN_END:
    UnregisterClass( "D3D Tutorial", wc.hInstance );

    return 0;
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg ) {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}