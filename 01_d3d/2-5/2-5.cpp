#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>

#define _E_INIT_STRUCT(x) ZeroMemory( &x, sizeof(x) )
#define _E_RELEASE(x)     if ( x ) { x->Release(); x = NULL; }


//#define SHOW_HOW_TO_USE_TCI

char * g_ImgFname = "banana.bmp";

struct CUSTOMVERTEX
{
    D3DXVECTOR3 pos;
    D3DCOLOR    color;
#ifndef SHOW_HOW_TO_USE_TCI
    FLOAT       tu;
    FLOAT       tv;
#endif
};

#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )
#else
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#endif

//=======================================================================
LPDIRECT3D9             g_pD3D        = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice  = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuff = NULL;
LPDIRECT3DTEXTURE9      g_pTexture    = NULL;

VOID Cleanup()
{
    if ( g_pTexture )    { g_pTexture->Release();    g_pTexture    = NULL; }
    if ( g_pVertexBuff ) { g_pVertexBuff->Release(); g_pVertexBuff = NULL; }
    if ( g_pd3dDevice )  { g_pd3dDevice->Release();  g_pd3dDevice  = NULL; }
    if ( g_pD3D )        { g_pD3D->Release();        g_pD3D        = NULL; }
}

HRESULT InitD3D( HWND hWnd )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    if( !g_pD3D ) return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = true;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN;
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

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );

    return S_OK;
}
//=======================================================================
HRESULT init_geometry()
{
    { // try create texture from img_file
        HRESULT hr = D3DXCreateTextureFromFile( g_pd3dDevice, g_ImgFname, &g_pTexture );
        if ( FAILED( hr ) ) return E_FAIL;
    }

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

            CUSTOMVERTEX& v0 = pVertices[ 2 * i + 0 ];
            v0.pos   = D3DXVECTOR3( sin, -1.0f, cos );
            v0.color = 0xFFFFFFFF;
#ifndef SHOW_HOW_TO_USE_TCI	
            v0.tu    = ((float) i) / (50 - 1);
            v0.tv    = 1.0f;
#endif
            CUSTOMVERTEX& v1 = pVertices[ 2 * i + 1 ];
            v1.pos   = D3DXVECTOR3( sin, 1.0f, cos );
            v1.color = 0xFF808080;
#ifndef SHOW_HOW_TO_USE_TCI	
            v1.tu    = ((float) i) / (50 - 1);
            v1.tv    = 0.0f;
#endif
        }
        g_pVertexBuff->Unlock();
    }

    return S_OK;
}

VOID SetupMatrices()
{
    // world
    D3DXMATRIXA16 world_matrix;

    D3DXMatrixIdentity( &world_matrix );

    float angle = timeGetTime() / 1000.0f;
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

void SetupTexture()
{
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->SetTexture( 0, g_pTexture );
    d->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    d->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    d->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    d->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

#ifdef SHOW_HOW_TO_USE_TCI
    D3DXMATRIXA16 m;
    m._11 = 0.25f; m._12 =  0.00f; m._13 = 0.00f; m._14 = 0.00f;
    m._21 = 0.00f; m._22 = -0.25f; m._23 = 0.00f; m._24 = 0.00f;
    m._31 = 0.00f; m._32 =  0.00f; m._33 = 1.00f; m._34 = 0.00f;
    m._41 = 0.50f; m._42 =  0.50f; m._43 = 0.00f; m._44 = 1.00f;


    d->SetTransform( D3DTS_TEXTURE0, &m );
    d->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
    d->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
#endif
}

VOID Render()
{
    if( !g_pd3dDevice ) return;
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if( SUCCEEDED( d->BeginScene() ) ) {
        SetupMatrices();

        SetupTexture();

        d->SetStreamSource( 0, g_pVertexBuff, 0, sizeof(CUSTOMVERTEX) );
        d->SetFVF( D3DFVF_CUSTOMVERTEX );
        d->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2 );
        //d->DrawPrimitive( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
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