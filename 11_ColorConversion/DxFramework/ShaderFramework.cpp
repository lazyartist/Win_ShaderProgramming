//**********************************************************************
//
// ShaderFramework.cpp
// 
// 쉐이더 데모를 위한 C스타일의 초간단 프레임워크입니다.
// (실제 게임을 코딩하실 때는 절대 이렇게 프레임워크를
// 작성하시면 안됩니다. -_-)
//
// Author: Pope Kim
//
//**********************************************************************

#include "ShaderFramework.h"
#include <stdio.h>


//----------------------------------------------------------------------
// 전역변수
//----------------------------------------------------------------------

#define PI 3.14159265f
#define FOV PI/4.f

// D3D 관련
LPDIRECT3D9             gpD3D			= NULL;				// D3D
LPDIRECT3DDEVICE9       gpD3DDevice		= NULL;				// D3D 장치

// 폰트
ID3DXFont*              gpFont			= NULL;

// 모델
LPD3DXMESH gpSphere = nullptr;

// 쉐이더
LPD3DXEFFECT gpEnvironmentMappingShader = NULL;

// 회전값
float gRotationY = 0.f;

// 빛의 색상
D3DXVECTOR4 gWorldLightColor(1.0f, 0.f, 0.f, 1.f);

// 빛의 위치
D3DXVECTOR4 gWorldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

// 카메라 위치
D3DXVECTOR4 gWorldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// 텍스쳐
LPDIRECT3DTEXTURE9 gpStoneDM = NULL;
LPDIRECT3DTEXTURE9 gpStoneSM = NULL;
LPDIRECT3DTEXTURE9 gpStoneNM = NULL;
LPDIRECT3DCUBETEXTURE9 gpSnowENV = NULL;

// 화면을 가득 채우는 사각형
LPDIRECT3DVERTEXDECLARATION9 gpFullscreenQuadDecl = NULL;
LPDIRECT3DVERTEXBUFFER9 gpFullscreenQuadVB = NULL;
LPDIRECT3DINDEXBUFFER9 gpFullscreenQuadIB = NULL;

// 프로그램 이름
const char*				gAppName		= "Super Simple Shader Demo Framework";
// const char*				gAppName		= "초간단 쉐이더 데모 프레임워크";


//-----------------------------------------------------------------------
// 프로그램 진입점/메시지 루프
//-----------------------------------------------------------------------

// 진입점
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    // 윈도우 클래스를 등록한다.
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      gAppName, NULL };
    RegisterClassEx( &wc );

    // 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hWnd = CreateWindow( gAppName, gAppName,
                              style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

	// Client Rect 크기가 WIN_WIDTH, WIN_HEIGHT와 같도록 크기를 조정한다.
	POINT ptDiff;
	RECT rcClient, rcWindow;
	
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd,rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

    ShowWindow( hWnd, SW_SHOWDEFAULT );
    UpdateWindow( hWnd );

	// D3D를 비롯한 모든 것을 초기화한다.
	if( !InitEverything(hWnd) )
		PostQuitMessage(1);

	// 메시지 루프
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while(msg.message!=WM_QUIT)
    {
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else // 메시지가 없으면 게임을 업데이트하고 장면을 그린다
		{
			PlayDemo();
		}
    }

    UnregisterClass( gAppName, wc.hInstance );
    return 0;
}

// 메시지 처리기
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
	case WM_KEYDOWN:
		ProcessInput(hWnd, wParam);
		break;

    case WM_DESTROY:
		Cleanup();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

// 키보드 입력처리
void ProcessInput( HWND hWnd, WPARAM keyPress)
{
	switch(keyPress)
	{
	// ESC 키가 눌리면 프로그램을 종료한다.
	case VK_ESCAPE:
		PostMessage(hWnd, WM_DESTROY, 0L, 0L);
		break;
	}
}

//------------------------------------------------------------
// 게임루프
//------------------------------------------------------------
void PlayDemo()
{
	Update();
    RenderFrame();
}

// 게임로직 업데이트
void Update()
{
    gRotationY += 1.f * PI / 180.f; // 회전각(Degee) 설정
    if(PI * 2.f < gRotationY)
    {
        gRotationY -= PI * 2.f; // 360도 이상이면 360만큼 빼서 계속 돌 수 있게 한다.
    }
}

//------------------------------------------------------------
// 렌더링
//------------------------------------------------------------

void RenderFrame()
{
	D3DCOLOR bgColour = 0xFF0000FF;	// 배경색상 - 파랑

    // 화면을 색상으로 채움. 결과적으로 화2면을 지우는 효과
    gpD3DDevice->Clear( 0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0 );

    // 장면 그리기
    gpD3DDevice->BeginScene();
    {
		RenderScene();				// 3D 물체등을 그린다.
		RenderInfo();				// 디버그 정보 등을 출력한다.
    }
	gpD3DDevice->EndScene();

    // 백버퍼에 저장되어 있는 렌더링 결과를 화면에 출력
    gpD3DDevice->Present( NULL, NULL, NULL, NULL );
}


// 3D 물체등을 그린다.
void RenderScene()
{
    // matrix
    D3DXMATRIX matWorld;
    D3DXMatrixRotationY(&matWorld, gRotationY);
    
    D3DXMATRIX matView;
    D3DXVECTOR3 vEyePt = {gWorldCameraPosition.x, gWorldCameraPosition.y, gWorldCameraPosition.z};
    D3DXVECTOR3 vLookAtPt = {0.f, 0.f, 0.f};
    D3DXVECTOR3 vUpVec = {0.f, 1.f, 0.f};
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookAtPt, &vUpVec);
    
    D3DXMATRIX matProjection;
    D3DXMatrixPerspectiveFovLH(&matProjection, FOV, static_cast<float>(WIN_WIDTH)/static_cast<float>(WIN_HEIGHT), 1.f, 10000.f);

    // 월드/뷰/투영행렬을 미리 곱한다.
    D3DXMATRIXA16 matWorldView;
    D3DXMATRIXA16 matWorldViewProjection;
    D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
    D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

    // shader
    gpEnvironmentMappingShader->SetMatrix("gWorldMatrix", &matWorld);
    gpEnvironmentMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);
	gpEnvironmentMappingShader->SetVector("gWorldLightColor", &gWorldLightColor);
	gpEnvironmentMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
	gpEnvironmentMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);
	
	gpEnvironmentMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
	gpEnvironmentMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);
	gpEnvironmentMappingShader->SetTexture("NormalMap_Tex", gpStoneNM);
	gpEnvironmentMappingShader->SetTexture("EnvironmentMap_Tex", gpSnowENV);

    // render
    UINT numPass;
    gpEnvironmentMappingShader->Begin(&numPass, 0);
    for (int i = 0; i < numPass; ++i)
    {
        gpEnvironmentMappingShader->BeginPass(i);
        gpSphere->DrawSubset(0);
        gpEnvironmentMappingShader->EndPass();
    }
    gpEnvironmentMappingShader->End();
}

// 디버그 정보 등을 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255,255,255,255);    

	// 텍스트를 출력할 위치
	RECT rct;
	rct.left=5;
	rct.right=WIN_WIDTH / 3;
	rct.top=5;
	rct.bottom = WIN_HEIGHT / 3;
	 
	// 키 입력 정보를 출력
	// gpFont->DrawTextW(NULL, L"abcd 한글", -1, &rct, 0, fontColor );
	gpFont->DrawText(NULL, "Demo Framework\n\nESC: Exit Demo", -1, &rct, 0, fontColor );
}


void InitFullScreenQuad()
{
	// 정점 선언을 만든다. 정점 선언은 Mesh를 그릴 때 정점버퍼를 해석하는 방식을 D3D에 알리는 용도.
	D3DVERTEXELEMENT9 vtxDesc[3]; // 정점에 들어갈 데이터는 D3DVERTEXELEMENT9 구조체 배열로 정의
	
	int offset = 0;
	int i = 0;

	// 위치
	vtxDesc[i].Stream = 0; // 정점버퍼가 들어가는 슬롯의 색인(D3D 장치에 정점버퍼들을 동시에 여러 개 넣을 수 있기 때문)
	vtxDesc[i].Offset = offset; // 정점정보가 시작하는 위치에서 현재 요소까지의 오프셋
	vtxDesc[i].Type = D3DDECLTYPE_FLOAT3; // 데이터형. 위치값이므로 x, y, z 요소 3개.
	vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT; // 무조건 D3DDECLMETHOD_DEFAULT 사용.
	vtxDesc[i].Usage = D3DDECLUSAGE_POSITION; // 용도. 위치값.
	vtxDesc[i].UsageIndex = 0; // 셰이더에서 TEXCOORD0, TEXCOORD1, ... 등으로 시맨틱을 사용할 때 뒤에 붙는 숫자.

	offset += sizeof(float) * 3; // 위치를 float3만큼 사용했으므로 그 만틈 오프셋을 옮긴다.
	++i;

	// UV 좌표 0
	vtxDesc[i].Stream = 0;
	vtxDesc[i].Offset = offset;
	vtxDesc[i].Type = D3DDECLTYPE_FLOAT2; // 데이터형. UV이므로 u, v 요소 2개.
	vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT;
	vtxDesc[i].Usage = D3DDECLUSAGE_TEXCOORD; // 용도. 텍스쳐.
	vtxDesc[i].UsageIndex = 0;

	offset += sizeof(float) * 2; // UV를 float2만큼 사용했으므로 그 만틈 오프셋을 옮긴다.
	++i;

	// 정점포맷의 끝임을 표현 (D3DDECL_END())
	// vtxDesc[i] = D3DDECL_END(); // 아래 코드는 이 코드를 풀어쓴 것으로써 같은 의미이다.
	vtxDesc[i].Stream = 0xFF;
	vtxDesc[i].Offset = 0;
	vtxDesc[i].Type = D3DDECLTYPE_UNUSED;
	vtxDesc[i].Method = 0;
	vtxDesc[i].Usage = 0;
	vtxDesc[i].UsageIndex = 0;

	// 여기까지 코드 작성하니 다음 경고가 났다.
	// "C26482: Only index into arrays using constant expressions"
	// 일단 ChatGPT에 물어봤다.
	// ----------
	// in English
	// The warning message "Only index into arrays using constant expressions" is given because the index i used to access elements in the vtxDesc array is not a constant expression. This means that it could be changed at runtime, which can lead to potential errors or security vulnerabilities if the index goes out of bounds.
	// In C++, it is recommended to use only constant expressions when indexing into arrays, as this ensures that the index will always be within the bounds of the arrayand that the access will be optimized by the compiler.
	// To resolve this warning, you can replace i with a constant expression, for example:
	// in Korean
	// "Only index into arrays using constant expressions" 경고 메시지는 코드에서 vtxDesc 배열에 접근하는 인덱스 i가 상수 표현식이 아닌 것 때문에 나오는 것입니다. 이는 런타임에 변경될 수 있다는 것을 의미하며, 인덱스가 배열의 범위를 벗어나는 경우 오류 또는 보안 취약점이 발생할 수 있습니다.
	// C++에서는 배열에 접근할 때 상수 표현식만 사용하는 것이 좋습니다.이는 인덱스가 항상 배열의 범위 안에 있음을 보장하며, 컴파일러에 의해 최적화될 수 있음을 뜻합니다.
	// 이 경고를 해결하려면 i를 상수 표현식으로 대체할 수 있습니다.예를 들어 :
	// ----------
	// 근데 배열에 접근하는데 왜 상수만 사용해야하나 이유를 모르겠어서 찾아보니 범위에 기반한 루프문에서 배열에 접근하라는 뜻이었다.
	// i가 코드 중간에 변경되면 배열 범위를 벗어나므로 위험하기 때문에 위 처럼 쓸거면 상수를 사용해서 코드를 명확하게 해야하고 아니면 범위를 지정해서 접근하라는 뜻.

}

//------------------------------------------------------------
// 초기화 코드
//------------------------------------------------------------
bool InitEverything(HWND hWnd)
{
	// D3D를 초기화
	if( !InitD3D(hWnd) )
	{
		return false;
	}
	
	// 모델, 쉐이더, 텍스처등을 로딩
	if( !LoadAssets() )
	{
		return false;
	}

	// 폰트를 로딩
    if(FAILED(D3DXCreateFont( gpD3DDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                           OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE), 
                           "Arial", &gpFont )))
	{
		return false;
	}

	return true;
}

// D3D 객체 및 장치 초기화
bool InitD3D(HWND hWnd)
{
    // D3D 객체
    gpD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !gpD3D )
	{
		return false;
	}

    // D3D장치를 생성하는데 필요한 구조체를 채워넣는다.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

	d3dpp.BackBufferWidth				= WIN_WIDTH; // 백버퍼의 너비
	d3dpp.BackBufferHeight				= WIN_HEIGHT; // 백버퍼의 높이
	d3dpp.BackBufferFormat				= D3DFMT_X8R8G8B8; // 백버퍼의 포맷
    d3dpp.BackBufferCount				= 1;
    d3dpp.MultiSampleType				= D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality			= 0;
    d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD; // 백버퍼를 Swap할 때의 효과. 성능상 D3DSWAPEFFECT_DISCARD 사용. 
    d3dpp.hDeviceWindow					= hWnd;
    d3dpp.Windowed						= TRUE;
    d3dpp.EnableAutoDepthStencil		= TRUE;
    d3dpp.AutoDepthStencilFormat		= D3DFMT_D24X8; // 깊이/스텐실 버퍼의 포맷
    d3dpp.Flags							= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    d3dpp.FullScreen_RefreshRateInHz	= 0;
    d3dpp.PresentationInterval			= D3DPRESENT_INTERVAL_ONE; // 모니터 주사율과 백버퍼를 스왑하는 빈도간의 관계. D3DPRESENT_INTERVAL_ONE == 모니터가 동기될 때마다 백버퍼를 스왑. 이 때 Tearing 현상 발생.

    // D3D장치를 생성한다.
    if( FAILED( gpD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &gpD3DDevice ) ) )
    {
        return false;
    }

    return true;
}

bool LoadAssets()
{
	// 쉐이더 로딩
    gpEnvironmentMappingShader = LoadShader("../EnvironmentMapping.fx");
    if(nullptr == gpEnvironmentMappingShader)
    {
        return false;
    }

	// 모델 로딩
    gpSphere = LoadModel("../TeapotWithTangent.x");
    // gpSphere = LoadModel("../../Resources/Sphere.x");
    if (nullptr == gpSphere)
    {
        return false;
    }

	// 텍스쳐 로딩
	gpStoneDM = LoadTexture("../../Resources/Fieldstone_DM.tga");
	if (nullptr == gpStoneDM)
	{
		return false;
	}

	gpStoneSM = LoadTexture("../../Resources/Fieldstone_SM.tga");
	if (nullptr == gpStoneSM)
	{
		return false;
	}

	gpStoneNM = LoadTexture("../../Resources/Fieldstone_NM.tga");
	if (nullptr == gpStoneNM)
	{
		return false;
	}

    D3DXCreateCubeTextureFromFile(gpD3DDevice, "../../Resources/Snow_ENV.dds", &gpSnowENV);
    if (nullptr == gpSnowENV)
    {
        return false;
    }

	return true;
}

// 쉐이더 로딩
// .fx 포맷으로 저장된 쉐이더 파일을 불러옴
// .fx : 정점쉐이더와 픽셀쉐이더를 모두 포함
LPD3DXEFFECT LoadShader(const char * filename )
{
	LPD3DXEFFECT ret = NULL;

	LPD3DXBUFFER pError = NULL;
	DWORD dwShaderFlags = 0;

#if _DEBUG
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

    // .fx 파일을 로딩 및 컴파일
	D3DXCreateEffectFromFile(gpD3DDevice, filename,
		NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

	// 쉐이더 로딩에 실패한 경우 output창에 쉐이더
	// 컴파일 에러를 출력한다.
	if(!ret && pError)
	{
		int size	= pError->GetBufferSize();
		void *ack	= pError->GetBufferPointer();

		if(ack)
		{
			char* str = new char[size];
			sprintf(str, (const char*)ack, size);
			OutputDebugString(str);
			delete [] str;
		}
	}

	return ret;
}

// 모델 로딩
// .x 파일 로딩
// .x : DirectX에서 자체적으로 지원하는 메쉬 포맷
LPD3DXMESH LoadModel(const char * filename)
{
	LPD3DXMESH ret = NULL;
	if ( FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, gpD3DDevice, NULL,NULL,NULL,NULL, &ret)) )
	{
		OutputDebugString("모델 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	};

	return ret;
}

// 텍스처 로딩
LPDIRECT3DTEXTURE9 LoadTexture(const char * filename)
{
    // 다양한 포맷으로 저장된 이미지들을 텍스처로 로딩
	LPDIRECT3DTEXTURE9 ret = NULL;
	if ( FAILED(D3DXCreateTextureFromFile(gpD3DDevice, filename, &ret)) )
	{
		OutputDebugString("텍스처 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}
//------------------------------------------------------------
// 뒷정리 코드.
//------------------------------------------------------------

void Cleanup()
{
	// 폰트를 release 한다.
	if(gpFont)
	{
		gpFont->Release();
		gpFont = NULL;
	}

	// 모델을 release 한다.

	// 쉐이더를 release 한다.
    if(nullptr != gpEnvironmentMappingShader)
    {
        gpEnvironmentMappingShader->Release();
        gpEnvironmentMappingShader = nullptr;
    }

    // 텍스쳐 release
    if(nullptr != gpStoneDM)
    {
        gpStoneDM->Release();
    }
    if(nullptr != gpStoneSM)
    {
        gpStoneSM->Release();
    }
    if(nullptr != gpStoneNM)
    {
        gpStoneNM->Release();
    }
    if(nullptr != gpSnowENV)
    {
        gpSnowENV->Release();
    }

	// 화면크기 사각형을 해제한다.
	if (gpFullscreenQuadDecl)
	{
		gpFullscreenQuadDecl->Release();
		gpFullscreenQuadDecl = NULL;
	}

	if (gpFullscreenQuadVB)
	{
		gpFullscreenQuadVB->Release();
		gpFullscreenQuadVB = NULL;
	}

	if (gpFullscreenQuadIB)
	{
		gpFullscreenQuadIB->Release();
		gpFullscreenQuadIB = NULL;
	}

	// D3D를 release 한다.
    if(gpD3DDevice)
	{
        gpD3DDevice->Release();
		gpD3DDevice = NULL;
	}

    if(gpD3D)
	{
        gpD3D->Release();
		gpD3D = NULL;
	}
}

