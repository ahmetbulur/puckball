//-----------------------------------------------------------------
// Sprite Object
// C++ Header - Sprite.h
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include <windows.h>
#include "Bitmap.h"
#include <cmath>

//-----------------------------------------------------------------
// Custom Data Types
//-----------------------------------------------------------------
typedef WORD        SPRITEACTION;
const SPRITEACTION  SA_NONE = 0x0000L,
SA_KILL = 0x0001L,
SA_ADDSPRITE = 0x0002L;

typedef WORD        BOUNDSACTION;
const BOUNDSACTION  BA_STOP = 0,
BA_WRAP = 1,
BA_BOUNCE = 2,
BA_DIE = 3;
const int _iStableVelocity = 6;
//-----------------------------------------------------------------
// Sprite Class
//-----------------------------------------------------------------
class Sprite
{
protected:
	// Member Variables
	Bitmap* m_pBitmap;
	int           m_iNumFrames, m_iCurFrame;
	int           m_iFrameDelay, m_iFrameTrigger;
	RECT          m_rcPosition,
		m_rcCollision;
	POINT         m_ptVelocity;
	int           m_iZOrder;
	RECT          m_rcBounds;
	BOUNDSACTION  m_baBoundsAction;
	BOOL          m_bHidden;
	BOOL          m_bDying;
	BOOL          m_bOneCycle;
	BOOL          m_bConnected;
	float         m_fRadius;
	POINT         m_pCenter;
	int           m_iId;
	int           m_iAvailCount;
	POINT         m_pHome;
	

	int m_iDirection;

	// Helper Methods
	void          UpdateFrame();
	virtual void  CalcCollisionRect();

public:
	// Constructor(s)/Destructor
	Sprite(Bitmap* pBitmap);
	Sprite(Bitmap* pBitmap, RECT& rcBounds,
		BOUNDSACTION baBoundsAction = BA_STOP);
	Sprite(Bitmap* pBitmap, POINT ptPosition, POINT ptVelocity, int iZOrder,
		RECT& rcBounds, BOUNDSACTION baBoundsAction = BA_STOP);
	Sprite(Bitmap* pBitmap, RECT& rcBounds, int x, int y, int id,
		BOUNDSACTION baBoundsAction = BA_STOP);
	virtual ~Sprite();

	// General Methods
	virtual SPRITEACTION  Update();
	virtual Sprite* AddSprite();
	void                  Draw(HDC hDC);
	BOOL                  IsPointInside(int x, int y);
	BOOL                  TestCollision(Sprite* pTestSprite);
	BOOL                  CircularTestCollision(Sprite* pTestSprite);
	void                  GoHome();
	void                  ChaseBall(POINT ballCenter);



	// Accessor Methods
	Bitmap* GetBitmap() { return m_pBitmap; };
	void    SetNumFrames(int iNumFrames, BOOL bOneCycle = FALSE);
	void    SetFrameDelay(int iFrameDelay) { m_iFrameDelay = iFrameDelay; };
	RECT& GetPosition() { return m_rcPosition; };
	void    SetPosition(int x, int y);
	void    SetPosition(POINT ptPosition);
	void    SetPosition(RECT& rcPosition);
	void    OffsetPosition(int x, int y);
	RECT& GetCollision() { return m_rcCollision; };
	POINT   GetVelocity() { return m_ptVelocity; };
	void    SetVelocity(int x, int y);
	void    SetVelocity(POINT ptVelocity);
	BOOL    GetZOrder() { return m_iZOrder; };
	void    SetZOrder(int iZOrder) { m_iZOrder = iZOrder; };
	void    SetBounds(RECT& rcBounds) { CopyRect(&m_rcBounds, &rcBounds); };
	void    SetBoundsAction(BOUNDSACTION ba) { m_baBoundsAction = ba; };
	BOOL    IsHidden() { return m_bHidden; };
	void    SetHidden(BOOL bHidden) { m_bHidden = bHidden; };
	int     GetWidth()
	{
		return (m_pBitmap->GetWidth() / m_iNumFrames);
	};
	int     GetHeight() { return m_pBitmap->GetHeight(); };

	int     GetDirection() { return m_iDirection; };
	void     SetDirection(int iDirection) { m_iDirection = iDirection; };

	BOOL     GetConnected() { return m_bConnected; };
	void     SetConnected(BOOL bConnected) { m_bConnected = bConnected; };

	POINT     GetCenter() { return m_pCenter; };
	void     SetCenter(POINT pCenter) { m_pCenter = pCenter; };

	float     GetRadius() { return m_fRadius; };
	void     SetRadius(float fRadius) { m_fRadius = fRadius; };

	int      GetId() { return m_iId; };
	void      SetId(int iId) { m_iId = iId; };

	int      GetAvailCount() { return m_iAvailCount; };
	void      SetAvailCount(int iCount) { m_iAvailCount = iCount; };

	POINT      GetHome() { return m_pHome; };

};

//-----------------------------------------------------------------
// Sprite Inline Helper Methods
//-----------------------------------------------------------------
inline void Sprite::UpdateFrame()
{
	if ((m_iFrameDelay >= 0) && (--m_iFrameTrigger <= 0))
	{
		// Reset the frame trigger;
		m_iFrameTrigger = m_iFrameDelay;

		// Increment the frame
		if (++m_iCurFrame >= m_iNumFrames)
		{
			// If it's a one-cycle frame animation, kill the sprite
			if (m_bOneCycle)
				m_bDying = TRUE;
			else
				m_iCurFrame = 0;
		}
	}
}

inline void Sprite::CalcCollisionRect()
{
	int iXShrink = (m_rcPosition.left - m_rcPosition.right) / 12;
	int iYShrink = (m_rcPosition.top - m_rcPosition.bottom) / 12;
	CopyRect(&m_rcCollision, &m_rcPosition);
	InflateRect(&m_rcCollision, iXShrink, iYShrink);
}

//-----------------------------------------------------------------
// Sprite Inline General Methods
//-----------------------------------------------------------------
inline BOOL Sprite::TestCollision(Sprite* pTestSprite)
{
	RECT& rcTest = pTestSprite->GetCollision();
	return m_rcCollision.left <= rcTest.right &&
		rcTest.left <= m_rcCollision.right &&
		m_rcCollision.top <= rcTest.bottom &&
		rcTest.top <= m_rcCollision.bottom;
}

//Static collision detection between two circles
inline BOOL Sprite::CircularTestCollision(Sprite* pCircularSprite)
{
	POINT pCenter = pCircularSprite->GetCenter();
	float fDistance = sqrt((m_pCenter.x - pCenter.x) * (m_pCenter.x - pCenter.x) + (m_pCenter.y - pCenter.y) * (m_pCenter.y - pCenter.y));

	return fDistance <= (m_fRadius + pCircularSprite->GetRadius());
}

inline BOOL Sprite::IsPointInside(int x, int y)
{
	POINT ptPoint;
	ptPoint.x = x;
	ptPoint.y = y;
	return PtInRect(&m_rcPosition, ptPoint);
}
//Opponent oyuncularýnýn rakipten topu kapmasýný saðlar.
inline void Sprite::ChaseBall(POINT ballCenter) {
	int iXdist = ballCenter.x - m_pCenter.x;
	int iYdist = ballCenter.y - m_pCenter.y;

	if (abs(iXdist) > 8 || abs(iYdist) > 8) {
		int x, y;
		if (abs(iXdist) > abs(iYdist) + 8) {
			if (iXdist >= 0) { x = _iStableVelocity; }
			else { x = -_iStableVelocity; }
			y = 0;
		}
		else if (abs(iYdist) > abs(iXdist) + 8) {
			if (iYdist >= 0) { y = _iStableVelocity; }
			else { y = -_iStableVelocity; }
			x = 0;
		}
		else {
			if (iXdist >= 0) { x = _iStableVelocity; }
			else { x = -_iStableVelocity; }
			if (iYdist >= 0) { y = _iStableVelocity; }
			else { y = -_iStableVelocity; }
		}
		SetVelocity(x, y);
	}

}
//Defans sürecinde tüm oyuncularýn geri konumlarýna dönecek hýz ayarlamasý yapýlýr.
inline void Sprite::GoHome() {
	int iXdist = m_pHome.x - m_pCenter.x;
	int iYdist = m_pHome.y - m_pCenter.y;

	if (abs(iXdist) > 8 || abs(iYdist) > 8) {
		int x, y;
		if (abs(iXdist) > abs(iYdist) + 8) {
			if (iXdist >= 0) { x = _iStableVelocity; }
			else { x = -_iStableVelocity; }
			y = 0;
		}
		else if (abs(iYdist) > abs(iXdist) + 8) {
			if (iYdist >= 0) { y = _iStableVelocity; }
			else { y = -_iStableVelocity; }
			x = 0;
		}
		else {
			if (iXdist >= 0) { x = 8; }
			else { x = -_iStableVelocity; }
			if (iYdist >= 0) { y = 8; }
			else { y = -_iStableVelocity; }
		}
		SetVelocity(x, y);
	}
	else {
		SetVelocity(0, 0);
	}

}

//-----------------------------------------------------------------
// Sprite Inline Accessor Methods
//-----------------------------------------------------------------
inline void Sprite::SetNumFrames(int iNumFrames, BOOL bOneCycle)
{
	// Set the number of frames and the one-cycle setting
	m_iNumFrames = iNumFrames;
	m_bOneCycle = bOneCycle;

	// Recalculate the position
	RECT rect = GetPosition();
	rect.right = rect.left + ((rect.right - rect.left) / iNumFrames);
	SetPosition(rect);
}

inline void Sprite::SetPosition(int x, int y)
{
	OffsetRect(&m_rcPosition, x - m_rcPosition.left, y - m_rcPosition.top);
	CalcCollisionRect();
}

inline void Sprite::SetPosition(POINT ptPosition)
{
	OffsetRect(&m_rcPosition, ptPosition.x - m_rcPosition.left,
		ptPosition.y - m_rcPosition.top);
	CalcCollisionRect();
}

inline void Sprite::SetPosition(RECT& rcPosition)
{
	CopyRect(&m_rcPosition, &rcPosition);
	CalcCollisionRect();
}

inline void Sprite::OffsetPosition(int x, int y)
{
	OffsetRect(&m_rcPosition, x, y);
	CalcCollisionRect();
}

inline void Sprite::SetVelocity(int x, int y)
{
	m_ptVelocity.x = x;
	m_ptVelocity.y = y;
}

inline void Sprite::SetVelocity(POINT ptVelocity)
{
	m_ptVelocity.x = ptVelocity.x;
	m_ptVelocity.y = ptVelocity.y;
}
