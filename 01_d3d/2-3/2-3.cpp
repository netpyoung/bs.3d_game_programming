#include <Windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9math.h>
#define _E_INIT_STRUCT(x) ZeroMemory( &x, sizeof(x) )
#define _E_RELEASE(x)     if ( x ) { x->Release(); x = NULL; }

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
};

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )
//=======================================================================
LPDIRECT3D9             g_pD3D        = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice  = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuff = NULL;

VOID Cleanup()
{
    if ( g_pVertexBuff ) { g_pVertexBuff->Release(); g_pVertexBuff = NULL; }
    if ( g_pd3dDevice )  { g_pd3dDevice->Release();  g_pd3dDevice = NULL; }
    if ( g_pD3D )        { g_pD3D->Release();        g_pD3D = NULL; }
}

HRESULT InitD3D( HWND hWnd )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    if( !g_pD3D ) return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    if( FAILED( g_pD3D->CreateDevice( 
                    D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL,
                    hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    return S_OK;
}
//=======================================================================
VOID SetupMatrices()
{
    // world
    D3DXMATRIXA16 world_matrix;

    UINT t = timeGetTime() % 1000;
    FLOAT angle = t * (2.0f * D3DX_PI) / 1000.0f;
    D3DXMatrixRotationY( &world_matrix, angle );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &world_matrix );
    
    // view
    D3DXMATRIXA16 view_matrix;

    D3DXVECTOR3 eye_pos( 0.0f, 3.0f, -5.0f );
    D3DXVECTOR3 look_pos( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 upper_vector ( 0.0f, 1.0f, 0.0f );

    D3DXMatrixLookAtLH( &view_matrix, &eye_pos, &look_pos, &upper_vector );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &view_matrix );

    // projection

    D3DXMATRIXA16 proj_matrix;
    float fov = D3DX_PI / 4; // Field Of View
    float ar  = 1.0f;        // aspect ratio
    float ncp = 1.0f;        // near clipping plane
    float fcp = 100.0f;      // far clipping plane
    D3DXMatrixPerspectiveLH( &proj_matrix, fov, ar, ncp, fcp );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &proj_matrix );
}


HRESULT init_vertex_buff()
{
    CUSTOMVERTEX vertices[] = {
         // x, y, z, color
        { -1.0f,-1.0f, 0.0f, 0xffff0000, },
        {  1.0f,-1.0f, 0.0f, 0xff0000ff, },
        {  0.0f, 1.0f, 0.0f, 0xffffffff, },
    };

    { // try create vertex buffer
        HRESULT hr = g_pd3dDevice->CreateVertexBuffer( 3 * sizeof(CUSTOMVERTEX),
                                                      0, D3DFVF_CUSTOMVERTEX,
                                                      D3DPOOL_DEFAULT, &g_pVertexBuff, NULL );
        if ( FAILED( hr ) ) return E_FAIL;
    }

    { // try fill vertex buffer
        VOID* pVertices;

        HRESULT hr = g_pVertexBuff->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 );
        if( FAILED( hr ) ) return E_FAIL;

        memcpy( pVertices, vertices, sizeof(vertices) );
        g_pVertexBuff->Unlock();
    }

    return S_OK;
}


VOID Render()
{
    if( !g_pd3dDevice ) return;
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if( SUCCEEDED( d->BeginScene() ) ) {
        SetupMatrices();

        d->SetStreamSource( 0, g_pVertexBuff, 0, sizeof(CUSTOMVERTEX) );
        d->SetFVF( D3DFVF_CUSTOMVERTEX );
        d->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 1 );
        d->EndScene();
    }
    d->Present( NULL, NULL, NULL, NULL );
}
//=======================================================================
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 02", 
                              WS_OVERLAPPEDWINDOW,
                              100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    if (FAILED( InitD3D(hWnd) ))      goto MAIN_END;
    if (FAILED( init_vertex_buff() )) goto MAIN_END;

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