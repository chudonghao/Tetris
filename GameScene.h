//
// Created by chudonghao on 22-7-9.
//

#ifndef INC_CHUDONGHAO_22_7_9_AC0EFB5859BB40DB99D46AEB9D6A4D19_
#define INC_CHUDONGHAO_22_7_9_AC0EFB5859BB40DB99D46AEB9D6A4D19_

#include <QDateTime>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

struct AdvanceContext {
  float gravity{10};
  QDateTime last_game_time{};
  QDateTime game_time{};
};

class Domino : public QGraphicsObject {
public:
  explicit Domino(int size, QGraphicsItem *parent = nullptr);
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public:
  int GetCollidingFlag();
  void SetCollidingFlag(int flag);

private:
  QGraphicsRectItem *_rect;
};

class Polyomino : public QGraphicsObject {
public:
  struct CreateInfo {
    std::vector<std::vector<int>> dominos;
    int core_x;
    int core_y;
    int domino_size;
  };

public:
  explicit Polyomino(CreateInfo ci, QGraphicsItem *parent = nullptr);
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public:
  QPainterPath Shape4CollidingDetection() const;
  static bool IsColliding(QGraphicsScene *scene, QPainterPath shape, QPointF pos);
  bool IsColliding();
  void Advance(const AdvanceContext &ac);
  void ForceLand();
  bool IsLanded();
  void Rotate();

  QVector<Domino *> Break();

private:
  void Descend(qreal ds);

  int _domino_size{1};
  int _core_x{};
  int _core_y{};
  QVector<Domino *> _dominos;
  bool _landed{};
};

class GameScene : public QGraphicsScene {
  Q_OBJECT
public:
  enum PolyominoShape {
    POLYOMINO_SHAPE_I,
    POLYOMINO_SHAPE_J,
    POLYOMINO_SHAPE_L,
    POLYOMINO_SHAPE_S,
    POLYOMINO_SHAPE_Z,
    POLYOMINO_SHAPE_O,
    POLYOMINO_SHAPE_T,
    POLYOMINO_SHAPE_SIZE,
  };

  static qreal GetBoardAlignedPosY(qreal pos, int domino_size);
  static QPoint ToBoardPos(QPointF pos, int domino_size);

  explicit GameScene(QObject *parent = nullptr);

  int GetDominoSize() const { return _domino_size; }
  QSize GetBoardSize() const { return _board_size; }

  void SetDominoSize(int size);
  void SetBoardSize(int column, int row);
  void SetGravity(float gravity);

  void StartGame();
  void StopGame();
  void Clear();

  void TryMovePolyominoLeft();
  void TryMovePolyominoRight();
  void TryMovePolyominoDown();
  void TryRotatePolyomino();
  void TryLandPolyomino();

  void ShowNextPolyomino(PolyominoShape shape);

  int PutPolyomino(PolyominoShape shape);

  Q_SIGNAL void BlockSizeChanged();
  Q_SIGNAL void BoardSizeChanged();

  Q_SIGNAL void PolyominoLanded();
  Q_SIGNAL void LineEliminated(int count);

protected:
  void timerEvent(QTimerEvent *event) override;
  void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
  Polyomino*Create(PolyominoShape shape);

  void RecreateBoard();
  int TryMovePolyomino(float dx, float dy);
  void TryEliminate();
  Domino *GetDomino(QPoint board_pos);
  int SetDomino(QPoint board_pos, Domino *domino);
  void SwapBoardRow(int row1, int row2);

  int _domino_size{20};
  QSize _board_size{10, 20};

  int _advance_timer{};
  AdvanceContext _context;
  AdvanceContext _saved_context;
  Polyomino *_current_polyomino{};
  Polyomino *_next_polyomino{};

  QVector<Domino *> _active_dominos;
  QVector<Domino *> _static_dominos;
};

#endif // INC_CHUDONGHAO_22_7_9_AC0EFB5859BB40DB99D46AEB9D6A4D19_
