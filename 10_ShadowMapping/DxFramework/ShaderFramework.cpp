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
LPDIRECT3D9             gpD3D = NULL;				// D3D
LPDIRECT3DDEVICE9       gpD3DDevice = NULL;				// D3D 장치

// 폰트
ID3DXFont* gpFont = NULL;

// 모델
LPD3DXMESH gpTorus = nullptr;
LPD3DXMESH gpDisc = nullptr;

// 모델 색상
D3DXVECTOR4 gTorusColor(1, 1, 0, 1);
D3DXVECTOR4 gDiscColor(0, 1, 1, 1);

// 렌더타겟
LPDIRECT3DTEXTURE9 gpShadowRenderTarget = NULL;
LPDIRECT3DSURFACE9 gpShadowDepthStencil = NULL;

// 쉐이더
LPD3DXEFFECT gpApplyShadowShader = NULL;
LPD3DXEFFECT gpCreateShadowShader = NULL;

// 회전값
float gRotationY = 0.f;

// 빛의 위치
D3DXVECTOR4 gWorldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

// 카메라 위치
D3DXVECTOR4 gWorldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// 프로그램 이름
const char* gAppName = "Super Simple Shader Demo Framework";
// const char*				gAppName		= "초간단 쉐이더 데모 프레임워크";


//-----------------------------------------------------------------------
// 프로그램 진입점/메시지 루프
//-----------------------------------------------------------------------

// 진입점
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스를 등록한다.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  gAppName, NULL };
	RegisterClassEx(&wc);

	// 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(gAppName, gAppName,
		style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Client Rect 크기가 WIN_WIDTH, WIN_HEIGHT와 같도록 크기를 조정한다.
	POINT ptDiff;
	RECT rcClient, rcWindow;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// D3D를 비롯한 모든 것을 초기화한다.
	if (!InitEverything(hWnd))
		PostQuitMessage(1);

	// 메시지 루프
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // 메시지가 없으면 게임을 업데이트하고 장면을 그린다
		{
			PlayDemo();
		}
	}

	UnregisterClass(gAppName, wc.hInstance);
	return 0;
}

// 메시지 처리기
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		ProcessInput(hWnd, wParam);
		break;

	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 키보드 입력처리
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
	switch (keyPress)
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
	if (PI * 2.f < gRotationY)
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
	gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	// 장면 그리기
	gpD3DDevice->BeginScene();
	{
		RenderScene();				// 3D 물체등을 그린다.
		RenderInfo();				// 디버그 정보 등을 출력한다.
	}
	gpD3DDevice->EndScene();

	// 백버퍼에 저장되어 있는 렌더링 결과를 화면에 출력
	gpD3DDevice->Present(NULL, NULL, NULL, NULL);
}


// 3D 물체등을 그린다.
void RenderScene()
{
	// 광원-뷰행렬
	D3DXMATRIXA16 matLightView;
	{
		D3DXVECTOR3 vEyePt(gWorldLightPosition.x, gWorldLightPosition.y, gWorldLightPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 0.0f, 0.0f);

		D3DXMatrixLookAtLH(&matLightView, &vEyePt, &vLookatPt, &vUpVec);
	}

	// 광원-투영행렬을 만든다.
	D3DXMATRIXA16 matLightProjection;
	{
		D3DXMatrixPerspectiveFovLH(&matLightProjection, D3DX_PI / 4.0f, 1.0f, 1.0f, 1000.f);
	}

	// 뷰/투영행렬을 만든다.
	D3DXMATRIXA16 matViewProjection;
	{
		// 뷰행렬을 만든다.
		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(gWorldCameraPosition.x, gWorldCameraPosition.y, gWorldCameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 0.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, WIN_WIDTH / WIN_HEIGHT, 1.0f, 1000.f);
		D3DXMatrixMultiply(&matViewProjection, &matView, &matProjection);
	}

	// 원환체의 월드행렬을 만든다.
	D3DXMATRIXA16 matTorusWorld;
	{
		// 프레임마다 0.4도씩 회전을 시킨다.
		gRotationY += 0.4f * PI / 180.0f; // Degree to Radian = PI / 180
		if (gRotationY > 2 * PI)
		{
			gRotationY -= 2 * PI; // 360을 0.4로 나눌경우 0으로 떨어지지만 다른 숫자는 그렇지 않을 수 있기 때문에 0으로 초기화하지 않고 360에 해당하는 2*PI를 빼준다.
		}
		D3DXMatrixRotationY(&matTorusWorld, gRotationY);
	}

	// 디스크의 월드행렬을 만든다.
	D3DXMATRIXA16 matDiscWorld;
	{
		D3DXMATRIXA16 matScale;
		D3DXMatrixScaling(&matScale, 2.0f, 2.0f, 2.0f);

		D3DXMATRIXA16 matTrans;
		D3DXMatrixTranslation(&matTrans, 0.0f, -40.f, 0.0f);

		D3DXMatrixMultiply(&matDiscWorld, &matScale, &matTrans);
	}

	// 현재 하드웨어 백버퍼와 깊이버퍼
	LPDIRECT3DSURFACE9 pHWBackBuffer = NULL;
	LPDIRECT3DSURFACE9 pHWDepthStencilBuffer = NULL;
	gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);
	gpD3DDevice->GetDepthStencilSurface(&pHWDepthStencilBuffer);

	//////////////////////////////
	// 1. 그림자 만들기
	//////////////////////////////

	// # RenderTarget과 Surface [https://www.gpgstudy.com/forum/viewtopic.php?t=18251]
	// D3D7까지는 Surface = Texture로 사용되었지만 MipMap, CubeMap 등을 좀 더 직관적으로 처리하기 위해 Texture는 1개 이상의 Surface를 보관하는 관리자로 만들어짐
	// 따라서 Texture는 1개 이상의 Surface가 들어있는 컨테이너 이고 실제 데이터는 Surface에 들어있음.

	// 그림자맵 렌더타깃 설정
	LPDIRECT3DSURFACE9 pShadowSurface = NULL;
	if (SUCCEEDED(gpShadowRenderTarget->GetSurfaceLevel(0 /*렌더타깃의 색인*/, &pShadowSurface)))
	{
		// 가져온 LPDIRECT3DSURFACE9를 LPDIRECT3DDEVICE9에 설정
		gpD3DDevice->SetRenderTarget(0 /*렌더타겟의 색인*/, pShadowSurface);

		// # GetSurfaceLevel()의 참조 수 증가 [https://learn.microsoft.com/ko-kr/windows/win32/api/d3d9/nf-d3d9-idirect3dtexture9-getsurfacelevel]
		// IDirect3DTexture9의 GetSurfaceLevel()를 사용하면 LPDIRECT3DSURFACE9의 내부 참조 수가 증가하므로 
		// 사용을 마친 후 Release(IUnknown::Release)를 호출하지 않으면 메모리 누수가 발생함.
		pShadowSurface->Release();
		pShadowSurface = NULL;
	}

	// 깊이 버퍼를 설정
	gpD3DDevice->SetDepthStencilSurface(gpShadowDepthStencil);

	// 지난 프레임에 그렸던 그림자 정보를 지움
	// # IDirect3DDevice9::Clear 메서드 [http://telnet.or.kr/directx/graphics/reference/d3d/interfaces/idirect3ddevice9/clear.htm]
	gpD3DDevice->Clear(0, 
		NULL, 
		(D3DCLEAR_TARGET /*렌더타깃 클리어*/ | D3DCLEAR_ZBUFFER /*깊이 버퍼를 클리어*/), 
		0xFFFFFFFF /*렌더링 타겟 표면을 클리어 하는 32 비트 ARGB 색값*/,
		1.0f /*깊이 버퍼에 보존하는 새로운 z 값*/,
		0 /*각 스텐실 버퍼의 엔트리에 보존하는 정수값*/
	);

	// 그림자 만들기 셰이더 전역변수들을 설정
	gpCreateShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld);
	gpCreateShadowShader->SetMatrix("gLightViewMatrix", &matLightView);
	gpCreateShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection);

	// 그림자 만들기 셰이더 시작
	{
		UINT numPasses = 0;
		gpCreateShadowShader->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i< numPasses; ++i)
			{
				gpCreateShadowShader->BeginPass(i);
				{
					// 원환체를 그린다.
					gpTorus->DrawSubset(0);
				}
				gpCreateShadowShader->EndPass();
			}
		}
		gpCreateShadowShader->End();
	}

	//////////////////////////////
	// 2. 그림자 입히기
	//////////////////////////////

	// 하드웨어 백버퍼/깊이 버퍼를 사용한다.
	gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
	gpD3DDevice->SetDepthStencilSurface(pHWDepthStencilBuffer);

	// GetRenderTarget(), GetDepthStencilSurface()를 통해 버퍼를 얻을 때 증가한 참조 카운터를 줄여주기 위해 Release() 해준다.
	pHWBackBuffer->Release();
	pHWBackBuffer = NULL;
	pHWDepthStencilBuffer->Release();
	pHWDepthStencilBuffer = NULL;

	// 그림자 입히기 셰이더 전역변수들을 설정
	gpApplyShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld); // 원환체
	gpApplyShadowShader->SetMatrix("gViewProjectionMatrix", &matViewProjection); // 뷰/투영행렬
	gpApplyShadowShader->SetMatrix("gLightViewMatrix", &matLightView); // 광원-뷰행렬
	gpApplyShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection); // 광원-투영행렬
	gpApplyShadowShader->SetVector("gWorldLightPosition", &gWorldLightPosition); // 광원 위치: 난반사광 계산용
	gpApplyShadowShader->SetVector("gObjectColor", &gTorusColor); // 물체 색상
	gpApplyShadowShader->SetTexture("ShadowMap_Tex", gpShadowRenderTarget); // 그림자맵

	// 원환체 그리기 셰이더 시작
	{
		UINT numPasses = 0;
		gpApplyShadowShader->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i < numPasses; ++i)
			{
				gpApplyShadowShader->BeginPass(i);
				{
					// 원환체를 그린다.
					gpTorus->DrawSubset(0);

					// 디스크를 그린다.
					// 디스크를 위치, 색상이 원환체와 다르므로 변수를 다시 설정해준다.
					gpApplyShadowShader->SetMatrix("gWorldMatrix", &matDiscWorld);
					gpApplyShadowShader->SetVector("gObjectColor", &gDiscColor);
					// BeginPass(), EndPass() 블록 사이에서 변수 값을 바꿔 줄 때는 CommitChange() 함수를 호출해야 새로운 값을 사용할 수 있다.
					gpApplyShadowShader->CommitChanges();
					gpDisc->DrawSubset(0);
				}
				gpApplyShadowShader->EndPass();
			}
		}
		gpApplyShadowShader->End();
	}
}

// 디버그 정보 등을 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rct;
	rct.left = 5;
	rct.right = WIN_WIDTH / 3;
	rct.top = 5;
	rct.bottom = WIN_HEIGHT / 3;

	// 키 입력 정보를 출력
	// gpFont->DrawTextW(NULL, L"abcd 한글", -1, &rct, 0, fontColor );
	gpFont->DrawText(NULL, "Demo Framework\n\nESC: Exit Demo", -1, &rct, 0, fontColor);
}

//------------------------------------------------------------
// 초기화 코드
//------------------------------------------------------------
bool InitEverything(HWND hWnd)
{
	// D3D를 초기화
	if (!InitD3D(hWnd))
	{
		return false;
	}

	// 모델, 쉐이더, 텍스처등을 로딩
	if (!LoadAssets())
	{
		return false;
	}

	// 렌더타깃 텍스처를 만든다.
	constexpr int shadowMapSize = 2048;
	if (FAILED(gpD3DDevice->CreateTexture(
		shadowMapSize /*렌더타깃의 너비*/,
		shadowMapSize/*렌더타깃의 높이*/,
		1 /*밉맵의 수*/,
		D3DUSAGE_RENDERTARGET /*텍스처의 용도*/,
		D3DFMT_R32F /*텍스처 포맷*/,
		D3DPOOL_DEFAULT /*텍스처 할당 메모리 클래스. 렌더타깃은 언제나 D3DPOOL_DEFAULT*/,
		&gpShadowRenderTarget /*새로 만든 텍스처를 저장할 포인터*/,
		NULL /*언제나 NULL*/))
		)
	{
		return false;
	}

	// 그림자맵과 동일한 크기의 깊이버퍼도 만들어줘야 한다.
	if (FAILED(gpD3DDevice->CreateDepthStencilSurface(
		shadowMapSize /*깊이 버퍼의 너비*/,
		shadowMapSize /*깊이 버퍼의 높이*/,
		D3DFMT_D24X8 /*텍스처 포맷, 깊이 버퍼로 24비트를 사용하고 8비트는 버림(8비트는 스텐실용?)*/,
		D3DMULTISAMPLE_NONE /*안티얼라이어싱을 사용하지 않음*/,
		0 /*멀티샘플 품직, 멀티샘플을 사용하지 않으니 상관없는 값*/,
		TRUE /*깊이 버퍼를 바꿀 때, 그 안의 내용을 보존하지 않음*/,
		&gpShadowDepthStencil /*새로 만든 깊이 버퍼를 저장할 포인터*/,
		NULL /*언제나 NULL*/)))
	{
		return FALSE;
	}

	// 폰트를 로딩
	if (FAILED(D3DXCreateFont(gpD3DDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE),
		"Arial", &gpFont)))
	{
		return false;
	}

	return true;
}

// D3D 객체 및 장치 초기화
bool InitD3D(HWND hWnd)
{
	// D3D 객체
	gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!gpD3D)
	{
		return false;
	}

	// D3D장치를 생성하는데 필요한 구조체를 채워넣는다.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = WIN_WIDTH; // 백버퍼의 너비
	d3dpp.BackBufferHeight = WIN_HEIGHT; // 백버퍼의 높이
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; // 백버퍼의 포맷
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // 백버퍼를 Swap할 때의 효과. 성능상 D3DSWAPEFFECT_DISCARD 사용. 
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8; // 깊이/스텐실 버퍼의 포맷
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // 모니터 주사율과 백버퍼를 스왑하는 빈도간의 관계. D3DPRESENT_INTERVAL_ONE == 모니터가 동기될 때마다 백버퍼를 스왑. 이 때 Tearing 현상 발생.

	// D3D장치를 생성한다.
	if (FAILED(gpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &gpD3DDevice)))
	{
		return false;
	}

	return true;
}

bool LoadAssets()
{
	// 쉐이더 로딩
	gpApplyShadowShader = LoadShader("ApplyShadow.fx");
	if (nullptr == gpApplyShadowShader)
	{
		return false;
	}
	gpCreateShadowShader = LoadShader("CreateShadow.fx");
	if (nullptr == gpCreateShadowShader)
	{
		return false;
	}

	// 모델 로딩
	gpTorus = LoadModel("torus.x");
	if (nullptr == gpTorus)
	{
		return false;
	}
	gpDisc = LoadModel("disc.x");
	if (nullptr == gpDisc)
	{
		return false;
	}

	return true;
}

// 쉐이더 로딩
// .fx 포맷으로 저장된 쉐이더 파일을 불러옴
// .fx : 정점쉐이더와 픽셀쉐이더를 모두 포함
LPD3DXEFFECT LoadShader(const char* filename)
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
	if (!ret && pError)
	{
		int size = pError->GetBufferSize();
		void* ack = pError->GetBufferPointer();

		if (ack)
		{
			char* str = new char[size];
			sprintf(str, (const char*)ack, size);
			OutputDebugString(str);
			delete[] str;
		}
	}

	return ret;
}

// 모델 로딩
// .x 파일 로딩
// .x : DirectX에서 자체적으로 지원하는 메쉬 포맷
LPD3DXMESH LoadModel(const char* filename)
{
	LPD3DXMESH ret = NULL;
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, gpD3DDevice, NULL, NULL, NULL, NULL, &ret)))
	{
		OutputDebugString("모델 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	};

	return ret;
}

// 텍스처 로딩
LPDIRECT3DTEXTURE9 LoadTexture(const char* filename)
{
	// 다양한 포맷으로 저장된 이미지들을 텍스처로 로딩
	LPDIRECT3DTEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateTextureFromFile(gpD3DDevice, filename, &ret)))
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
	if (gpFont)
	{
		gpFont->Release();
		gpFont = NULL;
	}

	// 모델을 release 한다.
	if (nullptr != gpTorus)
	{
		gpTorus->Release();
	}
	if (nullptr != gpDisc)
	{
		gpDisc->Release();
	}

	// 쉐이더를 release 한다.
	if (nullptr != gpApplyShadowShader)
	{
		gpApplyShadowShader->Release();
		gpApplyShadowShader = nullptr;
	}
	if (nullptr != gpCreateShadowShader)
	{
		gpCreateShadowShader->Release();
		gpCreateShadowShader = nullptr;
	}

	// 텍스처를 release 한다.
	if (nullptr != gpShadowRenderTarget)
	{
		gpShadowRenderTarget->Release();
		gpShadowRenderTarget = nullptr;
	}
	if (nullptr != gpShadowDepthStencil)
	{
		gpShadowDepthStencil->Release();
		gpShadowDepthStencil = nullptr;
	}

	// D3D를 release 한다.
	if (gpD3DDevice)
	{
		gpD3DDevice->Release();
		gpD3DDevice = NULL;
	}

	if (gpD3D)
	{
		gpD3D->Release();
		gpD3D = NULL;
	}
}

