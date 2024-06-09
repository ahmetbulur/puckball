//-----------------------------------------------------------------
// PuckBall Application
// C++ Source - PuckBall.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "SpaceOut.h"

//-----------------------------------------------------------------
// Game Engine Functions
//-----------------------------------------------------------------
BOOL GameInitialize(HINSTANCE hInstance)
{
	// Create the game engine
	_pGame = new GameEngine(hInstance, TEXT("Football"),
		TEXT("Football"), IDI_SPACEOUT, IDI_SPACEOUT_SM, 1044, 634);
	if (_pGame == NULL)
		return FALSE;

	// Set the frame rate
	_pGame->SetFrameRate(30);

	// Store the instance handle
	_hInstance = hInstance;

	return TRUE;
}

void GameStart(HWND hWindow)
{
	_iStartTime = GetTickCount();

	// Create the offscreen device context and bitmap
	_hOffscreenDC = CreateCompatibleDC(GetDC(hWindow));
	_hOffscreenBitmap = CreateCompatibleBitmap(GetDC(hWindow), _pGame->GetWidth(), _pGame->GetHeight());
	SelectObject(_hOffscreenDC, _hOffscreenBitmap);

	// Create and load the bitmaps
	HDC hDC = GetDC(hWindow);
	_pFieldBitmap = new Bitmap(hDC, IDB_FIELD, _hInstance);
	_pPlayerBitmap = new Bitmap(hDC, IDB_PLAYER, _hInstance);
	_pOpponentBitmap = new Bitmap(hDC, IDB_OPPONENT, _hInstance);
	_pBallBitmap = new Bitmap(hDC, IDB_BALL, _hInstance);
	_pRingBitmap = new Bitmap(hDC, IDB_RING, _hInstance);
	_pGameOverBitmap = new Bitmap(hDC, IDB_GAMEOVER, _hInstance);

	// Play the background music
	//_pGame->PlayMIDISong(TEXT("Cheer.mid"));

	// Start the game
	NewGame();
}

void GameEnd()
{
	// Close the MIDI player for the background music
	//_pGame->CloseMIDIPlayer();

	// Cleanup the offscreen device context and bitmap
	DeleteObject(_hOffscreenBitmap);
	DeleteDC(_hOffscreenDC);

	// Cleanup the bitmaps
	delete _pFieldBitmap;
	delete _pPlayerBitmap;
	delete _pOpponentBitmap;
	delete _pBallBitmap;
	delete _pRingBitmap;
	delete _pGameOverBitmap;
	// Cleanup the background

	// Cleanup the sprites
	_pGame->CleanupSprites();

	// Cleanup the game engine
	delete _pGame;
}

void GameActivate(HWND hWindow)
{
	// Resume the background music
	//_pGame->PlayMIDISong(TEXT(""), FALSE);
}

void GameDeactivate(HWND hWindow)
{
	// Pause the background music
	//_pGame->PauseMIDISong();
}

void GamePaint(HDC hDC)
{
	// Draw the Football field bitmap
	_pFieldBitmap->Draw(hDC, 0, 0);

	// Draw the sprites
	_pGame->DrawSprites(hDC);

	// Draw the indicator for selected player
	RECT selectedPos = _pPlayerSprites[_iSelected]->GetPosition();
	_pRingBitmap->Draw(hDC, selectedPos.left, selectedPos.top, TRUE);

	//Draw the score
	TCHAR szScoreText[64];
	RECT  rect = { 450, 10, 540, 30 };
	wsprintf(szScoreText, "%d        %d", _iTeamScore1, _iTeamScore2);
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(225, 220, 47));
	DrawText(hDC, szScoreText, -1, &rect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

	//Draw the time
	TCHAR szTimeText[64];
	RECT  rect2 = { 50, 10, 200, 30 };
	wsprintf(szTimeText, "Time: %d:%d", _iMinute, _iSecond);
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(225, 220, 47));
	DrawText(hDC, szTimeText, -1, &rect2, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

	if (_bGameOver) {

		_pGameOverBitmap->Draw(hDC, 387, 254, true);
		TCHAR szTimeText[128];
		RECT  rect = { 100, 280, 640, 640 };

		if (_iTeamScore1 > _iTeamScore2)
			wsprintf(szTimeText, " YOU WIN! PRESS ENTER TO RESTART");
		if (_iTeamScore1 < _iTeamScore2)
			wsprintf(szTimeText, "YOU LOST! PRESS ENTER TO RESTART");
		if (_iTeamScore1 == _iTeamScore2)
			wsprintf(szTimeText, "    DRAW! PRESS ENTER TO RESTART");

		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, RGB(255, 0, 0));
		DrawText(hDC, szTimeText, -1, &rect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

	}
}

void GameCycle()
{
	if (!_bGameOver)
	{
		Timer();
		checkGoalStatus();

		//ChooseSelected();
		OpSelected();

		// player defence - ai attack
		if (_iState == 1) {
			// player defence
			for (int i = 0; i < 5; i++) {
				if (i != _iSelected) {
					_pPlayerSprites[i]->GoHome();
				}
			}
			GoalkeeperPatrol();

			//ai attack
			for (int i = 0; i < 5; i++) {
				if (i != _iOpSelected) {
					// hücum hareketi
					OpponentMove();
				}
				else {
					OpponentSelectMove();
					if (!_pBallSprite->GetConnected()) {
						_pOpponentSprites[i]->ChaseBall(_pBallSprite->GetCenter());
					}
					ShootingOpponent();
				}

			}
		}

		// ai defence - player attack
		if (_iState == 2) {
			//ai defence
			for (int i = 0; i < 5; i++) {
				if (i != _iOpSelected) {
					_pOpponentSprites[i]->GoHome();
				}
				else
					_pOpponentSprites[i]->ChaseBall(_pBallSprite->GetCenter());
			}
			ChooseSelected();
			GoalkeeperPatrol();

			for (int i = 0; i < 5; i++) {
				if (i != _iSelected) {
					//_pPlayerSprites[i]->GoHome();
				}
			}

			//support attacker (player attack)
			PlayerAttack();
		}

		if (_iSelected == 4)
			_iSelected = 0;

		// slow ball down(friction effect)
		SlowDownBall();

		// Update the sprites
		_pGame->UpdateSprites();

		// Obtain a device context for repainting the game
		HWND  hWindow = _pGame->GetWindow();
		HDC   hDC = GetDC(hWindow);

		// Paint the game to the offscreen device context
		GamePaint(_hOffscreenDC);

		// Blit the offscreen bitmap to the game screen
		BitBlt(hDC, 0, 0, _pGame->GetWidth(), _pGame->GetHeight(),
			_hOffscreenDC, 0, 0, SRCCOPY);

		// Cleanup
		ReleaseDC(hWindow, hDC);
	}
}

void HandleKeys()
{
	if (!_bGameOver)
	{
		// Move the player based upon WASD key presses
		POINT ptVelocity = _pPlayerSprites[_iSelected]->GetVelocity();

		if (_iSelected != 4) {
			if (GetAsyncKeyState(0x41) < 0)//GetAsyncKeyState(VK_LEFT) < 0) //A
			{
				// Move left
				ptVelocity.x = max(ptVelocity.x - 2, -_iStableVelocity);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}
			else if (GetAsyncKeyState(0x44) < 0)//GetAsyncKeyState(VK_RIGHT) < 0) //D
			{
				// Move right
				ptVelocity.x = min(ptVelocity.x + 2, _iStableVelocity);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}
			else { // slow player down (friction effect)
				if (ptVelocity.x > 0)
					ptVelocity.x = max(ptVelocity.x - 1, 0);
				else if (ptVelocity.x < 0)
					ptVelocity.x = min(ptVelocity.x + 1, 0);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}
			if (GetAsyncKeyState(0x57) < 0)//GetAsyncKeyState(VK_UP) < 0) //W
			{
				// Move up
				ptVelocity.y = max(ptVelocity.y - 2, -_iStableVelocity);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}
			else if (GetAsyncKeyState(0x53) < 0)//GetAsyncKeyState(VK_DOWN) < 0) //S
			{
				// Move down
				ptVelocity.y = min(ptVelocity.y + 2, _iStableVelocity);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}
			else { // slow player down (friction effect)
				if (ptVelocity.y > 0)
					ptVelocity.y = max(ptVelocity.y - 1, 0);
				else if (ptVelocity.y < 0)
					ptVelocity.y = min(ptVelocity.y + 1, 0);
				_pPlayerSprites[_iSelected]->SetVelocity(ptVelocity);
			}

			// Pressing K key to shoot
			if (GetAsyncKeyState(0x4B) < 0) {

				if (_pBallSprite->GetConnected()) {

					POINT ballVelocity = _pBallSprite->GetVelocity();
					int iPlayerDirection = _pPlayerSprites[_iSelected]->GetDirection();
					if (_pPlayerSprites[_iSelected]->GetConnected())
						HittingBall(iPlayerDirection, false);
				}
			}

			//Pressing L key to pass another player
			if (GetAsyncKeyState(0x4C) < 0) {

				if (_pBallSprite->GetConnected()) {

					POINT ballVelocity = _pBallSprite->GetVelocity();
					int iPlayerDirection = _pPlayerSprites[_iSelected]->GetDirection();
					if (_pPlayerSprites[_iSelected]->GetConnected())
						HittingBall(iPlayerDirection, true);

				}
			}

			//Pressing Q key to switch player
			if (GetAsyncKeyState(0x51) < 0) {
				ChooseSelected();
			}
		}

	}

	// Start a new game based upon an Enter (Return) key press
	if (_bGameOver && (GetAsyncKeyState(VK_RETURN) < 0)) {

		NewGame();

	}
}

void MouseButtonDown(int x, int y, BOOL bLeft)
{

}

void MouseButtonUp(int x, int y, BOOL bLeft)
{
}

void MouseMove(int x, int y)
{
}

void HandleJoystick(JOYSTATE jsJoystickState)
{
}

//-----------------------------------------------------------------
// Functions
//-----------------------------------------------------------------

void Timer() {
	_iTimer = (GetTickCount() - _iStartTime) / 1000;

	_iSecond = _iTimer % 60;
	_iMinute = _iTimer / 60;

	if (_iMinute == 5) { // match ends after 5 minutes
		_bGameOver = true;
	}
}

void HittingBall(int direction, BOOL hitType) { // hitType == true means pass, false means shoot.

	if (_pBallSprite->GetConnected()) {
		_pGame->ResetConnected();
		int velocity, velocityD;
		POINT ballVelocity = _pBallSprite->GetVelocity();

		if (hitType) {
			velocity = 25;
			velocityD = 21;
		}
		else {
			velocity = 35;
			velocityD = 29;
		}

		if (direction == 1) { // left
			_pBallSprite->SetVelocity(ballVelocity.x - velocity, 0);
		}
		else if (direction == 2) { // right
			_pBallSprite->SetVelocity(ballVelocity.x + velocity, 0);
		}
		else if (direction == 3) { // up
			_pBallSprite->SetVelocity(0, ballVelocity.y - velocity);
		}
		else if (direction == 4) { // down
			_pBallSprite->SetVelocity(0, ballVelocity.y + velocity);
		}
		if (direction == 5) { // left-up
			_pBallSprite->SetVelocity(ballVelocity.x - velocityD, ballVelocity.y - velocityD);
		}
		else if (direction == 6) { // right-up
			_pBallSprite->SetVelocity(ballVelocity.x + velocityD, ballVelocity.y - velocityD);
		}
		else if (direction == 7) { // left-down
			_pBallSprite->SetVelocity(ballVelocity.x - velocityD, ballVelocity.y + velocityD);
		}
		else if (direction == 8) { //right-down
			_pBallSprite->SetVelocity(ballVelocity.x + velocityD, ballVelocity.y + velocityD);
		}
	}

}

BOOL SpriteCollision(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
	Bitmap* pHitter = pSpriteHitter->GetBitmap();
	Bitmap* pHittee = pSpriteHittee->GetBitmap();
	POINT pHitterCenter = pSpriteHitter->GetCenter();
	POINT pHitteeCenter = pSpriteHittee->GetCenter();
	float pHitterRadius = pSpriteHitter->GetRadius();
	float pHitteeRadius = pSpriteHittee->GetRadius();
	int HitterAvailCount = pSpriteHitter->GetAvailCount();
	int HitteeAvailCount = pSpriteHittee->GetAvailCount();

	// Topa deðen oyuncu mu deðil mi bakýlýr.
	if (pHitter == _pPlayerBitmap && pHittee == _pBallBitmap && HitterAvailCount <= 0)
	{
		_pGame->ResetConnected();
		pSpriteHitter->SetConnected(TRUE);
		pSpriteHittee->SetConnected(TRUE);
		_iSelected = pSpriteHitter->GetId();
		_iState = 2;

	}
	else if (pHittee == _pPlayerBitmap && pHitter == _pBallBitmap && HitteeAvailCount <= 0)
	{
		_pGame->ResetConnected();
		pSpriteHittee->SetConnected(TRUE);
		pSpriteHitter->SetConnected(TRUE);
		_iSelected = pSpriteHittee->GetId();
		_iState = 2;
	}

	// Topa deðen ai oyuncusu mu deðil mi bakýlýr
	if (pHitter == _pOpponentBitmap && pHittee == _pBallBitmap && HitterAvailCount <= 0)
	{
		_pGame->ResetConnected();
		pSpriteHitter->SetConnected(TRUE);
		pSpriteHittee->SetConnected(TRUE);
		_iOpSelected = pSpriteHitter->GetId();
		_iState = 1;
	}
	else if (pHittee == _pOpponentBitmap && pHitter == _pBallBitmap && HitteeAvailCount <= 0)
	{
		_pGame->ResetConnected();
		pSpriteHittee->SetConnected(TRUE);
		pSpriteHitter->SetConnected(TRUE);
		_iOpSelected = pSpriteHittee->GetId();
		_iState = 1;
	}

	float fDistance = sqrt((pHitterCenter.x - pHitteeCenter.x) * (pHitterCenter.x - pHitteeCenter.x) + (pHitterCenter.y - pHitteeCenter.y) * (pHitterCenter.y - pHitteeCenter.y));
	float fOverlap = 0.5 * (fDistance - pHitterRadius - pHitteeRadius);
	if (pHitter != _pBallBitmap && pHittee != _pBallBitmap) {

		pHitterCenter.x -= fOverlap * (pHitterCenter.x - pHitteeCenter.x) / fDistance;
		pHitterCenter.y -= fOverlap * (pHitterCenter.y - pHitteeCenter.y) / fDistance;

		pHitteeCenter.x -= fOverlap * (pHitteeCenter.x - pHitterCenter.x) / fDistance;
		pHitteeCenter.y -= fOverlap * (pHitteeCenter.y - pHitterCenter.y) / fDistance;
		pSpriteHitter->SetPosition(pHitterCenter.x - pHitterRadius, pHitterCenter.y - pHitterRadius);
		pSpriteHittee->SetPosition(pHitteeCenter.x - pHitteeRadius, pHitteeCenter.y - pHitteeRadius);


	}

	return FALSE;
}

void NewGame()
{
	_iState = 1;

	PlaySound((LPCSTR)IDW_WHISTLE, _hInstance, SND_ASYNC | SND_RESOURCE | SND_NOSTOP);

	// Clear the sprites
	_pGame->CleanupSprites();

	// Initialize the game variables
	_iSelected = 0;
	_iOpSelected = 0;
	PlaceBitmaps(true);
	_iTeamScore1 = 0, _iTeamScore2 = 0;
	_bGameOver = FALSE;
	_iStartTime = GetTickCount();

	// Play the background music
	//_pGame->PlayMIDISong();
}
//Q tuþu ile en yakýn oyuncuyu seçme
BOOL ChooseSelected() {
	float minDist = 9000;
	int minIndex = 0;

	POINT pHitteeCenter = _pBallSprite->GetCenter();
	for (int i = 0; i < 5; i++) {
		if (_pPlayerSprites[i]->GetConnected()) {
			return false;
		}
		POINT pHitterCenter = _pPlayerSprites[i]->GetCenter();
		float fDistance = sqrt((pHitterCenter.x - pHitteeCenter.x) * (pHitterCenter.x - pHitteeCenter.x) + (pHitterCenter.y - pHitteeCenter.y) * (pHitterCenter.y - pHitteeCenter.y));
		if (fDistance < minDist) {
			minDist = fDistance;
			minIndex = i;
		}
	}
	_iSelected = minIndex;

	return true;
}
//yapay zeka oyuncularýnýn içerisinde topa en yakýn olan oyuncuyu seçme
BOOL OpSelected() {
	float minDist = 9000;
	int minIndex = 0;

	POINT pHitteeCenter = _pBallSprite->GetCenter();
	for (int i = 0; i < 5; i++) {
		if (_pOpponentSprites[i]->GetConnected()) {
			return false;
		}
		POINT pHitterCenter = _pOpponentSprites[i]->GetCenter();
		float fDistance = sqrt((pHitterCenter.x - pHitteeCenter.x) * (pHitterCenter.x - pHitteeCenter.x) + (pHitterCenter.y - pHitteeCenter.y) * (pHitterCenter.y - pHitteeCenter.y));
		if (pHitteeCenter.x > 712) {
			if (fDistance < minDist && (i == 2 || i == 3)) {
				minDist = fDistance;
				minIndex = i;
			}
		}
		else {
			if (fDistance < minDist && (i == 0 || i == 1)) {
				minDist = fDistance;
				minIndex = i;
			}
		}
	}
	_iOpSelected = minIndex;

	return true;
}

// topu sürerken öne atma
void SlapBall() {
	if (_bcontroltemp == false) {
		if (_itemp < 10) {
			_itemp += 2;
		}
		if (_itemp == 10) {
			_bcontroltemp = true;
		}
	}
	else {
		if (_itemp > 0) {
			_itemp -= 2;
		}
		if (_itemp == 0) {
			_bcontroltemp = false;
		}
	}
}

// slow ball down (friction effect)
void SlowDownBall() {
	POINT ballVelocity = _pBallSprite->GetVelocity();
	if (!_pBallSprite->GetConnected()) {
		if (ballVelocity.x > 0)
			ballVelocity.x = max(ballVelocity.x - 1, 0);
		else if (ballVelocity.x < 0)
			ballVelocity.x = min(ballVelocity.x + 1, 0);
		if (ballVelocity.y > 0)
			ballVelocity.y = max(ballVelocity.y - 1, 0);
		else if (ballVelocity.y < 0)
			ballVelocity.y = min(ballVelocity.y + 1, 0);
	}
	else
		ballVelocity = { 0, 0 };
	_pBallSprite->SetVelocity(ballVelocity);
}

// determines where ball should take position on which side of player 
void setBallPosition(Sprite* pSpriteHitter) {

	float fHitterRadius = pSpriteHitter->GetRadius();
	float fBallRadius = _pBallSprite->GetRadius();

	POINT pHitterCenter = pSpriteHitter->GetCenter();
	POINT pBallCenter = _pBallSprite->GetCenter();

	POINT playerVelocity = pSpriteHitter->GetVelocity();
	int iDirection = pSpriteHitter->GetDirection();

	if (playerVelocity.x < 0 && playerVelocity.y == 0) { // left
		if (iDirection != 1) {
			_itemp = 0;
		}
		pBallCenter.x = pHitterCenter.x - _itemp - (fHitterRadius + fBallRadius);
		pBallCenter.y = pHitterCenter.y;
		pSpriteHitter->SetDirection(1);
		SlapBall();

	}
	else if (playerVelocity.x > 0 && playerVelocity.y == 0) { // right
		if (iDirection != 2) {
			_itemp = 0;
		}
		pBallCenter.x = pHitterCenter.x + _itemp + (fHitterRadius + fBallRadius);
		pBallCenter.y = pHitterCenter.y;
		pSpriteHitter->SetDirection(2);
		SlapBall();
	}
	else if (playerVelocity.x == 0 && playerVelocity.y < 0) { // up
		if (iDirection != 3) {
			_itemp = 0;
		}
		pBallCenter.x = pHitterCenter.x;
		pBallCenter.y = pHitterCenter.y - _itemp - (fHitterRadius + fBallRadius);
		pSpriteHitter->SetDirection(3);
		SlapBall();
	}
	else if (playerVelocity.x == 0 && playerVelocity.y > 0) { // down
		if (iDirection != 4) {
			_itemp = 0;
		}
		pBallCenter.x = pHitterCenter.x;
		pBallCenter.y = pHitterCenter.y + _itemp + (fHitterRadius + fBallRadius);
		pSpriteHitter->SetDirection(4);
		SlapBall();
	}
	else if (playerVelocity.x < 0 && playerVelocity.y < 0) { // left-up

		pBallCenter.x = pHitterCenter.x - (_itemp / sqrt(2)) - (fHitterRadius + fBallRadius) / sqrt(2);
		pBallCenter.y = pHitterCenter.y - (_itemp / sqrt(2)) - (fHitterRadius + fBallRadius) / sqrt(2);
		pSpriteHitter->SetDirection(5);
		SlapBall();
	}
	else if (playerVelocity.x > 0 && playerVelocity.y < 0) { // right-up

		pBallCenter.x = pHitterCenter.x + (_itemp / sqrt(2)) + (fHitterRadius + fBallRadius) / sqrt(2);
		pBallCenter.y = pHitterCenter.y - (_itemp / sqrt(2)) - (fHitterRadius + fBallRadius) / sqrt(2);
		pSpriteHitter->SetDirection(6);
		SlapBall();

	}
	else if (playerVelocity.x < 0 && playerVelocity.y > 0) { // left-down

		pBallCenter.x = pHitterCenter.x - (_itemp / sqrt(2)) - (fHitterRadius + fBallRadius) / sqrt(2);
		pBallCenter.y = pHitterCenter.y + (_itemp / sqrt(2)) + (fHitterRadius + fBallRadius) / sqrt(2);
		pSpriteHitter->SetDirection(7);
		SlapBall();

	}
	else if (playerVelocity.x > 0 && playerVelocity.y > 0) { // right-down

		pBallCenter.x = pHitterCenter.x + (_itemp / sqrt(2)) + (fHitterRadius + fBallRadius) / sqrt(2);
		pBallCenter.y = pHitterCenter.y + (_itemp / sqrt(2)) + (fHitterRadius + fBallRadius) / sqrt(2);
		pSpriteHitter->SetDirection(8);
		SlapBall();
	}
	_pBallSprite->SetPosition(pBallCenter.x - fBallRadius, pBallCenter.y - fBallRadius);

}

//Player attack control
void PlayerAttack() {
	POINT ballCenter = _pBallSprite->GetCenter();
	for (int i = 0; i < 5; i++) {
		int id = _pPlayerSprites[i]->GetId();
		POINT playerCenter = _pPlayerSprites[i]->GetCenter();
		if (i != _iSelected) {
			//ID 2 ve 3 defans oyuncularý
			if (id == 2 || id == 3) {
				if (ballCenter.x - playerCenter.x > 200 && playerCenter.x < 540)
					_pPlayerSprites[i]->SetVelocity(_iStableVelocity, 0);

				else if (ballCenter.x - playerCenter.x < 180 && playerCenter.x > _pPlayerSprites[i]->GetHome().x)
				{
					_pPlayerSprites[i]->SetVelocity(-_iStableVelocity, 0);
				}
				else {
					_pPlayerSprites[i]->SetVelocity(0, 0);
				}

			}
			if (id == 0 || id == 1) {
				if (ballCenter.x - playerCenter.x > 50 && playerCenter.x < 865)
					_pPlayerSprites[i]->SetVelocity(_iStableVelocity, 0);

				else if (ballCenter.x - playerCenter.x < 40 && playerCenter.x > _pPlayerSprites[i]->GetHome().x)
				{
					_pPlayerSprites[i]->SetVelocity(-_iStableVelocity, 0);
				}
				else {
					_pPlayerSprites[i]->SetVelocity(0, 0);
				}
			}

		}
	}
}
// ai hits ball according to its location on the field
void ShootingOpponent() {
	POINT pOpponentCenter = _pOpponentSprites[_iOpSelected]->GetCenter();
	if (pOpponentCenter.x < 217) {
		if (pOpponentCenter.y < 255) {
			HittingBall(7, false); // left-down
		}
		else if (pOpponentCenter.y < 405) {
			HittingBall(1, false); // left
		}
		else {
			HittingBall(5, false); // left-up
		}
	}
}

// ai players play in different styles due to their positions(id = 0 or 1 -> attacker, id = 2 or 3 -> defender)  
void OpponentSelectMove() {
	int random = rand() % 30;
	int id = _pOpponentSprites[_iOpSelected]->GetId();
	if (id == 0) {
		if (random < 10) {
			if (random < 7) {
				_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, 0);
			}
			else {
				_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, -_iStableVelocity);
			}
		}

	}
	if (id == 1) {
		if (random < 10) {
			if (random < 7) {
				_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, 0);
			}
			else {
				_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, _iStableVelocity);
			}
		}

	}
	if (id == 2) {
		if (random >= 10) {
			_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, 0);
		}
		else {
			if (random <= 4)
				HittingBall(1, true);
			else if (random <= 8)
				HittingBall(5, true);
			else HittingBall(7, true);
		}
	}
	if (id == 3) {
		if (random >= 10) {
			_pOpponentSprites[_iOpSelected]->SetVelocity(-_iStableVelocity, 0);
		}
		else {
			if (random <= 4)
				HittingBall(1, true);
			else if (random <= 8)
				HittingBall(7, true);
			else HittingBall(3, true);
		}
	}
}

void OpponentMove() {
	POINT ballCenter = _pBallSprite->GetCenter();

	for (int i = 0; i < 5; i++) {
		int id = _pOpponentSprites[i]->GetId();
		POINT opponentCenter = _pOpponentSprites[i]->GetCenter();
		if (i != _iOpSelected) {
			//ID: 2 or 3 -> defenders
			if (id == 2 || id == 3) {
				if (opponentCenter.x - ballCenter.x > 200 && opponentCenter.x > 490)
					_pOpponentSprites[i]->SetVelocity(-_iStableVelocity, 0);

				else if (opponentCenter.x - ballCenter.x  < 180 && opponentCenter.x > _pOpponentSprites[i]->GetHome().x)
				{
					_pOpponentSprites[i]->SetVelocity(_iStableVelocity, 0);
				}
				else {

					_pOpponentSprites[i]->SetVelocity(0, 0);
				}

			}
			//ID: 0 or 1 -> attackers
			if (id == 0 || id == 1) {
				if (opponentCenter.x - ballCenter.x > 50 && opponentCenter.x > 175)
					_pOpponentSprites[i]->SetVelocity(-_iStableVelocity, 0);

				else if (opponentCenter.x - ballCenter.x < 40 && opponentCenter.x > _pOpponentSprites[i]->GetHome().x)
				{
					_pOpponentSprites[i]->SetVelocity(_iStableVelocity, 0);
				}
				else {
					_pOpponentSprites[i]->SetVelocity(0, 0);
				}
			}

		}

	}
}

//ai attack control
void OpponentAttack(Sprite* pSpriteHitter) {
	Bitmap* pHitter = pSpriteHitter->GetBitmap();
	POINT pHitterCenter = pSpriteHitter->GetCenter();
	int id = pSpriteHitter->GetId();
	int random = rand() % 10;
	if (pHitter == _pOpponentBitmap) {
		for (int i = 0; i < 4; i++) {
			POINT pDefenderCenter = _pPlayerSprites[i]->GetCenter();
			float fDistance = sqrt((pHitterCenter.x - pDefenderCenter.x) * (pHitterCenter.x - pDefenderCenter.x) + (pHitterCenter.y - pDefenderCenter.y) * (pHitterCenter.y - pDefenderCenter.y));
			if (fDistance < 80 /* && pHitterCenter.x > pDefenderCenter.x*/) {
				if (id == 0) {
					if (random <= 8)
						HittingBall(3, true);
					else HittingBall(7, true);
					//yukarý dön ve pas at
				}
				else if (id == 1) {
					//aþaðý dön ve pas at
					HittingBall(4, true);
				}
				//right down defence player
				else if (id == 2) {
					// random 4 ten küçükse sola 4-8 arasý sol yukarý  8-10 arasý sol aþaðý vurucak.
					if (random <= 4)
						HittingBall(1, true);
					else if (random <= 8)
						HittingBall(5, true);
					else HittingBall(7, true);
				}
				//right up defence player
				else if (id == 3) {
					// random 4 ten küçükse sola 4-8 arasý sol aþaðý  8-10 arasý yukarý vurucak.
					if (random <= 4)
						HittingBall(1, true);
					else if (random <= 8)
						HittingBall(7, true);
					else HittingBall(3, true);
				}

			}
		}
	}
}
//Kalecinin y ekseni olarak 260 ile 400 arasýnda gezinmeli  
void GoalkeeperPatrol() {

	int random = rand() % 10;
	if (_pPlayerSprites[4]->GetConnected()) {
		//_pBallSprite->SetVelocity(0,0); //FÜZEEEEEEEEEEEEEE
		if (random < 4)
			HittingBall(6, false);
		else if (random < 8)
			HittingBall(8, false);
		else HittingBall(2, false);
	}
	if (_pOpponentSprites[4]->GetConnected()) {
		if (random < 4)
			HittingBall(5, false);
		else if (random < 8)
			HittingBall(7, false);
		else HittingBall(1, false);
	}

	POINT ballCenter = _pBallSprite->GetCenter();
	POINT goalkeeperCenter = _pPlayerSprites[4]->GetCenter();
	POINT OpKeeperCenter = _pOpponentSprites[4]->GetCenter();

	// if ball enters penalty area, goalkeeper dives
	if (ballCenter.x < 200 || ballCenter.x > 834) {

		if (goalkeeperCenter.y >= 260 && goalkeeperCenter.y <= 400) {

			if (goalkeeperCenter.y < ballCenter.y)
				_pPlayerSprites[4]->SetVelocity(0, 3);
			else if (goalkeeperCenter.y > ballCenter.y)
				_pPlayerSprites[4]->SetVelocity(0, -3);
			else
				_pPlayerSprites[4]->SetVelocity(0, 0);
		}
		else
			_pPlayerSprites[4]->SetVelocity(0, 0);

		if (OpKeeperCenter.y >= 260 && OpKeeperCenter.y <= 400) {

			if (OpKeeperCenter.y < ballCenter.y)
				_pOpponentSprites[4]->SetVelocity(0, 2);
			else if (OpKeeperCenter.y > ballCenter.y)
				_pOpponentSprites[4]->SetVelocity(0, -2);
			else
				_pOpponentSprites[4]->SetVelocity(0, 0);
		}
		else
			_pOpponentSprites[4]->SetVelocity(0, 0);

	}
	// else goalkeeper takes position according to ball's position
	else {
		if (goalkeeperCenter.y < ballCenter.y / 5 + 265)
			_pPlayerSprites[4]->SetVelocity(0, 2);
		else if (goalkeeperCenter.y > ballCenter.y / 5 + 275)
			_pPlayerSprites[4]->SetVelocity(0, -2);
		else
			_pPlayerSprites[4]->SetVelocity(0, 0);

		if (OpKeeperCenter.y < ballCenter.y / 5 + 265)
			_pOpponentSprites[4]->SetVelocity(0, 2);
		else if (OpKeeperCenter.y > ballCenter.y / 5 + 275)
			_pOpponentSprites[4]->SetVelocity(0, -2);
		else
			_pOpponentSprites[4]->SetVelocity(0, 0);
	}

}

// place players and ball on football field
void PlaceBitmaps(BOOL bFirstPlace) {

	if (bFirstPlace) {
		RECT rcBounds = { 29, 50, 1004, 611 };
		_pPlayerSprites[0] = new Sprite(_pPlayerBitmap, rcBounds, 390, 465, 0, BA_STOP);
		_pPlayerSprites[0]->SetPosition(390, 465);

		_pPlayerSprites[1] = new Sprite(_pPlayerBitmap, rcBounds, 390, 180, 1, BA_STOP);
		_pPlayerSprites[1]->SetPosition(390, 180);

		_pPlayerSprites[2] = new Sprite(_pPlayerBitmap, rcBounds, 200, 390, 2, BA_STOP);
		_pPlayerSprites[2]->SetPosition(200, 390);

		_pPlayerSprites[3] = new Sprite(_pPlayerBitmap, rcBounds, 200, 225, 3, BA_STOP);
		_pPlayerSprites[3]->SetPosition(200, 225);

		_pPlayerSprites[4] = new Sprite(_pPlayerBitmap, rcBounds, 58, 311, 4, BA_STOP);
		_pPlayerSprites[4]->SetPosition(58, 311);

		_pOpponentSprites[0] = new Sprite(_pOpponentBitmap, rcBounds, 610, 470, 0, BA_STOP);
		_pOpponentSprites[0]->SetPosition(610, 470);

		_pOpponentSprites[1] = new Sprite(_pOpponentBitmap, rcBounds, 610, 180, 1, BA_STOP);
		_pOpponentSprites[1]->SetPosition(610, 180);

		_pOpponentSprites[2] = new Sprite(_pOpponentBitmap, rcBounds, 800, 400, 2, BA_STOP);
		_pOpponentSprites[2]->SetPosition(800, 400);

		_pOpponentSprites[3] = new Sprite(_pOpponentBitmap, rcBounds, 800, 235, 3, BA_STOP);
		_pOpponentSprites[3]->SetPosition(800, 235);

		_pOpponentSprites[4] = new Sprite(_pOpponentBitmap, rcBounds, 940, 311, 4, BA_STOP);
		_pOpponentSprites[4]->SetPosition(940, 311);

		_pBallSprite = new Sprite(_pBallBitmap, rcBounds, BA_BOUNCE);
		_pBallSprite->SetPosition(508, 324);

		_pGame->AddSprite(_pPlayerSprites[0]);
		_pGame->AddSprite(_pPlayerSprites[1]);
		_pGame->AddSprite(_pPlayerSprites[2]);
		_pGame->AddSprite(_pPlayerSprites[3]);
		_pGame->AddSprite(_pPlayerSprites[4]);

		_pGame->AddSprite(_pOpponentSprites[0]);
		_pGame->AddSprite(_pOpponentSprites[1]);
		_pGame->AddSprite(_pOpponentSprites[2]);
		_pGame->AddSprite(_pOpponentSprites[3]);
		_pGame->AddSprite(_pOpponentSprites[4]);

		_pGame->AddSprite(_pBallSprite);
	}
	else {
		_pPlayerSprites[0]->SetPosition(390, 465);
		_pPlayerSprites[1]->SetPosition(390, 180);
		_pPlayerSprites[2]->SetPosition(200, 390);
		_pPlayerSprites[3]->SetPosition(200, 225);
		_pPlayerSprites[4]->SetPosition(58, 311);
		_pOpponentSprites[0]->SetPosition(610, 470);
		_pOpponentSprites[1]->SetPosition(610, 180);
		_pOpponentSprites[2]->SetPosition(800, 400);
		_pOpponentSprites[3]->SetPosition(800, 235);
		_pOpponentSprites[4]->SetPosition(940, 311);
		_pBallSprite->SetPosition(508, 324);
	}


}

// goal control and score update
void checkGoalStatus() {

	POINT pBallCenter = _pBallSprite->GetCenter();
	float fBallRadius = _pBallSprite->GetRadius();

	RECT ballPos = _pBallSprite->GetPosition();

	RECT leftGoalBounds = { 9, 273, 30, 392 };
	RECT rightGoalBounds = { 1002, 273, 1023, 392 };

	if (ballPos.right <= leftGoalBounds.right && (ballPos.top >= leftGoalBounds.top && ballPos.bottom <= leftGoalBounds.bottom)) {

		PlaySound((LPCSTR)IDW_GOAL, _hInstance, SND_ASYNC | SND_RESOURCE | SND_NOSTOP);

		_iTeamScore2++;
		_pBallSprite->SetVelocity({ 0,0 });
		_pGame->ResetConnected();
		PlaceBitmaps(false);
	}
	if (ballPos.left >= rightGoalBounds.left && (ballPos.top >= rightGoalBounds.top && ballPos.bottom <= rightGoalBounds.bottom)) {

		PlaySound((LPCSTR)IDW_GOAL, _hInstance, SND_ASYNC | SND_RESOURCE | SND_NOSTOP);

		_iTeamScore1++;
		_pBallSprite->SetVelocity({ 0,0 });
		_pGame->ResetConnected();
		PlaceBitmaps(false);
	}
}