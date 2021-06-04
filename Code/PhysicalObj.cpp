// PhysicalObj.cpp: implementation of the PhysicalObj class.
//
//////////////////////////////////////////////////////////////////////

#include "PhysicalObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// MEMO
// speed 5~15: 15, 13, 11, 9, 9, 7, 7, 7, 6, 6, 5
// left : speed [(14,15) 2.7, 1] [13, 2.6, 1] [12, 2.4 , 1] [11, 2.2 , 1] [10, 2, 1] [9, 1.8, 1] [8, 1.6, 1] [7, 1.4, 2] [6, 1.2 , 2] [5, 1, 3]
// right : 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3
// damage : 1, 2, 3, 4 ,5, 6, 7

PhysicalObj::PhysicalObj(float x, float y, float z)
{
	p.x = p.y = p.z = 0;
	v.x = v.y = v.z = 0;
	a.x = a.y = a.z = 0;
	a.y = -.8f;
	clock = 0;
	scale = 50;
	min = max = D3DXVECTOR3(0,0,0);
	rotation = 0;
	n_collision = 0;
	HP = 0;
	enemyExt = false;
}

PhysicalObj::~PhysicalObj()
{

}

void PhysicalObj::SetHP(int hp)
{
	HP = hp;
}

void PhysicalObj::HPReduction(int damage)
{
	HP -= damage;
}

int PhysicalObj::HPCheck()
{
	return HP;
}

void PhysicalObj::SetScale(float s)
{
	scale = s;
}

void PhysicalObj::SetRotation(float r) 
{
	rotation = r;
}

void PhysicalObj::SetPosition(float x, float y, float z)
{
	p.x = x;
	p.y = y;
	p.z = z;
}
// 애니비아 좌우 이동
void PhysicalObj::SetXposition(float x) 
{
	float isOK = p.x + x;
	if (isOK == 200) {
		p.x = 100;
	}
	else if (isOK == -200) {
		p.x = -100;
	}
	else {
		p.x = isOK;
	}
}

float PhysicalObj::Xposition() {
	return p.x;
}

float PhysicalObj::Zposition() {
	return p.z;
}

void PhysicalObj::SetVelocity(float x, float y, float z)
{
	v.x = x;
	v.y = y;
	v.z = z;
}
void PhysicalObj::AddVelocity(float x, float y, float z)
{
	v.x += x;
	v.y += y;
	v.z += z;
}

void PhysicalObj::SetAcceleration(float x, float y, float z)
{
	a.x = x;
	a.y = y;
	a.z = z;
}

void PhysicalObj::BoundCheck()
{
	float e = 1.0f;
/*
	if(p.y <= 0) {
		if( fabs(v.y) < 1.0 ) { // stop condition
			p.y = 0;
			v.y = 0;
		} else {
			v.y = (float)fabs(v.y) * e;
		}
	}
	if(p.x < -200) {
		p.x = -200;
		v.x = (float)fabs(v.x) * e;
	}
	if(p.x > 200) {
		p.x = 200;
		v.x = (float)-fabs(v.x) * e;
	}
	if(p.z < -200) {
		p.z = -200;
		v.z = (float)fabs(v.z) * e;
	}
	if(p.z > 200) {
		p.z = 200;
		v.z = (float)-fabs(v.z) * e;
	}
*/

	if(p.y + min.y< 0) {
		if( fabs(v.y) < 1.0) { // stop condition
			p.y = -min.y;
			v.y = 0;
		} else {
			v.y = (float)fabs(v.y) * e;
		}
	}
	if(p.x + min.x < -200) {
		p.x = -200 - min.x;
		v.x = (float)fabs(v.x) * e;
	}
	if(p.x + max.x > 200) {
		p.x = 200 - max.x;
		v.x = (float)-fabs(v.x) * e;
	}/*
	if(p.z + min.z < -200) {
		p.z = -200 - min.z;
		v.z = (float)fabs(v.z) * e;
	}
	if(p.z + max.z > 200) {
		p.z = 200 - max.z;
		v.z = (float)-fabs(v.z) * e;
	}*/

}
void PhysicalObj::Move(float current)
{

	if(current == -1) { // defafult

		p.x += v.x + 0.5f*a.x;
		p.y += v.y + 0.5f*a.y;
		p.z += v.z + 0.5f*a.z;

		v.x += a.x;
		v.y += a.y;
		v.z += a.z;
	}

	BoundCheck();

}

void PhysicalObj::Collision(PhysicalObj *target)
{
	D3DXVECTOR3 distance = (p+center) - (target->p + target->center); // 엄밀하게는 scale도 포함!
	float length = D3DXVec3Length(&distance);
	float rsum = radius + target->radius;
	if(rsum > length) { // collision!
		//MessageBox(NULL, "충돌", "충돌", MB_OK);

		////exit(0);
		//D3DXVECTOR3 d = (target->p + target->center) - (p + center); // normal
		//D3DXVec3Normalize(&d, &d);

		//D3DXVECTOR3 d1 = D3DXVec3Dot(&d, &v) * d;
		//D3DXVECTOR3 t1 = v - d1;

		//D3DXVECTOR3 d2 = D3DXVec3Dot(&d, &target->v) * d;
		//D3DXVECTOR3 t2 = target->v - d2;

		//v = d2 + t1;
		
		// Collision 함수
		// 왼쪽
		if (p.x <= -100) { 
			if (target->v.z <= -14.0f) {
				target->v.x = 2.7; 
			}
			else if (target->v.z <= -13.0f) {
				target->v.x = 2.6;
			}
			else if (target->v.z <= -12.0f) {
				target->v.x = 2.4;
			}
			else if (target->v.z <= -11.0f) {
				target->v.x = 2.2;
			}
			else if (target->v.z <= -10.0f) {
				target->v.x = 2.0;
			}
			else if (target->v.z <= -9.0f) {
				target->v.x = 1.8;
			}
			else if (target->v.z <= -8.0f) {
				target->v.x = 1.6;
			}
			else if (target->v.z <= -7.0f) {
				target->v.x = 1.4;
			}
			else if (target->v.z <= -6.0f) {
				target->v.x = 1.2;
			}
			else {
				target->v.x = 1.0;
			}
			target->v.z = -target->v.z;
		}
		// 오른쪽
		else if(p.x >= 100) { 
			if (target->v.z <= -15.0f) {
				target->v.x = -2.7;
			}
			else if (target->v.z <= -14.0f) {
				target->v.x = -2.5;
			}
			else if (target->v.z <= -13.0f) {
				target->v.x = -2.4;
			}
			else if (target->v.z <= -12.0f) {
				target->v.x = -2.2;
			}
			else if (target->v.z <= -11.0f) {
				target->v.x = -2.1;
			}
			else if (target->v.z <= -10.0f) {
				target->v.x = -1.9;
			}
			else if (target->v.z <= -9.0f) {
				target->v.x = -1.7;
			}
			else if (target->v.z <= -8.0f) {
				target->v.x = -1.5;
			}
			else if (target->v.z <= -7.0f) {
				target->v.x = -1.3;
			}
			else if (target->v.z <= -6.0f){ // 충돌 3
				target->v.x = -1.1;
			}else{ // 충돌 3
				target->v.x = -0.9; 
			}
			target->v.z = -target->v.z;
		}
		else {
			target->v.z = -target->v.z;
		}

	}

}

int PhysicalObj::NumberCollision()
{
	return n_collision;
}

void PhysicalObj::CollisionEnemy(PhysicalObj* target) 
{
	D3DXVECTOR3 distance = (p + center) - (target->p + target->center); // 엄밀하게는 scale도 포함!
	float length = D3DXVec3Length(&distance);
	float rsum = radius + target->radius;
	//CollisionEnemy
	if (rsum >= length) {
		n_collision++;
		//MessageBox(NULL, "충돌", "충돌", MB_OK);

		//exit(0);
		//D3DXVECTOR3 d = (target->p + target->center) - (p + center); // normal
		//D3DXVec3Normalize(&d, &d);

		//D3DXVECTOR3 d1 = D3DXVec3Dot(&d, &v) * d;
		//D3DXVECTOR3 t1 = v - d1;

		//D3DXVECTOR3 d2 = D3DXVec3Dot(&d, &target->v) * d;
		//D3DXVECTOR3 t2 = target->v - d2;

		//v = d2 + t1;
		//target->v = -v;
	}
}

void PhysicalObj::SetCollision(int n)
{
	n_collision -= n;
}

int PhysicalObj::RandomSpeed() {
	srand(time(NULL));
	return rand() % 11 + 5; // 5~15
}

void PhysicalObj::SetBoundingBox(D3DXVECTOR3 m, D3DXVECTOR3 M)
{
	min = m*scale;
	max = M*scale;
}

void PhysicalObj::SetBoundingSphere(D3DXVECTOR3 c, float r)
{
	center = c * scale;
	radius = r*scale;
}

D3DXMATRIXA16 PhysicalObj::GetWorldMatrix()
{
	D3DXMATRIXA16 matWorld, matScale, matRotation;
	D3DXMatrixTranslation(&matWorld, p.x, p.y, p.z);
	D3DXMatrixRotationY(&matRotation, rotation);
	D3DXMatrixScaling(&matScale, scale, scale, scale);

	D3DXMatrixMultiply(&matWorld, &matScale, &matWorld);
	D3DXMatrixMultiply(&matWorld, &matRotation, &matWorld);
	return matWorld;
}