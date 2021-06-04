#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "d3dx9.h"
#include "XFileUtil.h"
#include "PhysicalObj.h"
#include "Camera.h"

VOID SetupViewProjection();
//-----------------------------------------------------------------------------
// 전역 변수 
ZCamera* g_pCamera = NULL;	// Camera 클래스

LPDIRECT3D9             g_pD3D = NULL; // Direct3D 객체 
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // 랜더링 장치 (비디오카드)

LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // 버텍스 버퍼 
PDIRECT3DVERTEXBUFFER9  g_pVBTexture = NULL; // 텍스쳐 출력용버텍스 버퍼
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // 텍스쳐 로딩용 변수

CXFileUtil              g_XFile;   // X 파일 출력을 위한 클래스 객체 
CXFileUtil				g_XBall1;
CXFileUtil				g_XBall2;
CXFileUtil				g_XBall3;
CXFileUtil				g_XEnemy;
CXFileUtil				g_XAirPlane;
CXFileUtil				g_XAirPlane2;
PhysicalObj				g_PhyObject;
PhysicalObj				g_PhyBall1;
PhysicalObj				g_PhyBall2;
PhysicalObj				g_PhyBall3;
PhysicalObj				g_PhyEnemy;
PhysicalObj				g_PhyAirPlane;
PhysicalObj				g_PhyAirPlane2;
int randomSpeed[3];
bool fire = false;

// 스프라이트 처리를 위한 구조체 
struct SPRITE {
	int spriteNumber;   // 전체 스프라이트 이미지 수
	int curIndex;       // 현재 출력해야 하는 스프라이트 인덱스 
	int frameCounter;   // 현재 스프라이트를 출력하고 지속된 프레임 수 
	int frameDelay;     // 스프라이트 변경 속도 조절을 위한 프레임 딜레이 
};
// 스프라이트 구조체 변수 선언 및 초기화
SPRITE g_Fire = { 15, 0, 0, 2 }; // 선언 & 구조체의 각 요소 초기화 

// 커스텀 버텍스 타입 구조체 
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D 좌표값
	DWORD color;      // 버텍스 색상
};

// 커스텀 버텍스의 구조를 표시하기 위한 FVF(Flexible Vertex Format) 값 
// D3DFVF_XYZ(3D 월드 좌표) 와  D3DFVF_DIFFUSE(점의 색상) 특성을 가지도록.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// 텍스쳐 좌표를 가지는 버텍스 구조체 정의
struct TEXTUREVERTEX
{
	D3DXVECTOR3     position;  // 버텍스의 위치
	D3DCOLOR        color;     // 버텍스의 색상
	FLOAT           tu, tv;    // 텍스쳐 좌표 
};

// 위 구조체의 구조를 표현하는 FVF 값 정의
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// 텍스쳐 좌표를 가지는 버텍스 구조체 정의
struct MYVERTEX
{
	D3DXVECTOR3     position;  // 버텍스의 위치
	D3DCOLOR        color;     // 버텍스의 색상
};

struct MYINDEX
{
	WORD	_0, _1, _2;
};

// 위 구조체의 구조를 표현하는 FVF 값 정의
#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



//-----------------------------------------------------------------------------
// 이름: InitD3D()
// 기능: Direct3D 초기화 
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Direct3D 객체 생성 
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// 장치 생성용 데이타 준비

	D3DPRESENT_PARAMETERS d3dpp;         // 장치 생성용 정보 구조체 변수 선언

	ZeroMemory(&d3dpp, sizeof(d3dpp));                  // 구조체 클리어
	d3dpp.BackBufferWidth = 1024;               // 버퍼 해상도 넓이 설정
	d3dpp.BackBufferHeight = 768;               // 버퍼 해상도 높이 설정 
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;   // 버퍼 포맷 설정 
	d3dpp.BackBufferCount = 1;                 // 백버퍼 수 
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // 스왑 방법 설정
	d3dpp.hDeviceWindow = hWnd;              // 윈도우 핸들 설정 
	d3dpp.Windowed = true;              // 윈도우 모드로 실행 되도록 함 
	d3dpp.EnableAutoDepthStencil = true;              // 스탠실 버퍼를 사용하도록 함 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;      // 스탠실 버퍼 포맷 설정 


	// D3D객체의 장치 생성 함수 호출 (디폴트 비디오카드 사용, HAL 사용)
	
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// 이제 장치가 정상적으로 생성되었음.

	// zbuffer 사용하도록 설정
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	// 기본컬링 없음
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	return S_OK;
}

//-----------------------------------------------------------------------------
// 이름: InitGameData()
// 기능: 게임에 관련된 각종 데이터를 초기화 한다. 
//-----------------------------------------------------------------------------
HRESULT InitGameData()
{
	// 뷰 및 프로젝션 변환 설정
	SetupViewProjection();

	g_XFile.XFileLoad(g_pd3dDevice, "./images/av.x", 1.0f, 1.0f, 1.0f);
	g_XBall1.XFileLoad(g_pd3dDevice, "./images/ball.x", 0.3f, 0.3f, 1.0f);
	g_XBall2.XFileLoad(g_pd3dDevice, "./images/ball.x", 0.3f, 0.3f, 1.0f);
	g_XBall3.XFileLoad(g_pd3dDevice, "./images/ball.x", 0.3f, 0.3f, 1.0f);
	g_XEnemy.XFileLoad(g_pd3dDevice, "./images/malp.x", 1.0f, 1.0f, 1.0f);
	g_XAirPlane.XFileLoad(g_pd3dDevice, "./images/airplane.x", 1.0f, 1.0f, 1.0f);
	g_XAirPlane2.XFileLoad(g_pd3dDevice, "./images/airplane.x", 1.0f, 1.0f, 1.0f);

	return S_OK;
}


//-----------------------------------------------------------------------------
// 이름: InitMeshData()
// 기능: Mesh에 관련된 각종 데이터를 초기화 한다. 
//-----------------------------------------------------------------------------
HRESULT InitMeshData()
{
	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		randomSpeed[i] = rand() % 11 + 5; // 5 ~ 15
	}

	/*float 인자 3개는 R,G,B를 나타냅니다. 
	ball을 파랗게 만들기 위해 함수에 매개 변수를 추가하여 변경했습니다.*/
	g_XFile.XFileLoad(g_pd3dDevice, "./images/av.x",1.0f,1.0f,1.0f);
	g_XBall1.XFileLoad(g_pd3dDevice, "./images/ball.x",0.3f, 0.3f, 1.0f);
	g_XBall2.XFileLoad(g_pd3dDevice, "./images/ball.x", 0.3f, 0.3f, 1.0f);
	g_XBall3.XFileLoad(g_pd3dDevice, "./images/ball.x", 0.3f, 0.3f, 1.0f);
	g_XEnemy.XFileLoad(g_pd3dDevice, "./images/malp.x", 1.0f, 1.0f, 1.0f);
	g_XAirPlane.XFileLoad(g_pd3dDevice, "./images/airplane.x", 1.0f, 1.0f, 1.0f);
	g_XAirPlane2.XFileLoad(g_pd3dDevice, "./images/airplane.x", 1.0f, 1.0f, 1.0f);

	D3DXVECTOR3 min, max;
	D3DXVECTOR3 center;
	float radius;


	BYTE* v = 0;
	g_XFile.GetMesh()->LockVertexBuffer(0, (void**)&v);

	HRESULT hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XFile.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile.GetMesh()->GetFVF()),
		&min,
		&max);
	g_XFile.GetMesh()->UnlockVertexBuffer();
	g_PhyObject.SetScale(0.5f);
	g_PhyObject.SetRotation(3.1f);
	g_PhyObject.SetPosition(0.0, 0.0, -250);
	g_PhyObject.SetHP(15);
	g_PhyObject.SetBoundingBox(min, max);

	g_XBall1.GetMesh()->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XBall1.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XBall1.GetMesh()->GetFVF()),
		&min,
		&max);
	hr = D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		g_XBall1.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XBall1.GetMesh()->GetFVF()),
		&center,
		&radius);
	g_XBall1.GetMesh()->UnlockVertexBuffer();
	g_PhyBall1.SetBoundingBox(min, max);
	g_PhyBall1.SetBoundingSphere(center, radius);
	g_PhyBall1.SetPosition(-100, 0, 180);
	g_PhyBall1.AddVelocity(0, 0, -randomSpeed[0]);

	// Ball1과 ball2는 동일하므로 Bounding volume을 새로 계산할 필요는 없음
	g_PhyBall2.SetBoundingBox(min, max);
	g_PhyBall2.SetBoundingSphere(center, radius);
	g_PhyBall2.SetPosition(0, 0, 180);
	g_PhyBall2.AddVelocity(0, 0, -randomSpeed[1]);
	if (-g_PhyBall2.RandomSpeed() == -14) {
		g_PhyEnemy.SetCollision(-1);
	}

	g_PhyBall3.SetBoundingBox(min, max);
	g_PhyBall3.SetBoundingSphere(center, radius);
	g_PhyBall3.SetPosition(100, 0, 180);
	g_PhyBall3.AddVelocity(0, 0, -randomSpeed[2]);
	//g_PhyBall3.AddVelocity(0, 0, -15);

	g_XEnemy.GetMesh()->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XEnemy.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XEnemy.GetMesh()->GetFVF()),
		&min,
		&max);
	g_XEnemy.GetMesh()->UnlockVertexBuffer();
	//g_PhyEnemy.SetBoundingSphere(center, radius);
	g_PhyEnemy.SetPosition(0, 0, 300);
	g_PhyEnemy.SetScale(2);
	g_PhyEnemy.SetHP(35);
	g_PhyEnemy.SetBoundingBox(min, max);

	g_XAirPlane.GetMesh()->LockVertexBuffer(0, (void**)&v);
	g_XAirPlane.GetMesh()->UnlockVertexBuffer();
	g_PhyAirPlane.SetPosition(-100, 150, 200);
	g_PhyAirPlane.SetScale(10);

	g_XAirPlane2.GetMesh()->LockVertexBuffer(0, (void**)&v);
	g_XAirPlane2.GetMesh()->UnlockVertexBuffer();
	g_PhyAirPlane2.SetPosition(100, 100, 200);
	g_PhyAirPlane2.SetScale(10);

	return S_OK;
}


//-----------------------------------------------------------------------------
// 이름: InitGeometry()
// 기능: 버텍스 버퍼를 생성한 후 버텍스로 채운다. 
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// 버텍스 버퍼에 넣을 버텍스 자료를 임시로 만든다. 
	CUSTOMVERTEX vertices[] =
	{
		{ -200.0f,  0.0f, 0.0f, 0xff00ff00, }, // x축 라인을 위한 버텍스 
	{ 200.0f,  0.0f, 0.0f, 0xff00ff00, },

	{ 0.0f, 0.0f, -200.0f, 0xffffff00, },  // z축 라인을 위한 버텍스
	{ 0.0f, 0.0f,  200.0f, 0xffffff00, },

	{ 0.0f, -200.0f, 0.0f, 0xffff0000, },  // y축 라인을 위한 버텍스
	{ 0.0f,  200.0f, 0.0f, 0xffff0000, },

	{ 0.0f, 50.0f, 0.0f, 0xffff0000, },  // 삼각형의 첫 번째 버텍스 
	{ -50.0f,  0.0f, 0.0f, 0xffff0000, },  // 삼각형의 두 번째 버텍스 
	{ 50.0f,  0.0f, 0.0f, 0xffff0000, },  // 삼가형의 세 번째 버텍스 
	};

	// 버텍스 버퍼를 생성한다.
	// 각 버텍스의 포맷은 D3DFVF_CUSTOMVERTEX 라는 것도 전달 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// 버텍스 버퍼에 락을 건 후 버텍스를 넣는다. 
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}


//-----------------------------------------------------------------------------
// 이름: InitGeometryTexture()
// 기능: 텍스쳐 출력을 위한 버텍스 버퍼를 생성한 후 버텍스로 채운다. 
//-----------------------------------------------------------------------------
HRESULT InitGeometryTexture()
{
	// 텍스쳐 로딩 
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/Fire.bmp", &g_pTexture)))
	{
		return E_FAIL;
	}

	// 버텍스 버퍼 생성 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL)))
	{
		return E_FAIL;
	}

	// 버텍스 버퍼 설정 
	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	pVertices[0].position = D3DXVECTOR3(-50, 100, 200);  // 버텍스 위치 
	pVertices[0].color = 0xffffffff;                  // 버텍스 알파 및 색상 
	pVertices[0].tu = 0.0f;                        // 버텍스 U 텍스쳐 좌표 
	pVertices[0].tv = 0.0f;                        // 버텍스 V 텍스쳐 좌표 

	pVertices[1].position = D3DXVECTOR3(50, 100, 200);
	pVertices[1].color = 0xffffffff;
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;

	pVertices[2].position = D3DXVECTOR3(-50, 0, 200);
	pVertices[2].color = 0xffffffff;
	pVertices[2].tu = 0.0f;
	pVertices[2].tv = 1.0f;

	pVertices[3].position = D3DXVECTOR3(50, 0, 200);
	pVertices[3].color = 0xffffffff;
	pVertices[3].tu = 1.0f;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	return S_OK;
}

//-----------------------------------------------------------------------------
// 이름: Cleanup()
// 기능: 초기화되었던 모든 객체들을 해제한다. 
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();
	if (g_pVBTexture != NULL)
		g_pVBTexture->Release();
	if (g_pTexture != NULL)
		g_pTexture->Release();


	if (g_pd3dDevice != NULL)    // 장치 객체 해제 
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)          // D3D 객체 해제 
		g_pD3D->Release();

}



//-----------------------------------------------------------------------------
// 이름: SetupViewProjection()
// 기능: 뷰 변환과 프로젝션 변환을 설정한다. 
//-----------------------------------------------------------------------------
VOID SetupViewProjection()
{
	// 뷰 변환 설정 
	D3DXVECTOR3 vEyePt(0.0f, 300.0f, -600.0f);    // 카메라의 위치 
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);       // 바라보는 지점 
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);          // 업벡터 설정 
	D3DXMATRIXA16 matView;                           // 뷰변환용 매트릭스 
													 // 뷰 매트릭스 설정 
	//D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D 장치에 뷰 매트릭스 전달 
	//g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// 프로젝션 변환 설정 
	D3DXMATRIXA16 matProj;   // 프로젝션용 매트릭스 
							 // 프로젝션 매트릭스 설정 
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	// Direct3D 장치로 프로젝션 매트릭스 전달 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	/// 카메라 초기화
	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
	D3DXMATRIXA16* pmatView = g_pCamera->GetViewMatrix();		// 카메라 행렬을 얻는다.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// 카메라 행렬 셋팅

}

// 색상을 미리 정해 놓으면 편리하다.
D3DCOLORVALUE black = { 0, 0, 0, 1 };
D3DCOLORVALUE dark_gray = { 0.2f, 0.2f, 0.2f, 1.0f };
D3DCOLORVALUE gray = { 0.5f, 0.5f, 0.5f, 1.0f };
D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
D3DCOLORVALUE blue = { 0.0f, 0.0f, 1.0f, 1.0f };
VOID SetupLight()
{
	D3DLIGHT9 light;                         // Direct3D 9 조명 구조체 변수 선언

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;       // 조명 타입을 디렉셔널로 설정
	light.Diffuse = white;                   // 조명의 색 설정
	light.Specular = white;
	light.Direction = D3DXVECTOR3(10, -10, 10);       //  조명의 방향 (진행하는 방향) 
													  //light.Direction = D3DXVECTOR3(20*sin(g_counter*0.01f), -10, 10);       //  조명의 방향 (진행하는 방향) 
													  //light.Direction = D3DXVECTOR3(10, 25, -40);       //  조명의 방향 (진행하는 방향) 
	g_pd3dDevice->SetLight(0, &light);      // 라이트 번호 지정 (여기에서는 0번)
	g_pd3dDevice->LightEnable(0, TRUE);     // 0번 라이트 켜기


											// 라이트 사용 기능을 TRUE로 함. (이 기능을 끄면 모든 라이트 사용은 중지됨)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	// 최종적으로 엠비언트 라이트 켜기 (환경광의 양을 결정)
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00303030 );
}

//-----------------------------------------------------------------------------
// 이름: ChangeSpriteUV()
// 기능: 스프라이트를 위한 uv 변경 함수  
//-----------------------------------------------------------------------------
HRESULT ChangeSpriteUV(SPRITE* sp)
{
	// for tree texture

	float u = (sp->curIndex * 64.0f) / 960.0f;       // 현재 인덱스를 이용한 u 계산 
	float u2 = ((sp->curIndex + 1) * 64.0f) / 960.0f;  // 현재 인덱스+1을 이용한 u 계산

	TEXTUREVERTEX* pVertices;   // 버텍스 버퍼 접근용 포인터
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	pVertices[0].tu = u;     // u 좌표 변경 
	pVertices[0].tv = 0.0f;  // v 좌표 변경 

	pVertices[1].tu = u2;
	pVertices[1].tv = 0.0f;

	pVertices[2].tu = u;
	pVertices[2].tv = 1.0f;

	pVertices[3].tu = u2;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	// 지정된 딜레이 프레임이 지난 경우 
	if (sp->frameCounter >= sp->frameDelay) {
		sp->curIndex = (sp->curIndex + 1) % sp->spriteNumber; // 인덱스 변경 
		sp->frameCounter = 0;   // 프레임 카운터 초기화 
	}
	else  // 아직 변경할 시간이 안된 경우 
		sp->frameCounter++;    // 프레임 카운터 증가 

	return S_OK;

}

//-----------------------------------------------------------------------------
// 이름: Render()
// 기능: 화면을 그린다.
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)  // 장치 객체가 생성되지 않았으면 리턴 
		return;

	// 뷰 및 프로젝션 변환 설정
	//SetupViewProjection();

	// 삼각형의 앞/뒤 변을 모두 렌더링하도록 컬링 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 조명을 끈다. (조명에 의한 색상이 이나고, 버텍스 자체의 색상을 사용하도록)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 백버퍼를 지정된 색상으로 지운다.
	// 백버퍼를 클리어
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// 화면 그리기 시작 
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		///// 버텍스 출력 
		// 버텍스 버퍼 지정 
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // 버텍스 포멧 지정 

		D3DXMATRIXA16 matWorld;  // 월드 변환용 매트릭스 선언 

		for (float x = -200; x <= 200; x += 20) {  // z 축에 평행한 라인을 여러 개 그리기 
			D3DXMatrixTranslation(&matWorld, x, 0.0, 0.0);  // x축에 따라 위치 이동 매트릭스   
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld); // 변환매트릭스 적용 
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);  // z축 라인 그리기 
		}

		for (float z = -200; z <= 200; z += 20) {  // x 축에 평행한 라인을 여러 개 그리기 
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);  // z 축에 따라 위치 이동 매트릭스
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // 변환매트릭스 적용 
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);   // x축 라인 그리기 
		}

		D3DXMatrixIdentity(&matWorld);   // 매트릭스를 단위 행렬로 리셋 
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // 변환 매트릭스 전달 
		//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);   // y 축 그리기 


		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

		SetupLight();

		// X 파일 출력
		// player
		matWorld = g_PhyObject.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);

		// enemy
		if (g_PhyEnemy.enemyExt == false) {
			matWorld = g_PhyEnemy.GetWorldMatrix();
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			g_XEnemy.XFileDisplay(g_pd3dDevice);
		}

		// airplane
		matWorld = g_PhyAirPlane.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XAirPlane.XFileDisplay(g_pd3dDevice);

		// airplane2
		matWorld = g_PhyAirPlane2.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XAirPlane2.XFileDisplay(g_pd3dDevice);

		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);

		if (g_PhyEnemy.HPCheck() <= 0) { // 적 HP가 0이되면
			TCHAR msg[100];
			sprintf_s(msg, "적 처치 성공! 남은 HP %d", g_PhyObject.HPCheck());
			if (MessageBox(NULL, msg, "GAME SUCCESS", MB_OK) == IDOK) {
				exit(0); // 플레이어 남은 HP를 알려주는 메세지박스
			}
		}

		if (g_PhyObject.HPCheck() <= 0) { // 플레이어 HP가 0이되면
			TCHAR msg[100];
			sprintf_s(msg, "남은 적 HP %d", g_PhyEnemy.HPCheck());
			if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
				exit(0); // 적의 남은 HP를 알려주는 메세지박스
			}
		}

		// ball1
		matWorld = g_PhyBall1.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall1.Zposition() < -400) { // 못 맞추고 새 뒤로 넘어 갔을 때 초기화
			g_PhyObject.HPReduction(1); // 플레이어 HP 감소
			g_PhyBall1.SetPosition(-100, 0, 180); // 위치 초기화
			g_PhyBall1.SetVelocity(0, 0, -g_PhyBall1.RandomSpeed()); // 스피드 랜덤 초기화
		}
		if (g_PhyBall1.Zposition() > 500) { // 맞췄을 때 초기화
			g_PhyEnemy.HPReduction(1); // 적 HP 감소
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision()); // 충돌 횟수 초기화
			g_PhyBall1.SetPosition(-100, 0, 180);// 위치 초기화
			g_PhyBall1.SetVelocity(0, 0, -g_PhyBall1.RandomSpeed()); // 스피드 랜덤 초기화
		}
		g_XBall1.XFileDisplay(g_pd3dDevice);

		// ball2
		matWorld = g_PhyBall2.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall2.Zposition() < -400) { // 못 맞추고 애니비아 뒤로 넘어 갔을 때 초기화
			g_PhyObject.HPReduction(1);
			if (g_PhyObject.HPCheck() <= 0) {
				TCHAR msg[100];
				sprintf_s(msg, "남은 적 HP %d", g_PhyEnemy.HPCheck());
				if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
					exit(0);
				}
			}
			g_PhyBall2.SetPosition(0, 0, 180);
			g_PhyBall2.SetVelocity(0, 0, -g_PhyBall2.RandomSpeed());
		}
		if (g_PhyBall2.Zposition() > 500) { // 맞췄을 때 초기화
			g_PhyEnemy.HPReduction(1);
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision());
			g_PhyBall2.SetVelocity(0, 0, -g_PhyBall2.RandomSpeed());
			g_PhyBall2.SetPosition(0, 0, 180);
		}
		g_XBall2.XFileDisplay(g_pd3dDevice);

		// ball3
		matWorld = g_PhyBall3.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall3.Zposition() < -400) { // 못 맞추고 애니비아 뒤로 넘어 갔을 때 초기화
			g_PhyObject.HPReduction(1);
			if (g_PhyObject.HPCheck() <= 0) {
				TCHAR msg[100];
				sprintf_s(msg, "남은 적 HP %d", g_PhyEnemy.HPCheck());
				if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
					exit(0);
				}
			}
			g_PhyBall3.SetPosition(100, 0, 180);
			g_PhyBall3.SetVelocity(0, 0, -g_PhyBall3.RandomSpeed());
		}
		if (g_PhyBall3.Zposition() > 500) { // 맞췄을 때 초기화
			g_PhyEnemy.HPReduction(1);
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision());
			g_PhyBall3.SetVelocity(0, 0, -g_PhyBall3.RandomSpeed());
			g_PhyBall3.SetPosition(100, 0, 180);
		}
		g_XBall3.XFileDisplay(g_pd3dDevice);

		

		// 텍스처 필터 설정

		// 조명 중지
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// 텍스쳐 설정 (텍스쳐 매핑을 위하여 g_pTexture를 사용하였다.)
		g_pd3dDevice->SetTexture(0, g_pTexture);

		// 텍스쳐 출력 환경 설정
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // TRUE 로 변경
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		// 출력할 버텍스 버퍼 설정
		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));
		// FVF 값 설정
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);
		// 사각형 영역 (삼각형 2개를 이용하여 사각형 영역을 만들었음) 출력 
		D3DXMatrixIdentity(&matWorld);   // 매트릭스를 단위 행렬로 리셋
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyEnemy.NumberCollision()>=1) {
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		}
		
		

		// 텍스쳐 설정 해제
		g_pd3dDevice->SetTexture(0, NULL);

		// 폭발 스프라이트 애니메이션을 위한 uv 애니메이션 함수 호출
		ChangeSpriteUV(&g_Fire);

		// 화면 그리기 끝 
		g_pd3dDevice->EndScene();
	}

	// 백버퍼의 내용을 화면으로 보낸다. 
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



//-----------------------------------------------------------------------------
// 이름 : MsgProc()
// 기능 : 윈도우 메시지 핸들러 
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool move = false;
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();   // 프로그램 종료시 객체 해제를 위하여 호출함 
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		Render();    // 화면 출력을 담당하는 렌더링 함수 호출 
		ValidateRect(hWnd, NULL);
		return 0;
	// 키보드 입력
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			TCHAR msg[100];
			sprintf_s(msg,"내 HP %d 적 HP %d", g_PhyObject.HPCheck(),g_PhyEnemy.HPCheck());
			MessageBox(NULL, msg, "HP 확인", MB_OK);
			break;
		case VK_LEFT:
			g_PhyObject.SetXposition(-100);
			break;
		case VK_RIGHT:
			g_PhyObject.SetXposition(100);
			break;
		case VK_ESCAPE:
			if (MessageBox(NULL, "게임을 종료 하시겠습니까?", "게임 종료 창", MB_YESNO) == IDYES)
				exit(0);
			break;
		case VK_UP:
			break;
		}
		break;
	// 마우스 입력 (카메라)
	case WM_LBUTTONDOWN:
		move = true;
		break;
	case WM_MOUSEMOVE:
		if(move)
			if (LOWORD(lParam) >= 504 && HIWORD(lParam) >= 377) {
				g_pCamera->MoveLocalX(2.5f);
				g_pCamera->MoveLocalY(-2.5f);
			}
			else if (LOWORD(lParam) >= 504 && HIWORD(lParam) <= 377) {
				g_pCamera->MoveLocalX(2.5f);
				g_pCamera->MoveLocalY(2.5f);
			}
			else if (LOWORD(lParam) <= 504 && HIWORD(lParam) >= 377) {
				g_pCamera->MoveLocalX(-2.5f);
				g_pCamera->MoveLocalY(-2.5f);
			}
			else {
				g_pCamera->MoveLocalX(-2.5f);
				g_pCamera->MoveLocalY(2.5f);
			}
		break;
	case WM_RBUTTONDOWN:
		move = false;
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/**-----------------------------------------------------------------------------
 * 키보드 입력 처리
 *------------------------------------------------------------------------------
 */
void ProcessKey(void)
{
	D3DXMATRIXA16* pmatView = g_pCamera->GetViewMatrix();		// 카메라 행렬을 얻는다.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// 카메라 행렬 셋팅

}

void AAction()
{
	g_PhyObject.Move();
	g_PhyBall1.Move();
	g_PhyBall2.Move();
	g_PhyBall3.Move();
	g_PhyEnemy.Move();
	g_PhyObject.Collision(&g_PhyBall1);
	g_PhyObject.Collision(&g_PhyBall2);
	g_PhyObject.Collision(&g_PhyBall3);
	g_PhyEnemy.CollisionEnemy(&g_PhyBall1);
	g_PhyEnemy.CollisionEnemy(&g_PhyBall2);
	g_PhyEnemy.CollisionEnemy(&g_PhyBall3);

	// 카메라
	ProcessKey();
}

//-----------------------------------------------------------------------------
// 이름: WinMain()
// 기능: 프로그램의 시작점 
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스 변수 선언 및 설정 
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"D3D Game", NULL };
	// 윈도우 클래스 등록 
	RegisterClassEx(&wc);

	// 윈도우 생성 
	HWND hWnd = CreateWindow("D3D Game", "미니 게임",
		WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	g_pCamera = new ZCamera;

	// Direct3D 초기화에 성공하면 진행하고, 실패하면 종료한다.
	if (SUCCEEDED(InitD3D(hWnd)) &&       // Direct3D의 초기화도 성공하고 
		SUCCEEDED(InitGeometry()) &&         // 버텍스 버퍼 생성도 성공하고
		SUCCEEDED(InitGeometryTexture()) &&  // 텍스쳐 버텍스 버퍼 생성도 성공하고 
		SUCCEEDED(InitMeshData()) &&
		SUCCEEDED(InitGameData()))// 기타 게임 데이타 로드 
	{
		// 윈도우 출력 
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		// 메시지 루프 시작하기
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT)
		{
			// 메시자가 있으면 가져 온다. 
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				AAction();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
	}

	delete g_pCamera;

	UnregisterClass("D3D Game", wc.hInstance);
	return 0;
}
