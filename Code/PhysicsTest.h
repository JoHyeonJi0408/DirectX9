#pragma once
#include <Eigen/dense>
#include "d3dx9.h"
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D ��ǥ��
	DWORD color;      // ���ؽ� ����
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

using namespace std;
using namespace Eigen;

class Particle {
public:
	float m;
	Vector3d p, v, a; // ��ġ, �ӵ�, ���ӵ�
	Vector3d f; // ��
};
class Spring {
public:
	static float ks; // ���������. ���ڰ� ũ�� �ܴ��� �������̴�
	static float kd; // damping ���. ���ڰ� ũ�� ���� �����Ѵ�

	static float r; // �ڿ������� ����
	int i, j; // i, j�� ���� particle ��ȣ
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
	void DeleteDXBuffers(); // ��������� ȣ���ص� ����.

	// ���� ���ο��� ���� ���, ��ġ ���
	void CalcForce();
	void Move(float dt);
	// DX ���۸� ����, �׸� �׸���
	void UpdateBuffers();
	void RenderBuffers(LPDIRECT3DDEVICE9 pd3dDevice);
};

// dx�� �̿��Ͽ� VB, IB�� ȭ�鿡 �׸���
void PhysicsSystem::RenderBuffers(LPDIRECT3DDEVICE9 pd3dDevice) {
	pd3dDevice->SetStreamSource(0, _pVB, 0, sizeof(CUSTOMVERTEX));
	pd3dDevice->SetIndices(_pIB);
	pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // ���ؽ� ���� ���� 
	pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, SIZE * SIZE, 0, (SIZE - 1) * SIZE * 2);
}
// ���� ������ ����� ������ DX buffer�� �����Ѵ�.
void PhysicsSystem::UpdateBuffers() {
	// ��������
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

	// �ε��� ���۴� �ѹ��� ���� �Է��ϸ� ����� ��� ����.
	unsigned short* pIndices;
	_pIB->Lock(0, 0, (void**)&pIndices, 0);
	for (int i = 0; i < 2 * (SIZE - 1) * SIZE; i++) {
		pIndices[i * 2 + 0] = s[i].i;
		pIndices[i * 2 + 1] = s[i].j;
	}
	_pIB->Unlock();
}
// �� ���ڿ� ���ؼ� ������ ���
void PhysicsSystem::Move(float dt) {
	for (int i = 0; i < SIZE * SIZE; i++) {
		// f = ma, a = f / m = 
		p[i].a = p[i].f / p[i].m;
		p[i].v += p[i].a * dt;
		p[i].p += p[i].v * dt;
	}
}
// �� ���ڿ� �ݿ��Ǵ� ���� ���� ���
void PhysicsSystem::CalcForce() {
	// ���� �ʱ�ȭ
	for (int i = 0; i < SIZE * SIZE; i++) {
		p[i].f = Vector3d(0, -9.8 * p[i].m, 0); // p[i].m == 1 ����
	}
	for (int i = 0; i < 2 * (SIZE - 1) * SIZE; i++) {
		int from = s[i].i;
		int to = s[i].j;
		Vector3d dis = p[to].p - p[from].p; // �� ��ü ������ ����
		// f = kx 
		double x = dis.norm() / s[i].r - 1; // ������º��� �� �� �þ�°�
		Vector3d force = s[i].ks * x * (dis / dis.norm());

		// damping start
		dis.normalize(); // �������ͷ� �����
		Vector3d damping =
			s[i].kd * ((p[to].v - p[from].v) / s[i].r).dot(dis)
			* dis;
		force += damping;
		// damping end
		p[from].f += force;
		p[to].f -= force;
	}
	// �������� ��ġ�Ѵ�.
	p[0].f = Vector3d(0, 0, 0);
	p[SIZE - 1].f = Vector3d(0, 0, 0);

}
PhysicsSystem::PhysicsSystem() {
	// DX buffer �ʱ�ȭ
	_pVB = NULL;
	_pIB = NULL;

	// particle �⺻ ����
	Vector3d vel(0, 0, 0), acc(0, 0, 0), force(0, 0, 0);
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE; x++) {
			Vector3d pos(x * 10, y * 10, 0);
			p[y * SIZE + x] = { 1, pos, vel, acc, force };
		}
	}
	// ���ι��� spring �⺻ ����
	int index = 0;
	for (int y = 0; y < SIZE; y++) {
		for (int x = 0; x < SIZE - 1; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + 1;
			index++;
		}
	}
	// ���ι��� spring �⺻ ����
	for (int y = 0; y < SIZE - 1; y++) {
		for (int x = 0; x < SIZE; x++) {
			s[index].i = y * SIZE + x;
			s[index].j = y * SIZE + x + SIZE;
			index++;
		}
	}
}