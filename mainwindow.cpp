#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QStyle>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , processManager(new ProcessManager(this))
{
    // 加载背景图片
    backgroundPixmap = QPixmap(":/icon.png");

    setupUI();

    // 连接信号
    connect(processManager, &ProcessManager::processListUpdated,
            this, &MainWindow::onProcessListUpdated);

    // 初始加载进程列表
    QTimer::singleShot(100, this, &MainWindow::onRefreshClicked);
}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!backgroundPixmap.isNull()) {
        // 缩放图片以适应窗口
        QPixmap scaled = backgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                                 Qt::SmoothTransformation);

        // 居中绘制
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);

        // 添加半透明遮罩让背景变淡
        painter.fillRect(rect(), QColor(255, 255, 255, 180));
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("窗口置顶管理器 v1.0");
    resize(680, 450);
    setMinimumSize(500, 350);

    // 设置透明背景属性
    setAutoFillBackground(false);

    // 设置样式表 - 增强文字可见性
    setStyleSheet(R"(
    QMainWindow {
        background: transparent;
    }

    QWidget#centralWidget {
        background: transparent;
    }

    QPushButton {
        background-color: rgba(74, 144, 226, 200);
        color: black;
        border: 2px solid #4a90e2;
        padding: 6px 14px;
        border-radius: 4px;
        font-size: 12px;
        font-weight: bold;
        min-width: 70px;
    }

    QPushButton:hover {
        background-color: #357abd;
        color: black;
    }

    QPushButton:pressed {
        background-color: #2a5f9e;
        color: black;
    }

    QPushButton:disabled {
        background-color: #b0b0b0;
        color: #666666;
    }

    QCheckBox {
        spacing: 5px;
        font-size: 12px;
        color: black;
    }

    QCheckBox::indicator {
        width: 16px;
        height: 16px;
        border: 2px solid #4a90e2;
        border-radius: 3px;
        background-color: white;
    }

    QCheckBox::indicator:checked {
        background-color: #4a90e2;
    }

    QTableWidget {
        background-color: white;
        border: 1px solid #ddd;
        border-radius: 6px;
        gridline-color: #f0f0f0;
        font-size: 12px;
        color: black;
        selection-background-color: #e3f2fd;
        selection-color: black;
    }

    QTableWidget::item {
        padding: 5px;
        border-bottom: 1px solid #f0f0f0;
        color: black;
    }

    QTableWidget::item:selected {
        background-color: #d0e8ff;
        color: black;
    }

    QHeaderView::section {
        background-color: #667eea;
        color: white;
        padding: 6px;
        border: none;
        font-weight: bold;
        font-size: 12px;
    }

    QScrollBar:vertical {
        background: #f0f0f0;
        width: 8px;
        border-radius: 4px;
    }

    QScrollBar::handle:vertical {
        background: #c0c0c0;
        border-radius: 4px;
        min-height: 20px;
    }

    QScrollBar::handle:vertical:hover {
        background: #a0a0a0;
    }

    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
        height: 0px;
    }

    QLabel#statusLabel {
        color: black;
        font-size: 12px;
        padding: 2px;
    }
)");

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setStyleSheet("background: transparent;");
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // 工具栏区域
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    refreshBtn = new QPushButton("刷新列表", this);
    refreshBtn->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topMostBtn = new QPushButton("设置置顶", this);
    topMostBtn->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    topMostBtn->setCursor(Qt::PointingHandCursor);
    topMostBtn->setEnabled(false);

    bringToTopBtn = new QPushButton("前置窗口", this);
    bringToTopBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    bringToTopBtn->setCursor(Qt::PointingHandCursor);
    bringToTopBtn->setEnabled(false);

    autoRefreshCheck = new QCheckBox("自动刷新 (2秒)", this);

    statusLabel = new QLabel("就绪", this);
    statusLabel->setObjectName("statusLabel");

    toolbarLayout->addWidget(refreshBtn);
    toolbarLayout->addWidget(topMostBtn);
    toolbarLayout->addWidget(bringToTopBtn);
    toolbarLayout->addWidget(autoRefreshCheck);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(statusLabel);

    mainLayout->addLayout(toolbarLayout);

    // 进程列表表格
    processTable = new QTableWidget(this);
    processTable->setColumnCount(4);
    processTable->setHorizontalHeaderLabels({"进程名", "PID", "窗口标题", "置顶状态"});
    processTable->setColumnWidth(0, 130);
    processTable->setColumnWidth(1, 60);
    processTable->setColumnWidth(2, 280);
    processTable->setColumnWidth(3, 80);
    processTable->horizontalHeader()->setStretchLastSection(false);
    processTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processTable->setSelectionMode(QAbstractItemView::SingleSelection);
    processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    processTable->setAlternatingRowColors(true);
    processTable->setShowGrid(false);
    processTable->verticalHeader()->setVisible(false);
    processTable->setMouseTracking(true);

    mainLayout->addWidget(processTable);

    // 连接信号槽
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(topMostBtn, &QPushButton::clicked, this, &MainWindow::onSetTopMostClicked);
    connect(bringToTopBtn, &QPushButton::clicked, this, &MainWindow::onBringToTopClicked);
    connect(autoRefreshCheck, &QCheckBox::toggled, this, &MainWindow::onAutoRefreshToggled);
    connect(processTable, &QTableWidget::cellClicked, this, &MainWindow::onTableItemClicked);
}

void MainWindow::onRefreshClicked()
{
    statusLabel->setText("正在刷新...");
    QApplication::processEvents();

    QList<ProcessInfo> processes = processManager->getProcessList();

    if (processes.isEmpty()) {
        statusLabel->setText("未找到窗口，请确保有打开的程序窗口");
    } else {
        populateTable(processes);
        statusLabel->setText(QString("就绪 - 共 %1 个窗口").arg(processes.size()));
    }
}

void MainWindow::onProcessListUpdated(const QList<ProcessInfo>& processes)
{
    populateTable(processes);
    statusLabel->setText(QString("自动刷新 - 共 %1 个窗口 [%2]")
                             .arg(processes.size())
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void MainWindow::populateTable(const QList<ProcessInfo>& processes)
{
    processTable->setRowCount(0);
    processTable->setRowCount(processes.size());

    for (int i = 0; i < processes.size(); ++i) {
        const ProcessInfo &info = processes[i];

        QTableWidgetItem *nameItem = new QTableWidgetItem(info.name);
        nameItem->setData(Qt::UserRole, QVariant::fromValue((quint64)info.hwnd));

        QTableWidgetItem *pidItem = new QTableWidgetItem(QString::number(info.pid));
        QTableWidgetItem *titleItem = new QTableWidgetItem(info.windowTitle);

        bool isTopMost = processManager->isWindowTopMost(info.hwnd);
        QTableWidgetItem *topMostItem = new QTableWidgetItem(isTopMost ? "✓ 已置顶" : "✗ 未置顶");

        if (isTopMost) {
            topMostItem->setForeground(QColor("#00aa00"));
        } else {
            topMostItem->setForeground(QColor("#888888"));
        }

        processTable->setItem(i, 0, nameItem);
        processTable->setItem(i, 1, pidItem);
        processTable->setItem(i, 2, titleItem);
        processTable->setItem(i, 3, topMostItem);
    }
}

ProcessInfo MainWindow::getSelectedProcess()
{
    ProcessInfo info;
    int row = processTable->currentRow();
    if (row >= 0) {
        QTableWidgetItem *item = processTable->item(row, 0);
        info.hwnd = (HWND)item->data(Qt::UserRole).value<quint64>();
        info.name = processTable->item(row, 0)->text();
        info.pid = processTable->item(row, 1)->text().toULong();
        info.windowTitle = processTable->item(row, 2)->text();
    }
    return info;
}

void MainWindow::onTableItemClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)

    ProcessInfo info = getSelectedProcess();
    if (info.hwnd) {
        topMostBtn->setEnabled(true);
        bringToTopBtn->setEnabled(true);

        bool isTopMost = processManager->isWindowTopMost(info.hwnd);
        if (isTopMost) {
            topMostBtn->setText("取消置顶");
            topMostBtn->setStyleSheet("background-color: #e74c3c; color: white; border: none; padding: 6px 14px; border-radius: 4px; font-size: 12px; font-weight: bold;");
        } else {
            topMostBtn->setText("设置置顶");
            topMostBtn->setStyleSheet("");
        }
    } else {
        topMostBtn->setEnabled(false);
        bringToTopBtn->setEnabled(false);
        topMostBtn->setText("设置置顶");
        topMostBtn->setStyleSheet("");
    }
}

void MainWindow::onSetTopMostClicked()
{
    ProcessInfo info = getSelectedProcess();
    if (!info.hwnd) {
        QMessageBox::warning(this, "提示", "请先选择一个窗口");
        return;
    }

    if (!IsWindow(info.hwnd)) {
        QMessageBox::warning(this, "窗口不存在",
                             QString("窗口 \"%1\" 已经不存在了，请刷新列表")
                                 .arg(info.windowTitle));
        onRefreshClicked();
        return;
    }

    bool isTopMost = processManager->isWindowTopMost(info.hwnd);
    bool success = processManager->setWindowTopMost(info.hwnd, !isTopMost);

    if (success) {
        QString msg = !isTopMost ?
                          QString("窗口 \"%1\" 已成功设置为置顶").arg(info.windowTitle) :
                          QString("窗口 \"%1\" 已取消置顶").arg(info.windowTitle);
        statusLabel->setText(msg);
        QTimer::singleShot(500, this, &MainWindow::onRefreshClicked);
    } else {
        DWORD error = GetLastError();
        QString errorMsg;
        switch (error) {
        case ERROR_INVALID_WINDOW_HANDLE:
            errorMsg = "无效的窗口句柄";
            break;
        case ERROR_ACCESS_DENIED:
            errorMsg = "访问被拒绝，可能需要管理员权限";
            break;
        default:
            errorMsg = QString("未知错误 (代码: %1)").arg(error);
        }

        QMessageBox::critical(this, "设置失败",
                              QString("无法设置窗口 \"%1\" 为置顶\n%2\n\n"
                                      "建议：\n"
                                      "1. 刷新列表后重试\n"
                                      "2. 检查窗口是否完全加载\n"
                                      "3. 以管理员身份运行本程序")
                                  .arg(info.windowTitle)
                                  .arg(errorMsg));
    }
}

void MainWindow::onBringToTopClicked()
{
    ProcessInfo info = getSelectedProcess();
    if (!info.hwnd) {
        QMessageBox::warning(this, "提示", "请先选择一个窗口");
        return;
    }

    processManager->bringWindowToTop(info.hwnd);
    statusLabel->setText(QString("已将窗口 \"%1\" 前置").arg(info.windowTitle));
}

void MainWindow::onAutoRefreshToggled(bool checked)
{
    if (checked) {
        processManager->startAutoRefresh(2000);
        refreshBtn->setEnabled(false);
        statusLabel->setText("自动刷新已开启");
    } else {
        processManager->stopAutoRefresh();
        refreshBtn->setEnabled(true);
        statusLabel->setText("自动刷新已关闭");
    }
}
