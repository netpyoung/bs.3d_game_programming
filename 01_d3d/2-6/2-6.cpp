#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>

#define _E_INIT_STRUCT(x) ZeroMemory( &x, sizeof(x) )
#define _E_RELEASE(x)     if ( x ) { x->Release(); x = NULL; }

char * g_xFname = "tiger.x";

//=======================================================================
LPDIRECT3D9             g_pD3D        = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice  = NULL;

LPD3DXMESH          g_pMesh          = NULL;
D3DMATERIAL9*       g_pMeshMaterials = NULL;
LPDIRECT3DTEXTURE9* g_pMeshTextures  = NULL;
DWORD               g_materialNums   = 0L;

VOID Cleanup()
{
    if ( g_pMeshMaterials ) {
        delete [] g_pMeshMaterials;
        g_pMeshMaterials = NULL;
    }

    if ( g_pMeshTextures ) {
        for (int i = 0; i < g_materialNums; ++i) {
            if ( !g_pMeshTextures[i] ) continue;

            g_pMeshTextures[i]->Release();
            g_pMeshTextures[i] = NULL;
        }
    }

    if ( g_pMesh )       { g_pMesh->Release();       g_pMesh       = NULL; }

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
    d3dpp.EnableAutoDepthStencil = true;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    if( FAILED( g_pD3D->CreateDevice( 
                    D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL,
                    hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );

    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );
    return S_OK;
}
//=======================================================================
HRESULT init_geometry()
{
    LPD3DXBUFFER pD3DXMtrlBuffer;

    { // try load mesh from x-file
        HRESULT hr = D3DXLoadMeshFromX(g_xFname, D3DXMESH_SYSTEMMEM,
                                        g_pd3dDevice, NULL,
                                        &pD3DXMtrlBuffer, NULL, &g_materialNums,
                                        &g_pMesh);
        if ( FAILED( hr ) ) return E_FAIL;
    }

    // init material & texture
    D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_pMeshMaterials            = new D3DMATERIAL9[g_materialNums];
    g_pMeshTextures             = new LPDIRECT3DTEXTURE9[g_materialNums];

    for (int i = 0; i < g_materialNums; ++i) {
        g_pMeshMaterials[i]         = d3dxMaterials[i].MatD3D;
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
        g_pMeshTextures[i] = NULL;

        if (d3dxMaterials[i].pTextureFilename != NULL
            && lstrlen(d3dxMaterials[i].pTextureFilename) > 0 ) {
            HRESULT hr = D3DXCreateTextureFromFile( g_pd3dDevice, 
                                                d3dxMaterials[i].pTextureFilename, 
                                                &g_pMeshTextures[i]);
            if ( FAILED( hr ) ) return E_FAIL;
        }
    }

    pD3DXMtrlBuffer->Release();
    return S_OK;
}

VOID SetupMatrices()
{
    // world
    D3DXMATRIXA16 world_matrix;

    D3DXMatrixIdentity( &world_matrix );

    float angle = timeGetTime() / 1000.0f;
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
    D3DXMatrixPerspectiveLH( &proj_matrix, fov, ar, ncp, fcp );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &proj_matrix );
}

VOID Render()
{
    if ( !g_pd3dDevice ) return;
    LPDIRECT3DDEVICE9 d = g_pd3dDevice;

    d->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if ( SUCCEEDED( d->BeginScene() )) {
        SetupMatrices();

        for (int i = 0; i< g_materialNums; ++i) {
            d->SetMaterial( &g_pMeshMaterials[i] );
            d->SetTexture( 0, g_pMeshTextures[i] );
            g_pMesh->DrawSubset( i );
        }

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
    while ( msg.message != WM_QUIT ) {
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
    switch ( msg ) {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}