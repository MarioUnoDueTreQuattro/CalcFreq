#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once
#include <QMainWindow>
#include <QTableWidget>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void calculateHarmonics();
    void calculateNoteToBpm();
    void calculateTempo();
    void calculateLfo();
    void calculateCycleTuner();
    void exportVoxengoGlissEq();
    void copyTableCellToClipboard(int row, int column, QTableWidget *table);
private:
    Ui::MainWindow *ui;
    void initUi();
    void loadSettings();
    void saveSettings();
};
#endif // MAINWINDOW_H
