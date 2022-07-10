//
// Created by chudonghao on 22-7-9.
//

#include "GameView.h"

#include "GameScene.h"

GameView::GameView(QWidget *parent) : QGraphicsView(parent) {
  _scene = new GameScene(this);
  _scene->setObjectName("scene");
  setScene(_scene);
  QMetaObject::connectSlotsByName(this);

  UpdateViewRect();
}

GameScene *GameView::GetGameScene() const { return _scene; }

void GameView::on_scene_BoardSizeChanged() { UpdateViewRect(); }

void GameView::on_scene_BlockSizeChanged() { UpdateViewRect(); }

void GameView::UpdateViewRect() {
  auto rect = QRectF(QPointF(), _scene->GetBoardSize() * _scene->GetDominoSize());
  float border = _scene->GetDominoSize() + 2;
  rect.adjust(-border, -_scene->GetDominoSize() * 4, border, border);
  setSceneRect(rect);
  setFixedSize(rect.size().toSize());
}
