#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MotorThresholdDetermination.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_clear_clicked();

    void on_sliderVMT_sliderMoved(int position);

    void on_sliderMT_sliderMoved(int position);

    void on_sliderVMT_valueChanged(int value);

    void on_sliderMT_valueChanged(int value);

    void on_VMT_returnPressed();

    void on_EMT_returnPressed();

    void on_Start_clicked();

    void on_checkBox_stateChanged();

private:
    void makePlot(std::vector<double> prior, std::vector<double> likelihood, std::vector<double> posterior);
    void makePESTPlot(std::vector<double> loglikelihoodFunction);
    Ui::MainWindow *ui;
    bool isPEST;
};

#endif // MAINWINDOW_H
