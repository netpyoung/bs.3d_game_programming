#include <Windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9math.h>

#define _E_INIT_STRUCT(x) ZeroMemory( &x, sizeof(x) )
#define _E_RELEASE(x)     if ( x ) { x->Release(); x = NULL; }

struct CUSTOMVERTEX
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 normal; // 광원을 처리하기 위한 법선벡터
};

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL )
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
    d3dpp.EnableAutoDepthStencil = true;       // Direct3D manages the depth buffer
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16; // 깊이버퍼 형식이 오게됨.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb172558%28v=vs.85%29.aspx 의 Buffer flags 참조

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

     // Z버퍼 켜기
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );

    return S_OK;
}
//=======================================================================

void SetupLights()
{
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    { /// material
        D3DMATERIAL9 material;
        _E_INIT_STRUCT( material );

        material.Diffuse.r = material.Ambient.r = 1.0f;
        material.Diffuse.g = material.Ambient.g = 1.0f;
        material.Diffuse.b = material.Ambient.b = 0.0f;
        material.Diffuse.a = material.Ambient.a = 1.0f;

        // set
        d->SetMaterial( &material );
    }

    { /// light
        D3DLIGHT9   light;
        _E_INIT_STRUCT( light );

        // fill value
        light.Range     = 1000.0f;
        light.Type      = D3DLIGHT_DIRECTIONAL;
        light.Diffuse.r = 1.0f;
        light.Diffuse.g = 1.0f;
        light.Diffuse.b = 1.0f;

        // dir
        D3DXVECTOR3 light_dir;
        float time_angle = timeGetTime() / 350.0f;
        light_dir = D3DXVECTOR3( cosf(time_angle), 1.0f, sinf(time_angle) );
        D3DXVec3Normalize( (D3DXVECTOR3 *)&light.Direction, &light_dir );

        // set
        d->SetLight( 0, &light );
        d->LightEnable( 0, true );
        d->SetRenderState( D3DRS_LIGHTING, true );

        d->SetRenderState( D3DRS_AMBIENT, 0x00202020 );
    }
}

VOID SetupMatrices()
{
    // world
    D3DXMATRIXA16 world_matrix;

    D3DXMatrixIdentity( &world_matrix );

    float angle = timeGetTime() / 500.0f;
    D3DXMatrixRotationX( &world_matrix, angle );
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
    D3DXMatrixPerspectiveLH( &proj_matrix, fov, ar, ncp, fcp );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &proj_matrix );
}


HRESULT init_geometry()
{
    { // try create vertex buffer
        HRESULT hr = g_pd3dDevice->CreateVertexBuffer( 50 * 2 * sizeof(CUSTOMVERTEX),
                                                      0, D3DFVF_CUSTOMVERTEX,
                                                      D3DPOOL_DEFAULT, &g_pVertexBuff, NULL );
        if ( FAILED( hr ) ) return E_FAIL;
    }

    { // try fill vertex buffer
        CUSTOMVERTEX* pVertices;

        HRESULT hr = g_pVertexBuff->Lock( 0, 0, (void**)&pVertices, 0 );
        if( FAILED( hr ) ) return E_FAIL;

        for (int i = 0; i < 50; ++i) {
            float angle = ( 2 * D3DX_PI * i) / (50 - 1 );
            float sin = sinf(angle);
            float cos = cosf(angle);

            pVertices[ 2 * i + 0 ].pos    = D3DXVECTOR3( sin, -1.0f, cos );
            pVertices[ 2 * i + 0 ].normal = D3DXVECTOR3( sin,  0.0f, cos );
            pVertices[ 2 * i + 1 ].pos    = D3DXVECTOR3( sin,  1.0f, cos );
            pVertices[ 2 * i + 1 ].normal = D3DXVECTOR3( sin,  0.0f, cos );
        }
        g_pVertexBuff->Unlock();
    }

    return S_OK;
}


VOID Render()
{
    if( !g_pd3dDevice ) return;
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if( SUCCEEDED( d->BeginScene() ) ) {
        SetupLights();
        SetupMatrices();

        d->SetStreamSource( 0, g_pVertexBuff, 0, sizeof(CUSTOMVERTEX) );
        d->SetFVF( D3DFVF_CUSTOMVERTEX );
        d->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2 );
        //d->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
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

    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 04", 
                              WS_OVERLAPPEDWINDOW,
                              100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    if (FAILED( InitD3D(hWnd) ))      goto MAIN_END;
    if (FAILED( init_geometry() ))    goto MAIN_END;

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