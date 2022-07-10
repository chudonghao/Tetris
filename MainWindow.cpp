//
// Created by chudonghao on 22-7-9.
//

#include "MainWindow.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSettings>
#include <QShortcut>

#include "GameView.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  _game_scene = ui->game_view->GetGameScene();

  QSettings settings;
  settings.beginGroup("MainWindow");
  _level = std::max(1, settings.value("level").toInt());
  _next_shape = QRandomGenerator::global()->generate() % _game_scene->POLYOMINO_SHAPE_SIZE;

  ui->label_level->setText(QString::number(_level));
  ui->label_score->setText(QString::number(_score));
  ui->label_line->setText(QString::number(_line));

  _game_scene->SetBoardSize(10, 20);
  _game_scene->SetGravity(_level * _game_scene->GetDominoSize());
  _game_scene->ShowNextPolyomino(static_cast<GameScene::PolyominoShape>(_next_shape));
  auto text = _game_scene->addText("下一个：");
  text->setY(-_game_scene->GetDominoSize() * 4);

  connect(_game_scene, &GameScene::PolyominoLanded, this, &MainWindow::on_game_scene_PolyominoLanded);
  connect(_game_scene, &GameScene::LineEliminated, this, &MainWindow::on_game_scene_LineEliminated);

  new QShortcut(QKeySequence(Qt::Key_Left), this, this, &MainWindow::handle_shortcut_left);
  new QShortcut(QKeySequence(Qt::Key_Right), this, this, &MainWindow::handle_shortcut_right);
  new QShortcut(QKeySequence(Qt::Key_Up), this, this, &MainWindow::handle_shortcut_up);
  new QShortcut(QKeySequence(Qt::Key_Down), this, this, &MainWindow::handle_shortcut_down);
  new QShortcut(QKeySequence(Qt::Key_A), this, this, &MainWindow::handle_shortcut_left);
  new QShortcut(QKeySequence(Qt::Key_D), this, this, &MainWindow::handle_shortcut_right);
  new QShortcut(QKeySequence(Qt::Key_W), this, this, &MainWindow::handle_shortcut_up);
  new QShortcut(QKeySequence(Qt::Key_S), this, this, &MainWindow::handle_shortcut_down);
  new QShortcut(QKeySequence(Qt::Key_Space), this, this, &MainWindow::handle_shortcut_space);
  new QShortcut(QKeySequence("Alt+Up"), this, this, &MainWindow::handle_shortcut_alt_up);
  new QShortcut(QKeySequence("Alt+Down"), this, this, &MainWindow::handle_shortcut_alt_down);
}

MainWindow::~MainWindow() {
  QSettings settings;
  settings.beginGroup("MainWindow");
  settings.setValue("level", _level);
  delete ui;
}

void MainWindow::on_action_restart_triggered() {
  _game_scene->Clear();
  _game_scene->StartGame();
  _score = 0;
  _line = 0;
  ui->label_score->setText(QString::number(_score));
  ui->label_line->setText(QString::number(_line));

  // put first polyomino
  int shape = _next_shape;
  _game_running = 0 == _game_scene->PutPolyomino(static_cast<GameScene::PolyominoShape>(shape));
  _next_shape = QRandomGenerator::global()->generate() % _game_scene->POLYOMINO_SHAPE_SIZE;
  _game_scene->ShowNextPolyomino(static_cast<GameScene::PolyominoShape>(_next_shape));
}

void MainWindow::on_action_set_level_triggered() {
  _level = QInputDialog::getInt(this, "设置级别", "级别", _level, 1, 100);
  ui->label_level->setText(QString::number(_level));
  _game_scene->SetGravity(_level * _game_scene->GetDominoSize());
}

void MainWindow::on_game_scene_PolyominoLanded() {
  int shape = _next_shape;
  _game_running = 0 == _game_scene->PutPolyomino(static_cast<GameScene::PolyominoShape>(shape));
  _next_shape = QRandomGenerator::global()->generate() % _game_scene->POLYOMINO_SHAPE_SIZE;
  _game_scene->ShowNextPolyomino(static_cast<GameScene::PolyominoShape>(_next_shape));
  if (!_game_running) {
    QMessageBox::information(this, "游戏结束", "GAME OVER");
  } else {
    _score += _level * 10;
    ui->label_score->setText(QString::number(_score));
  }
}

void MainWindow::on_game_scene_LineEliminated(int count) {
  _score += _level * 100 * count * count;
  _line += count;
  ui->label_score->setText(QString::number(_score));
  ui->label_line->setText(QString::number(_line));
}

void MainWindow::handle_shortcut_left() { _game_scene->TryMovePolyominoLeft(); }
void MainWindow::handle_shortcut_right() { _game_scene->TryMovePolyominoRight(); }
void MainWindow::handle_shortcut_up() { _game_scene->TryRotatePolyomino(); }
void MainWindow::handle_shortcut_down() { _game_scene->TryMovePolyominoDown(); }
void MainWindow::handle_shortcut_space() {
  if (_game_running) {
    _game_scene->TryLandPolyomino();
  } else {
    // simulate restart action
    on_action_restart_triggered();
  }
}
void MainWindow::handle_shortcut_alt_up() {
  _level = std::clamp(_level + 1, 1, 100);
  ui->label_level->setText(QString::number(_level));
  _game_scene->SetGravity(_level * _game_scene->GetDominoSize());
}
void MainWindow::handle_shortcut_alt_down() {
  _level = std::clamp(_level - 1, 1, 100);
  ui->label_level->setText(QString::number(_level));
  _game_scene->SetGravity(_level * _game_scene->GetDominoSize());
}
