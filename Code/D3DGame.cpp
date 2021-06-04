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
// ���� ���� 
ZCamera* g_pCamera = NULL;	// Camera Ŭ����

LPDIRECT3D9             g_pD3D = NULL; // Direct3D ��ü 
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // ������ ��ġ (����ī��)

LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // ���ؽ� ���� 
PDIRECT3DVERTEXBUFFER9  g_pVBTexture = NULL; // �ؽ��� ��¿���ؽ� ����
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // �ؽ��� �ε��� ����

CXFileUtil              g_XFile;   // X ���� ����� ���� Ŭ���� ��ü 
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

// ��������Ʈ ó���� ���� ����ü 
struct SPRITE {
	int spriteNumber;   // ��ü ��������Ʈ �̹��� ��
	int curIndex;       // ���� ����ؾ� �ϴ� ��������Ʈ �ε��� 
	int frameCounter;   // ���� ��������Ʈ�� ����ϰ� ���ӵ� ������ �� 
	int frameDelay;     // ��������Ʈ ���� �ӵ� ������ ���� ������ ������ 
};
// ��������Ʈ ����ü ���� ���� �� �ʱ�ȭ
SPRITE g_Fire = { 15, 0, 0, 2 }; // ���� & ����ü�� �� ��� �ʱ�ȭ 

// Ŀ���� ���ؽ� Ÿ�� ����ü 
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D ��ǥ��
	DWORD color;      // ���ؽ� ����
};

// Ŀ���� ���ؽ��� ������ ǥ���ϱ� ���� FVF(Flexible Vertex Format) �� 
// D3DFVF_XYZ(3D ���� ��ǥ) ��  D3DFVF_DIFFUSE(���� ����) Ư���� ��������.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct TEXTUREVERTEX
{
	D3DXVECTOR3     position;  // ���ؽ��� ��ġ
	D3DCOLOR        color;     // ���ؽ��� ����
	FLOAT           tu, tv;    // �ؽ��� ��ǥ 
};

// �� ����ü�� ������ ǥ���ϴ� FVF �� ����
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct MYVERTEX
{
	D3DXVECTOR3     position;  // ���ؽ��� ��ġ
	D3DCOLOR        color;     // ���ؽ��� ����
};

struct MYINDEX
{
	WORD	_0, _1, _2;
};

// �� ����ü�� ������ ǥ���ϴ� FVF �� ����
#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



//-----------------------------------------------------------------------------
// �̸�: InitD3D()
// ���: Direct3D �ʱ�ȭ 
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Direct3D ��ü ���� 
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// ��ġ ������ ����Ÿ �غ�

	D3DPRESENT_PARAMETERS d3dpp;         // ��ġ ������ ���� ����ü ���� ����

	ZeroMemory(&d3dpp, sizeof(d3dpp));                  // ����ü Ŭ����
	d3dpp.BackBufferWidth = 1024;               // ���� �ػ� ���� ����
	d3dpp.BackBufferHeight = 768;               // ���� �ػ� ���� ���� 
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;   // ���� ���� ���� 
	d3dpp.BackBufferCount = 1;                 // ����� �� 
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // ���� ��� ����
	d3dpp.hDeviceWindow = hWnd;              // ������ �ڵ� ���� 
	d3dpp.Windowed = true;              // ������ ���� ���� �ǵ��� �� 
	d3dpp.EnableAutoDepthStencil = true;              // ���Ľ� ���۸� ����ϵ��� �� 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;      // ���Ľ� ���� ���� ���� 


	// D3D��ü�� ��ġ ���� �Լ� ȣ�� (����Ʈ ����ī�� ���, HAL ���)
	
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// ���� ��ġ�� ���������� �����Ǿ���.

	// zbuffer ����ϵ��� ����
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	// �⺻�ø� ����
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	return S_OK;
}

//-----------------------------------------------------------------------------
// �̸�: InitGameData()
// ���: ���ӿ� ���õ� ���� �����͸� �ʱ�ȭ �Ѵ�. 
//-----------------------------------------------------------------------------
HRESULT InitGameData()
{
	// �� �� �������� ��ȯ ����
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
// �̸�: InitMeshData()
// ���: Mesh�� ���õ� ���� �����͸� �ʱ�ȭ �Ѵ�. 
//-----------------------------------------------------------------------------
HRESULT InitMeshData()
{
	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		randomSpeed[i] = rand() % 11 + 5; // 5 ~ 15
	}

	/*float ���� 3���� R,G,B�� ��Ÿ���ϴ�. 
	ball�� �Ķ��� ����� ���� �Լ��� �Ű� ������ �߰��Ͽ� �����߽��ϴ�.*/
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

	// Ball1�� ball2�� �����ϹǷ� Bounding volume�� ���� ����� �ʿ�� ����
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
// �̸�: InitGeometry()
// ���: ���ؽ� ���۸� ������ �� ���ؽ��� ä���. 
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// ���ؽ� ���ۿ� ���� ���ؽ� �ڷḦ �ӽ÷� �����. 
	CUSTOMVERTEX vertices[] =
	{
		{ -200.0f,  0.0f, 0.0f, 0xff00ff00, }, // x�� ������ ���� ���ؽ� 
	{ 200.0f,  0.0f, 0.0f, 0xff00ff00, },

	{ 0.0f, 0.0f, -200.0f, 0xffffff00, },  // z�� ������ ���� ���ؽ�
	{ 0.0f, 0.0f,  200.0f, 0xffffff00, },

	{ 0.0f, -200.0f, 0.0f, 0xffff0000, },  // y�� ������ ���� ���ؽ�
	{ 0.0f,  200.0f, 0.0f, 0xffff0000, },

	{ 0.0f, 50.0f, 0.0f, 0xffff0000, },  // �ﰢ���� ù ��° ���ؽ� 
	{ -50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰢ���� �� ��° ���ؽ� 
	{ 50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰡ���� �� ��° ���ؽ� 
	};

	// ���ؽ� ���۸� �����Ѵ�.
	// �� ���ؽ��� ������ D3DFVF_CUSTOMVERTEX ��� �͵� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// ���ؽ� ���ۿ� ���� �� �� ���ؽ��� �ִ´�. 
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}


//-----------------------------------------------------------------------------
// �̸�: InitGeometryTexture()
// ���: �ؽ��� ����� ���� ���ؽ� ���۸� ������ �� ���ؽ��� ä���. 
//-----------------------------------------------------------------------------
HRESULT InitGeometryTexture()
{
	// �ؽ��� �ε� 
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/Fire.bmp", &g_pTexture)))
	{
		return E_FAIL;
	}

	// ���ؽ� ���� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL)))
	{
		return E_FAIL;
	}

	// ���ؽ� ���� ���� 
	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	pVertices[0].position = D3DXVECTOR3(-50, 100, 200);  // ���ؽ� ��ġ 
	pVertices[0].color = 0xffffffff;                  // ���ؽ� ���� �� ���� 
	pVertices[0].tu = 0.0f;                        // ���ؽ� U �ؽ��� ��ǥ 
	pVertices[0].tv = 0.0f;                        // ���ؽ� V �ؽ��� ��ǥ 

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
// �̸�: Cleanup()
// ���: �ʱ�ȭ�Ǿ��� ��� ��ü���� �����Ѵ�. 
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();
	if (g_pVBTexture != NULL)
		g_pVBTexture->Release();
	if (g_pTexture != NULL)
		g_pTexture->Release();


	if (g_pd3dDevice != NULL)    // ��ġ ��ü ���� 
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)          // D3D ��ü ���� 
		g_pD3D->Release();

}



//-----------------------------------------------------------------------------
// �̸�: SetupViewProjection()
// ���: �� ��ȯ�� �������� ��ȯ�� �����Ѵ�. 
//-----------------------------------------------------------------------------
VOID SetupViewProjection()
{
	// �� ��ȯ ���� 
	D3DXVECTOR3 vEyePt(0.0f, 300.0f, -600.0f);    // ī�޶��� ��ġ 
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);       // �ٶ󺸴� ���� 
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);          // ������ ���� 
	D3DXMATRIXA16 matView;                           // �亯ȯ�� ��Ʈ���� 
													 // �� ��Ʈ���� ���� 
	//D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D ��ġ�� �� ��Ʈ���� ���� 
	//g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// �������� ��ȯ ���� 
	D3DXMATRIXA16 matProj;   // �������ǿ� ��Ʈ���� 
							 // �������� ��Ʈ���� ���� 
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	// Direct3D ��ġ�� �������� ��Ʈ���� ���� 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	/// ī�޶� �ʱ�ȭ
	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
	D3DXMATRIXA16* pmatView = g_pCamera->GetViewMatrix();		// ī�޶� ����� ��´�.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// ī�޶� ��� ����

}

// ������ �̸� ���� ������ ���ϴ�.
D3DCOLORVALUE black = { 0, 0, 0, 1 };
D3DCOLORVALUE dark_gray = { 0.2f, 0.2f, 0.2f, 1.0f };
D3DCOLORVALUE gray = { 0.5f, 0.5f, 0.5f, 1.0f };
D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
D3DCOLORVALUE blue = { 0.0f, 0.0f, 1.0f, 1.0f };
VOID SetupLight()
{
	D3DLIGHT9 light;                         // Direct3D 9 ���� ����ü ���� ����

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;       // ���� Ÿ���� �𷺼ųη� ����
	light.Diffuse = white;                   // ������ �� ����
	light.Specular = white;
	light.Direction = D3DXVECTOR3(10, -10, 10);       //  ������ ���� (�����ϴ� ����) 
													  //light.Direction = D3DXVECTOR3(20*sin(g_counter*0.01f), -10, 10);       //  ������ ���� (�����ϴ� ����) 
													  //light.Direction = D3DXVECTOR3(10, 25, -40);       //  ������ ���� (�����ϴ� ����) 
	g_pd3dDevice->SetLight(0, &light);      // ����Ʈ ��ȣ ���� (���⿡���� 0��)
	g_pd3dDevice->LightEnable(0, TRUE);     // 0�� ����Ʈ �ѱ�


											// ����Ʈ ��� ����� TRUE�� ��. (�� ����� ���� ��� ����Ʈ ����� ������)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	// ���������� �����Ʈ ����Ʈ �ѱ� (ȯ�汤�� ���� ����)
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00303030 );
}

//-----------------------------------------------------------------------------
// �̸�: ChangeSpriteUV()
// ���: ��������Ʈ�� ���� uv ���� �Լ�  
//-----------------------------------------------------------------------------
HRESULT ChangeSpriteUV(SPRITE* sp)
{
	// for tree texture

	float u = (sp->curIndex * 64.0f) / 960.0f;       // ���� �ε����� �̿��� u ��� 
	float u2 = ((sp->curIndex + 1) * 64.0f) / 960.0f;  // ���� �ε���+1�� �̿��� u ���

	TEXTUREVERTEX* pVertices;   // ���ؽ� ���� ���ٿ� ������
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	pVertices[0].tu = u;     // u ��ǥ ���� 
	pVertices[0].tv = 0.0f;  // v ��ǥ ���� 

	pVertices[1].tu = u2;
	pVertices[1].tv = 0.0f;

	pVertices[2].tu = u;
	pVertices[2].tv = 1.0f;

	pVertices[3].tu = u2;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	// ������ ������ �������� ���� ��� 
	if (sp->frameCounter >= sp->frameDelay) {
		sp->curIndex = (sp->curIndex + 1) % sp->spriteNumber; // �ε��� ���� 
		sp->frameCounter = 0;   // ������ ī���� �ʱ�ȭ 
	}
	else  // ���� ������ �ð��� �ȵ� ��� 
		sp->frameCounter++;    // ������ ī���� ���� 

	return S_OK;

}

//-----------------------------------------------------------------------------
// �̸�: Render()
// ���: ȭ���� �׸���.
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)  // ��ġ ��ü�� �������� �ʾ����� ���� 
		return;

	// �� �� �������� ��ȯ ����
	//SetupViewProjection();

	// �ﰢ���� ��/�� ���� ��� �������ϵ��� �ø� ����� ����.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// ������ ����. (���� ���� ������ �̳���, ���ؽ� ��ü�� ������ ����ϵ���)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// ����۸� ������ �������� �����.
	// ����۸� Ŭ����
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// ȭ�� �׸��� ���� 
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		///// ���ؽ� ��� 
		// ���ؽ� ���� ���� 
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // ���ؽ� ���� ���� 

		D3DXMATRIXA16 matWorld;  // ���� ��ȯ�� ��Ʈ���� ���� 

		for (float x = -200; x <= 200; x += 20) {  // z �࿡ ������ ������ ���� �� �׸��� 
			D3DXMatrixTranslation(&matWorld, x, 0.0, 0.0);  // x�࿡ ���� ��ġ �̵� ��Ʈ����   
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld); // ��ȯ��Ʈ���� ���� 
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);  // z�� ���� �׸��� 
		}

		for (float z = -200; z <= 200; z += 20) {  // x �࿡ ������ ������ ���� �� �׸��� 
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);  // z �࿡ ���� ��ġ �̵� ��Ʈ����
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // ��ȯ��Ʈ���� ���� 
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);   // x�� ���� �׸��� 
		}

		D3DXMatrixIdentity(&matWorld);   // ��Ʈ������ ���� ��ķ� ���� 
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // ��ȯ ��Ʈ���� ���� 
		//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);   // y �� �׸��� 


		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

		SetupLight();

		// X ���� ���
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

		if (g_PhyEnemy.HPCheck() <= 0) { // �� HP�� 0�̵Ǹ�
			TCHAR msg[100];
			sprintf_s(msg, "�� óġ ����! ���� HP %d", g_PhyObject.HPCheck());
			if (MessageBox(NULL, msg, "GAME SUCCESS", MB_OK) == IDOK) {
				exit(0); // �÷��̾� ���� HP�� �˷��ִ� �޼����ڽ�
			}
		}

		if (g_PhyObject.HPCheck() <= 0) { // �÷��̾� HP�� 0�̵Ǹ�
			TCHAR msg[100];
			sprintf_s(msg, "���� �� HP %d", g_PhyEnemy.HPCheck());
			if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
				exit(0); // ���� ���� HP�� �˷��ִ� �޼����ڽ�
			}
		}

		// ball1
		matWorld = g_PhyBall1.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall1.Zposition() < -400) { // �� ���߰� �� �ڷ� �Ѿ� ���� �� �ʱ�ȭ
			g_PhyObject.HPReduction(1); // �÷��̾� HP ����
			g_PhyBall1.SetPosition(-100, 0, 180); // ��ġ �ʱ�ȭ
			g_PhyBall1.SetVelocity(0, 0, -g_PhyBall1.RandomSpeed()); // ���ǵ� ���� �ʱ�ȭ
		}
		if (g_PhyBall1.Zposition() > 500) { // ������ �� �ʱ�ȭ
			g_PhyEnemy.HPReduction(1); // �� HP ����
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision()); // �浹 Ƚ�� �ʱ�ȭ
			g_PhyBall1.SetPosition(-100, 0, 180);// ��ġ �ʱ�ȭ
			g_PhyBall1.SetVelocity(0, 0, -g_PhyBall1.RandomSpeed()); // ���ǵ� ���� �ʱ�ȭ
		}
		g_XBall1.XFileDisplay(g_pd3dDevice);

		// ball2
		matWorld = g_PhyBall2.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall2.Zposition() < -400) { // �� ���߰� �ִϺ�� �ڷ� �Ѿ� ���� �� �ʱ�ȭ
			g_PhyObject.HPReduction(1);
			if (g_PhyObject.HPCheck() <= 0) {
				TCHAR msg[100];
				sprintf_s(msg, "���� �� HP %d", g_PhyEnemy.HPCheck());
				if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
					exit(0);
				}
			}
			g_PhyBall2.SetPosition(0, 0, 180);
			g_PhyBall2.SetVelocity(0, 0, -g_PhyBall2.RandomSpeed());
		}
		if (g_PhyBall2.Zposition() > 500) { // ������ �� �ʱ�ȭ
			g_PhyEnemy.HPReduction(1);
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision());
			g_PhyBall2.SetVelocity(0, 0, -g_PhyBall2.RandomSpeed());
			g_PhyBall2.SetPosition(0, 0, 180);
		}
		g_XBall2.XFileDisplay(g_pd3dDevice);

		// ball3
		matWorld = g_PhyBall3.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyBall3.Zposition() < -400) { // �� ���߰� �ִϺ�� �ڷ� �Ѿ� ���� �� �ʱ�ȭ
			g_PhyObject.HPReduction(1);
			if (g_PhyObject.HPCheck() <= 0) {
				TCHAR msg[100];
				sprintf_s(msg, "���� �� HP %d", g_PhyEnemy.HPCheck());
				if (MessageBox(NULL, msg, "GAME OVER", MB_OK) == IDOK) {
					exit(0);
				}
			}
			g_PhyBall3.SetPosition(100, 0, 180);
			g_PhyBall3.SetVelocity(0, 0, -g_PhyBall3.RandomSpeed());
		}
		if (g_PhyBall3.Zposition() > 500) { // ������ �� �ʱ�ȭ
			g_PhyEnemy.HPReduction(1);
			g_PhyEnemy.SetCollision(g_PhyEnemy.NumberCollision());
			g_PhyBall3.SetVelocity(0, 0, -g_PhyBall3.RandomSpeed());
			g_PhyBall3.SetPosition(100, 0, 180);
		}
		g_XBall3.XFileDisplay(g_pd3dDevice);

		

		// �ؽ�ó ���� ����

		// ���� ����
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// �ؽ��� ���� (�ؽ��� ������ ���Ͽ� g_pTexture�� ����Ͽ���.)
		g_pd3dDevice->SetTexture(0, g_pTexture);

		// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // TRUE �� ����
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		// ����� ���ؽ� ���� ����
		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));
		// FVF �� ����
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);
		// �簢�� ���� (�ﰢ�� 2���� �̿��Ͽ� �簢�� ������ �������) ��� 
		D3DXMatrixIdentity(&matWorld);   // ��Ʈ������ ���� ��ķ� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		if (g_PhyEnemy.NumberCollision()>=1) {
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		}
		
		

		// �ؽ��� ���� ����
		g_pd3dDevice->SetTexture(0, NULL);

		// ���� ��������Ʈ �ִϸ��̼��� ���� uv �ִϸ��̼� �Լ� ȣ��
		ChangeSpriteUV(&g_Fire);

		// ȭ�� �׸��� �� 
		g_pd3dDevice->EndScene();
	}

	// ������� ������ ȭ������ ������. 
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



//-----------------------------------------------------------------------------
// �̸� : MsgProc()
// ��� : ������ �޽��� �ڵ鷯 
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool move = false;
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();   // ���α׷� ����� ��ü ������ ���Ͽ� ȣ���� 
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		Render();    // ȭ�� ����� ����ϴ� ������ �Լ� ȣ�� 
		ValidateRect(hWnd, NULL);
		return 0;
	// Ű���� �Է�
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			TCHAR msg[100];
			sprintf_s(msg,"�� HP %d �� HP %d", g_PhyObject.HPCheck(),g_PhyEnemy.HPCheck());
			MessageBox(NULL, msg, "HP Ȯ��", MB_OK);
			break;
		case VK_LEFT:
			g_PhyObject.SetXposition(-100);
			break;
		case VK_RIGHT:
			g_PhyObject.SetXposition(100);
			break;
		case VK_ESCAPE:
			if (MessageBox(NULL, "������ ���� �Ͻðڽ��ϱ�?", "���� ���� â", MB_YESNO) == IDYES)
				exit(0);
			break;
		case VK_UP:
			break;
		}
		break;
	// ���콺 �Է� (ī�޶�)
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
 * Ű���� �Է� ó��
 *------------------------------------------------------------------------------
 */
void ProcessKey(void)
{
	D3DXMATRIXA16* pmatView = g_pCamera->GetViewMatrix();		// ī�޶� ����� ��´�.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// ī�޶� ��� ����

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

	// ī�޶�
	ProcessKey();
}

//-----------------------------------------------------------------------------
// �̸�: WinMain()
// ���: ���α׷��� ������ 
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// ������ Ŭ���� ���� ���� �� ���� 
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"D3D Game", NULL };
	// ������ Ŭ���� ��� 
	RegisterClassEx(&wc);

	// ������ ���� 
	HWND hWnd = CreateWindow("D3D Game", "�̴� ����",
		WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	g_pCamera = new ZCamera;

	// Direct3D �ʱ�ȭ�� �����ϸ� �����ϰ�, �����ϸ� �����Ѵ�.
	if (SUCCEEDED(InitD3D(hWnd)) &&       // Direct3D�� �ʱ�ȭ�� �����ϰ� 
		SUCCEEDED(InitGeometry()) &&         // ���ؽ� ���� ������ �����ϰ�
		SUCCEEDED(InitGeometryTexture()) &&  // �ؽ��� ���ؽ� ���� ������ �����ϰ� 
		SUCCEEDED(InitMeshData()) &&
		SUCCEEDED(InitGameData()))// ��Ÿ ���� ����Ÿ �ε� 
	{
		// ������ ��� 
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		// �޽��� ���� �����ϱ�
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT)
		{
			// �޽��ڰ� ������ ���� �´�. 
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
