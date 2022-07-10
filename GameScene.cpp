//
// Created by chudonghao on 22-7-9.
//

#include "GameScene.h"

#include <cmath>

#include <QDebug>
#include <QPainter>
#include <QTimerEvent>

//======================
namespace {
enum {
  COLLIDING_FLAG_ROLE = Qt::UserRole + 1,
};
}

//======================
Domino::Domino(int size, QGraphicsItem *parent) : QGraphicsObject(parent) {
  setFlag(ItemHasNoContents);
  _rect = new QGraphicsRectItem(0, 0, size, size, this);
  _rect->setBrush(Qt::blue);
  _rect->setData(COLLIDING_FLAG_ROLE, 0);
}

QRectF Domino::boundingRect() const { return QRectF(); }

void Domino::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {}

int Domino::GetCollidingFlag() { return _rect->data(COLLIDING_FLAG_ROLE).toInt(); }

void Domino::SetCollidingFlag(int flag) { _rect->setData(COLLIDING_FLAG_ROLE, flag); }

//======================
Polyomino::Polyomino(Polyomino::CreateInfo ci, QGraphicsItem *parent) : QGraphicsObject(parent) {
  for (int row = 0; row < ci.dominos.size(); ++row) {
    auto &flag_row = ci.dominos[row];
    for (int column = 0; column < flag_row.size(); ++column) {
      if (flag_row[column]) {
        auto d = new Domino(ci.domino_size, this);
        d->setPos(QPointF((column - ci.core_x) * ci.domino_size, (row - ci.core_y) * ci.domino_size));
        _dominos.push_back(d);
      }
    }
  }
  _domino_size = ci.domino_size;
  _core_x = ci.core_x;
  _core_y = ci.core_y;
}

QRectF Polyomino::boundingRect() const { return QRectF(); }

void Polyomino::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {}

bool Polyomino::IsColliding(QGraphicsScene *scene, QPainterPath shape, QPointF pos) {
  bool colliding = false;
  QTransform t = QTransform::fromTranslate(pos.x(), pos.y());
  shape = shape * t;
  auto items = scene->items(shape);
  for (const auto &item : items) {
    if (item->data(COLLIDING_FLAG_ROLE).toInt()) {
      colliding = true;
      break;
    }
  }
  return colliding;
}

QPainterPath Polyomino::Shape4CollidingDetection() const {
  QPainterPath pp;
  for (const auto &d : _dominos) {
    pp.addRect(QRectF(d->pos() + QPointF(_domino_size / 4, _domino_size / 4), QSizeF(_domino_size / 2, _domino_size / 2)));
  }
  return pp;
}

bool Polyomino::IsColliding() { return IsColliding(scene(), Shape4CollidingDetection(), pos()); }

void Polyomino::Advance(const AdvanceContext &ac) {
  if (!scene() || _landed) {
    return;
  }

  auto dt = ac.last_game_time.msecsTo(ac.game_time);
  auto s = (float)dt / 1000.f * ac.gravity;

  Descend(s);
}
void Polyomino::ForceLand() {
  if (!scene() || _landed) {
    return;
  }

  Descend(std::numeric_limits<qreal>::max());
}

bool Polyomino::IsLanded() { return _landed; }

void Polyomino::Rotate() {
  QTransform t;
  t.rotate(90);

  QPainterPath shape;
  for (const auto &d : _dominos) {
    auto new_pos = d->pos() * t;
    shape.addRect(QRectF(new_pos + QPointF(_domino_size / 4, _domino_size / 4), QSizeF(_domino_size / 2, _domino_size / 2)));
  }

  if (IsColliding(scene(), shape, QPointF(pos().x(), GameScene::GetBoardAlignedPosY(pos().y(), _domino_size)))) {
    return;
  }

  for (auto &d : _dominos) {
    d->setPos(d->pos() * t);
  }
}

QVector<Domino *> Polyomino::Break() {
  for (auto &domino : _dominos) {
    domino->setParentItem(nullptr);
  }
  return std::move(_dominos);
}

void Polyomino::Descend(qreal ds) {
  auto shape = this->Shape4CollidingDetection();

  int detect_offset = std::max(_domino_size, 1);
  qreal detect_range_min = this->pos().y();
  qreal detect_range_max = this->pos().y() + ds;
  qreal detect_start = GameScene::GetBoardAlignedPosY(detect_range_min, detect_offset);
  qreal detect_pos = detect_start;
  bool colliding = false;
  qreal max_y = detect_range_max;
  for (; detect_pos < detect_range_max;) {
    if (IsColliding(scene(), shape, QPointF(this->pos().x(), detect_pos))) {
      colliding = true;
      max_y = detect_pos - detect_offset;
      break;
    }
    detect_pos = detect_pos + detect_offset;
  }
  if (!colliding && IsColliding(scene(), shape, QPointF(this->pos().x(), detect_pos))) {
    colliding = true;
    max_y = detect_pos - detect_offset;
  }

  setY(std::min(max_y, detect_range_max));

  _landed = colliding;
}

//======================
qreal GameScene::GetBoardAlignedPosY(qreal pos, int domino_size) { return std::ceil(pos / domino_size) * domino_size; }

QPoint GameScene::ToBoardPos(QPointF pos, int domino_size) {
  return QPoint((pos.x() + domino_size / 2) / domino_size, (pos.y() + domino_size / 2) / domino_size);
}

GameScene::GameScene(QObject *parent) : QGraphicsScene(parent) { RecreateBoard(); }

void GameScene::SetDominoSize(int size) {
  if (_domino_size == size || size == 0) {
    return;
  }
  _domino_size = size;
  RecreateBoard();
  emit BlockSizeChanged();
}

void GameScene::SetBoardSize(int column, int row) {
  if (_board_size == QSize(column, row) || column == 0 || row == 0) {
    return;
  }
  _board_size = QSize(column, row);
  RecreateBoard();
  emit BoardSizeChanged();
}

void GameScene::SetGravity(float gravity) { _context.gravity = gravity; }

void GameScene::StartGame() {
  killTimer(_advance_timer);

  _context.last_game_time = QDateTime::currentDateTime();

  _advance_timer = startTimer(0);
}

void GameScene::StopGame() { killTimer(_advance_timer); }

void GameScene::Clear() {
  if (_current_polyomino) {
    removeItem(_current_polyomino);
    _current_polyomino->deleteLater();
    _current_polyomino = nullptr;
  }
  for (auto &domino : _active_dominos) {
    if (!domino) {
      continue;
    }
    removeItem(domino);
    domino->deleteLater();
  }
  _active_dominos.clear();
  _active_dominos.resize(_board_size.width() * _board_size.height());
}

void GameScene::TryMovePolyominoLeft() { TryMovePolyomino(-_domino_size, 0.f); }

void GameScene::TryMovePolyominoRight() { TryMovePolyomino(_domino_size, 0.f); }

void GameScene::TryMovePolyominoDown() {
  if (!_current_polyomino) {
    return;
  }

  if (TryMovePolyomino(0.f, _domino_size) == 0) {
    return;
  }

  auto dy = GetBoardAlignedPosY(_current_polyomino->y(), _domino_size) - _current_polyomino->pos().y();
  TryMovePolyomino(0.f, dy);
}

void GameScene::TryRotatePolyomino() {
  if (!_current_polyomino) {
    return;
  }

  _current_polyomino->Rotate();
}

void GameScene::TryLandPolyomino() {
  if (!_current_polyomino) {
    return;
  }

  _current_polyomino->ForceLand();
}

void GameScene::ShowNextPolyomino(PolyominoShape shape) {
  if (_next_polyomino) {
    removeItem(_next_polyomino);
    _next_polyomino->deleteLater();
    _next_polyomino = nullptr;
  }

  _next_polyomino = Create(shape);
  if (!_next_polyomino) {
    return;
  }

  _next_polyomino->setX((_board_size.width() / 4 * 3) * _domino_size);
  _next_polyomino->setY(-4 * _domino_size);
  addItem(_next_polyomino);
}

int GameScene::PutPolyomino(GameScene::PolyominoShape shape) {
  if (_current_polyomino) {
    return 1;
  }

  auto polyomino = Create(shape);
  if (!polyomino) {
    return 1;
  }

  polyomino->setX((_board_size.width() / 2) * _domino_size);
  polyomino->setY(-2 * _domino_size);
  if (Polyomino::IsColliding(this, polyomino->Shape4CollidingDetection(), polyomino->pos())) {
    polyomino->deleteLater();
    return 1;
  }

  _current_polyomino = polyomino;
  addItem(_current_polyomino);
  return 0;
}

void GameScene::timerEvent(QTimerEvent *event) {
  if (event->timerId() != _advance_timer || !_current_polyomino) {
    return;
  }

  _context.game_time = QDateTime::currentDateTime();

  bool try_eliminate = _current_polyomino->IsLanded();

  _current_polyomino->Advance(_context);
  if (_current_polyomino->IsLanded()) {
    auto dominos = _current_polyomino->Break();

    for (auto &domino : dominos) {
      domino->setPos(_current_polyomino->mapToScene(domino->pos()));
      domino->SetCollidingFlag(1);
      if (SetDomino(ToBoardPos(domino->pos(), _domino_size), domino)) {
        domino->deleteLater();
      }
    }

    _current_polyomino->deleteLater();
    _current_polyomino = nullptr;
    emit PolyominoLanded();
  }

  if (try_eliminate) {
    TryEliminate();
  }

  _context.last_game_time = _context.game_time;
}

void GameScene::drawBackground(QPainter *painter, const QRectF &rect) {
  QGraphicsScene::drawBackground(painter, rect);

  QPainter &p = *painter;
  p.setPen(Qt::black);
  p.setClipRect(rect);
  for (int c = 0; c < _board_size.width(); ++c) {
    for (int r = 0; r < _board_size.height(); ++r) {
      QPoint pos(c * _domino_size, r * _domino_size);
      QSize size(_domino_size - 2, _domino_size - 2);
      p.drawRect(QRect(pos, size));
    }
  }
}

Polyomino *GameScene::Create(PolyominoShape shape) {
  Polyomino::CreateInfo ci;
  ci.domino_size = _domino_size;

  switch (shape) {
  case POLYOMINO_SHAPE_I:
    ci.dominos = {{1, 1, 1, 1}};
    ci.core_x = 1;
    ci.core_y = 0;
    break;
  case POLYOMINO_SHAPE_J:
    ci.dominos = {
        {1, 1, 1},
        {0, 0, 1},
    };
    ci.core_x = 2;
    ci.core_y = 0;
    break;
  case POLYOMINO_SHAPE_L:
    ci.dominos = {
        {0, 0, 1},
        {1, 1, 1},
    };
    ci.core_x = 2;
    ci.core_y = 1;
    break;
  case POLYOMINO_SHAPE_S:
    ci.dominos = {
        {0, 1, 1},
        {1, 1, 0},
    };
    ci.core_x = 1;
    ci.core_y = 0;
    break;
  case POLYOMINO_SHAPE_Z:
    ci.dominos = {
        {1, 1, 0},
        {0, 1, 1},
    };
    ci.core_x = 1;
    ci.core_y = 0;
    break;
  case POLYOMINO_SHAPE_O:
    ci.dominos = {
        {1, 1},
        {1, 1},
    };
    ci.core_x = 0;
    ci.core_y = 0;
    break;
  case POLYOMINO_SHAPE_T:
    ci.dominos = {
        {1, 1, 1},
        {0, 1, 0},
    };
    ci.core_x = 1;
    ci.core_y = 0;
    break;
  default:
    return nullptr;
  }

  return new Polyomino(ci);
}

void GameScene::RecreateBoard() {
  for (auto &domino : _static_dominos) {
    removeItem(domino);
    domino->deleteLater();
  }

  for (int i = -4; i < _board_size.height(); ++i) {
    auto ld = new Domino(_domino_size);
    addItem(ld);
    ld->setPos(-_domino_size, i * _domino_size);

    auto rd = new Domino(_domino_size);
    addItem(rd);
    rd->setPos(_domino_size * _board_size.width(), i * _domino_size);

    _static_dominos.push_back(ld);
    _static_dominos.push_back(rd);
  }

  for (int i = -1; i <= _board_size.width(); ++i) {
    auto d = new Domino(_domino_size);
    addItem(d);
    d->setPos(i * _domino_size, _board_size.height() * _domino_size);

    _static_dominos.push_back(d);
  }

  for (auto &domino : _static_dominos) {
    domino->SetCollidingFlag(1);
  }

  _active_dominos.resize(_board_size.width() * _board_size.height());
}

int GameScene::TryMovePolyomino(float dx, float dy) {
  if (!_current_polyomino) {
    return 1;
  }

  auto pos = _current_polyomino->pos();
  auto new_pos = pos + QPointF(dx, dy);
  auto detect_pos = QPointF(new_pos.x(), GetBoardAlignedPosY(new_pos.y(), _domino_size));

  if (Polyomino::IsColliding(this, _current_polyomino->Shape4CollidingDetection(), detect_pos)) {
    return 1;
  }
  _current_polyomino->setPos(new_pos);
  return 0;
}

void GameScene::TryEliminate() {

  int row_count = 0;
  for (int row = _board_size.height() - 1; row >= 0; --row) {
    int valid_count = 0;
    for (int column = 0; column < _board_size.width(); ++column) {
      if (GetDomino(QPoint(column, row))) {
        ++valid_count;
      }
    }

    if (valid_count == _board_size.width()) {
      ++row_count;
      for (int column = 0; column < _board_size.width(); ++column) {
        SetDomino(QPoint(column, row), nullptr);
      }
    } else if (row_count) {
      for (int column = 0; column < _board_size.width(); ++column) {
        if (auto d = GetDomino(QPoint(column, row))) {
          d->setY(d->y() + row_count * _domino_size);
        }
      }
      SwapBoardRow(row, row + row_count);
    }
  }

  if (row_count) {
    emit LineEliminated(row_count);
  }
}

Domino *GameScene::GetDomino(QPoint board_pos) {
  auto index = board_pos.y() * _board_size.width() + board_pos.x();
  if (index < 0 || index >= _active_dominos.size()) {
    return nullptr;
  }
  return _active_dominos[index];
}

int GameScene::SetDomino(QPoint bp, Domino *d) {
  if (bp.x() < 0 || bp.x() >= _board_size.width() || bp.y() < 0 || bp.y() >= _board_size.height()) {
    return 1;
  }

  auto index = bp.y() * _board_size.width() + bp.x();

  if (_active_dominos[index]) {
    _active_dominos[index]->deleteLater();
  }

  _active_dominos[index] = d;
  return 0;
}

void GameScene::SwapBoardRow(int row1, int row2) {
  if (row1 < 0 || row1 >= _board_size.height() || row2 < 0 || row2 >= _board_size.height()) {
    return;
  }

  for (int i = 0; i < _board_size.width(); ++i) {
    auto index1 = row1 * _board_size.width() + i;
    auto index2 = row2 * _board_size.width() + i;
    std::swap(_active_dominos[index1], _active_dominos[index2]);
  }
}
