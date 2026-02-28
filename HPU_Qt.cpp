#include "HPU_Qt.h"

void HPU_Qt::InitButtons()
{
    connect(PInterface->btnLoad, &QPushButton::clicked, this, &HPU_Qt::Load);
    connect(PInterface->btnSave, &QPushButton::clicked, this, &HPU_Qt::Save);
    connect(PInterface->btnSaveAs, &QPushButton::clicked, this, &HPU_Qt::SaveAs);
    connect(PInterface->btnCreateReport, &QPushButton::clicked, this, &HPU_Qt::OpenReport);
    connect(PInterface->btnExit, &QPushButton::clicked, this, &HPU_Qt::Close);
    connect(PInterface->btnExtract, &QPushButton::clicked, this, &HPU_Qt::Extract);
    connect(PInterface->btnReplace, &QPushButton::clicked, this, &HPU_Qt::Replace);
    connect(PInterface->btnFixup, &QPushButton::clicked, this, &HPU_Qt::Fixup);
    connect(PInterface->btnBrowse_Extract, &QPushButton::clicked, this, &HPU_Qt::Browse_Extract);
    connect(PInterface->btnBrowse_Replace, &QPushButton::clicked, this, &HPU_Qt::Browse_Replace);
}

void HPU_Qt::Load()
{
    romFileName = QFileDialog::getOpenFileName(this, 
                                                tr("Open ROM"),
                                                QDir::currentPath(),
                                                tr("All Files (*.*)"));

    if (romFileName.isEmpty()) { return; }
    else
    {
        CheckLoaded(TRUE);
        for (int i = 0; i <= 255; i++) {
            headerOffsets[i] = NULL;
        }
        isHeader2 = FALSE;
    }

    if (UINT8_MAX < romFileName.toStdString().length()){
        messageBox.critical(0, "Error", "Filepath exceeds windows path length limit!");
        return;
    }

    Utils::saveFileToBuffer(romFileName.toStdString(), biosData);
    if (biosData.empty())
    {
        messageBox.critical(0, "Error", "Failed to load ROM file!");
    }

    headerOffsets[0] = *HPUnpack_revive::DecatenateBlocks(romFileName.toStdString(), isHeader2);
    headerSize = isHeader2 ? 0x14 : 0x10;

    if (headerOffsets[0]) {
        HPUnpack_revive::CreateReport(biosData, headerOffsets, headerSize, isHeader2);
    }
    else
    {
        messageBox.critical(0, "Error", "Incompatible ROM file!\n" + romFileName);
        return;
    }

    QFile reportFile(QDir::currentPath() + "/report.txt");
    for (uint8_t readAttempts = 0; !reportFile.open(QIODevice::ReadOnly | QIODevice::Text); readAttempts++) {
        if (readAttempts >= 3) 
        { 
            messageBox.critical(0, "Error", "Could not open report file for reading!\n" + QDir::currentPath() + "/report.txt");
            return; 
        }
        QThread::sleep(3);
    }

    QTextStream in(&reportFile);
    uint8_t lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty() || lineNumber >= PInterface->tableWidget->rowCount()) break;

        QStringList parts = line.split(QRegularExpression("[ ;]+"), Qt::SkipEmptyParts);

        SetRows(lineNumber, parts);

        lineNumber++;
    }

    if (!lineNumber)
    {
        messageBox.critical(0, "Error", "Incompatible ROM file!\n" + romFileName);
        return;
    }
    else
    {
        ResetRows(lineNumber);

        messageBox.information(0, "Information", romFileName + " loaded successfully.");
    }

    CheckLoaded(FALSE);
    return;
}

void HPU_Qt::Save()
{
    newRomFileName = QDir::currentPath() + (QString)"/newBios.rom";
    Utils::saveBufferToFile(newRomFileName.toStdString(), biosData, biosData.size());
    messageBox.information(0, "Information", "Saved new ROM as " + newRomFileName);
}

void HPU_Qt::SaveAs()
{
    newRomFileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save New ROM File"),
                                                    QDir::currentPath(),
                                                    tr("All Files (*.*)"));

    if (newRomFileName.isEmpty()) {
        return;
    }
    else
    {
        Utils::saveBufferToFile(newRomFileName.toStdString(), biosData, biosData.size());
        messageBox.information(0, "Information", "Saved new ROM as " + newRomFileName);
    }
}

void HPU_Qt::OpenReport()
{
    systemReturnCode = std::system((char *)"start report.txt");
    if (romFileName.isEmpty() || systemReturnCode != 0) {
        messageBox.critical(0, "Error", "Failed opening report!");
        return;
    }
}

void HPU_Qt::Close()
{
    HPU_Qt::~HPU_Qt();
}

void HPU_Qt::Extract()
{
    if (!CheckLoaded(FALSE)) return;

    if (PInterface->linePath_Extract->text().isEmpty() || romFileName.isEmpty()) {
        messageBox.critical(0, "Error", "Please specify module file path!");
        return;
    }

    QList<QTableWidgetItem *> selectedItems = PInterface->tableWidget->selectedItems();
    int8_t SelectedRow = -1;

    if (!selectedItems.isEmpty()) {
        QModelIndexList selectedRows = PInterface->tableWidget->selectionModel()->selectedRows();
        SelectedRow = selectedRows.at(0).row();
    }
    else
    {
        messageBox.critical(0, "Error", "Please select some module!");
        return;
    }
    PInterface->lineModuleId_Extract->setText(QString::number(SelectedRow));

    if (PInterface->tableWidget->item(SelectedRow, 0)->text().isEmpty())
	{
		messageBox.critical(0, "Error", "Failed to find module with ID: " + QString::number(SelectedRow));
        return;
	}

    if (
        HPUnpack_revive::Unpack(biosData,
                                romFileName.toStdString(),
                                savedModuleName.toStdString(),
                                headerOffsets,
                                PInterface->lineModuleId_Extract->text().toInt(),
                                PInterface->checkBox_uncompressed->isChecked(),
                                isHeader2)
    )
    {
        messageBox.information(0, "Information", "Saved BIOS module as " + PInterface->linePath_Extract->text());
    }
}

void HPU_Qt::Replace()
{  
    if (!CheckLoaded(FALSE)) return;

    std::vector<uint8_t> freshBiosData;
    if (biosData == freshBiosData || biosData.empty())
    {
        Utils::saveFileToBuffer(romFileName.toStdString(), biosData);
    }

    QList<QTableWidgetItem *> selectedItems = PInterface->tableWidget->selectedItems();
    int8_t SelectedRow = -1;

    if (!selectedItems.isEmpty()) {
        QModelIndexList selectedRows = PInterface->tableWidget->selectionModel()->selectedRows();
        SelectedRow = selectedRows.at(0).row();
    }
    else
    {
        messageBox.critical(0, "Error", "Please select some module!");
        return;
    }

    if (!isHeader2) {
        messageBox.information(0, "Information", "Header 1 BIOS type unsupported for compression.\nGoing to replace with 0% compression rate.");
    }
    PInterface->lineModuleId_Replace->setText(QString::number(SelectedRow));

    if (PInterface->tableWidget->item(SelectedRow, 0)->text().isEmpty())
	{
		messageBox.critical(0, "Error", "Failed to find module with ID: " + QString::number(SelectedRow));
        return;
	}
    

    if (!PInterface->linePath_Replace->text().isEmpty())
    {
        biosData = HPUnpack_revive::CompressAndReplace(biosData, PInterface->linePath_Replace->text().toStdString(), PInterface->lineModuleId_Replace->text().toInt(), isHeader2);

        if (biosData.size() < UINT16_MAX) {
            messageBox.critical(0, "Error", "Fatal error replacing module in BIOS buffer!");
            Close();
            return;
        }
    }
    else
    {
        messageBox.critical(0, "Error", "Please specify module file path!");
        return;
    }

    Utils::saveFileToBuffer(romFileName.toStdString(), freshBiosData);
    if (biosData == freshBiosData)
    {
        messageBox.critical(0, "Error", "Failed to compress the module file: empty data!");
    }
    else
    {
        PInterface->btnSave->setEnabled(TRUE);
        PInterface->btnSaveAs->setEnabled(TRUE);

        messageBox.information(0, "Information", "Module ID: " + QString::number(SelectedRow) + " replace success!");
    }

    HPUnpack_revive::CreateReport(biosData, headerOffsets, headerSize, isHeader2);

    QFile reportFile(QDir::currentPath() + "/report.txt");
    for (uint8_t readAttempts = 0; !reportFile.open(QIODevice::ReadOnly | QIODevice::Text); readAttempts++) {
        if (readAttempts >= 3) 
        { 
            messageBox.critical(0, "Error", "Could not open report file for reading!\n" + QDir::currentPath() + "/report.txt");
            return; 
        }
        QThread::sleep(3);
    }
    QTextStream in(&reportFile);
    uint8_t lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.isEmpty() || lineNumber >= PInterface->tableWidget->rowCount()) {
            ResetRows(lineNumber);

            break;
        }
        QStringList parts = line.split(QRegularExpression("[ ;]+"), Qt::SkipEmptyParts);

        SetRows(lineNumber, parts);

        lineNumber++;
    }
}

void HPU_Qt::Fixup()
{
    if (CheckLoaded(FALSE))
    {
        std::vector<uint8_t> freshBiosData;
        Utils::saveFileToBuffer(romFileName.toStdString(), freshBiosData);
        biosData = HPUnpack_revive::DisableCHKSUM(romFileName.toStdString(), biosData, isHeader2);

        if (biosData == freshBiosData)
        {
            messageBox.information(0, "Information", "Couldn't find the checksum check, patch not applied!");
        }
        else
        {
            PInterface->btnSave->setEnabled(TRUE);
            PInterface->btnSaveAs->setEnabled(TRUE);

            messageBox.information(0, "Information", "Checksum check disabled, fixup success!");
        }
    }
    else
    {
        messageBox.critical(0, "Error", "Please load ROM first!");
        return;
    }
}

void HPU_Qt::Browse_Extract()
{
    savedModuleName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Module File"),
                                                    QDir::currentPath(),
                                                    tr("All Files (*.*)"));

    PInterface->linePath_Extract->setText(savedModuleName);

    if (!PInterface->linePath_Extract->text().isEmpty()) {
        PInterface->btnExtract->setEnabled(TRUE);
    }
}

void HPU_Qt::Browse_Replace()
{
    newModuleName = QFileDialog::getOpenFileName(this,
        tr("New Module File"),
        QDir::currentPath(),
        tr("All Files (*.*)"));

    PInterface->linePath_Replace->setText(newModuleName);

    if (!PInterface->linePath_Replace->text().isEmpty()) {
        PInterface->btnReplace->setEnabled(TRUE);
    }
}

bool HPU_Qt::CheckLoaded(bool firstIn)
{
    /* Способ просмотра property
    QObject *tableWidgetObject = PInterface->tableWidget;

    
    QVariant propertyValue = tableWidgetObject->property("selectionMode");
    if (propertyValue.isValid()) {
        int propertyValueInt = propertyValue.toInt();
        if (propertyValueInt == QAbstractItemView::SingleSelection) {
            ...

        }
    }*/
    if (firstIn) {
        PInterface->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        PInterface->btnSave->setDisabled(TRUE);
        PInterface->btnSaveAs->setDisabled(TRUE);
        PInterface->btnCreateReport->setDisabled(TRUE);
        PInterface->tabWidget->setDisabled(TRUE);
        PInterface->btnExtract->setDisabled(TRUE);
        PInterface->btnReplace->setDisabled(TRUE);
    }
    else if (!PInterface->tableWidget->item(0, 0)->text().isEmpty()) {
        PInterface->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        PInterface->tableWidget->setEnabled(TRUE);
        PInterface->tabWidget->setEnabled(TRUE);
        PInterface->btnCreateReport->setEnabled(TRUE);

        return TRUE;
    }
    else
    {
        messageBox.critical(0, "Error", "Internal error! tableWidgetObject property is not valid.");
        return FALSE;
    }

    return FALSE;
}

void HPU_Qt::SetRows(uint8_t lineNumber, QStringList parts)
{
    PInterface->tableWidget->setItem(lineNumber, 0, new QTableWidgetItem(QString::number(lineNumber)));
    PInterface->tableWidget->setItem(lineNumber, 1, new QTableWidgetItem(parts[0]));
    PInterface->tableWidget->setItem(lineNumber, 2, new QTableWidgetItem(parts[1]));
    PInterface->tableWidget->setItem(lineNumber, 3, new QTableWidgetItem(parts[2]));
    PInterface->tableWidget->setItem(lineNumber, 4, new QTableWidgetItem(parts[3]));
    PInterface->tableWidget->setItem(lineNumber, 5, new QTableWidgetItem(parts[4]));
}

void HPU_Qt::ResetRows(uint8_t lineNumber)
{
    while (lineNumber <= PInterface->tableWidget->rowCount()) {
        for (uint8_t i = 0; i < PInterface->tableWidget->columnCount(); i++) {
            PInterface->tableWidget->setItem(lineNumber, i, new QTableWidgetItem(QString("")));
        }
        lineNumber++;
    }
}

HPU_Qt::HPU_Qt(QWidget *parent)
    : QMainWindow(parent)
    , PInterface(new Ui::HPU_QtClass())
{
    PInterface->setupUi(this);
    messageBox.setFixedSize(500, 200);
    InitButtons();
}

HPU_Qt::~HPU_Qt()
{
    delete PInterface;
}