//
// Created by chudonghao on 22-7-9.
//

#ifndef INC_CHUDONGHAO_22_7_9_B2D2FD578852427F9519F849AE1C03E5_
#define INC_CHUDONGHAO_22_7_9_B2D2FD578852427F9519F849AE1C03E5_

#include <QGraphicsView>

#include "GameScene.h"

class GameView : public QGraphicsView {
  Q_OBJECT
public:
  GameView(QWidget *parent = nullptr);

  GameScene *GetGameScene() const;

private:
  Q_SLOT void on_scene_BoardSizeChanged();
  Q_SLOT void on_scene_BlockSizeChanged();

private:
  void UpdateViewRect();
private:
  GameScene *_scene{};
};

#endif // INC_CHUDONGHAO_22_7_9_B2D2FD578852427F9519F849AE1C03E5_
