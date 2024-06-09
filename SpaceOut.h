//-----------------------------------------------------------------
// Space Out Application
// C++ Header - SpaceOut.h
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include <windows.h>
#include "Resource.h"
#include "GameEngine.h"
#include "Bitmap.h"
#include "Sprite.h"
#include "Background.h"

//-----------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------
HINSTANCE         _hInstance;
GameEngine*		  _pGame;
HDC               _hOffscreenDC;
HBITMAP           _hOffscreenBitmap;
Bitmap*			  _pFieldBitmap;
Bitmap*			  _pPlayerBitmap;
Sprite*			  _pPlayerSprites[5];
BOOL              _bGameOver;
int				  _itemp;
BOOL			  _bcontroltemp;
int               _iSelected, _iOpSelected;
int               _iStartTime, _iTimer, _iSecond, _iMinute;
int               _iTeamScore1, _iTeamScore2;
int				  _iState;

Bitmap*			  _pBallBitmap;
Sprite*			  _pBallSprite;
Bitmap*			  _pOpponentBitmap;
Sprite*			  _pOpponentSprites[5];
Bitmap*			  _pRingBitmap;
Bitmap*			  _pGameOverBitmap;

//-----------------------------------------------------------------
// Function Declarations
//-----------------------------------------------------------------
void NewGame();
BOOL ChooseSelected();
void SlapBall();
void checkGoalStatus();
void Timer();
void PlaceBitmaps(BOOL bFirstPlace);
BOOL OpSelected();
void HittingBall(int direction, BOOL hitType);
void PlayerAttack();
void OpponentMove();
void OpponentSelectMove();
void ShootingOpponent();
void GoalkeeperPatrol();
void SlowDownBall();