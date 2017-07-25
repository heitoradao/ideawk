/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "awksyntaxhighlighter.h"

MainWindow::MainWindow()
    : m_ui(new Ui::MainWindow)
{
    init();
    setCurrentFile(QString());
}

MainWindow::MainWindow(const QString &fileName)
    : m_ui(new Ui::MainWindow)
{
    init();
    loadFile(fileName);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    MainWindow *other = new MainWindow;
    other->tile(this);
    other->show();
}

void MainWindow::open()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::openFile(const QString &fileName)
{
    MainWindow *existing = findMainWindow(fileName);
    if (existing) {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }

    if (isUntitled && textEdit->document()->isEmpty() && !isWindowModified()) {
        loadFile(fileName);
        return;
    }

    MainWindow *other = new MainWindow(fileName);
    if (other->isUntitled) {
        delete other;
        return;
    }
    other->tile(this);
    other->show();
}

bool MainWindow::save()
{
    return isUntitled ? saveAs() : saveFile(mAwkProgram);
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    mAwkProgram);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About IDEAWK"),
            tr("The <b>SDI</b> example demonstrates how to write single "
               "document interface applications using Qt."));
}

void MainWindow::documentWasModified()
{
    setWindowModified(true);
}

void MainWindow::loadInput()
{
    QString filters(tr("All files (*.*);;AWK files (*.awk)"));
    QString selectedFilter(tr("AWK files (*.awk)"));
    QString inputFilePath =
    QFileDialog::getOpenFileName(this, qApp->applicationDisplayName(),
                                 QStringLiteral(""),
                                 filters,
                                 &selectedFilter);

    if (inputFilePath.isEmpty()) {
        return;
    }

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        return;
    }
    mInputPath = inputFilePath;
    m_ui->textEditInputAwk->setPlainText(inputFile.readAll());
}

void MainWindow::run()
{
    QProcess *process = new QProcess;
    QString program("awk");
    QStringList arguments;
    arguments << "-f" << mAwkProgram << mInputPath;
    //connect(process, &QIODevice::finished, this, &MainWindow::readOutputFromAwk);
    connect(process, SIGNAL(finished(int)), this, SLOT(readOutputFromAwk()));
    process->start(program, arguments);

    // take the content of input and store in a temp file
    // store the path of this temp file
    // invoke awk using QProcess, passing the program path as the argument
    // and the path of temporary file wich is holding the input to the program
}

void MainWindow::readOutputFromAwk()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    QString output = process->readAllStandardOutput();
    m_ui->textBrowser->setPlainText(output);
    process->deleteLater();
}

void MainWindow::init()
{
    m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    isUntitled = true;

    textEdit = m_ui->textEditAwkProgram;


#ifdef Q_OS_LINUX
    KSyntaxHighlighter syntaxHighlighter;
#endif

    new AwkSyntaxHighlighter(textEdit->document());





    createActions();
    createStatusBar();

    readSettings();

    connect(textEdit->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::documentWasModified);

    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::tile(const QMainWindow *previous)
{
    if (!previous)
        return;
    int topFrameWidth = previous->geometry().top() - previous->pos().y();
    if (!topFrameWidth) {
        topFrameWidth = 40;
    }
    const QPoint pos = previous->pos() + 2 * QPoint(topFrameWidth, topFrameWidth);
    if (QApplication::desktop()->availableGeometry(this).contains(rect().bottomRight() + pos)) {
        move(pos);
    }
}

//! [implicit tr context]
void MainWindow::createActions()
{
    QMenu *fileMenu = m_ui->menuFile;
//! [implicit tr context]

    // new
    m_ui->actionNew->setShortcuts(QKeySequence::New);
    connect(m_ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);

    // open
    m_ui->actionOpen->setShortcuts(QKeySequence::Open);
    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::open);

    // save
    m_ui->actionSave->setShortcuts(QKeySequence::Save);
    connect(m_ui->actionSave, &QAction::triggered, this, &MainWindow::save);


    m_ui->actionSaveAs->setShortcuts(QKeySequence::SaveAs);

    fileMenu->addSeparator();

    QMenu *recentMenu = m_ui->menuRecentFiles;
    connect(m_ui->menuRecentFiles, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    mRecentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    connect(m_ui->actionClose, SIGNAL(triggered(bool)), this, SLOT(close()));

    // exit
    connect(m_ui->actionExit, SIGNAL(triggered(bool)), qApp, SLOT(closeAllWindows()));
    m_ui->actionExit->setShortcuts(QKeySequence::Quit);

#ifndef QT_NO_CLIPBOARD
    // cut
    m_ui->actionCut->setShortcuts(QKeySequence::Cut);
    connect(m_ui->actionCut, &QAction::triggered, textEdit, &QTextEdit::cut);

    // copy
    m_ui->actionCopy->setShortcuts(QKeySequence::Copy);
    connect(m_ui->actionCopy, &QAction::triggered, textEdit, &QTextEdit::copy);

    // paste
    m_ui->actionPaste->setShortcuts(QKeySequence::Paste);
    connect(m_ui->actionPaste, &QAction::triggered, textEdit, &QTextEdit::paste);

    menuBar()->addSeparator();
#endif // !QT_NO_CLIPBOARD

    // the AWK related things
    connect(m_ui->actionLoadInput, &QAction::triggered, this, &MainWindow::loadInput);
    connect(m_ui->actionRun, &QAction::triggered, this, &MainWindow::run);



    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}



/*!
 * \return Should exit?
 */
bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

#if 0
    QMessageBox *messagebox = new QMessageBox(this);
    ret = messagebox->exec();
#else
    const QMessageBox::StandardButton
    ret = QMessageBox::warning(this, qApp->applicationDisplayName(),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard
                               | QMessageBox::Cancel);
#endif


    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::setRecentFilesVisible(bool visible)
{
    mRecentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
static inline QString fileKey() { return QStringLiteral("file"); }

static QStringList readRecentFiles(QSettings &settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

static void writeRecentFiles(const QStringList &files, QSettings &settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

bool MainWindow::hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MainWindow::prependToRecentFiles(const QString &fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);

    setRecentFilesVisible(!recentFiles.isEmpty());
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for ( ; i < count; ++i) {
        const QString fileName = MainWindow::strippedName(recentFiles.at(i));
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for ( ; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);
}

void MainWindow::openRecentFile()
{
    if (const QAction *action = qobject_cast<const QAction *>(sender()))
        openFile(action->data().toString());
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SDI"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    static int sequenceNumber = 1;

    isUntitled = fileName.isEmpty();
    if (isUntitled) {
        mAwkProgram = tr("document%1.awk").arg(sequenceNumber++);
    } else {
        mAwkProgram = QFileInfo(fileName).canonicalFilePath();
    }

    textEdit->document()->setModified(false);
    setWindowModified(false);

    if (!isUntitled && windowFilePath() != mAwkProgram)
        MainWindow::prependToRecentFiles(mAwkProgram);

    setWindowFilePath(mAwkProgram);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

MainWindow *MainWindow::findMainWindow(const QString &fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin && mainWin->mAwkProgram == canonicalFilePath)
            return mainWin;
    }

    return nullptr;
}
