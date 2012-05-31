#include <d3d9.h>

LPDIRECT3D9             g_pD3D       = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;

VOID Cleanup()
{
    if( g_pd3dDevice ) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if( g_pD3D )       { g_pD3D->Release();       g_pD3D = NULL; }
}

HRESULT InitD3D( HWND hWnd )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    if( !g_pD3D ) return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;                /// ����̽� ������ ���� ����ü
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;   /// ���� ȿ������ SWAPȿ��
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;    /// ���� ����ȭ�� ��忡 ���缭 �ĸ���۸� ����

    if( FAILED( g_pD3D->CreateDevice( 
                    D3DADAPTER_DEFAULT,                  // ����Ʈ ����ī��(��κ��� ����ī�尡 1��)
                    D3DDEVTYPE_HAL,                      // HAL����̽��� �����Ѵ�.(HW���� ���)
                    hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, // ����ó���� SWó��.(������ HW > SW)
                    &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    /// ����̽� ���������� ó���Ұ�� ���⿡�� �Ѵ�.
    return S_OK;
}

VOID Render()
{
    if( !g_pd3dDevice ) return;

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) ) {
        g_pd3dDevice->EndScene();
    }

    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


//=======================================================================
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 01: CreateDevice", 
                              WS_OVERLAPPEDWINDOW,
                              100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    if( SUCCEEDED( InitD3D( hWnd ) ) ) { 
        ShowWindow( hWnd, SW_SHOWDEFAULT );
        UpdateWindow( hWnd );

        MSG msg; 
        while( GetMessage( &msg, NULL, 0, 0 ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

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
        case WM_PAINT:
            Render();
            ValidateRect( hWnd, NULL );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}