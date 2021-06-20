/**
 * VGMTrans (c) - 2002-2021
 * Licensed under the zlib license
 * See the included LICENSE for more information
 */

#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QSplitter>
#include <QFileDialog>
#include <QStandardPaths>
#include "MainWindow.h"
#include "QtVGMRoot.h"
#include "MenuBar.h"
#include "IconBar.h"
#include "About.h"
#include "Logger.h"
#include "MusicPlayer.h"
#include "workarea/RawFileListView.h"
#include "workarea/VGMFileListView.h"
#include "workarea/VGMCollListView.h"
#include "workarea/MdiArea.h"

const int defaultWindowWidth = 800;
const int defaultWindowHeight = 600;
const int defaultCollListHeight = 140;
const int defaultFileListWidth = 200;
const int splitterHandleWidth = 1;

MainWindow::MainWindow() : QMainWindow(nullptr) {
  setWindowTitle("VGMTrans");
  setWindowIcon(QIcon(":/vgmtrans.png"));

  setUnifiedTitleAndToolBarOnMac(true);
  setAcceptDrops(true);
  setContextMenuPolicy(Qt::NoContextMenu);

  createElements();
  routeSignals();

  resize(defaultWindowWidth, defaultWindowHeight);
}

void MainWindow::createElements() {
  m_menu_bar = new MenuBar(this);
  setMenuBar(m_menu_bar);
  m_icon_bar = new IconBar(this);
  addToolBar(m_icon_bar);

  rawFileListView = new RawFileListView();
  vgmFileListView = new VGMFileListView();
  m_vgmcollview = new VGMCollListView();

  vertSplitter = new QSplitter(Qt::Vertical, this);
  horzSplitter = new QSplitter(Qt::Horizontal, vertSplitter);
  vertSplitterLeft = new QSplitter(Qt::Vertical, horzSplitter);

  QList<int> sizes({defaultWindowHeight - defaultCollListHeight, defaultCollListHeight});
  vertSplitter->addWidget(horzSplitter);
  vertSplitter->addWidget(m_vgmcollview);
  vertSplitter->setStretchFactor(0, 1);
  vertSplitter->setSizes(sizes);
  vertSplitter->setHandleWidth(splitterHandleWidth);

  sizes = QList<int>({defaultFileListWidth, defaultWindowWidth - defaultFileListWidth});
  horzSplitter->addWidget(vertSplitterLeft);
  horzSplitter->addWidget(MdiArea::the());
  horzSplitter->setStretchFactor(1, 1);
  horzSplitter->setSizes(sizes);
  horzSplitter->setHandleWidth(splitterHandleWidth);
  horzSplitter->setMinimumSize(100, 100);
  horzSplitter->setMaximumSize(500, 0);
  horzSplitter->setCollapsible(0, false);
  horzSplitter->setCollapsible(1, false);

  vertSplitterLeft->addWidget(rawFileListView);
  vertSplitterLeft->addWidget(vgmFileListView);
  vertSplitterLeft->setHandleWidth(splitterHandleWidth);

  setCentralWidget(vertSplitter);

  m_logger = new Logger();
  addDockWidget(Qt::BottomDockWidgetArea, m_logger);
}

void MainWindow::routeSignals() {
  connect(m_menu_bar, &MenuBar::OpenFile, this, &MainWindow::OpenFile);
  connect(m_menu_bar, &MenuBar::Exit, this, &MainWindow::close);
  connect(m_menu_bar, &MenuBar::ShowAbout, [=]() {
    About about(this);
    about.exec();
  });
  connect(m_menu_bar, &MenuBar::ShowLogger, m_logger, &Logger::setVisible);

  connect(m_icon_bar, &IconBar::openPressed, this, &MainWindow::OpenFile);
  connect(m_icon_bar, &IconBar::playToggle, m_vgmcollview,
          &VGMCollListView::HandlePlaybackRequest);
  connect(m_icon_bar, &IconBar::stopPressed, m_vgmcollview, &VGMCollListView::HandleStopRequest);
  connect(m_icon_bar, &IconBar::seekingTo, &MusicPlayer::the(), &MusicPlayer::seek);

  connect(m_logger, &Logger::closeEvent, m_menu_bar, &MenuBar::SetLoggerHidden);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
  event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
  const auto &files = event->mimeData()->urls();

  if (files.isEmpty())
    return;

  for (const auto &file : files) {
    qtVGMRoot.OpenRawFile(QFileInfo(file.toLocalFile()).filePath().toStdWString());
  }

  setBackgroundRole(QPalette::Dark);
  event->acceptProposedAction();
}

void MainWindow::OpenFile() {
  auto filenames = QFileDialog::getOpenFileNames(
      this, "Select a file...", QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
      "All files (*)");

  if (filenames.isEmpty())
    return;

  for (QString &filename : filenames) {
    qtVGMRoot.OpenRawFile(filename.toStdWString());
  }
}