#include "oxygine-framework.h"
#include <functional>

using namespace oxygine;

class ChessTile;
class ChessPiece;
class PieceMoveButton;
typedef oxygine::intrusive_ptr<ChessTile> spChassTile;
typedef oxygine::intrusive_ptr<ChessPiece> spChessPiece;
typedef oxygine::intrusive_ptr<PieceMoveButton> spPieceMoveButton;

Resources gameResources;

int nTurnNumber = 0;
bool bCurTurnTeam = 0;
std::vector<spPieceMoveButton> PieceMoveButtons;
spChessPiece SelectedPiece = nullptr;

spChassTile Tiles[8][8];
std::string TilesStartTemplate[8][8] = {
  { "Rook", "Knight", "Bishop", "Queen", "King", "Bishop", "Knight", "Rook" },
  { "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn"},
  { "", "", "", "", "", "", "", ""},
  { "", "", "", "", "", "", "", ""},
  { "", "", "", "", "", "", "", ""},
  { "", "", "", "", "", "", "", ""},
  { "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn", "Pawn"},
  { "Rook", "Knight", "Bishop", "King", "Queen", "Bishop", "Knight", "Rook" }
};


bool IsOutOfBounds(int nNumber)
{
    if (nNumber < 0 || nNumber > 7) return true;
    return false;
}

void NextTurn()
{
    nTurnNumber++;
    bCurTurnTeam = !(nTurnNumber % 2);
}

class ChessTile : public Actor
{
public:
    int nI;
    int nJ;
    spSprite TileSprite;
    ChessPiece* Piece = nullptr;
    ChessTile(bool bColorType, Vector2 vPos, int i, int j);
};

class PieceMoveButton : public Actor
{
public:
    spSprite ButtonSprite;
    ChessPiece* Piece = nullptr;
    int nI;
    int nJ;
    PieceMoveButton(Vector2 vPos, int i, int j);

    void PieceMoveButtonClicked(Event* e);
};

class ChessPiece : public Actor
{
public:
    spSprite PieceSprite;
    ChessTile* Tile = nullptr;
    bool bTeam;
    bool bAlive;
    int nNumberOfMoves = 0;
    std::string sName;

    ChessPiece(bool Team, std::string& Name);

    void PieceClicked(Event* e);
    void PieceHovered(Event* e);
    std::vector<Point> GetPossibleMoves();
};

ChessTile::ChessTile(bool bColorType, Vector2 vPos, int i, int j)
{
    nI = i;
    nJ = j;
	spSprite sSprite = new Sprite();
    sSprite->setAnchor(0.5, 0.5);
	sSprite->setPosition(vPos);
	sSprite->setResAnim(gameResources.getResAnim(bColorType ? "ChessTile1" : "ChessTile2"));
	addChild(sSprite);
	TileSprite = sSprite;
}

void ClearSelectedPiece()
{
    for (spPieceMoveButton& Button : PieceMoveButtons) Button->detach();
    PieceMoveButtons.clear();
    SelectedPiece = nullptr;

}

PieceMoveButton::PieceMoveButton(Vector2 vPos, int i, int j)
{
	nI = i;
	nJ = j;
	spSprite sSprite = new Sprite();
	sSprite->setAnchor(0.5, 0.5);
	sSprite->setPosition(vPos);
	sSprite->setResAnim(gameResources.getResAnim("ChessTile3"));
	sSprite->setColor(Color(20, 255, 40, 128));
	addChild(sSprite);
	ButtonSprite = sSprite;
	EventCallback cb = CLOSURE(this, &PieceMoveButton::PieceMoveButtonClicked);
	sSprite->addEventListener(TouchEvent::CLICK, cb);
}

void PieceMoveButton::PieceMoveButtonClicked(Event* e)
{
    SelectedPiece->Tile->Piece = nullptr;
    if (Tiles[nI][nJ]->Piece)
    {
        spTweenQueue tweenQueue = new TweenQueue();
        tweenQueue->add(Sprite::TweenAlpha(0), 500, 1);
        Tiles[nI][nJ]->Piece->PieceSprite->addTween(tweenQueue);
        tweenQueue->detachWhenDone();
    }
    Tiles[nI][nJ]->Piece = SelectedPiece.get();
    SelectedPiece->Tile = Tiles[nI][nJ].get();
    SelectedPiece->PieceSprite->addTween(Sprite::TweenPosition(ButtonSprite->getPosition().x, ButtonSprite->getPosition().y), 1000, 1, false, 0, Tween::ease_linear);
    SelectedPiece->nNumberOfMoves++;
    ClearSelectedPiece();
	NextTurn();
}

ChessPiece::ChessPiece(bool Team, std::string& Name)
{
	bTeam = Team;
	sName = Name;
	bAlive = true;
	spSprite sSprite = new Sprite();
	sSprite->setAnchor(0.5, 0.5);
	sSprite->setResAnim(gameResources.getResAnim(sName + (bTeam ? "1" : "2")));
	sSprite->setScale(0.7f);
	addChild(sSprite);
	EventCallback cb = CLOSURE(this, &ChessPiece::PieceHovered);
	sSprite->addEventListener(TouchEvent::OVER, cb);
	sSprite->addEventListener(TouchEvent::OUTX, cb);
	cb = CLOSURE(this, &ChessPiece::PieceClicked);
	sSprite->addEventListener(TouchEvent::CLICK, cb);
	PieceSprite = sSprite;
}

void ChessPiece::PieceClicked(Event* e)
{
	if (SelectedPiece.get() == this)
	{
        ClearSelectedPiece();
	} else if (bCurTurnTeam == bTeam)
	{
        ClearSelectedPiece();
		for (auto Moves : GetPossibleMoves())
		{
			PieceMoveButtons.push_back(new PieceMoveButton(Tiles[Moves.x][Moves.y]->TileSprite->getPosition(), Moves.x, Moves.y));
		}
		for (spPieceMoveButton& Button : PieceMoveButtons) getStage()->addChild(Button);
		if (!PieceMoveButtons.empty()) SelectedPiece = this;
	}
}

void ChessPiece::PieceHovered(Event* e)
{
	if (e->type == TouchEvent::OVER & bCurTurnTeam == bTeam)
	{
		PieceSprite->addTween(Sprite::TweenAddColor(Color(64, 64, 64, 0)), 300);
	}

	if (e->type == TouchEvent::OUTX)
	{
		PieceSprite->addTween(Sprite::TweenAddColor(Color(0, 0, 0, 0)), 300);
	}
}

std::vector<Point> ChessPiece::GetPossibleMoves()
{
    std::vector<Point> Moves;

    if (!Tile) return Moves;

    Point CurPos(Tile->nI, Tile->nJ);

    int nDirection = bTeam ? 1 : -1;
    if (sName == "Pawn")
    {
        if (!IsOutOfBounds(CurPos.y + nDirection) && !Tiles[CurPos.x][CurPos.y + nDirection]->Piece) {
            Moves.push_back(Point(CurPos.x, CurPos.y + nDirection));
            if (nNumberOfMoves == 0 && !IsOutOfBounds(CurPos.y + nDirection * 2) && !Tiles[CurPos.x][CurPos.y + nDirection * 2]->Piece) {
                Moves.push_back(Point(CurPos.x, CurPos.y + nDirection * 2));
            }
        }
        if (!IsOutOfBounds(CurPos.x + 1) && !IsOutOfBounds(CurPos.y + nDirection) && Tiles[CurPos.x + 1][CurPos.y + nDirection]->Piece && Tiles[CurPos.x + 1][CurPos.y + nDirection]->Piece->bTeam != bTeam) {
        	Moves.push_back(Point(CurPos.x + 1, CurPos.y + nDirection));
        }
        if (!IsOutOfBounds(CurPos.x - 1) && !IsOutOfBounds(CurPos.y + nDirection) && Tiles[CurPos.x - 1][CurPos.y + nDirection]->Piece && Tiles[CurPos.x - 1][CurPos.y + nDirection]->Piece->bTeam != bTeam) {
        	Moves.push_back(Point(CurPos.x - 1, CurPos.y + nDirection));
        }
    }
    if (sName == "Rook" || sName == "Bishop" || sName == "King" || sName == "Queen")
    {
	    for (int i = -1; i <= 1; ++i)
	    {
		    for (int j = -1; j <= 1; ++j)
		    {
                if (sName == "Rook" && i != 0 && j != 0) continue;
                if (sName == "Bishop" && (i == 0 || j == 0)) continue;
                int k = 0;
                while (true)
                {
                    k++;
                    if (!IsOutOfBounds(CurPos.x + i * k) && !IsOutOfBounds(CurPos.y + j * k))
                    {
                        if (!Tiles[CurPos.x + i * k][CurPos.y + j * k]->Piece)
                        {
                            Moves.push_back(Point(CurPos.x + i * k, CurPos.y + j * k));
                        } else {
                            if (Tiles[CurPos.x + i * k][CurPos.y + j * k]->Piece->bTeam != bTeam) Moves.push_back(Point(CurPos.x + i * k, CurPos.y + j * k));
                            break;
                        }
                    }
                    else break;
                    if (sName == "King") break;
                }
		    }
	    }
    }
    if (sName == "Knight")
    {
        for (int i = -2; i <= 2; ++i)
        {
            for (int j = -2; j <= 2; ++j)
            {
                if ((abs(i) == 1 && abs(j) == 1) || (abs(i) == 2 && abs(j) == 2) || (i == 0 || j == 0)) continue;
                if (!IsOutOfBounds(CurPos.x + i) && !IsOutOfBounds(CurPos.y + j) && (!Tiles[CurPos.x + i][CurPos.y + j]->Piece || Tiles[CurPos.x + i][CurPos.y + j]->Piece->bTeam != bTeam))
                {
                    Moves.push_back(Point(CurPos.x + i, CurPos.y + j));
                }
            }
        }
	    
    }
    

    return Moves;
}

void example_preinit() {}

void example_init()
{
    gameResources.loadXML("res.xml");

    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            bool bTileColorType = (i % 2 ? (j % 2) : !(j % 2));
            spChassTile TileActor = new ChessTile(bTileColorType, Vector2(50 + i * 100, 50 + j * 100), i, j);
            getStage()->addChild(TileActor);
            Tiles[i][j] = TileActor;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (!TilesStartTemplate[j][i].empty())
            {
                spChessPiece PieceActor = new ChessPiece(j < 2, TilesStartTemplate[j][i]);
                PieceActor->Tile = Tiles[i][j].get();
                PieceActor->PieceSprite->setPosition(Tiles[i][j]->TileSprite->getPosition().x, Tiles[i][j]->TileSprite->getPosition().y);
                getStage()->addChild(PieceActor);
                Tiles[i][j]->Piece = PieceActor.get();
            }
        }
    }

    NextTurn();
}


//called each frame from main.cpp
void example_update()
{
}

//called each frame from main.cpp
void example_destroy()
{
    //free previously loaded resources
    gameResources.free();
}
