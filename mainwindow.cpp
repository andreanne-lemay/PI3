#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MotorThresholdDetermination.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    srand (time(NULL));
    ui->setupUi(this);

    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(QColor(108, 52, 253), 4));
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(QColor(66, 249, 122), 4)); // line color red for second graph
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(QColor(247, 111, 113), 4));
    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot, "Évolution de l'estimation du seuil moteur", QFont("FontAwesome", 12, QFont::Bold)));
    // configure right and top axis to show ticks but no labels:
    // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    QFont font("FontAwesome", 11);
    ui->customPlot->xAxis->setTickLabelFont(font);
    ui->customPlot->yAxis->setTickLabelFont(font);
    ui->customPlot->yAxis->setLabel("Densité");
    ui->customPlot->xAxis->setLabel("Puissance (%MSO)");
    ui->customPlot->xAxis->setLabelFont(font);
    ui->customPlot->yAxis->setLabelFont(font);
    ui->customPlot->xAxis->setRange(0,100);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    ui->customPlot->yAxis->setRange(0, 0.35);


    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // generate some points of data (y0 for first, y1 for second graph):
    QVector<double> x(1001), y0(1001), y1(1001), y2(1001);
    for (int i=0; i<1001; i++)
    {
        x[i] = i / 10.0;
        y0[i] = 0;
        y1[i] = 0;
        y2[i] = 0;
    }

    // pass data points to graphs:
    ui->centralWidget->setStyleSheet("background-color: rgb(245, 245, 245)");
    ui->customPlot->graph(0)->setData(x, y0);
    ui->customPlot->graph(0)->setName("Connaissances antérieures");
    ui->customPlot->graph(1)->setData(x, y1);
    ui->customPlot->graph(1)->setName("Données recueillies");
    ui->customPlot->graph(2)->setData(x, y2);
    ui->customPlot->graph(2)->setName("Prédiction");
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setFont(QFont("FontAwesome",10));
    ui->Start->setStyleSheet("background-color: rgb(180, 224, 184)");
    ui->Start->setFont(QFont("FontAwesome", 18));
    ui->Start->resize(80,30);
    ui->clear->resize(80,30);

    ui->clear->setStyleSheet("background-color: rgb(230,230,230)");
    ui->clear->setFont(QFont("FontAwesome", 18));

    // Note: we could have also just called customPlot->rescaleAxes(); instead
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->seuil->setFixedSize(160,100);
    ui->seuil->setSegmentStyle(QLCDNumber::Filled);
    ui->seuil->setStyleSheet("color: rgb(0,0,0)");
    ui->seuil->setPalette(Qt::black);
    ui->nbPulses->setSegmentStyle(QLCDNumber::Filled);
    ui->nbPulses->setStyleSheet("color: rgb(0,0,0)");
    ui->nbPulses->setPalette(Qt::black);
    ui->nbPulses->setFixedSize(160,100);
    ui->sliderMT->setValue(50);
    ui->sliderMT->setRange(0, 100);
    ui->sliderVMT->setValue(50);
    ui->sliderVMT->setRange(0, 100);
    connect(ui->sliderMT, SIGNAL(valueChanged(int)) ,ui->EMT, SLOT(display(int)));
    connect(ui->sliderVMT, SIGNAL(valueChanged(int)) ,ui->VMT, SLOT(display(int)));
    ui->EMT->setText("50");
    ui->EMT->setFont(QFont("FontAwesome", 18));
    ui->EMT->setAlignment(Qt::AlignCenter);
    ui->EMT->setStyleSheet("background-color: rgb(255,255,255)");
    ui->VMT->setText("50");
    ui->VMT->setFont(QFont("FontAwesome", 18));
    ui->VMT->setAlignment(Qt::AlignCenter);
    ui->VMT->setStyleSheet("background-color: rgb(255,255,255)");

    ui->nbPulses->setDigitCount(QString("%1").arg(0).length());
    ui->seuil->setDigitCount(QString("%1").arg(0).length());
    isPEST = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::makePlot(std::vector<double> prior, std::vector<double> likelihood, std::vector<double> posterior)
{
    // generate some points of data (y0 for first, y1 for second graph):
    QVector<double> x(1001), y0(1001), y1(1001), y2(1001);
    for (size_t i=0; i<1001; i++)
    {
      x[i] = i / 10.0;
      y0[i] = prior[i];
      y1[i] = likelihood[i];
      y2[i] = posterior[i];
    }
    ui->customPlot->graph(0)->setData(x, y0);
    ui->customPlot->graph(0)->setName("Connaissances antérieures");
    ui->customPlot->graph(1)->setData(x, y1);
    ui->customPlot->graph(1)->setName("Données recueillies");
    ui->customPlot->graph(2)->setData(x, y2);
    ui->customPlot->graph(2)->setName("Prédiction");
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->graph(2)->rescaleAxes(true);
    ui->customPlot->graph(1)->rescaleAxes(true);
    ui->customPlot->graph(0)->rescaleAxes(true);
    ui->customPlot->yAxis->setLabel("Densité");
    ui->customPlot->replot();
    ui->customPlot->update();
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::makePESTPlot(std::vector<double> loglikelihoodFunction)
{
    ui->customPlot->graph(0)->data()->clear();
    ui->customPlot->graph(1)->data()->clear();
    ui->customPlot->graph(2)->data()->clear();
    ui->customPlot->legend->setVisible(false);
    QVector<double> x(1001), y0(1001);
    for (size_t i=0; i<1001; i++)
    {
      x[i] = i / 10.0;
      y0[i] = loglikelihoodFunction[i];
    }
    ui->customPlot->graph(0)->setData(x, y0);
    ui->customPlot->graph(0)->rescaleAxes();
    ui->customPlot->yAxis->setLabel("Fonction de vraisemblance");


    ui->customPlot->replot();
    ui->customPlot->update();
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::on_clear_clicked()
{
    ui->customPlot->graph(0)->data()->clear();
    ui->customPlot->graph(1)->data()->clear();
    ui->customPlot->graph(2)->data()->clear();
    ui->customPlot->yAxis->setRange(0, 0.35);
    ui->customPlot->replot();

    ui->sliderMT->setValue(50);
    ui->EMT->setText("50");
    ui->sliderVMT->setValue(50);
    ui->VMT->setText("50");
    ui->seuil->setDigitCount(1);
    ui->seuil->display(0);
    ui->nbPulses->setDigitCount(1);
    ui->nbPulses->display(0);
}


void MainWindow::on_sliderVMT_sliderMoved(int position)
{
    QString valeur = QString::number(double(position));
    ui->VMT->setText(valeur);
}

void MainWindow::on_sliderMT_sliderMoved(int position)
{
    QString valeur = QString::number(double(position));
    ui->EMT->setText(valeur);
}

void MainWindow::on_sliderVMT_valueChanged(int value)
{
    QString valeur = QString::number(double(value));
    ui->VMT->setText(valeur);
}

void MainWindow::on_sliderMT_valueChanged(int value)
{
    QString valeur = QString::number(double(value));
    ui->EMT->setText(valeur);
}


void MainWindow::on_VMT_returnPressed()
{
    QString valeur = ui->VMT->text();
    ui->sliderVMT->setValue(valeur.toDouble());
}

void MainWindow::on_EMT_returnPressed()
{
    QString valeur = ui->EMT->text();
    ui->sliderMT->setValue(valeur.toDouble());
}

void MainWindow::on_Start_clicked()
{
     MotorThresholdDetermination MT;

     bool stopCriteria = false;

     std::vector<double> prior = MT.GetPriorFunction();
     std::vector<double> likelihood = MT.GetLikelihoodFunction();
     std::vector<double> posterior = MT.GetPosteriorFunction();

     QString priorInfo = ui->EMT->text();
     MT.SetPriorMT(priorInfo.toDouble());
     QString realMT = ui->VMT->text();
     double trueMT = realMT.toDouble();

     if (isPEST)
     {

         MT.ChangeAlgorithm(true);
     }

     while (!stopCriteria)
     {
         ui->Start->setStyleSheet("background-color: rgb(255, 219, 219)");
         ui->Start->setText("En cours");

         prior = MT.GetPriorFunction();
         MT.UpdateMotorThresholdCandidate();
         double MTC = MT.GetMotorThresholdCandidate();
         MT.AddPowerCandidate(MTC);
         MT.AddMEPResult(MT.MotorThresholdSimulation(MTC, trueMT));

         if (!isPEST)
         {
            MT.UpdateBayesianFunctions();
            likelihood = MT.GetLikelihoodFunction();
            posterior = MT.GetPosteriorFunction();

            this->makePlot(prior, likelihood, posterior);


            if (MT.GetConfidenceInterval() > 0.95)
            {
                stopCriteria = true;
            }
            usleep(5e5);
         }

         else
         {
            std::vector<double> MTCandidates = MT.GetMotorThresholdVector();
            this->makePESTPlot(MT.GetLogLikelihoodFunction());


            if(round(MTCandidates.back()) == round(MTCandidates[MTCandidates.size() - 2]))
            {
                stopCriteria = true;
            }
            usleep(2e5);
         }

     }
     MT.UpdateMotorThresholdCandidate();
     ui->Start->setText("Commencer");
     ui->Start->setStyleSheet("background-color: rgb(180, 224, 184)");
     QString seuilMoteur = QString::number(double(MT.GetMotorThresholdCandidate()));
     ui->seuil->setDigitCount(QString("%1").arg(MT.GetMotorThresholdCandidate()).length());
     ui->seuil->display(seuilMoteur);
     ui->nbPulses->setDigitCount(QString("%1").arg(MT.GetNumberOfPulses()).length());
     ui->nbPulses->display(double(MT.GetNumberOfPulses()));


}


void MainWindow::on_checkBox_stateChanged()
{
    ui->customPlot->graph(0)->data()->clear();
    ui->customPlot->graph(1)->data()->clear();
    ui->customPlot->graph(2)->data()->clear();
    ui->customPlot->yAxis->setRange(0, 0.35);
    ui->customPlot->replot();

    isPEST = !isPEST;
}
