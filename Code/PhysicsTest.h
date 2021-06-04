#pragma once
#include <Eigen/dense>
#include "d3dx9.h"
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D 좌표값
	DWORD color;      // 버텍스 색상
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

using namespace std;
using namespace Eigen;

class Particle {
public:
	float m;
	Vector3d p, v, a; // 위치, 속도, 가속도
	Vector3d f; // 힘
};
class Spring {
public:
	static float ks; // 스프링상수. 숫자가 크면 단단한 스프링이다
	static float kd; // damping 상수. 숫자가 크면 빨리 수렴한다

	static float r; // 자연상태의 길이
	int i, j; // i, j는 각각 particle 번호
};
float Spring::ks = 250.0;
float Spring::kd = 14.5;
float Spring::r = 10;

class PhysicsSystem {
	static const int SIZE = 10;
	LPDIRECT3DVERTEXBUFFER9 _pVB;
	LPDIRECT3DINDEXBUFFER9 _pIB;
public:
	Particle p[SIZE * SIZE];
	Spring s[2 * (SIZE - 1) * SIZE];
	PhysicsSystem();
	~PhysicsSystem() { DeleteDXBuffers(); }
	void CreateDXBuffers(LPDIRECT3DDEVICE9 pd3dDevice);
	void DeleteDXBuffers(); // 명시적으로 호출해도 좋다.

	// 엔진 내부에서 힘을 계산, 위치 계산
	void CalcForce();
	void Move(float dt);
	// DX 버퍼를 갱신, 그림 그리기
	void UpdateBuffers();
	void RenderBuffers(LPDIRECT3DDEVICE9 pd3dDevice);
};

// dx를 이용하여 VB, IB를 화면에 그린다
void PhysicsSystem::RenderBuffers(LPDIRECT3DDEVICE9 pd3dDevice) {
	pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(CUSTOMVERTEX));
	pd3dDevice->SetIndices(_pIB);
	pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // 버텍스 포멧 지정 
	pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, SIZE * SIZE, 0, (SIZE - 1) * SIZE * 2);
}
// 물리 엔진에 저장된 정보를 DX buffer에 저장한다.
void PhysicsSystem::UpdateBuffers() {
	// 정점버퍼
	CUSTOMVERTEX* pVertices;
	if (FAILED(_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return;
	for (int i = 0; i < SIZE * SIZE; i++) {
		pVertices[i].x = (float)p[i].p[0]; // 0, 1, 2 : x, y, z
		pVertices[i].y = (float)p[i].p[1];
		pVertices[i].z = (float)p[i].p[2];
		pVertices[i].color = 0x0000ffff; // cyan
	}
	_pVB->Unlock();
}
void PhysicsSystem::DeleteDXBuffers() {
	if (_pVB) {
		_pVB->Release();
		_pVB = NULL;
	}
	if (_pIB) {
		_pIB->Release();
		_pIB = NULL;
	}
}
void PhysicsSystem::CreateDXBuffers(LPDIRECT3DDEVICE9 pd3dDevice) {

	HRESULT hr = pd3dDevice->CreateVertexBuffer(SIZE * SIZE * sizeof(CUSTOMVERTEX), 0,
		D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &_pVB, NULL);
	if (FAILED(hr))
		return;
	hr = pd3dDevice->CreateIndexBuffer((SIZE - 1) * SIZE * 2 * 2 * sizeof(unsigned short), 0,
		D3DFMT_INDEX16, D3DPOOL_DEFAULT, &_pIB, NULL);
	if (FAILED(hr))
		return;

	// 인덱스 버퍼는 한번만 값을 입력하면 변경될 경우 없다.
	unsigned short* pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);
	for (int i = 0; i < 2 * (SIZE - 1) * SIZE; i++) {
		pIndices[i * 2 + 0] = s[i].i;
		pIndices[i * 2 + 1] = s[i].j;
	}
	_pIB->Unlock();
}
// 각 입자에 대해서 움직임 계산
void PhysicsSystem::Move(float dt) {
	for (int i = 0; i < SIZE * SIZE; i++) {
		// f = ma, a = f / m = 
		p[i].a = p[i].f / p[i].m;
		p[i].v += p[i].a * dt;
		p[i].p += p[i].v * dt;
	}
}
// 각 입자에 반영되는 힘의 총합 계산
void PhysicsSystem::CalcForce() {
	// 힘의 초기화
	for (int i = 0; i < SIZE * SIZE; i++) {
		p[i].f = Vector3d(0, -9.8 * p[i].m, 0); // p[i].m == 1 가정
	}
	for (int i = 0; i < 2 * (SIZE - 1) * SIZE; i++) {
		int from = s[i].i;
		int to = s[i].j;
		Vector3d dis = p[to].p - p[from].p; // 두 물체 사이의 변위
		// f = kx 
		double x = dis.norm() / s[i].r - 1; // 정상상태보다 몇 배 늘어났는가
		Vector3d force = s[i].ks * x * (dis / dis.norm());

		// damping start
		dis.normalize(); // 단위벡터로 만든다
		Vector3d damping =
			s[i].kd * ((p[to].v - p[from].v) / s[i].r).dot(dis)
			* dis;
		force += damping;
		// damping end
		p[from].f += force;
		p[to].f -= force;
	}
	// 고정점을 설치한다.
	p[0].f = Vector3d(0, 0, 0);
	p[SIZE - 1].f = Vector3d(0, 0, 0);

}
PhysicsSystem::PhysicsSystem() {
	// DX buffer 초기화
	_pVB = NULL;
	_pIB = NULL;

	// particle 기본 설정
	Vector3d vel(0, 0, 0), acc(0, 0, 0), force(0, 0, 0);
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			Vector3d pos(x * 10, y * 10, 0);
			p[y * SIZE + x] = { 1, pos, vel, acc, force };
		}
	}
	// 가로방향 spring 기본 설정
	int index = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE - 1; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + 1;
			index++;
		}
	}
	// 세로방향 spring 기본 설정
	for (int y = 0; y < SIZE - 1; y++) {
		for (int x = 0; x < SIZE; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + SIZE;
			index++;
		}
	}
}