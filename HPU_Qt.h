#pragma once

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QtWidgets/QMainWindow>

#include "ui_HPU_Qt.h"
#include "HPUnpack_revive.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HPU_QtClass; };
QT_END_NAMESPACE

class HPU_Qt : public QMainWindow
{
    Q_OBJECT

public:
    HPU_Qt(QWidget *parent = nullptr);
    ~HPU_Qt();
    QMessageBox messageBox;

private slots:
    void InitButtons();
    void Load();
    void Save();
    void SaveAs();
    void OpenReport();
    void Close();
    void Extract();
    void Replace();
    void Fixup();
    void Browse_Extract();
    void Browse_Replace();
    bool CheckLoaded(bool firstIn);
    void SetRows(uint8_t lineNumber, QStringList parts);
    void ResetRows(uint8_t lineNumber);

private:
    Ui::HPU_QtClass *PInterface;
    QString romFileName;
    QString newRomFileName;
    QString savedModuleName;
    QString newModuleName;
    std::vector<uint8_t> biosData;
    uint32_t headerOffsets[255];
    uint32_t dataOffsets[255];
    uint8_t headerSize;
    int systemReturnCode;
    bool isHeader2;
};

