#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QPixmap>
#include <QPainter>
#include "processmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onRefreshClicked();
    void onProcessListUpdated(const QList<ProcessInfo>& processes);
    void onTableItemClicked(int row, int column);
    void onSetTopMostClicked();
    void onBringToTopClicked();
    void onAutoRefreshToggled(bool checked);

private:
    void setupUI();
    void populateTable(const QList<ProcessInfo>& processes);
    ProcessInfo getSelectedProcess();

    ProcessManager *processManager;
    QTableWidget *processTable;
    QPushButton *refreshBtn;
    QPushButton *topMostBtn;
    QPushButton *bringToTopBtn;
    QCheckBox *autoRefreshCheck;
    QLabel *statusLabel;
    QPixmap backgroundPixmap;
};

#endif // MAINWINDOW_H
