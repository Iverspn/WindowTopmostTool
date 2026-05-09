#include "processmanager.h"
#include <psapi.h>

static QList<ProcessInfo>* g_processList = nullptr;
static ProcessManager* g_manager = nullptr;

ProcessManager::ProcessManager(QObject *parent)
    : QObject(parent)
    , refreshTimer(new QTimer(this))
{
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        QList<ProcessInfo> processes = getProcessList();
        emit processListUpdated(processes);
    });
}

ProcessManager::~ProcessManager()
{
    stopAutoRefresh();
}

QList<ProcessInfo> ProcessManager::getProcessList()
{
    QList<ProcessInfo> processList;
    g_processList = &processList;
    g_manager = this;

    EnumWindows(enumWindowsCallback, 0);

    cachedProcesses = processList;
    return processList;
}

BOOL CALLBACK ProcessManager::enumWindowsCallback(HWND hwnd, LPARAM lParam)
{
    Q_UNUSED(lParam)

    if (!g_manager || !g_processList)
        return TRUE;

    if (!IsWindowVisible(hwnd))
        return TRUE;

    if (!g_manager->isMainWindow(hwnd))
        return TRUE;

    QString title = g_manager->getWindowTitle(hwnd);
    if (title.isEmpty())
        return TRUE;

    ProcessInfo info;
    info.hwnd = hwnd;
    info.windowTitle = title;
    info.pid = g_manager->getProcessId(hwnd);
    info.name = g_manager->getProcessName(info.pid);

    bool exists = false;
    for (const auto& p : *g_processList) {
        if (p.pid == info.pid && p.windowTitle == info.windowTitle) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        g_processList->append(info);
    }

    return TRUE;
}

QString ProcessManager::getWindowTitle(HWND hwnd)
{
    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    return QString::fromWCharArray(title);
}

DWORD ProcessManager::getProcessId(HWND hwnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

QString ProcessManager::getProcessName(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess) {
        wchar_t name[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, name, &size)) {
            CloseHandle(hProcess);
            QString fullPath = QString::fromWCharArray(name);
            return fullPath.split('\\').last();
        }
        CloseHandle(hProcess);
    }
    return QString("Unknown (PID: %1)").arg(pid);
}

bool ProcessManager::isMainWindow(HWND hwnd)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (!(style & WS_VISIBLE))
        return false;

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW)
        return false;

    HWND parent = GetParent(hwnd);
    if (parent != nullptr)
        return false;

    HWND owner = GetWindow(hwnd, GW_OWNER);
    if (owner != nullptr)
        return false;

    return true;
}

bool ProcessManager::setWindowTopMost(HWND hwnd, bool topMost)
{
    if (!hwnd || !IsWindow(hwnd)) {
        qDebug() << "Invalid window handle:" << hwnd;
        return false;
    }

    int retryCount = 0;
    const int maxRetries = 10;

    while (retryCount < maxRetries) {
        DWORD_PTR result;
        LRESULT ret = SendMessageTimeout(hwnd, WM_NULL, 0, 0,
                                         SMTO_ABORTIFHUNG, 1000, &result);
        if (ret != 0) {
            break;
        }
        Sleep(200);
        retryCount++;
    }

    HWND insertAfter = topMost ? HWND_TOPMOST : HWND_NOTOPMOST;
    UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS;

    BOOL success = SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0, flags);

    if (!success) {
        DWORD error = GetLastError();
        qDebug() << "SetWindowPos failed with error:" << error;
    }

    return success;
}

bool ProcessManager::isWindowTopMost(HWND hwnd)
{
    if (!IsWindow(hwnd))
        return false;

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    return (exStyle & WS_EX_TOPMOST) != 0;
}

bool ProcessManager::bringWindowToTop(HWND hwnd)
{
    if (!IsWindow(hwnd))
        return false;

    SetForegroundWindow(hwnd);
    return true;
}

void ProcessManager::startAutoRefresh(int intervalMs)
{
    refreshTimer->start(intervalMs);
}

void ProcessManager::stopAutoRefresh()
{
    refreshTimer->stop();
}
