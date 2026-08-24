#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "audioutils.h"

#include <QSettings>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUi();
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings()
{
    QSettings settings;
    // Ripristino Geometria e Stato Finestra
    if (settings.contains("geometry"))
    {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
    if (settings.contains("windowState"))
    {
        restoreState(settings.value("windowState").toByteArray());
    }
    // Ripristino valori controlli UI
    if (settings.contains("comboNoteIndex"))
    {
        ui->comboNote->setCurrentIndex(settings.value("comboNoteIndex").toInt());
    }
    if (settings.contains("comboOctaveModeIndex"))
    {
        ui->comboOctaveMode->setCurrentIndex(settings.value("comboOctaveModeIndex").toInt());
    }
    if (settings.contains("bpmValue"))
    {
        ui->spinBpm->setValue(settings.value("bpmValue").toDouble());
    }
    if (settings.contains("cycleVal"))
    {
        ui->spinCycleVal->setValue(settings.value("cycleVal").toDouble());
    }
    if (settings.contains("radioMsChecked"))
    {
        bool isMs = settings.value("radioMsChecked").toBool();
        ui->radioMs->setChecked(isMs);
        ui->radioHz->setChecked(!isMs);
    }
    // Esegue i calcoli aggiornati dopo il ripristino
    calculateHarmonics();
    calculateNoteToBpm();
    calculateTempo();
    calculateLfo();
    calculateCycleTuner();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    // Salvataggio Geometria e Stato Finestra
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    // Salvataggio Valori Controlli
    settings.setValue("comboNoteIndex", ui->comboNote->currentIndex());
    settings.setValue("comboOctaveModeIndex", ui->comboOctaveMode->currentIndex());
    settings.setValue("bpmValue", ui->spinBpm->value());
    settings.setValue("cycleVal", ui->spinCycleVal->value());
    settings.setValue("radioMsChecked", ui->radioMs->isChecked());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}
void MainWindow::initUi()
{
    ui->tableLfo->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableHarmonics->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableNoteToBpm->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableTempo->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->comboNote->addItems(AudioUtils::NOTE_NAMES);
    ui->comboOctaveMode->addItems(
    {
        "Octave 1 (Linear)", "Octave 2 (Linear)", "Octave 3 (Linear)", "Octave 4 (Linear)",
        "Octave 1 (Exponential)", "Octave 2 (Exponential)", "Octave 3 (Exponential)", "Octave 4 (Exponential)"
    });
    // Signals
    connect(ui->comboNote, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::calculateHarmonics);
    connect(ui->comboNote, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::calculateNoteToBpm);
    connect(ui->comboOctaveMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::calculateHarmonics);
    connect(ui->spinBpm, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::calculateTempo);
    connect(ui->spinBpm, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::calculateLfo);
    connect(ui->spinCycleVal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::calculateCycleTuner);
    connect(ui->radioMs, &QRadioButton::toggled, this, &MainWindow::calculateCycleTuner);
    connect(ui->radioHz, &QRadioButton::toggled, this, &MainWindow::calculateCycleTuner);
    connect(ui->btnExport, &QPushButton::clicked, this, &MainWindow::exportVoxengoGlissEq);
    // Clipboard copy on click
    auto copyLambda = [this](int r, int c, QTableWidget * t) { copyTableCellToClipboard(r, c, t); };
    connect(ui->tableHarmonics, &QTableWidget::cellClicked, this, [ =](int r, int c) { copyLambda(r, c, ui->tableHarmonics); });
    connect(ui->tableNoteToBpm, &QTableWidget::cellClicked, this, [ =](int r, int c) { copyLambda(r, c, ui->tableNoteToBpm); });
    connect(ui->tableTempo, &QTableWidget::cellClicked, this, [ =](int r, int c) { copyLambda(r, c, ui->tableTempo); });
    connect(ui->tableLfo, &QTableWidget::cellClicked, this, [ =](int r, int c) { copyLambda(r, c, ui->tableLfo); });
    ui->statusBar->showMessage("Click any cell to copy its value.");
    // First Calculation
    calculateHarmonics();
    calculateNoteToBpm();
    calculateTempo();
    calculateLfo();
    calculateCycleTuner();
}

void MainWindow::calculateHarmonics()
{
    ui->tableHarmonics->setRowCount(0);
    int noteIndex = ui->comboNote->currentIndex();
    int mode = ui->comboOctaveMode->currentIndex();
    int octaveMultiplier = 1;
    bool isExponential = (mode >= 4);
    int modeIndex = mode % 4;
    if (modeIndex == 1) octaveMultiplier = 2;
    else if (modeIndex == 2) octaveMultiplier = 4;
    else if (modeIndex == 3) octaveMultiplier = 8;
    double baseFreq = AudioUtils::noteToFrequency(noteIndex, 0) * octaveMultiplier;
    int maxHarmonics = isExponential ? 10 : 256;
    for (int i = 1; i <= maxHarmonics; ++i)
    {
        double harmonicFactor = isExponential ? std::pow(2.0, i) : static_cast<double>(i + 1);
        double freq = baseFreq * harmonicFactor;
        int row = ui->tableHarmonics->rowCount();
        ui->tableHarmonics->insertRow(row);
        ui->tableHarmonics->setItem(row, 0, new QTableWidgetItem(QString::number(freq, 'f', 2)));
        ui->tableHarmonics->setItem(row, 1, new QTableWidgetItem(QString::number(harmonicFactor)));
    }
    ui->tableHarmonics->resizeColumnsToContents();
}

void MainWindow::calculateNoteToBpm()
{
    ui->tableNoteToBpm->setRowCount(0);
    int noteIndex = ui->comboNote->currentIndex();
    double noteFreqHz = AudioUtils::noteToFrequency(noteIndex, 0);
    double baseBpm = noteFreqHz * 60.0;
    for (int coef = 2; coef <= 60; ++coef)
    {
        double subdividedBpm = baseBpm / static_cast<double>(coef);
        int row = ui->tableNoteToBpm->rowCount();
        ui->tableNoteToBpm->insertRow(row);
        ui->tableNoteToBpm->setItem(row, 0, new QTableWidgetItem(QString::number(subdividedBpm, 'f', 6)));
        ui->tableNoteToBpm->setItem(row, 1, new QTableWidgetItem(QString::number(coef)));
    }
    ui->tableNoteToBpm->resizeColumnsToContents();
}

void MainWindow::calculateTempo()
{
    ui->tableTempo->setRowCount(0);
    double bpm = ui->spinBpm->value();
    if (bpm <= 0.0) return;
    // Durata del quarto (1/4) in ms
    double quarterNoteMs = 60000.0 / bpm;
    // Sottomultipli: da 1/1024 a 1/8
    for (int i = 10; i >= 3; --i)
    {
        double factor = std::pow(2.0, i); // 1024 down to 8
        double durationMs = quarterNoteMs * (4.0 / factor);
        int row = ui->tableTempo->rowCount();
        ui->tableTempo->insertRow(row);
        ui->tableTempo->setItem(row, 0, new QTableWidgetItem(QString("1/%1").arg(factor)));
        ui->tableTempo->setItem(row, 1, new QTableWidgetItem(QString::number(durationMs, 'f', 6)));
    }
    // Multipli: da 1/4 (1) fino a battute intere (512)
    for (int i = 0; i <= 9; ++i)
    {
        double factor = std::pow(2.0, i); // 1, 2, 4, 8 ... 512
        double durationMs = quarterNoteMs * factor;
        int row = ui->tableTempo->rowCount();
        ui->tableTempo->insertRow(row);
        // Se factor == 1 è il quarto (1/4), altrimenti mostra il moltiplicatore
        QString noteLabel = (i == 0) ? "1/4" : QString::number(factor);
        ui->tableTempo->setItem(row, 0, new QTableWidgetItem(noteLabel));
        ui->tableTempo->setItem(row, 1, new QTableWidgetItem(QString::number(durationMs, 'f', 6)));
    }
    ui->tableTempo->resizeColumnsToContents();
}
void MainWindow::calculateLfo()
{
    ui->tableLfo->setRowCount(0);
    double bpm = ui->spinBpm->value();
    if (bpm <= 0.0) return;
    // Frequenza del quarto (1/4) in Hz -> (BPM / 60)
    double quarterHz = bpm / 60.0;
    // Sottomultipli veloci: da 1/1024 a 1/8
    for (int i = 10; i >= 3; --i)
    {
        double factor = std::pow(2.0, i); // 1024 down to 8
        double lfoHz = quarterHz * (factor / 4.0);
        int row = ui->tableLfo->rowCount();
        ui->tableLfo->insertRow(row);
        ui->tableLfo->setItem(row, 0, new QTableWidgetItem(QString("1/%1").arg(factor)));
        //ui->tableLfo->setItem(row, 1, new QTableWidgetItem(QString::number(lfoHz, 'f', 12)));
        ui->tableLfo->setItem(row, 1, new QTableWidgetItem(QString::number(lfoHz, 'g', 16)));
    }
    // Multipli lenti: da 1/4 fino a 512 battute
    for (int i = 0; i <= 9; ++i)
    {
        double factor = std::pow(2.0, i); // 1, 2, 4... 512
        double lfoHz = (quarterHz / 4.0) / factor;
        int row = ui->tableLfo->rowCount();
        ui->tableLfo->insertRow(row);
        QString noteLabel = (i == 0) ? "1/4" : QString::number(factor);
        ui->tableLfo->setItem(row, 0, new QTableWidgetItem(noteLabel));
        // 8 decimali di precisione
        ui->tableLfo->setItem(row, 1, new QTableWidgetItem(QString::number(lfoHz, 'f', 8)));
    }
    ui->tableLfo->resizeColumnsToContents();
}

void MainWindow::calculateCycleTuner()
{
    double val = ui->spinCycleVal->value();
    NoteInfo info = ui->radioMs->isChecked() ? AudioUtils::periodMsToNote(val) : AudioUtils::frequencyToNote(val);
    if (info.name.isEmpty()) return;
    ui->lblResNote->setText(QString("%1%2").arg(info.name).arg(info.octave));
    ui->lblResFreq->setText(QString::number(info.targetFrequency, 'f', 2) + " Hz");
    QString sign = info.centsOffset >= 0 ? "+" : "";
    ui->lblResCents->setText(QString("%1%2 cents").arg(sign).arg(info.centsOffset, 0, 'f', 1));
}

void MainWindow::copyTableCellToClipboard(int row, int column, QTableWidget *table)
{
    QTableWidgetItem *item = table->item(row, column);
    if (item)
    {
        QApplication::clipboard()->setText(item->text());
        ui->statusBar->showMessage(QString("Copied '%1' to clipboard.").arg(item->text()), 3000);
    }
}

void MainWindow::exportVoxengoGlissEq()
{
    QList<QTableWidgetItem *> selected = ui->tableHarmonics->selectedItems();
    QStringList validFreqs;
    for (auto *item : selected)
    {
        if (item->column() == 0) validFreqs.append(item->text());
    }
    if (validFreqs.isEmpty())
    {
        QMessageBox::information(this, "Export", "Select frequencies in the harmonics table first.");
        return;
    }
    if (validFreqs.size() > 5)
    {
        QMessageBox::warning(this, "Export", "Only the first 5 selected frequencies will be exported.");
        while (validFreqs.size() > 5) validFreqs.removeLast();
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Save GlissEQ File", "", "EQ Files (*.EQ)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        for (const QString &f : validFreqs) out << f << ",3,0.2\n";
        file.close();
        ui->statusBar->showMessage("GlissEQ file exported successfully.", 3000);
    }
}
