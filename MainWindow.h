//
// Created by chudonghao on 22-7-9.
//

#ifndef INC_CHUDONGHAO_22_7_9_1DD262849C3249ADB28E0868BF14AA0D_
#define INC_CHUDONGHAO_22_7_9_1DD262849C3249ADB28E0868BF14AA0D_

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class GameScene;
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private:
  Q_SLOT void on_action_restart_triggered();
  Q_SLOT void on_action_set_level_triggered();
  void on_game_scene_PolyominoLanded();
  void on_game_scene_LineEliminated(int count);
  void handle_shortcut_left();
  void handle_shortcut_right();
  void handle_shortcut_up();
  void handle_shortcut_down();
  void handle_shortcut_space();
  void handle_shortcut_alt_up();
  void handle_shortcut_alt_down();

private:
  Ui::MainWindow *ui;

  GameScene *_game_scene{};
  long long _score{};
  long long _line{};
  int _level{1};
  bool _game_running{};
  int _next_shape{};
};

#endif // INC_CHUDONGHAO_22_7_9_1DD262849C3249ADB28E0868BF14AA0D_
